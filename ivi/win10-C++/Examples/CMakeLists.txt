# ===============================================================================
# IVI 测试程序 CMake 配置
# ===============================================================================
cmake_minimum_required(VERSION 3.16.3)
project(ivi_examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===============================================================================
# Python 和 pybind11 配置（可选）
# ===============================================================================
option(ENABLE_PYTHON_TESTS "Enable Python-based tests" OFF)

if(ENABLE_PYTHON_TESTS)
    # 尝试查找 Python
    find_package(Python 3.10 COMPONENTS Interpreter Development)
    
    if(Python_FOUND)
        # 尝试查找 pybind11
        find_package(pybind11 CONFIG)
        if(pybind11_FOUND)
            message(STATUS "Python and pybind11 found, enabling Python tests")
            set(PYTHON_SUPPORT_AVAILABLE TRUE)
        else()
            message(WARNING "pybind11 not found, Python tests will be disabled")
            set(PYTHON_SUPPORT_AVAILABLE FALSE)
        endif()
    else()
        message(WARNING "Python not found, Python tests will be disabled")
        set(PYTHON_SUPPORT_AVAILABLE FALSE)
    endif()
else()
    set(PYTHON_SUPPORT_AVAILABLE FALSE)
endif()

# ===============================================================================
# 包含目录配置
# ===============================================================================
include_directories(./include)
include_directories(./deps)
include_directories(./deps/ivi)
include_directories(./src)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    include_directories(./deps/nsukitcpp)
    link_directories(/lib/)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    link_directories(./lib)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # macOS-specific settings
else()
    message(STATUS "Operating System: Unknown")
    # Configuration for other systems
endif()

configure_file(version.h.in ./build/version.h)

# ===============================================================================
# 查找依赖包
# ===============================================================================
find_package(Threads REQUIRED)

# ===============================================================================
# 通用链接库列表
# ===============================================================================
set(COMMON_LIBRARIES
    ivi
    NSUKit
    xdma_api
)

set(JSONCPP_SOURCES
        ./deps/jsoncpp/lib_json/json_reader.cpp
        ./deps/jsoncpp/lib_json/json_value.cpp
        ./deps/jsoncpp/lib_json/json_writer.cpp
)

set(THREADED_LIBRARIES
    ${COMMON_LIBRARIES}
    Threads::Threads
)

set(PYTHON_LIBRARIES ${COMMON_LIBRARIES})
if(PYTHON_SUPPORT_AVAILABLE)
    list(APPEND PYTHON_LIBRARIES pybind11::embed Python::Python)
endif()

# ===============================================================================
# 辅助函数：创建简单测试程序
# ===============================================================================
function(add_simple_test target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} ${COMMON_LIBRARIES} ${THREADED_LIBRARIES})
endfunction()


# ===============================================================================
# AWG (任意波形发生器) 测试
# ===============================================================================
add_simple_test(wfmPlayAWG "AWG_wfmPlay.cpp" "tool_config.h")

# ===============================================================================
# DAQ (数据采集) 测试
# ===============================================================================
add_simple_test(ReadRingBufferDAQ "DAQ_read_RingBuffer.cpp" "tool_config.h")

# ===============================================================================
# SUATools () 测试
# ===============================================================================
add_simple_test(testCombined "AWG_DAQ_Combined.cpp")

# ===============================================================================
# RF
# =================================
add_executable(RF "RF.cpp" ${JSONCPP_SOURCES})
target_link_libraries(RF ${COMMON_LIBRARIES})

# ===============================================================================
# 构建信息输出
# ===============================================================================
message(STATUS "=== IVI Test Configuration ===")
message(STATUS "Total test programs: ~50+")
message(STATUS "Python tests option: ${ENABLE_PYTHON_TESTS}")
message(STATUS "Python support available: ${PYTHON_SUPPORT_AVAILABLE}")
message(STATUS "Threading support: ${Threads_FOUND}")
if(PYTHON_SUPPORT_AVAILABLE)
    message(STATUS "Python version: ${Python_VERSION}")
    message(STATUS "pybind11 version: ${pybind11_VERSION}")
endif()
message(STATUS "===============================")

# ===============================================================================
# 可选：创建测试组
# ===============================================================================
# 为了方便运行特定类型的测试，可以创建自定义目标

# AWG 测试组
add_custom_target(test_awg_all
    COMMENT "Build all AWG tests"
    DEPENDS
        QIPlayAWG sequencePlayAWG wfmPlayAWG
)

# DAQ 测试组
add_custom_target(test_daq_all
    COMMENT "Build all DAQ tests"
    DEPENDS
        QIRingBufferDAQ RingbufferDAQ StreamDAQ workModeSwitchDAQ
)

# SUATools 测试组
add_custom_target(test_all
        COMMENT "Build SUATools tests"
        DEPENDS
            getInfoSUATools syncSUATools syncSUATools runSUATools stopSUATools
)