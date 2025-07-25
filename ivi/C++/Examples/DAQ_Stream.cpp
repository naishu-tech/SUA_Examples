//
// Created by sn06129 on 2025/7/25.
//

#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <string>
#include <filesystem>

# include "IviSUATools.h"
# include "IviDigitizer.h"
# include "tool_config.h"

#define string2int(channelName, numChannel, m){     \
    char *end;                                      \
    numChannel = std::strtol(channelName, &end, m);}\

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    //
    void Push(T *value) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(value);
        lock.unlock();
        condition_.notify_one();
    }

    //
    T * Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !queue_.empty(); });
        auto value = (T *)queue_.front();
        queue_.pop();
        return value;
    }

private:
    std::queue<T *> queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
};


typedef ThreadSafeQueue<void> memQueue;

struct Deque {
    memQueue full{};
    memQueue empty{};
    int stopFlag = 0;
    int dataNum = 0;
};

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string temp;
    while (std::getline(ss, temp, delimiter)) {
        result.push_back(temp);
    }
    return result;
}

void write_file_thread(iviDigitizer_ViSession *vi, Deque *q, std::string channelName, ViUInt32 waveformArraySize, const std::string &path, std::mutex *mu) {
    std::unique_lock<std::mutex> *lock;
    iviDigitizer_memData* mem;
    std::ofstream outF;
//    outF.open(path + channelName + ".data", std::ofstream::binary);

    int cnt = 0;
    nsuSize_t speed_count = 0;
    auto st = std::chrono::steady_clock::now();
    while (true) {
        lock = new std::unique_lock<std::mutex>(*mu);
        if (q->stopFlag == 1 && q->dataNum == 0) {
            break;
        }
        delete lock;
        if (speed_count % 2147483648 == 0){
            if (outF.is_open()) {
                outF.close();
                cnt++;
            }
            outF.open(path + channelName + "_" + std::to_string(cnt) + ".data", std::ofstream::binary);
        }

        mem = reinterpret_cast<iviDigitizer_memData *>(q->full.Pop());
        outF.write(mem->memDataHandle, (waveformArraySize*sizeof(ViInt16)));
        q->empty.Push(reinterpret_cast<nsuMemory_p>(mem));
        lock = new std::unique_lock<std::mutex>(*mu);
        q->dataNum -= 1;
        delete lock;
        speed_count += waveformArraySize;
    }
    std::cout << std::endl;
    outF.close();
}

void upload_thread(iviDigitizer_ViSession *vi, Deque *q, std::string channelName, ViUInt32 waveformArraySize, ViUInt32 times, std::mutex *mu) {
    std::unique_lock<std::mutex> *lock;
    iviDigitizer_memData* mem;
    std::cout << "upload thread start: " << waveformArraySize << std::endl;
    ViStatus s;
    ViInt32 numChannel;
    std::vector<int> channelsInt;
    int dictionary = 0;
    int cnt = 0;
    nsuSize_t speed_count = 0;

    auto st = std::chrono::steady_clock::now();


    while (true) {
        if (dictionary > times) {
            break;
        }

        mem = reinterpret_cast<iviDigitizer_memData*>(q->empty.Pop());
        std::cout << "channel: " << channelName << " upData start " << std::endl;
        s = IviDigitizer_ReadWaveformInt16(vi, channelName.c_str(), waveformArraySize, mem, 0);
        dictionary += 1;
        speed_count += waveformArraySize;

        if (cnt % 20 == 0) {
            auto count = std::chrono::steady_clock::now() - st;
            std::cout << std::flush << '\r' << "current write file speed: " << speed_count*1000./(count.count()) << "MB/s" << std::endl;
            speed_count = 0;
            st = std::chrono::steady_clock::now();
        }
        cnt++;
        q->full.Push(reinterpret_cast<nsuMemory_p>(mem));
        lock = new std::unique_lock<std::mutex>(*mu);
        q->dataNum += 1;
        delete lock;
        std::cout << "channel: " << channelName << " upData successful!!!!!!!!!!!!" << std::endl;
    }
    lock = new std::unique_lock<std::mutex>(*mu);
    q->stopFlag = 1;
    delete lock;
}

