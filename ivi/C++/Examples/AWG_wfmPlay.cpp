//
// Created by sn06129 on 2025/7/25.
//

#include "IviSUATools.h"
#include "IviFgen.h"
#include "tool_config.h"

ViStatus preAWG(iviFgen_ViSession* iviFgen_vi){
    ViStatus error = VI_STATE_SUCCESS;
//    error = IviFgen_SetAttributeViInt32(iviFgen_vi, "0", IVIFGEN_ATTR_TRIGGER_SOURCE_EXE, 0);
    ViReal64 DAC_sample_rate;
    error = IviFgen_GetAttributeViReal64(iviFgen_vi, "0", IVIFGEN_ATTR_DAC_SAMPLE_RATE, &DAC_sample_rate);
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_CHNL_TYPE, 1);
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_SAMPLE_RATE, int(DAC_sample_rate / 1e6 ));
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_OUTPUT_GEAR, 250);
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_VOLTAGE, 0);
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_EXE, 0);
    return error;
}

int main() {
    std::cout << "=== AWG wfm Play Test ===" << std::endl;

    auto iviSUATools_vi = new iviSUATools_ViSession;
    auto s = IviSUATools_Initialize(iviSUATools_vi);

    std::cout << "\n=== Creating Resource DB ===" << std::endl;
    std::string resource_db_path = IviSUATools_ScanOnlineBoards(iviSUATools_vi, "conf/config.yaml");
    std::cout << "BVC Manager initialized successfully. Resource DB: " << resource_db_path << std::endl;

    auto iviFgen_vi = new iviFgen_ViSession;
    s = IviFgen_Initialize("PXI::0::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviFgen_vi, resource_db_path);

    std::cout << "\n=== LSDADC Config ===" << std::endl;
    s = preAWG(iviFgen_vi);

    std::cout << "\n=== RF Config ===" << std::endl;
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIFGEN_ATTR_DAC_INTERNAL_MULTIPLE, 1);
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIFGEN_ATTR_DAC_SAMPLE_RATE, 4000);
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_OFFLINE_WORK, 1);
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_RF_CONFIG, 0);

    auto iviSyncATrig_vi = new iviSyncATrig_ViSession;
    s = IviSyncATrig_Initialize("PXI::1::INSTR", VI_STATE_FALSE, VI_STATE_TRUE, iviSyncATrig_vi, resource_db_path);

    std::cout << "\n=== Trig Config ===" << std::endl;
    ViUInt32 triggerSource = IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_INTERNAL; //来源
    ViUInt32 triggerPeriod = 6400; // 周期(需要被800整除)
    ViUInt32 triggerRepetSize = 4294967295;// 触发数量
    ViUInt32 triggerPulseWidth = 1600; // 脉宽(为周期的一半)

    if (triggerSource == IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_INTERNAL){
        s = triggerConfigAWG(iviFgen_vi, IVIFGEN_VAL_TRIGGER_SOURCE_PXI_STAR_TRIG);
        s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_TRIGGER_SOURCE_P_PXI_STAR, triggerSource);
        s = internalTriggerConfigSAT(iviSyncATrig_vi, triggerSource, triggerPeriod, triggerRepetSize, triggerPulseWidth);
    } else if (triggerSource == IVISYNCATRIG_VAL_TRIGGER_SOURCE_P_PXI_STAR_EXTERNAL){
        s = triggerConfigAWG(iviFgen_vi, IVIFGEN_VAL_TRIGGER_SOURCE_PXI_STAR_TRIG);
        s = IviSyncATrig_SetAttributeViUInt32(iviSyncATrig_vi, "0", IVISYNCATRIG_ATTR_TEST_TRIGGER_SOURCE_P_PXI_STAR, triggerSource);
        s = internalTriggerConfigSAT(iviSyncATrig_vi, triggerSource);
    }
    else
        std::cout << "\n=== Trig Source Error ===" << std::endl;

    std::cout << "\n=== Chnl EN ===" << std::endl;
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "-1", IVIFGEN_ATTR_SYSTEM_STATUS_ENABLE, 1);

    std::cout << "\n=== DUC Config ===" << std::endl;
    s = DUCConfigAWG(iviFgen_vi, 0, "-1", 0);

    std::cout << "\n=== Internal Multiple Config ===" << std::endl;
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIFGEN_ATTR_DAC_INTERNAL_MULTIPLE, 1);

    std::cout << "\n=== Sample Rate Config ===" << std::endl;
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIFGEN_ATTR_DAC_SAMPLE_RATE, 4000);


    std::cout << "\n=== SYNC Config ===" << std::endl;
    std::list<iviFgen_ViSession *> iviFgen_vi_list;
    std::list<iviDigitizer_ViSession *> iviDigitizer_vi_list;
    iviFgen_vi_list.push_back(iviFgen_vi);
    s = IviSUATools_Sync(iviSUATools_vi, iviSyncATrig_vi, iviFgen_vi_list, iviDigitizer_vi_list);

    std::cout << "\n=== Create NSWave ===" << std::endl;
    // 顺序播放
    ViString nswave_code = R"(
