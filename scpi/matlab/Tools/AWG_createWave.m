function AWG_createWave()
% AWG_createWave - MATLAB版本AWG波形创建工具
% 功能：生成和分析各种类型的波形信号

    fprintf('=== AWG波形创建工具 (MATLAB版本) ===\n');
    fprintf('所有依赖库已成功导入！\n\n');
    
    % 创建信号生成器和分析器实例
    generator = SignalGenerator();
    analyzer = SignalAnalyzer(generator);
    
    % 示例：生成正弦波
    fprintf('示例1：生成正弦波\n');
    params = SignalParams('sine', 1.0, 44100, 1.0, 1000.0, 0.0, 0.0);
    [t, signal_data] = generator.generate_signal(params);
    analyzer.plot_signal_inline(t, signal_data, params, false);
    
    % 示例：生成方波并显示FFT
    fprintf('\n示例2：生成方波并显示FFT\n');
    params = SignalParams('square', 0.1, 44100, 1.0, 100.0, 0.0, 0.0);
    params.duty_cycle = 0.3;
    [t, signal_data] = generator.generate_signal(params);
    analyzer.plot_signal_inline(t, signal_data, params, true);
    
    % 示例：生成IQ波形
    fprintf('\n示例3：生成IQ波形\n');
    params = SignalParams('iq', 0.01, 44100, 1.0, 1000.0, 0.0, 0.0);
    [t, signal_data] = generator.generate_signal(params);
    analyzer.plot_signal_inline(t, signal_data, params, false);
    
    % 示例：生成多音信号
    fprintf('\n示例4：生成多音信号\n');
    params = SignalParams('multi_tone', 0.01, 44100, 1.0, 1000.0, 0.0, 0.0);
    params.frequencies = [1000, 2000, 3000];
    params.amplitudes = [1.0, 0.5, 0.3];
    params.phases = [0, pi/4, pi/2];
    [t, signal_data] = generator.generate_signal(params);
    analyzer.plot_signal_inline(t, signal_data, params, false);
    
    % 示例：生成线性调频信号
    fprintf('\n示例5：生成线性调频信号\n');
    params = SignalParams('chirp', 0.01, 44100, 1.0, 1000.0, 0.0, 0.0);
    params.f0 = 100;
    params.f1 = 2000;
    [t, signal_data] = generator.generate_signal(params);
    analyzer.plot_signal_inline(t, signal_data, params, false);
    
    % 示例：波形比较
    fprintf('\n示例6：波形类型比较\n');
    waveform_types = {'sine', 'square', 'triangle', 'pulse'};
    base_params = SignalParams('sine', 0.01, 44100, 1.0, 1000.0, 0.0, 0.0);
    analyzer.compare_waveforms(waveform_types, base_params);
    
    % 示例：频率谱分析
    fprintf('\n示例7：频率谱分析\n');
    params = SignalParams('sine', 0.01, 44100, 1.0, 1000.0, 0.0, 0.0);
    [t, signal_data] = generator.generate_signal(params);
    analyzer.plot_frequency_spectrum(t, signal_data, params, true);
    
    % 示例：信号统计分析
    fprintf('\n示例8：信号统计分析\n');
    params = SignalParams('sine', 0.01, 44100, 1.0, 1000.0, 0.0, 0.0);
    [t, signal_data] = generator.generate_signal(params);
    analyzer.analyze_signal_statistics(signal_data, params);
    
    fprintf('\n=== 所有示例完成 ===\n');
end