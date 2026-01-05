//
// Created by sn06129 on 2025/7/25.
//

// Project Headers
#include "tool_config.h"

//#include "IviSUATools.h"
#include "IviSUATools.h"
#include "IviFgen.h"
#include "IviDigitizer.h"

#include <fstream>
#include <cstring>
#include <thread>
#include <chrono>
#include <list>
#include <vector>
#include <map>
#include <exception>

ViStatus preRF(iviFgen_ViSession* iviFgen_vi){
    ViStatus error = VI_STATE_SUCCESS;
    ViReal64 DAC_sample_rate;
    error = IviFgen_GetAttributeViReal64(iviFgen_vi, "0", IVIDIGITIZER_ATTR_ADC_SAMPLE_RATE, &DAC_sample_rate);
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_CHNL_TYPE, 0);
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_SAMPLE_RATE, int(DAC_sample_rate / 1e6 ));
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_OUTPUT_GEAR, 500);
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_VOLTAGE, 0);
    error = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_LSDADC_SET_EXE, 0);
    return error;
}


int main() {

    ViStatus s = VI_STATE_SUCCESS;

    std::string resource_db_path ="C:/Program Files/IVI Foundation/IVI/RIGOL/file/resourceDB.json";
    std::string logicalName = "PXI::0::INSTR";

    std::cout << "\n=== Initialize IviSUATools ===" << std::endl;
    auto iviSUATools_vi = new iviSUATools_ViSession;
    s = IviSUATools_Initialize(iviSUATools_vi, logicalName, VI_STATE_FALSE, VI_STATE_TRUE, resource_db_path);
    std::cout << "IviSUATools initialized successfully" << std::endl;

    std::cout << "\n=== Initialize IviFgen ===" << std::endl;
    auto iviFgen_vi = new iviFgen_ViSession;
    s = IviFgen_Initialize(logicalName, VI_STATE_FALSE, VI_STATE_TRUE, iviFgen_vi, resource_db_path);
    std::cout << "IviFgen initialized successfully" << std::endl;

    std::cout << "\n=== LSDADC Config ===" << std::endl;                       // Output LSDADC configuration title
    s = preRF(iviFgen_vi);                                               // Call pre-RF configuration function

    std::cout << "\n=== RF Config ===" << std::endl;                           // Output RF configuration title
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_OFFLINE_WORK, 1);  // Set offline work mode to enabled
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "0", IVIBASE_ATTR_RF_CONFIG, 0);     // Set RF configuration parameter to 0


    std::cout << "\n=== Trig Config ===" << std::endl;
    ViUInt32 triggerSource = IVIFGEN_VAL_TRIGGER_SOURCE_INTERNAL; // Source
    ViUInt32 triggerPeriod = 40000000; // Period (must be divisible by 800)
    ViUInt32 triggerRepetSize = 4294967295;// Trigger count
    ViUInt32 triggerPulseWidth = 20000000; // Pulse width (half of the period)

    s = triggerConfigAWG(iviFgen_vi, triggerSource);
    s = internalTriggerConfigAWG(iviFgen_vi, triggerPulseWidth, triggerRepetSize, triggerPeriod, 0);

    std::cout << "\n=== Chnl EN ===" << std::endl;
    s = IviFgen_SetAttributeViUInt32(iviFgen_vi, "-1", IVIFGEN_ATTR_SYSTEM_STATUS_ENABLE, 1);

    std::cout << "\n=== Create NSWave ===" << std::endl;
    // Sequential playback
    ViString nswave_code = R"(
