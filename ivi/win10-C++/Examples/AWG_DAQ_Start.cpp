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
    // System configuration
    std::string resource_db_path = "./resourceDB.json";
    std::string logicalName = "PXI::0::INSTR";
};


int main(int argc, char *argv[]){
    SystemConfig config;

    // Initialize configuration structure
    ViStatus s = VI_STATE_SUCCESS;

    std::cout << "\n=== Initialize IviFgen ===" << std::endl;
    auto* iviFgen_vi = new iviFgen_ViSession;
    s = IviFgen_Initialize(config.logicalName, VI_STATE_FALSE, VI_STATE_TRUE, iviFgen_vi, config.resource_db_path);
    std::cout << "IviFgen initialized successfully" << std::endl;

    std::cout << "\n=== Initialize IviDigitizer ===" << std::endl;
    auto* iviDigitizer_vi = new iviDigitizer_ViSession;
    s = IviDigitizer_Initialize(config.logicalName, VI_STATE_FALSE, VI_STATE_TRUE, iviDigitizer_vi, config.resource_db_path);
    std::cout << "IviDigitizer initialized successfully" << std::endl;

    // Check if digitizer is in RingBuffer mode
    ViInt32 mode;
    s = IviDigitizer_GetAttributeViInt32(iviDigitizer_vi, "0", IVIDIGITIZER_ATTR_SAMPLE_WORK_MODE, &mode);
    if (mode != IVIDIGITIZER_VAL_WORK_MODE_RING_BUFFER) {
        std::cout << "Error: The Digitizer Work Mode is not RingBuffer!" << std::endl;
        isFAIL(IviDigitizer_Close(iviDigitizer_vi));
        isFAIL(IviFgen_Close(iviFgen_vi));
        delete iviDigitizer_vi;
        delete iviFgen_vi;
        return 0;
    }

    // ========== Start AWG and DAQ ==========
    std::cout << "\n=== Start AWG Waveform Playback and DAQ Data Acquisition ===" << std::endl;
    s = IviFgen_InitiateGeneration(iviFgen_vi);
    s = IviDigitizer_InitiateAcquisition(iviDigitizer_vi);

    // ========== Close Sessions ==========
    std::cout << "\n=== Test Completed ===" << std::endl;
    isFAIL(IviFgen_Close(iviFgen_vi));
    isFAIL(IviDigitizer_Close(iviDigitizer_vi));
    delete iviFgen_vi;
    delete iviDigitizer_vi;

    return 0;
}