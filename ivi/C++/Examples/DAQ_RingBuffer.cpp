//
// Created by sn06129 on 2025/7/25.
//

# include "IviDigitizer.h"
# include "IviSUATools.h"
# include "tool_config.h"

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

int main(int argc, char *argv[]){
    ViUInt32 res = 0;
    auto iviSUATools_vi = new iviSUATools_ViSession;
    auto s = IviSUATools_Initialize(iviSUATools_vi);

    std::cout << "\n=== Creating Resource DB ===" << std::endl;
    std::string resource_db_path = IviSUATools_ScanOnlineBoards(iviSUATools_vi, "conf/config.yaml");
    std::cout << "BVC Manager initialized successfully. Resource DB: " << resource_db_path << std::endl;

    auto iviDigitizer_vi = new iviDigitizer_ViSession;
    s = IviDigitizer_Initialize("PXI::0::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviDigitizer_vi, resource_db_path);

    ViInt32 mode;
    s = IviDigitizer_GetAttributeViInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_SAMPLE_WORK_MODE, &mode);
    if (mode != IVIDIGITIZER_VAL_WORK_MODE_RING_BUFFER) {
        std::cout << "Error: The Digitizer Work Mode is not RingBuffer!" << std::endl;
        isFAIL(IviDigitizer_Close(iviDigitizer_vi));
        isFAIL(IviSUATools_Close(iviSUATools_vi));
        delete iviDigitizer_vi;
        delete iviSUATools_vi;
        return 0;
    }

    std::cout << "\n=== LSDADC Config ===" << std::endl;
    s = preDAQ(iviDigitizer_vi);

    std::cout << "\n=== RF Config ===" << std::endl;
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_OFFLINE_WORK, 1);
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_RF_CONFIG, 0);

    auto iviSyncATrig_vi = new iviSyncATrig_ViSession;
    s = IviSyncATrig_Initialize("PXI::1::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviSyncATrig_vi, resource_db_path);

    std::cout << "\n=== DDC Config ===" << std::endl;
    s = DDConfigDAQ(iviDigitizer_vi, 0, "-1", 0);

    std::cout << "\n=== Extract Multiple Config ===" << std::endl;
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_ADC_EXTRACT_MULTIPLE, 1);

    std::cout << "\n=== Sample Rate Config ===" << std::endl;
    s = IviDigitizer_SetAttributeViReal64(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_ADC_SAMPLE_RATE, 2000000000);

    std::cout << "\n=== SAT Clock Config ===" << std::endl;
    s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_CLOCK_SOURCE_100MHZ, IVISYNCATRIG_VAL_100MHZ_SOURCE_INTERNAL);
    s = IviSyncATrig_SetAttributeViInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_CLOCK_SOURCE_100MHZ_EXE, 0);
    s = IviSyncATrig_GetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_CLOCK_SOURCE_100MHZ_RESULT, &res);
    std::cout << "IVISYNCATRIG_ATTR_TEST_CLOCK_SOURCE_100MHZ res is " << res << std::endl;

    std::cout << "\n=== Digitizer Clock Config ===" << std::endl;
    s = IviDigitizer_SetAttributeViUInt32(iviDigitizer_vi, "0", IVIBASE_ATTR_REF_CLOCK_SOURCE, IVIBASE_VAL_REF_CLOCK_EXTERNAL);
    s = IviDigitizer_SetAttributeViReal64(iviDigitizer_vi, "0", IVIBASE_ATTR_REF_FREQ_FREQUENCY, 100000000.0);

    std::cout << "\n=== SYNC Config ===" << std::endl;
    std::list<iviFgen_ViSession *> iviFgen_vi_list;
    std::list<iviDigitizer_ViSession *> iviDigitizer_vi_list;
    iviDigitizer_vi_list.push_back(iviDigitizer_vi);
    s = IviSUATools_Sync(iviSUATools_vi, iviSyncATrig_vi, iviFgen_vi_list, iviDigitizer_vi_list);

    std::cout << "\n=== Trig Config ===" << std::endl;
    ViUInt32 triggerSource = IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_INTERNAL; //来源
    ViUInt32 triggerPeriod = 40000000; // 周期(需要被800整除)
    ViUInt32 triggerRepetSize = 4294967295;// 触发数量
    ViUInt32 triggerPulseWidth = 20000000; // 脉宽(为周期的一半)
    if (triggerSource == IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_INTERNAL){
        s = triggerConfigDAQ(iviDigitizer_vi, IVIDIGITIZER_VAL_TRIGGER_SOURCE_PXI_STAR_TRIG);
        s = internalTriggerConfigSAT(iviSyncATrig_vi, triggerSource, triggerPeriod, triggerRepetSize, triggerPulseWidth);
        s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_TRIGGER_SOURCE_P_PXI_STAR, triggerSource);
    } else if (triggerSource == IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_EXTERNAL){
        s = triggerConfigDAQ(iviDigitizer_vi, IVIDIGITIZER_VAL_TRIGGER_SOURCE_PXI_STAR_TRIG);
        s = internalTriggerConfigSAT(iviSyncATrig_vi, triggerSource);
        s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_TRIGGER_SOURCE_P_PXI_STAR, triggerSource);
    }
    else
        std::cout << "\n=== Trig Source Error ===" << std::endl;

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

    ViUInt32 waveformArraySize = sampleStrageDepth + 16;
    ViReal64 maximumTime_s = 0.1;
    ViReal64 times = 20;

    auto waveformArray = IviDigitizer_CreateMemInt16(iviDigitizer_vi, waveformArraySize);
    auto st = std::chrono::steady_clock::now();
    std::ofstream outFile_0(directoryPath +"/up-res-14Bit-0.data", std::ios::binary);
    std::ofstream outFile_1(directoryPath +"/up-res-14Bit-1.data", std::ios::binary);

    std::cout << "\n=== RingBuffer Start Get the Data ===" << std::endl;
    s = IviSUATools_RunDigitizer(iviSUATools_vi, iviSyncATrig_vi, iviDigitizer_vi);

    ViUInt32 m = 0;
    ViUInt32 speed_count = 0;
    while (true){
        if (m == times){
            break;
        }
        while (IviDigitizer_WaitForAcquisitionComplete(iviDigitizer_vi, "0", maximumTime_s) == VI_ERROR_MAX_TIME_EXCEEDED){
            std::cout << "chnl 0 wait...." << std::endl;
        }
        s = IviDigitizer_FetchWaveformMemInt16 (iviDigitizer_vi, "0", waveformArraySize, waveformArray);
        s = IviDigitizer_TriggerStart (iviDigitizer_vi, "0");

        outFile_0.write(waveformArray->memDataHandle, (waveformArraySize*sizeof(ViInt16)));

        while (IviDigitizer_WaitForAcquisitionComplete(iviDigitizer_vi, "1", maximumTime_s) == VI_ERROR_MAX_TIME_EXCEEDED){
            std::cout << "chnl 1 wait...." << std::endl;
        }
        s = IviDigitizer_FetchWaveformMemInt16 (iviDigitizer_vi, "1", waveformArraySize, waveformArray);
        s = IviDigitizer_TriggerStart (iviDigitizer_vi, "1");

        outFile_1.write(waveformArray->memDataHandle, (waveformArraySize*sizeof(ViInt16)));

        m ++;
        speed_count += waveformArraySize * 2;
        std::cout << "upload num is "<< m <<std::endl;
        if (m % 20 == 0) {
            auto count = std::chrono::steady_clock::now() - st;
            std::cout << std::flush << '\r' << "current up Data speed: " << ((speed_count*sizeof(ViInt16))*1000.)/(count.count()) << "MB/s" << std::endl;
            st = std::chrono::steady_clock::now();
        }
    }
    std::cout << "\n=== RingBuffer Stop Get the Data ===" << std::endl;
    s = IviSUATools_StopDigitizer(iviSUATools_vi, iviSyncATrig_vi, iviDigitizer_vi);
    outFile_0.close();
    outFile_1.close();

    s = IviDigitizer_ClearMem(iviDigitizer_vi, waveformArray);
    isFAIL(IviDigitizer_Close(iviDigitizer_vi));
    isFAIL(IviSyncATrig_Close(iviSyncATrig_vi));
    isFAIL(IviSUATools_Close(iviSUATools_vi));
    delete iviDigitizer_vi;
    delete iviSyncATrig_vi;
    delete iviSUATools_vi;
    return 0;
}