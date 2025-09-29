function Digitizer_quan()
% Digitizer_quan - MATLAB版本数字转换器量化位数配置数据采集
% 功能：配置数字转换器、量化位数、数据采集、数据保存和分析
% 作者：基于Python版本转换
% 日期：2024

fprintf('=== 数字转换器量化位数配置数据采集 (MATLAB版本) ===\n');
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
    fprintf('错误信息: %s\n', ME.message);
    return;
end

%% 配置设备参数
device_ip = "172.16.152.94";

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

% 显示波形数量
show_data_nums = 1;

% 量化位数
quan = 14;

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

    fprintf('设备上的所有模块的功能和通道...\n');
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
module_name = 'S2_M3302';
board_name = 'S0_M1210';

% 通道使能
channel_en = containers.Map();
channel_en('CH1') = true;
channel_en('CH2') = true;

%% 关闭DUC
fprintf('关闭DUC...\n');
try
    UltraBVC.send_command(sprintf(':DAQ:ACQuire:ExtractMultiple %s,1', module_name));
    
    channel_keys = keys(channel_en);
    for i = 1:length(channel_keys)
        key = channel_keys{i};
        value = channel_en(key);
        
        UltraBVC.send_command(sprintf(':DAQ:CHANnel:DDC:EN %s,False', key));
    end
    
    fprintf('DUC关闭完成\n');
catch ME
    fprintf('DUC关闭失败: %s\n', ME.message);
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
            UltraBVC.send_command(sprintf(':DAQ:CHANnel:QUANtization %s,%d', key, quan));
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

%% 断开连接
bvcTools.disconnect();
fprintf('\n=== 数字转换器量化位数配置数据采集完成 ===\n');

end
