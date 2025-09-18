//
// Created by sn06129 on 2025/7/25.
//

# include "IviSUATools.h"
# include "IviDigitizer.h"
# include "tool_config.h"

#define string2int(channelName, numChannel, m){     \
    char *end;                                      \
    numChannel = std::strtol(channelName, &end, m);}\

ViStatus preDAQ(iviDigitizer_ViSession* iviDigitizer_vi){
    ViStatus error = VI_STATE_SUCCESS;
//    error = IviDigitizer_SetAttributeViInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_TRIGGER_CONFIG_EXECUTE, 0);
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
        outF.write(mem->memDataHandle, (waveformArraySize * sizeof(ViInt16)));
        q->empty.Push(reinterpret_cast<nsuMemory_p>(mem));
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
    nsuSize_t speed_count = 0;
    ViInt32 * m[30];

    auto st = std::chrono::steady_clock::now();

    while (true) {
        if (dictionary > times) {
            break;
        }

        mem = reinterpret_cast<iviDigitizer_memData*>(q->empty.Pop());
        //std::cout << "channel: " << channelName << " upData start " << std::endl;
        s = IviDigitizer_ReadWaveformInt16(vi, channelName.c_str(), waveformArraySize, mem, 0);
        if (s != VI_STATE_SUCCESS){
            q->empty.Push(reinterpret_cast<nsuMemory_p>(mem));
            continue;
        }
        dictionary += 1;
        std::cout << "channel: " << channelName << " dictionary: " << dictionary << std::endl;
        speed_count += waveformArraySize;

        if (cnt % 20 == 0) {
            auto count = std::chrono::steady_clock::now() - st;
            //std::cout << std::flush << '\r' << "current write file speed: " << speed_count*1000./(count.count()) << "MB/s" << std::endl;
            speed_count = 0;
            st = std::chrono::steady_clock::now();
        }
        cnt++;
        q->full.Push(reinterpret_cast<nsuMemory_p>(mem));
        lock = new std::unique_lock<std::mutex>(*mu);
        q->dataNum += 1;
        delete lock;
        //std::cout << "channel: " << channelName << " upData successful!!!!!!!!!!!!" << std::endl;
    }
    lock = new std::unique_lock<std::mutex>(*mu);
    q->stopFlag = 1;
    delete lock;
}

