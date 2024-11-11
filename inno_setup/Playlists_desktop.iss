[Setup]
AppName=T_Dragons
AppVersion=1.0
DefaultDirName={pf}\T_Dragons
DefaultGroupName=T_Dragons
OutputDir=output
OutputBaseFilename=T_Dragons_setup
Compression=lzma
SolidCompression=yes
SetupIconFile=C:\Users\Amage\Desktop\Programacao\GKT_C\WorldOfDragons\winDistribution\assets\img_files\T_dragons.ico
PrivilegesRequired=admin

[Files]
Source: "C:\Users\Amage\Desktop\Programacao\GKT_C\WorldOfDragons\winDistribution\assets\*"; DestDir: "{app}\assets"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\GKT_C\WorldOfDragons\winDistribution\bin\*"; DestDir: "{app}\bin"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\GKT_C\WorldOfDragons\winDistribution\lib\*"; DestDir: "{app}\lib"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\GKT_C\WorldOfDragons\winDistribution\share\*"; DestDir: "{app}\share"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\GKT_C\WorldOfDragons\winDistribution\bin\main.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\Users\Amage\Desktop\Programacao\GKT_C\WorldOfDragons\winDistribution\assets\fonts\MedievalSharp-Regular.ttf"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Icons]
Name: "{group}\T_Dragons"; Filename: "{app}\bin\main.exe"; IconFilename: "{app}\assets\img_files\T_dragons.ico"
Name: "{commondesktop}\T_Dragons"; Filename: "{app}\bin\main.exe"; IconFilename: "{app}\assets\img_files\T_dragons.ico"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Run]
Filename: "{app}\assets\fonts\install_font.bat"; Parameters: """{tmp}\MedievalSharp-Regular.ttf"" MedievalSharp-Regular.ttf"; Flags: runhidden runascurrentuser
Filename: "{app}\bin\main.exe"; Description: "{cm:LaunchProgram}"; Flags: nowait postinstall skipifsilent runhidden runascurrentuser

[UninstallRun]
Filename: "{app}\assets\fonts\uninstall_font.bat"; Parameters: "MedievalSharp-Regular.ttf"; Flags: runhidden runascurrentuser; RunOnceId: uninstall_font
