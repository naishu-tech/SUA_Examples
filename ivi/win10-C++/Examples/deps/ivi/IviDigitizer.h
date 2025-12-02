#ifndef IVI_IVIDIGITIZER_H
#define IVI_IVIDIGITIZER_H

#include "IviBase.h"
#include "jsoncpp/json/json.h"

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
#include <queue>
#include <deque>

class BinarySemaphore {
private:
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> available_{false};

public:
    explicit BinarySemaphore(int initial_count = 0) noexcept
            : available_(initial_count > 0) {}

    // Move constructor and assignment
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

    // Copy disabled
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
//            available_.store(false, std::memory_order_release);
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
//            available_.store(false, std::memory_order_release);
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
//            available_.store(false, std::memory_order_release);
            return true;
        }
        return false;
    }

    bool available() const noexcept {
        return available_.load(std::memory_order_acquire);
    }
};


// C++17 optimized channel transfer state
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
            : isTransferring(std::move(other.isTransferring)) // Modified by tjf-20250912
            , dataReadySemaphore(std::move(other.dataReadySemaphore))
            , lastDataDepth(other.lastDataDepth.load())
            , lastCheckTime(other.lastCheckTime.load()) {
        other.lastDataDepth.store(0);
    }

    ChannelTransferState& operator=(ChannelTransferState&& other) noexcept {
        if (this != &other) {
            isTransferring= std::move(other.isTransferring); // Modified by tjf-20250912
            dataReadySemaphore = std::move(other.dataReadySemaphore);
            lastDataDepth.store(other.lastDataDepth.load());
            lastCheckTime.store(other.lastCheckTime.load());

            other.lastDataDepth.store(0);
        }
        return *this;
    }

    // Copy disabled
    ChannelTransferState(const ChannelTransferState&) = delete;
    ChannelTransferState& operator=(const ChannelTransferState&) = delete;

    // Get last check time
    std::chrono::steady_clock::time_point getLastCheckTime() const noexcept {
        auto count = lastCheckTime.load(std::memory_order_acquire);
        return std::chrono::steady_clock::time_point{std::chrono::steady_clock::duration{count}};
    }

    // Update last check time
    void updateLastCheckTime() noexcept {
        lastCheckTime.store(std::chrono::steady_clock::now().time_since_epoch().count(),
                            std::memory_order_release);
    }
};

struct iviDigitizer_memData{
    nsuMemory_p memData{};
    ViString memDataType = "ViInt16";
    ViUInt32 memDataSize=0;
    char *memDataHandle{};
};

struct Ringbuffer_DMA_channel_struct
{
    ViConstString channelName{}; // DMA read channel
    ViUInt32 waveformArraySize{}; // DMA read size
    iviDigitizer_memData *waveformArray{}; // DMA read data pointer
    ViReal64 startTime_s{}; // Time when Read function starts
    ViReal64 maximumTime_s{}; // DMA read maximum time
    const char *dataTypeName{}; // DMA read data type

    // Use condition variable to implement blocking wait
    std::shared_ptr<std::condition_variable> completion_cv;
    std::shared_ptr<bool> is_success; // DMA read success flag (thread wakeup indicates DMA completion)
};

struct iviDigitizer_ViSession{
    iviBase_ViSession* vi{};

    std::mutex mtx;
    ViReal64 maxSampleRate = 5000000000.0;
    ViReal64 maxChannelSampleRate = 5000000000.0;
    ViUInt32 channelNumber = 2;
    bool is_locked = false;

    std::mutex wait_mtx;
    std::mutex lock_fetch;

    std::mutex mtx_fetchInt16;
    std::mutex mtx_fetchInt8;
    
    // DMA transfer dedicated lock - global lock, shared by all channels
    std::mutex dma_transfer_lock;

    // Data ready channel queue - stores channel numbers with ready data, supports query and FIFO operations
    std::deque<ViUInt32> dataReadyChannelQueue;
    std::mutex queueMutex; // Mutex to protect the queue

    std::map<ViUInt32, channelInfor> channelInforMap{};
    std::map<ViUInt32, channelXDMAInfor> channelXDMAInforMap{};
    std::map<ViInt32, ViBoolean> isTargetFound{};
    std::map<ViInt32, ViBoolean> isTimeout{};

    ViUInt32 chanlNum;

    ViUInt32 dataDepth;
    std::mutex mtxKeepRunning;
    std::mutex mtxReadWfm;
    std::mutex mtxRead;
    std::atomic<ViBoolean> keepRunning = false;
    std::map<ViInt32, std::condition_variable> dataDepthMap{};
    std::thread queryWorkThread;

    ViReal64 timeOut = 1.0;
    ViInt32 waitTimes = 3;

    // Use unordered_map to improve lookup performance
    std::map<std::int32_t, ChannelTransferState> channelTransferStates;
    mutable std::shared_mutex queryThreadMutex;  // Read-write lock
    std::atomic<bool> queryThreadActive{true};

