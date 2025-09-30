function show_wave_FFT()
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