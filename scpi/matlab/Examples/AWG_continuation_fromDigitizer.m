function AWG_continuation_fromDigitizer()
    % AWG_continuation_fromDigitizer - MATLAB版本AWG从数字转换器数据连续波形生成
    % 功能：读取数字转换器数据、生成波形、设备连接、波形上传、配置和控制
    % 作者：基于Python版本转换
    % 日期：2024
    
    fprintf('=== AWG从数字转换器数据连续波形生成 (MATLAB版本) ===\n');
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
        generator = SignalGenerator();
        analyzer = SignalAnalyzer(generator);
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
    
    % 显示通道
    show_channel = "CH1";
    
    % DUC参数
    DUC_en = containers.Map();
    DUC_en('CH1') = false;
    DUC_en('CH2') = false;
    internal_multiple = 2; % 插值倍数
    NCO_freq = 0.0; % NCO频率
    
    % 显示波形数量
    show_data_nums = 1;
    
    % 文件路径
    % save_file_path = "/UserSpace";
    
    %% 数据类型映射
    data_type_map = containers.Map('KeyType', 'int32', 'ValueType', 'char');
    data_type_map(3) = 'int8';
    data_type_map(5) = 'int16';
    data_type_map(6) = 'int16'; % QI拼接，每个都是int16
    
    data_type_len_map = containers.Map('KeyType', 'char', 'ValueType', 'int32');
    data_type_len_map('int8') = 1;
    data_type_len_map('int16') = 2;
    
    %% 结果保存路径
    result_dir = fullfile(current_dir, 'Digitizer_result');
    if ~exist(result_dir, 'dir')
        mkdir(result_dir);
    end
    
    %% 获取文件列表
    fprintf('获取数字转换器数据文件...\n');
    try
        % original_list = dir(save_file_path);
        % original_names = {original_list.name};
        % 
        % % 删除'WaveList'和'NSQC'
        % filtered_list = {};
        % for j = 1:length(original_names)
        %     name = original_names{j};
        %     if ~strcmp(name, 'WaveList') && ~strcmp(name, 'NSQC')
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
        
        % % 根据DUC使能状态选择文件
        % if DUC_en(show_channel)
        %     file_name = fullfile(save_file_path, sorted_list{1}, sprintf('%s_IQ_0.data', show_channel));
        % else
        %     file_name = fullfile(save_file_path, sorted_list{1}, sprintf('%s_0.data', show_channel));
        % end
        
        % fprintf('选择数据文件: %s\n', file_name);
        
        file_name = sprintf('\\\\%s\\UserSpace\\%s\\CH1_0.data', device_ip,'1');
        
        if ~exist(file_name, 'file')
            fprintf('数据文件不存在: %s\n', file_name);
            return;
        end
        
    catch ME
        fprintf('获取文件列表失败: %s\n', ME.message);
        return;
    end
    
    %% 读取数字转换器数据并生成波形文件
    fprintf('读取数字转换器数据并生成波形文件...\n');
    try
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
                title(sprintf('解包数据 %d', i));
                xlabel('采样点');
                ylabel('幅值');
                grid on;

            catch fig_error
                fprintf('图形处理错误: %s\n', fig_error.message);
            end

            % 确保关闭图形窗口
            if exist('h_fig', 'var') && ishandle(h_fig)
                close(h_fig);
            end

            % 调整数据长度以满足16384的倍数要求
            current_length = length(data);
            target_length = ceil((current_length - 1) / 16384) * 16384;
            
            if current_length < target_length
                padding_length = target_length - current_length;
                fprintf('需要填充的零数量: %d\n', padding_length);

                % 使用零填充
                signal_data = [data; zeros(padding_length, 1)];
            else
                signal_data = data;
            end

            % 保存波形文件
            file_path = fullfile(result_dir, sprintf('wave_%d.bin', i-1));
            fileID = fopen(file_path, 'wb');
            if fileID == -1
                fprintf('无法创建文件: %s\n', file_path);
                continue;
            end

            % 写入int16数据
            fwrite(fileID, int16(signal_data), 'int16');
            fclose(fileID);

            fprintf('%s 波形文件保存成功\n', file_path);
            
            fprintf('NO.%d, prt: %d, read_len: %d/%d\n', i, prt, read_len, file_len);
            
            if read_len >= file_len
                break;
            end
        end
        
        fclose(file_obj);
        fprintf('数据读取和波形文件生成完成\n');
        
    catch ME
        fprintf('读取数据失败: %s\n', ME.message);
        if exist('file_obj', 'var') && file_obj ~= -1
            fclose(file_obj);
        end
        return;
    end
    
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
        fprintf('注意：此示例需要实际的AWG设备连接\n');
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
    module_name = 'S2_M4901';
    board_name = 'S0_M1210';
    
    % 通道使能
    channel_en = containers.Map();
    channel_en('CH1') = true;
    channel_en('CH2') = true;
    
    %% DUC相关配置
    fprintf('配置DUC...\n');
    try
        UltraBVC.send_command(sprintf(':AWG:SOURce:WAVE:InternalMultiple %s,%d', module_name, internal_multiple));
        
        channel_keys = keys(DUC_en);
        for i = 1:length(channel_keys)
            key = channel_keys{i};
            value = DUC_en(key);
            
            UltraBVC.send_command(sprintf(':AWG:OUTPut:DUC:EN %s,True', key));
            if value
                UltraBVC.send_command(sprintf(':AWG:OUTPut:DUC:NCOFrequency %s,%.6f', key, NCO_freq));
            end
        end
        fprintf('DUC配置完成\n');
    catch ME
        fprintf('DUC配置失败: %s\n', ME.message);
    end
    
    %% 配置采样率并同步
    fprintf('配置采样率并同步设备...\n');
    try
        UltraBVC.send_command(sprintf(':AWG:SOURce:DAC:SRATe %s,%d', module_name, sample_rate));
        bvcTools.sync(UltraBVC, module_name, board_name);
        fprintf('采样率配置和设备同步完成\n');
    catch ME
        fprintf('采样率配置或设备同步失败: %s\n', ME.message);
    end
    
    %% 上传波形文件
    fprintf('上传波形文件...\n');
    try
        seg_list = {};
        
        % 查找所有.bin文件
        file_pattern = fullfile(result_dir, '*.bin');
        bin_files = dir(file_pattern);
        
        if isempty(bin_files)
            fprintf('在目录 %s 中没有找到.bin文件\n', result_dir);
            return;
        end
        
        fprintf('找到 %d 个波形文件\n', length(bin_files));
        
        for i = 1:length(bin_files)
            file_path = fullfile(bin_files(i).folder, bin_files(i).name);
            fprintf('%d. %s\n', i-1, bin_files(i).name);
            
            
            % 获取文件大小
            file_info = dir(file_path);
            file_len = file_info.bytes/2;
            wave_list = sprintf('seg%d',i-1);
            
            % 添加波形列表
            UltraBVC.send_command(sprintf(':AWG:WAVList:ADDList %s,%s,%d,Int16', module_name, wave_list, file_len));
            
            % 上传波形数据
            try
                % 使用HTTP POST上传波形文件
                upload_success = upload_waveform_http(device_ip, module_name, wave_list, file_path, file_len);
                
                if upload_success
                    fprintf('波形文件上传成功: %s\n', bin_files(i).name);
                    seg_list{1} = wave_list;
                else
                    fprintf('波形文件上传失败: %s\n', bin_files(i).name);
                    return;
                end

            catch upload_error
                fprintf('波形上传失败: %s\n', upload_error.message);
            end
        end
        
        fprintf('波形文件上传完成\n');
        
    catch ME
        fprintf('波形文件上传失败: %s\n', ME.message);
    end
    
    %% 创建NSWave序列
    fprintf('创建NSWave序列...\n');
    try
        % 子循环
        NSWave_name = "sequence6";
        
        % 创建波形表
        UltraBVC.send_command(sprintf(':AWG:NSQC:ADD %s', NSWave_name));

        % 构建NSQC代码
        nsqc = sprintf("@nw.kernel\ndef program(wlist: dict[str, np.ndarray]):");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n    seg%d: nw.ArbWave = nw.init_arbwave(wlist, 'seg%d')", i-1, i-1);
        end

        nsqc = nsqc + sprintf("\n    while True:");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n        nw.play_arb(seg%d)  # Play segment", i-1);
        end

        nsqc = nsqc + sprintf("\n    return nw.Kernel()");
        
        fprintf('NSQC代码:\n%s\n', nsqc);
        
        % 上传NSWave
        upload_nswave_success = upload_nswave_http(device_ip, NSWave_name, nsqc);
        if ~upload_nswave_success
            fprintf('NSWave上传失败\n');
            return;
        end
        fprintf('NSWave上传完成\n');
        
        % 编译
        channel_keys = keys(channel_en);
        for i = 1:length(channel_keys)
            key = channel_keys{i};
            value = channel_en(key);
            
            if value
                UltraBVC.send_command(sprintf(':AWG:NSQC:COMPile %s,%s', key, NSWave_name));
            end
        end
        
        % 部署
        UltraBVC.send_command(sprintf(':AWG:WAVList:NSQC:COMPile:Send %s', module_name));
        
        fprintf('NSWave序列创建完成\n');
        
    catch ME
        fprintf('NSWave序列创建失败: %s\n', ME.message);
    end
    
    %% 配置使能并启动
    fprintf('配置使能并启动设备...\n');
    try
        channel_keys = keys(channel_en);
        for i = 1:length(channel_keys)
            key = channel_keys{i};
            value = channel_en(key);
            
            UltraBVC.send_command(sprintf(':AWG:OUTPut:EN %s,%s', key, logical_to_string(value)));
        end
        
        UltraBVC.send_command(sprintf(':SYS:Control:RUN %s', module_name));
        UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:Run %s', board_name));
        
        fprintf('设备启动成功，开始波形播放\n');
        
        % 等待10秒
        pause(10);
        
        % 停止设备
        UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:Stop %s', board_name));
        UltraBVC.send_command(sprintf(':SYS:Control:STOP %s', module_name));
        
        fprintf('设备停止完成\n');
        
    catch ME
        fprintf('设备启动或停止失败: %s\n', ME.message);
    end
    
    %% 断开连接
    bvcTools.disconnect();
    fprintf('\n=== AWG从数字转换器数据连续波形生成完成 ===\n');

end