int main(int argc, char *argv[]){

    auto iviSUATools_vi = new iviSUATools_ViSession;
    auto s = IviSUATools_Initialize(iviSUATools_vi);

    std::cout << "\n=== Creating Resource DB ===" << std::endl;
    std::string resource_db_path = IviSUATools_ScanOnlineBoards(iviSUATools_vi, "conf/config.yaml");
    std::cout << "BVC Manager initialized successfully. Resource DB: " << resource_db_path << std::endl;

    auto iviDigitizer_vi = new iviDigitizer_ViSession;
    s = IviDigitizer_Initialize("PXI::0::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviDigitizer_vi, resource_db_path);

    auto iviSyncATrig_vi = new iviSyncATrig_ViSession;
    s = IviSyncATrig_Initialize("PXI::1::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviSyncATrig_vi, resource_db_path);

    ViUInt32 waveformArraySize = 4 * 1024 * 1024;
    ViReal64 maximumTime_s = 0.1;
    ViReal64 times = 20;
    std::string channelName = "0,1";


    std::cout << "\n=== Trig Config ===" << std::endl;
    ViUInt32 triggerSource = IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_INTERNAL; //来源
    ViUInt32 triggerPeriod = 6400; // 周期(需要被800整除)
    ViUInt32 triggerRepetSize = 4294967295;// 触发数量
    ViUInt32 triggerPulseWidth = 1600; // 脉宽(为周期的一半)

    if (triggerSource == IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_INTERNAL){
        s = triggerConfigDAQ(iviDigitizer_vi, IVIFGEN_VAL_TRIGGER_SOURCE_PXI_STAR_TRIG);
        s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_TRIGGER_SOURCE_P_PXI_STAR, triggerSource);
        s = internalTriggerConfigSAT(iviSyncATrig_vi, triggerSource, triggerPeriod, triggerRepetSize, triggerPulseWidth);
    } else if (triggerSource == IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_EXTERNAL){
        s = triggerConfigDAQ(iviDigitizer_vi, IVIFGEN_VAL_TRIGGER_SOURCE_PXI_STAR_TRIG);
        s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_TRIGGER_SOURCE_P_PXI_STAR, triggerSource);
        s = internalTriggerConfigSAT(iviSyncATrig_vi, triggerSource);
    }
    else
        std::cout << "\n=== Trig Source Error ===" << std::endl;

    std::cout << "\n=== DDC Config ===" << std::endl;
    s = DDConfigDAQ(iviDigitizer_vi, 0, "-1", 0);

    std::cout << "\n=== Extract Multiple Config ===" << std::endl;
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_ADC_EXTRACT_MULTIPLE, 1);

    std::cout << "\n=== Sample Rate Config ===" << std::endl;
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_ADC_SAMPLE_RATE, 4000);


    std::cout << "\n=== SYNC Config ===" << std::endl;
    std::list<iviFgen_ViSession *> iviFgen_vi_list;
    std::list<iviDigitizer_ViSession *> iviDigitizer_vi_list;
    iviDigitizer_vi_list.push_back(iviDigitizer_vi);
    s = IviSUATools_Sync(iviSUATools_vi, iviSyncATrig_vi, iviFgen_vi_list, iviDigitizer_vi_list);

    std::cout << "\n=== Sample Config ===" << std::endl;

    ViConstString sampleEnableChannel = "-1";
    ViInt32 sampleEnable = 1;
    ViConstString sampleStrageDepthChannel = "-1";
    ViUInt32 sampleStrageDepth = 65536;
    ViConstString sampleLenPreChannel = "-1";
    ViUInt32 sampleLenPre = 32768;
    ViConstString sampleTimesChannel = "-1";
    ViUInt32 sampleTimes = 4294967295;
    ViConstString sampleLogicalExtractionMultipleChannel = "-1";
    ViUInt32 sampleLogicalExtractionMultiple = 1;
    ViConstString sampleCollectDataTruncationChannel = "-1";
    ViUInt32 sampleCollectDataTruncation = 0;
    ViConstString sampleCollectDataTypeChannel = "-1";
    ViUInt32 sampleCollectDataType = 0;

    sampleConfigDAQ(iviDigitizer_vi,
                    sampleEnableChannel, sampleEnable,
                    sampleStrageDepthChannel, sampleStrageDepth,
                    sampleLenPreChannel, sampleLenPre,
                    sampleTimesChannel, sampleTimes,
                    sampleLogicalExtractionMultipleChannel, sampleLogicalExtractionMultiple,
                    sampleCollectDataTruncationChannel, sampleCollectDataTruncation,
                    sampleCollectDataTypeChannel, sampleCollectDataType);


    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y%m%d%H%M%S");
    std::string directoryPath = "./DAQ-14Bit-" + oss.str();
    if (!std::filesystem::exists(directoryPath)) {
        std::filesystem::create_directories(directoryPath);
    }

    std::unordered_map<int, Deque> chnl_Deque;
    std::unordered_map<int, std::ofstream> chnl_outF;
    std::unordered_map<int, std::mutex> chnl_mutex;
    std::unordered_map<int, std::thread> chnl_up_trd;
    std::unordered_map<int, std::thread> chnl_write_trd;

    std::cout << "\n=== Stream Start Get the Data ===" << std::endl;
    s = IviSUATools_RunDigitizer(iviSUATools_vi, iviSyncATrig_vi, iviDigitizer_vi);

    ViInt32 numChannel;
    std::vector<std::string> channels = split(channelName, ',');
    for (const auto &channel: channels) {

        string2int(channel.data(), numChannel, 10)

        if (chnl_Deque.find(numChannel) == chnl_Deque.end()) {
            for (int i=0; i<10; i++) {
                auto mem = reinterpret_cast<nsuMemory_p>(IviDigitizer_CreateMemInt16(iviDigitizer_vi, waveformArraySize));
                chnl_Deque[numChannel].empty.Push(mem);
            }
        }
        chnl_mutex[numChannel];
        chnl_up_trd.emplace(numChannel, std::thread(upload_thread, iviDigitizer_vi, &chnl_Deque[numChannel], channel, waveformArraySize, times, &chnl_mutex[numChannel]));
        chnl_write_trd.emplace(numChannel, std::thread(write_file_thread, iviDigitizer_vi, &chnl_Deque[numChannel], channel, waveformArraySize, directoryPath +"/up-res-14Bit_", &chnl_mutex[numChannel]));
    }

    for (const auto &channel: channels) {

        string2int(channel.data(), numChannel, 10)

        chnl_up_trd[numChannel].join();
        chnl_write_trd[numChannel].join();
    }

    std::cout << "\n=== Stream Stop Get the Data ===" << std::endl;
    s = IviSUATools_StopDigitizer(iviSUATools_vi, iviSyncATrig_vi, iviDigitizer_vi);

    for (const auto &channel: channels) {

        string2int(channel.data(), numChannel, 10)

        for (int i=0; i<10; i++) {
            auto mem = reinterpret_cast<iviDigitizer_memData*>(chnl_Deque[numChannel].empty.Pop());
            s = IviDigitizer_ClearMem(iviDigitizer_vi, mem);
        }
    }

    isFAIL(IviDigitizer_Close(iviDigitizer_vi));
    isFAIL(IviSyncATrig_Close(iviSyncATrig_vi));
    isFAIL(IviSUATools_Close(iviSUATools_vi));
    delete iviDigitizer_vi;
    delete iviSyncATrig_vi;
    delete iviSUATools_vi;

    std::cout << "All threads have completed.\n";
    return 0;
}


