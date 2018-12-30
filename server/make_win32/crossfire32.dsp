# Microsoft Developer Studio Project File - Name="crossfire32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=crossfire32 - Win32 FullDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "crossfire32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "crossfire32.mak" CFG="crossfire32 - Win32 FullDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "crossfire32 - Win32 FullDebug" (based on "Win32 (x86) Console Application")
!MESSAGE "crossfire32 - Win32 ReleaseQuit" (based on "Win32 (x86) Console Application")
!MESSAGE "crossfire32 - Win32 ReleaseLog" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "crossfire32___Win32_FullDebug"
# PROP BASE Intermediate_Dir "crossfire32___Win32_FullDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "FullDebug"
# PROP Intermediate_Dir "FullDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "DEBUG" /D "TIME_ARCH_LOAD" /D "CS_LOGSTATS" /D "ESRV_DEBUG" /FD /GZ /I./include /I./random_maps /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\random_maps" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "DEBUG" /D "ESRV_DEBUG" /FR /FD /P /GZ /I./include /I./random_maps /c
# ADD BASE RSC /l 0x407 /i "../include" /d "_DEBUG"
# ADD RSC /l 0x407 /i "../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib pthreadVC2.lib libcurl.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"d:\python21\libs"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy FullDebug\crossfire32.exe ..\crossfire32.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "crossfire32___Win32_ReleaseQuit"
# PROP BASE Intermediate_Dir "crossfire32___Win32_ReleaseQuit"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseQuit"
# PROP Intermediate_Dir "ReleaseQuit"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Gi /GX /O2 /I ".\include" /I ".\random_maps" /I ".\\" /I ".\plugin\include" /I "..\include" /I "..\random_maps" /I "..\\" /I "..\plugin\include" /I "d:\Python21\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MT /W3 /Gi /GX /O2 /I ".\include" /I ".\random_maps" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib /nologo /subsystem:console /machine:I386 /libpath:"d:\python21\libs"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib /nologo /subsystem:console /machine:I386 /libpath:"d:\python21\libs"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ReleaseQuit\crossfire32.exe ..\crossfire32.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "crossfire32___Win32_ReleaseLog"
# PROP BASE Intermediate_Dir "crossfire32___Win32_ReleaseLog"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseLog"
# PROP Intermediate_Dir "ReleaseLog"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Gi /GX /O2 /Ob2 /I "..\include" /I "..\random_maps" /I "..\\" /I "..\plugin\include" /I "d:\Python21\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "DEBUG" /YX"preheader.pch" /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O2 /Ob2 /I "..\include" /I "..\random_maps" /D "_CONSOLE" /D "DEBUG" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG_MOVEATTACK" /FR /YX"preheader.pch" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib /nologo /subsystem:console /pdb:none /machine:I386 /libpath:"d:\python21\libs"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib pthreadVC2.lib libcurl.lib /nologo /subsystem:console /pdb:none /machine:I386 /libpath:"d:\python21\libs"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ReleaseLog\crossfire32.exe ..\crossfire32.exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "crossfire32 - Win32 FullDebug"
# Name "crossfire32 - Win32 ReleaseQuit"
# Name "crossfire32 - Win32 ReleaseLog"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "socket"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\socket\image.c
# End Source File
# Begin Source File

SOURCE=..\socket\info.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\socket"
# PROP Intermediate_Dir "ReleaseQuit\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\socket\init.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\socket"
# PROP Intermediate_Dir "ReleaseQuit\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\socket\item.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\socket"
# PROP Intermediate_Dir "ReleaseQuit\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\socket\loop.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\socket"
# PROP Intermediate_Dir "ReleaseQuit\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\socket\lowlevel.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\socket"
# PROP Intermediate_Dir "ReleaseQuit\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\socket\metaserver.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\socket"
# PROP Intermediate_Dir "ReleaseQuit\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\socket\request.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\socket"
# PROP Intermediate_Dir "ReleaseQuit\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\socket\requestinfo.c
# End Source File
# Begin Source File

SOURCE=..\socket\sounds.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\socket"
# PROP Intermediate_Dir "ReleaseQuit\socket"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# End Group
# Begin Group "server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\server\account.c
# End Source File
# Begin Source File

SOURCE=..\server\account_char.c
# End Source File
# Begin Source File

SOURCE=..\server\alchemy.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\apply.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\attack.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\ban.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\build_map.c
# End Source File
# Begin Source File

SOURCE=..\server\c_chat.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\c_misc.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\c_move.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\c_new.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\c_object.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\c_party.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\c_range.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\c_wiz.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\commands.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\disease.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\gods.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\hiscore.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\init.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\knowledge.c
# End Source File
# Begin Source File

SOURCE=..\server\login.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\main.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\monster.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\move.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\ob_methods.c
# End Source File
# Begin Source File

SOURCE=..\server\ob_types.c
# End Source File
# Begin Source File

SOURCE=..\server\party.c
# End Source File
# Begin Source File

