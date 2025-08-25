function Digitizer_RingBuffer()
% Digitizer_RingBuffer - MATLAB版本数字转换器环形缓冲区数据采集
% 功能：配置数字转换器、数据采集、数据保存和分析
% 作者：基于Python版本转换
% 日期：2024

fprintf('=== 数字转换器环形缓冲区数据采集 (MATLAB版本) ===\n');
fprintf('所有依赖库已成功导入！\n\n');

% 添加Tools目录到路径
current_dir = pwd;
tools_dir = fullfile(fileparts(current_dir), 'Tools');
if ~exist(tools_dir, 'dir')
    % 如果当前目录是Examples，尝试向上查找Tools目录
    tools_dir = fullfile(current_dir, '..', 'Tools');
    if ~exist(tools_dir, 'dir')
        error('无法找到Tools目录，请确保在正确的目录中运行此脚本');
    end
end
addpath(tools_dir);
fprintf('已添加Tools目录到路径: %s\n', tools_dir);

% 创建工具类实例
try
    bvcTools = BVC_Tools();
    fprintf('工具类导入成功！\n\n');
catch ME
    fprintf('错误：无法创建工具类实例。请确保Tools目录中的文件存在。\n');
    fprintf('错误信息：%s\n', ME.message);
    return;
end

%% 配置设备参数
device_ip = "172.16.152.68";

% 采样率
sample_rate = 4; % 4GHz采样率

% 存储使能
save_en = containers.Map();
save_en('CH1') = true;
save_en('CH2') = true;

% 采样点数
pre_sample = 32 * 1024;
pst_sample = 32 * 1024;

% 文件路径
save_file_path = "/UserSpace";

% 切片大小
save_file_size = 1024 * 1024 * 1024;

show_data_nums = 1;

%% 触发参数
trigger_source = "Internal";
in_trigger_repeat = 4294967295;
in_trigger_cycle = 0.001;

%% 连接设备
fprintf('连接设备...\n');
try
    % 设备配置
    config = struct();
    config.UltraBVC_ip = device_ip;
    
    UltraBVC = TCP_UltraBVC(config.UltraBVC_ip);
    fprintf('设备连接成功\n');
catch ME
    fprintf('设备连接失败: %s\n', ME.message);
    fprintf('注意：此示例需要实际的数字转换器设备连接\n');
    return;
end

%% 查询设备信息
fprintf('查询设备信息...\n');
try
    model_info = containers.Map();

    % 设备名称
    device_name_str = UltraBVC.recv_command(':SYS:DEVice:NAMe?');
    device_name = strsplit(device_name_str, ',');

    % 设备上的所有模块名称
    module_names_str = UltraBVC.recv_command(':SYS:DEVice:MODUles?');
    module_names = strsplit(module_names_str, ',');

    fprintf('step3: 设备上的所有模块的功能和通道...\n');
    for i = 1:length(module_names)
        module_name = module_names{i};
        module_dict = containers.Map();

        % 此模块支持的功能
        funcs_str = UltraBVC.recv_command(sprintf(':SYS:DEVice:MODUles:FUNC? %s', module_name));
        funcs = strsplit(funcs_str, ',');

        % 查询所有通道
        in_chnls = {};
        out_chnls = {};

        for j = 1:length(funcs)
            func = funcs{j};
            if strcmp(func, 'AWG')
                in_chnls_str = UltraBVC.recv_command(sprintf(':SYS:DEVice:MODUles:CHNLs:OUT? %s', module_name));
                in_chnls = strsplit(in_chnls_str, ',');
            end
            if strcmp(func, 'Digitizer')
                out_chnls_str = UltraBVC.recv_command(sprintf(':SYS:DEVice:MODUles:CHNLs:IN? %s', module_name));
                out_chnls = strsplit(out_chnls_str, ',');
            end
        end

        module_dict('AWG') = in_chnls;
        module_dict('Digitizer') = out_chnls;
        model_info(module_name) = module_dict;
    end

    fprintf('设备名称: %s\n', strjoin(device_name, ', '));
    fprintf('模块信息:\n');
    module_keys = keys(model_info);
    for i = 1:length(module_keys)
        key = module_keys{i};
        module_data = model_info(key);
        awg_channels = module_data('AWG');
        dig_channels = module_data('Digitizer');
        fprintf('  %s: AWG通道[%s], Digitizer通道[%s]\n', ...
            key, strjoin(awg_channels, ','), strjoin(dig_channels, ','));
    end

