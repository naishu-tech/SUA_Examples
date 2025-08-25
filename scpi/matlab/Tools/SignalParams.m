%% 信号参数配置类
classdef SignalParams
    properties
        waveform_type    % 波形类型
        duration         % 持续时间
        sample_rate      % 采样率
        amplitude        % 幅度
        frequency        % 频率
        phase           % 相位
        dc_offset       % 直流偏置
        
        % 方波参数
        duty_cycle      % 占空比
        
        % 多音信号参数
        frequencies     % 频率列表
        amplitudes      % 幅度列表
        phases          % 相位列表
        
        % 脉冲信号参数
        pulse_width     % 脉冲宽度
        pulse_period    % 脉冲周期
        
        % 线性调频信号参数
        f0              % 起始频率
        f1              % 结束频率
        method          % 调频方法
        
        % 波形长度
        wave_len        % 波形长度
    end
    
    methods
        function obj = SignalParams(waveform_type, duration, sample_rate, amplitude, frequency, phase, dc_offset)
            obj.waveform_type = waveform_type;
            obj.duration = duration;
            obj.sample_rate = sample_rate;
            obj.amplitude = amplitude;
            obj.frequency = frequency;
            obj.phase = phase;
            obj.dc_offset = dc_offset;
            
            % 设置默认值
            obj.duty_cycle = 0.5;
            obj.frequencies = [];
            obj.amplitudes = [];
            obj.phases = [];
            obj.pulse_width = 1e-6;
            obj.pulse_period = 1e-5;
            obj.f0 = frequency;
            obj.f1 = frequency * 2;
            obj.method = 'linear';
            obj.wave_len = 0;
        end
    end
end
