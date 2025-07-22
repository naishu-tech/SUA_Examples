# SUA8000 Remote Control V2

## Project Overview / 项目概述

SUA8000 Remote Control V2 is a comprehensive remote control solution for SUA8000 series test and measurement equipment. This project provides complete example code and demonstration documentation, covering various functions of AWG (Arbitrary Waveform Generator) and Digitizer.

SUA8000 Remote Control V2 是针对SUA8000系列测试测量设备的完整远程控制解决方案。本项目提供了全面的示例代码和演示文档，涵盖了AWG（任意波形发生器）和Digitizer（数字化仪）的各种功能。

## SCPI / 文件结构

```
remote control V2/
├── README.md                     # Project documentation / 项目说明文档
├── tools/                        # Tool modules / 工具模块
│   ├── BVC_Tools.ipynb           # BVC tools and utilities / BVC工具和实用程序
│   └── AWG_createWave.ipynb      # AWG waveform creation / AWG波形创建
├── Examples/                     # Example files / 示例文件
│   ├── AWG_*.ipynb               # AWG function examples / AWG功能示例
│   ├── Digitizer_*.ipynb         # Digitizer function examples / Digitizer功能示例
│   └── file/                     # Example files / 示例文件
├── demos/                        # Demo files / 演示文件
│   ├── AWG.ipynb                 # AWG demo / AWG演示
│   ├── DAQ.ipynb                 # DAQ demo / DAQ演示
│   └── string_to_array_example.py # String conversion example / 字符串转换示例
└── wave_file/                    # Waveform file directory / 波形文件目录
```

## Features / 功能特性

### AWG Functions / AWG功能
- **Waveform Generation / 波形生成**: Sine wave, multi-tone signals, square wave, pulse signals, triangle wave, linear chirp signals / 正弦波、多音信号、方波、脉冲信号、三角波、线性调频信号
- **Signal Visualization / 信号可视化**: Time domain and frequency domain analysis / 时域和频域分析
- **Waveform Upload / 波形上传**: Upload generated waveforms to AWG device / 将生成的波形上传到AWG设备
- **NSWave Configuration / NSWave配置**: Create and compile NSWave sequences / 创建和编译NSWave序列
- **Device Control / 设备控制**: Start and stop AWG playback / 启动和停止AWG播放
- **DUC Function / DUC功能**: Digital Up Converter configuration / 数字上变频配置

### Digitizer Functions / Digitizer功能
- **Trigger Configuration / 触发配置**: Internal trigger, external trigger, and level trigger / 内部触发、外部触发和电平触发
- **Data Acquisition / 数据采集**: Real-time data acquisition and storage / 实时数据采集和存储
- **Data Parsing / 数据解析**: Parse acquired data packets / 解析采集的数据包
- **Frequency Domain Analysis / 频域分析**: FFT analysis and spectrum display / FFT分析和频谱显示
- **Result Visualization / 结果可视化**: Time domain and frequency domain charts / 时域和频域图表
- **Working Modes / 工作模式**: Stream mode and RingBuffer mode / Stream模式和RingBuffer模式
- **DDC Function / DDC功能**: Digital Down Converter configuration / 数字下变频配置

### Tools Module / 工具模块
- **BVCTools / BVC工具**: Device synchronization and FFT analysis / 设备同步和FFT分析
- **Signal Generator / 信号生成器**: Multiple waveform type generation / 多种波形类型生成
- **Signal Analyzer / 信号分析器**: Signal analysis and visualization / 信号分析和可视化

### Examples / 示例
- **AWG Examples / AWG示例**: Comprehensive AWG functionality demonstration / 全面的AWG功能演示
- **Digitizer Examples / Digitizer示例**: Complete Digitizer operation examples / 完整的Digitizer操作示例
- **Integration Examples / 集成示例**: AWG and Digitizer combined workflows / AWG和Digitizer结合的工作流程

### Demos / 演示
- **Integrated Demos / 集成演示**: Complete workflow from signal generation to data acquisition / 从信号生成到数据采集的完整工作流程
- **Closed-loop Testing / 闭环测试**: Signal generation, acquisition, and analysis closed loop / 信号生成、采集、分析的完整闭环
- **Result Comparison / 结果对比**: Comparison analysis of original signals and acquired signals / 原始信号与采集信号的对比分析
- **Frequency Verification / 频率验证**: Frequency component verification of multi-tone signals / 多音信号的频率分量验证

## Getting Started / 快速开始

### Prerequisites / 前提条件
- Python 3.7 or higher / Python 3.7或更高版本
- Jupyter Notebook / Jupyter Notebook
- Required Python packages: numpy, matplotlib, scipy, pyvisa, requests / 必需的Python包：numpy, matplotlib, scipy, pyvisa, requests

### Installation / 安装
1. Clone this repository / 克隆此仓库
2. Install required dependencies / 安装必需的依赖项
3. Configure device IP address / 配置设备IP地址
4. Run the example notebooks / 运行示例notebook

### Usage / 使用方法
1. Start with the tools module to understand basic functions / 从工具模块开始了解基本功能
2. Explore Examples for detailed functionality / 浏览Examples了解详细功能
3. Try demos for integrated workflows / 尝试demos进行集成工作流程
4. Customize parameters for your specific needs / 根据具体需求自定义参数

## Advanced Features / 高级功能
- **System Control / 系统控制**: Device status query and control / 设备状态查询和控制
- **Synchronized Triggering / 同步触发**: Advanced trigger configuration / 高级触发配置
- **Sample Rate Configuration / 采样率配置**: AWG and Digitizer sample rate settings / AWG和Digitizer的采样率设置
- **Troubleshooting / 故障排除**: System diagnostics and debugging functions / 系统诊断和调试功能

## Documentation / 文档
- All code is well-documented with bilingual comments (English/Chinese) / 所有代码都有双语注释（英文/中文）
- Each notebook contains detailed explanations and examples / 每个notebook都包含详细的说明和示例
- Function parameters and return values are clearly documented / 函数参数和返回值都有清晰的文档

## Support / 支持
For technical support and questions, please refer to the documentation or contact the development team.
For technical support and questions, please refer to the documentation or contact the development team. / 如需技术支持和问题咨询，请参考文档或联系开发团队。 