//
// Created by sn06129 on 2025/7/25.
//

// C++ Standard Library
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <thread>
#include <mutex>
#include <cmath>
#include <filesystem>

// C Standard Library
#include <cstring>
#include <cstdio>
#include <cstdlib>

// Third-party Libraries
#include "deps/ivi/IviFgen.h"
#include "deps/ivi/IviSUATools.h"

// Platform-specific Headers
#ifdef _WIN32
#include <windows.h>
#endif

#define string2int(channelName, numChannel, m){     \
    char *end;                                      \
    numChannel = std::strtol(channelName, &end, m);}\

// Mathematical constants
#define PI 3.14159265358979323846
#define TWO_PI (2.0 * PI)

// 配置 Python 路径
// 在 IVI 库初始化 Python 之前设置环境变量
void configure_python_paths(const std::string& default_python_home) {

    // 检查 PYTHONHOME 环境变量
    char* python_home_env = std::getenv("PYTHONHOME");
    if (python_home_env == nullptr) {
        std::string env_var = "PYTHONHOME=" + default_python_home;
        _putenv(env_var.c_str());
        std::cout << "Setting PYTHONHOME=" << default_python_home
                  << " (if incorrect, set PYTHONHOME environment variable)" << std::endl;
    } else {
        std::cout << "PYTHONHOME already set: " << python_home_env << std::endl;
    }

    // 配置 PYTHONPATH
    char* python_path_env = std::getenv("PYTHONPATH");
    if (python_path_env == nullptr) {
        // 重新获取 PYTHONHOME（可能在上面已经设置）
        char* current_python_home = std::getenv("PYTHONHOME");
        std::string python_home = (current_python_home != nullptr) ? current_python_home : default_python_home;
        std::string python_path = python_home + "\\Lib";
        std::string env_var = "PYTHONPATH=" + python_path;
        _putenv(env_var.c_str());
        std::cout << "Setting PYTHONPATH=" << python_path << std::endl;
    } else {
        std::cout << "PYTHONPATH already set: " << python_path_env << std::endl;
    }
}



ViStatus triggerConfigAWG(iviFgen_ViSession *vi,  ViUInt32 triggerSource){
    auto s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_TRIGGER_SOURCE, triggerSource);
    s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_TRIGGER_SOURCE_EXE, 0);

    ViUInt32 result = 0;
    s = IviFgen_GetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_TRIGGER_SOURCE_RESULT, &result);
    std::cout << "triggerConfigAWG result is " << result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus internalTriggerConfigAWG(iviFgen_ViSession *vi, ViUInt32 triggerPulseWidth, ViUInt32 triggerRepeatSize, ViUInt32 triggerCycle, ViUInt32 triggerDelay){
    auto s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_PULSE_WIDTH, triggerPulseWidth);
    s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_REPEAT_SIZE, triggerRepeatSize);
    s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_PERIOD, triggerCycle);
    s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_DELAY, triggerDelay);
    s = IviFgen_SetAttributeViInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_EXE, 0);

    ViUInt32 result = 0;
    s = IviFgen_GetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_INTERNAL_TRIGGER_RESULT, &result);
    std::cout << "internalTriggerConfigAWG result is "<< result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }

}

ViString create_nswave_code(const std::map<ViString, waveformHandle *>& waveformHandle_map ,ViInt32 nswaveType, ViUInt32 triggerPeriod = 6400){
    ViString nsqc = R"(@nw.kernel
def program(wlist: dict[str, np.ndarray]):
)";

    int i = 0;
    ViString seg = "seg0";

    switch (nswaveType) {
        case 1:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }

            for (int j = 0; j < i; j++) {
                nsqc += "    nw.wait_for_trigger_with_source(1)  # Wait for trigger\n";
                nsqc += "    nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
        case 2:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }

            nsqc += "    while True:\n";

            for (int j = 0; j < i; j++) {
                nsqc += "        nw.wait_for_trigger_with_source(1)  # Wait for trigger\n";
                nsqc += "        nw.wait(0.001)  # Delay one-tenth of the cycle\n";
                nsqc += "        nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
        case 3:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }
            nsqc += "    i:nw.Reg\n";
            for (int j = 0; j < i; j++) {
                nsqc += "    for i in nw.loop(10):  # Loop iteration\n";
                nsqc += "        nw.wait_for_trigger_with_source(1)  # Wait for trigger\n";
                nsqc += "        nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
        case 4:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }
            nsqc += "    i:nw.Reg\n";
            nsqc += "    while True:\n";

            for (int j = 0; j < i; j++) {
                nsqc += "        for i in nw.loop(10):  # Loop iteration\n";
                nsqc += "            nw.wait_for_trigger_with_source(1)  # Wait for trigger\n";
                nsqc += "            nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
        default:
            for (const auto& pair : waveformHandle_map) {
                seg = pair.first;
                nsqc += "    seg" + std::to_string(i) + ": nw.ArbWave = nw.init_arbwave(wlist, '" + seg + "')\n";
                i++;
            }

            nsqc += "    while True:\n";

            for (int j = 0; j < i; j++) {
                nsqc += "        nw.play_arb(seg" + std::to_string(j) + ")  # Play segment\n";
            }
            break;
    }

    nsqc += "    return nw.Kernel()\n";
    
    return nsqc;
}

