% main_advanced.m
% MATLAB版本的QSYNC设备控制高级主程序
% 包含错误处理、配置验证和更多功能选项

function main_advanced(ipaddress)
    % 高级主函数：QSYNC设备控制
    % 参数：
    %   ipaddress: QSYNC设备IP地址
    
    fprintf('=====================================\n');
    fprintf('      QSYNC控制                      \n');
    fprintf('=====================================\n');
    
    try
        % 设备配置
        config = struct();
        config.UltraBVC_ip = ipaddress;
        config.timeout = 10.0;

        % 执行主程序
        fprintf(' 开始执行QSYNC控制流程...\n');
        run_UltraBVC_control(config);
        
        fprintf('=====================================\n');
        fprintf('      程序执行成功完成！            \n');
        fprintf('=====================================\n');
        
    catch ME
        fprintf('\n 程序执行出错: %s\n', ME.message);
        fprintf('错误文件: %s\n', ME.stack(1).file);
        fprintf('错误位置: %s (第%d行)\n', ME.stack(1).name, ME.stack(1).line);
        fprintf('错误详情:\n');
        for i = 1:length(ME.stack)
            fprintf('  [%d] %s: %d\n', i, ME.stack(i).name, ME.stack(i).line);
        end
        fprintf('=====================================\n');
        rethrow(ME);
    end
end