int main(int argc, char *argv[]){                                                // 主函数入口，接收命令行参数
    ViUInt32 res = 0;                                                            // 初始化结果变量为0
    auto iviSUATools_vi = new iviSUATools_ViSession;                            // 创建SUA工具会话对象指针
    auto s = IviSUATools_Initialize(iviSUATools_vi);                            // 初始化SUA工具会话

    std::cout << "\n=== Creating Resource DB ===" << std::endl;                // 输出资源数据库创建标题
    std::string resource_db_path = IviSUATools_ScanOnlineBoards(iviSUATools_vi, "conf/config.yaml");  // 扫描在线板卡并创建资源数据库，返回数据库路径
    std::cout << "BVC Manager initialized successfully. Resource DB: " << resource_db_path << std::endl;  // 输出BVC管理器初始化成功信息和数据库路径

    auto iviDigitizer_vi = new iviDigitizer_ViSession;                          // 创建数字化仪会话对象指针
    s = IviDigitizer_Initialize("PXI::0::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviDigitizer_vi, resource_db_path);  // 初始化PXI插槽0的数字化仪设备

    ViInt32 mode;                                                               // 声明工作模式变量
    s = IviDigitizer_GetAttributeViInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_SAMPLE_WORK_MODE, &mode);  // 获取数字化仪的采样工作模式
    if (mode != IVIDIGITIZER_VAL_WORK_MODE_RING_BUFFER) {                      // 检查工作模式是否为环形缓冲区模式
        std::cout << "Error: The Digitizer Work Mode is not RingBuffer!" << std::endl;  // 输出工作模式错误信息
        isFAIL(IviDigitizer_Close(iviDigitizer_vi));                           // 关闭数字化仪会话
        isFAIL(IviSUATools_Close(iviSUATools_vi));                             // 关闭SUA工具会话
        delete iviDigitizer_vi;                                                 // 删除数字化仪对象
        delete iviSUATools_vi;                                                  // 删除SUA工具对象
        return 0;                                                               // 退出程序返回0
    }

    std::cout << "\n=== LSDADC Config ===" << std::endl;                       // 输出LSDADC配置标题
    s = preDAQ(iviDigitizer_vi);                                               // 调用预DAQ配置函数

    std::cout << "\n=== RF Config ===" << std::endl;                           // 输出射频配置标题
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_OFFLINE_WORK, 1);  // 设置数字化仪离线工作模式为启用
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_RF_CONFIG, 0);     // 设置射频配置参数为0

    auto iviSyncATrig_vi = new iviSyncATrig_ViSession;                         // 创建同步和触发会话对象指针
    s = IviSyncATrig_Initialize("PXI::1::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviSyncATrig_vi, resource_db_path);  // 初始化PXI插槽1的同步触发设备

    std::cout << "\n=== DDC Config ===" << std::endl;                          // 输出数字下变频配置标题
    s = DDConfigDAQ(iviDigitizer_vi, 0, "-1", 0);                             // 配置数字下变频参数

    std::cout << "\n=== Extract Multiple Config ===" << std::endl;             // 输出提取倍数配置标题
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_ADC_EXTRACT_MULTIPLE, 1);  // 设置ADC数据提取倍数为1

    std::cout << "\n=== Sample Rate Config ===" << std::endl;                  // 输出采样率配置标题
    s = IviDigitizer_SetAttributeViReal64(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_ADC_SAMPLE_RATE, 2000000000);  // 设置ADC采样率为2GHz

    std::cout << "\n=== SAT Clock Config ===" << std::endl;                    // 输出SAT时钟配置标题
    s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_CLOCK_SOURCE_100MHZ, IVISYNCATRIG_VAL_100MHZ_SOURCE_INTERNAL);  // 设置100MHz测试时钟源为内部时钟
    s = IviSyncATrig_SetAttributeViInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_CLOCK_SOURCE_100MHZ_EXE, 0);  // 执行100MHz时钟源设置
    s = IviSyncATrig_GetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_CLOCK_SOURCE_100MHZ_RESULT, &res);  // 获取100MHz时钟源设置结果
    std::cout << "IVISYNCATRIG_ATTR_TEST_CLOCK_SOURCE_100MHZ res is " << res << std::endl;  // 输出时钟源设置结果

    std::cout << "\n=== Digitizer Clock Config ===" << std::endl;              // 输出数字化仪时钟配置标题
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_REF_CLOCK_SOURCE, IVIBASE_VAL_REF_CLOCK_EXTERNAL);  // 设置参考时钟源为外部时钟
    s = IviDigitizer_SetAttributeViReal64(iviDigitizer_vi, "0", IVIBASE_ATTR_REF_FREQ_FREQUENCY, 100000000.0);  // 设置参考频率为100MHz

    std::cout << "\n=== SYNC Config ===" << std::endl;                         // 输出同步配置标题
    std::list<iviFgen_ViSession *> iviFgen_vi_list;                           // 创建函数发生器会话列表（空列表）
    std::list<iviDigitizer_ViSession *> iviDigitizer_vi_list;                 // 创建数字化仪会话列表
    iviDigitizer_vi_list.push_back(iviDigitizer_vi);                          // 将当前数字化仪会话添加到列表中
    s = IviSUATools_Sync(iviSUATools_vi, iviSyncATrig_vi, iviFgen_vi_list, iviDigitizer_vi_list);  // 执行设备间同步操作

    std::cout << "\n=== Trig Config ===" << std::endl;                         // 输出触发配置标题
    ViUInt32 triggerSource = IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_INTERNAL; // 设置触发源为内部PXI_STAR触发
    ViUInt32 triggerPeriod = 40000000;                                         // 设置触发周期为40M个时钟周期(需要被800整除)
    ViUInt32 triggerRepetSize = 4294967295;                                    // 设置触发重复次数为最大值
    ViUInt32 triggerPulseWidth = 20000000;                                     // 设置触发脉冲宽度为20M个时钟周期(为周期的一半)
    if (triggerSource == IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_INTERNAL){  // 判断触发源是否为内部PXI_STAR
        s = triggerConfigDAQ(iviDigitizer_vi, IVIDIGITIZER_VAL_TRIGGER_SOURCE_PXI_STAR_TRIG);  // 配置数字化仪使用PXI_STAR触发
        s = internalTriggerConfigSAT(iviSyncATrig_vi, triggerSource, triggerPeriod, triggerRepetSize, triggerPulseWidth);  // 配置同步触发器的内部触发参数
        s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_TRIGGER_SOURCE_P_PXI_STAR, triggerSource);  // 设置PXI_STAR触发源测试属性
    } else if (triggerSource == IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_EXTERNAL){  // 判断触发源是否为外部PXI_STAR
        s = triggerConfigDAQ(iviDigitizer_vi, IVIDIGITIZER_VAL_TRIGGER_SOURCE_PXI_STAR_TRIG);  // 配置数字化仪使用PXI_STAR触发
        s = internalTriggerConfigSAT(iviSyncATrig_vi, triggerSource);          // 配置同步触发器的外部触发源
        s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_TRIGGER_SOURCE_P_PXI_STAR, triggerSource);  // 设置PXI_STAR触发源测试属性
    }
    else
        std::cout << "\n=== Trig Source Error ===" << std::endl;             // 输出触发源配置错误信息

    std::cout << "\n=== Sample Config ===" << std::endl;                       // 输出采样配置标题
    ViConstString sampleEnableChannel = "-1";                                  // 设置采样使能通道为"-1"（表示所有通道）
    ViInt32 sampleEnable = 1;                                                  // 设置采样使能标志为1（启用）
    ViConstString sampleStrageDepthChannel = "-1";                            // 设置采样存储深度通道为"-1"（表示所有通道）
    ViUInt32 sampleStrageDepth = 65536;                                        // 设置采样存储深度为65536个采样点
    ViConstString sampleLenPreChannel = "-1";                                 // 设置采样预触发长度通道为"-1"（表示所有通道）
    ViUInt32 sampleLenPre = 32768;                                             // 设置采样预触发长度为32768个采样点
    ViConstString sampleTimesChannel = "-1";                                  // 设置采样次数通道为"-1"（表示所有通道）
    ViUInt32 sampleTimes = 4294967295;                                         // 设置采样次数为最大值（连续采样）
    ViConstString sampleLogicalExtractionMultipleChannel = "-1";              // 设置逻辑提取倍数通道为"-1"（表示所有通道）
    ViUInt32 sampleLogicalExtractionMultiple = 1;                             // 设置逻辑提取倍数为1
    ViConstString sampleCollectDataTruncationChannel = "-1";                  // 设置数据收集截断通道为"-1"（表示所有通道）
    ViUInt32 sampleCollectDataTruncation = 0;                                  // 设置数据收集截断为0（不截断）
    ViConstString sampleCollectDataTypeChannel = "-1";                        // 设置数据收集类型通道为"-1"（表示所有通道）
    ViUInt32 sampleCollectDataType = 0;                                        // 设置数据收集类型为0
    sampleConfigDAQ(iviDigitizer_vi,                                          // 调用采样配置函数，传入以下参数：
         sampleEnableChannel, sampleEnable,                                    // 采样使能通道和使能标志
         sampleStrageDepthChannel, sampleStrageDepth,                         // 存储深度通道和深度值
         sampleLenPreChannel, sampleLenPre,                                   // 预触发长度通道和长度值
         sampleTimesChannel, sampleTimes,                                     // 采样次数通道和次数值
         sampleLogicalExtractionMultipleChannel, sampleLogicalExtractionMultiple,  // 逻辑提取倍数通道和倍数值
         sampleCollectDataTruncationChannel, sampleCollectDataTruncation,     // 数据截断通道和截断设置
         sampleCollectDataTypeChannel, sampleCollectDataType);               // 数据类型通道和类型设置

    auto now = std::chrono::system_clock::now();                              // 获取当前系统时间
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);       // 将系统时间转换为time_t类型

    std::tm* localTime = std::localtime(&currentTime);                        // 将time_t转换为本地时间结构体
    std::ostringstream oss;                                                    // 创建字符串输出流对象
    oss << std::put_time(localTime, "%Y%m%d%H%M%S");                          // 格式化时间为YYYYMMDDHHMMSS格式

    std::string directoryPath = "./DAQ-14Bit-" + oss.str();                   // 创建数据存储目录路径，包含时间戳
    if (!std::filesystem::exists(directoryPath)) {                            // 检查目录是否存在
        std::filesystem::create_directories(directoryPath);                    // 如果不存在则创建目录
    }

    std::unordered_map<int, Deque> chnl_Deque;                               // 创建通道队列映射表
    std::unordered_map<int, std::ofstream> chnl_outF;                        // 创建通道文件流映射表
    std::unordered_map<int, std::mutex> chnl_mutex;                          // 创建通道互斥锁映射表
    std::unordered_map<int, std::thread> chnl_up_trd;                        // 创建通道上传线程映射表
    std::unordered_map<int, std::thread> chnl_write_trd;                     // 创建通道写文件线程映射表

    std::cout << "\n=== Config ===" << std::endl;                            // 输出配置标题

    // ViUInt32 waveformArraySize = 4 * 1024 * 1024;                            // 设置波形数组大小为4MB
