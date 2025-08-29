classdef DeviceManager < handle
    % DeviceManager - 设备管理器类
    % 功能：统一管理设备连接、配置、工具路径等公共功能
    % 作者：重构自多个示例文件的公共部分
    % 日期：2024
    
    properties (Access = private)
        device_ip = '';           % 设备IP地址
        timeout = 10.0;           % 超时时间
        UltraBVC = [];            % UltraBVC设备实例
        tools_dir = '';           % 工具目录路径
        bvcTools = [];            % BVC工具实例
        generator = [];           % 信号生成器实例
        analyzer = [];            % 信号分析器实例
    end
    
    properties (Constant)
        % 默认配置参数
        DEFAULT_SAMPLE_RATE = 4;  % 4GHz采样率
        DEFAULT_PORT = 5555;      % 默认端口
        DEFAULT_TIMEOUT = 10.0;   % 默认超时时间
    end
    
    methods
        function obj = DeviceManager(device_ip, timeout)
            % 构造函数
            % 参数：
            %   device_ip: 设备IP地址
            %   timeout: 超时时间（可选）
            
            if nargin < 1
                error('必须提供设备IP地址');
            end
            if nargin < 2
                timeout = obj.DEFAULT_TIMEOUT;
            end
            
            % 验证IP地址
            if ~obj.is_valid_ip(device_ip)
                error('无效的IP地址: %s', device_ip);
            end
            
            obj.device_ip = device_ip;
            obj.timeout = timeout;
            
            % 初始化工具路径
            obj.init_tools_path();
            
            % 初始化工具类
            obj.init_tools();
        end
        
        function init_tools_path(obj)
            % 初始化工具路径
            current_dir = pwd;
            tools_dir = fullfile(fileparts(current_dir), 'Tools');
            if ~exist(tools_dir, 'dir')
                % 如果当前目录是Examples，尝试向上查找Tools目录
                tools_dir = fullfile(current_dir, '..', 'Tools');
                if ~exist(tools_dir, 'dir')
                    error('无法找到Tools目录，请确保在正确的目录中运行此脚本');
                end
            end
            obj.tools_dir = tools_dir;
            addpath(tools_dir);
            fprintf('已添加Tools目录到路径: %s\n', tools_dir);
        end
        
        function init_tools(obj)
            % 初始化工具类实例
            try
                obj.bvcTools = BVC_Tools();
                obj.generator = SignalGenerator();
                obj.analyzer = SignalAnalyzer(obj.generator);
                fprintf('工具类导入成功！\n\n');
            catch ME
                fprintf('错误：无法创建工具类实例。请确保Tools目录中的文件存在。\n');
                fprintf('错误信息：%s\n', ME.message);
                rethrow(ME);
            end
        end
        
        function connect(obj)
            % 连接设备
            fprintf('连接设备...\n');
            try
                obj.UltraBVC = TCP_UltraBVC(obj.device_ip, obj.timeout);
                fprintf('设备连接成功\n');
            catch ME
                fprintf('设备连接失败: %s\n', ME.message);
                fprintf('注意：此示例需要实际的设备连接\n');
                rethrow(ME);
            end
        end
        
        function disconnect(obj)
            % 断开设备连接
            if ~isempty(obj.UltraBVC)
                try
                    obj.UltraBVC.close();
                    fprintf('设备连接已断开\n');
                catch ME
                    fprintf('断开连接时出错: %s\n', ME.message);
                end
                obj.UltraBVC = [];
            end
        end
        
        function device_info = query_device_info(obj)
            % 查询设备信息
            fprintf('查询设备信息...\n');
            try
                device_info = containers.Map();

                % 设备名称
                device_name_str = obj.UltraBVC.recv_command(':SYS:DEVice:NAMe?');
                device_name = strsplit(device_name_str, ',');
                device_info('device_name') = device_name;

                % 设备上的所有模块名称
                module_names_str = obj.UltraBVC.recv_command(':SYS:DEVice:MODUles?');
                module_names = strsplit(module_names_str, ',');
                device_info('module_names') = module_names;

                fprintf('设备上的所有模块的功能和通道...\n');
                module_info = containers.Map();
                
                for i = 1:length(module_names)
                    module_name = module_names{i};
                    module_dict = containers.Map();

                    % 此模块支持的功能
                    funcs_str = obj.UltraBVC.recv_command(sprintf(':SYS:DEVice:MODUles:FUNC? %s', module_name));
                    funcs = strsplit(funcs_str, ',');
                    module_dict('functions') = funcs;

                    % 查询所有通道
                    in_chnls = {};
                    out_chnls = {};
                    
                    % 根据功能查询通道
                    for j = 1:length(funcs)
                        func = funcs{j};
                        if contains(func, 'AWG')
                            % AWG输出通道
                            chnls_str = obj.UltraBVC.recv_command(sprintf(':SYS:DEVice:MODUles:CHNL? %s,%s', module_name, func));
                            chnls = strsplit(chnls_str, ',');
                            out_chnls = [out_chnls, chnls];
                        elseif contains(func, 'DIG')
                            % 数字转换器输入通道
                            chnls_str = obj.UltraBVC.recv_command(sprintf(':SYS:DEVice:MODUles:CHNL? %s,%s', module_name, func));
                            chnls = strsplit(chnls_str, ',');
                            in_chnls = [in_chnls, chnls];
                        end
                    end
                    
                    module_dict('input_channels') = in_chnls;
                    module_dict('output_channels') = out_chnls;
                    module_info(module_name) = module_dict;
                end
                
                device_info('module_info') = module_info;
                fprintf('设备信息查询完成\n');
                
            catch ME
                fprintf('查询设备信息失败: %s\n', ME.message);
                rethrow(ME);
            end
        end
        
        function result = send_command(obj, command)
            % 发送SCPI命令
            if isempty(obj.UltraBVC)
                error('设备未连接，请先调用connect()方法');
            end
            result = obj.UltraBVC.send_command(command);
        end
        
        function result = recv_command(obj, command)
            % 接收SCPI命令响应
            if isempty(obj.UltraBVC)
                error('设备未连接，请先调用connect()方法');
            end
            result = obj.UltraBVC.recv_command(command);
        end
        
        function sync_device(obj, module_name, board_name)
            % 设备同步
            if isempty(obj.bvcTools)
                error('BVC工具未初始化');
            end
            obj.bvcTools.sync(obj.UltraBVC, module_name, board_name);
        end
        
        function [t, signal_data] = generate_signal(obj, waveParams)
            % 生成信号
            if isempty(obj.generator)
                error('信号生成器未初始化');
            end
            [t, signal_data] = obj.generator.generate_signal(waveParams);
        end
        
        function plot_signal(obj, t, signal_data, waveParams, show_stats)
            % 绘制信号
            if isempty(obj.analyzer)
                error('信号分析器未初始化');
            end
            if nargin < 5
                show_stats = false;
            end
            obj.analyzer.plot_signal_inline(t, signal_data, waveParams, show_stats);
        end
        
        function stats = analyze_signal(obj, signal_data, waveParams)
            % 分析信号统计
            if isempty(obj.analyzer)
                error('信号分析器未初始化');
            end
            stats = obj.analyzer.analyze_signal_statistics(signal_data, waveParams);
        end
        
        function save_waveform(obj, signal_data, waveParams, file_path)
            % 保存波形文件
            if nargin < 4
                file_path = fullfile(pwd, 'wave_file');
            end
            
            if ~exist(file_path, 'dir')
                mkdir(file_path);
            end
            
            file_name = sprintf('wave_%s_%d_%.0f_%.1f_%.0f_%.1f_%.1f.bin', ...
                waveParams.waveform_type, length(signal_data), waveParams.sample_rate, ...
                waveParams.amplitude, waveParams.frequency, waveParams.phase, waveParams.dc_offset);
            full_path = fullfile(file_path, file_name);
            
            % 保存为二进制文件
            fid = fopen(full_path, 'wb');
            if fid == -1
                error('无法创建文件: %s', full_path);
            end
            
            fwrite(fid, signal_data, 'double');
            fclose(fid);
            
            fprintf('波形文件已保存: %s\n', full_path);
        end
        
        function upload_waveform_http(obj, file_path, module_name, channel_name)
            % 通过HTTP上传波形文件
            if nargin < 4
                error('必须提供模块名称和通道名称');
            end
            
            % 调用现有的上传函数
            upload_waveform_http(file_path, obj.device_ip, module_name, channel_name);
        end
        
        function upload_nswave_http(obj, file_path, module_name, channel_name)
            % 通过HTTP上传NS波形文件
            if nargin < 4
                error('必须提供模块名称和通道名称');
            end
            
            % 调用现有的上传函数
            upload_nswave_http(file_path, obj.device_ip, module_name, channel_name);
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
        
        function str = logical_to_string(logical_val)
            % 将逻辑值转换为字符串
            if logical_val
                str = 'True';
            else
                str = 'False';
            end
        end
        
        function config = create_default_config(device_ip, sample_rate)
            % 创建默认配置
            if nargin < 1
                error('必须提供设备IP地址');
            end
            if nargin < 2
                sample_rate = DeviceManager.DEFAULT_SAMPLE_RATE;
            end
            
            config = struct();
            config.device_ip = device_ip;
            config.sample_rate = sample_rate;
            config.timeout = DeviceManager.DEFAULT_TIMEOUT;
            config.port = DeviceManager.DEFAULT_PORT;
            
            % 存储使能
            config.save_en = containers.Map();
            config.save_en('CH1') = true;
            config.save_en('CH2') = true;
            
            % 采样点数
            config.pre_sample = 32 * 1024;
            config.pst_sample = 32 * 1024;
            
            % 文件路径
            config.save_file_path = "/UserSpace";
            config.save_file_size = 1024 * 1024 * 1024;
            config.show_data_nums = 1;
            
            % 触发参数
            config.trigger_source = "Internal";
            config.in_trigger_repeat = 4294967295;
            config.in_trigger_cycle = 0.001;
        end
    end
    
    methods (Access = private)
        function print_debug(obj, message)
            % 打印调试信息
            if obj.DEBUG_PRINT
                fprintf('[DEBUG] %s\n', message);
            end
        end
    end
end
