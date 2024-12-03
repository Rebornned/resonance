[Setup]
AppName=Ressonance
AppVersion=1.0
DefaultDirName={pf}\Ressonance
DefaultGroupName=Ressonance
OutputDir=output
OutputBaseFilename=Ressonance_setup
Compression=lzma
SolidCompression=yes
SetupIconFile=C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\assets\ui_images\playlists_ico.ico
PrivilegesRequired=admin

[Files]
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\assets\*"; DestDir: "{app}\assets"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\bin\*"; DestDir: "{app}\bin"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\lib\*"; DestDir: "{app}\lib"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\share\*"; DestDir: "{app}\share"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\files\*"; DestDir: "{app}\files"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\bin\main.exe"; DestDir: "{app}\bin"; Flags: ignoreversion

[Icons]
Name: "{group}\Ressonance"; Filename: "{app}\bin\main.exe"; IconFilename: "{app}\assets\ui_images\playlists_ico.ico"
Name: "{commondesktop}\Ressonance"; Filename: "{app}\bin\main.exe"; IconFilename: "{app}\assets\ui_images\playlists_ico.ico"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Run]
Filename: "{app}\assets\fonts\install_font.bat"; Parameters: """{app}\assets\fonts\JotiOne-Regular.ttf"" JotiOne-Regular.ttf"; Flags: runhidden runascurrentuser
Filename: "{app}\bin\main.exe"; Description: "{cm:LaunchProgram}"; Flags: nowait postinstall skipifsilent runhidden runascurrentuser

[UninstallDelete]
Type: filesandordirs; Name: "{app}\bin"
Type: filesandordirs; Name: "{app}\files"
Type: filesandordirs; Name: "{app}" ; 

[UninstallRun]
Filename: "{app}\assets\fonts\uninstall_font.bat"; Parameters: "JotiOne-Regular.ttf"; Flags: runhidden runascurrentuser; RunOnceId: uninstall_font
