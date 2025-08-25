function run_UltraBVC_control(config)
    % UltraBVC设备控制主流程
    % 参数：
    %   config: 配置结构体，包含设备IP、系统参数、触发参数等

    fprintf('===============================\n\n');
    fprintf('设备IP: %s\n',      config.UltraBVC_ip);
    fprintf('超时时间: %.1f秒\n', config.timeout);

    UltraBVC = [];  % 初始化变量
    try
        % 步骤1: 创建UltraBVC实例
        fprintf(' 创建UltraBVC实例...\n');
        UltraBVC = TCP_UltraBVC(config.UltraBVC_ip, config.timeout);
        fprintf(' UltraBVC实例创建成功\n\n');
        
        % 步骤2: 测试设备连接
        fprintf(' 发送SCPI...\n');
        result = UltraBVC.send_command(':AWG:TRIGger:SOURce S2_M4901,PXISTARTrig');
        result = UltraBVC.send_command(':AWG:TRIGger:SOURce? S2_M4901,');
        fprintf('result: %s\n', result);

        UltraBVC.close();
        fprintf('✓ 资源清理完成\n\n');
    
    catch ME
        fprintf('\n 控制流程执行失败: %s\n', ME.message);
        % 尝试清理资源
        if ~isempty(UltraBVC)
            try
                % fprintf('尝试清理UltraBVC资源...\n');
                UltraBVC.set('ResetTrig');  % 确保触发停止
                UltraBVC.close();
                % fprintf('✓ 资源清理完成\n');
            catch cleanup_error
                fprintf(' 资源清理失败: %s\n', cleanup_error.message);
            end
        end
        rethrow(ME);
    end
end