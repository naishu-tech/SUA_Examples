#ifndef IVI_TYPE_H
#define IVI_TYPE_H

// ===============================================================================
// 平台相关宏定义
// ===============================================================================
#ifdef __linux__
    #define _API_CALL
    #define DLLEXTERN
    #define RIGOLLIB_API
#else  // Windows, RTX
    #define RIGOLLIB_API __declspec(dllexport)
    #define DLLEXTERN extern "C"
    #define NOMINMAX  // 防止Windows.h定义min/max宏
#endif

// ===============================================================================
// C 标准库头文件
// ===============================================================================
#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstdint>

// ===============================================================================
// C++ 标准库头文件
// ===============================================================================
// 容器类
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>

// 流和文件操作
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

// 其他核心功能
#include <memory>
#include <variant>
#include <complex>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <exception>
#include <unordered_set>

// 并发和时间
#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>

// ===============================================================================
// 系统特定头文件
// ===============================================================================
#ifdef __linux__
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
#elif defined(_WIN32)
    // Windows特定头文件可以在这里添加
    // #include <windows.h>  // 如果需要的话
#endif

// ===============================================================================
// 第三方库头文件
// ===============================================================================
// JSON库
#include "jsoncpp/json/json.h"

// NSUKit相关
#include "../nsukitcpp/NSUKit.h"
#include "../nsukitcpp/xdma_api.h"
#include "../nsukitcpp/high_level/HardwareSync.h"

// 条件依赖的接口
#ifdef IVI_WITH_XDMA
    #include "interface/pcie_interface.h"
#endif

#ifdef IVI_WITH_VPS
    #include "interface/vps_interface.h"
#endif

// ===============================================================================
// 项目内部头文件
// ===============================================================================
#include "IviATTR.h"
#include "IviVAL.h"
#include "version.h"

// ===============================================================================
// 数学常量定义
// ===============================================================================
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// ===============================================================================
// 状态码和错误码定义
// ===============================================================================

// 基本状态码
#define VI_STATE_SUCCESS                        (0)
#define VI_STATE_FAIL                           (1)
#define VI_STATE_NULL                           (0)
#define VI_STATE_TRUE                           (1)
#define VI_STATE_FALSE                          (0)

// 错误码定义
#define VI_TIMEOUT                              (3)
#define VI_ERROR_MAX_TIME_EXCEEDED              (4)
#define VI_ERROR_VALUE_OUTPUT_RANGE             (5)
#define VI_ERROR_ATTR_NO_SCOPE                  (6)
#define VI_ERROR_ATTR_MODULE                    (7)
#define VI_ERROR_ATTR_DATA_TYPE                 (8)
#define VI_ERROR_ATTR_SYSTEM_STOP               (9)
#define VI_ERROR_DAQ_FATCH_OVERTIME             (10)
#define VI_ERROR_SAT_REPEAT_CONNECTION          (11)
#define VI_ERROR_RESOURCEDB_NOT_FIND            (12)
#define VI_ERROR_RESOURCEDB_FORMAT_ERROR        (13)
#define VI_ERROR_CS_CR_CONNECTION_FAILED        (14)
#define VI_ERROR_DS_CONNECTION_FAILED           (15)
#define VI_ERROR_JSON_INITIALIZE_FAIL           (16)
#define VI_ERROR_JSON_NOT_FIND                  (17)
#define VI_ERROR_JSON_FORMAT_ERROR              (18)
#define VI_ERROR_VALUE_GET_FAIL                 (19)
#define VI_ERROR_VALUE_SET_FAIL                 (20)
#define VI_ERROR_NO_SUCH_OPERATION              (21)
#define VI_ERROR_ACT_NOT_EXIST                  (22)
#define VI_ERROR_SEQ_FAIL                       (23)
#define VI_ERROR_CHANNEL_ERROR                  (24)
#define VI_ERROR_SEQ_FIND_ACT_FAIL              (25)
#define VI_ERROR_SEQ_EMPTY                      (26)
#define VI_ERROR_ATTR_NOT_SEQ                   (27)
#define VI_ERROR_ATTR_NOT_EXIST                 (28)
#define VI_ERROR_AWG_SEND_OVERTIME              (29)
#define VI_ERROR_RECV_STREAM_OVERTIME           (30)
#define VI_ERROR_SEND_STREAM_OVERTIME           (31)
#define VI_ERROR_RFCONFIG_FAIL                  (32)
#define VI_ERROR_TIMEOUT                        (33)

// 通用错误码
#define VI_ERROR_GRT_PARAM_ERROR                (-99999)
#define VI_ERROR_NO_DATA_AVAILABLE              (-99999)
#define VI_ERROR_DATA_NOT_READY                 (-99999)
#define VI_DEBUG_ENABLED                        (-99999)
#define VI_ERROR_RESOURCE_BUSY                  (-99999)

// ===============================================================================
// 兼容性宏定义
// ===============================================================================
#define VISAFN              _VI_FUNC
#define ViPtr               _VI_PTR

// 状态检查宏
#define isSUCCESS(error) if ((error) == VI_STATE_SUCCESS) return VI_STATE_SUCCESS
#define isFAIL(error) if ((error) != VI_STATE_SUCCESS) return error

// ===============================================================================
// 类型定义
// ===============================================================================

// 基本整数类型
typedef uint8_t             ViUInt8;
typedef int8_t              ViInt8;
typedef uint16_t            ViUInt16;
typedef int16_t             ViInt16;
typedef uint32_t            ViUInt32;
typedef int32_t             ViInt32;
typedef uint64_t            ViUInt64;
typedef int64_t             ViInt64;

// 字符和字符串类型
typedef char                ViChar;
typedef char*               ViCharMem;
typedef const ViChar*       ViConstString;
typedef std::string         ViString;

// 浮点类型
typedef float               ViReal32;
typedef double              ViReal64;

// 复数类型
typedef std::complex<double> ViCPReal128;
typedef std::complex<float>  ViCPReal64;

// 其他类型
typedef unsigned char       ViByte;
typedef void                ViAddr;
typedef ViByte              ViBuf;
typedef bool                ViBoolean;
typedef ViInt32             ViStatus;
typedef ViUInt32            ViVersion;
typedef ViUInt32            ViObject;
typedef ViUInt32            ViAttr;
typedef void*               ViMem;

#endif // IVI_TYPE_H
