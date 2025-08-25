%% 信号生成器类
classdef SignalGenerator
    properties
        DATA_TYPE_MAP
        generated_signals
    end
    
    methods
        function obj = SignalGenerator()
            obj.DATA_TYPE_MAP = containers.Map([0,1,2,3,4,5,6,7,9,11], ...
                {'uint32', 'int32', 'uint8', 'int8', 'uint16', 'int16', 'int16', 'single', 'uint64', 'double'});
            obj.generated_signals = containers.Map();
        end
        
        function [t, signal_data] = generate_sine_wave(obj, params)
            % 生成正弦波
            t = linspace(0, params.duration, round(params.sample_rate * params.duration));
            signal_data = params.amplitude * sin(2 * pi * params.frequency * t + params.phase) + params.dc_offset;
        end
        
        function [t, signal_data] = generate_cose_wave(obj, params)
            % 生成余弦波
            t = linspace(0, params.duration, round(params.sample_rate * params.duration));
            signal_data = params.amplitude * cos(2 * pi * params.frequency * t + params.phase) + params.dc_offset;
        end
        
        function [t, signal_data] = generate_IQ_wave(obj, params)
            % 生成IQ波形
            t = linspace(0, params.duration, round(params.sample_rate * params.duration));
            
            % 生成I分量（实部，正弦）
            signal_data_I = params.amplitude * sin(2 * pi * params.frequency * t + params.phase) + params.dc_offset;
            
            % 生成Q分量（虚部，余弦）
            signal_data_Q = params.amplitude * cos(2 * pi * params.frequency * t + params.phase) + params.dc_offset;
            
            % 交错I和Q分量到IQ波形 [I0, Q0, I1, Q1, I2, Q2, ...]
            signal_data_IQ = zeros(1, length(t) * 2);
            signal_data_IQ(1:2:end) = signal_data_I;  % 偶数位置为I分量
            signal_data_IQ(2:2:end) = signal_data_Q;  % 奇数位置为Q分量
            
            % 创建对应的IQ波形时间数组
            t_IQ = linspace(0, params.duration, length(signal_data_IQ));
            
            t = t_IQ;
            signal_data = signal_data_IQ;
        end
        
        function [t, signal_data] = generate_multi_tone(obj, params)
            % 生成多音信号
            if isempty(params.frequencies)
                error('多音信号需要频率列表');
            end
            
            t = linspace(0, params.duration, round(params.sample_rate * params.duration));
            
            num_tones = length(params.frequencies);
            if isempty(params.amplitudes)
                amplitudes = repmat(params.amplitude, 1, num_tones);
            else
                amplitudes = params.amplitudes;
                if length(amplitudes) < num_tones
                    amplitudes = [amplitudes, repmat(amplitudes(end), 1, num_tones - length(amplitudes))];
                end
            end
            
            if isempty(params.phases)
                phases = zeros(1, num_tones);
            else
                phases = params.phases;
                if length(phases) < num_tones
                    phases = [phases, repmat(phases(end), 1, num_tones - length(phases))];
                end
            end
            
            signal_data = zeros(size(t));
            for i = 1:num_tones
                signal_data = signal_data + amplitudes(i) * sin(2 * pi * params.frequencies(i) * t + phases(i));
            end
            
            signal_data = signal_data + params.dc_offset;
        end
        
        function [t, signal_data] = generate_square_wave(obj, params)
            % 生成方波
            t = linspace(0, params.duration, round(params.sample_rate * params.duration));
            signal_data = params.amplitude * square(2 * pi * params.frequency * t + params.phase, params.duty_cycle * 100) + params.dc_offset;
        end
        
        function [t, signal_data] = generate_pulse_signal(obj, params)
            % 生成脉冲信号
            t = linspace(0, params.duration, round(params.sample_rate * params.duration));
            signal_data = zeros(size(t)) + params.dc_offset;
            
            pulse_samples = round(params.pulse_width * params.sample_rate);
            period_samples = round(params.pulse_period * params.sample_rate);
            
            for i = 1:period_samples:length(t)
                end_idx = min(i + pulse_samples - 1, length(t));
                signal_data(i:end_idx) = params.amplitude + params.dc_offset;
            end
        end
        
        function [t, signal_data] = generate_triangle_wave(obj, params)
            % 生成三角波
            t = linspace(0, params.duration, round(params.sample_rate * params.duration));
            signal_data = params.amplitude * sawtooth(2 * pi * params.frequency * t + params.phase, 0.5) + params.dc_offset;
        end
        
        function [t, signal_data] = generate_chirp_signal(obj, params)
            % 生成线性调频信号
            t = linspace(0, params.duration, round(params.sample_rate * params.duration));
            signal_data = params.amplitude * chirp(t, params.f0, params.duration, params.f1, params.method, params.phase) + params.dc_offset;
        end
        
        function [t, signal_data] = generate_signal(obj, params)
            % 根据参数生成信号
            switch params.waveform_type
                case 'sine'
                    [t, signal_data] = obj.generate_sine_wave(params);
                case 'cose'
                    [t, signal_data] = obj.generate_cose_wave(params);
                case 'iq'
                    [t, signal_data] = obj.generate_IQ_wave(params);
                case 'multi_tone'
                    [t, signal_data] = obj.generate_multi_tone(params);
                case 'square'
                    [t, signal_data] = obj.generate_square_wave(params);
                case 'pulse'
                    [t, signal_data] = obj.generate_pulse_signal(params);
                case 'triangle'
                    [t, signal_data] = obj.generate_triangle_wave(params);
                case 'chirp'
                    [t, signal_data] = obj.generate_chirp_signal(params);
                otherwise
                    error('不支持的波形类型: %s', params.waveform_type);
            end
        end
    end
end
