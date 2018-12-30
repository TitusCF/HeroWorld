!include "MUI.nsh"
Name "Crossfire Server trunk-14250 - Bigworld maps"

CRCCheck On

OutFile "crossfire-server-bigworld-maps-trunk-14250.exe"

InstallDir "$PROGRAMFILES\Crossfire Server"

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

DirText "Please select the folder below"

CompletedText "Installation complete"

UninstallText "This will uninstall Crossfire Server Bigworld maps from your system"

SetOverwrite IfNewer

Section "Bigworld maps" maps
  SectionIn RO
  CreateDirectory "$INSTDIR\share"
  SetOutPath "$INSTDIR\share\maps"

  File /r /x .svn /x .git /x unlinked /x test /x python ..\share\maps\*.*

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Crossfire Server Bigworld maps" "DisplayName" "Crossfire Server Bigworld maps (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Crossfire Server Bigworld maps" "UninstallString" "$INSTDIR\UninstMaps.exe"
  WriteUninstaller "UninstMaps.exe"
SectionEnd


Section "un.Bigworld maps"
  RmDir /r "$INSTDIR\share\maps"

  Delete "$INSTDIR\UninstMaps.exe"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Crossfire Server Bigworld maps"
SectionEnd

Section "Unlinked maps" unlinked
  CreateDirectory "$INSTDIR\share\maps\unlinked"
  SetOutPath "$INSTDIR\share\maps"

  File /r /x .svn /x .git ..\share\maps\unlinked

SectionEnd

Section "un.Unlinked maps"
  RmDir /r "$INSTDIR\share\maps\unlinked"
SectionEnd

Section "Test maps" test
  CreateDirectory "$INSTDIR\share\maps\test"
  SetOutPath "$INSTDIR\share\maps"

  File /r /x .svn /x .git ..\share\maps\test
SectionEnd

Section "un.Test maps"
  RmDir /r "$INSTDIR\share\maps\test"

SectionEnd

Section "Python scripts" python
  CreateDirectory "$INSTDIR\share\maps\python"
  SetOutPath "$INSTDIR\share\maps\"

  File /r /x .svn ..\share\maps\python

SectionEnd

Section "un.Python scripts"

  RmDir /r "$INSTDIR\share\maps\python"

SectionEnd


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${maps} "The main game maps. Required."
  !insertmacro MUI_DESCRIPTION_TEXT ${unlinked} "Maps that can't be accessed from the main maps."
  !insertmacro MUI_DESCRIPTION_TEXT ${test} "Game test maps."
  !insertmacro MUI_DESCRIPTION_TEXT ${python} "Python scripts. Require the server to have the Python plugin installed."
!insertmacro MUI_FUNCTION_DESCRIPTION_END
