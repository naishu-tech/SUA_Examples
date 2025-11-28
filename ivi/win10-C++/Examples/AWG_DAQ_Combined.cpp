//
// Created by combining AWG_wfmPlay.cpp and DAQ_read_RingBuffer.cpp
// This file combines AWG waveform playback with DAQ ring buffer data acquisition
//

#include "IviSUATools.h"
#include "IviFgen.h"
#include "IviDigitizer.h"
#include "IviBase.h"
#include "tool_config.h"

#include <fstream>
#include <cstring>
#include <thread>
#include <chrono>
#include <list>
#include <vector>
#include <map>
#include <exception>
#include <queue>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <unordered_map>
#include <algorithm>
#include <cmath>

// Forward declaration
struct SystemConfig;

// Thread-safe queue for data transfer
template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    void Push(T *value) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(value);
        lock.unlock();
        condition_.notify_one();
    }

    T * Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !queue_.empty(); });
        auto value = (T *)queue_.front();
        queue_.pop();
        return value;
    }

    bool IsEmpty() {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t Size() {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
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
};

// Configuration structure to hold all variables
struct SystemConfig {
    // System configuration
    std::string resource_db_path = "./resourceDB.json";
    std::string logicalName = "PXI::0::INSTR";

    // LSDADC configuration
    ViUInt32 lsdadc_chnl_type = 0;
    ViUInt32 lsdadc_sample_rate = 4000;
    ViUInt32 lsdadc_output_gear = 500;
    ViUInt32 lsdadc_voltage = 0;

    // Trigger configuration
    ViUInt32 triggerPeriod = 40000000;
    ViUInt32 triggerRepetSize = 4294967295;
    ViUInt32 triggerPulseWidth = 20000000;
    ViUInt32 triggerDelay = 0;
    ViUInt32 triggerEdgeType = 0;
    ViUInt32 triggerHoldOffTime = 0;
    std::string triggerEdgechannelName = "-1";
    ViInt32 triggerEdgeSet = 32768;
    
    // AWG configuration
    std::string AWG_channelName = "0,1";
    std::string nswave_code = R"(
@nw.kernel
def program(wlist: dict[str, np.ndarray]):
    seg1: nw.ArbWave = nw.init_arbwave(wlist, 'seg1')
    while True:
        nw.play_arb(seg1)
    return nw.Kernel()
)";
    std::list<ViString> wfmPath_list = {
        "./wfm/Sin_Fixed_4000MSps_0Offset_1Amp_100MHz_0MBw_0Phase_1024us_16bit_Signed_1024us_1Row_1Column.dat"
    };
    ViInt32 wfmSize = 8192000;
    // AWG Work times
    ViUInt32 workTimes = 10;
    // AWG Sample Rate
    ViReal64 AWG_sampleRate = 4000000000;

    // DAQ configuration parameters
    ViUInt32 triggerSource = IVIFGEN_VAL_TRIGGER_SOURCE_INTERNAL;
    std::string DAQ_channelName = "0,1";
    ViUInt32 times = 20;
    ViUInt32 syncTriggerChannel = 0xFF;
    ViInt32 triggerEdgetype = 0x01;
    ViInt32 chTriggerEdgetype = 0x01;
    // DAQ sample configuration
    std::string sampleEnableChannel = "-1";
    ViInt32 sampleEnable = 1;
    std::string sampleStrageDepthChannel = "-1";
    ViUInt32 sampleStrageDepth = 65536;
    std::string sampleLenPreChannel = "-1";
    ViUInt32 sampleLenPre = 32768;
    std::string sampleTimesChannel = "-1";
    ViUInt32 sampleTimes = 4294967295;
    std::string sampleLogicalExtractionMultipleChannel = "-1";
    ViUInt32 sampleLogicalExtractionMultiple = 1;
    std::string sampleCollectDataTruncationChannel = "-1";
    ViUInt32 sampleCollectDataTruncation = 0;
    std::string sampleCollectDataTypeChannel = "-1";
    ViUInt32 sampleCollectDataType = 0;
    // DAQ Save file Path
    std::string directoryPath = "./";
    ViUInt32 head_len = 16;
    // DAQ Sample Rate
    ViReal64 DAQ_sampleRate = 2000000000;
};