ViStatus sampleConfigDAQ(iviDigitizer_ViSession *vi, ViConstString sampleEnableChannel, ViInt32 sampleEnable,
                     ViConstString sampleStrageDepthChannel, ViUInt32 sampleStrageDepth,
                     ViConstString sampleLenPreChannel, ViUInt32 sampleLenPre,
                     ViConstString sampleTimesChannel, ViUInt32 sampleTimes,
                     ViConstString sampleLogicalExtractionMultipleChannel, ViUInt32 sampleLogicalExtractionMultiple,
                     ViConstString sampleCollectDataTruncationChannel, ViUInt32 sampleCollectDataTruncation,
                     ViConstString sampleCollectDataTypeChannel, ViUInt32 sampleCollectDataType){

    auto s = IviDigitizer_SetAttributeViUInt32(vi, sampleEnableChannel, IVIDIGITIZER_ATTR_SAMPLE_ENABLE, sampleEnable);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleStrageDepthChannel, IVIDIGITIZER_ATTR_SAMPLE_STRAGE_DEPTH, sampleStrageDepth);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleLenPreChannel, IVIDIGITIZER_ATTR_SAMPLE_POINT_PRE, sampleLenPre);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleTimesChannel, IVIDIGITIZER_ATTR_SAMPLE_TIMES, sampleTimes);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleLogicalExtractionMultipleChannel, IVIDIGITIZER_ATTR_SAMPLE_LOGICAL_EXTRACTION_MULTIPLE, sampleLogicalExtractionMultiple);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleCollectDataTruncationChannel, IVIDIGITIZER_ATTR_SAMPLE_COLLECT_DATA_TRUNCATION, sampleCollectDataTruncation);
    s = IviDigitizer_SetAttributeViUInt32(vi, sampleCollectDataTypeChannel, IVIDIGITIZER_ATTR_SAMPLE_COLLECT_DATA_TYPE, sampleCollectDataType);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_SAMPLE_CONFIG_EXECUTE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_SAMPLE_CONFIG_RESULT, &result);
    std::cout << "sampleConfigDAQ result is " << result << std::endl;

    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus triggerConfigDAQ(iviDigitizer_ViSession *vi, ViUInt32 triggerSource){
    auto s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_TRIGGER_SOURCE, triggerSource);
    s = IviDigitizer_SetAttributeViInt32(vi, "0", IVIDIGITIZER_ATTR_TRIGGER_CONFIG_EXECUTE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_TRIGGER_SOURCE_RESULT, &result);
    std::cout << "triggerConfigDAQ result is " << result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}


ViStatus internaltriggerConfigDAQ(iviDigitizer_ViSession *vi, ViUInt32 triggerPulseWidth, ViUInt32 triggerRepeatSize, ViUInt32 triggerCycle, ViUInt32 triggerDelay, ViUInt32 triggerEdgeType, ViUInt32 triggerHoldOffTime, ViConstString channelName, ViUInt32 triggerEdgeSet){
    auto s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_PULSE_WIDTH, triggerPulseWidth);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_REPEAT_SIZE, triggerRepeatSize);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_PERIOD, triggerCycle);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_DELAY, triggerDelay);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_TYPE, triggerEdgeType);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_HOLDOFF_TIME, triggerHoldOffTime);
    s = IviDigitizer_SetAttributeViUInt32(vi, channelName, IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET, triggerEdgeSet);

    s = IviDigitizer_SetAttributeViInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_CONFIG_TRIGGER_EXECUTE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_RESULT, &result);
    std::cout << "internaltriggerConfigDAQ IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_RESULT is " << result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

// String utility functions
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string temp;
    while (std::getline(ss, temp, delimiter)) {
        result.push_back(temp);
    }
    return result;
}

// Smart number parsing: supports hexadecimal (0x prefix) and decimal (including negative numbers)
long parseNumber(const char* str) {
    std::string s(str);
    if (s.length() >= 2 && s.substr(0, 2) == "0x") {
        // Hexadecimal parsing
        return std::strtol(str, nullptr, 16);
    } else {
        // Decimal parsing (supports negative numbers)
        return std::strtol(str, nullptr, 10);
    }
}

#ifdef _WIN32
// Plot window class - each instance has its own canvas
class PlotWindow {
private:
    // Data structure for plotting
    struct PlotData {
        std::vector<ViInt16> data;
        ViInt16 minVal{};
        ViInt16 maxVal{};
        size_t numSamples{};
        std::string fileName;
    };
    
    PlotData m_plotData;
    HWND m_hwnd;
    static bool s_classRegistered;

    // Static window procedure - routes to instance method
    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        PlotWindow* pThis = nullptr;
        
