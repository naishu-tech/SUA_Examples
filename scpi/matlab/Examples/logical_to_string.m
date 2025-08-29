function str = logical_to_string(logical_val)
    % 将逻辑值转换为字符串
    if logical_val
        str = 'True';
    else
        str = 'False';
    end
end