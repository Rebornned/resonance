; =============================================================================
;  Resonance - installer script (Inno Setup 6)
;
;  How to build the installer:
;    1. In the MSYS2 MINGW64 shell, from the repository root:
;           bash installer/collect-runtime.sh
;    2. Open this file in Inno Setup Compiler and run Build > Compile (Ctrl+F9).
;    The installer is written to installer/output/Resonance_setup.exe
;
;  Installed layout:
;    {app}\bin\      resonance.exe + GTK DLLs (from build\windows\bin)
;    {app}\lib\      gdk-pixbuf image loaders
;    {app}\share\    GLib schemas and icon theme index
;    {app}\assets\   interface (.glade), CSS and images
;    {app}\files\    song database and playlists (created at run time)
;
;  The executable and the DLLs share {app}\bin: GTK finds lib\ and share\
;  through the parent folder of the folder that holds its DLLs.
; =============================================================================

#define AppName      "Resonance"
#define AppVersion   "1.0.0"
#define AppPublisher "Dhemerson Sousa"
#define AppURL       "https://github.com/Rebornned/resonance"
#define AppExeName   "resonance.exe"
#define AppIcon      "playlists_ico.ico"

; Repository root, relative to this script
#define Root ".."
#define Runtime Root + "\build\windows"

[Setup]
; Unique application ID. Never change it: Windows uses it to recognize
; upgrades and uninstalls of previous versions.
AppId={{F092A940-D980-4479-8EC8-818F95454258}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}/issues
AppUpdatesURL={#AppURL}/releases

; Per-user install, no administrator permission needed.
; With PrivilegesRequired=lowest, {autopf} points to %LOCALAPPDATA%\Programs,
; where the program can write its playlists.
PrivilegesRequired=lowest
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes

; 64-bit Windows only (the DLLs come from MSYS2 MINGW64)
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; Appearance
SetupIconFile={#Root}\assets\ui_images\{#AppIcon}
UninstallDisplayIcon={app}\assets\ui_images\{#AppIcon}
UninstallDisplayName={#AppName}
WizardStyle=modern

; Output
OutputDir=output
OutputBaseFilename=Resonance_setup
Compression=lzma2/max
SolidCompression=yes

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Dirs]
; Playlist folder. The program writes its files here at run time.
Name: "{app}\files\playlists"

[Files]
; Executable, DLLs and GTK runtime collected by installer/collect-runtime.sh
Source: "{#Runtime}\bin\*";   DestDir: "{app}\bin";   Flags: ignoreversion
Source: "{#Runtime}\lib\*";   DestDir: "{app}\lib";   Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Runtime}\share\*"; DestDir: "{app}\share"; Flags: ignoreversion recursesubdirs createallsubdirs

; Interface files. The .ttf is left out here because it is installed into
; the system fonts below; its license (OFL.txt) is kept next to the assets.
Source: "{#Root}\assets\*"; DestDir: "{app}\assets"; Excludes: "*.ttf,*~,desktop.ini"; Flags: ignoreversion recursesubdirs createallsubdirs

; Song database. Playlists are not shipped: they belong to each user.
Source: "{#Root}\files\musics_database.bin"; DestDir: "{app}\files"; Flags: ignoreversion

; Licenses and credits
Source: "{#Root}\LICENSE";    DestDir: "{app}"; Flags: ignoreversion
Source: "{#Root}\CREDITS.md"; DestDir: "{app}"; Flags: ignoreversion

; Interface font. In a non-administrator install, {autofonts} points to the
; user's fonts (Windows 10 1803 or later). Without the uninsneveruninstall
; flag, the font is removed when the program is uninstalled.
Source: "{#Root}\assets\fonts\JotiOne-Regular.ttf"; DestDir: "{autofonts}"; FontInstall: "Joti One"; Flags: onlyifdoesntexist

[Icons]
; WorkingDir is required: the program opens its files through paths relative
; to bin\ (..\assets, ..\files), resolved from the working folder.
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\bin\{#AppExeName}"; WorkingDir: "{app}\bin"; IconFilename: "{app}\assets\ui_images\{#AppIcon}"
Name: "{autodesktop}\{#AppName}";  Filename: "{app}\bin\{#AppExeName}"; WorkingDir: "{app}\bin"; IconFilename: "{app}\assets\ui_images\{#AppIcon}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#AppExeName}"; WorkingDir: "{app}\bin"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Playlists created by the user, which the uninstaller does not know about
Type: filesandordirs; Name: "{app}\files\playlists"