catch ME
    fprintf('查询设备信息失败: %s\n', ME.message);
end

%% 配置模块和通道
module_name = 'S2_M4901';
board_name = 'S0_M1210';

% 通道使能
channel_en = containers.Map();
channel_en('CH1') = true;
channel_en('CH2') = true;

%% 检查工作模式
fprintf('检查工作模式...\n');
try
    mode = UltraBVC.recv_command(sprintf(':DAQ:ACQuire:WKMode? %s,', module_name));
    if strcmp(mode, 'Stream')
        UltraBVC.send_command(sprintf(':DAQ:ACQuire:WKMode %s,RingBuffer', module_name));
        fprintf('工作模式已切换到RingBuffer\n');
    elseif strcmp(mode, 'RingBuffer')
        fprintf('工作模式已经是RingBuffer\n');
    else
        error('未知的工作模式: %s', mode);
    end
catch ME
    fprintf('检查工作模式失败: %s\n', ME.message);
end

%% 关闭DDC
fprintf('配置DAQ设置...\n');
try
    UltraBVC.send_command(sprintf(':DAQ:ACQuire:ExtractMultiple %s,1', module_name));

    % 关闭DDC
    channel_keys = keys(channel_en);
    for i = 1:length(channel_keys)
        key = channel_keys{i};
        if channel_en(key)
            UltraBVC.send_command(sprintf(':DAQ:CHANnel:DDC:EN %s,False', key));
        end
    end

    fprintf('DAQ设置配置完成\n');
catch ME
    fprintf('DAQ设置配置失败: %s\n', ME.message);
end

%% 配置采样率并同步
fprintf('配置采样率并同步设备...\n');
try
    UltraBVC.send_command(sprintf(':DAQ:ACQuire:ADC:SRATe %s,%d', module_name, sample_rate));
    bvcTools.sync(UltraBVC, module_name, board_name);
    fprintf('采样率配置和设备同步完成\n');
catch ME
    fprintf('采样率配置或设备同步失败: %s\n', ME.message);
end

%% 触发配置
fprintf('配置触发...\n');
try
    if strcmp(trigger_source, 'Internal')
        UltraBVC.send_command(sprintf(':SAT:TRIGger:SOURce %s,Internal', board_name));
        UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:REPeat %s,%d', board_name, in_trigger_repeat));
        UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:CYCle %s,%.6f', board_name, in_trigger_cycle));

        UltraBVC.send_command(sprintf(':DAQ:TRIGger:SOURce %s,PXISTARTrig', module_name));
        UltraBVC.send_command(sprintf(':DAQ:TRIGger:INTernal:REPeat %s,%d', module_name, in_trigger_repeat));
        UltraBVC.send_command(sprintf(':DAQ:TRIGger:INTernal:CYCle %s,%.6f', module_name, in_trigger_cycle));
    elseif strcmp(trigger_source, 'External')
        UltraBVC.send_command(sprintf(':SAT:TRIGger:SOURce %s,External', board_name));
        UltraBVC.send_command(sprintf(':DAQ:TRIGger:SOURce %s,PXISTARTrig', module_name));
    else
        fprintf('错误: %s 不存在\n', trigger_source);
        return;
    end
    fprintf('触发配置完成\n');
catch ME
    fprintf('触发配置失败: %s\n', ME.message);
end

%% 存储配置
fprintf('配置存储...\n');
try
    save_keys = keys(save_en);
    for i = 1:length(save_keys)
        key = save_keys{i};
        value = save_en(key);

        % 使能数据保存
        UltraBVC.send_command(sprintf(':DAQ:UPSTream:SAVe:EN %s,%s', key, logical_to_string(value)));
        if value
            % 保存路径
            UltraBVC.send_command(sprintf(':DAQ:UPSTream:SAVe:DIRectory %s,%s', key, save_file_path));
            % 单文件大小
            UltraBVC.send_command(sprintf(':DAQ:UPSTream:SAVe:SIZe %s,%d', key, save_file_size));
        end
    end
    fprintf('存储配置完成\n');