// Pre-RF configuration for AWG
ViStatus RF(iviBase_ViSession* iviBase_vi, const SystemConfig& config){
    ViStatus error = VI_STATE_SUCCESS;

    std::cout << "\n=== LSDADC Set ===" << std::endl;
    error = IviBase_SetAttributeViUInt32(iviBase_vi, "0", IVIBASE_ATTR_LSDADC_SET_CHNL_TYPE, config.lsdadc_chnl_type);
    error = IviBase_SetAttributeViUInt32(iviBase_vi, "0", IVIBASE_ATTR_LSDADC_SET_SAMPLE_RATE, config.lsdadc_sample_rate);
    error = IviBase_SetAttributeViUInt32(iviBase_vi, "0", IVIBASE_ATTR_LSDADC_SET_OUTPUT_GEAR, config.lsdadc_output_gear);
    error = IviBase_SetAttributeViUInt32(iviBase_vi, "0", IVIBASE_ATTR_LSDADC_SET_VOLTAGE, config.lsdadc_voltage);
    error = IviBase_SetAttributeViUInt32(iviBase_vi, "0", IVIBASE_ATTR_LSDADC_SET_EXE, 0);

    std::cout << "\n=== RF Config ===" << std::endl;
    error = IviBase_SetAttributeViUInt32(iviBase_vi, "0", IVIBASE_ATTR_OFFLINE_WORK, 1);
    error = IviBase_SetAttributeViUInt32(iviBase_vi, "0", IVIBASE_ATTR_RF_CONFIG, 0);

    return error;
}

// Function to read and plot binary data file


// Write file thread for DAQ data
void write_file_thread(iviDigitizer_ViSession *vi, Deque *q, const std::string& channelName, ViUInt32 waveformArraySize, const std::string &path, std::mutex *mu) {
    std::unique_lock<std::mutex> *lock;
    iviDigitizer_memData* mem;
    std::ofstream outF;
    int cnt = 0;
    std::string file_path = path+ channelName + "_" + std::to_string(cnt) + ".data";
    nsuSize_t speed_count = 0;
    auto st = std::chrono::steady_clock::now();
    while (true) {
        lock = new std::unique_lock<std::mutex>(*mu);
        if (q->stopFlag == 1 && q->full.IsEmpty()) {
            delete lock;
            break;
        }
        delete lock;
        if (speed_count % 2147483648 == 0){
            if (outF.is_open()) {
                outF.close();
                cnt++;
            }
            file_path = path + channelName + "_" + std::to_string(cnt) + ".data";
            outF.open(file_path, std::ofstream::binary);
        }

        mem = reinterpret_cast<iviDigitizer_memData *>(q->full.Pop());

        outF.write(mem->memDataHandle, (waveformArraySize * sizeof(ViInt16)));
        q->empty.Push(reinterpret_cast<nsuMemory_p>(mem));
        lock = new std::unique_lock<std::mutex>(*mu);
        delete lock;
        speed_count += waveformArraySize;
    }
    std::cout << std::endl;
    outF.close();
}

// Upload thread for DAQ data
void upload_thread(iviDigitizer_ViSession *vi, Deque *q, const std::string& channelName, ViUInt32 waveformArraySize, ViUInt32 times, std::mutex *mu) {
    std::unique_lock<std::mutex> *lock;
    iviDigitizer_memData* mem;
    std::cout << "upload thread start: " << waveformArraySize << std::endl;
    ViStatus s;
    std::vector<int> channelsInt;
    int dictionary = 0;
    int cnt = 0;
    nsuSize_t speed_count = 0;
    ViInt32 * m[30];

    auto st = std::chrono::steady_clock::now();

    while (true) {
        if (dictionary > times) {
            break;
        }
        cnt++;
        mem = reinterpret_cast<iviDigitizer_memData*>(q->empty.Pop());

        s = IviDigitizer_ReadWaveformInt16(vi, channelName.c_str(), waveformArraySize, mem, 0.1);//100ms timeout
        if (s != VI_STATE_SUCCESS){
            q->empty.Push(reinterpret_cast<nsuMemory_p>(mem));
            continue;
        }
        dictionary += 1;
//        std::cout << "[Upload] Channel " << channelName << " completed successfully, dictionary=" << dictionary << std::endl;
        speed_count += waveformArraySize;

        if (cnt % 20 == 0) {
            auto count = std::chrono::steady_clock::now() - st;
            std::cout << std::flush << '\r' << "current write file speed: " << speed_count*1000./(count.count()) << "MB/s" << std::endl;
            speed_count = 0;
            st = std::chrono::steady_clock::now();
        }
        q->full.Push(reinterpret_cast<nsuMemory_p>(mem));
    }
    lock = new std::unique_lock<std::mutex>(*mu);
    q->stopFlag = 1;
    delete lock;
}

