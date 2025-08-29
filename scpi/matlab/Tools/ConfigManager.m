classdef ConfigManager < handle
    % ConfigManager - 配置管理器类
    % 功能：统一管理设备配置参数，包括AWG、DAQ、触发等配置
    % 作者：重构自多个示例文件的配置部分
    % 日期：2024
    
    properties (Access = public)
        device_ip = '';           % 设备IP地址
        sample_rate = 4;          % 采样率 (GHz)
        timeout = 10.0;           % 超时时间
        port = 5555;              % 端口号
        
        % AWG配置
        awg_config = struct();
        
        % DAQ配置
        daq_config = struct();
        
        % 触发配置
        trigger_config = struct();
        
        % 文件配置
        file_config = struct();
        
        % 存储配置
        save_config = struct();
    end
    
    properties (Constant)
        % 默认值常量
        DEFAULT_SAMPLE_RATE = 4;      % 4GHz采样率
        DEFAULT_TIMEOUT = 10.0;       % 默认超时时间
        DEFAULT_PORT = 5555;          % 默认端口
        DEFAULT_PRE_SAMPLE = 32 * 1024;   % 默认前置采样点数
        DEFAULT_PST_SAMPLE = 32 * 1024;   % 默认后置采样点数
        DEFAULT_SAVE_FILE_SIZE = 1024 * 1024 * 1024;  % 默认文件大小
        DEFAULT_TRIGGER_REPEAT = 4294967295;  % 默认触发重复次数
        DEFAULT_TRIGGER_CYCLE = 0.001;        % 默认触发周期
    end
    
    methods
        function obj = ConfigManager(device_ip, varargin)
            % 构造函数
            % 参数：
            %   device_ip: 设备IP地址
            %   varargin: 可选参数对 (参数名, 参数值)
            
            if nargin < 1
                error('必须提供设备IP地址');
            end
            
            % 验证IP地址
            if ~ConfigManager.is_valid_ip(device_ip)
                error('无效的IP地址: %s', device_ip);
            end
            
            obj.device_ip = device_ip;
            
            % 设置默认值
            obj.sample_rate = obj.DEFAULT_SAMPLE_RATE;
            obj.timeout = obj.DEFAULT_TIMEOUT;
            obj.port = obj.DEFAULT_PORT;
            
            % 初始化各配置结构
            obj.init_default_configs();
            
            % 处理可选参数
            if nargin > 1
                obj.parse_optional_params(varargin{:});
            end
        end
        
        function init_default_configs(obj)
            % 初始化默认配置
            
            % AWG默认配置
            obj.awg_config.waveform_type = "sine";
            obj.awg_config.amplitude = 0.8;
            obj.awg_config.frequency = 100e6;
            obj.awg_config.phase = 0.0;
            obj.awg_config.dc_offset = 0.0;
            obj.awg_config.duty_cycle = 0.5;
            obj.awg_config.frequencies = [100e6, 300e6, 400e6];
            obj.awg_config.amplitudes = [0.6, 0.4, 0.2];
            obj.awg_config.phases = [0.0, 0.0, 0.0];
            obj.awg_config.pulse_width = 1e-8;
            obj.awg_config.pulse_period = 1e-7;
            obj.awg_config.f0 = 100e6;
            obj.awg_config.f1 = 500e6;
            
            % DAQ默认配置
            obj.daq_config.save_en = containers.Map();
            obj.daq_config.save_en('CH1') = true;
            obj.daq_config.save_en('CH2') = true;
            obj.daq_config.pre_sample = obj.DEFAULT_PRE_SAMPLE;
            obj.daq_config.pst_sample = obj.DEFAULT_PST_SAMPLE;
            obj.daq_config.show_data_nums = 1;
            
            % 触发默认配置
            obj.trigger_config.source = "Internal";
            obj.trigger_config.repeat = obj.DEFAULT_TRIGGER_REPEAT;
            obj.trigger_config.cycle = obj.DEFAULT_TRIGGER_CYCLE;
            obj.trigger_config.width = 800e-9;
            obj.trigger_config.delay = 0;
            obj.trigger_config.from = 0;
            
            % 文件默认配置
            obj.file_config.save_path = "/UserSpace";
            obj.file_config.save_size = obj.DEFAULT_SAVE_FILE_SIZE;
            obj.file_config.wave_file_path = fullfile(pwd, 'wave_file');
            obj.file_config.result_path = fullfile(pwd, 'result');
            obj.file_config.seq_file_path = fullfile(pwd, 'seq_file');
            
            % 存储默认配置
            obj.save_config.enable = true;
            obj.save_config.format = 'binary';
            obj.save_config.compression = false;
        end
        
        function parse_optional_params(obj, varargin)
            % 解析可选参数
            for i = 1:2:length(varargin)
                if i+1 <= length(varargin)
                    param_name = varargin{i};
                    param_value = varargin{i+1};
                    obj.set_param(param_name, param_value);
                end
            end
        end
        
        function set_param(obj, param_name, param_value)
            % 设置参数值
            switch lower(param_name)
                case 'sample_rate'
                    obj.sample_rate = param_value;
                case 'timeout'
                    obj.timeout = param_value;
                case 'port'
                    obj.port = param_value;
                case 'waveform_type'
                    obj.awg_config.waveform_type = param_value;
                case 'amplitude'
                    obj.awg_config.amplitude = param_value;
                case 'frequency'
                    obj.awg_config.frequency = param_value;
                case 'phase'
                    obj.awg_config.phase = param_value;
                case 'dc_offset'
                    obj.awg_config.dc_offset = param_value;
                case 'duty_cycle'
                    obj.awg_config.duty_cycle = param_value;
                case 'trigger_source'
                    obj.trigger_config.source = param_value;
                case 'trigger_repeat'
                    obj.trigger_config.repeat = param_value;
                case 'trigger_cycle'
                    obj.trigger_config.cycle = param_value;
                case 'pre_sample'
                    obj.daq_config.pre_sample = param_value;
                case 'pst_sample'
                    obj.daq_config.pst_sample = param_value;
                case 'save_path'
                    obj.file_config.save_path = param_value;
                case 'save_size'
                    obj.file_config.save_size = param_value;
                otherwise
                    warning('未知参数: %s', param_name);
            end
        end
        
        function value = get_param(obj, param_name)
            % 获取参数值
            switch lower(param_name)
                case 'device_ip'
                    value = obj.device_ip;
                case 'sample_rate'
                    value = obj.sample_rate;
                case 'timeout'
                    value = obj.timeout;
                case 'port'
                    value = obj.port;
                case 'waveform_type'
                    value = obj.awg_config.waveform_type;
                case 'amplitude'
                    value = obj.awg_config.amplitude;
                case 'frequency'
                    value = obj.awg_config.frequency;
                case 'phase'
                    value = obj.awg_config.phase;
                case 'dc_offset'
                    value = obj.awg_config.dc_offset;
                case 'duty_cycle'
                    value = obj.awg_config.duty_cycle;
                case 'trigger_source'
                    value = obj.trigger_config.source;
                case 'trigger_repeat'
                    value = obj.trigger_config.repeat;
                case 'trigger_cycle'
                    value = obj.trigger_config.cycle;
                case 'pre_sample'
                    value = obj.daq_config.pre_sample;
                case 'pst_sample'
                    value = obj.daq_config.pst_sample;
                case 'save_path'
                    value = obj.file_config.save_path;
                case 'save_size'
                    value = obj.file_config.save_size;
                otherwise
                    error('未知参数: %s', param_name);
            end
        end
        
        function waveParams = create_wave_params(obj)
            % 创建波形参数对象
            waveParams = SignalParams(obj.awg_config.waveform_type, ...
                                    obj.awg_config.amplitude, ...
                                    obj.sample_rate * 10^9, ...
                                    obj.awg_config.frequency, ...
                                    obj.awg_config.phase, ...
                                    obj.awg_config.dc_offset);
            
            % 设置其他参数
            waveParams.duty_cycle = obj.awg_config.duty_cycle;
            waveParams.frequencies = obj.awg_config.frequencies;
            waveParams.amplitudes = obj.awg_config.amplitudes;
            waveParams.phases = obj.awg_config.phases;
            waveParams.pulse_width = obj.awg_config.pulse_width;
            waveParams.pulse_period = obj.awg_config.pulse_period;
            waveParams.f0 = obj.awg_config.f0;
            waveParams.f1 = obj.awg_config.f1;
        end
        
        function config_struct = to_struct(obj)
            % 转换为结构体
            config_struct = struct();
            config_struct.device_ip = obj.device_ip;
            config_struct.sample_rate = obj.sample_rate;
            config_struct.timeout = obj.timeout;
            config_struct.port = obj.port;
            config_struct.awg_config = obj.awg_config;
            config_struct.daq_config = obj.daq_config;
            config_struct.trigger_config = obj.trigger_config;
            config_struct.file_config = obj.file_config;
            config_struct.save_config = obj.save_config;
        end
        
        function print_config(obj)
            % 打印配置信息
            fprintf('=== 设备配置信息 ===\n');
            fprintf('设备IP: %s\n', obj.device_ip);
            fprintf('采样率: %.1f GHz\n', obj.sample_rate);
            fprintf('超时时间: %.1f 秒\n', obj.timeout);
            fprintf('端口: %d\n', obj.port);
            
            fprintf('\n=== AWG配置 ===\n');
            fprintf('波形类型: %s\n', obj.awg_config.waveform_type);
            fprintf('幅度: %.2f\n', obj.awg_config.amplitude);
            fprintf('频率: %.0f Hz\n', obj.awg_config.frequency);
            fprintf('相位: %.2f\n', obj.awg_config.phase);
            fprintf('直流偏移: %.2f\n', obj.awg_config.dc_offset);
            
            fprintf('\n=== DAQ配置 ===\n');
            fprintf('前置采样点数: %d\n', obj.daq_config.pre_sample);
            fprintf('后置采样点数: %d\n', obj.daq_config.pst_sample);
            fprintf('显示数据数量: %d\n', obj.daq_config.show_data_nums);
            
            fprintf('\n=== 触发配置 ===\n');
            fprintf('触发源: %s\n', obj.trigger_config.source);
            fprintf('触发重复次数: %d\n', obj.trigger_config.repeat);
            fprintf('触发周期: %.6f 秒\n', obj.trigger_config.cycle);
            
            fprintf('\n=== 文件配置 ===\n');
            fprintf('保存路径: %s\n', obj.file_config.save_path);
            fprintf('文件大小: %d 字节\n', obj.file_config.save_size);
            fprintf('波形文件路径: %s\n', obj.file_config.wave_file_path);
            fprintf('结果路径: %s\n', obj.file_config.result_path);
            fprintf('序列文件路径: %s\n', obj.file_config.seq_file_path);
            fprintf('========================\n\n');
        end
        
        function validate_config(obj)
            % 验证配置参数
            errors = {};
            
            % 验证IP地址
            if ~ConfigManager.is_valid_ip(obj.device_ip)
                errors{end+1} = '无效的设备IP地址';
            end
            
            % 验证采样率
            if obj.sample_rate <= 0
                errors{end+1} = '采样率必须大于0';
            end
            
            % 验证超时时间
            if obj.timeout <= 0
                errors{end+1} = '超时时间必须大于0';
            end
            
            % 验证端口
            if obj.port < 1 || obj.port > 65535
                errors{end+1} = '端口号必须在1-65535范围内';
            end
            
            % 验证AWG参数
            if obj.awg_config.amplitude < 0 || obj.awg_config.amplitude > 1
                errors{end+1} = 'AWG幅度必须在0-1范围内';
            end
            
            if obj.awg_config.frequency <= 0
                errors{end+1} = 'AWG频率必须大于0';
            end
            
            if obj.awg_config.duty_cycle < 0 || obj.awg_config.duty_cycle > 1
                errors{end+1} = '占空比必须在0-1范围内';
            end
            
            % 验证DAQ参数
            if obj.daq_config.pre_sample <= 0
                errors{end+1} = '前置采样点数必须大于0';
            end
            
            if obj.daq_config.pst_sample <= 0
                errors{end+1} = '后置采样点数必须大于0';
            end
            
            % 验证触发参数
            if obj.trigger_config.cycle <= 0
                errors{end+1} = '触发周期必须大于0';
            end
            
            if obj.trigger_config.repeat < 0
                errors{end+1} = '触发重复次数不能为负数';
            end
            
            % 如果有错误，抛出异常
            if ~isempty(errors)
                error_msg = sprintf('配置验证失败:\n%s', strjoin(errors, '\n'));
                error(error_msg);
            end
            
            fprintf('配置验证通过！\n');
        end
    end
    
    methods (Static)
        function result = is_valid_ip(ip_str)
            % 验证IP地址格式
            if isempty(ip_str)
                result = false;
                return;
            end
            
            % 简单的IP格式验证
            parts = strsplit(ip_str, '.');
            if length(parts) ~= 4
                result = false;
                return;
            end
            
            for i = 1:4
                num = str2double(parts{i});
                if isnan(num) || num < 0 || num > 255
                    result = false;
                    return;
                end
            end
            
            result = true;
        end
        
        function config = load_from_file(file_path)
            % 从文件加载配置
            if ~exist(file_path, 'file')
                error('配置文件不存在: %s', file_path);
            end
            
            try
                % 尝试加载MAT文件
                if endsWith(file_path, '.mat')
                    data = load(file_path);
                    if isfield(data, 'config')
                        config = data.config;
                    else
                        error('MAT文件中未找到config变量');
                    end
                % 尝试加载JSON文件
                elseif endsWith(file_path, '.json')
                    fid = fopen(file_path, 'r');
                    json_str = fread(fid, '*char')';
                    fclose(fid);
                    config = jsondecode(json_str);
                else
                    error('不支持的文件格式，请使用.mat或.json文件');
                end
            catch ME
                error('加载配置文件失败: %s', ME.message);
            end
        end
        
        function save_to_file(config, file_path)
            % 保存配置到文件
            try
                if endsWith(file_path, '.mat')
                    save(file_path, 'config');
                elseif endsWith(file_path, '.json')
                    json_str = jsonencode(config);
                    fid = fopen(file_path, 'w');
                    fprintf(fid, '%s', json_str);
                    fclose(fid);
                else
                    error('不支持的文件格式，请使用.mat或.json文件');
                end
                fprintf('配置已保存到: %s\n', file_path);
            catch ME
                error('保存配置文件失败: %s', ME.message);
            end
        end
    end
end