catch ME
    fprintf('存储配置失败: %s\n', ME.message);
end

%% 数据采集配置
fprintf('配置数据采集...\n');
try
    channel_keys = keys(channel_en);
    for i = 1:length(channel_keys)
        key = channel_keys{i};
        value = channel_en(key);

        % 通道使能
        UltraBVC.send_command(sprintf(':DAQ:CHANnel:EN %s,%s', key, logical_to_string(value)));
        if value
            % 量化位数
            UltraBVC.send_command(sprintf(':DAQ:CHANnel:QUANtization %s,14', key));
            % 预采样点数
            % pre_pts: 整数RingBuffer模式，点数必须是32768字节的倍数；Stream模式，点数必须是64字节的倍数
            UltraBVC.send_command(sprintf(':DAQ:ACQuire:PRESample %s,%d', key, pre_sample));
            % 后采样点数
            % pst_pts: 整数RingBuffer模式，点数必须是32768字节的倍数；Stream模式，点数必须是64字节的倍数
            UltraBVC.send_command(sprintf(':DAQ:ACQuire:PSTSample %s,%d', key, pst_sample));
        end
    end
    fprintf('数据采集配置完成\n');
catch ME
    fprintf('数据采集配置失败: %s\n', ME.message);
end

%% 启动设备
fprintf('启动设备...\n');
try
    UltraBVC.send_command(sprintf(':SYS:Control:RUN %s', module_name));
    UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:Run %s', board_name));

    fprintf('设备启动成功，开始数据采集\n');
    pause(5);

    % 停止设备
    UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:Stop %s', board_name));
    UltraBVC.send_command(sprintf(':SYS:Control:STOP %s', module_name));

    % 禁用数据保存
    save_keys = keys(save_en);
    for i = 1:length(save_keys)
        key = save_keys{i};
        UltraBVC.send_command(sprintf(':DAQ:UPSTream:SAVe:EN %s,False', key));
    end

    fprintf('数据采集完成\n');
catch ME
    fprintf('设备启动或停止失败: %s\n', ME.message);
end

