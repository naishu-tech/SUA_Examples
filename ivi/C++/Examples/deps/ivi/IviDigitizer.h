#ifndef IVI_IVIDIGITIZER_H
#define IVI_IVIDIGITIZER_H

#include "IviBase.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

class BinarySemaphore {
private:
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> available_{false};

public:
    explicit BinarySemaphore(int initial_count = 0) noexcept
            : available_(initial_count > 0) {}

    // 移动构造和赋值
    BinarySemaphore(BinarySemaphore&& other) noexcept
            : available_(other.available_.load()) {
        other.available_.store(false);
    }

    BinarySemaphore& operator=(BinarySemaphore&& other) noexcept {
        if (this != &other) {
            available_.store(other.available_.load());
            other.available_.store(false);
        }
        return *this;
    }

    // 禁用拷贝
    BinarySemaphore(const BinarySemaphore&) = delete;
    BinarySemaphore& operator=(const BinarySemaphore&) = delete;

    void release() noexcept {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            available_.store(true, std::memory_order_release);
        }
        cv_.notify_one();
    }

    void acquire() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] {
            return available_.load(std::memory_order_acquire);
        });
        available_.store(false, std::memory_order_release);
    }

    bool try_acquire() noexcept {
        std::lock_guard<std::mutex> lock(mtx_);
        if (available_.load(std::memory_order_acquire)) {
            return true;
        }
        return false;
    }

    template<class Rep, class Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& timeout_duration) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (cv_.wait_for(lock, timeout_duration, [this] {
            return available_.load(std::memory_order_acquire);
        })) {
            return true;
        }
        return false;
    }

    template<class Clock, class Duration>
    bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& timeout_time) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (cv_.wait_until(lock, timeout_time, [this] {
            return available_.load(std::memory_order_acquire);
        })) {
            return true;
        }
        return false;
    }

    bool available() const noexcept {
        return available_.load(std::memory_order_acquire);
    }
};


// C++17 优化的通道传输状态
struct ChannelTransferState {
    BinarySemaphore isTransferring{0};
    BinarySemaphore dataReadySemaphore{0};
    mutable std::mutex transferMutex;
    std::atomic<std::uint32_t> lastDataDepth{0};
    std::atomic<std::chrono::steady_clock::time_point::rep> lastCheckTime{};

    ChannelTransferState() noexcept {
        lastCheckTime.store(std::chrono::steady_clock::now().time_since_epoch().count(),
                            std::memory_order_release);
    }

    ChannelTransferState(ChannelTransferState&& other) noexcept
            : isTransferring(std::move(other.dataReadySemaphore))
            , dataReadySemaphore(std::move(other.dataReadySemaphore))
            , lastDataDepth(other.lastDataDepth.load())
            , lastCheckTime(other.lastCheckTime.load()) {
        other.lastDataDepth.store(0);
    }

    ChannelTransferState& operator=(ChannelTransferState&& other) noexcept {
        if (this != &other) {
            isTransferring= std::move(other.dataReadySemaphore);
            dataReadySemaphore = std::move(other.dataReadySemaphore);
            lastDataDepth.store(other.lastDataDepth.load());
            lastCheckTime.store(other.lastCheckTime.load());

            other.lastDataDepth.store(0);
        }
        return *this;
    }

    // 禁用拷贝
    ChannelTransferState(const ChannelTransferState&) = delete;
    ChannelTransferState& operator=(const ChannelTransferState&) = delete;

    // 获取最后检查时间
    std::chrono::steady_clock::time_point getLastCheckTime() const noexcept {
        auto count = lastCheckTime.load(std::memory_order_acquire);
        return std::chrono::steady_clock::time_point{std::chrono::steady_clock::duration{count}};
    }

    // 更新最后检查时间
    void updateLastCheckTime() noexcept {
        lastCheckTime.store(std::chrono::steady_clock::now().time_since_epoch().count(),
                            std::memory_order_release);
    }
};

struct iviDigitizer_ViSession{
    iviBase_ViSession* vi{};

    std::mutex mtx;
    ViReal64 maxSampleRate = 8000000000.0;
    ViReal64 maxChannelSampleRate = 4000000000.0;
    ViUInt32 channelNumber = 2;
    bool is_locked = false;

    std::mutex wait_mtx;
    std::mutex lock_fetch;

    std::mutex mtx_fetchInt16;
    std::mutex mtx_fetchInt8;

    std::map<ViUInt32, channelInfor> channelInforMap{};
    std::map<ViUInt32, channelXDMAInfor> channelXDMAInforMap{};
    std::map<ViInt32, ViBoolean> isTargetFound{};
    std::map<ViInt32, ViBoolean> isTimeout{};

    ViUInt32 chanlNum;

