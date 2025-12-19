//
// Created by sn06129 on 2025/12/19.
//

// Project Headers
#include "tool_config.h"

// Third-party IVI Libraries
#include "IviSUATools.h"
#include "IviFgen.h"
#include "IviDigitizer.h"
#include "IviBase.h"

// C++ Standard Library (additional headers not in tool_config.h)
#include <list>

// Forward declaration
// Configuration structure to hold all variables
struct SystemConfig {
    std::string python_path = "C:/Users/sn06129/.conda/envs/JupyterServer";

    // System configuration
    std::string resource_db_path = "./resourceDB.json";
    std::string logicalName = "PXI::0::INSTR";
};


int main(int argc, char *argv[]){
    SystemConfig config;
#ifdef _WIN32
    std::cout << "=== Configuring Python Paths ===" << std::endl;
    configure_python_paths(config.python_path);
    std::cout << "Python paths configured" << std::endl;
#endif

    // Initialize configuration structure
    ViStatus s = VI_STATE_SUCCESS;
    std::map<ViString, waveformHandle *> waveformHandle_map;
    // Session pointers
    iviSUATools_ViSession* iviSUATools_vi = nullptr;
    iviFgen_ViSession* iviFgen_vi = nullptr;
    iviDigitizer_ViSession* iviDigitizer_vi = nullptr;

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

    // ========== Stop AWG and DAQ ==========
    std::cout << "\n=== Stop AWG Waveform Playback and DAQ Data Acquisition ===" << std::endl;
    s = IviFgen_AbortGeneration(iviFgen_vi);
    s = IviDigitizer_Abort(iviDigitizer_vi);

    // ========== Close Sessions ==========
    std::cout << "\n=== Test Completed ===" << std::endl;
    isFAIL(IviFgen_Close(iviFgen_vi));
    isFAIL(IviDigitizer_Close(iviDigitizer_vi));
    isFAIL(IviSUATools_Close(iviSUATools_vi));
    delete iviFgen_vi;
    delete iviDigitizer_vi;
    delete iviSUATools_vi;

    return 0;
}