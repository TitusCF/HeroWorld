!include "MUI.nsh"

;Title Of Your Application
Name "Crossfire Server trunk-14250"

VIAddVersionKey "ProductName" "Crossfire server installer"
VIAddVersionKey "Comments" "Website: http://crossfire.real-time.com"
VIAddVersionKey "FileDescription" "Crossfire server installer"
VIAddVersionKey "FileVersion" "trunk-14250"
VIAddVersionKey "LegalCopyright" "Crossfire is released under the GPL."
VIProductVersion "2.0.0.14250"

;Do A CRC Check
CRCCheck On
SetCompressor /SOLID lzma

;Output File Name
OutFile "CrossfireServer.exe"

;License Page Introduction
LicenseText "You must agree to this license before installing."

;The Default Installation Directory
InstallDir "$PROGRAMFILES\Crossfire Server"
InstallDirRegKey HKCU "Software\Crossfire Server" ""

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\Release_notes.txt"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show release notes"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH


!insertmacro MUI_LANGUAGE "English"

Section "Crossfire Server (required)" cf
  SectionIn RO
  ;Install Files
  SetOutPath $INSTDIR
  SetCompress Auto
  SetOverwrite IfNewer
  File "ReleaseLog\crossfire32.exe"
  File "..\pthreadVC2.dll"
  File "..\zlib1.dll"
  File "..\libcurl.dll"
  File "Release_notes.txt"
  File /oname=Changelog.rtf "..\changelog"
  SetOutPath $INSTDIR\share
  File "..\lib\archetypes"
  File "..\lib\artifacts"
  File "..\lib\attackmess"
  File "..\lib\ban_file"
  File "..\lib\bmaps.paths"
  File "..\lib\crossfire.0"
  File "..\lib\crossfire.1"
  File "..\lib\def_help"
  File "..\lib\dm_file"
  File "..\lib\exp_table"
  File "..\lib\faces"
  File "..\lib\forbid"
  File "..\lib\formulae"
  File "..\lib\image_info"
  File "..\lib\materials"
  File "..\lib\messages"
  File "..\lib\motd"
  File "..\lib\news"
  File "..\lib\races"
  File "..\lib\rules"
  File "..\lib\settings"
  File "..\lib\smooth"
  File "..\lib\animations"
  File /oname=treasures "..\lib\treasures.bld"
  File "..\lib\stat_bonus"
  SetOutPath $INSTDIR\share\help
  File "..\lib\help\*.*"
  SetOutPath $INSTDIR\share\wizhelp
  File "..\lib\wizhelp\*.*"
  SetOutPath $INSTDIR\share\i18n
  File "..\lib\i18n\*.*"

  ; Additional directories
  CreateDirectory $INSTDIR\tmp
  CreateDirectory $INSTDIR\var
  CreateDirectory $INSTDIR\var\players
  CreateDirectory $INSTDIR\var\template-maps
  CreateDirectory $INSTDIR\var\unique-items
  CreateDirectory $INSTDIR\var\datafiles
  CreateDirectory $INSTDIR\var\account

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Crossfire Server" "DisplayName" "Crossfire Server (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Crossfire Server" "UninstallString" "$INSTDIR\Uninst.exe"
  WriteUninstaller "Uninst.exe"

  ;Ask about Windows service
  MessageBox MB_YESNO|MB_ICONQUESTION "Register Crossfire server as a Windows service?" /SD IDYES IDNO dont_install

        ;Install service
        DetailPrint "Registering service..."
        ExecWait '"$INSTDIR\Crossfire32.exe" -regsrv'
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Crossfire Server" "ServiceInstalled" "1"
        
  dont_install:
SectionEnd

Section "Python plugin" py
  DetailPrint "Checking for Python26.dll..."
  GetDllVersion "Python26.dll" $R0 $R1
  IntOp $R2 $R0 / 0x00010000
  IntOp $R3 $R0 & 0x0000FFFF
  IntCmp $R2 2 0 wrong
  IntCmp $R3 4 0 wrong
  DetailPrint "   found"
  Goto ok
wrong:
  MessageBox MB_YESNO|MB_ICONQUESTION "Couldn't find Python26.dll. Make sure Python is installed, and that Python26.dll is in your PATH.$\rServer may fail to start if this DLL is not found.$\rInstall plugin anyway?" /SD IDNO IDNO end
  DetailPrint "  install anyway."
ok:
  SetOutPath $INSTDIR\share\plugins
  File "plugin_python\ReleaseLog\plugin_python.dll"
end:
SectionEnd

Section /o "Animator plugin" anim
  SetOutPath $INSTDIR\share\plugins
  File "plugin_anim\ReleaseLog\plugin_animator.dll"
SectionEnd

Section "Menu Shortcuts" menus
  ;Add Shortcuts
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\Crossfire Server"
  CreateShortCut "$SMPROGRAMS\Crossfire Server\Crossfire Server.lnk" "$INSTDIR\crossfire32.exe" "" "$INSTDIR\crossfire32.exe" 0
  CreateShortCut "$SMPROGRAMS\Crossfire Server\Install as Windows service.lnk" "$INSTDIR\crossfire32.exe" "-regsrv" "$INSTDIR\crossfire32.exe" 0
  CreateShortCut "$SMPROGRAMS\Crossfire Server\Uninstall Windows service.lnk" "$INSTDIR\crossfire32.exe" "-unregsrv" "$INSTDIR\crossfire32.exe" 0
  CreateShortCut "$SMPROGRAMS\Crossfire Server\Release notes.lnk" "$INSTDIR\Release_notes.txt"
  CreateShortCut "$SMPROGRAMS\Crossfire Server\Changelog.lnk" "$INSTDIR\Changelog.rtf"
  CreateShortCut "$SMPROGRAMS\Crossfire Server\Uninstall.lnk" "$INSTDIR\uninst.exe" "" "$INSTDIR\uninst.exe" 0
SectionEnd

UninstallText "This will uninstall Crossfire Server from your system"

Section "un.Crossfire Server" un_cf
  SectionIn RO
  ;Unregister service if it was installed
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Crossfire Server" "ServiceInstalled"
  StrCmp $0 "1" 0 +2
        ExecWait '"$INSTDIR\Crossfire32.exe" -unregsrv'

  ;Delete Files
  Delete "$INSTDIR\crossfire32.exe"
  Delete "$INSTDIR\pthreadvc2.dll"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\libcurl.dll"
  Delete "$INSTDIR\Changelog.rtf"
  Delete "$INSTDIR\Share\plugins\python21.dll"
  Delete "$INSTDIR\Release_notes.txt"
  Delete "$INSTDIR\Share\treasures"
  Delete "$INSTDIR\Share\archetypes"
  Delete "$INSTDIR\Share\artifacts"
  Delete "$INSTDIR\Share\attackmess"
  Delete "$INSTDIR\Share\ban_file"
  Delete "$INSTDIR\Share\bmaps.paths"
  Delete "$INSTDIR\Share\crossfire.0"
  Delete "$INSTDIR\Share\crossfire.1"
  Delete "$INSTDIR\Share\def_help"
  Delete "$INSTDIR\Share\dm_file"
  Delete "$INSTDIR\Share\exp_table"
  Delete "$INSTDIR\Share\faces"
  Delete "$INSTDIR\Share\forbid"
  Delete "$INSTDIR\Share\formulae"
  Delete "$INSTDIR\Share\image_info"
  Delete "$INSTDIR\Share\materials"
  Delete "$INSTDIR\Share\messages"
  Delete "$INSTDIR\Share\motd"
  Delete "$INSTDIR\Share\news"
  Delete "$INSTDIR\Share\races"
  Delete "$INSTDIR\Share\rules"
  Delete "$INSTDIR\Share\settings"
  Delete "$INSTDIR\Share\smooth"
  Delete "$INSTDIR\Share\animations"
  
  ;Delete help files
  RmDir /r "$INSTDIR\Share\Help"
  RmDir /r "$INSTDIR\Share\WizHelp"
  RmDir /r "$INSTDIR\Share\i18n"
  
  ;Delete plugins
  RmDir /r "$INSTDIR\Share\Plugins"
  
  ;Remove 'temp' directory
  rmdir /r "$INSTDIR\tmp"
  
  ;Remove some data files
  Delete "$INSTDIR\Var\bookarch"
  Delete "$INSTDIR\Var\clockdata"
  Delete "$INSTDIR\Var\crossfire.log"
  Delete "$INSTDIR\Var\crossfiremail"
  Delete "$INSTDIR\Var\highscore"
  Delete "$INSTDIR\Var\accounts"
  rmdir /r "$INSTDIR\account"

  rmdir $INSTDIR

  ;Delete Start Menu Shortcuts
  RmDir /r "$SMPROGRAMS\Crossfire Server"

  ;Delete Uninstaller And Unistall Registry Entries
  Delete "$INSTDIR\Uninst.exe"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Crossfire Server"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Crossfire Server"
SectionEnd

Section "un.Player files and unique maps data" un_pl
  ;Remove player data section
  MessageBox MB_YESNO|MB_ICONEXCLAMATION "Warning, this will remove all player files, player data, and template maps!$\rAre you sure?" IDNO skip
  RmDir /r "$INSTDIR\var\players"
  RmDir /r "$INSTDIR\var\template-maps"
  RmDir /r "$INSTDIR\var\unique-items"
  RmDir /r "$INSTDIR\var\datafiles"
  RmDir /r "$INSTDIR\var\account"
  skip:
SectionEnd


Section -un.final_clean

  ;Let's check for map uninstaller
  IfFileExists "$INSTDIR\UninstMaps.exe" maps no_maps

maps:
    MessageBox MB_YESNO|MB_ICONQUESTION "Do you want to also remove the maps?" IDNO no_maps
    
    Banner::Show /NOUNLOAD /set 76 "Please wait" "Uninstalling maps..."

    ;Remove maps, let's call the uninstaller in silent mode, and no copying itself somewhere else
    ;(else ExecWait can't wait!)
    ExecWait '"$INSTDIR\UninstMaps.exe" /S _?=$INSTDIR'
	;Remove map directory
	RmDir "$INSTDIR\share\maps"
    ;Need to remove installer, as it couldn't remove itself
    Delete "$INSTDIR\UninstMaps.exe"

    Banner::Destroy

no_maps:

  ;Delete Share directory, if empty
  Rmdir "$INSTDIR\share"

  ;Remove 'var' directory if possible (no force, since user can leave player data)
  rmdir "$INSTDIR\var"

  ;Clean main directory if possible
  RmDir "$INSTDIR"

SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${cf} "Crossfire Server (required)."
  !insertmacro MUI_DESCRIPTION_TEXT ${py} "Python plugin support. Enables post office and a few goodies. Python required."
  !insertmacro MUI_DESCRIPTION_TEXT ${anim} "Animator plugin support. Experimental, use at your own risk!"
  !insertmacro MUI_DESCRIPTION_TEXT ${menus} "Insert icons in Start Menu."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${un_cf} "Remove Crossfire Server."
  !insertmacro MUI_DESCRIPTION_TEXT ${un_pl} "Remove ALL player data, as well as unique maps information."
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END

