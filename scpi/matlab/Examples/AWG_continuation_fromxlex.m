function AWG_continuation_fromxlex()
    % AWG_continuation_fromxlex - MATLAB版本AWG从预定义波形数据连续波形生成
    % 功能：使用预定义的波形数据、设备连接、波形上传、配置和控制
    % 作者：基于Python版本转换
    % 日期：2024
    
    fprintf('=== AWG从预定义波形数据连续波形生成 (MATLAB版本) ===\n');
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
    
    % 波形类型
    wave_type = "Int16"; % Int16/Float/Double/IQ(Int16)/IQ(Float32)/IQ(Double)
    
    %% 预定义波形数据
    wave = [13312, 14784, 14784, 2048, 2048, 14912, 14912, 10304, 10304, 1536, 1536, 4544, 4544, 8960, 8960, 15680, 15680, 15808, 15808, 2560, 2560, 15872, 15872, 15680, 15680, 7936, 7936, 13056, 13056, 2304, 2304, 6848, 6848, 14976, 14976, 12928, 12928, 15680, 15680, 10688, 10688, 576, 576, 13888, 13888, 15296, 15296, 11072, 11072, 12352, 12352, 12160, 12160, 6400, 6400, 10688, 10688, 2752, 2752, 11520, 11520, 512, 512, 4480, 4480, 704, 704, 1536, 1536, 13440, 13440, 11328, 11328, 5184, 5184, 15552, 15552, 512, 512, 7168, 7168, 6208, 6208, 12480, 12480, 12992, 12992, 3008, 3008, 8000, 8000, 7296, 7296, 10560, 10560, 11584, 11584, 12352, 12352, 4480, 4480, 11136, 11136, 10688, 10688, 2624, 2624, 1920, 1920, 8128, 8128, 15680, 15680, 5568, 5568, 9536, 9536, 3648, 3648, 12288, 12288, 4160, 4160, 8256, 8256, 11392, 11392, 14592, 14592, 15680, 15680, 8960, 8960, 2240, 2240, 2432, 2432, 4160, 4160, 13760, 13760, 4160, 4160, 13312, 13312, 3968, 3968, 15168, 15168, 5696, 5696, 3200, 3200, 4096, 4096, 10048, 10048, 7744, 7744, 5760, 5760, 13568, 13568, 9536, 9536, 8960, 8960, 14976, 14976, 4672, 4672, 12352, 12352, 12288, 12288, 6208, 6208, 9280, 9280, 1216, 1216, 832, 832, 8640, 8640, 12736, 12736, 15296, 15296, 2112, 2112, 9280, 9280, 7680, 7680, 192, 192, 5504, 5504, 2624, 2624, 12992, 12992, 5056, 5056, 8640, 8640, 2688, 2688, 9856, 9856, 4288, 4288, 10688, 10688, 11264, 11264, 12224, 12224, 7360, 7360, 1344, 1344, 3712, 3712, 14912, 14912, 2496, 2496, 13504, 13504, 8768, 8768, 16320, 16320, 1280, 1280, 7232, 7232, 1728, 1728, 15744, 15744, 64, 64, 12672, 12672, 13376, 13376, 14208, 14208, 1344, 1344, 6528, 6528, 4224, 4224, 13056, 13056, 7040, 7040, 14912, 14912, 2944, 2944, 4288, 4288, 2368, 2368, 2176, 2176, 14208, 14208, 9472, 9472, 8960, 8960, 2368, 2368, 13952, 13952, 10176, 10176, 5696, 5696, 8384, 8384, 6528, 6528, 1216, 1216, 3904, 3904, 1984, 1984, 3008, 3008, 3904, 3904, 6784, 6784, 768, 768, 14784, 14784, 15424, 15424, 8000, 8000, 8000, 8000, 5504, 5504, 14720, 14720, 6016, 6016, 1792, 1792, 12736, 12736, 6336, 6336, 3904, 3904, 6592, 6592, 1536, 1536, 2112, 2112, 15424, 15424, 15616, 15616, 9408, 9408, 960, 960, 3840, 3840, 5760, 5760, 13440, 13440, 192, 192, 704, 704, 2752, 2752, 10624, 10624, 11968, 11968, 10560, 10560, 7360, 7360, 8960, 8960, 4800, 4800, 12160, 12160, 3072, 3072, 11200, 11200, 2944, 2944, 6016, 6016, 10240, 10240, 12736, 12736, 1280, 1280, 15168, 15168, 12672, 12672, 7936, 7936, 7104, 7104, 7296, 7296, 4992, 4992, 8320, 8320, 8320, 8320, 13376, 13376, 12992, 12992, 10496, 10496, 6144, 6144, 13248, 13248, 8704, 8704, 5696, 5696, 15360, 15360, 14336, 14336, 8960, 8960, 10176, 10176, 9600, 9600, 3392, 3392, 4928, 4928, 7680, 7680, 3776, 3776, 13824, 13824, 3136, 3136, 3648, 3648, 2752, 2752, 3712, 3712, 7104, 7104, 5056, 5056, 15104, 15104, 7040, 7040, 3008, 3008, 14784, 14784, 16000, 16000, 7168, 7168, 1792, 1792, 4224, 4224, 6656, 6656, 9728, 9728, 4288, 4288, 9856, 9856, 11648, 11648, 3584, 3584, 1920, 1920, 4800, 4800, 5184, 5184, 6912, 6912, 8320, 8320, 1344, 1344, 4288, 4288, 13120, 13120, 448, 448, 15168, 15168, 11904, 11904, 8000, 8000, 9472, 9472, 3840, 3840, 7488, 7488, 7488];
    
    %% 处理波形数据
    fprintf('处理波形数据...\n');
    
    % 获取当前长度和目标长度
    current_length = length(wave);
    target_length = ceil((current_length - 1) / 16384) * 16384;
    
    if current_length < target_length
        padding_length = target_length - current_length;
        fprintf('需要填充的零的数量: %d\n', padding_length);
        
        % 使用零填充
        signal_data = [wave, zeros(1, padding_length)];
    else
        signal_data = wave;
    end
    
    % 绘制波形
    figure;
    plot(signal_data);
    title('解包后的数据');
    grid on;
    
    %% 数据类型映射
    type_map = containers.Map();
    type_map('Int16') = 'int16';
    type_map('Float') = 'single';
    type_map('Double') = 'double';
    type_map('IQ(Int16)') = 'int32';
    type_map('IQ(Float32)') = 'single';
    type_map('IQ(Double)') = 'double';
    
    %% 保存波形文件
    wave_file_path = fullfile(current_dir, 'wave_file');
    if ~exist(wave_file_path, 'dir')
        mkdir(wave_file_path);
    end
    
    file_path = fullfile(wave_file_path, 'wave.bin');
    
    % 保存为二进制文件
    try
        fid = fopen(file_path, 'wb');
        if fid == -1
            error('无法创建波形文件');
        end
        
        % 根据数据类型保存
        data_type = type_map(wave_type);
        if strcmp(data_type, 'int16')
            fwrite(fid, int16(signal_data), 'int16');
        elseif strcmp(data_type, 'single')
            fwrite(fid, single(signal_data), 'single');
        elseif strcmp(data_type, 'double')
            fwrite(fid, double(signal_data), 'double');
        elseif strcmp(data_type, 'int32')
            fwrite(fid, int32(signal_data), 'int32');
        else
            fwrite(fid, signal_data, 'double');
        end
        
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
    module_name = 'S2_M4901';
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
        wave_list = "seg1";
        
        % SCPI类型映射
        type_scpi_map = containers.Map();
        type_scpi_map('Int16') = 'INT16';
        type_scpi_map('Float') = 'FLOAT32';
        type_scpi_map('Double') = 'DOUBLE';
        type_scpi_map('IQ(Int16)') = 'COMPLEX32';
        type_scpi_map('IQ(Float32)') = 'COMPLEX64';
        type_scpi_map('IQ(Double)') = 'COMPLEX128';
        
        UltraBVC.send_command(sprintf(':AWG:WAVList:ADDList %s,%s,%d,%s', module_name, wave_list, length(signal_data), type_scpi_map(wave_type)));
        
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
        NSWave_name = "sequence1";
        
        % 创建wave table
        UltraBVC.send_command(sprintf(':AWG:NSQC:ADD %s', NSWave_name));
        
        % 准备NSWave数据
        nswave_program = ['@nw.kernel' newline ...
                         'def program(wlist: dict[str, np.ndarray]):' newline ...
                         '    seg1: nw.ArbWave = nw.init_arbwave(wlist, ''seg1'')' newline ...
                         '    while True:' newline ...
                         '        nw.play_arb(seg1)' newline ...
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
        fprintf('设备启动成功，开始生成波形\n');
        
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
    fprintf('\n=== AWG从预定义波形数据连续波形控制完成 ===\n');

end