int main(int argc, char *argv[]){

#ifdef _WIN32
    std::cout << "=== Configuring Python Paths ===" << std::endl;
    configure_python_paths("C:/Users/ll/.conda/envs/JupyterServer");
    std::cout << "Python paths configured" << std::endl;
#endif

    // Initialize configuration structure
    SystemConfig config;
    ViStatus s = VI_STATE_SUCCESS;
    std::map<ViString, waveformHandle *> waveformHandle_map;
    // Session pointers
    iviSUATools_ViSession* iviSUATools_vi = nullptr;
    iviFgen_ViSession* iviFgen_vi = nullptr;
    iviDigitizer_ViSession* iviDigitizer_vi = nullptr;
    // Data buffers
    ViChar* buffer = nullptr;
    ViInt16* bufferWfm = nullptr;
    // DAQ thread management
    std::unordered_map<int, Deque> chnl_Deque;
    std::unordered_map<int, std::ofstream> chnl_outF;
    std::unordered_map<int, std::mutex> chnl_mutex;
    std::unordered_map<int, std::thread> chnl_up_trd;
    std::unordered_map<int, std::thread> chnl_write_trd;
    // Helper vectors
    std::vector<std::string> AWG_channels;
    std::vector<std::string> DAQ_channels;

    std::cout << "\n=== Initialize IviSUATools ===" << std::endl;
    iviSUATools_vi = new iviSUATools_ViSession;
    s = IviSUATools_Initialize(iviSUATools_vi);
    std::cout << "IviSUATools initialized successfully" << std::endl;

    std::cout << "\n=== Initialize IviFgen ===" << std::endl;
    iviFgen_vi = new iviFgen_ViSession;
    s = IviFgen_Initialize(config.logicalName, VI_STATE_FALSE, VI_STATE_TRUE, iviFgen_vi, config.resource_db_path);
    std::cout << "IviFgen initialized successfully" << std::endl;

    std::cout << "\n=== Initialize IviDigitizer ===" << std::endl;
    iviDigitizer_vi = new iviDigitizer_ViSession;
    s = IviDigitizer_Initialize(config.logicalName, VI_STATE_FALSE, VI_STATE_TRUE, iviDigitizer_vi, config.resource_db_path);
    std::cout << "IviDigitizer initialized successfully" << std::endl;

    // Check if digitizer is in RingBuffer mode
    ViInt32 mode;
    s = IviDigitizer_GetAttributeViInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_SAMPLE_WORK_MODE, &mode);
    if (mode != IVIDIGITIZER_VAL_WORK_MODE_RING_BUFFER) {
        std::cout << "Error: The Digitizer Work Mode is not RingBuffer!" << std::endl;
        isFAIL(IviDigitizer_Close(iviDigitizer_vi));
        isFAIL(IviFgen_Close(iviFgen_vi));
        isFAIL(IviSUATools_Close(iviSUATools_vi));
        delete iviDigitizer_vi;
        delete iviFgen_vi;
        delete iviSUATools_vi;
        return 0;
    }

    std::cout << "\n=== Sample Rate Config ===" << std::endl;
    s = IviFgen_SetAttributeViReal64(iviFgen_vi, "0", IVIFGEN_ATTR_DAC_SAMPLE_RATE, config.AWG_sampleRate);

    std::cout << "\n=== Sample Rate Config ===" << std::endl;
    s = IviDigitizer_SetAttributeViReal64(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_ADC_SAMPLE_RATE, config.DAQ_sampleRate);

    // ========== RF Configuration ==========
    s = RF(iviFgen_vi->vi, config);

    // Set trigger mode for DAQ
    ViUInt32 triggerSourcemask = config.syncTriggerChannel;
    // Level channel trigger setting
    for (int i = 0; i < 8; i++) {
        bool b = ((1 << i) & triggerSourcemask) ? true : false;
        ViUInt32 resUInt32 = 0;
        std::string channelStr = std::to_string(i);
        s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, channelStr.c_str(), IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET_ENABLE, b?1:0);
        s = IviDigitizer_GetAttributeViUInt32(iviDigitizer_vi, channelStr.c_str(), IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET_ENABLE, &resUInt32);
        std::cout << "===Level channel:" << i << " trigger setting:0x" << std::hex << resUInt32 << std::dec<< std::endl;
    }

    // ========== Trigger Configuration ==========
    std::cout << "\n=== Trig Config ===" << std::endl;

    s = triggerConfigAWG(iviFgen_vi, config.triggerSource);
    s = internalTriggerConfigAWG(iviFgen_vi, config.triggerPulseWidth, config.triggerRepetSize, config.triggerPeriod, 0);
    s = triggerConfigDAQ(iviDigitizer_vi, config.triggerSource);
    s = internaltriggerConfigDAQ(iviDigitizer_vi, config.triggerPulseWidth, config.triggerRepetSize, config.triggerPeriod,
                                  config.triggerDelay, config.triggerEdgeType, config.triggerHoldOffTime, 
                                  config.triggerEdgechannelName.c_str(), config.triggerEdgeSet);

    // ========== AWG Channel Enable ==========
    std::cout << "\n=== AWG Chnl EN ===" << std::endl;
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "-1", IVIFGEN_ATTR_SYSTEM_STATUS_ENABLE, 1);

    // ========== AWG Waveform Creation ==========
    std::cout << "\n=== Create NSWave ===" << std::endl;

    std::cout << "NSWave Code Preview:" << std::endl;
    std::cout << config.nswave_code << std::endl;

    std::cout << "\n=== Read Waveform ===" << std::endl;

    buffer = new ViChar[config.wfmSize / sizeof(ViChar)];
    bufferWfm = new ViInt16 [config.wfmSize / sizeof(ViInt16)];

    for (auto& wfmPath : config.wfmPath_list) {
        std::ifstream fileWfm(wfmPath, std::ios::binary);
        if (!fileWfm) {
            std::cerr << "Open " << wfmPath <<" File Failed!" << std::endl;
            return 1;
        }

        fileWfm.seekg(0, std::ios::end);
        std::streamsize sizeWfm = fileWfm.tellg();
        std::cout << "The stream file size is " << sizeWfm << " bytes" << std::endl;
        fileWfm.seekg(0, std::ios::beg);

        if (fileWfm.read(buffer, config.wfmSize)) {
            std::cout << "Successfully read data in "<< config.wfmSize <<" Byte" << std::endl;
        } else {
            std::cerr << "Failed to read file" << std::endl;
            return 1;
        }
        fileWfm.close();

        memcpy(bufferWfm, buffer, config.wfmSize);
        auto *wfmHandle = new waveformHandle;
        std::cout << "Create Arb Waveform: " << (config.wfmSize / sizeof(ViInt16)) << std::endl;
        isFAIL(IviFgen_CreateArbWaveformViInt16(iviFgen_vi, config.wfmSize / sizeof(ViInt16), bufferWfm, wfmHandle));

        waveformHandle_map["seg1"] = wfmHandle;
    }

    std::cout << "\n=== Download NSWave & Waveform ===" << std::endl;
    AWG_channels = split(config.AWG_channelName, ',');
    for (const auto &channel: AWG_channels) {
        auto result = IviSUATools_NSWave(iviSUATools_vi, iviFgen_vi, channel.c_str(), waveformHandle_map, config.nswave_code);
    }

    for (const auto& pair : waveformHandle_map) {
        const ViString& key = pair.first;
        waveformHandle* wfmHandle = pair.second;
        isFAIL(IviFgen_ClearArbWaveform(iviFgen_vi, wfmHandle));
    }

    // ========== DAQ Sample Configuration ==========
    std::cout << "\n=== Sample Config ===" << std::endl;
    sampleConfigDAQ(iviDigitizer_vi,
                    config.sampleEnableChannel.c_str(), config.sampleEnable,
                    config.sampleStrageDepthChannel.c_str(), config.sampleStrageDepth,
                    config.sampleLenPreChannel.c_str(), config.sampleLenPre,
                    config.sampleTimesChannel.c_str(), config.sampleTimes,
                    config.sampleLogicalExtractionMultipleChannel.c_str(), config.sampleLogicalExtractionMultiple,
                    config.sampleCollectDataTruncationChannel.c_str(), config.sampleCollectDataTruncation,
                    config.sampleCollectDataTypeChannel.c_str(), config.sampleCollectDataType);

    // Create directory for DAQ data
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y%m%d%H%M%S");
    std::string directoryPath = config.directoryPath + "/DAQ-14Bit-" + oss.str();
    if (!std::filesystem::exists(directoryPath)) {
        std::filesystem::create_directories(directoryPath);
    }

    // ========== DAQ Channel Configuration ==========
    std::cout << "\n=== DAQ Channel Config ===" << std::endl;
    ViUInt32 waveformArraySize = config.sampleStrageDepth + config.head_len;

    ViInt32 numChannel;
    DAQ_channels = split(config.DAQ_channelName, ',');
    for (const auto &channel: DAQ_channels) {
        string2int(channel.data(), numChannel, 10)

        if (chnl_Deque.find(numChannel) == chnl_Deque.end()) {
            s = IviDigitizer_ConfigureChannelDataDepthInt16(iviDigitizer_vi, channel, waveformArraySize);
            for (int i=0; i<10; i++) {
                auto mem = reinterpret_cast<nsuMemory_p>(IviDigitizer_CreateMemInt16(iviDigitizer_vi, waveformArraySize));
                chnl_Deque[numChannel].empty.Push(mem);
            }
        }
        chnl_mutex[numChannel];
        chnl_up_trd.emplace(numChannel, std::thread(upload_thread, iviDigitizer_vi, &chnl_Deque[numChannel], channel, waveformArraySize, config.times, &chnl_mutex[numChannel]));
        chnl_write_trd.emplace(numChannel, std::thread(write_file_thread, iviDigitizer_vi, &chnl_Deque[numChannel], channel, waveformArraySize, directoryPath +"/up-res-14Bit_", &chnl_mutex[numChannel]));
    }

    // ========== Start AWG and DAQ ==========
    std::cout << "\n=== Start AWG Waveform Playback and DAQ Data Acquisition ===" << std::endl;
    s = IviFgen_InitiateGeneration(iviFgen_vi);
    s = IviDigitizer_InitiateAcquisition(iviDigitizer_vi);

    std::cout << "\n=== wait " << config.workTimes << "S ===" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(config.workTimes));

    // Wait for DAQ threads to complete
    for (const auto &channel: DAQ_channels) {
        string2int(channel.data(), numChannel, 10)
        chnl_up_trd[numChannel].join();
        chnl_write_trd[numChannel].join();
    }

    // ========== Stop AWG and DAQ ==========
    std::cout << "\n=== Stop AWG and DAQ ===" << std::endl;
    s = IviFgen_AbortGeneration(iviFgen_vi);
    s = IviDigitizer_Abort(iviDigitizer_vi);

    // Clean up DAQ memory
    for (const auto &channel: DAQ_channels) {
        string2int(channel.data(), numChannel, 10)
        for (int i=0; i<10; i++) {
            auto mem = reinterpret_cast<iviDigitizer_memData*>(chnl_Deque[numChannel].empty.Pop());
            s = IviDigitizer_ClearMem(iviDigitizer_vi, mem);
        }
    }

    // Clean up waveform buffer
    delete[] buffer;
    delete[] bufferWfm;

    // ========== Close Sessions ==========
    std::cout << "\n=== Test Completed ===" << std::endl;
    isFAIL(IviFgen_Close(iviFgen_vi));
    isFAIL(IviDigitizer_Close(iviDigitizer_vi));
    isFAIL(IviSUATools_Close(iviSUATools_vi));
    delete iviFgen_vi;
    delete iviDigitizer_vi;
    delete iviSUATools_vi;

    PlotDataFile(directoryPath +"/up-res-14Bit_0_0.data");

    return 0;
}