%% 查看保存的数据
fprintf('查看保存的数据...\n');
try
    % 数据类型映射
    data_type_map = containers.Map('KeyType', 'int32', 'ValueType', 'char');
    data_type_map(3) = 'int8';
    data_type_map(5) = 'int16';
    data_type_map(6) = 'int16'; % QI拼接，每个都是int16
    
    data_type_len_map = containers.Map('KeyType', 'char', 'ValueType', 'int32');
    data_type_len_map('int8') = 1;
    data_type_len_map('int16') = 2;
    
    % 结果保存路径
    result_dir = fullfile(current_dir, 'result');
    if ~exist(result_dir, 'dir')
        mkdir(result_dir);
    end
    
    % % % % % % todo 自动查询保存文件 % % % % % % 
    % % 获取文件列表
    % save_file_path = sprintf('\\\\%s\\UserSpace', device_ip);
    % original_list = dir(save_file_path);
    % original_names = {original_list.name};
    % 
    % % 删除'WaveList'和'NSQC'
    % filtered_list = {};
    % for j = 1:length(original_names)
    %     name = original_names{j};
    %     if ~strcmp(name, 'WaveList') && ~strcmp(name, 'NSQC') && ...
    %        ~strcmp(name, 'lost+found') && ~strcmp(name, 'SUA_Examples')
    %         filtered_list{end+1} = name;
    %     end
    % end
    % 
    % % 按日期和时间从新到旧排序
    % sorted_list = sort(filtered_list, 'descend');
    % 
    % if isempty(sorted_list)
    %     fprintf('没有找到数据文件\n');
    %     return;
    % end
    % 
    % file_name = fullfile(save_file_path, sorted_list{1}, 'CH1_0.data'); % 原始数据路径
    
    % % % % % % todo 手动输入路径 % % % % % % 
    file_name = sprintf('\\\\%s\\UserSpace\\%s\\%s\\CH1_0.data', device_ip,'25-08-14_21-42-09','S4_M3301');
    if ~exist(file_name, 'file')
        fprintf('数据文件不存在: %s\n', file_name);
        return;
    end
   
    % 读取文件
    file_obj = fopen(file_name, 'rb');
    if file_obj == -1
        fprintf('无法打开文件: %s\n', file_name);
        return;
    end
    
    head_length = 32; % 头部长度
    file_info = dir(file_name);
    file_len = file_info.bytes; % 总文件长度
    read_len = 0; % 读取文件长度
    
    fprintf('开始读取数据文件: %s\n', file_name);
    fprintf('文件大小: %d 字节\n', file_len);
    
    for i = 1:show_data_nums
        % 读取头部
        head_data = fread(file_obj, head_length, 'uint8');
        if length(head_data) < head_length
            fprintf('文件读取完毕\n');
            break;
        end
        read_len = read_len + head_length;
        % 解析头部信息
        pack_length = typecast(uint8(head_data(9:12)), 'uint32'); % 数据包长度
        data_length = pack_length - uint32(head_length); % 确保类型匹配
        read_len = read_len + data_length;
        data_type_code = typecast(uint8(head_data(25)), 'int8'); % 数据类型
        % % % % % % todo 处理第二包数据时会有错误 % % % % % % 
        data_type = data_type_map(data_type_code); % 数据类型映射
        prt = typecast(uint8(head_data(5:8)), 'uint32'); % 数据包计数
        % 读取数据
        data = fread(file_obj, data_length, data_type); 
        % 绘制数据
        try
            h_fig = figure('Position', [100, 100, 800, 600]);
            if ~ishandle(h_fig) || ~isvalid(h_fig)
                error('无法创建有效的图形窗口');
            end
            
            plot(data);
            title(sprintf('解包数据 - 包 %d', prt));
            xlabel('采样点');
            ylabel('幅值');
            grid on;
            
            % 保存图像 - 使用明确的句柄而不是gcf
            save_path = fullfile(result_dir, sprintf('unpacked_data_%d.png', prt));
            saveas(h_fig, save_path);
            fprintf('图像已保存到: %s\n', save_path);
            
        catch fig_error
            fprintf('图形处理错误: %s\n', fig_error.message);
            fprintf('跳过图像保存，继续处理数据\n');
        end
        
        % 确保关闭图形窗口
        if exist('h_fig', 'var') && ishandle(h_fig)
            close(h_fig);
        end
        
        fprintf('NO.%d, prt: %d, read_len: %d/%d\n', i, prt, read_len, file_len);
        
        if read_len >= file_len
            break;
        end
    end
    
    fclose(file_obj);
    fprintf('数据查看完成\n');
    
catch ME
    % todo: 查看保存数据失败: 此容器中不存在指定的键。
    fprintf('查看保存数据失败: %s\n', ME.message);
end