//    ViUInt32 waveformArraySize = 67125248;                                   // 备选波形数组大小（已注释）
    ViUInt32 waveformArraySize = sampleStrageDepth + 16;                                   // 备选波形数组大小（已注释）
    
    ViReal64 maximumTime_s = 0.1;                                             // 设置最大等待时间为0.1秒
    ViReal64 times = 20;                                                       // 设置数据采集次数为20次
    std::string channelName = "0,1,2";                                          // 设置要使用的通道名称为"0,1"

    ViInt32 numChannel;                                                        // 定义通道号变量
    std::vector<std::string> channels = split(channelName, ',');              // 按逗号分割通道名称字符串
    for (const auto &channel: channels) {                                     // 遍历每个通道

        string2int(channel.data(), numChannel, 10)                            // 将通道名称字符串转换为整数

        if (chnl_Deque.find(numChannel) == chnl_Deque.end()) {               // 如果通道队列中不存在该通道
            s = IviDigitizer_ConfigureChannelDataDepthInt16(iviDigitizer_vi, channel, waveformArraySize);  // 配置通道数据深度
            for (int i=0; i<10; i++) {  
                std::cout << " 0------------------" << std::endl;                                      // 循环10次创建内存缓冲区

                auto mem = reinterpret_cast<nsuMemory_p>(IviDigitizer_CreateMemInt16(iviDigitizer_vi, waveformArraySize));  // 创建16位整数内存缓冲区
                std::cout << " 00------------------" << std::endl;
                chnl_Deque[numChannel].empty.Push(mem);                       // 将内存缓冲区推入空队列
            }
            std::cout << " 1------------------" << std::endl;
        }
        chnl_mutex[numChannel];                                               // 初始化通道互斥锁
        std::cout << " 2------------------" << std::endl;
        chnl_up_trd.emplace(numChannel, std::thread(upload_thread, iviDigitizer_vi, &chnl_Deque[numChannel], channel, waveformArraySize, times, &chnl_mutex[numChannel]));  // 创建上传线程
        chnl_write_trd.emplace(numChannel, std::thread(write_file_thread, iviDigitizer_vi, &chnl_Deque[numChannel], channel, waveformArraySize, directoryPath +"/up-res-14Bit_", &chnl_mutex[numChannel]));  // 创建写文件线程
    }

    std::cout << "\n=== RingBuffer Start Get the Data ===" << std::endl;      // 输出环形缓冲区数据采集开始信息
    s = IviSUATools_RunDigitizer(iviSUATools_vi, iviSyncATrig_vi, iviDigitizer_vi);  // 启动数字化仪开始数据采集

    for (const auto &channel: channels) {                                     // 遍历每个通道

        string2int(channel.data(), numChannel, 10)                            // 将通道名称字符串转换为整数

        chnl_up_trd[numChannel].join();                                       // 等待上传线程完成
        chnl_write_trd[numChannel].join();                                    // 等待写文件线程完成
    }

    std::cout << "\n=== RingBuffer Stop Get the Data ===" << std::endl;       // 输出环形缓冲区数据采集结束信息
    s = IviSUATools_StopDigitizer(iviSUATools_vi, iviSyncATrig_vi, iviDigitizer_vi);  // 停止数字化仪数据采集

    for (const auto &channel: channels) {                                     // 遍历每个通道

        string2int(channel.data(), numChannel, 10)                            // 将通道名称字符串转换为整数

        for (int i=0; i<10; i++) {                                            // 循环10次清理内存缓冲区
            auto mem = reinterpret_cast<iviDigitizer_memData*>(chnl_Deque[numChannel].empty.Pop());  // 从空队列中弹出内存缓冲区
            s = IviDigitizer_ClearMem(iviDigitizer_vi, mem);                  // 清理内存缓冲区
        }
    }
    isFAIL(IviDigitizer_Close(iviDigitizer_vi));                             // 关闭数字化仪会话
    isFAIL(IviSyncATrig_Close(iviSyncATrig_vi));                             // 关闭同步触发会话
    isFAIL(IviSUATools_Close(iviSUATools_vi));                               // 关闭SUA工具会话
    delete iviDigitizer_vi;                                                   // 删除数字化仪会话对象
    delete iviSyncATrig_vi;                                                   // 删除同步触发会话对象
    delete iviSUATools_vi;                                                    // 删除SUA工具会话对象
    return 0;                                                                 // 程序正常结束，返回0
}