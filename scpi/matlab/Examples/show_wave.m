function show_wave()
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