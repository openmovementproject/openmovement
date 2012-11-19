; --- AX3-Driver-InnoSetup.iss ---

[Setup]
AppId={{C12D7D33-3050-44D8-8ADA-8ADCFA9368A5}
AppName=AX3-Driver
AppVersion=1.5
;AppVerName=AX3-Driver 1.5
AppPublisher=Open Movement
CreateAppDir=yes
DefaultDirName={pf}\AX3-Driver
OutputBaseFilename=setup-ax3-driver
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
Type: files; Name: "{win}\inf\MSD_CDC.inf"
Type: files; Name: "{win}\inf\mchp_MSD_CDC.inf"
Type: files; Name: "{win}\inf\mchp_msd_cdc.cat"
; Remove old precompiled driver cache file if present
Type: files; Name: "{win}\inf\MSD_CDC.pnf"
Type: files; Name: "{win}\inf\mchp_MSD_CDC.pnf"

[Files]
Source: "mchp_MSD_CDC.inf"; DestDir: "{app}"; Flags: ignoreversion
Source: "mchp_msd_cdc.cat"; DestDir: "{app}"; Flags: ignoreversion
Source: "dpinst32.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "dpinst64.exe"; DestDir: "{app}"; Flags: ignoreversion

[Run]
; Filename: "rundll32"; Parameters: "advpack.dll,LaunchINFSectionEx {app}\mchp_MSD_CDC.inf,,,"; StatusMsg: "Installing driver..."
Filename: "{app}\DPInst32.exe"; Parameters: "/F /SA /SE /SW"; StatusMsg: "Installing driver..."; Flags: waituntilterminated; Check: Not Is64BitInstallMode
Filename: "{app}\DPInst64.exe"; Parameters: "/F /SA /SE /SW"; StatusMsg: "Installing driver..."; Flags: waituntilterminated; Check: Is64BitInstallMode

[UninstallRun]
Filename: "{app}\DPInst32.exe"; Parameters: "/U ""{app}\mchp_MSD_CDC.inf"" /Q"; StatusMsg: "Uninstalling driver..."; Flags: waituntilterminated; Check: Not Is64BitInstallMode
Filename: "{app}\DPInst64.exe"; Parameters: "/U ""{app}\mchp_MSD_CDC.inf"" /Q"; StatusMsg: "Uninstalling driver..."; Flags: waituntilterminated; Check: Is64BitInstallMode