@nw.kernel
def program(wlist: dict[str, np.ndarray]):
    seg1: nw.ArbWave = nw.init_arbwave(wlist, 'seg1')
    while True:
        nw.play_arb(seg1)
    return nw.Kernel()
)";

    // Wait for trigger playback
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

    // Waveform generation parameters
    std::string waveformType = "Sawtooth"; // Sin, Square, Triangle, Sawtooth, Chirp
    ViReal64 sampleRate = 4000000000.0; // Hz, default 4GSps
    ViReal64 frequency = 50000000.0;   // Hz, default 50MHz
    WaveformParameters waveformParams = WaveformParameters(waveformType, sampleRate, frequency);

    // Waveform file list (if empty, waveform will be generated)
    std::list<ViString> wfmPath_list = {
//            "./wfm/Sin_Fixed_4000MSps_0Offset_1Amp_100MHz_0MBw_0Phase_1024us_16bit_Signed_1024us_1Row_1Column.dat"
    };

    std::map<ViString, waveformHandle *> waveformHandle_map;
    ViChar* buffer = nullptr;
    ViInt16* bufferWfm = nullptr;

    // If wfmPath_list is empty, generate waveform using GenerateWaveformFile
    if (wfmPath_list.empty()) {
        std::cout << "Waveform path list is empty, generating waveform..." << std::endl;
        WaveformResult result = GenerateWaveformFile(waveformParams);
        if (result.filePath.empty()) {
            std::cerr << "Failed to generate waveform file" << std::endl;
            return 1;
        }
        wfmPath_list.push_back(result.filePath);
        std::cout << "Generated waveform file: " << result.filePath << std::endl;
    }

    for (auto& wfmPath : wfmPath_list) {
        // Check if file exists
        std::ifstream testFile(wfmPath, std::ios::binary);
        bool fileExists = testFile.good();
        testFile.close();

        // If file doesn't exist, generate waveform
        if (!fileExists) {
            std::cout << "Waveform file does not exist: " << wfmPath << std::endl;
            std::cout << "Generating waveform..." << std::endl;
            WaveformResult result = GenerateWaveformFile(waveformParams);
            if (result.filePath.empty()) {
                std::cerr << "Failed to generate waveform file" << std::endl;
                return 1;
            }
            wfmPath = result.filePath;  // Update path to generated file
            std::cout << "Generated waveform file: " << result.filePath << std::endl;
        }

        std::ifstream fileWfm(wfmPath, std::ios::binary);
        if (!fileWfm) {
            std::cerr << "Open " << wfmPath <<" File Failed!" << std::endl;
            return 1;
        }

        fileWfm.seekg(0, std::ios::end);
        std::streamsize sizeWfm = fileWfm.tellg();
        std::cout << "The stream file size is " << sizeWfm << " bytes" << std::endl;
        fileWfm.seekg(0, std::ios::beg);

        // Free previous buffers if they exist
        if (buffer != nullptr) {
            delete[] buffer;
            buffer = nullptr;
        }
        if (bufferWfm != nullptr) {
            delete[] bufferWfm;
            bufferWfm = nullptr;
        }

        // Allocate buffers based on actual file size
        buffer = new ViChar[sizeWfm];
        bufferWfm = new ViInt16[sizeWfm / sizeof(ViInt16)];

        if (fileWfm.read(buffer, sizeWfm)) {
            std::cout << "Successfully read data in "<< sizeWfm <<" Byte" << std::endl;
        } else {
            std::cerr << "Failed to read file" << std::endl;
            delete[] buffer;
            delete[] bufferWfm;
            return 1;
        }
        fileWfm.close();

        memcpy(bufferWfm, buffer, sizeWfm);
        auto *wfmHandle = new waveformHandle;
        std::cout << "Create Arb Waveform: " << (sizeWfm / sizeof(ViInt16)) << std::endl;
        isFAIL(IviFgen_CreateArbWaveformViInt16(iviFgen_vi, sizeWfm / sizeof(ViInt16), bufferWfm, wfmHandle));

        waveformHandle_map["seg1"] = wfmHandle;
    }

    std::cout << "\n=== Download NSWave & Waveform ===" << std::endl;
    int result = IviSUATools_NSWave(iviSUATools_vi, iviFgen_vi, "0", waveformHandle_map, nswave_code);
    result = IviSUATools_NSWave(iviSUATools_vi, iviFgen_vi, "1", waveformHandle_map, nswave_code);

    for (const auto& pair : waveformHandle_map) {
        const ViString& key = pair.first;
        waveformHandle* wfmHandle = pair.second;
        isFAIL(IviFgen_ClearArbWaveform(iviFgen_vi, wfmHandle));
    }

    std::cout << "\n=== Play the Waveform ===" << std::endl;
    s = IviFgen_InitiateGeneration(iviFgen_vi);


    std::cout << "\n=== wait 10S ===" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "\n=== Stop the Waveform ===" << std::endl;
    s = IviFgen_AbortGeneration(iviFgen_vi);

    // Free buffers
    if (buffer != nullptr) {
        delete[] buffer;
        buffer = nullptr;
    }
    if (bufferWfm != nullptr) {
        delete[] bufferWfm;
        bufferWfm = nullptr;
    }

    std::cout << "\n=== Test Completed ===" << std::endl;
    isFAIL(IviFgen_Close(iviFgen_vi));
    isFAIL(IviSUATools_Close(iviSUATools_vi));
    delete iviFgen_vi;
    delete iviSUATools_vi;

    return 0;
}