% % % % % % todo FFT问题同上 % % % % % %
%% FFT分析
fprintf('进行FFT分析...\n');
try
    % 数据类型映射（重新定义，因为在不同作用域中）
    data_type_map = containers.Map('KeyType', 'int32', 'ValueType', 'char');
    data_type_map(3) = 'int8';
    data_type_map(5) = 'int16';
    data_type_map(6) = 'int16';

    data_type_len_map = containers.Map('KeyType', 'char', 'ValueType', 'int32');
    data_type_len_map('int8') = 1;
    data_type_len_map('int16') = 2;

    % 结果保存路径
    fft_result_dir = fullfile(current_dir, 'ttf_result');
    if ~exist(fft_result_dir, 'dir')
        mkdir(fft_result_dir);
    end

    % % 获取文件列表
    % original_list = dir(save_file_path);
    % original_names = {original_list.name};
    % 
    % % 删除'WaveList'和'NSQC'
    % filtered_list = {};
    % for j = 1:length(original_names)
    %     name = original_names{j};
    %     if ~strcmp(name, 'WaveList') && ~strcmp(name, 'NSQC') && ...
    %        ~strcmp(name, 'lost+found')
    %         filtered_list{end+1} = name;
    %     end
    % end
    % 
    % % 按日期和时间从新到旧排序
    % sorted_list = sort(filtered_list, 'descend');
    % 
    % if isempty(sorted_list)
    %     fprintf('没有找到数据文件进行FFT分析\n');
    %     return;
    % end
    % 
    % file_name = fullfile(save_file_path, sorted_list{1}, 'CH1_0.data'); % 原始数据路径
    file_name = sprintf('\\\\%s\\UserSpace\\%s\\%s\\CH1_0.data', device_ip,'25-08-14_21-42-09','S4_M3301');
    if ~exist(file_name, 'file')
        fprintf('数据文件不存在: %s\n', file_name);
        return;
    end

    % 读取文件
    file_obj = fopen(file_name, 'rb');
    if file_obj == -1
        fprintf('无法打开文件: %s\n', file_name);
        return;
    end

    head_length = 32; % 头部长度
    file_info = dir(file_name);
    file_len = file_info.bytes; % 总文件长度
    read_len = 0; % 读取文件长度

    fprintf('开始FFT分析数据文件: %s\n', file_name);

    for i = 1:show_data_nums
        % 读取头部
        head_data = fread(file_obj, head_length, 'uint8');
        if length(head_data) < head_length
            fprintf('文件读取完毕\n');
            break;
        end
        read_len = read_len + head_length;

        % 解析头部信息
        pack_length = typecast(uint8(head_data(9:12)), 'uint32'); % 数据包长度
        data_length = pack_length - uint32(head_length); % 确保类型匹配
        read_len = read_len + data_length;

        data_type_code = typecast(uint8(head_data(25)), 'int8'); % 数据类型
        data_type = data_type_map(data_type_code); % 数据类型映射
        prt = typecast(uint8(head_data(5:8)), 'uint32'); % 数据包计数

        % 读取数据
        data = fread(file_obj, data_length, data_type);

        % 计算时间轴
        % 确保类型匹配，转换为双精度进行计算
        data_length_double = double(data_length);
        fprintf('data_length: %d, data_length_double: %.2f\n', data_length, data_length_double);

        % 确保类型匹配
        type_len = double(data_type_len_map(data_type));
        fprintf('data_type: %s, type_len: %.2f\n', data_type, type_len);

        % 计算实际的数据点数
        N = floor(data_length_double / type_len); % 点数
        fprintf('N: %d\n', N);

        % 检查数据长度是否匹配
        actual_data_length = length(data);
        fprintf('实际数据长度: %d\n', actual_data_length);

        if N ~= actual_data_length
            fprintf('警告：计算的点数 %d 与实际数据长度 %d 不匹配，使用实际数据长度\n', N, actual_data_length);
            N = actual_data_length;
        end

        % 重新计算持续时间，基于实际数据长度
        duration = (N * type_len) / 4000000000.0; % 持续时间
        fprintf('修正后的持续时间: %.6e 秒\n', duration);

        % 创建时间轴，确保长度与数据匹配
        t = linspace(0, duration, N); % 时间轴
        fprintf('时间轴长度: %d, 数据长度: %d, 时间范围: [%.6e, %.6e]\n', length(t), length(data), t(1), t(end));

        % 验证长度匹配
        if length(t) ~= length(data)
            error('时间轴长度 (%d) 与数据长度 (%d) 不匹配', length(t), length(data));
        end

        % 进行FFT分析
        h_fig = bvcTools.plot_fft(i, data, t, 4000000000, 100000000, 0);


        % 绘制数据
        try
            % 保存图像 - 使用明确的句柄而不是gcf
            save_path = fullfile(fft_result_dir, sprintf('unpacked_data_%d.png', prt));
            saveas(h_fig, save_path);
            fprintf('FFT图像已保存到: %s\n', save_path);

        catch fig_error
            fprintf('FFT图形处理错误: %s\n', fig_error.message);
            fprintf('跳过FFT图像保存，继续处理数据\n');
        end

        % 确保关闭图形窗口
        if exist('h_fig', 'var') && ishandle(h_fig)
            close(h_fig);
        end

        fprintf('NO.%d, prt: %d, read_len: %d/%d\n', i, prt, read_len, file_len);

        if read_len >= file_len
            break;
        end
    end

    fclose(file_obj);
    fprintf('FFT分析完成\n');

catch ME
    fprintf('FFT分析失败: %s\n', ME.message);
end

%% 断开连接
bvcTools.disconnect();
fprintf('\n=== 数字转换器环形缓冲区数据采集完成 ===\n');

end

%% 辅助函数
function str = logical_to_string(logical_val)
    % 将逻辑值转换为字符串
    if logical_val
        str = 'True';
    else
        str = 'False';
    end
end

