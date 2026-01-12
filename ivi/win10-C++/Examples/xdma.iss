; Script for RIGOL XDMA Driver Installation
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!
; This script installs XDMA driver only
; Install RIGOL IVI software separately using ivi.iss
#define MyAppName "RIGOL_XDMA_Driver"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "RIGOL Solution"
#define MyAppURL "https://www.rigol.com/"
#define XDMAInstallPath "C:\Program Files\IVI Foundation\IVI\RIGOL"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
AppId={{A8F3D921-1B45-4C89-9E2A-5F6D8C7B4E12}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={#XDMAInstallPath}
DisableDirPage=yes
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
; Require administrator privileges for driver installation
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog
; Custom uninstall program name
UninstallDisplayName={#MyAppName}
UninstallFilesDir={app}\Uninstall_XDMA
OutputBaseFilename=RIGOL_XDMA_Setup
SolidCompression=yes
WizardStyle=modern
; Install in 64-bit mode on x64 systems
ArchitecturesInstallIn64BitMode=x64
; Show restart reminder after installation
AlwaysShowDirOnReadyPage=no
AlwaysShowGroupOnReadyPage=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
FinishedLabel=Setup has finished installing {#MyAppName} on your computer.%n%nIt is recommended to restart your computer for the driver to take effect.

[Files]
; XDMA driver files for Windows 10 x64
Source: "D:\work\SUA_Examples\ivi\win10-C++\Examples\IVI\win10xdma\*"; DestDir: "{app}\win10xdma"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Uninstall XDMA Driver"; Filename: "{uninstallexe}"
Name: "{group}\Install XDMA Driver Manually"; Filename: "{app}\win10xdma\InstallXDMADriver.bat"; IconFilename: "{sys}\imageres.dll"; IconIndex: 109
Name: "{group}\Reinstall XDMA Driver"; Filename: "{app}\win10xdma\ReinstallXDMADriver.bat"; IconFilename: "{sys}\imageres.dll"; IconIndex: 78

[UninstallDelete]
; Delete dynamically created batch scripts
Type: files; Name: "{app}\win10xdma\InstallXDMADriver.bat"
Type: files; Name: "{app}\win10xdma\ReinstallXDMADriver.bat"
; Delete XDMA driver directory and all contents
Type: filesandordirs; Name: "{app}\win10xdma"
; Delete custom uninstall directory
Type: filesandordirs; Name: "{app}\Uninstall_XDMA"

[Run]
; Provide restart option on the finish page
Filename: "{sys}\shutdown.exe"; Parameters: "/r /t 0"; Flags: postinstall runhidden; Description: "Restart computer now (recommended)"


[Code]
// Helper function: Convert boolean to string
function BoolToStr(Value: Boolean): String;
begin
  if Value then
    Result := 'True'
  else
    Result := 'False';
end;

// Helper function: Check if test signing is already enabled
function IsTestSigningEnabled: Boolean;
var
  ResultCode: Integer;
  TempFile: String;
  FileContent: AnsiString;
begin
  Result := False;
  TempFile := ExpandConstant('{tmp}\bcdedit_output.txt');
  
  // Execute bcdedit and save output to file
  if Exec(ExpandConstant('{sys}\bcdedit.exe'), '/enum {current}', TempFile, SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    if FileExists(TempFile) and LoadStringFromFile(TempFile, FileContent) then
    begin
      // Check if testsigning is set to Yes
      if Pos('testsigning', Lowercase(String(FileContent))) > 0 then
      begin
        if Pos('Yes', String(FileContent)) > 0 then
          Result := True;
      end;
      DeleteFile(TempFile);
    end;
  end;
  
  Log('Test signing currently enabled: ' + BoolToStr(Result));
end;

// Helper function: Check if PCI Serial Port device exists (uninstalled XDMA device)
function CheckPCISerialPortExists: Boolean;
var
  ResultCode: Integer;
  TempFile: String;
  FileContent: AnsiString;
begin
  Result := False;
  TempFile := ExpandConstant('{tmp}\pci_device_check.txt');
  
  Log('Checking for PCI Serial Port device in Device Manager...');
  
  // Use PowerShell to check for PCI Serial Port or unknown devices
  if Exec('powershell.exe',
          '-NoProfile -ExecutionPolicy Bypass -Command "' +
          '$devices = Get-PnpDevice | Where-Object {' +
          '($_.FriendlyName -like ''*PCI Serial Port*'') -or ' +
          '($_.FriendlyName -like ''*Unknown*'' -and $_.Class -eq ''System'') -or ' +
          '($_.Status -eq ''Unknown'')' +
          '}; ' +
          'if ($devices) { ' +
          'Write-Output ''FOUND''; ' +
          '$devices | ForEach-Object { Write-Output \"Device: $($_.FriendlyName) - $($_.InstanceId)\" } ' +
          '} else { Write-Output ''NOTFOUND'' }"',
          TempFile, SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    if FileExists(TempFile) and LoadStringFromFile(TempFile, FileContent) then
    begin
      if Pos('FOUND', String(FileContent)) > 0 then
      begin
        Result := True;
        Log('PCI Serial Port or unknown device found:');
        Log(String(FileContent));
      end
      else
        Log('No PCI Serial Port device found');
      DeleteFile(TempFile);
    end;
  end
  else
    Log('Failed to execute PowerShell device check');
end;

// Create a standalone batch script for XDMA driver installation
procedure CreateXDMAInstallScript(DriverPath: String);
var
  ScriptPath: String;
  ScriptContent: String;
  XDMAInfBatPath: String;
begin
  ScriptPath := ExpandConstant('{app}') + '\win10xdma\InstallXDMADriver.bat';
  XDMAInfBatPath := ExpandConstant('{app}') + '\win10xdma\win10\XDMA.inf';
  
  ScriptContent := '@echo off' + #13#10 +
                  'echo ========================================' + #13#10 +
                  'echo RIGOL XDMA Driver FORCE Installation' + #13#10 +
                  'echo ========================================' + #13#10 +
                  'echo.' + #13#10 +
                  'echo This script will FORCE install XDMA driver.' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Driver file location:' + #13#10 +
                  'echo   ' + XDMAInfBatPath + #13#10 +
                  'echo.' + #13#10 +
                  'echo Prerequisites:' + #13#10 +
                  'echo 1. Test signing mode must be ENABLED' + #13#10 +
                  'echo 2. System must be restarted after enabling test signing' + #13#10 +
                  'echo 3. Hardware should be connected (recommended)' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Press any key to continue or Ctrl+C to cancel...' + #13#10 +
                  'pause >nul' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Step 1: Checking for PCI Serial Port device...' + #13#10 +
                  'powershell -NoProfile -Command "$d = Get-PnpDevice | Where-Object {$_.FriendlyName -like ''*PCI Serial Port*''}; if ($d) { Write-Host ''  [FOUND] PCI Serial Port device detected'' } else { Write-Host ''  [INFO] No PCI Serial Port found (hardware may not be connected)'' }"' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Step 2: Checking if driver file exists...' + #13#10 +
                  'if exist "' + XDMAInfBatPath + '" (' + #13#10 +
                  '    echo   [OK] XDMA.inf found' + #13#10 +
                  ') else (' + #13#10 +
                  '    echo   [ERROR] XDMA.inf NOT found!' + #13#10 +
                  '    echo   Expected at: ' + XDMAInfBatPath + #13#10 +
                  '    pause' + #13#10 +
                  '    exit /b 1' + #13#10 +
                  ')' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Step 3: FORCE installing XDMA driver to driver store...' + #13#10 +
                  'echo   Executing: pnputil /add-driver "' + XDMAInfBatPath + '" /install' + #13#10 +
                  'echo.' + #13#10 +
                  'pnputil /add-driver "' + XDMAInfBatPath + '" /install' + #13#10 +
                  'set INSTALL_CODE=%errorlevel%' + #13#10 +
                  'echo.' + #13#10 +
                  'echo   Result code: %INSTALL_CODE%' + #13#10 +
                  'if %INSTALL_CODE% == 0 (' + #13#10 +
                  '    echo   [SUCCESS] Driver installed successfully' + #13#10 +
                  '    goto :scan' + #13#10 +
                  ')' + #13#10 +
                  'if %INSTALL_CODE% == 259 (' + #13#10 +
                  '    echo   [INFO] Driver already exists in store (OK)' + #13#10 +
                  '    goto :scan' + #13#10 +
                  ')' + #13#10 +
                  'if %INSTALL_CODE% == 3010 (' + #13#10 +
                  '    echo   [SUCCESS] Driver installed, restart required' + #13#10 +
                  '    goto :scan' + #13#10 +
                  ')' + #13#10 +
                  'echo.' + #13#10 +
                  'echo   [ERROR] Driver installation failed!' + #13#10 +
                  'echo   Possible causes:' + #13#10 +
                  'echo     - Test signing mode is not enabled' + #13#10 +
                  'echo     - Secure Boot is enabled' + #13#10 +
                  'echo     - System needs restart after enabling test signing' + #13#10 +
                  'echo     - Access denied' + #13#10 +
                  'echo.' + #13#10 +
                  'echo   To enable test signing:' + #13#10 +
                  'echo     Start Menu ^> RIGOL_IVI ^> Enable Test Signing' + #13#10 +
                  'echo     Then restart computer and run this script again' + #13#10 +
                  'pause' + #13#10 +
                  'exit /b 1' + #13#10 +
                  '' + #13#10 +
                  ':scan' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Step 4: Scanning for hardware changes...' + #13#10 +
                  'pnputil /scan-devices' + #13#10 +
                  'echo.' + #13#10 +
                  'echo ========================================' + #13#10 +
                  'echo XDMA Driver installation completed!' + #13#10 +
                  'echo ========================================' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Verification steps:' + #13#10 +
                  'echo   1. Open Device Manager (devmgmt.msc)' + #13#10 +
                  'echo   2. Look for: Xilinx Drivers ^> Xilinx DMA' + #13#10 +
                  'echo.' + #13#10 +
                  'echo If device is not shown:' + #13#10 +
                  'echo   - Ensure hardware is connected and powered on' + #13#10 +
                  'echo   - Restart computer' + #13#10 +
                  'echo   - Check: Other devices ^> PCI Serial Port' + #13#10 +
                  'echo   - If found: Right-click ^> Update driver ^> Browse' + #13#10 +
                  'echo   - Select: ' + XDMAInfBatPath + #13#10 +
                  'echo.' + #13#10 +
                  'echo Driver location: ' + XDMAInfBatPath + #13#10 +
                  'echo.' + #13#10 +
                  'echo Press any key to exit...' + #13#10 +
                  'pause >nul' + #13#10;
  
  SaveStringToFile(ScriptPath, ScriptContent, False);
  Log('Created XDMA manual installation script at: ' + ScriptPath);
end;

// Create a standalone batch script to reinstall XDMA driver
procedure CreateXDMAReinstallScript(DriverPath: String);
var
  ScriptPath: String;
  ScriptContent: String;
  XDMAInfBatPath: String;
begin
  ScriptPath := ExpandConstant('{app}') + '\win10xdma\ReinstallXDMADriver.bat';
  XDMAInfBatPath := ExpandConstant('{app}') + '\win10xdma\win10\XDMA.inf';
  
  ScriptContent := '@echo off' + #13#10 +
                  'echo ========================================' + #13#10 +
                  'echo RIGOL XDMA Driver FORCE Reinstallation' + #13#10 +
                  'echo ========================================' + #13#10 +
                  'echo.' + #13#10 +
                  'echo This script will:' + #13#10 +
                  'echo 1. FORCE remove all existing XDMA drivers and devices' + #13#10 +
                  'echo 2. FORCE reinstall XDMA driver' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Driver location: ' + XDMAInfBatPath + #13#10 +
                  'echo.' + #13#10 +
                  'echo Press any key to continue or Ctrl+C to cancel...' + #13#10 +
                  'pause >nul' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Step 1: FORCE removing all XDMA devices...' + #13#10 +
                  'powershell -NoProfile -Command "$devices = Get-PnpDevice | Where-Object {$_.FriendlyName -like ''*Xilinx*'' -or $_.FriendlyName -like ''*XDMA*''}; if ($devices) { $devices | ForEach-Object { Write-Host \"  Removing: $($_.FriendlyName)\"; pnputil /remove-device $_.InstanceId /force 2>&1 | Out-Null } } else { Write-Host ''  No devices found'' }"' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Step 2: FORCE removing driver from driver store...' + #13#10 +
                  'pnputil /delete-driver xdma.inf /uninstall /force 2>nul' + #13#10 +
                  'pnputil /delete-driver XDMA.inf /uninstall /force 2>nul' + #13#10 +
                  'powershell -NoProfile -Command "$output = pnputil /enum-drivers; $lines = $output -split ''\r?\n''; $currentOem = ''''; foreach ($line in $lines) { if ($line -match ''Published Name.*:(oem\d+\.inf)'') { $currentOem = $matches[1] }; if ($line -match ''xdma|Xilinx'') { if ($currentOem) { Write-Host \"  Removing: $currentOem\"; pnputil /delete-driver $currentOem /uninstall /force 2>&1 | Out-Null } } }"' + #13#10 +
                  'echo   Driver cleanup completed' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Step 3: FORCE installing XDMA driver...' + #13#10 +
                  'echo   Command: pnputil /add-driver "' + XDMAInfBatPath + '" /install' + #13#10 +
                  'echo.' + #13#10 +
                  'pnputil /add-driver "' + XDMAInfBatPath + '" /install' + #13#10 +
                  'set INSTALL_CODE=%errorlevel%' + #13#10 +
                  'echo.' + #13#10 +
                  'echo   Result code: %INSTALL_CODE%' + #13#10 +
                  'if %INSTALL_CODE% == 0 (' + #13#10 +
                  '    echo   [SUCCESS] Driver installed successfully' + #13#10 +
                  '    goto :scan' + #13#10 +
                  ')' + #13#10 +
                  'if %INSTALL_CODE% == 259 (' + #13#10 +
                  '    echo   [INFO] Driver already exists in store (OK)' + #13#10 +
                  '    goto :scan' + #13#10 +
                  ')' + #13#10 +
                  'if %INSTALL_CODE% == 3010 (' + #13#10 +
                  '    echo   [SUCCESS] Driver installed, restart required' + #13#10 +
                  '    goto :scan' + #13#10 +
                  ')' + #13#10 +
                  'echo.' + #13#10 +
                  'echo   [ERROR] Driver installation failed!' + #13#10 +
                  'echo     - Test signing mode is not enabled' + #13#10 +
                  'echo     - Secure Boot is enabled' + #13#10 +
                  'echo     - System needs restart after enabling test signing' + #13#10 +
                  'echo     - Access denied' + #13#10 +
                  'echo.' + #13#10 +
                  'echo   To enable test signing:' + #13#10 +
                  'echo     Start Menu ^> RIGOL_IVI ^> Enable Test Signing' + #13#10 +
                  'echo     Then restart computer and run this script again' + #13#10 +
                  'pause' + #13#10 +
                  'exit /b 1' + #13#10 +
                  '' + #13#10 +
                  ':scan' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Step 4: Scanning for hardware changes...' + #13#10 +
                  'pnputil /scan-devices' + #13#10 +
                  'echo.' + #13#10 +
                  'echo ========================================' + #13#10 +
                  'echo XDMA Driver installation completed!' + #13#10 +
                  'echo ========================================' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Verification steps:' + #13#10 +
                  'echo   1. Open Device Manager (devmgmt.msc)' + #13#10 +
                  'echo   2. Look for: Xilinx Drivers ^> Xilinx DMA' + #13#10 +
                  'echo.' + #13#10 +
                  'echo If device is not shown:' + #13#10 +
                  'echo   - Ensure hardware is connected and powered on' + #13#10 +
                  'echo   - Restart computer' + #13#10 +
                  'echo   - Check: Other devices ^> PCI Serial Port' + #13#10 +
                  'echo   - If found: Right-click ^> Update driver ^> Browse' + #13#10 +
                  'echo   - Select directory containing XDMA.inf' + #13#10 +
                  'echo.' + #13#10 +
                  'echo Driver file location:' + #13#10 +
                  'echo   ' + XDMAInfBatPath + #13#10 +
                  'echo.' + #13#10 +
                  'echo Press any key to exit...' + #13#10 +
                  'pause >nul' + #13#10;
  
  SaveStringToFile(ScriptPath, ScriptContent, False);
  Log('Created XDMA reinstallation script at: ' + ScriptPath);
end;

// Main installation procedure for XDMA driver
procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
  XDMAInfPath: String;
  PnpUtilPath: String;
  BcdEditPath: String;
  TestSignEnabled: Boolean;
  DriverInstalled: Boolean;
begin
  if CurStep = ssPostInstall then
  begin
    TestSignEnabled := False;
    DriverInstalled := False;
    
    // Get pnputil and bcdedit paths
    if Is64BitInstallMode then
    begin
      PnpUtilPath := ExpandConstant('{sys}\pnputil.exe');
      BcdEditPath := ExpandConstant('{sys}\bcdedit.exe');
    end
    else
    begin
      PnpUtilPath := ExpandConstant('{sysnative}\pnputil.exe');
      BcdEditPath := ExpandConstant('{sysnative}\bcdedit.exe');
      if not FileExists(PnpUtilPath) then
      begin
        PnpUtilPath := ExpandConstant('{sys}\pnputil.exe');
        BcdEditPath := ExpandConstant('{sys}\bcdedit.exe');
      end;
    end;
    
    Log('========================================');
    Log('Starting XDMA driver installation...');
    Log('========================================');
    Log('System Information:');
    Log('  - Is 64-bit Windows: ' + BoolToStr(Is64BitInstallMode));
    Log('  - Install path: ' + ExpandConstant('{app}'));
    Log('  - PnpUtil path: ' + PnpUtilPath);
    Log('  - BcdEdit path: ' + BcdEditPath);
    
    XDMAInfPath := ExpandConstant('{app}') + '\win10xdma\win10\XDMA.inf';
    Log('  - XDMA.inf expected at: ' + XDMAInfPath);
    Log('  - File exists: ' + BoolToStr(FileExists(XDMAInfPath)));
    
    // Always create utility scripts first
    CreateXDMAInstallScript(ExpandConstant('{app}'));
    CreateXDMAReinstallScript(ExpandConstant('{app}'));
    Log('Utility scripts created');
    
    // No test signing check - directly install driver
    Log('========================================');
    Log('Starting automatic driver installation (no checks)');
    Log('========================================');
    
    // Verify XDMA.inf exists
    if not FileExists(XDMAInfPath) then
    begin
      Log('ERROR: XDMA.inf not found at: ' + XDMAInfPath);
      
      MsgBox('Installation Error: Driver files not found!' + #13#10 + #13#10 + 
             'Expected location: ' + XDMAInfPath + #13#10 + #13#10 + 
             'The installation package may be incomplete.' + #13#10 + #13#10 + 
             'However, utility scripts have been created.' + #13#10 + 
             'You can try using "Install XDMA Driver Manually"' + #13#10 + 
             'from the Start Menu if the files are present.', 
             mbError, MB_OK);
      Exit;
    end;
    
    Log('Step 1: XDMA.inf found successfully');
    
    // Check for PCI Serial Port device (informational only)
    Log('Step 2: Checking for PCI Serial Port device...');
    if CheckPCISerialPortExists then
    begin
      Log('✓ PCI Serial Port or unknown device detected in Device Manager');
    end
    else
    begin
      Log('⚠ No PCI Serial Port device found - hardware may not be connected');
      Log('Will proceed with driver installation anyway');
    end;
    
    Log('Step 3: Force installing driver to Windows driver store...');
    Log('Full command: "' + PnpUtilPath + '" /add-driver "' + XDMAInfPath + '" /install');
    Log('Working directory: ' + ExpandConstant('{app}'));
    
    // Force install driver using /install flag (will bind to matching devices)
    // Use SW_SHOW to see the actual output during installation
    if Exec(PnpUtilPath, '/add-driver "' + XDMAInfPath + '" /install', '', SW_SHOW, ewWaitUntilTerminated, ResultCode) then
    begin
      Log('========================================');
      Log('pnputil command executed');
      Log('Exit code: ' + IntToStr(ResultCode));
      
      // Log detailed result interpretation
      if ResultCode = 0 then
        Log('Result: SUCCESS - Driver installed')
      else if ResultCode = 259 then
        Log('Result: WARNING - Driver already exists in store (0x103)')
      else if ResultCode = 2 then
        Log('Result: ERROR - File not found or access denied (0x2)')
      else if ResultCode = 3010 then
        Log('Result: SUCCESS - Restart required (0xBC2)')
      else
        Log('Result: ERROR - Unknown error code');
      
      Log('========================================');
      
      if (ResultCode = 0) or (ResultCode = 259) or (ResultCode = 3010) then
      begin
        // Success cases: 0 = installed, 259 = already exists, 3010 = restart required
        Log('SUCCESS: XDMA driver installed (code: ' + IntToStr(ResultCode) + ')');
        
        // Scan for hardware changes to trigger driver installation
        Log('Step 4: Scanning for hardware changes...');
        if Exec(PnpUtilPath, '/scan-devices', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
          Log('Hardware scan completed with code: ' + IntToStr(ResultCode))
        else
          Log('Hardware scan command failed to execute');
        
        DriverInstalled := True;
        Log('========================================');
        Log('XDMA driver installation COMPLETED SUCCESSFULLY');
        Log('========================================');
        
        MsgBox('XDMA driver installation completed successfully!' + #13#10 + #13#10 + 
               '✓ XDMA driver installed to Windows driver store' + #13#10 + 
               '✓ Driver files copied to:' + #13#10 + 
               '  C:\Program Files\IVI Foundation\IVI\RIGOL\win10xdma\win10' + #13#10 + 
               '✓ Hardware scan performed' + #13#10 + 
               '✓ Driver will auto-apply to matching devices' + #13#10 + #13#10 + 
               'NEXT STEPS:' + #13#10 + 
               '  1. Restart your computer (see restart option on next page)' + #13#10 + 
               '  2. Connect your RIGOL hardware' + #13#10 + 
               '  3. Check Device Manager → Xilinx Drivers → Xilinx DMA' + #13#10 + #13#10 + 
               'If device does not appear after restart:' + #13#10 + 
               '  - Open Device Manager → Other devices' + #13#10 + 
               '  - Find "PCI Serial Port" or unknown device' + #13#10 + 
               '  - Right-click → Update driver → Browse' + #13#10 + 
               '  - Select: C:\Program Files\IVI Foundation\IVI\RIGOL\win10xdma\win10', 
               mbInformation, MB_OK);
      end
      else
      begin
        Log('ERROR: pnputil returned non-zero error code: ' + IntToStr(ResultCode));
        
        // Provide detailed error explanation
        MsgBox('XDMA driver installation failed!' + #13#10 + #13#10 + 
               'Error code: ' + IntToStr(ResultCode) + #13#10 + #13#10 + 
               'Common error codes:' + #13#10 + 
               '  259 (0x103) - Driver is already in the store' + #13#10 + 
               '  2    (0x2)   - File not found or access denied' + #13#10 + 
               '  3010 (0xBC2) - Requires restart' + #13#10 + #13#10 + 
               'TROUBLESHOOTING STEPS:' + #13#10 + 
               '═══════════════════════════════════' + #13#10 + #13#10 + 
               '1. Check test signing status:' + #13#10 + 
               '   → Run: bcdedit /enum {current}' + #13#10 + 
               '   → Look for: testsigning = Yes' + #13#10 + #13#10 + 
               '2. If test signing is off:' + #13#10 + 
               '   → Start Menu → RIGOL_IVI → Enable Test Signing' + #13#10 + 
               '   → Restart computer' + #13#10 + 
               '   → Run this installer again' + #13#10 + #13#10 + 
               '3. Check installation log at: %TEMP%' + #13#10 + #13#10 + 
               '4. Try manual installation:' + #13#10 + 
               '   → Start Menu → Install XDMA Driver Manually' + #13#10 + #13#10 + 
               'Utility scripts have been created for troubleshooting.', 
               mbError, MB_OK);
      end;
    end
    else
    begin
      Log('CRITICAL ERROR: Failed to execute pnputil.exe');
      Log('Command: ' + PnpUtilPath + ' /add-driver "' + XDMAInfPath + '" /install');
      
      MsgBox('Critical Error: Cannot execute driver installation command!' + #13#10 + #13#10 + 
             'Command failed: pnputil.exe' + #13#10 + 
             'Path: ' + PnpUtilPath + #13#10 + #13#10 + 
             'This indicates a serious system issue:' + #13#10 + 
             '  • pnputil.exe is missing or corrupted' + #13#10 + 
             '  • Antivirus is blocking the command' + #13#10 + 
             '  • Insufficient administrator privileges' + #13#10 + #13#10 + 
             'SOLUTIONS:' + #13#10 + 
             '  1. Verify you are running as Administrator' + #13#10 + 
             '  2. Temporarily disable antivirus' + #13#10 + 
             '  3. Check system file integrity: sfc /scannow' + #13#10 + 
             '  4. Try manual installation from Start Menu', 
             mbError, MB_OK);
    end;
    
    Log('Installation procedure completed');
    Log('========================================');
  end;
end;

var
  XDMAUninstallSuccess: Boolean;

// Force uninstall XDMA driver
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ResultCode: Integer;
  PnpUtilPath: String;
  UninstallSuccess: Boolean;
begin
  if CurUninstallStep = usUninstall then
  begin
    Log('========================================');
    Log('Starting XDMA driver uninstallation...');
    Log('Only removing driver packages, NOT devices');
    Log('========================================');
    
    UninstallSuccess := False;
    
    // Get pnputil path
    if Is64BitInstallMode then
      PnpUtilPath := ExpandConstant('{sys}\pnputil.exe')
    else
    begin
      PnpUtilPath := ExpandConstant('{sysnative}\pnputil.exe');
      if not FileExists(PnpUtilPath) then
        PnpUtilPath := ExpandConstant('{sys}\pnputil.exe');
    end;
    
    Log('PnpUtil path: ' + PnpUtilPath);
    Log('NOTE: Device Manager devices will NOT be removed, only driver packages');
    
    // Step 1: Force uninstall driver by INF name
    Log('Step 1: Force uninstalling XDMA driver by INF name...');
    
    Log('Attempting: pnputil /delete-driver xdma.inf /uninstall /force');
    if Exec(PnpUtilPath, '/delete-driver xdma.inf /uninstall /force', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
    begin
      Log('xdma.inf deletion result code: ' + IntToStr(ResultCode));
      if ResultCode = 0 then
        UninstallSuccess := True;
    end;
    
    Log('Attempting: pnputil /delete-driver XDMA.inf /uninstall /force');
    if Exec(PnpUtilPath, '/delete-driver XDMA.inf /uninstall /force', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
    begin
      Log('XDMA.inf deletion result code: ' + IntToStr(ResultCode));
      if ResultCode = 0 then
        UninstallSuccess := True;
    end;
    
    // Step 2: Find and force delete all XDMA driver packages from driver store
    Log('Step 2: Searching and force removing all XDMA packages from driver store...');
    if Exec('powershell.exe',
            '-NoProfile -ExecutionPolicy Bypass -Command "' +
            '$found = $false; ' +
            '$output = pnputil /enum-drivers; ' +
            '$lines = $output -split ''\r?\n''; ' +
            '$currentOem = ''''; ' +
            'foreach ($line in $lines) { ' +
            'if ($line -match ''Published Name.*:(oem\d+\.inf)'') { ' +
            '$currentOem = $matches[1] ' +
            '}; ' +
            'if ($line -match ''xdma|Xilinx'') { ' +
            'if ($currentOem) { ' +
            'Write-Host \"Force removing driver package: $currentOem\"; ' +
            'pnputil /delete-driver $currentOem /uninstall /force 2>&1 | Out-Null; ' +
            '$found = $true; ' +
            '$currentOem = '''' ' +
            '} ' +
            '} ' +
            '}; ' +
            'if ($found) { exit 0 } else { exit 1 }"',
            '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
    begin
      Log('PowerShell driver store cleanup executed');
      if ResultCode = 0 then
      begin
        Log('Successfully removed XDMA packages from driver store');
        UninstallSuccess := True;
      end
      else
        Log('No XDMA packages found in driver store or already removed');
    end;
    
    // Final log
    Log('========================================');
    if UninstallSuccess then
      Log('XDMA driver packages uninstallation COMPLETED SUCCESSFULLY')
    else
      Log('XDMA driver uninstallation completed (driver may not have been installed)');
    Log('Devices in Device Manager were NOT removed');
    Log('========================================');
    
    // Save status for later use in usPostUninstall
    XDMAUninstallSuccess := UninstallSuccess;
  end;
  
  if CurUninstallStep = usPostUninstall then
  begin
    Log('Post-uninstall cleanup...');
    
    // Delete batch scripts
    if DeleteFile(ExpandConstant('{app}\win10xdma\InstallXDMADriver.bat')) then
      Log('Deleted: InstallXDMADriver.bat');
    
    if DeleteFile(ExpandConstant('{app}\win10xdma\ReinstallXDMADriver.bat')) then
      Log('Deleted: ReinstallXDMADriver.bat');
    
    // Try to remove win10xdma directory
    if DelTree(ExpandConstant('{app}\win10xdma'), True, True, True) then
      Log('Deleted win10xdma directory')
    else
      Log('win10xdma directory not found or could not be deleted');
    
    Log('XDMA uninstallation cleanup completed');
    Log('========================================');
    Log('XDMA uninstallation finished');
    Log('Uninstall success: ' + BoolToStr(XDMAUninstallSuccess));
    
    // Show completion message only (no restart prompt)
    if XDMAUninstallSuccess then
    begin
      Log('XDMA driver was uninstalled successfully');
      MsgBox('XDMA driver uninstallation completed successfully!' + #13#10 + #13#10 + 
             '✓ XDMA driver deleted from Windows driver store' + #13#10 + 
             '✓ Driver files deleted' + #13#10 + 
             '✓ All driver packages removed' + #13#10 + #13#10 + 
             'NOTE:' + #13#10 + 
             '  • Devices in Device Manager are NOT removed' + #13#10 + 
             '  • Test signing mode is still ENABLED' + #13#10 + 
             '  • To remove "Test Mode" watermark: Uninstall RIGOL_IVI', 
             mbInformation, MB_OK);
    end
    else
    begin
      Log('XDMA driver may not have been installed');
      MsgBox('XDMA driver uninstallation completed.' + #13#10 + #13#10 + 
             'The driver may not have been installed previously,' + #13#10 + 
             'or was already removed.' + #13#10 + #13#10 + 
             '✓ Cleanup completed', 
             mbInformation, MB_OK);
    end;
  end;
end;
