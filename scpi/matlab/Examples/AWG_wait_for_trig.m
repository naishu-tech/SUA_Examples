function AWG_wait_for_trig()
    % AWG_wait_for_trig - MATLAB版本AWG等待触发波形生成和控制
    % 功能：生成波形、等待外部触发、设备连接、波形上传、配置和控制
    % 作者：基于Python版本转换
    % 日期：2024
    
    fprintf('=== AWG等待触发波形生成和控制 (MATLAB版本) ===\n');
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
    
    % 采样率
    sample_rate = 4; % 4GHz采样率
    
    % 波形类型
    waveform_type = "sine"; % sine/cose/multi_tone/square/pulse/triangle/chirp
    
    %% 触发参数
    trigger_source = "Internal"; % Internal/External
    in_trigger_repeat = hex2dec('FFFFFFFF');
    in_trigger_cycle = 0.001;
    
    %% 配置波形参数
    waveParams = SignalParams(waveform_type, 0.0001024, sample_rate * 10^9, 0.8, 100e6, 0.0, 0.0);
    
    % 设置其他参数
    waveParams.duty_cycle = 0.5;                % 50%占空比
    waveParams.frequencies = [100e6, 300e6, 400e6]; % 频率组合
    waveParams.amplitudes = [0.6, 0.4, 0.2];   % 递减幅度
    waveParams.phases = [0.0, 0.0, 0.0];        % 相同相位
    waveParams.pulse_width = 1e-8;              % 10ns脉冲宽度
    waveParams.pulse_period = 1e-7;             % 100ns脉冲周期
    waveParams.f0 = 100e6;                      % 起始频率100MHz
    waveParams.f1 = 500e6;                      % 结束频率500MHz
    
    %% 调整持续时间以满足16384的倍数要求
    duration_samples = round(waveParams.sample_rate * waveParams.duration);
    aligned_samples = ceil((duration_samples - 1) / 16384) * 16384;
    waveParams.duration = aligned_samples / waveParams.sample_rate;
    
    fprintf('调整后的持续时间: %.9f 秒\n', waveParams.duration);
    fprintf('样本数: %d\n', aligned_samples);
    
    %% 生成信号
    fprintf('生成信号...\n');
    try
        [t, signal_data] = generator.generate_signal(waveParams);
        waveParams.wave_len = length(signal_data);
        
        % 计算显示点数
        show_pts = round(waveParams.sample_rate / waveParams.frequency) * 20;
        show_pts = min(show_pts, length(signal_data));
        
        % 显示信号
        analyzer.plot_signal_inline(t(1:show_pts), signal_data(1:show_pts), waveParams, false);
        
        % 统计分析
        stats = analyzer.analyze_signal_statistics(signal_data, waveParams);
        
        fprintf('信号生成成功，长度: %d 点\n', length(signal_data));
        
    catch ME
        fprintf('信号生成失败: %s\n', ME.message);
        return;
    end
    
    %% 保存波形文件
    wave_file_path = fullfile(current_dir, 'wave_file');
    if ~exist(wave_file_path, 'dir')
        mkdir(wave_file_path);
    end
    
    file_name = sprintf('wave_%s_%d_%.0f_%.1f_%.0f_%.1f_%.1f.bin', ...
        waveParams.waveform_type, length(signal_data), waveParams.sample_rate, ...
        waveParams.amplitude, waveParams.frequency, waveParams.phase, waveParams.dc_offset);
    file_path = fullfile(wave_file_path, file_name);
    
    % 保存为二进制文件
    try
        fid = fopen(file_path, 'wb');
        if fid == -1
            error('无法创建波形文件');
        end
        fwrite(fid, signal_data, 'double');
        fclose(fid);
        fprintf('%s 波形文件保存成功\n', file_path);
    catch ME
        fprintf('波形文件保存失败: %s\n', ME.message);
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
        UltraBVC.send_command(sprintf(':AWG:SOURce:DAC:SRATe %s,%d', module_name, sample_rate));
        bvcTools.sync(UltraBVC, module_name, board_name);
        fprintf('采样率配置和设备同步完成\n');
    catch ME
        fprintf('采样率配置或设备同步失败: %s\n', ME.message);
    end
    
    %% 上传波形文件
    fprintf('上传波形文件...\n');
    try
        wave_list = "seg2";
        UltraBVC.send_command(sprintf(':AWG:WAVList:ADDList %s,%s,%d,Double', module_name, wave_list, length(signal_data)));
        
        % 使用HTTP POST上传波形文件
        upload_success = upload_waveform_http(device_ip, module_name, wave_list, file_path, length(signal_data));
        
        if upload_success
            fprintf('波形文件上传成功\n');
        else
            fprintf('波形文件上传失败\n');
            return;
        end
        
    catch ME
        fprintf('波形文件上传失败: %s\n', ME.message);
        return;
    end
    
    %% 创建和上传NSWave
    fprintf('创建和上传NSWave...\n');
    try
        NSWave_name = "sequence2";
        
        % 创建wave table
        UltraBVC.send_command(sprintf(':AWG:NSQC:ADD %s', NSWave_name));
        
        % 准备NSWave数据 - 等待外部触发播放
        % nw.wait_for_trigger_with_source 将修改逻辑触发源
        nswave_program = ['@nw.kernel' newline ...
                         'def program(wlist: dict[str, np.ndarray]):' newline ...
                         '    seg2: nw.ArbWave = nw.init_arbwave(wlist, ''seg2'')' newline ...
                         '    while True:' newline ...
                         '        nw.wait_for_trigger_with_source(1)' newline ...
                         '        nw.play_arb(seg2)' newline ...
                         '    return nw.Kernel()' newline ...
                         ];
        
        % 上传NSWave
        upload_nswave_success = upload_nswave_http(device_ip, NSWave_name, nswave_program);
    
        if upload_nswave_success
            fprintf('NSWave上传成功\n');
        else
            fprintf('NSWave上传失败\n');
            return;
        end
        
        % 编译
        channel_keys = keys(channel_en);
        for i = 1:length(channel_keys)
            key = channel_keys{i};
            if channel_en(key)
                UltraBVC.send_command(sprintf(':AWG:NSQC:COMPile %s,%s', key, NSWave_name));
            end
        end
        
        % 部署
        UltraBVC.send_command(sprintf(':AWG:WAVList:NSQC:COMPile:Send %s', module_name));
        fprintf('NSWave编译和部署完成\n');
        
    catch ME
        fprintf('NSWave创建和上传失败: %s\n', ME.message);
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
        
        % 启动内部触发
        UltraBVC.send_command(sprintf(':SAT:TRIGger:INTernal:Run %s', board_name));
        fprintf('设备启动成功，等待外部触发\n');
        
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
    fprintf('\n=== AWG等待触发波形控制完成 ===\n');

end
