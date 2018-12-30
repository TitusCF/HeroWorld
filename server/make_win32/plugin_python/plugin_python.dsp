# Microsoft Developer Studio Project File - Name="plugin_python" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=plugin_python - Win32 FullDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "plugin_python.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "plugin_python.mak" CFG="plugin_python - Win32 FullDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "plugin_python - Win32 FullDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "plugin_python - Win32 ReleaseLog" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "plugin_python - Win32 FullDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "FullDebug"
# PROP BASE Intermediate_Dir "FullDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "FullDebug"
# PROP Intermediate_Dir "FullDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PLUGIN_PYTHON_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "C:\Program Files\Python 2.4\include" /I "../../plugins/cfpython/include" /I "../../plugins/common/include" /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PYTHON_PLUGIN_EXPORTS" /FR /YX"plugin.pch" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib plugin_common.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"c:\program files\python 2.4\libs" /libpath:"../plugin_common/fulldebug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy FullDebug\plugin_python.dll ..\..\share\plugins\*.*
# End Special Build Tool

!ELSEIF  "$(CFG)" == "plugin_python - Win32 ReleaseLog"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseLog"
# PROP BASE Intermediate_Dir "ReleaseLog"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseLog"
# PROP Intermediate_Dir "ReleaseLog"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PLUGIN_PYTHON_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "C:\Program Files\Python 2.4\include" /I "../../plugins/cfpython/include" /I "../../plugins/common/include" /I "../../include" /D "_WINDOWS" /D "_USRDLL" /D "PYTHON_PLUGIN_EXPORTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX"plugin.pch" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /libpath:"C:\Program Files\Python 2.4\libs"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ReleaseLog\plugin_python.dll ..\..\share\plugins\*.*
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "plugin_python - Win32 FullDebug"
# Name "plugin_python - Win32 ReleaseLog"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\plugins\cfpython\cfpython.c
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\cfpython_archetype.c
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\cfpython_map.c
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\cfpython_object.c
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\cfpython_party.c
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\cfpython_region.c
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\cjson.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_archetype.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_archetype_private.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_map.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_map_private.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_object.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_object_private.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_party.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_party_private.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_proto.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_region.h
# End Source File
# Begin Source File

SOURCE=..\..\plugins\cfpython\include\cfpython_region_private.h
# End Source File
# End Group
# Begin Group "resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\plugin_python.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# End Target
# End Project