    // Use C++17 inline variable optimization
    inline static constexpr std::chrono::milliseconds defaultPollInterval{1};
    inline static constexpr std::chrono::microseconds channelInterval{1};

    // Statistics
    std::atomic<std::uint64_t> totalDataReadySignals{0};
    std::atomic<std::uint64_t> totalDataTransfers{0};
    std::atomic<std::uint64_t> totalPollCycles{0};

    // Performance monitoring
    mutable std::mutex statsMutex;
    std::chrono::steady_clock::time_point startTime{std::chrono::steady_clock::now()};

    // New non-blocking control variables
    std::mutex wakeupMutex;
    std::condition_variable wakeupCV;
    std::atomic<bool> forceWakeup{false};
    std::atomic<bool> quickExit{false};

    ViUInt32 chnlLevelEnabled = 0b11111111;

    std::queue<std::shared_ptr<Ringbuffer_DMA_channel_struct>> Ringbuffer_DMA_channel_queue;
    std::mutex Ringbuffer_queue_mutex; // Queue operation lock
    std::mutex Ringbuffer_state_mutex; // State variable protection lock

    // Use atomic variables to ensure thread safety
    std::atomic<ViUInt8> ringbuffer_res{0};
    ViUInt8 ringbuffer_ChaFinMask=0x00; // Channel transfer completion mask, used in sync mode to determine if all channels have completed transfer

    ViUInt8 ringbuffer_ChaEnMask=0x00; // Channel enable mask, used in sync mode to determine if all channels are ready, the initial value of this parameter needs to be set in the init function
    bool is_AllChannelReady = false;
};



DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_Initialize (const ViString& logicalName, ViBoolean IDQuery, ViBoolean resetDevice, iviDigitizer_ViSession *vi, const ViString& resourceDBPath = "./resourceDB.json");
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_Reset (iviDigitizer_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_Close (iviDigitizer_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_RFConfig (iviDigitizer_ViSession *vi);

DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, const ViString& attributeValue);
//DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttribute (ViSession *vi, ViConstString channel, ViAttr attributeID, waveformHandle *attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViString *attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_CheckAttributeViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_CheckAttributeViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_CheckAttributeViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_CheckAttributeViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_CheckAttributeViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_CheckAttributeViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMaxValueViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession *attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_GetAttributeRangeMinValueViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString *attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession* attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMaxValueViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViUInt32 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViUInt32 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViReal64 (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViReal64 attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViBoolean (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViBoolean attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViSession (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, iviDigitizer_ViSession* attributeValue);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_SetAttributeRangeMinValueViString (iviDigitizer_ViSession *vi, ViConstString channel, ViAttr attributeID, ViConstString attributeValue);

DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_Abort (iviDigitizer_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_InitiateAcquisition (iviDigitizer_ViSession *vi);

DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_WaitForAcquisitionComplete (iviDigitizer_ViSession *vi, ViConstString channelName, ViReal64 maximumTime_s = 1.0);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_FetchWaveformMemInt16 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, iviDigitizer_memData* waveformArray);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_FetchWaveformInt16 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, ViInt16 * waveformArray);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_FetchWaveformMemInt8 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, iviDigitizer_memData* waveformArray);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_FetchWaveformInt8 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, ViInt8 * waveformArray);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_TriggerStart (iviDigitizer_ViSession *vi, ViConstString channelName);

DLLEXTERN RIGOLLIB_API iviDigitizer_memData* IviDigitizer_CreateMemInt16 (iviDigitizer_ViSession *vi, ViUInt32 memDataSize);
DLLEXTERN RIGOLLIB_API iviDigitizer_memData* IviDigitizer_CreateMemInt8 (iviDigitizer_ViSession *vi, ViUInt32 memDataSize);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_ClearMem (iviDigitizer_ViSession *vi, iviDigitizer_memData* memData);

DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_ConfigureChannelDataDepth(iviDigitizer_ViSession *vi, const std::string& channelsName, ViUInt32 dataDepth);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_ConfigureChannelDataDepthInt16(iviDigitizer_ViSession *vi, const std::string& channelsName, ViUInt32 dataDepth);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_ConfigureChannelDataDepthInt8(iviDigitizer_ViSession *vi, const std::string& channelsName, ViUInt32 dataDepth);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_ReadWaveformInt16 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, iviDigitizer_memData* waveformArray, ViReal64 maximumTime_s);
DLLEXTERN RIGOLLIB_API ViStatus IviDigitizer_ReadWaveformInt8 (iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, iviDigitizer_memData* waveformArray, ViReal64 maximumTime_s);

DLLEXTERN RIGOLLIB_API ViStatus Ringbuffer_queryFunction(iviDigitizer_ViSession *vi,bool is_sync);
DLLEXTERN RIGOLLIB_API ViStatus Ringbuffer_readFunction(iviDigitizer_ViSession *vi, ViConstString channelName, ViUInt32 waveformArraySize, iviDigitizer_memData *waveformArray, ViReal64 maximumTime_s, const char *dataTypeName);
#endif //IVI_IVIDIGITIZER_H