    ViUInt32 dataDepth;
    std::mutex mtxKeepRunning;
    std::mutex mtxReadWfm;
    std::mutex mtxRead;
    std::atomic<ViBoolean> keepRunning;
    std::map<ViInt32, std::condition_variable> dataDepthMap{};
    std::thread queryWorkThread;

    ViReal64 timeOut = 1.0;
    ViInt32 waitTimes = 3;

    // 使用 unordered_map 提高查找性能
    std::map<std::int32_t, ChannelTransferState> channelTransferStates;
    mutable std::shared_mutex queryThreadMutex;  // 读写锁
    std::atomic<bool> queryThreadActive{true};

    // 使用 C++17 的内联变量优化
    inline static constexpr std::chrono::milliseconds defaultPollInterval{1};
    inline static constexpr std::chrono::microseconds channelInterval{1};

    // 统计信息
    std::atomic<std::uint64_t> totalDataReadySignals{0};
    std::atomic<std::uint64_t> totalDataTransfers{0};
    std::atomic<std::uint64_t> totalPollCycles{0};

    // 性能监控
    mutable std::mutex statsMutex;
    std::chrono::steady_clock::time_point startTime{std::chrono::steady_clock::now()};

    // 新增非阻塞控制变量
    std::mutex wakeupMutex;
    std::condition_variable wakeupCV;
    std::atomic<bool> forceWakeup{false};
    std::atomic<bool> quickExit{false};

};

struct iviDigitizer_memData{
    nsuMemory_p memData{};
    ViString memDataType = "ViInt16";
    ViUInt32 memDataSize=0;
    char *memDataHandle{};
};

DLLEXTERN MYLIB_API ViStatus IviDigitizer_Initialize (const ViString& logicalName, ViBoolean IDQuery, ViBoolean resetDevice, iviDigitizer_ViSession *vi, const ViString& resourceDBPath = "./resourceDB.json");
DLLEXTERN MYLIB_API ViStatus IviDigitizer_Reset (iviDigitizer_ViSession *vi);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_Close (iviDigitizer_ViSession *vi);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_RFConfig (iviDigitizer_ViSession *vi);

DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, const ViString& attributeValue);
//MYLIB_API ViStatus IviDigitizer_SetAttribute (ViSession *vi, ViConstString channel, ViAttr attributeID, waveformHandle *attributeValue);

DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViString *attributeValue);

DLLEXTERN MYLIB_API ViStatus IviDigitizer_CheckAttributeViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_CheckAttributeViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_CheckAttributeViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_CheckAttributeViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_CheckAttributeViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_CheckAttributeViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);

DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession* attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession* attributeValue);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN MYLIB_API ViStatus IviDigitizer_Abort (iviDigitizer_ViSession *vi);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_InitiateAcquisition (iviDigitizer_ViSession *vi);

DLLEXTERN MYLIB_API ViStatus IviDigitizer_WaitForAcquisitionComplete (iviDigitizer_ViSession *vi, ViConstString channelName, ViReal64 maximumTime_s = 1.0);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_FetchWaveformMemInt16 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, iviDigitizer_memData* waveformArray);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_FetchWaveformInt16 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, ViInt16 * waveformArray);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_FetchWaveformMemInt8 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, iviDigitizer_memData* waveformArray);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_FetchWaveformInt8 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, ViInt8 * waveformArray);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_TriggerStart (iviDigitizer_ViSession *vi, ViConstString channelName);

DLLEXTERN MYLIB_API iviDigitizer_memData* IviDigitizer_CreateMemInt16 (iviDigitizer_ViSession *vi, ViUInt32 memDataSize);
DLLEXTERN MYLIB_API iviDigitizer_memData* IviDigitizer_CreateMemInt8 (iviDigitizer_ViSession *vi, ViUInt32 memDataSize);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_ClearMem (iviDigitizer_ViSession *vi, iviDigitizer_memData* memData);

DLLEXTERN MYLIB_API ViStatus IviDigitizer_ConfigureChannelDataDepth(iviDigitizer_ViSession *vi, const std::string& channelsName, ViUInt32 dataDepth);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_ConfigureChannelDataDepthInt16(iviDigitizer_ViSession *vi, const std::string& channelsName, ViUInt32 dataDepth);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_ConfigureChannelDataDepthInt8(iviDigitizer_ViSession *vi, const std::string& channelsName, ViUInt32 dataDepth);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_ReadWaveformInt16 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, iviDigitizer_memData* waveformArray, ViReal64 maximumTime_s);
DLLEXTERN MYLIB_API ViStatus IviDigitizer_ReadWaveformInt8 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, iviDigitizer_memData* waveformArray, ViReal64 maximumTime_s);
#endif //IVI_IVIDIGITIZER_H
