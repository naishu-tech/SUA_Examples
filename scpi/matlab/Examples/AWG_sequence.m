function AWG_sequence()
    % AWG_sequence - MATLAB版本AWG序列波形生成和控制
    % 功能：读取多个波形文件、创建序列、设备连接、波形上传、配置和控制
    % 作者：基于Python版本转换
    % 日期：2024
    
    fprintf('=== AWG序列波形生成和控制 (MATLAB版本) ===\n');
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
    
    % 创建信号生成器和分析器实例
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
    device_ip = "172.16.152.83";
    
    %% 触发参数
    trigger_source = "Internal"; % Internal/External
    in_trigger_repeat = hex2dec('FFFFFFFF');
    in_trigger_cycle = 0.001;
    
    %% 读取波形文件
    fprintf('读取波形文件...\n');
    directory_path = fullfile(current_dir, 'seq_file');
    file_pattern = fullfile(directory_path, '*.bin');
    bin_files = dir(file_pattern);
    
    if isempty(bin_files)
        fprintf('在目录 %s 中没有找到.bin文件\n', directory_path);
        return;
    end
    
    fprintf('找到 %d 个波形文件\n', length(bin_files));
    
    %% 显示所有波形
    fprintf('显示所有波形...\n');
    num_files = length(bin_files);
    figure('Position', [100, 100, 1200, 300*num_files]);
    
    for i = 1:num_files
        try
            file_path = fullfile(bin_files(i).folder, bin_files(i).name);
            
            % 读取二进制文件
            fid = fopen(file_path, 'rb');
            if fid == -1
                fprintf('无法打开文件: %s\n', file_path);
                continue;
            end
            data = fread(fid, inf, 'double');
            fclose(fid);
            
            % 提取文件名和波形类型
            [~, file_name, ~] = fileparts(bin_files(i).name);
            parts = strsplit(file_name, '_');
            if length(parts) >= 2
                wave_type = parts{2};
            else
                wave_type = 'unknown';
            end
            
            % 绘制波形
            subplot(num_files, 1, i);
            display_len = min(length(data), 5000); % 显示前5000个样本
            time_axis = 1:display_len;
            
            plot(time_axis, data(1:display_len), 'LineWidth', 0.8);
            title(sprintf('%s Wave - %s', upper(wave_type), bin_files(i).name));
            xlabel('Sample Index');
            ylabel('Amplitude');
            grid on;
            
            % 添加数据信息
            info_text = sprintf('Length: %d, Range: [%.3f, %.3f]', length(data), min(data), max(data));
            text(0.02, 0.98, info_text, 'Units', 'normalized', 'VerticalAlignment', 'top', ...
                'BackgroundColor', 'yellow', 'EdgeColor', 'black', 'Margin', 2);
            
            fprintf('✓ 成功读取 %s: %d 个样本\n', bin_files(i).name, length(data));
            
        catch ME
            fprintf('✗ 读取文件失败 %s: %s\n', file_path, ME.message);
            continue;
        end
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
        % 设备名称
        fprintf('step1: 获取设备名称...\n');
        device_name_str = UltraBVC.recv_command(':SYS:DEVice:NAMe?');
        device_name = strsplit(device_name_str, ',');
        
        % 设备上的所有模块名称
        fprintf('step2: 设备上的所有模块名称...\n');
        module_names_str = UltraBVC.recv_command(':SYS:DEVice:MODUles?');
        module_names = strsplit(module_names_str, ',');
        
        model_info = containers.Map();
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
    module_name = 'S2_M6201';
    board_name = 'S0_M1210';
    
    % 通道使能
    channel_en = containers.Map();
    channel_en('CH1') = true;
    channel_en('CH2') = true;
    
    %% 关闭DUC
    fprintf('配置AWG设置...\n');
    try
        UltraBVC.send_command(sprintf(':AWG:SOURce:WAVE:InternalMultiple %s,1', module_name));
        
        % 关闭DUC
        channel_keys = keys(channel_en);
        for i = 1:length(channel_keys)
            key = channel_keys{i};
            if channel_en(key)
                UltraBVC.send_command(sprintf(':AWG:OUTPut:DUC:EN %s,False', key));
            end
        end
        
        fprintf('AWG设置配置完成\n');
    catch ME
        fprintf('AWG设置配置失败: %s\n', ME.message);
    end
    
    %% 配置采样率并同步
    fprintf('配置采样率并同步设备...\n');
    try
        UltraBVC.send_command(sprintf(':AWG:SOURce:DAC:SRATe %s,4', module_name));
        bvcTools.sync(UltraBVC, module_name, board_name);
        fprintf('采样率配置和设备同步完成\n');
    catch ME
        fprintf('采样率配置或设备同步失败: %s\n', ME.message);
    end
    
    %% 上传波形文件
    fprintf('上传波形文件...\n');
    seg_list = {};
    
    for i = 1:length(bin_files)
        fprintf('%d. %s\n', i-1, bin_files(i).name);
        
        try
            file_path = fullfile(bin_files(i).folder, bin_files(i).name);
            file_info = dir(file_path);
            file_len = file_info.bytes; % 文件总长度
            
            seg_name = sprintf('seg%d', i-1);
            seg_list{end+1} = seg_name;
            
            UltraBVC.send_command(sprintf(':AWG:WAVList:ADDList %s,%s,%d,Double', module_name, seg_name, file_len/8));
            
            % 使用HTTP POST上传波形文件
            upload_success = upload_waveform_http(device_ip, module_name, seg_name, file_path, file_len/8);
            
            if ~upload_success
                fprintf('波形文件上传失败: %s\n', bin_files(i).name);
                return;
            end
            
        catch ME
            fprintf('上传波形文件失败: %s\n', ME.message);
            return;
        end
    end
    
    %% 创建序列1：播放一次
    fprintf('创建序列1：播放一次...\n');
    try
        NSWave_name = "sequence3";
        UltraBVC.send_command(sprintf(':AWG:NSQC:ADD %s', NSWave_name));

        % 构建NSQC代码
        nsqc = sprintf("@nw.kernel\ndef program(wlist: dict[str, np.ndarray]):");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n    seg%d: nw.ArbWave = nw.init_arbwave(wlist, 'seg%d')", i-1, i-1);
        end

        nsqc = nsqc + sprintf("\n    while True:");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n        nw.wait_for_trigger_with_source(1)  # Wait for trigger");
            nsqc = nsqc + sprintf("\n        nw.play_arb(seg%d)  # Play segment", i-1);
        end

        nsqc = nsqc + sprintf("\n    return nw.Kernel()");
        
        fprintf('NSWave程序:\n%s\n', nsqc);
        
        % 上传NSWave
        upload_nswave_success = upload_nswave_http(device_ip, NSWave_name, nsqc);
        
        if upload_nswave_success
            fprintf('序列1上传成功\n');
        else
            fprintf('序列1上传失败\n');
            return;
        end
        
        % 编译和部署
        channel_keys = keys(channel_en);
        for j = 1:length(channel_keys)
            key = channel_keys{j};
            if channel_en(key)
                UltraBVC.send_command(sprintf(':AWG:NSQC:COMPile %s,%s', key, NSWave_name));
            end
        end
        UltraBVC.send_command(sprintf(':AWG:WAVList:NSQC:COMPile:Send %s', module_name));
        
    catch ME
        fprintf('创建序列1失败: %s\n', ME.message);
        return;
    end
    
    %% 创建序列2：延迟播放
    fprintf('创建序列2：延迟播放...\n');
    try
        NSWave_name = "sequence4";
        UltraBVC.send_command(sprintf(':AWG:NSQC:ADD %s', NSWave_name));

        % 构建NSQC代码
        nsqc = sprintf("@nw.kernel\ndef program(wlist: dict[str, np.ndarray]):");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n    seg%d: nw.ArbWave = nw.init_arbwave(wlist, 'seg%d')", i-1, i-1);
        end

        nsqc = nsqc + sprintf("\n    while True:");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n        nw.wait_for_trigger_with_source(1)  # Wait for trigger");
            nsqc = nsqc + sprintf("\n        nw.wait(%.6f)  # Delay one-tenth of the cycle", in_trigger_cycle/10);
            nsqc = nsqc + sprintf("\n        nw.play_arb(seg%d)  # Play segment", i-1);
        end

        nsqc = nsqc + sprintf("\n    return nw.Kernel()");

        
        fprintf('NSWave程序:\n%s\n', nsqc);
        
        % 上传NSWave
        upload_nswave_success = upload_nswave_http(device_ip, NSWave_name, nsqc);
        
        if upload_nswave_success
            fprintf('序列2上传成功\n');
        else
            fprintf('序列2上传失败\n');
            return;
        end
        
        % 编译和部署
        channel_keys = keys(channel_en);
        for j = 1:length(channel_keys)
            key = channel_keys{j};
            if channel_en(key)
                UltraBVC.send_command(sprintf(':AWG:NSQC:COMPile %s,%s', key, NSWave_name));
            end
        end
        UltraBVC.send_command(sprintf(':AWG:WAVList:NSQC:COMPile:Send %s', module_name));
        
    catch ME
        fprintf('创建序列2失败: %s\n', ME.message);
        return;
    end
    
    %% 创建序列3：循环播放
    fprintf('创建序列3：循环播放...\n');
    try
        NSWave_name = "sequence5";
        UltraBVC.send_command(sprintf(':AWG:NSQC:ADD %s', NSWave_name));
        
        % 构建NSQC代码
        nsqc = sprintf("@nw.kernel\ndef program(wlist: dict[str, np.ndarray]):");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n    seg%d: nw.ArbWave = nw.init_arbwave(wlist, 'seg%d')", i-1, i-1);
        end

        nsqc = nsqc + sprintf("\n    i:nw.Reg");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n    for i in nw.loop(10):  # Loop iteration");
            nsqc = nsqc + sprintf("\n        nw.wait_for_trigger_with_source(1)  # Wait for trigger");
            nsqc = nsqc + sprintf("\n        nw.play_arb(seg%d)  # Play segment", i-1);
        end

        nsqc = nsqc + sprintf("\n    return nw.Kernel()");
        
        
        fprintf('NSWave程序:\n%s\n', nsqc);
        
        % 上传NSWave
        upload_nswave_success = upload_nswave_http(device_ip, NSWave_name, nsqc);
        
        if upload_nswave_success
            fprintf('序列3上传成功\n');
        else
            fprintf('序列3上传失败\n');
            return;
        end
        
        % 编译和部署
        channel_keys = keys(channel_en);
        for j = 1:length(channel_keys)
            key = channel_keys{j};
            if channel_en(key)
                UltraBVC.send_command(sprintf(':AWG:NSQC:COMPile %s,%s', key, NSWave_name));
            end
        end
        UltraBVC.send_command(sprintf(':AWG:WAVList:NSQC:COMPile:Send %s', module_name));
        
    catch ME
        fprintf('创建序列3失败: %s\n', ME.message);
        return;
    end
    
    %% 创建序列4：子循环
    fprintf('创建序列4：子循环...\n');
    try
        NSWave_name = "sequence6";
        UltraBVC.send_command(sprintf(':AWG:NSQC:ADD %s', NSWave_name));
        
        % 构建NSQC代码
        nsqc = sprintf("@nw.kernel\ndef program(wlist: dict[str, np.ndarray]):");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n    seg%d: nw.ArbWave = nw.init_arbwave(wlist, 'seg%d')", i-1, i-1);
        end

        nsqc = nsqc + sprintf("\n    i:nw.Reg");
        nsqc = nsqc + sprintf("\n    while True:");

        for i = 1:length(seg_list)
            nsqc = nsqc + sprintf("\n        for i in nw.loop(10):  # Loop iteration");
            nsqc = nsqc + sprintf("\n            nw.wait_for_trigger_with_source(1)  # Wait for trigger");
            nsqc = nsqc + sprintf("\n            nw.play_arb(seg%d)  # Play segment", i-1);
        end

        nsqc = nsqc + sprintf("\n    return nw.Kernel()");
        
        
        fprintf('NSWave程序:\n%s\n', nsqc);
        
        % 上传NSWave
        upload_nswave_success = upload_nswave_http(device_ip, NSWave_name, nsqc);
        
        if upload_nswave_success
            fprintf('序列4上传成功\n');
        else
            fprintf('序列4上传失败\n');
            return;
        end
        
        % 编译和部署
        channel_keys = keys(channel_en);
        for j = 1:length(channel_keys)
            key = channel_keys{j};
            if channel_en(key)
                UltraBVC.send_command(sprintf(':AWG:NSQC:COMPile %s,%s', key, NSWave_name));
            end
        end
        UltraBVC.send_command(sprintf(':AWG:WAVList:NSQC:COMPile:Send %s', module_name));
        
    catch ME
        fprintf('创建序列4失败: %s\n', ME.message);
        return;
    end
    
    %% 配置触发
    fprintf('配置触发...\n');
    try
        if strcmp(trigger_source, "Internal")
            UltraBVC.send_command(sprintf(':SAT:TRIGger:SOURce %s,Internal', board_name));
            UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:REPeat %s,%d', board_name, in_trigger_repeat));
            UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:CYCle %s,%.6f', board_name, in_trigger_cycle));
            
            UltraBVC.send_command(sprintf(':AWG:TRIGger:SOURce %s,PXISTARTrig', module_name));
            UltraBVC.send_command(sprintf(':AWG:TRIGger:INTernal:REPeat %s,%d', module_name, in_trigger_repeat));
            UltraBVC.send_command(sprintf(':AWG:TRIGger:INTernal:CYCle %s,%.6f', module_name, in_trigger_cycle));
        elseif strcmp(trigger_source, "External")
            UltraBVC.send_command(sprintf(':SAT:TRIGger:SOURce %s,External', board_name));
            UltraBVC.send_command(sprintf(':AWG:TRIGger:SOURce %s,PXISTARTrig', module_name));
        else
            fprintf('错误: %s 不存在\n', trigger_source);
        end
        
        fprintf('触发配置完成\n');
    catch ME
        fprintf('触发配置失败: %s\n', ME.message);
    end
    
    %% 启动设备
    fprintf('启动设备...\n');
    try
        % 配置使能
        channel_keys = keys(channel_en);
        for i = 1:length(channel_keys)
            key = channel_keys{i};
            UltraBVC.send_command(sprintf(':AWG:OUTPut:EN %s,%s', key, logical_to_string(channel_en(key))));
        end
        
        % 运行AWG
        UltraBVC.send_command(sprintf(':SYS:Control:RUN %s', module_name));
        pause(5);
        
        % 启动内部触发
        UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:Run %s', board_name));
        fprintf('设备启动成功，开始生成序列波形\n');
        
    catch ME
        fprintf('设备启动失败: %s\n', ME.message);
        return;
    end
    
    %% 运行一段时间
    fprintf('运行10秒...\n');
    pause(10);
    
    %% 停止设备
    fprintf('停止设备...\n');
    try
        UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:Stop %s', board_name));
        UltraBVC.send_command(sprintf(':SYS:Control:STOP %s', module_name));
        fprintf('设备停止成功\n');
    catch ME
        fprintf('设备停止失败: %s\n', ME.message);
    end
    
    %% 断开连接
    bvcTools.disconnect();
    fprintf('\n=== AWG序列波形控制完成 ===\n');

end
