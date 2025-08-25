classdef BVC_Tools < handle
    % BVCTools - MATLAB版本BVC工具类
    % 功能：设备同步和FFT分析
    % 作者：基于Python版本转换
    % 日期：2024
    
    properties
        scpi_query    % SCPI查询函数句柄
        scpi_write    % SCPI写入函数句柄
        scpi_read     % SCPI读取函数句柄
        session       % VISA会话对象
    end
    
    methods
        function obj = BVC_Tools()
            % 构造函数
            obj.scpi_query = [];
            obj.scpi_write = [];
            obj.scpi_read = [];
            obj.session = [];
        end
        
        function sync(obj, UltraBVC, module_name, board_name)
            % 设备同步功能
            % 输入参数：
            %   device_ip: 设备IP地址
            %   module_name: 模块名称
            %   board_name: 板卡名称
            
            fprintf('开始设备同步...\n');
            
            % 执行同步步骤
            UltraBVC.send_command(sprintf(':SYS:REFerence:SAT:STEP1 %s', module_name));
            fprintf('步骤1: 参考同步步骤1完成\n');

            UltraBVC.send_command(sprintf(':SYS:REFerence:SAT:STEP2 %s', module_name));
            fprintf('步骤2: 参考同步步骤2完成\n');
            
            UltraBVC.send_command(sprintf(':SAT:MULTisync:SAT:STEP3 %s', board_name));
            fprintf('步骤3: 多同步步骤3完成\n');
            
            UltraBVC.send_command(sprintf(':SYS:REFerence:SAT:STEP4 %s', module_name));
            fprintf('步骤4: 参考同步步骤4完成\n');

            UltraBVC.send_command(sprintf(':SAT:MULTisync:SAT:STEP5 %s', board_name));
            fprintf('步骤5: 多同步步骤5完成\n');

            UltraBVC.send_command(sprintf(':SYS:REFerence:SAT:STEP6 %s', module_name));
            fprintf('步骤6: 参考同步步骤6完成\n');

            UltraBVC.send_command(sprintf(':SAT:MULTisync:SAT:STEP7 %s', board_name));
            fprintf('步骤7: 多同步步骤7完成\n');
            
            % 执行最终同步命令
            UltraBVC.send_command(sprintf(':exe %s,14', module_name));
            fprintf('步骤8: 最终同步命令完成\n');
            
            fprintf('设备同步完成！\n');
        end
        
        function [scpi_query, scpi_write, scpi_read] = link_BVC(obj, device_ip)
            % 连接BVC设备
            % 输入参数：
            %   device_ip: 设备IP地址
            % 输出参数：
            %   scpi_query: SCPI查询函数句柄
            %   scpi_write: SCPI写入函数句柄
            %   scpi_read: SCPI读取函数句柄
            
            try
                % 构建VISA资源字符串
                visa_resource = sprintf('TCPIP::%s::5555::SOCKET', device_ip);
                fprintf('尝试连接到: %s\n', visa_resource);
                
                % 创建VISA资源管理器
                rm = visa('ni', visa_resource);
                
                % 设置会话参数
                rm.OutputBufferSize = 8192;
                rm.InputBufferSize = 8192;
                rm.Timeout = 50;  % 50秒超时
                rm.Terminator = 'LF';  % 行终止符
                
                % 打开连接
                fopen(rm);
                fprintf('成功连接到设备: %s\n', device_ip);
                
                % 保存会话对象
                obj.session = rm;
                
                % 创建SCPI函数句柄
                obj.scpi_query = @(cmd) obj.scpi_query_internal(cmd);
                obj.scpi_write = @(cmd) obj.scpi_write_internal(cmd);
                obj.scpi_read = @() obj.scpi_read_internal();
                
                % 返回函数句柄
                scpi_query = obj.scpi_query;
                scpi_write = obj.scpi_write;
                scpi_read = obj.scpi_read;
                
            catch ME
                fprintf('连接设备失败: %s\n', ME.message);
                error('无法连接到BVC设备: %s', device_ip);
            end
        end
        
        function response = scpi_query_internal(obj, command)
            % 内部SCPI查询函数
            % 输入参数：
            %   command: SCPI命令字符串
            % 输出参数：
            %   response: 设备响应
            
            try
                if isempty(obj.session)
                    error('设备未连接');
                end
                
                % 发送命令并读取响应
                fprintf(obj.session, command);
                response = fscanf(obj.session);
                
                % 去除换行符
                response = strtrim(response);
                
                fprintf('SCPI查询: %s -> %s\n', command, response);
                
            catch ME
                fprintf('SCPI查询失败: %s\n', ME.message);
                response = '';
            end
        end
        
        function scpi_write_internal(obj, command)
            % 内部SCPI写入函数
            % 输入参数：
            %   command: SCPI命令字符串
            
            try
                if isempty(obj.session)
                    error('设备未连接');
                end
                
                % 发送命令
                fprintf(obj.session, command);
                fprintf('SCPI写入: %s\n', command);
                
            catch ME
                fprintf('SCPI写入失败: %s\n', ME.message);
            end
        end
        
        function response = scpi_read_internal(obj)
            % 内部SCPI读取函数
            % 输出参数：
            %   response: 设备响应
            
            try
                if isempty(obj.session)
                    error('设备未连接');
                end
                
                % 读取响应
                response = fscanf(obj.session);
                response = strtrim(response);
                
                fprintf('SCPI读取: %s\n', response);
                
            catch ME
                fprintf('SCPI读取失败: %s\n', ME.message);
                response = '';
            end
        end
        
        function h_fig = plot_fft(obj, image_num, signal, t, fs, fc, phase)
            % FFT分析函数
            % 输入参数：
            %   image_num: 图像编号
            %   signal: 信号数据
            %   t: 时间数组
            %   fs: 采样频率
            %   fc: 载波频率
            %   phase: 相位
            
            if nargin < 6
                fs = 20e6;  % 默认采样频率20MHz
            end
            if nargin < 7
                fc = 1e6;   % 默认载波频率1MHz
            end
            if nargin < 8
                phase = 0;  % 默认相位0
            end
            
            fprintf('开始FFT分析，图像编号: %d\n', image_num);
            
            % 计算频谱 (-fs/2, fs/2)
            % 直接FFT计算，默认点数
            fft_data = fft(signal);  % 生成频谱 [0, fs)
            freq = (0:length(t)-1) * fs / length(t);  % 默认频率轴 [0, fs)
            
            % 生成对称频率格式 [-fs/2, fs/2)
            freq_shifted = freq - fs/2;
            fft_shifted = fftshift(fft_data);  % 将频谱移位到 [-fs/2, fs/2)
            
            % 生成频率轴 [0, fs)
            fft_freq_fs = linspace(0, fs, length(fft_data));
            
            % 创建图形窗口
            h_fig = figure('Position', [100, 100, 1600, 800]);
            
            % 绘制 [0, fs) 频谱
            subplot(2, 1, 1);
            stem(fft_freq_fs, abs(fft_data), 'b-', 'LineWidth', 1.5, 'MarkerSize', 4);
            title(sprintf('No.%d Data Signal FFT (0, fs)', image_num), 'FontSize', 14);
            xlabel('Frequency (Hz)', 'FontSize', 12);
            ylabel('Magnitude', 'FontSize', 12);
            grid on;
            xlim([0, fs]);

            % 绘制 [-fs/2, fs/2) 频谱
            subplot(2, 1, 2);
            stem(freq_shifted, abs(fft_shifted), 'r-', 'LineWidth', 1.5, 'MarkerSize', 4);
            title(sprintf('No.%d Data Signal FFT (-fs/2, fs/2)', image_num), 'FontSize', 14);
            xlabel('Frequency (Hz)', 'FontSize', 12);
            ylabel('Magnitude', 'FontSize', 12);
            grid on;
            xlim([-fs/2, fs/2]);
            
            % 输出统计信息
            fprintf('信号FFT最大值: %.6f\n', max(abs(fft_data)));
            fprintf('信号FFT移位后最大值: %.6f\n', max(abs(fft_shifted)));
            
            % 添加信号参数信息
            signal_info = sprintf(['信号参数:\n', ...
                                 '采样频率: %.2e Hz\n', ...
                                 '载波频率: %.2e Hz\n', ...
                                 '相位: %.2f rad\n', ...
                                 '信号长度: %d 点\n', ...
                                 '时间范围: %.2e - %.2e s'], ...
                                 fs, fc, phase, length(signal), t(1), t(end));
            
            annotation('textbox', [0.02, 0.02, 0.3, 0.1], ...
                      'String', signal_info, ...
                      'EdgeColor', 'black', ...
                      'BackgroundColor', 'yellow', ...
                      'FontSize', 10);
        end
        
        function disconnect(obj)
            % 断开设备连接
            
            if ~isempty(obj.session)
                try
                    fclose(obj.session);
                    delete(obj.session);
                    obj.session = [];
                    obj.scpi_query = [];
                    obj.scpi_write = [];
                    obj.scpi_read = [];
                    fprintf('设备连接已断开\n');
                catch ME
                    fprintf('断开连接时出错: %s\n', ME.message);
                end
            end
        end
        
        function delete(obj)
            % 析构函数，确保断开连接
            obj.disconnect();
        end
    
    % --- 修改点 1: connect 方法 ---
        function client = connect(obj, addr, port, timeout)
            % 建立TCP连接 (使用 tcpclient)
            if nargin < 2, addr = obj.addr; end
            if nargin < 3, port = 5555; end
            if nargin < 4, timeout = obj.timeout; end

            try
                % 使用新的 tcpclient 函数，它会直接返回一个连接好的客户端对象
                client = tcpclient(addr, port, 'Timeout', timeout, 'ConnectTimeout', timeout);
                obj.print_debug(sprintf('TCP连接建立成功: %s:%d', addr, port));
            catch ME
                fprintf('device: %s无法连接 %s\n', addr, ME.message);
                client = []; % 连接失败返回空
            end
        end

        % --- 修改点 2: close_socket 方法 ---
        function close_socket(~, client) % obj不再需要，用~代替
            % 关闭TCP连接 (tcpclient 对象)
            if ~isempty(client)
                try
                    clear client; % 清除对象即可自动关闭连接
                catch
                    % 忽略清除时可能发生的错误
                end
            end
        end
    
    
    end
end
