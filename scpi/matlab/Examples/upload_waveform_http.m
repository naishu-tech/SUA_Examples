function success = upload_waveform_http(device_ip, module_name, wave_list, file_path, data_length)
    import matlab.net.*
    import matlab.net.http.*
    import matlab.net.http.io.*

    try
        % 构建 SCPI 命令
        scpi_cmd = sprintf(':AWG:WAVLIST:WAVeform:RTTRACE %s,%s,0,%d,', ...
                              module_name, wave_list, data_length);

        device_url = sprintf('http://%s:8000/scpi', device_ip);
        fileProvider = FileProvider(file_path);
        
        try
            % 创建 multipart 表单数据 - 完全模仿 Python requests
            multipartBody = MultipartFormProvider(...
                'scpi', scpi_cmd, ...           % 文本字段
                'file', fileProvider...         % 文件字段
            );
            
            % 创建并发送请求
            request = RequestMessage('POST', [], multipartBody);
            
            % 发送请求
            try
                response = send(request, URI(device_url), HTTPOptions('ConnectTimeout', 30));
                
                if response.StatusCode == 200
                    disp('请求成功！');
                    success = true;
                    if isstruct(response.Body.Data)
                        disp('服务器响应:');
                        disp(response.Body.Data);
                    else
                        fprintf('服务器响应: %s\n', response.Body.Data);
                    end
                    return
                else
                    fprintf('请求失败，状态码: %d\n', response.StatusCode);
                    fprintf('错误信息: %s\n', response.Body.Data);
                    success = false;
                return
                end
                
            catch ME
                fprintf('0 请求异常: %s\n', ME.message);
                success = false;
                return
            end
        catch ME1
            fprintf('1 请求异常: %s\n', ME1.message);
            success = false;
            return
        end
    catch ME2
        fprintf('Wave 完整multipart方法失败: %s\n', ME2.message);
        success = false;
        return
    end
end