; --- WAX3-Driver-InnoSetup.iss ---

[Setup]
AppId={{33F11492-0B4E-46FF-B2B8-E850F96F930B}
AppName=WAX3-Driver
AppVersion=1.0
;AppVerName=WAX3-Driver 1.5
AppPublisher=Open Movement
CreateAppDir=yes
DefaultDirName={pf}\WAX3-Driver
OutputBaseFilename=setup-wax3-driver
OutputDir=.
Compression=zip
SolidCompression=no
PrivilegesRequired=admin
InfoAfterFile=instructions.txt
ArchitecturesInstallIn64BitMode=x64 ia64
Uninstallable=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[InstallDelete]
; Remove old versions of inf file if present
Type: files; Name: "{win}\inf\mchpcdc.inf"
Type: files; Name: "{win}\inf\mchpcdc.cat"
; Remove old precompiled driver cache file if present
Type: files; Name: "{win}\inf\mchpcdc.pnf"

[Files]
Source: "mchpcdc.inf"; DestDir: "{app}"; Flags: ignoreversion
Source: "mchpcdc.cat"; DestDir: "{app}"; Flags: ignoreversion
Source: "dpinst32.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "dpinst64.exe"; DestDir: "{app}"; Flags: ignoreversion

[Run]
; Filename: "rundll32"; Parameters: "advpack.dll,LaunchINFSectionEx {app}\mchpcdc.inf,,,"; StatusMsg: "Installing driver..."
Filename: "{app}\DPInst32.exe"; Parameters: "/F /SA /SE /SW"; StatusMsg: "Installing driver..."; Flags: waituntilterminated; Check: Not Is64BitInstallMode
Filename: "{app}\DPInst64.exe"; Parameters: "/F /SA /SE /SW"; StatusMsg: "Installing driver..."; Flags: waituntilterminated; Check: Is64BitInstallMode

[UninstallRun]
Filename: "{app}\DPInst32.exe"; Parameters: "/U ""{app}\mchpcdc.inf"" /Q"; StatusMsg: "Uninstalling driver..."; Flags: waituntilterminated; Check: Not Is64BitInstallMode
Filename: "{app}\DPInst64.exe"; Parameters: "/U ""{app}\mchpcdc.inf"" /Q"; StatusMsg: "Uninstalling driver..."; Flags: waituntilterminated; Check: Is64BitInstallMode