@nw.kernel
def program(wlist: dict[str, np.ndarray]):
    seg1: nw.ArbWave = nw.init_arbwave(wlist, 'seg1')
    while True:
        nw.play_arb(seg1)
    return nw.Kernel()
)";

    // 等待触发播放
//    ViString nswave_code = R"(
//@nw.kernel
//def program(wlist: dict[str, np.ndarray]):
//    seg1: nw.ArbWave = nw.init_arbwave(wlist, 'seg1')
//    while True:
//        nw.wait_for_trigger_with_source(1)
//        nw.play_arb(seg1)
//    return nw.Kernel()
//)";

    std::cout << "NSWave Code Preview:" << std::endl;
    std::cout << nswave_code << std::endl;

    std::cout << "\n=== Read Waveform ===" << std::endl;

    // 波形列表
    std::list<ViString> wfmPath_list = {
            "./wfm/Sin_Fixed_4000MSps_0Offset_1Amp_100MHz_0MBw_0Phase_1024us_16bit_Signed_1024us_1Row_1Column.dat"
    };
    ViInt32 wfmSize = 8192000;

    std::map<ViString, waveformHandle *> waveformHandle_map;

    auto buffer = new ViChar[wfmSize / sizeof(ViChar)];
    auto bufferWfm = new ViInt16 [wfmSize / sizeof(ViInt16)];

    for (auto& wfmPath : wfmPath_list) {
        std::ifstream fileWfm(wfmPath, std::ios::binary);
        if (!fileWfm) {
            std::cerr << "Open " << wfmPath <<" File Failed!" << std::endl;
            return 1;
        }

        fileWfm.seekg(0, std::ios::end);
        std::streamsize sizeWfm = fileWfm.tellg();
        std::cout << "The stream file size is " << sizeWfm << std::endl;
        fileWfm.seekg(0, std::ios::beg);

        if (fileWfm.read(buffer, wfmSize)) {
            std::cout << "Successfully read data in "<< wfmSize <<" Byte" << std::endl;
        } else {
            std::cerr << "Failed to read file" << std::endl;
            return 1;
        }
        fileWfm.close();

        auto *wfmHandle = new waveformHandle;

        std::cout << "Create Arb Waveform: " << (wfmSize / sizeof(ViInt16)) << std::endl;
        isFAIL(IviFgen_CreateArbWaveformViInt16(iviFgen_vi, wfmSize / sizeof(ViInt16), bufferWfm, wfmHandle));

        waveformHandle_map["seg1"] = wfmHandle;
    }

    std::cout << "\n=== Download NSWave & Waveform ===" << std::endl;
    int result = IviSUATools_NSWave(iviSUATools_vi, iviFgen_vi, "0", waveformHandle_map, nswave_code);
    result = IviSUATools_NSWave(iviSUATools_vi, iviFgen_vi, "1", waveformHandle_map, nswave_code);
    result = IviSUATools_NSWave(iviSUATools_vi, iviFgen_vi, "2", waveformHandle_map, nswave_code);
    result = IviSUATools_NSWave(iviSUATools_vi, iviFgen_vi, "3", waveformHandle_map, nswave_code);

    for (const auto& pair : waveformHandle_map) {
        const ViString& key = pair.first;
        waveformHandle* wfmHandle = pair.second;
        isFAIL(IviFgen_ClearArbWaveform(iviFgen_vi, wfmHandle));
    }

    std::cout << "\n=== Play the Waveform ===" << std::endl;
    s = IviSUATools_RunFgen(iviSUATools_vi, iviSyncATrig_vi, iviFgen_vi);

    std::cout << "\n=== wait 10S ===" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "\n=== Stop the Waveform ===" << std::endl;
    s = IviSUATools_StopFgen(iviSUATools_vi, iviSyncATrig_vi, iviFgen_vi);

    std::cout << "\n=== Test Completed ===" << std::endl;
    isFAIL(IviFgen_Close(iviFgen_vi));
    isFAIL(IviSUATools_Close(iviSUATools_vi));
    isFAIL(IviSyncATrig_Close(iviSyncATrig_vi));
    delete iviFgen_vi;
    delete iviSUATools_vi;
    delete iviSyncATrig_vi;
    delete[] buffer;
    delete[] bufferWfm;
    return 0;
}