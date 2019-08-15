; Inno Setup Script for OmGui

[Setup]
AppId={{8CDD410D-4556-4A8A-BF86-D67276A10EA5}
AppName=OmGui
AppVersion=1.0.0.43
;AppVerName=OmGui 1.0.0.43
AppPublisher=Open Movement
AppPublisherURL=http://www.openmovement.co.uk
AppSupportURL=http://www.openmovement.co.uk
AppUpdatesURL=http://www.openmovement.co.uk
DefaultDirName={pf}\Open Movement\OM GUI
DefaultGroupName=OmGui
AllowNoIcons=yes
OutputBaseFilename=AX3-GUI-43
SetupIconFile=Application Icon.ico
Compression=zip
SolidCompression=no
OutputDir=.
;UninstallDisplayName={#AppName} {#AppVerStr}
;VersionInfoVersion={#FileVerStr}
;VersionInfoTextVersion={#AppVerStr}
;PrivilegesRequired = admin
UsePreviousAppDir=no
UsePreviousGroup=no


[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "bin\Release\OmGui.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "bin\Release\libomapi.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "bin\Release\OmApiNet.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Plugins for release\*"; DestDir: "{app}"; Excludes: ".svn,Thumbs.db"; Flags: ignoreversion recursesubdirs createallsubdirs
;Source: "utils\cwa-convert.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "utils\setup-ax3-driver.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "firmware\*"; DestDir: "{app}\firmware"; Excludes: ".svn,Thumbs.db"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "Resources\uninstall.ico"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\OmGui"; Filename: "{app}\OmGui.exe"
Name: "{group}\{cm:UninstallProgram,OmGui}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\OmGui"; Filename: "{app}\OmGui.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\OmGui"; Filename: "{app}\OmGui.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\setup-ax3-driver.exe"; WorkingDir: "{app}"; Description: "Install AX3 Driver"; Flags: hidewizard postinstall skipifsilent
Filename: "{app}\OmGui.exe"; Description: "{cm:LaunchProgram,OmGui}"; Flags: nowait postinstall skipifsilent

[CustomMessages]
InstallDotNet=Your system must have the runtime for Microsoft .NET 3.5 -- download now?%n...or manually from: http://tinyurl.com/dotnet35setup%n%nAlternatively, you can enable the feature:%n1. Press Windows+R, enter "appwiz.cpl"%n2. Click 'Turn Windows features on or off'%n3. Select '.NET Framework 3.5 (includes .NET 2.0 and 3.0)', and 'OK'.%n%nIf you get error 0x800f081f:%n1. Press Ctrl+Shift+Esc, Alt+F, N%n2. Type: DISM.EXE /Online /Add-Capability /CapabilityName:NetFx3~~~~%n3. Click 'Create this task with administrative privileges', and 'OK'.%n%nIf you get error 0x800f0906, you can change your system update source:%n1. Press Windows+R, type "gpedit.msc"%n2. Select 'Computer Configuration' / 'Administrative Templates' / 'System' / 'Specify settings for optional component installation and component repair' / 'Enabled' / 'Contact Windows Update directly to download repair content instead of Windows Server Update Services (WSUS)'%n3. Press Ctrl+Shift+Esc, Alt+F, N%n4. Type: gpupdate /force%n5. Click 'Create this task with administrative privileges', and 'OK'.

; DISM.EXE /Online /Enable-Feature /FeatureName:NetFx3 /source:C:\Features /LimitAccess
; DISM.EXE /Online /Add-Package /PackagePath:C:\Features\Microsoft-Windows-NetFx3-OnDemand-Package.cab





[Code]
const
  dotnet35URL = 'http://download.microsoft.com/download/7/0/3/703455ee-a747-4cc8-bd3e-98a615c3aedb/dotNetFx35setup.exe';

function InitializeSetup(): Boolean;
var
  msgRes : integer;
  errCode : integer;

begin
  Result := true;
  // Check for required dotnetfx 3.5 installation
  if (not RegKeyExists(HKLM, 'SOFTWARE\Microsoft\NET Framework Setup\NDP\v3.5')) then begin
    msgRes := MsgBox(CustomMessage('InstallDotNet'), mbError, MB_OKCANCEL);
    if(msgRes = 1) then begin
      ShellExec('Open', dotnet35URL, '', '', SW_SHOW, ewNoWait, errCode);
    end;
    Abort();
  end;
end;
