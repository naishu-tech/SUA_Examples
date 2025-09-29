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

// ===============================================================================
// 数据结构定义
// ===============================================================================

/**
 * @brief 设备信息结构体
 */
typedef struct {
    ViString fpga_version;           // 固件版本号
    ViString fpga_hardware_version;  // 硬件版本号
    ViInt32 fpga_temper;                // 业务卡温度
    ViReal64 cpu_temper;                // 主控温度
    ViUInt32 status;                    // 模块运行状态
    ViReal64 usage;                     // CPU使用率
    ViReal64 controler_memory_total;    // 控制器总内存
    ViReal64 controler_memory_usage;    // 控制器内存使用情况
    ViString ivi_version;            // IVI版本号
} IviDeviceInfo;

struct iviSUATools_ViSession{
    ViStatus error_code{};                // 操作状态码
    IviDeviceInfo infoFgen;                 // 设备信息
    IviDeviceInfo infoDigitizer;                 // 设备信息
    IviDeviceInfo infoSyncATrig;                 // 设备信息
    IviDeviceInfo infoPXIMainBoard;                 // 设备信息
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
// 信息获取函数
// ===============================================================================
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_GetInfoFgen(iviSUATools_ViSession *vi, iviFgen_ViSession *iviFgen_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_GetInfoDigitizer(iviSUATools_ViSession *vi, iviDigitizer_ViSession *iviDigitizer_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_GetInfoSyncATrig(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_GetInfoPXIMainBoard(iviSUATools_ViSession *vi, iviPXIMainBoard_ViSession *iviPXIMainBoard_vi);

// ===============================================================================
// Python 集成函数
// ===============================================================================

// Python 集成函数
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_NSWave(iviSUATools_ViSession *vi, iviFgen_ViSession *iviFgen_vi, ViConstString channel, std::map<ViString, waveformHandle *> waveformHandle_map, const ViString& NSWaveData);

// ===============================================================================
// 运行控制函数
// ===============================================================================

// 运行控制函数
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_RunFgen(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi, iviFgen_ViSession *iviFgen_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_StopFgen(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi, iviFgen_ViSession *iviFgen_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_RunDigitizer(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi, iviDigitizer_ViSession *iviDigitizer_vi);
DLLEXTERN RIGOLLIB_API ViStatus IviSUATools_StopDigitizer(iviSUATools_ViSession *vi, iviSyncATrig_ViSession *iviSyncATrig_vi, iviDigitizer_ViSession *iviDigitizer_vi);

#endif //C_SUA_TOOLS_H
