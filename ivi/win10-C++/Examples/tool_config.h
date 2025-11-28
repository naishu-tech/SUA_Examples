//
// Created by sn06129 on 2025/7/25.
//
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <cmath>
#include "deps/ivi/IviFgen.h"
#include "deps/ivi/IviSUATools.h"

# include <iostream>
# include <cstdlib>
# include <vector>
# include <string>
# include <algorithm>
# include <fstream>
# include <iomanip>
# include <sstream>

#ifdef _WIN32
# include <windows.h>
#else
# include <cstdio>
# include <cstdlib>
#endif

#define string2int(channelName, numChannel, m){     \
    char *end;                                      \
    numChannel = std::strtol(channelName, &end, m);}\

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

ViStatus DUCConfigAWG(iviFgen_ViSession *vi, ViInt32 DUC_Enable, ViConstString channelName, ViReal64 DUC_NCO_Frequency){
    auto s = IviFgen_SetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_DAC_DUC_ENABLE, DUC_Enable);
    s = IviFgen_SetAttributeViReal64(vi, channelName, IVIFGEN_ATTR_DAC_NCO_FREQUENCY, DUC_NCO_Frequency);
    s = IviFgen_SetAttributeViInt32(vi, "0", IVIFGEN_ATTR_DAC_DUC_EXE, 1);
    ViUInt32 result = 0;
    s = IviFgen_GetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_DAC_DUC_RESULT, &result);
    std::cout << "IVIFGEN_ATTR_DAC_DUC_RESULT result is " << result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
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

ViStatus NYQ_ZONEAWG(iviFgen_ViSession *vi, ViConstString channelName, ViInt32 NYQ_ZONE){
    auto s = IviFgen_SetAttributeViUInt32(vi, channelName, IVIFGEN_ATTR_DAC_NYQ_ZONE, NYQ_ZONE);
    s = IviFgen_SetAttributeViInt32(vi, "0", IVIFGEN_ATTR_DAC_NYQ_ZONE_EXE, 0);

    ViUInt32 result = 0;
    s = IviFgen_GetAttributeViUInt32(vi, "0", IVIFGEN_ATTR_DAC_NYQ_ZONE_RESULT, &result);
    std::cout << "NYQ_ZONEAWG result is "<< result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus internalTriggerConfigSAT(iviSyncATrig_ViSession *vi, ViUInt32 triggerSource, ViUInt32 triggerPeriod = 6400, ViUInt32 triggerRepetSize=4294967295, ViUInt32 triggerPulseWidthy=1600){
    auto s = IviSyncATrig_SetAttributeViUInt32(vi, "0", IVISYNCATRIG_ATTR_TRIGGER_SOURCE, triggerSource);
    s = IviSyncATrig_SetAttributeViUInt32(vi, "0", IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_PERIOD, triggerPeriod);
    s = IviSyncATrig_SetAttributeViUInt32(vi, "0", IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_REPEAT_SIZE, triggerRepetSize);
    s = IviSyncATrig_SetAttributeViUInt32(vi, "0", IVISYNCATRIG_ATTR_INTERNAL_TRIGGER_PULSE_WIDTH, triggerPulseWidthy);
    ViUInt32 result = 0;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViString create_nswave_code(std::map<ViString, waveformHandle *> waveformHandle_map ,ViInt32 nswaveType, ViUInt32 triggerPeriod = 6400){
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

ViStatus DDConfigDAQ(iviDigitizer_ViSession *vi, ViInt32 DDC_Enable, ViConstString channelName, ViReal64 DDC_NCO_Frequency){

    auto s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_DDC_ENABLE, DDC_Enable);
    s = IviDigitizer_SetAttributeViReal64(vi, channelName, IVIDIGITIZER_ATTR_ADC_NCO_FREQUENCY, DDC_NCO_Frequency);
    s = IviDigitizer_SetAttributeViInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_DDC_EXE, 1);
    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_DDC_RESULT, &result);
    std::cout <<"DDConfigDAQ  result is " << result << std::endl;
    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus internalTriggerConfigDAQ(iviDigitizer_ViSession *vi, ViConstString channelName, ViInt32 triggerEdgeSet, ViUInt32 triggerPulseWidth=100000000, ViUInt32 triggerRepeatSize=4294967295, ViUInt32 triggerCycle=1000000000){

    auto s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_PULSE_WIDTH, triggerPulseWidth);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_REPEAT_SIZE, triggerRepeatSize);
    s = IviDigitizer_SetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_PERIOD, triggerCycle);
    s = IviDigitizer_SetAttributeViUInt32(vi, channelName, IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_EDGE_SET, triggerEdgeSet);
    s = IviDigitizer_SetAttributeViInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_CONFIG_TRIGGER_EXECUTE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_INTERNAL_TRIGGER_RESULT, &result);
    std::cout << "internalTriggerConfigDAQ result is " << result << std::endl;

    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
}