        if (uMsg == WM_NCCREATE) {
            auto* pCreate = (CREATESTRUCT*)lParam;
            pThis = (PlotWindow*)pCreate->lpCreateParams;
            if (pThis) {
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
                return TRUE;  // Allow window creation
            }
            return FALSE;  // Prevent window creation if pThis is null
        } else {
            pThis = (PlotWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        
        if (pThis) {
            return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    // Instance window procedure
    LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            
            // Clear background
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            
            if (m_plotData.numSamples > 0) {
            // Set up drawing area with margins
            int marginLeft = 80;
            int marginRight = 20;
            int marginTop = 40;
            int marginBottom = 60;
            int plotWidth = width - marginLeft - marginRight;
            int plotHeight = height - marginTop - marginBottom;
            
            // Draw axes
            HPEN axisPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            HPEN oldPen = (HPEN)SelectObject(hdc, axisPen);
            
            // Y-axis
            MoveToEx(hdc, marginLeft, marginTop, nullptr);
            LineTo(hdc, marginLeft, height - marginBottom);
            
            // X-axis
            MoveToEx(hdc, marginLeft, height - marginBottom, nullptr);
            LineTo(hdc, width - marginRight, height - marginBottom);
            
            // Draw grid lines
            HPEN gridPen = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
            SelectObject(hdc, gridPen);
            
            // Horizontal grid lines
            for (int i = 0; i <= 10; ++i) {
                int y = marginTop + (plotHeight * i / 10);
                MoveToEx(hdc, marginLeft, y, nullptr);
                LineTo(hdc, width - marginRight, y);
            }
            
            // Vertical grid lines
            for (int i = 0; i <= 10; ++i) {
                int x = marginLeft + (plotWidth * i / 10);
                MoveToEx(hdc, x, marginTop, nullptr);
                LineTo(hdc, x, height - marginBottom);
            }
            
            // Draw waveform
            HPEN wavePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
            SelectObject(hdc, wavePen);
            
            auto range = static_cast<double>(m_plotData.maxVal - m_plotData.minVal);
            if (range == 0) range = 1.0;
            
            size_t sampleStep = std::max(static_cast<size_t>(1), m_plotData.numSamples / plotWidth);
            bool firstPoint = true;
            
            for (size_t i = 0; i < m_plotData.numSamples; i += sampleStep) {
                double normalized = (static_cast<double>(m_plotData.data[i]) - m_plotData.minVal) / range;
                    int x = marginLeft + static_cast<int>((static_cast<double>(i) / m_plotData.numSamples) * plotWidth);
                int y = height - marginBottom - static_cast<int>(normalized * plotHeight);
                
                if (firstPoint) {
                    MoveToEx(hdc, x, y, nullptr);
                    firstPoint = false;
                } else {
                    LineTo(hdc, x, y);
                }
            }
            
            // Draw axis labels
            SetTextColor(hdc, RGB(0, 0, 0));
            SetBkMode(hdc, TRANSPARENT);
            
                // Y-axis labels
                char label[32];
                for (int i = 0; i <= 10; ++i) {
                    double value = m_plotData.maxVal - (static_cast<double>(i) / 10.0) * range;
                    int y = marginTop + (plotHeight * i / 10);
                    sprintf_s(label, "%.0f", value);
                    TextOut(hdc, 10, y - 8, label, static_cast<int>(strlen(label)));
                }
                
                // X-axis labels
                for (int i = 0; i <= 10; ++i) {
                    size_t sampleIndex = (m_plotData.numSamples * i / 10);
                    int x = marginLeft + (plotWidth * i / 10);
                    sprintf_s(label, "%zu", sampleIndex);
                    TextOut(hdc, x - 15, height - marginBottom + 10, label, static_cast<int>(strlen(label)));
                }
                
                // Title
                std::string title = "Waveform: " + m_plotData.fileName;
                TextOut(hdc, marginLeft, 10, title.c_str(), static_cast<int>(title.length()));
                
                // Statistics
                double sum = 0.0;
                for (const auto& val : m_plotData.data) {
                    sum += static_cast<double>(val);
                }
                double mean = sum / m_plotData.numSamples;
                
                char stats[256];
                sprintf_s(stats, "Min: %d  Max: %d  Mean: %.2f  Samples: %zu", 
                         m_plotData.minVal, m_plotData.maxVal, mean, m_plotData.numSamples);
            TextOut(hdc, marginLeft, height - 20, stats, static_cast<int>(strlen(stats)));
            
            // Cleanup
            SelectObject(hdc, oldPen);
            DeleteObject(axisPen);
            DeleteObject(gridPen);
            DeleteObject(wavePen);
        }
        
        EndPaint(hwnd, &ps);
        return 0;
        }
        case WM_DESTROY:
            m_hwnd = nullptr;
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    
    static void RegisterWindowClass() {
        if (!s_classRegistered) {
            const char* className = "PlotWindowClass";
            
            // Check if class already exists
            WNDCLASS wc = {};
            if (GetClassInfo(GetModuleHandle(nullptr), className, &wc)) {
                s_classRegistered = true;
                return;
            }
            
            wc.lpfnWndProc = StaticWindowProc;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.lpszClassName = className;
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            
            ATOM atom = RegisterClass(&wc);
            if (atom == 0) {
                DWORD error = GetLastError();
                if (error != ERROR_CLASS_ALREADY_EXISTS) {
                    // Registration failed for a reason other than already exists
                    std::cerr << "Failed to register window class. Error: " << error << std::endl;
                    return;
                }
            }
            s_classRegistered = true;
        }
    }
    
public:
    PlotWindow() : m_hwnd(nullptr) {
        m_plotData.minVal = 0;
        m_plotData.maxVal = 0;
        m_plotData.numSamples = 0;
    }
    
    ~PlotWindow() {
        if (m_hwnd != nullptr && IsWindow(m_hwnd)) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
    }

    // Create and show plot window
    void Create(const std::vector<ViInt16>& data, ViInt16 minVal, ViInt16 maxVal, 
                size_t numSamples, const std::string& fileName) {
        // Store data
        m_plotData.data = data;
        m_plotData.minVal = minVal;
        m_plotData.maxVal = maxVal;
        m_plotData.numSamples = numSamples;
        m_plotData.fileName = fileName;
        
        // Check if window already exists
        if (m_hwnd != nullptr && IsWindow(m_hwnd)) {
            // Window exists, update data and refresh the canvas
            SetWindowText(m_hwnd, ("Waveform Plot - " + fileName).c_str());
            InvalidateRect(m_hwnd, nullptr, TRUE);  // Force complete redraw
            UpdateWindow(m_hwnd);
            
            // Process pending messages non-blocking
            MSG msg = {};
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            return;
        }
        
        // Register window class
        RegisterWindowClass();
        const char* className = "PlotWindowClass";
        
        // Create window
        m_hwnd = CreateWindowEx(
            0,
            className,
            ("Waveform Plot - " + fileName).c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            1000, 700,
            nullptr, nullptr,
            GetModuleHandle(nullptr),
            this  // Pass 'this' pointer as creation parameter
        );
        
        if (m_hwnd) {
            ShowWindow(m_hwnd, SW_SHOW);
            UpdateWindow(m_hwnd);
            
            // Process initial messages to ensure window is fully created
            MSG msg = {};
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            DWORD error = GetLastError();
            std::cerr << "Failed to create plot window. Error: " << error << std::endl;
        }
    }
    
    // Update data and refresh canvas
    void UpdateData(const std::vector<ViInt16>& data, ViInt16 minVal, ViInt16 maxVal, 
                    size_t numSamples, const std::string& fileName) {
        // Update data
        m_plotData.data = data;
        m_plotData.minVal = minVal;
        m_plotData.maxVal = maxVal;
        m_plotData.numSamples = numSamples;
        m_plotData.fileName = fileName;
        
        // Refresh canvas if window exists
        if (m_hwnd != nullptr && IsWindow(m_hwnd)) {
            SetWindowText(m_hwnd, ("Waveform Plot - " + fileName).c_str());
            InvalidateRect(m_hwnd, nullptr, TRUE);  // Force complete redraw
            UpdateWindow(m_hwnd);
            
            // Process pending messages non-blocking
            MSG msg = {};
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    
    // Refresh canvas with existing data
    void Refresh() {
        if (m_hwnd != nullptr && IsWindow(m_hwnd)) {
            InvalidateRect(m_hwnd, nullptr, TRUE);
            UpdateWindow(m_hwnd);
            
            // Process pending messages non-blocking
            MSG msg = {};
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    
    // Check if window is valid
    bool IsValid() const {
        return m_hwnd != nullptr && IsWindow(m_hwnd);
    }
    
    // Get window handle
    HWND GetHandle() const {
        return m_hwnd;
    }
    
    // Process messages (non-blocking)
    void ProcessMessages() {
        MSG msg = {};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
};

// Static member initialization
bool PlotWindow::s_classRegistered = false;

// Convenience function for backward compatibility
void CreatePlotWindow(const std::vector<ViInt16>& data, ViInt16 minVal, ViInt16 maxVal, 
                     size_t numSamples, const std::string& fileName) {
    static PlotWindow defaultWindow;
    defaultWindow.Create(data, minVal, maxVal, numSamples, fileName);
    
    // Message loop (only for new window creation)
    if (defaultWindow.IsValid()) {
        MSG msg = {};
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}
#endif

void PlotDataFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filePath << std::endl;
        return;
    }

    // Read only first 2048+32 = 2080 bytes (1040 int16 samples)
    const size_t readBytes = 2048+32;  // 2080 bytes
    const size_t numSamples = readBytes / sizeof(ViInt16);  // 1040 samples
    
    std::cout << "\n=== Reading Data File ===" << std::endl;
    std::cout << "File: " << filePath << std::endl;
    std::cout << "Reading first " << readBytes << " bytes (" << numSamples << " int16 samples)" << std::endl;

    // Read data (only first 2080 bytes)
    std::vector<ViInt16> allData(numSamples);
    if (!file.read(reinterpret_cast<char*>(allData.data()), readBytes)) {
        std::cerr << "Error: Failed to read file data" << std::endl;
        file.close();
        return;
    }
    file.close();

    // Skip first 32 bytes (16 int16 samples) for plotting
    const size_t skipBytes = 32;
    const size_t skipSamples = skipBytes / sizeof(ViInt16);  // 16 samples
    const size_t plotSamples = numSamples - skipSamples;     // 1024 samples
    
    // Create data vector without first 32 bytes
    std::vector<ViInt16> data(allData.begin() + skipSamples, allData.end());
    
    std::cout << "Skipping first " << skipBytes << " bytes (" << skipSamples << " samples)" << std::endl;
    std::cout << "Plotting " << plotSamples << " samples" << std::endl;

    // Find min and max values for scaling
    ViInt16 minVal = *std::min_element(data.begin(), data.end());
    ViInt16 maxVal = *std::max_element(data.begin(), data.end());
    std::cout << "Min value: " << minVal << std::endl;
    std::cout << "Max value: " << maxVal << std::endl;

#ifdef _WIN32
    // Extract filename from path for display
    std::string fileName = filePath;
    size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        fileName = filePath.substr(lastSlash + 1);
    }
    
#else
    std::cout << "\n=== Plot Window not available on this platform ===" << std::endl;
    std::cout << "CSV file has been generated for plotting with external tools." << std::endl;
#endif

    // Print statistics (using data without first 32 bytes)
    double sum = 0.0;
    for (const auto& val : data) {
        sum += static_cast<double>(val);
    }
    double mean = sum / plotSamples;

    double variance = 0.0;
    for (const auto& val : data) {
        variance += (static_cast<double>(val) - mean) * (static_cast<double>(val) - mean);
    }
    double stdDev = std::sqrt(variance / plotSamples);

    std::cout << "\n=== Statistics ===" << std::endl;
    std::cout << "Mean: " << std::fixed << std::setprecision(2) << mean << std::endl;
    std::cout << "Std Dev: " << std::fixed << std::setprecision(2) << stdDev << std::endl;
    std::cout << "Min: " << minVal << std::endl;
    std::cout << "Max: " << maxVal << std::endl;
    std::cout << "Range: " << (maxVal - minVal) << std::endl;
}

#ifdef _WIN32
// Real-time plot data structure
struct RealTimePlotData {
    std::vector<ViInt16> data;
    ViInt16 minVal{};
    ViInt16 maxVal{};
    size_t numSamples{};
    std::string channelName;
    std::mutex dataMutex;
    bool dataUpdated = false;
};

static bool g_windowClassRegistered = false;

// Window procedure for real-time plotting
LRESULT CALLBACK RealTimePlotWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Get plot data from window user data
    auto plotData = reinterpret_cast<RealTimePlotData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            // Clear background
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));

            if (plotData) {
                std::lock_guard<std::mutex> lock(plotData->dataMutex);
                if (plotData->numSamples > 0) {
                    // Set up drawing area with margins
                    int marginLeft = 80;
                    int marginRight = 20;
                    int marginTop = 40;
                    int marginBottom = 60;
                    int plotWidth = width - marginLeft - marginRight;
                    int plotHeight = height - marginTop - marginBottom;

                    // Draw axes
                    HPEN axisPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                    HPEN oldPen = (HPEN)SelectObject(hdc, axisPen);

                    // Y-axis
                    MoveToEx(hdc, marginLeft, marginTop, nullptr);
                    LineTo(hdc, marginLeft, height - marginBottom);

                    // X-axis
                    MoveToEx(hdc, marginLeft, height - marginBottom, nullptr);
                    LineTo(hdc, width - marginRight, height - marginBottom);

                    // Draw grid lines
                    HPEN gridPen = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
                    SelectObject(hdc, gridPen);

                    // Horizontal grid lines
                    for (int i = 0; i <= 10; ++i) {
                        int y = marginTop + (plotHeight * i / 10);
                        MoveToEx(hdc, marginLeft, y, nullptr);
                        LineTo(hdc, width - marginRight, y);
                    }

                    // Vertical grid lines
                    for (int i = 0; i <= 10; ++i) {
                        int x = marginLeft + (plotWidth * i / 10);
                        MoveToEx(hdc, x, marginTop, nullptr);
                        LineTo(hdc, x, height - marginBottom);
                    }

                    // Draw waveform
                    HPEN wavePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
                    SelectObject(hdc, wavePen);

                    auto range = static_cast<double>(plotData->maxVal - plotData->minVal);
                    if (range == 0) range = 1.0;

                    // Skip first 32 bytes (16 samples) for plotting
                    const size_t skipSamples = 16;
                    size_t plotSamples = plotData->numSamples - skipSamples;
                    if (plotSamples > plotData->data.size()) {
                        plotSamples = plotData->data.size() - skipSamples;
                    }

                    size_t sampleStep = std::max(static_cast<size_t>(1), plotSamples / plotWidth);
                    bool firstPoint = true;

                    for (size_t i = skipSamples; i < skipSamples + plotSamples; i += sampleStep) {
                        if (i >= plotData->data.size()) break;
                        double normalized = (static_cast<double>(plotData->data[i]) - plotData->minVal) / range;
                        int x = marginLeft + static_cast<int>((static_cast<double>(i - skipSamples) / plotSamples) * plotWidth);
                        int y = height - marginBottom - static_cast<int>(normalized * plotHeight);

                        if (firstPoint) {
                            MoveToEx(hdc, x, y, nullptr);
                            firstPoint = false;
                        } else {
                            LineTo(hdc, x, y);
                        }
                    }

                    // Draw axis labels
                    SetTextColor(hdc, RGB(0, 0, 0));
                    SetBkMode(hdc, TRANSPARENT);

                    // Y-axis labels
                    char label[32];
                    for (int i = 0; i <= 10; ++i) {
                        double value = plotData->maxVal - (static_cast<double>(i) / 10.0) * range;
                        int y = marginTop + (plotHeight * i / 10);
                        sprintf_s(label, "%.0f", value);
                        TextOut(hdc, 10, y - 8, label, static_cast<int>(strlen(label)));
                    }

                    // X-axis labels
                    for (int i = 0; i <= 10; ++i) {
                        size_t sampleIndex = (plotSamples * i / 10);
                        int x = marginLeft + (plotWidth * i / 10);
                        sprintf_s(label, "%zu", sampleIndex);
                        TextOut(hdc, x - 15, height - marginBottom + 10, label, static_cast<int>(strlen(label)));
                    }

                    // Title
                    std::string title = "Real-time Waveform - Channel " + plotData->channelName;
                    TextOut(hdc, marginLeft, 10, title.c_str(), static_cast<int>(title.length()));

                    // Statistics
                    double sum = 0.0;
                    for (size_t i = skipSamples; i < skipSamples + plotSamples && i < plotData->data.size(); ++i) {
                        sum += static_cast<double>(plotData->data[i]);
                    }
                    double mean = plotSamples > 0 ? sum / plotSamples : 0.0;

                    char stats[256];
                    sprintf_s(stats, "Min: %d  Max: %d  Mean: %.2f  Samples: %zu",
                              plotData->minVal, plotData->maxVal, mean, plotSamples);
                    TextOut(hdc, marginLeft, height - 20, stats, static_cast<int>(strlen(stats)));

                    // Cleanup
                    SelectObject(hdc, oldPen);
                    DeleteObject(axisPen);
                    DeleteObject(gridPen);
                    DeleteObject(wavePen);
                }
            }

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY: {
            // Clean up plot data
            if (plotData) {
                delete plotData;
                SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            }
            PostQuitMessage(0);
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// Window message loop thread for a specific window
void WindowMessageLoop(HWND hwnd) {
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Create a new real-time plot window
HWND CreateRealTimePlotWindow(const std::string& channelName, RealTimePlotData* plotData) {
    // Register window class (only once)
    if (!g_windowClassRegistered) {
        const char* className = "RealTimePlotWindowClass";
        WNDCLASS wc = {};
        wc.lpfnWndProc = RealTimePlotWindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        
        RegisterClass(&wc);
        g_windowClassRegistered = true;
    }

    // Generate unique window title with timestamp
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%H%M%S", localTime);
    
    std::string windowTitle = "Real-time Waveform - Channel " + channelName + " - " + timeStr;

    // Create window
    HWND hwnd = CreateWindowEx(
            0,
            "RealTimePlotWindowClass",
            windowTitle.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            1000, 700,
            nullptr, nullptr,
            GetModuleHandle(nullptr),
            nullptr
    );

    if (hwnd) {
        // Store plot data in window user data
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(plotData));
        
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        // Start message loop in separate thread for this window
        std::thread windowThread(WindowMessageLoop, hwnd);
        windowThread.detach(); // Detach thread so it runs independently
    }
    
    return hwnd;
}

// Create a new plot window with data (non-blocking)
void UpdateRealTimePlot(const void* memDataHandle, size_t dataSizeBytes, const std::string& channelName) {
    // Calculate number of samples
    size_t numSamples = dataSizeBytes / sizeof(ViInt16);
    if (numSamples == 0) return;

    // Read only first 2048+32 bytes (1040 samples)
    const size_t readBytes = 2048 + 32;
    size_t readSamples = std::min(numSamples, static_cast<size_t>(readBytes / sizeof(ViInt16)));

    // Copy data
    const auto srcData = reinterpret_cast<const ViInt16*>(memDataHandle);
    std::vector<ViInt16> allData(srcData, srcData + readSamples);

    // Skip first 32 bytes (16 samples) for plotting
    const size_t skipSamples = 16;
    if (readSamples <= skipSamples) return;

    size_t plotSamples = readSamples - skipSamples;
    std::vector<ViInt16> plotData(allData.begin() + skipSamples, allData.end());

    // Find min and max
    if (plotData.empty()) return;
    ViInt16 minVal = *std::min_element(plotData.begin(), plotData.end());
    ViInt16 maxVal = *std::max_element(plotData.begin(), plotData.end());

    // Create new plot data structure
    auto newPlotData = new RealTimePlotData;
    newPlotData->data = plotData;
    newPlotData->minVal = minVal;
    newPlotData->maxVal = maxVal;
    newPlotData->numSamples = plotSamples;
    newPlotData->channelName = channelName;
    newPlotData->dataUpdated = true;

    // Create a new window for this plot (non-blocking)
    CreateRealTimePlotWindow(channelName, newPlotData);
}
#endif

// Helper function: Calculate Greatest Common Divisor (GCD)
ViUInt64 CalculateGCD(ViUInt64 a, ViUInt64 b) {
    while (b != 0) {
        ViUInt64 temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Helper function: Calculate Least Common Multiple (LCM)
ViUInt64 CalculateLCM(ViUInt64 a, ViUInt64 b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return (a / CalculateGCD(a, b)) * b;
}

// Calculate optimal sample count based on sampleRate and frequency
ViUInt64 CalculateOptimalSampleCount(ViReal64 sampleRate, ViReal64 frequency) {
    // Step 1: Divide sampleRate and frequency by 1000000 to get a and b
    auto a = static_cast<ViUInt64>(std::round(sampleRate / 1000000.0));
    auto b = static_cast<ViUInt64>(std::round(frequency / 1000000.0));
    
    // Step 2: Calculate LCM of a and b
    ViUInt64 n = CalculateLCM(a, b);
    
    // Step 3: Calculate LCM of n and 64
    ViUInt64 m = CalculateLCM(n, 64);
    
    // Step 4: Apply constraints
    const ViUInt64 minValue = 16384;
    const ViUInt64 maxValue = 16 * 1024 * 1024;  // 16 * 1024 * 1024
    
    if (m < minValue) {
        return minValue;
    } else if (m > maxValue) {
        return maxValue;
    } else {
        return m;
    }
}

// Structure to hold waveform generation result
struct WaveformResult {
    ViInt16* waveformData = nullptr;  // Pointer to waveform data memory
    size_t dataSize = 0;              // Number of samples (size of waveformData)
    std::string filePath;
    
    // Default constructor
    WaveformResult() = default;
    
    // Destructor to free memory
    ~WaveformResult() {
        if (waveformData != nullptr) {
            delete[] waveformData;
            waveformData = nullptr;
        }
    }
    
    // Copy constructor (deep copy)
    WaveformResult(const WaveformResult& other) : filePath(other.filePath), dataSize(other.dataSize) {
        if (other.waveformData != nullptr && other.dataSize > 0) {
            waveformData = new ViInt16[other.dataSize];
            std::memcpy(waveformData, other.waveformData, other.dataSize * sizeof(ViInt16));
        }
    }
    
    // Copy assignment operator (deep copy)
    WaveformResult& operator=(const WaveformResult& other) {
        if (this != &other) {
            // Free existing memory
            if (waveformData != nullptr) {
                delete[] waveformData;
                waveformData = nullptr;
            }
            
            // Copy new data
            filePath = other.filePath;
            dataSize = other.dataSize;
            if (other.waveformData != nullptr && other.dataSize > 0) {
                waveformData = new ViInt16[other.dataSize];
                std::memcpy(waveformData, other.waveformData, other.dataSize * sizeof(ViInt16));
            }
        }
        return *this;
    }
    
    // Move constructor
    WaveformResult(WaveformResult&& other) noexcept 
        : waveformData(other.waveformData), dataSize(other.dataSize), filePath(std::move(other.filePath)) {
        other.waveformData = nullptr;
        other.dataSize = 0;
    }
    
    // Move assignment operator
    WaveformResult& operator=(WaveformResult&& other) noexcept {
        if (this != &other) {
            // Free existing memory
            if (waveformData != nullptr) {
                delete[] waveformData;
            }
            
            // Move data
            waveformData = other.waveformData;
            dataSize = other.dataSize;
            filePath = std::move(other.filePath);
            
            // Reset source
            other.waveformData = nullptr;
            other.dataSize = 0;
        }
        return *this;
    }
};


// Structure to hold waveform generation parameters
struct WaveformParameters {
    std::string waveformType = "Sin";
    ViReal64 sampleRate = 4000000000.0;      // Hz, default 4GSps
    ViReal64 frequency = 100000000.0;         // Hz, default 100MHz
    std::string saveFilePath = "./wfm_file/";
    double offset = 0.0;                      // default 0
    double amplitude = 1.0;                   // default 1.0
    double phase = 0.0;                        // default 0
    double bandwidth = 0.0;                   // MHz, default 0
    ViReal64 maxFrequency = 200000000.0;    // Hz, default 200MHz (for Chirp)
    double dutyCycle = 0.5;                   // default 50% (for Square wave)
    double triangleRatio = 0.5;               // default 50% (for Triangle wave, rising/falling ratio)
    
//    // Default constructor
//    WaveformParameters() = default;
//
    // Parameterized constructor
    explicit WaveformParameters(std::string  type,
                      ViReal64 sr = 4000000000.0,
                      ViReal64 freq = 100000000.0,
                      std::string  path = "./wfm_file/",
                      double off = 0.0,
                      double amp = 1.0,
                      double ph = 0.0,
                      double bw = 0.0,
                      ViReal64 maxFreq = 200000000.0,
                      double duty = 0.5,
                      double triRatio = 0.5)
        : waveformType(std::move(type)), sampleRate(sr), frequency(freq),
          saveFilePath(std::move(path)), offset(off), amplitude(amp),
          phase(ph), bandwidth(bw), maxFrequency(maxFreq), 
          dutyCycle(duty), triangleRatio(triRatio) {}
    
    // Copy constructor
    WaveformParameters(const WaveformParameters& other) = default;
    
    // Move constructor
    WaveformParameters(WaveformParameters&& other) noexcept = default;
    
    // Copy assignment operator
    WaveformParameters& operator=(const WaveformParameters& other) = default;
    
    // Move assignment operator
    WaveformParameters& operator=(WaveformParameters&& other) noexcept = default;
    
    // Destructor
    ~WaveformParameters() = default;
};

// Waveform generation function
WaveformResult GenerateWaveformFile(const WaveformParameters& params) {
    // Calculate number of samples
    size_t totalSamples;
    double actualDuration;
    size_t integerCycles = 0;

    double samplesPerCycle = params.sampleRate / params.frequency;
    ViUInt64 numCycles = CalculateOptimalSampleCount(params.sampleRate, params.frequency);


    // For Chirp signal, frequency changes, so we don't need integer cycles
    if (params.waveformType == "Chirp" || params.waveformType == "chirp") {
        totalSamples = static_cast<size_t>(std::round(numCycles));
        actualDuration = static_cast<double>(totalSamples) / params.sampleRate;
    } else {
        // Calculate number of samples for integer cycles
        integerCycles = static_cast<size_t>(std::round(numCycles));

        // Recalculate duration to ensure integer cycles
        actualDuration = integerCycles / params.frequency;
        totalSamples = static_cast<size_t>(std::round(actualDuration * params.sampleRate));

        // Ensure we have at least one complete cycle
        if (totalSamples < static_cast<size_t>(samplesPerCycle)) {
            totalSamples = static_cast<size_t>(std::round(samplesPerCycle));
            integerCycles = 1;
            actualDuration = integerCycles / params.frequency;
        }
    }

    // Generate waveform data
    auto waveformData = new ViInt16[totalSamples];
    const double maxInt16 = 32767.0;

    for (size_t i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / params.sampleRate;
        double value = 0.0;

        if (params.waveformType == "Sin" || params.waveformType == "sin" || params.waveformType == "Sine") {
            // Sine wave: A * sin(2*pi*f*t + phase) + offset
            value = params.amplitude * std::sin(TWO_PI * params.frequency * t + params.phase) + params.offset;
        }
        else if (params.waveformType == "Square" || params.waveformType == "square") {
            // Square wave with duty cycle: A * sign(sin(2*pi*f*t + phase) - threshold) + offset
            // dutyCycle: 0.0 to 1.0, where 0.5 = 50%
            double phaseValue = std::fmod(TWO_PI * params.frequency * t + params.phase, TWO_PI);
            double threshold = TWO_PI * (1.0 - params.dutyCycle);
            value = params.amplitude * (phaseValue < threshold ? 1.0 : -1.0) + params.offset;
        }
        else if (params.waveformType == "Triangle" || params.waveformType == "triangle") {
            // Triangle wave: linear ramp from -A to A and back
            // triangleRatio: 0.0 to 1.0, where 0.5 = 50% rising, 50% falling
            double phaseValue = std::fmod(TWO_PI * params.frequency * t + params.phase, TWO_PI);
            double risingPhase = TWO_PI * params.triangleRatio;
            if (phaseValue < risingPhase) {
                // Rising edge: -1 to 1
                value = params.amplitude * (2.0 * phaseValue / risingPhase - 1.0) + params.offset;
            } else {
                // Falling edge: 1 to -1
                double fallingPhase = TWO_PI - risingPhase;
                double fallingPos = phaseValue - risingPhase;
                value = params.amplitude * (1.0 - 2.0 * fallingPos / fallingPhase) + params.offset;
            }
        }
        else if (params.waveformType == "Sawtooth" || params.waveformType == "sawtooth") {
            // Sawtooth wave: A * (2*(t*f - floor(t*f + 0.5)) + offset
            value = params.amplitude * 2.0 * (t * params.frequency - std::floor(t * params.frequency + 0.5)) + params.offset;
        }
        else if (params.waveformType == "Chirp" || params.waveformType == "chirp") {
            // Chirp signal: frequency increases from 'frequency' to 'maxFrequency' in first 50%,
            // then decreases from 'maxFrequency' back to 'frequency' in second 50%
            double instantaneousPhase = 0.0;
            double halfDuration = actualDuration * 0.5;
            
            if (t <= halfDuration) {
                // First half: frequency increases linearly from frequency to maxFrequency
                // f(t) = frequency + (maxFrequency - frequency) * (2*t / duration)
                // Phase = ∫ f(t) dt = frequency * t + (maxFrequency - frequency) * t^2 / duration
                double freqSlope = (params.maxFrequency - params.frequency) / halfDuration;
                instantaneousPhase = TWO_PI * (params.frequency * t + 0.5 * freqSlope * t * t) + params.phase;
            } else {
                // Second half: frequency decreases linearly from maxFrequency to frequency
                // f(t) = maxFrequency - (maxFrequency - frequency) * (2*(t - duration/2) / duration)
                // Phase at halfDuration: frequency * halfDuration + (maxFrequency - frequency) * halfDuration^2 / duration
                // = frequency * halfDuration + (maxFrequency - frequency) * halfDuration / 2
                // = halfDuration * (frequency + (maxFrequency - frequency) / 2)
                // = halfDuration * (frequency + maxFrequency) / 2
                double phaseAtHalf = TWO_PI * halfDuration * (params.frequency + params.maxFrequency) * 0.5 + params.phase;
                double tSecondHalf = t - halfDuration;
                double freqSlope = (params.maxFrequency - params.frequency) / halfDuration;
                // Phase in second half: phaseAtHalf + ∫ f(t) dt from halfDuration to t
                // = phaseAtHalf + maxFrequency * tSecondHalf - 0.5 * freqSlope * tSecondHalf^2
                instantaneousPhase = phaseAtHalf + TWO_PI * (params.maxFrequency * tSecondHalf - 0.5 * freqSlope * tSecondHalf * tSecondHalf);
            }
            value = params.amplitude * std::sin(instantaneousPhase) + params.offset;
        }
        else {
            // Default to sine wave
            value = params.amplitude * std::sin(TWO_PI * params.frequency * t + params.phase) + params.offset;
        }

        // Convert to int16 (signed 16-bit)
        // Clamp to [-1, 1] range, then scale to [-32767, 32767]
        if (value > 1.0) value = 1.0;
        if (value < -1.0) value = -1.0;
        waveformData[i] = static_cast<ViInt16>(value * maxInt16);
    }

    // Generate filename
    // Format: ./wfm/{Type}_Fixed_{SampleRate}MSps_{Offset}Offset_{Amp}Amp_{Freq}MHz_{MaxFreq}MHz_{Bandwidth}MBw_{Phase}Phase_{Duration}us_16bit_Signed_{Duration}us_1Row_1Column.dat
    std::ostringstream filename;
    filename << params.saveFilePath;
    filename << params.waveformType << "_Fixed_";
    filename << std::fixed << std::setprecision(0) << (params.sampleRate / 1e6) << "MSps_";
    filename << static_cast<int>(params.offset) << "Offset_";
    filename << static_cast<int>(params.amplitude) << "Amp_";
    filename << std::fixed << std::setprecision(0) << (params.frequency / 1e6) << "MHz_";
    if (params.waveformType == "Chirp" || params.waveformType == "chirp") {
        filename << std::fixed << std::setprecision(0) << (params.maxFrequency / 1e6) << "MaxFreq_";
    }
    filename << std::fixed << std::setprecision(0) << params.bandwidth << "MBw_";
    filename << static_cast<int>(params.phase) << "Phase_";
    filename << std::fixed << std::setprecision(0) << (actualDuration * 1e6) << "us_";
    filename << "16bit_Signed_";
    filename << std::fixed << std::setprecision(0) << (actualDuration * 1e6) << "us_";
    filename << "1Row_1Column.dat";

    std::string filePath = filename.str();

    // Create directory if it doesn't exist
    std::filesystem::path dirPath = std::filesystem::path(filePath).parent_path();
    if (!dirPath.empty() && !std::filesystem::exists(dirPath)) {
        std::filesystem::create_directories(dirPath);
    }

    // Write binary file
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Cannot create waveform file " << filePath << std::endl;
        delete[] waveformData;
        WaveformResult result;
        result.filePath = "";
        return result;
    }

    outFile.write(reinterpret_cast<const char*>(waveformData),
                  totalSamples * sizeof(ViInt16));
    outFile.close();

    std::cout << "Waveform file generated: " << filePath << std::endl;
    std::cout << "  Waveform type: " << params.waveformType << std::endl;
    std::cout << "  Sample rate: " << params.sampleRate / 1e6 << " MSps" << std::endl;
    std::cout << "  Frequency: " << params.frequency / 1e6 << " MHz" << std::endl;
    if (params.waveformType == "Chirp" || params.waveformType == "chirp") {
        std::cout << "  Max frequency: " << params.maxFrequency / 1e6 << " MHz" << std::endl;
    }
    std::cout << "  Duration: " << actualDuration * 1e6 << " us" << std::endl;
    std::cout << "  Offset: " << params.offset << std::endl;
    std::cout << "  Amplitude: " << params.amplitude << std::endl;
    std::cout << "  Phase: " << params.phase << std::endl;
    std::cout << "  Bandwidth: " << params.bandwidth << " MHz" << std::endl;
    if (integerCycles > 0) {
        std::cout << "  Integer cycles: " << integerCycles << std::endl;
    }
    std::cout << "  Total samples: " << totalSamples << std::endl;
    std::cout << "  File size: " << (totalSamples * sizeof(ViInt16)) << " bytes" << std::endl;

    WaveformResult result;
    result.waveformData = waveformData;
    result.dataSize = totalSamples;
    result.filePath = filePath;
    return result;
}