SOURCE=..\server\pets.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\player.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\plugins.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\quest.c
# End Source File
# Begin Source File

SOURCE=..\server\resurrection.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\rune.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\server.c
# End Source File
# Begin Source File

SOURCE=..\server\shop.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\skill_util.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\skills.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\spell_attack.c
# End Source File
# Begin Source File

SOURCE=..\server\spell_effect.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\spell_util.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\swap.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\time.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\timers.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\weather.c
# End Source File
# Begin Source File

SOURCE=..\server\win32.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\server"
# PROP Intermediate_Dir "ReleaseQuit\server"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\server"
# PROP Intermediate_Dir "ReleaseLog\server"
# ADD BASE CPP /YX"crossfire32.pch"
# ADD CPP /YX"crossfire32.pch"

!ENDIF 

# End Source File
# End Group
# Begin Group "random_maps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\random_maps\decor.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\door.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\exit.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\expand2x.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\floor.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\maze_gen.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\monster.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\random_map.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\reader.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"
# ADD CPP /D "YY_NO_UNISTD_H" /D "YY_USE_CONST"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"
# ADD CPP /D "YY_NO_UNISTD_H" /D "YY_USE_CONST"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\rogue_layout.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\room_gen_onion.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\room_gen_spiral.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\snake.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\special.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\square_spiral.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\style.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\treasure.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\wall.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\random_maps"
# PROP Intermediate_Dir "ReleaseQuit\random_maps"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP BASE Intermediate_Dir "Win32_ReleaseNormal\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\anim.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\arch.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\artifact.c
# End Source File
# Begin Source File

SOURCE=..\common\button.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\dialog.c
# End Source File
# Begin Source File

SOURCE=..\common\exp.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\friend.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\glue.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\holy.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\image.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\info.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\init.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\item.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\languages.c
# End Source File
# Begin Source File

SOURCE=..\common\links.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\living.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\loader.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"
# ADD CPP /D "YY_NO_UNISTD_H" /D "YY_USE_CONST" /D "YY_NEVER_INTERACTIVE"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"
# ADD CPP /D "YY_NO_UNISTD_H"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"
# ADD CPP /D "YY_NO_UNISTD_H" /D "YY_USE_CONST" /D "YY_NEVER_INTERACTIVE"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\logger.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\los.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\map.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\ob_methods.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\ob_types.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\object.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\path.c
# End Source File
# Begin Source File

SOURCE=..\common\player.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\porting.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\common\re-cmp.c"

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\readable.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\recipe.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\region.c
# End Source File
# Begin Source File

SOURCE=..\common\shstr.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\stringbuffer.c
# End Source File
# Begin Source File

SOURCE=..\common\time.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\treasure.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\common\utils.c

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

# PROP Intermediate_Dir "FullDebug\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP BASE Intermediate_Dir "Win32_Release\common"
# PROP Intermediate_Dir "ReleaseQuit\common"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\common"

!ENDIF 

# End Source File
# End Group
# Begin Group "types"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\types\altar\altar.c
# End Source File
# Begin Source File

SOURCE=..\types\legacy\apply.c
# End Source File
# Begin Source File

SOURCE=..\types\armour_improver\armour_improver.c
# End Source File
# Begin Source File

SOURCE=..\types\arrow\arrow.c
# End Source File
# Begin Source File

SOURCE=..\types\blindness\blindness.c
# End Source File
# Begin Source File

SOURCE=..\types\book\book.c
# End Source File
# Begin Source File

SOURCE=..\types\button\button.c
# End Source File
# Begin Source File

SOURCE=..\types\cf_handle\cf_handle.c
# End Source File
# Begin Source File

SOURCE=..\types\check_inv\check_inv.c
# End Source File
# Begin Source File

SOURCE=..\types\clock\clock.c
# End Source File
# Begin Source File

SOURCE=..\types\common\common_apply.c
# End Source File
# Begin Source File

SOURCE=..\types\trap\common_trap.c
# End Source File
# Begin Source File

SOURCE=..\types\container\container.c
# End Source File
# Begin Source File

SOURCE=..\types\converter\converter.c
# End Source File
# Begin Source File

SOURCE=..\types\creator\creator.c
# End Source File
# Begin Source File

SOURCE=..\types\deep_swamp\deep_swamp.c
# End Source File
# Begin Source File

SOURCE=..\types\common\describe.c
# End Source File
# Begin Source File

SOURCE=..\types\detector\detector.c
# End Source File
# Begin Source File

SOURCE=..\types\director\director.c
# End Source File
# Begin Source File

SOURCE=..\types\dragon_focus\dragon_focus.c
# End Source File
# Begin Source File

SOURCE=..\types\duplicator\duplicator.c
# End Source File
# Begin Source File

SOURCE=..\types\exit\exit.c
# End Source File
# Begin Source File

SOURCE=..\types\food\food.c
# End Source File
# Begin Source File

SOURCE=..\types\gate\gate.c
# End Source File
# Begin Source File

