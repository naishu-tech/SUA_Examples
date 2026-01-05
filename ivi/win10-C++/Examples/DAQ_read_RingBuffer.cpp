//
// Created by sn06129 on 2025/7/25.
//

# include "IviSUATools.h"
# include "IviDigitizer.h"
# include "IviBase.h"
# include "tool_config.h"

#define string2int(channelName, numChannel, m){     \
    char *end;                                      \
    numChannel = std::strtol(channelName, &end, m);}\

ViStatus preDAQ(iviDigitizer_ViSession* iviDigitizer_vi){
    ViStatus error = VI_STATE_SUCCESS;
    ViReal64 DAC_sample_rate;
    error = IviDigitizer_GetAttributeViReal64(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_ADC_SAMPLE_RATE, &DAC_sample_rate);
    error = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_LSDADC_SET_CHNL_TYPE, 0);
    error = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_LSDADC_SET_SAMPLE_RATE, int(DAC_sample_rate / 1e6 ));
    error = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_LSDADC_SET_OUTPUT_GEAR, 500);
    error = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_LSDADC_SET_VOLTAGE, 0);
    error = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_LSDADC_SET_EXE, 0);
    return error;
}

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

void write_file_thread(iviDigitizer_ViSession *vi, Deque *q, std::string channelName, ViUInt32 waveformArraySize, const std::string &path, std::mutex *mu) {
    std::unique_lock<std::mutex> *lock;
    iviDigitizer_memData* mem;
    std::ofstream outF;
//    outF.open(path + channelName + ".data", std::ofstream::binary);
    int cnt = 0;
    size_t speed_count = 0;
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

        outF.write(IviDigitizer_GetMemDataHandle(mem), (waveformArraySize * sizeof(ViInt16)));
        q->empty.Push(reinterpret_cast<ViMem>(mem));
        lock = new std::unique_lock<std::mutex>(*mu);
        q->dataNum -= 1;
        delete lock;
        speed_count += waveformArraySize;
    }
    std::cout << std::endl;
    outF.close();
}

void upload_thread(iviDigitizer_ViSession *vi, Deque *q, const std::string& channelName, ViUInt32 waveformArraySize, ViUInt32 times, std::mutex *mu) {
    std::unique_lock<std::mutex> *lock;
    iviDigitizer_memData* mem;
    std::cout << "upload thread start: " << waveformArraySize << std::endl;
    ViStatus s;
    std::vector<int> channelsInt;
    int dictionary = 0;
    int cnt = 0;
    size_t speed_count = 0;
    ViInt32 * m[30];

    auto st = std::chrono::steady_clock::now();

    while (true) {
        if (dictionary > times) {
            break;
        }

        mem = reinterpret_cast<iviDigitizer_memData*>(q->empty.Pop());
        // std::cout << "[Upload] Channel " << channelName << " starting ReadWaveformInt16 call, dictionary=" << dictionary << std::endl;

        s = IviDigitizer_ReadWaveformInt16(vi, channelName.c_str(), waveformArraySize, mem, 0.1);//100ms timeout

        // std::cout << "[Upload] Channel " << channelName << " ReadWaveformInt16 returned, status=" << s << ", dictionary=" << dictionary << std::endl;

        if (s != VI_STATE_SUCCESS){
            // std::cout << "[Upload] Channel " << channelName << " failed, putting back into queue, dictionary=" << dictionary << std::endl;
            q->empty.Push(reinterpret_cast<ViMem>(mem));
            continue;
        }


        dictionary += 1;
        std::cout << "[Upload] Channel " << channelName << " completed successfully, dictionary=" << dictionary << std::endl;
        speed_count += waveformArraySize;

        if (cnt % 20 == 0) {
            auto count = std::chrono::steady_clock::now() - st;
            //std::cout << std::flush << '\r' << "current write file speed: " << speed_count*1000./(count.count()) << "MB/s" << std::endl;
            speed_count = 0;
            st = std::chrono::steady_clock::now();
        }
        cnt++;
        q->full.Push(reinterpret_cast<ViMem>(mem));
        lock = new std::unique_lock<std::mutex>(*mu);
        q->dataNum += 1;
        delete lock;
        //std::cout << "channel: " << channelName << " upData successful!!!!!!!!!!!!" << std::endl;
    }
    lock = new std::unique_lock<std::mutex>(*mu);
    q->stopFlag = 1;
    delete lock;
}