ViStatus NYQ_ZONEDAQ(iviDigitizer_ViSession *vi, ViConstString channelName, ViInt32 NYQ_ZONE){

    auto s = IviDigitizer_SetAttributeViUInt32(vi, channelName, IVIDIGITIZER_ATTR_ADC_NYQ_ZONE, NYQ_ZONE);
    s = IviDigitizer_SetAttributeViInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_NYQ_ZONE_EXE, 0);

    ViUInt32 result;
    s = IviDigitizer_GetAttributeViUInt32(vi, "0", IVIDIGITIZER_ATTR_ADC_NYQ_ZONE_RESULT, &result);
    std::cout << "NYQ_ZONEDAQ result is" << result << std::endl;

    if (result){
        return VI_STATE_FAIL;
    } else {
        return VI_STATE_SUCCESS;
    }
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
// Global data structure for plotting
struct PlotData {
    std::vector<ViInt16> data;
    ViInt16 minVal;
    ViInt16 maxVal;
    size_t numSamples;
    std::string fileName;
};

static PlotData g_plotData;
static HWND g_hwnd = nullptr;

// Window procedure for plotting window
LRESULT CALLBACK PlotWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
        
        if (g_plotData.numSamples > 0) {
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
            MoveToEx(hdc, marginLeft, marginTop, NULL);
            LineTo(hdc, marginLeft, height - marginBottom);
            
            // X-axis
            MoveToEx(hdc, marginLeft, height - marginBottom, NULL);
            LineTo(hdc, width - marginRight, height - marginBottom);
            
            // Draw grid lines
            HPEN gridPen = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
            SelectObject(hdc, gridPen);
            
            // Horizontal grid lines
            for (int i = 0; i <= 10; ++i) {
                int y = marginTop + (plotHeight * i / 10);
                MoveToEx(hdc, marginLeft, y, NULL);
                LineTo(hdc, width - marginRight, y);
            }
            
            // Vertical grid lines
            for (int i = 0; i <= 10; ++i) {
                int x = marginLeft + (plotWidth * i / 10);
                MoveToEx(hdc, x, marginTop, NULL);
                LineTo(hdc, x, height - marginBottom);
            }
            
            // Draw waveform
            HPEN wavePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
            SelectObject(hdc, wavePen);
            
            double range = static_cast<double>(g_plotData.maxVal - g_plotData.minVal);
            if (range == 0) range = 1.0;
            
            size_t sampleStep = std::max(static_cast<size_t>(1), g_plotData.numSamples / plotWidth);
            bool firstPoint = true;
            
            for (size_t i = 0; i < g_plotData.numSamples; i += sampleStep) {
                double normalized = (static_cast<double>(g_plotData.data[i]) - g_plotData.minVal) / range;
                int x = marginLeft + static_cast<int>((static_cast<double>(i) / g_plotData.numSamples) * plotWidth);
                int y = height - marginBottom - static_cast<int>(normalized * plotHeight);
                
                if (firstPoint) {
                    MoveToEx(hdc, x, y, NULL);
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
                double value = g_plotData.maxVal - (static_cast<double>(i) / 10.0) * range;
                int y = marginTop + (plotHeight * i / 10);
                sprintf_s(label, "%.0f", value);
                TextOut(hdc, 10, y - 8, label, static_cast<int>(strlen(label)));
            }
            
            // X-axis labels
            for (int i = 0; i <= 10; ++i) {
                size_t sampleIndex = (g_plotData.numSamples * i / 10);
                int x = marginLeft + (plotWidth * i / 10);
                sprintf_s(label, "%zu", sampleIndex);
                TextOut(hdc, x - 15, height - marginBottom + 10, label, static_cast<int>(strlen(label)));
            }
            
            // Title
            std::string title = "Waveform: " + g_plotData.fileName;
            TextOut(hdc, marginLeft, 10, title.c_str(), static_cast<int>(title.length()));
            
            // Statistics
            double sum = 0.0;
            for (const auto& val : g_plotData.data) {
                sum += static_cast<double>(val);
            }
            double mean = sum / g_plotData.numSamples;
            
            char stats[256];
            sprintf_s(stats, "Min: %d  Max: %d  Mean: %.2f  Samples: %zu", 
                     g_plotData.minVal, g_plotData.maxVal, mean, g_plotData.numSamples);
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
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        g_hwnd = nullptr;
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// Create and show plot window
void CreatePlotWindow(const std::vector<ViInt16>& data, ViInt16 minVal, ViInt16 maxVal, 
                     size_t numSamples, const std::string& fileName) {
    // Store data globally
    g_plotData.data = data;
    g_plotData.minVal = minVal;
    g_plotData.maxVal = maxVal;
    g_plotData.numSamples = numSamples;
    g_plotData.fileName = fileName;
    
    // Register window class
    const char* className = "PlotWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = PlotWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = className;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    // Create window
    g_hwnd = CreateWindowEx(
        0,
        className,
        ("Waveform Plot - " + fileName).c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 700,
        NULL, NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    if (g_hwnd) {
        ShowWindow(g_hwnd, SW_SHOW);
        UpdateWindow(g_hwnd);
        
        // Message loop
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0)) {
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
    
    std::cout << "\n=== Opening Plot Window ===" << std::endl;
    std::cout << "Close the window to continue..." << std::endl;
    
    // Create and show plot window (using data without first 32 bytes)
    CreatePlotWindow(data, minVal, maxVal, plotSamples, fileName);
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
