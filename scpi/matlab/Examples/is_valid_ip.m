function result = is_valid_ip(ip_str)
    % is_valid_ip 验证IP地址格式
    % 功能：检查IP地址字符串是否有效
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