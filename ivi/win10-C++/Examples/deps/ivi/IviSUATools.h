//
// Created by sn06129 on 2025/7/18.
//

#ifndef C_SUA_TOOLS_H
#define C_SUA_TOOLS_H

#include "type.h"

#include "IviATTR.h"
#include "IviVAL.h"

#include "IviBase.h"
#include "IviDigitizer.h"
#include "IviFgen.h"
#include "IviSyncATrig.h"
#include "IviPXIMainBoard.h"
#include "jsoncpp/json/json.h"

// ===============================================================================
// Data Structure Definitions
// ===============================================================================

/**
 * @brief Device information structure
 */
typedef struct {
    ViString fpga_version;           // Firmware version number
    ViString fpga_hardware_version;  // Hardware version number
    ViInt32 fpga_temper;                // Service card temperature
    ViReal64 cpu_temper;                // Main controller temperature
    ViUInt32 status;                    // Module running status
    ViReal64 usage;                     // CPU usage
    ViReal64 controler_memory_total;    // Controller total memory
    ViReal64 controler_memory_usage;    // Controller memory usage
    ViString ivi_version;            // IVI version number
} IviDeviceInfo;

struct iviSUATools_ViSession{
    ViStatus error_code{};                // Operation status code
    IviDeviceInfo infoFgen;                 // Device information
    IviDeviceInfo infoDigitizer;                 // Device information
    IviDeviceInfo infoSyncATrig;                 // Device information
    IviDeviceInfo infoPXIMainBoard;                 // Device information
    ViInt32 code_size = 1310720;
    ViInt32 code_chnl_size  = 163840;
    ViChar *NSQC = nullptr;
};

DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_Initialize (iviSUATools_ViSession *vi, const ViString& logicalName="", ViBoolean IDQuery=false, ViBoolean resetDevice=false, const ViString& resourceDBPath = "");
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_Reset (iviSUATools_ViSession *vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_Close (iviSUATools_ViSession *vi);

DLLEXTERN RIGOLLIB_API ViString IviSUATools_ScanOnlineBoards(iviSUATools_ViSession *vi, const ViString& config_path="conf/config.yaml");

DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_Sync(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi, const std::list<iviFgen_ViSession *>& iviFgen_vis, const std::list<iviDigitizer_ViSession *>&  iviDigitizer_vis);

// ===============================================================================
// Information Retrieval Functions
// ===============================================================================
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_GetInfoFgen(iviSUATools_ViSession *vi, iviFgen_ViSession *iviFgen_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_GetInfoDigitizer(iviSUATools_ViSession *vi, iviDigitizer_ViSession *iviDigitizer_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_GetInfoSyncATrig(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_GetInfoPXIMainBoard(iviSUATools_ViSession *vi, iviPXIMainBoard_ViSession *iviPXIMainBoard_vi);

// ===============================================================================
// Python Integration Functions
// ===============================================================================
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_NSWave(iviSUATools_ViSession *vi, iviFgen_ViSession *iviFgen_vi, ViConstString channel, std::map<ViString, waveformHandle *> waveformHandle_map, const ViString& NSWaveData);

// ===============================================================================
// Runtime Control Functions
// ===============================================================================
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_RunFgen(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi, iviFgen_ViSession *iviFgen_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_StopFgen(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi, iviFgen_ViSession *iviFgen_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_RunDigitizer(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi, iviDigitizer_ViSession *iviDigitizer_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_StopDigitizer(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi, iviDigitizer_ViSession *iviDigitizer_vi);

#endif //C_SUA_TOOLS_H