int main(int argc, char *argv[]){

    ViUInt32 res = 0;
    ViStatus s = VI_STATE_SUCCESS;

    std::string resource_db_path ="C:/Program Files/IVI Foundation/IVI/RIGOL/file/resourceDB.json";
    std::string logicalName = "PXI::0::INSTR";

    std::cout << "\n=== Initialize IviSUATools ===" << std::endl;
    auto iviSUATools_vi = new iviSUATools_ViSession;
    s = IviSUATools_Initialize(iviSUATools_vi, logicalName, VI_STATE_FALSE, VI_STATE_TRUE, resource_db_path);
    std::cout << "IviSUATools initialized successfully" << std::endl;

    std::cout << "\n=== Initialize IviDigitizer ===" << std::endl;
    auto iviDigitizer_vi = new iviDigitizer_ViSession;
    s = IviDigitizer_Initialize(logicalName, VI_STATE_FALSE, VI_STATE_TRUE, iviDigitizer_vi, resource_db_path);
    std::cout << "IviDigitizer initialized successfully" << std::endl;

    ViUInt32 triggerSource = IVIDIGITIZER_VAL_TRIGGER_SOURCE_PXI_STAR_TRIG|IVIDIGITIZER_VAL_TRIGGER_SOURCE_PXI_SYNC; // Default trigger source
    std::string channelName = "0,1";                                        // Default channels
    ViUInt32 times = 20;                                                     // Default number of data packets
    ViUInt32 syncTriggerChannel = 0xFF;                                      // Default sync trigger channel setting
    ViInt32 triggerEdgetype = 0x01;                                          // Default trigger edge type
    ViInt32 chTriggerEdgetype = 0x01;                                        // Default channel trigger edge type


    ViInt32 mode;                                                               // Declare work mode variable
    s = IviDigitizer_GetAttributeViInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_SAMPLE_WORK_MODE, &mode);  // Get the sampling work mode of the digitizer
    if (mode != IVIDIGITIZER_VAL_WORK_MODE_RING_BUFFER) {                      // Check if the work mode is ring buffer mode
        std::cout << "Error: The Digitizer Work Mode is not RingBuffer!" << std::endl;  // Output work mode error message
        isFAIL(IviDigitizer_Close(iviDigitizer_vi));                           // Close digitizer session
        isFAIL(IviSUATools_Close(iviSUATools_vi));                             // Close SUA tools session
        delete iviDigitizer_vi;                                                 // Delete digitizer object
        delete iviSUATools_vi;                                                  // Delete SUA tools object
        return 0;                                                               // Exit program and return 0
    }

    std::cout << "\n=== LSDADC Config ===" << std::endl;                       // Output LSDADC configuration title
    s = preDAQ(iviDigitizer_vi);                                               // Call pre-DAQ configuration function

    std::cout << "\n=== RF Config ===" << std::endl;                           // Output RF configuration title
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_OFFLINE_WORK, 1);  // Set digitizer offline work mode to enabled
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_RF_CONFIG, 0);     // Set RF configuration parameter to 0

    // Set trigger mode
    ViUInt32 triggerSourcemask = syncTriggerChannel; // Use command line parameter to set sync trigger channel
    // Level channel trigger setting
    for (int i = 0; i < 8; i++) {
        bool b = ((1 << i) & triggerSourcemask) ? true : false;
        ViUInt32 resUInt32 = 0;
        std::string channelStr = std::to_string(i);
        s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, channelStr.c_str(), IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET_ENABLE, b?1:0);
        s = IviDigitizer_GetAttributeViUInt32(iviDigitizer_vi, channelStr.c_str(), IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET_ENABLE, &resUInt32);
        std::cout << "===Level channel:" << i << " trigger setting:0x" << std::hex << resUInt32 << std::dec<< std::endl;
    }


    IviDigitizer_SetAttributeViInt32(iviDigitizer_vi, "0",IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_TYPE , triggerEdgetype);
    IviDigitizer_GetAttributeViInt32(iviDigitizer_vi, "0",IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_TYPE , &triggerEdgetype);
    std::cout << "===Internal trigger channel trigger edge setting:0x" << std::hex << triggerEdgetype << std::dec << std::endl;

    for (int i = 0; i < 8; i++) {
        ViInt32 resInt32 = 0;
        std::string channelStr = std::to_string(i);
        s = IviDigitizer_SetAttributeViInt32(iviDigitizer_vi, channelStr.c_str(), IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET,chTriggerEdgetype);
        s = IviDigitizer_GetAttributeViInt32(iviDigitizer_vi, channelStr.c_str(), IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET, &resInt32);
        std::cout << "===Internal trigger channel:" << i << " internal trigger channel trigger edge setting:0x" << std::hex << resInt32 << std::dec << std::endl;
    }

    std::cout << "\n=== Trig Config ===" << std::endl;                         // Output trigger configuration title
    ViUInt32 satTriggerSource = IVIFGEN_VAL_TRIGGER_SOURCE_INTERNAL; // Set SAT trigger source to internal PXI_STAR trigger
    ViUInt32 triggerPeriod = 40000000;                                         // Set trigger period to 40M clock cycles (must be divisible by 800)
    ViUInt32 triggerRepetSize = 4294967295;                                    // Set trigger repeat count to maximum value
    ViUInt32 triggerPulseWidth = 20000000;                                     // Set trigger pulse width to 20M clock cycles (half of the period)
    ViUInt32 triggerDelay = 0;
    ViUInt32 triggerEdgeType = 0;
    ViUInt32 triggerHoldOffTime = 0;
    ViConstString triggerEdgechannelName = "-1";
    ViInt32 triggerEdgeSet = 32768;
    s = triggerConfigDAQ(iviDigitizer_vi, IVIFGEN_VAL_TRIGGER_SOURCE_INTERNAL);
    s = internaltriggerConfigDAQ(iviDigitizer_vi, triggerPulseWidth, triggerRepetSize, triggerPeriod, triggerDelay, triggerEdgeType, triggerHoldOffTime, triggerEdgechannelName, triggerEdgeSet);

    // Print trigger mode
    ViUInt32 get_triggerSource = 0;
    IviDigitizer_GetAttributeViUInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_TRIGGER_SOURCE, &get_triggerSource);
    if((get_triggerSource & IVIDIGITIZER_VAL_TRIGGER_SOURCE_PXI_SYNC)!=0){
        std::cout << "\n=== Set to synchronous trigger mode ===:0x" << std::hex << get_triggerSource << std::dec << std::endl;
    }
    else{
        std::cout << "\n=== Set to asynchronous trigger mode ===:0x" << std::hex << get_triggerSource << std::dec << std::endl;
    }

    std::cout << "\n=== Sample Config ===" << std::endl;                       // Output sample configuration title
    ViConstString sampleEnableChannel = "-1";                                  // Sample enable channel "-1" (all channels)
    ViInt32 sampleEnable = 1;                                                  // Sample enable flag 1 (enabled)
    ViConstString sampleStrageDepthChannel = "-1";                            // Sample storage depth channel "-1" (all channels)
    ViUInt32 sampleStrageDepth = 65536;                                        // Sample storage depth 65536 points
    ViConstString sampleLenPreChannel = "-1";                                 // Sample pre-trigger length channel "-1" (all channels)
    ViUInt32 sampleLenPre = 32768;                                             // Sample pre-trigger length 32768 points
    ViConstString sampleTimesChannel = "-1";                                  // Sample times channel "-1" (all channels)
    ViUInt32 sampleTimes = 4294967295;                                         // Sample times maximum value (continuous sampling)
    ViConstString sampleLogicalExtractionMultipleChannel = "-1";              // Logical extraction multiple channel "-1" (all channels)
    ViUInt32 sampleLogicalExtractionMultiple = 1;                             // Logical extraction multiple 1
    ViConstString sampleCollectDataTruncationChannel = "-1";                  // Data collection truncation channel "-1" (all channels)
    ViUInt32 sampleCollectDataTruncation = 0;                                  // Data collection truncation 0 (no truncation)
    ViConstString sampleCollectDataTypeChannel = "-1";                        // Data collection type channel "-1" (all channels)
    ViUInt32 sampleCollectDataType = 0;                                        // Data collection type 0
    sampleConfigDAQ(iviDigitizer_vi,                                          // Call sample configuration function:
                    sampleEnableChannel, sampleEnable,                                    // Sample enable channel and flag
                    sampleStrageDepthChannel, sampleStrageDepth,                         // Storage depth channel and value
                    sampleLenPreChannel, sampleLenPre,                                   // Pre-trigger length channel and value
                    sampleTimesChannel, sampleTimes,                                     // Sample times channel and value
                    sampleLogicalExtractionMultipleChannel, sampleLogicalExtractionMultiple,  // Logical extraction multiple channel and value
                    sampleCollectDataTruncationChannel, sampleCollectDataTruncation,     // Data truncation channel and value
                    sampleCollectDataTypeChannel, sampleCollectDataType);               // Data type channel and value

    auto now = std::chrono::system_clock::now();                              // Get current system time
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);       // Convert system time to time_t type

    std::tm* localTime = std::localtime(&currentTime);                        // Convert to local time structure
    std::ostringstream oss;                                                    // String output stream object
    oss << std::put_time(localTime, "%Y%m%d%H%M%S");                          // Format time as YYYYMMDDHHMMSS

    std::string directoryPath = "./DAQ-14Bit-" + oss.str();                   // Data storage directory path with timestamp
    if (!std::filesystem::exists(directoryPath)) {                            // Check if directory exists
        std::filesystem::create_directories(directoryPath);                    // Create directory if it doesn't exist
    }

    std::unordered_map<int, Deque> chnl_Deque;                               // Channel queue mapping table
    std::unordered_map<int, std::ofstream> chnl_outF;                        // Channel file stream mapping table
    std::unordered_map<int, std::mutex> chnl_mutex;                          // Channel mutex mapping table
    std::unordered_map<int, std::thread> chnl_up_trd;                        // Channel upload thread mapping table
    std::unordered_map<int, std::thread> chnl_write_trd;                     // Channel write file thread mapping table

    std::cout << "\n=== Config ===" << std::endl;                            // Output configuration title
    ViUInt32 waveformArraySize = sampleStrageDepth + 16;                       // Waveform array size

    ViReal64 maximumTime_s = 0.1;                                             // Maximum wait time 0.1 seconds
    // times and channelName are now obtained from command line arguments

    ViInt32 numChannel;                                                        // Channel number variable
    std::vector<std::string> channels = split(channelName, ',');              // Split channel name string by comma
    for (const auto &channel: channels) {                                     // Iterate through each channel

        string2int(channel.data(), numChannel, 10)                            // Convert channel name string to integer

        if (chnl_Deque.find(numChannel) == chnl_Deque.end()) {               // If channel does not exist in queue
            s = IviDigitizer_ConfigureChannelDataDepthInt16(iviDigitizer_vi, channel, waveformArraySize);  // Configure channel data depth
            for (int i=0; i<10; i++) {
                auto mem = reinterpret_cast<ViMem>(IviDigitizer_CreateMemInt16(iviDigitizer_vi, waveformArraySize));  // Create 16-bit integer memory buffer
                chnl_Deque[numChannel].empty.Push(mem);                       // Push memory buffer into empty queue
            }
        }
        chnl_mutex[numChannel];                                               // Initialize channel mutex
        chnl_up_trd.emplace(numChannel, std::thread(upload_thread, iviDigitizer_vi, &chnl_Deque[numChannel], channel, waveformArraySize, times, &chnl_mutex[numChannel]));  // Create upload thread
        chnl_write_trd.emplace(numChannel, std::thread(write_file_thread, iviDigitizer_vi, &chnl_Deque[numChannel], channel, waveformArraySize, directoryPath +"/up-res-14Bit_", &chnl_mutex[numChannel]));  // Create write file thread
    }

    std::cout << "\n=== RingBuffer Start Get the Data ===" << std::endl;      // Start ring buffer data acquisition
    s = IviDigitizer_InitiateAcquisition(iviDigitizer_vi);

    for (const auto &channel: channels) {                                     // Iterate through each channel

        string2int(channel.data(), numChannel, 10)                            // Convert channel name string to integer

        chnl_up_trd[numChannel].join();                                       // Wait for upload thread to complete
        chnl_write_trd[numChannel].join();                                    // Wait for write file thread to complete
    }

    std::cout << "\n=== RingBuffer Stop Get the Data ===" << std::endl;       // Stop ring buffer data acquisition
    s = IviDigitizer_Abort(iviDigitizer_vi);

    for (const auto &channel: channels) {                                     // Iterate through each channel

        string2int(channel.data(), numChannel, 10)                            // Convert channel name string to integer

        for (int i=0; i<10; i++) {                                            // Loop 10 times to clean up memory buffers
            auto mem = reinterpret_cast<iviDigitizer_memData*>(chnl_Deque[numChannel].empty.Pop());  // Pop memory buffer from empty queue
            s = IviDigitizer_ClearMem(iviDigitizer_vi, mem);                  // Clear memory buffer
        }
    }
    isFAIL(IviDigitizer_Close(iviDigitizer_vi));                           // Close digitizer session
    isFAIL(IviSUATools_Close(iviSUATools_vi));                             // Close SUA tools session
    delete iviDigitizer_vi;                                                   // Delete digitizer session object
    delete iviSUATools_vi;                                                    // Delete SUA tools session object
    return 0;                                                                 // Program ends normally, return 0
}