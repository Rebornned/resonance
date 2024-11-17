[Setup]
AppName=Playlists
AppVersion=1.0
DefaultDirName={pf}\Playlists
DefaultGroupName=Playlists
OutputDir=output
OutputBaseFilename=Playlists_setup
Compression=lzma
SolidCompression=yes
SetupIconFile=C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\assets\ui_images\playlists.ico
PrivilegesRequired=admin

[Files]
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\assets\*"; DestDir: "{app}\assets"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\bin\*"; DestDir: "{app}\bin"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\lib\*"; DestDir: "{app}\lib"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\share\*"; DestDir: "{app}\share"; Flags: recursesubdirs createallsubdirs
Source: "C:\Users\Amage\Desktop\Programacao\Playlists_Final_Prog2\Final_Project_Prog2_Playlists\bin\main.exe"; DestDir: "{app}\bin"; Flags: ignoreversion

[Icons]
Name: "{group}\Playlists"; Filename: "{app}\bin\main.exe"; IconFilename: "{app}\assets\ui_images\playlists.ico"
Name: "{commondesktop}\Playlists"; Filename: "{app}\bin\main.exe"; IconFilename: "{app}\assets\ui_images\playlists.ico"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Run]
Filename: "{app}\bin\main.exe"; Description: "{cm:LaunchProgram}"; Flags: nowait postinstall skipifsilent runhidden runascurrentuser


