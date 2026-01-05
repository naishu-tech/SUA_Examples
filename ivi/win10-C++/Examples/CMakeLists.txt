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
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # Use installed IVI libraries
    include_directories("C:/Program Files/IVI Foundation/IVI/RIGOL/include")
    link_directories("C:/Program Files/IVI Foundation/IVI/RIGOL/lib")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # Use installed IVI libraries
    include_directories("C:/Program Files/IVI Foundation/IVI/RIGOL/include")
    link_directories("C:/Program Files/IVI Foundation/IVI/RIGOL/lib")
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
# 两收两发 测试
# ===============================================================================
add_simple_test(testCombined "AWG_DAQ_Combined.cpp" "tool_config.h")
add_simple_test(startCombined "AWG_DAQ_Start.cpp" "tool_config.h")
add_simple_test(stopCombined "AWG_DAQ_Stop.cpp" "tool_config.h")

# ===============================================================================
# 板卡信息查询 测试
# ===============================================================================
add_simple_test(getBoardInfo "getBoardInfo.cpp" "tool_config.h")