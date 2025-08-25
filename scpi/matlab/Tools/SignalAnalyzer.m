%% 信号分析器类
classdef SignalAnalyzer
    properties
        generator
    end
    
    methods
        function obj = SignalAnalyzer(generator)
            obj.generator = generator;
        end
        
        function plot_signal_inline(obj, t, signal_data, params, show_fft)
            if nargin < 5
                show_fft = False;
            end
            
            % 在MATLAB中显示信号
            if show_fft
                figure('Position', [100, 100, 1400, 1000]);
                subplot(2, 1, 1);
            else
                figure('Position', [100, 100, 1400, 600]);
            end
            
            % 时域图

            plot(t, signal_data, 'b-', 'LineWidth', 1.5);
            title(sprintf('%s Signal - Time Domain', upper(params.waveform_type)), 'FontSize', 14);
            xlabel('Time (s)', 'FontSize', 12);
            ylabel('Amplitude', 'FontSize', 12);
            grid on;

            % 添加统计信息
            stats_text = sprintf(['Sample Rate: %d Hz\n', ...
                                'Duration: %.3f s\n', ...
                                'Amplitude: %.3f\n', ...
                                'Max: %.3f\n', ...
                                'Min: %.3f\n', ...
                                'Mean: %.3f\n', ...
                                'RMS: %.3f'], ...
                                params.sample_rate, params.duration, params.amplitude, ...
                                max(signal_data), min(signal_data), mean(signal_data), ...
                                sqrt(mean(signal_data.^2)));

            text(0.02, 0.98, stats_text, 'Units', 'normalized', ...
                 'VerticalAlignment', 'top', 'BackgroundColor', 'yellow', ...
                 'EdgeColor', 'black', 'FontSize', 10);


            if show_fft
                % 频域图
                subplot(2, 1, 2);
                fft_data = fft(signal_data);
                freqs = (0:length(signal_data)-1) * params.sample_rate / length(signal_data);

                positive_freqs = freqs(1:floor(end/2));
                magnitude = abs(fft_data(1:floor(end/2)));
                magnitude_db = 20 * log10(magnitude + 1e-10);

                plot(positive_freqs, magnitude_db, 'r-', 'LineWidth', 1.5);
                title('Frequency Domain Analysis (FFT)', 'FontSize', 14);
                xlabel('Frequency (Hz)', 'FontSize', 12);
                ylabel('Magnitude (dB)', 'FontSize', 12);
                grid on;
                xlim([0, min(5000, params.sample_rate/2)]);
            end
        end
        
        function compare_waveforms(obj, waveform_types, base_params)
            % 比较不同波形类型
            figure('Position', [100, 100, 1600, 1200]);
            
            for i = 1:min(4, length(waveform_types))
                subplot(2, 2, i);
                
                params = SignalParams(waveform_types{i}, base_params.duration, ...
                                    base_params.sample_rate, base_params.amplitude, ...
                                    base_params.frequency, base_params.phase, base_params.dc_offset);
                
                [t, signal_data] = obj.generator.generate_signal(params);
                
                % 只显示前几个周期
                display_samples = min(round(0.01 * params.sample_rate), length(t));
                t_display = t(1:display_samples);
                signal_display = signal_data(1:display_samples);
                
                plot(t_display * 1000, signal_display, 'LineWidth', 2);
                title(sprintf('%s Waveform', upper(waveform_types{i})), 'FontSize', 12);
                xlabel('Time (ms)', 'FontSize', 10);
                ylabel('Amplitude', 'FontSize', 10);
                grid on;
            end
            
            sgtitle('Waveform Types Comparison', 'FontSize', 16);
        end
        
        function plot_frequency_spectrum(obj, t, signal_data, params, log_scale)
            % 绘制频率谱
            figure('Position', [100, 100, 1600, 600]);
            
            % 计算FFT
            fft_data = fft(signal_data);
            freqs = (0:length(signal_data)-1) * params.sample_rate / length(signal_data);
            
            % 正频率部分
            positive_freqs = freqs(1:floor(end/2));
            magnitude = abs(fft_data(1:floor(end/2)));
            phase = angle(fft_data(1:floor(end/2)));
            
            % 幅度谱
            subplot(1, 2, 1);
            if log_scale
                magnitude_db = 20 * log10(magnitude + 1e-10);
                plot(positive_freqs, magnitude_db, 'b-', 'LineWidth', 1.5);
                ylabel('Magnitude (dB)', 'FontSize', 12);
            else
                plot(positive_freqs, magnitude, 'b-', 'LineWidth', 1.5);
                ylabel('Magnitude', 'FontSize', 12);
            end
            title('Magnitude Spectrum', 'FontSize', 14);
            xlabel('Frequency (Hz)', 'FontSize', 12);
            grid on;
            xlim([0, min(5000, params.sample_rate/2)]);
            
            % 相位谱
            subplot(1, 2, 2);
            plot(positive_freqs, phase, 'r-', 'LineWidth', 1.5);
            title('Phase Spectrum', 'FontSize', 14);
            xlabel('Frequency (Hz)', 'FontSize', 12);
            ylabel('Phase (radians)', 'FontSize', 12);
            grid on;
            xlim([0, min(5000, params.sample_rate/2)]);
        end
        
        function stats = analyze_signal_statistics(obj, signal_data, params)
            % 分析信号统计特性
            stats = struct();
            stats.Signal_Length = length(signal_data);
            stats.Duration = sprintf('%.3f s', params.duration);
            stats.Sample_Rate = sprintf('%d Hz', params.sample_rate);
            stats.Maximum = sprintf('%.6f', max(signal_data));
            stats.Minimum = sprintf('%.6f', min(signal_data));
            stats.Mean = sprintf('%.6f', mean(signal_data));
            stats.RMS = sprintf('%.6f', sqrt(mean(signal_data.^2)));
            stats.Std_Dev = sprintf('%.6f', std(signal_data));
            stats.Peak_to_Peak = sprintf('%.6f', max(signal_data) - min(signal_data));
            stats.Crest_Factor = sprintf('%.3f', max(abs(signal_data)) / sqrt(mean(signal_data.^2)));
            stats.Form_Factor = sprintf('%.3f', sqrt(mean(signal_data.^2)) / mean(abs(signal_data)));
            
            fprintf('=== Signal Statistical Analysis ===\n');
            fields = fieldnames(stats);
            for i = 1:length(fields)
                fprintf('%-15s: %s\n', fields{i}, stats.(fields{i}));
            end
        end
    end
end
