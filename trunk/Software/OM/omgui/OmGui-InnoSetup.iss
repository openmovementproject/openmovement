; --- OmGui-InnoSetup.iss ---

[Setup]
AppId={{35C03686-D04D-4807-A60C-52B03087436C}
AppName=OmGui
AppVersion=1.0.0.6
AppVerName=OmGui Beta 6
DefaultDirName={pf}\OmGui
DefaultGroupName=OmGui
AllowNoIcons=yes
OutputBaseFilename=setup-omgui
SetupIconFile=OmGui.ico
Compression=zip
SolidCompression=yes
OutputDir=.
;UninstallDisplayName={#AppName} {#AppVerStr}
;VersionInfoVersion={#FileVerStr}
;VersionInfoTextVersion={#AppVerStr}
PrivilegesRequired = admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "utils\cwa-convert.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "bin\Release\OmGui.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "bin\Release\OmApiNet.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "bin\Release\libomapi.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "convert\convert.exe"; DestDir: "{app}"; Flags: ignoreversion
;Source: "convert\convert-all.cmd"; DestDir: "{app}"; Flags: ignoreversion
Source: "Resources\uninstall.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "drivers\MSD_CDC.inf"; DestDir: "{win}\inf"; Flags: ignoreversion
;Source: "drivers\mchp_MSD_CDC.inf"; DestDir: "{win}\inf"; Flags: ignoreversion
;Source: "drivers\mchp_msd_cdc.cat"; DestDir: "{win}\inf"; Flags: ignoreversion
;Source: "data\*"; DestDir: "{app}\data"; Excludes: ".svn,Thumbs.db"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\OmGui"; Filename: "{app}\OmGui.exe"; WorkingDir: "{app}"
;Name: "{group}\Convert"; Filename: "{app}\convert-all.cmd"; IconFilename: "{app}\convert.ico"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,OmGui}"; IconFilename: "{app}\uninstall.ico"; Filename: "{uninstallexe}"
Name: "{commondesktop}\OmGui"; Filename: "{app}\OmGui.exe"; WorkingDir: "{app}"; Tasks: desktopicon
;Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\OmGui"; Filename: "{app}\OmGui.exe"; WorkingDir: "{app}"; Tasks: quicklaunchicon

[Run]
;Filename: "rundll32"; Parameters: "advpack.dll,LaunchINFSectionEx {win}\inf\MSD_CDC.inf,,,"; StatusMsg: "Installing driver..."
Filename: "{app}\OmGui.exe"; WorkingDir: "{app}"; Description: "{cm:LaunchProgram,OmGui}"; Flags: nowait postinstall skipifsilent

[CustomMessages]
InstallDotNet=Download and install the required runtime for Microsoft .NET 3.5?

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

