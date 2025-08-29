classdef CommonUtils
    % CommonUtils - 通用工具函数库
    % 功能：提供常用的工具函数，如IP验证、逻辑转换、文件操作等
    % 作者：重构自多个示例文件的公共函数
    % 日期：2024
    
    methods (Static)
        function result = is_valid_ip(ip_str)
            % 验证IP地址格式
            % 参数：
            %   ip_str: IP地址字符串
            % 返回：
            %   result: 布尔值
            
            if isempty(ip_str)
                result = false;
                return;
            end
            
            % 简单的IP格式验证
            parts = strsplit(ip_str, '.');
            if length(parts) ~= 4
                result = false;
                return;
            end
            
            for i = 1:4
                num = str2double(parts{i});
                if isnan(num) || num < 0 || num > 255
                    result = false;
                    return;
                end
            end
            
            result = true;
        end
        
        function str = logical_to_string(logical_val)
            % 将逻辑值转换为字符串
            % 参数：
            %   logical_val: 逻辑值
            % 返回：
            %   str: 字符串 ('True' 或 'False')
            
            if logical_val
                str = 'True';
            else
                str = 'False';
            end
        end
        
        function result = string_to_logical(str_val)
            % 将字符串转换为逻辑值
            % 参数：
            %   str_val: 字符串
            % 返回：
            %   result: 逻辑值
            
            if ischar(str_val) || isstring(str_val)
                str_val = lower(str_val);
                if strcmp(str_val, 'true') || strcmp(str_val, '1') || strcmp(str_val, 'on')
                    result = true;
                elseif strcmp(str_val, 'false') || strcmp(str_val, '0') || strcmp(str_val, 'off')
                    result = false;
                else
                    error('无法识别的逻辑值: %s', str_val);
                end
            else
                result = logical(str_val);
            end
        end
        
        function ensure_directory_exists(dir_path)
            % 确保目录存在，如果不存在则创建
            % 参数：
            %   dir_path: 目录路径
            
            if ~exist(dir_path, 'dir')
                mkdir(dir_path);
                fprintf('创建目录: %s\n', dir_path);
            end
        end
        
        function file_path = get_unique_filename(base_path, extension)
            % 获取唯一的文件名
            % 参数：
            %   base_path: 基础路径
            %   extension: 文件扩展名（可选）
            % 返回：
            %   file_path: 唯一文件路径
            
            if nargin < 2
                extension = '';
            end
            
            if ~isempty(extension) && ~startsWith(extension, '.')
                extension = ['.', extension];
            end
            
            counter = 1;
            file_path = [base_path, extension];
            
            while exist(file_path, 'file')
                file_path = sprintf('%s_%d%s', base_path, counter, extension);
                counter = counter + 1;
            end
        end
        
        function save_binary_data(data, file_path, data_type)
            % 保存二进制数据到文件
            % 参数：
            %   data: 要保存的数据
            %   file_path: 文件路径
            %   data_type: 数据类型（可选，默认为'double'）
            
            if nargin < 3
                data_type = 'double';
            end
            
            % 确保目录存在
            [dir_path, ~, ~] = fileparts(file_path);
            if ~isempty(dir_path)
                CommonUtils.ensure_directory_exists(dir_path);
            end
            
            % 保存数据
            fid = fopen(file_path, 'wb');
            if fid == -1
                error('无法创建文件: %s', file_path);
            end
            
            try
                fwrite(fid, data, data_type);
                fclose(fid);
                fprintf('数据已保存到: %s\n', file_path);
            catch ME
                fclose(fid);
                rethrow(ME);
            end
        end
        
        function data = load_binary_data(file_path, data_type, num_elements)
            % 从文件加载二进制数据
            % 参数：
            %   file_path: 文件路径
            %   data_type: 数据类型（可选，默认为'double'）
            %   num_elements: 元素数量（可选）
            % 返回：
            %   data: 加载的数据
            
            if nargin < 2
                data_type = 'double';
            end
            
            if ~exist(file_path, 'file')
                error('文件不存在: %s', file_path);
            end
            
            fid = fopen(file_path, 'rb');
            if fid == -1
                error('无法打开文件: %s', file_path);
            end
            
            try
                if nargin >= 3
                    data = fread(fid, num_elements, data_type);
                else
                    data = fread(fid, inf, data_type);
                end
                fclose(fid);
            catch ME
                fclose(fid);
                rethrow(ME);
            end
        end
        
        function aligned_samples = align_samples(samples, alignment)
            % 将采样点数对齐到指定值
            % 参数：
            %   samples: 原始采样点数
            %   alignment: 对齐值（如16384）
            % 返回：
            %   aligned_samples: 对齐后的采样点数
            
            aligned_samples = ceil(samples / alignment) * alignment;
        end
        
        function duration = samples_to_duration(samples, sample_rate)
            % 将采样点数转换为持续时间
            % 参数：
            %   samples: 采样点数
            %   sample_rate: 采样率（Hz）
            % 返回：
            %   duration: 持续时间（秒）
            
            duration = samples / sample_rate;
        end
        
        function samples = duration_to_samples(duration, sample_rate)
            % 将持续时间转换为采样点数
            % 参数：
            %   duration: 持续时间（秒）
            %   sample_rate: 采样率（Hz）
            % 返回：
            %   samples: 采样点数
            
            samples = round(duration * sample_rate);
        end
        
        function formatted_str = format_frequency(freq_hz)
            % 格式化频率显示
            % 参数：
            %   freq_hz: 频率（Hz）
            % 返回：
            %   formatted_str: 格式化后的字符串
            
            if freq_hz >= 1e9
                formatted_str = sprintf('%.3f GHz', freq_hz / 1e9);
            elseif freq_hz >= 1e6
                formatted_str = sprintf('%.3f MHz', freq_hz / 1e6);
            elseif freq_hz >= 1e3
                formatted_str = sprintf('%.3f kHz', freq_hz / 1e3);
            else
                formatted_str = sprintf('%.3f Hz', freq_hz);
            end
        end
        
        function formatted_str = format_time(time_sec)
            % 格式化时间显示
            % 参数：
            %   time_sec: 时间（秒）
            % 返回：
            %   formatted_str: 格式化后的字符串
            
            if time_sec >= 1
                formatted_str = sprintf('%.3f s', time_sec);
            elseif time_sec >= 1e-3
                formatted_str = sprintf('%.3f ms', time_sec * 1e3);
            elseif time_sec >= 1e-6
                formatted_str = sprintf('%.3f μs', time_sec * 1e6);
            elseif time_sec >= 1e-9
                formatted_str = sprintf('%.3f ns', time_sec * 1e9);
            else
                formatted_str = sprintf('%.3f ps', time_sec * 1e12);
            end
        end
        
        function formatted_str = format_file_size(bytes)
            % 格式化文件大小显示
            % 参数：
            %   bytes: 字节数
            % 返回：
            %   formatted_str: 格式化后的字符串
            
            if bytes >= 1e9
                formatted_str = sprintf('%.2f GB', bytes / 1e9);
            elseif bytes >= 1e6
                formatted_str = sprintf('%.2f MB', bytes / 1e6);
            elseif bytes >= 1e3
                formatted_str = sprintf('%.2f KB', bytes / 1e3);
            else
                formatted_str = sprintf('%d B', bytes);
            end
        end
        
        function print_separator(title, char)
            % 打印分隔线
            % 参数：
            %   title: 标题（可选）
            %   char: 分隔字符（可选，默认为'='）
            
            if nargin < 2
                char = '=';
            end
            
            if nargin >= 1 && ~isempty(title)
                fprintf('\n%s %s %s\n', char, title, char);
            else
                fprintf('\n%s\n', repmat(char, 1, 50));
            end
        end
        
        function print_progress(current, total, message)
            % 打印进度信息
            % 参数：
            %   current: 当前进度
            %   total: 总进度
            %   message: 消息（可选）
            
            if nargin < 3
                message = '进度';
            end
            
            percentage = round(current / total * 100);
            fprintf('\r%s: %d/%d (%d%%)', message, current, total, percentage);
            
            if current >= total
                fprintf('\n');
            end
        end
        
        function result = wait_for_condition(condition_func, timeout, interval)
            % 等待条件满足
            % 参数：
            %   condition_func: 条件函数句柄
            %   timeout: 超时时间（秒）
            %   interval: 检查间隔（秒，可选，默认为0.1）
            % 返回：
            %   result: 是否满足条件
            
            if nargin < 3
                interval = 0.1;
            end
            
            start_time = tic;
            while toc(start_time) < timeout
                if condition_func()
                    result = true;
                    return;
                end
                pause(interval);
            end
            
            result = false;
        end
        
        function retry_operation(operation_func, max_attempts, delay)
            % 重试操作
            % 参数：
            %   operation_func: 操作函数句柄
            %   max_attempts: 最大尝试次数
            %   delay: 延迟时间（秒，可选，默认为1）
            
            if nargin < 3
                delay = 1;
            end
            
            for attempt = 1:max_attempts
                try
                    operation_func();
                    return;
                catch ME
                    if attempt < max_attempts
                        fprintf('操作失败 (尝试 %d/%d): %s\n', attempt, max_attempts, ME.message);
                        fprintf('等待 %.1f 秒后重试...\n', delay);
                        pause(delay);
                    else
                        fprintf('操作最终失败 (尝试 %d/%d): %s\n', attempt, max_attempts, ME.message);
                        rethrow(ME);
                    end
                end
            end
        end
        
        function result = is_numeric_string(str)
            % 检查字符串是否为数字
            % 参数：
            %   str: 字符串
            % 返回：
            %   result: 布尔值
            
            if ischar(str) || isstring(str)
                num = str2double(str);
                result = ~isnan(num);
            else
                result = false;
            end
        end
        
        function result = parse_numeric_list(str, delimiter)
            % 解析数字列表字符串
            % 参数：
            %   str: 字符串
            %   delimiter: 分隔符（可选，默认为','）
            % 返回：
            %   result: 数字数组
            
            if nargin < 2
                delimiter = ',';
            end
            
            if ischar(str) || isstring(str)
                parts = strsplit(str, delimiter);
                result = zeros(1, length(parts));
                for i = 1:length(parts)
                    result(i) = str2double(strtrim(parts{i}));
                end
            else
                result = [];
            end
        end
        
        function result = safe_str2double(str, default_value)
            % 安全的字符串转数字转换
            % 参数：
            %   str: 字符串
            %   default_value: 默认值（可选）
            % 返回：
            %   result: 数字值
            
            if nargin < 2
                default_value = NaN;
            end
            
            if ischar(str) || isstring(str)
                num = str2double(str);
                if isnan(num)
                    result = default_value;
                else
                    result = num;
                end
            else
                result = str;
            end
        end
    end
end
