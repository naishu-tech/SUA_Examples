function success = upload_nswave_http(device_ip, nswave_name, nswave_program)
    import matlab.net.*
    import matlab.net.http.*
    import matlab.net.http.io.*


    % 方法1: 使用 matlab.net.http
    json_data = struct();
    json_data.scpi = sprintf(':AWG:NSQC:UPload %s,', nswave_name);
    json_data.nsqc = char(nswave_program); % 确保是字符数组

    device_url = sprintf('http://%s:8000/scpi', device_ip);

    request = RequestMessage('POST', [], json_data);
        
    try
        response = send(request, URI(device_url), HTTPOptions('ConnectTimeout', 30));
        if response.StatusCode == 200
            disp('✅ matlab.net.http 请求成功');
            disp(response.Body.Data);
            success = true;
            return;
        else
            fprintf('请求失败，状态码: %d\n', response.StatusCode);
            fprintf('错误信息: %s\n', response.Body.Data);
        end
    catch ME
        fprintf('请求异常: %s\n', ME.message);
    end

    success = false;
    return
end