SOURCE=..\types\hole\hole.c
# End Source File
# Begin Source File

SOURCE=..\types\identify_altar\identify_altar.c
# End Source File
# Begin Source File

SOURCE=..\types\lamp\lamp.c
# End Source File
# Begin Source File

SOURCE=..\types\legacy\legacy_describe.c
# End Source File
# Begin Source File

SOURCE=..\types\lighter\lighter.c
# End Source File
# Begin Source File

SOURCE=..\types\marker\marker.c
# End Source File
# Begin Source File

SOURCE=..\types\mood_floor\mood_floor.c
# End Source File
# Begin Source File

SOURCE=..\types\peacemaker\peacemaker.c
# End Source File
# Begin Source File

SOURCE=..\types\pedestal\pedestal.c
# End Source File
# Begin Source File

SOURCE=..\types\player_changer\player_changer.c
# End Source File
# Begin Source File

SOURCE=..\types\player_mover\player_mover.c
# End Source File
# Begin Source File

SOURCE=..\types\poison\poison.c
# End Source File
# Begin Source File

SOURCE=..\types\poisoning\poisoning.c
# End Source File
# Begin Source File

SOURCE=..\types\potion\potion.c
# End Source File
# Begin Source File

SOURCE=..\types\power_crystal\power_crystal.c
# End Source File
# Begin Source File

SOURCE=..\types\legacy\process.c
# End Source File
# Begin Source File

SOURCE=..\types\common\projectile.c
# End Source File
# Begin Source File

SOURCE=..\types\trap\rune.c
# End Source File
# Begin Source File

SOURCE=..\types\savebed\savebed.c
# End Source File
# Begin Source File

SOURCE=..\types\scroll\scroll.c
# End Source File
# Begin Source File

SOURCE=..\types\shop_inventory\shop_inventory.c
# End Source File
# Begin Source File

SOURCE=..\types\shop_mat\shop_mat.c
# End Source File
# Begin Source File

SOURCE=..\types\sign\sign.c
# End Source File
# Begin Source File

SOURCE=..\types\skillscroll\skillscroll.c
# End Source File
# Begin Source File

SOURCE=..\types\spell_effect\spell_effect.c
# End Source File
# Begin Source File

SOURCE=..\types\spellbook\spellbook.c
# End Source File
# Begin Source File

SOURCE=..\types\spinner\spinner.c
# End Source File
# Begin Source File

SOURCE=..\types\teleporter\teleporter.c
# End Source File
# Begin Source File

SOURCE=..\types\thrown_object\thrown_object.c
# End Source File
# Begin Source File

SOURCE=..\types\transport\transport.c
# End Source File
# Begin Source File

SOURCE=..\types\trap\trap.c
# End Source File
# Begin Source File

SOURCE=..\types\trapdoor\trapdoor.c
# End Source File
# Begin Source File

SOURCE=..\types\treasure\treasure.c
# End Source File
# Begin Source File

SOURCE=..\types\trigger\trigger.c
# End Source File
# Begin Source File

SOURCE=..\types\trigger_altar\trigger_altar.c
# End Source File
# Begin Source File

SOURCE=..\types\trigger_button\trigger_button.c
# End Source File
# Begin Source File

SOURCE=..\types\trigger_pedestal\trigger_pedestal.c
# End Source File
# Begin Source File

SOURCE=..\types\weapon_improver\weapon_improver.c
# End Source File
# End Group
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\arch.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\artifact.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\attack.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\book.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\commands.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\config.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\define.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\expand2x.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\face.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\funcpoint.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\global.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\god.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\includes.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\libproto.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\living.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\loader.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\logger.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\map.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\material.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\maze_gen.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\newclient.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\newserver.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\ob_methods.h
# End Source File
# Begin Source File

SOURCE=..\include\ob_types.h
# End Source File
# Begin Source File

SOURCE=..\include\object.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\path.h
# End Source File
# Begin Source File

SOURCE=..\include\player.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\plugin.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\plugproto.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\quest.h
# End Source File
# Begin Source File

SOURCE=..\include\race.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\random_map.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\include\re-cmp.h"

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\recipe.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\room_gen.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random_maps\rproto.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\shstr.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\skillist.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\skills.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\sockproto.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\sounds.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\spellist.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\spells.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\sproto.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\tod.h
# End Source File
# Begin Source File

SOURCE=..\include\treasure.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\version.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\win32.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\xdir.h

!IF  "$(CFG)" == "crossfire32 - Win32 FullDebug"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseQuit"

# PROP Intermediate_Dir "ReleaseQuit"

!ELSEIF  "$(CFG)" == "crossfire32 - Win32 ReleaseLog"

!ENDIF 

# End Source File
# End Group
# Begin Group "resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\crossfire.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\ChangeLog
# End Source File
# Begin Source File

SOURCE=..\common\loader.l
# End Source File
# Begin Source File

SOURCE=..\random_maps\reader.l
# End Source File
# Begin Source File

SOURCE=..\share\settings
# End Source File
# End Target
# End Project
