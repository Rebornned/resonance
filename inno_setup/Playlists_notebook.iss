[Setup]
AppName=Playlists
AppVersion=1.0
DefaultDirName={pf}\Playlists
DefaultGroupName=Playlists
OutputDir=output
OutputBaseFilename=Playlists_setup
Compression=lzma
SolidCompression=yes
SetupIconFile=C:\Users\Amage\Desktop\Prog\Projects\Final_Project_Prog2_Playlists\assets\ui_files\playlists.ico
PrivilegesRequired=admin

[Files]
Source: "C:\Users\Amage\Desktop\Prog\Projects\Final_Project_Prog2_Playlists\assets\*"; DestDir: "{app}\assets"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Prog\Projects\Final_Project_Prog2_Playlists\bin\*"; DestDir: "{app}\bin"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Prog\Projects\Final_Project_Prog2_Playlists\lib\*"; DestDir: "{app}\lib"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Prog\Projects\Final_Project_Prog2_Playlists\share\*"; DestDir: "{app}\share"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Prog\Projects\Final_Project_Prog2_Playlists\bin\main.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\Users\Amage\Desktop\Prog\Projects\Final_Project_Prog2_Playlists\assets\fonts\MedievalSharp-Regular.ttf"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Icons]
Name: "{group}\Playlists"; Filename: "{app}\bin\main.exe"; IconFilename: "{app}\assets\ui_files\playlists.ico"
Name: "{commondesktop}\Playlists"; Filename: "{app}\bin\main.exe"; IconFilename: "{app}\assets\ui_files\playlists.ico"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Run]
Filename: "{app}\assets\fonts\install_font.bat"; Parameters: """{tmp}\MedievalSharp-Regular.ttf"" MedievalSharp-Regular.ttf"; Flags: runhidden runascurrentuser
Filename: "{app}\bin\main.exe"; Description: "{cm:LaunchProgram}"; Flags: nowait postinstall skipifsilent runhidden runascurrentuser

[UninstallRun]
Filename: "{app}\assets\fonts\uninstall_font.bat"; Parameters: "MedievalSharp-Regular.ttf"; Flags: runhidden runascurrentuser; RunOnceId: uninstall_font
