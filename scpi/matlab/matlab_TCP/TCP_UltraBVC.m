classdef TCP_UltraBVC< handle
    % TCP_UltraBVC TCP交互UltraBVC设备驱动程序 (已更新为使用 tcpclient)
    properties (Access = private)
        addr = '';           % 设备地址
        timeout = 10.0;      % 超时时间
        handle = [];         % 设备句柄 (tcpclient 对象)
        model = 'TCP_UltraBVC';  % 设备型号
        srate = [];          % 采样率
        gen_trig_num = 0;    % 触发生成次数
        param = struct();    % 参数字典
        subprtparam = {};    % 子参数结构
    end
    properties (Constant)
        % ... (这部分常量无需修改，保持原样)
        SCAN_MODE_LOCAL = 0;
        SCAN_MODE_REMOTE = 1;
        SCAN_MODE_ALONE = 2;
        ICD_HEAD_RESET = hex2dec('41000002');
        ICD_HEAD_STATUS = hex2dec('4100000E');
        ICD_HEAD_CMD_2 = hex2dec('31000015');
        ICD_HEAD_CMD_3 = hex2dec('410000B1');
        ICD_HEAD_CMD_4 = hex2dec('31000013');
        ICD_HEAD_CMD_5 = hex2dec('410000B2');
        ICD_HEAD_CMD_6 = hex2dec('3100001A');
        ICD_HEAD_CMD_7 = hex2dec('410000B3');
        CHS = 1:16;
        SYSTEM_PARAMETER = struct(...
            'RefClock', 'in', ...
            'TrigFrom', 0, ...
            'TrigPeriod', 200e-6, ...
            'TrigWidth', 800e-9, ...
            'TrigDelay', 0, ...
            'Shot', 1024, ...
            'DiscoveryMode', 0 ...
        );
    end
    properties (Access = private, Constant)
        DEBUG_PRINT = false;
    end

    % --- 从这里开始是主要修改区域 ---
    methods
        function obj = TCP_UltraBVC(addr, timeout)
            % 构造函数 (内容不变)
            if nargin < 1
                addr = '';
            end
            if nargin < 2
                timeout = 10.0;
            end
            if ~isempty(addr) && ~is_valid_ip(addr)
                error('无效的IP地址: %s', addr);
            end
            obj.addr = addr;
            obj.timeout = timeout;
            obj.param.Shot = 1024;
            obj.param.TrigPeriod = 200e-6;
            obj.param.MixMode = 2;
            obj.param.DiscoveryMode = obj.SCAN_MODE_LOCAL;
            obj.param.TrigWidth = 800e-9;
            obj.param.TrigDelay = 0;
            obj.param.TrigFrom = 0;
            obj.param.RefClock = 'out';
            obj.subprtparam = {...
                {hex2dec('5F5F5F5F'), obj.ICD_HEAD_RESET, 0, 4*(17*100+17*4+5), 0}, ...
                cell(1, 17), ...
                cell(1, 17) ...
            };
            for i = 1:17
                obj.subprtparam{2}{i} = [0, 0, 800, 0];
                obj.subprtparam{3}{i} = zeros(1, 100);
            end
            obj.print_debug(sprintf('QSYNC: 实例化成功 %s', addr));
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

        % --- 修改点 3: send_command 方法 ---
        function result = send_command(obj, data, varargin)
            % 发送命令 - 完整实现 (使用 tcpclient 的 read/write)
            p = inputParser;
            addParameter(p, 'wait', 0);
            addParameter(p, 'addr', obj.addr);
            addParameter(p, 'port', 5555);
            addParameter(p, 'check_feedback', false); % 默认不检查反馈，避免超时
            addParameter(p, 'return_fdk', false);
            addParameter(p, 'connect_timeout', 10);
            addParameter(p, 'timeout', 10); % read/write timeout
            parse(p, varargin{:});
            
            if nargin < 2
                data = ':AWG:TRIGger:SOURce S2_M4901,PXISTARTrig';
            end

            max_retries = 3;
            for retry = 1:max_retries
                % 建立连接
                client = obj.connect(p.Results.addr, p.Results.port, p.Results.connect_timeout);
                if isempty(client)
                    if retry == max_retries, result = false; return; end
                    pause(0.1); continue;
                end

                % 设置读写超时
                client.Timeout = p.Results.timeout;

                try
                    
                    configureTerminator(client, "LF");
                    writeline(client, data);
                    feedback = readline(client);
                    fprintf('feedback: %s\n', feedback);

                    obj.close_socket(client);
                    result = true;
                    return; % 成功则直接返回

                catch ME
                    fprintf('device: %s指令%s发送失败 %s\n', p.Results.addr, data, ME.message);
                    obj.close_socket(client);
                    if retry == max_retries, result = false; return; end
                    pause(0.1);
                end
            end
            result = false;
        end

        function close(obj)
            % 关闭设备
            obj.print_debug('QSYNC: 关闭设备连接');
        end

        function print_debug(obj, message)
            % 调试打印
            if obj.DEBUG_PRINT
                fprintf('[DEBUG] %s\n', message);
            end
        end
    end
end