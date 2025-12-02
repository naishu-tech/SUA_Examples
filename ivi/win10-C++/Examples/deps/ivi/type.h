#ifndef IVI_TYPE_H
#define IVI_TYPE_H

// ===============================================================================
// Platform-related Macro Definitions
// ===============================================================================
#ifdef __linux__
    #define _API_CALL
    #define DLLEXTERN
    #define RIGOLLIB_API
#else  // Windows, RTX
    #define RIGOLLIB_API __declspec(dllexport)
    #define DLLEXTERN extern "C"
    #define NOMINMAX  // Prevent Windows.h from defining min/max macros
#endif

// ===============================================================================
// C Standard Library Headers
// ===============================================================================
#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstdint>

// ===============================================================================
// C++ Standard Library Headers
// ===============================================================================
// Container classes
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>

// Stream and file operations
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

// Other core functionality
#include <memory>
#include <variant>
#include <complex>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <exception>
#include <unordered_set>

// Concurrency and time
#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>

// ===============================================================================
// System-specific Headers
// ===============================================================================
#ifdef __linux__
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
#elif defined(_WIN32)
    // Windows-specific headers can be added here
    // #include <windows.h>  // If needed
#endif

// ===============================================================================
// Third-party Library Headers
// ===============================================================================
// JSON library
#include "jsoncpp/json/json.h"

// NSUKit related
#include "../nsukitcpp/NSUKit.h"
#include "../nsukitcpp/xdma_api.h"
#include "../nsukitcpp/high_level/HardwareSync.h"

// Conditionally dependent interfaces
#ifdef IVI_WITH_XDMA
    #include "interface/pcie_interface.h"
#endif

#ifdef IVI_WITH_VPS
    #include "interface/vps_interface.h"
#endif

// ===============================================================================
// Project Internal Headers
// ===============================================================================
#include "IviATTR.h"
#include "IviVAL.h"
#include "version.h"

// ===============================================================================
// Mathematical Constant Definitions
// ===============================================================================
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// ===============================================================================
// Status Code and Error Code Definitions
// ===============================================================================

// Basic status codes
#define VI_STATE_SUCCESS                        (0)
#define VI_STATE_FAIL                           (1)
#define VI_STATE_NULL                           (0)
#define VI_STATE_TRUE                           (1)
#define VI_STATE_FALSE                          (0)

// Error code definitions
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

// General error codes
#define VI_ERROR_GRT_PARAM_ERROR                (-99999)
#define VI_ERROR_NO_DATA_AVAILABLE              (-99999)
#define VI_ERROR_DATA_NOT_READY                 (-99999)
#define VI_DEBUG_ENABLED                        (-99999)
#define VI_ERROR_RESOURCE_BUSY                  (-99999)

// ===============================================================================
// Compatibility Macro Definitions
// ===============================================================================
#define VISAFN              _VI_FUNC
#define ViPtr               _VI_PTR

// Status check macros
#define isSUCCESS(error) if ((error) == VI_STATE_SUCCESS) return VI_STATE_SUCCESS
#define isFAIL(error) if ((error) != VI_STATE_SUCCESS) return error

// ===============================================================================
// Type Definitions
// ===============================================================================

// Basic integer types
typedef uint8_t             ViUInt8;
typedef int8_t              ViInt8;
typedef uint16_t            ViUInt16;
typedef int16_t             ViInt16;
typedef uint32_t            ViUInt32;
typedef int32_t             ViInt32;
typedef uint64_t            ViUInt64;
typedef int64_t             ViInt64;

// Character and string types
typedef char                ViChar;
typedef char*               ViCharMem;
typedef const ViChar*       ViConstString;
typedef std::string         ViString;

// Floating-point types
typedef float               ViReal32;
typedef double              ViReal64;

// Complex number types
typedef std::complex<double> ViCPReal128;
typedef std::complex<float>  ViCPReal64;

// Other types
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
