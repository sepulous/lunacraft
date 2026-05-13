[Setup]
AppName=Lunacraft
AppVersion=1.0.1
DefaultDirName={pf}\Lunacraft
DefaultGroupName=Lunacraft
OutputDir=dist
OutputBaseFilename=lunacraft-installer
Compression=lzma
SolidCompression=yes


[Files]
Source: "install\bin\Lunacraft.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "install\bin\OpenAL32.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "install\bin\libgcc_s_seh-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "install\bin\libstdc++-6.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "install\bin\libwinpthread-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "install\lib\*"; DestDir: "{app}\lib"; Flags: ignoreversion
Source: "install\share\*"; DestDir: "{app}\share"; Flags: recursesubdirs

[Icons]
Name: "{group}\Lunacraft"; Filename: "{app}\bin\Lunacraft.exe"; IconFilename: "{app}\bin\Lunacraft.exe"
Name: "{group}\Uninstall Lunacraft"; Filename: "{uninstallexe}"

[Dirs]
Name: "{app}"; Flags: uninsalwaysuninstall

[Run]
Filename: "{app}\bin\Lunacraft.exe"; Description: "Launch Lunacraft"; Flags: nowait postinstall skipifsilent
