# Microsoft Developer Studio Project File - Name="Spirex" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Spirex - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Spirex.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Spirex.mak" CFG="Spirex - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Spirex - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Spirex - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Spirex - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "src-common" /I "src-win" /D "NDEBUG" /D DEBUG=0 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "VC6" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib GdiPlus.lib comctl32.lib shlwapi.lib opengl32.lib glu32.lib shfolder.lib wininet.lib winmm.lib Iphlpapi.lib /nologo /subsystem:windows /machine:I386 /out:"Release/Spirex.scr"

!ELSEIF  "$(CFG)" == "Spirex - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "src-common" /I "src-win" /D "_DEBUG" /D DEBUG=1 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "VC6" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib GdiPlus.lib comctl32.lib shlwapi.lib opengl32.lib glu32.lib shfolder.lib wininet.lib winmm.lib Iphlpapi.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Spirex - Win32 Release"
# Name "Spirex - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\src-common\Color.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-win\Configure.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-win\FileCheck.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-win\LinkWin.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-common\myGlut_shapes.c"
# End Source File
# Begin Source File

SOURCE=".\src-common\myGlut_teapot.c"
# End Source File
# Begin Source File

SOURCE=".\src-common\SaverSettings.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-win\SaverSettingsWin32.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-win\SaverWin.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-win\SelfDelete.c"
# End Source File
# Begin Source File

SOURCE=".\src-common\Spirex.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-common\SpirexGeom.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-common\SpirexGL.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-win\SpirexScr.cpp"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\SpirexScr.rc"
# End Source File
# Begin Source File

SOURCE=".\src-win\texture.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=".\src-win\CGdiPlusBitmap.h"
# End Source File
# Begin Source File

SOURCE=".\src-common\Color.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\Configure.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\Debug.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\FileCheck.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\LinkWin.h"
# End Source File
# Begin Source File

SOURCE=".\src-common\myGl.h"
# End Source File
# Begin Source File

SOURCE=".\src-common\myGlut.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\myWindows.h"
# End Source File
# Begin Source File

SOURCE=".\src-common\Point3D.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\RESOURCE.H"
# End Source File
# Begin Source File

SOURCE=".\src-common\RingBuffer.h"
# End Source File
# Begin Source File

SOURCE=".\src-common\SaverSettings.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\SaverSettingsWin32.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\SaverWin.h"
# End Source File
# Begin Source File

SOURCE=".\src-common\Spirex.h"
# End Source File
# Begin Source File

SOURCE=".\src-common\SpirexGeom.h"
# End Source File
# Begin Source File

SOURCE=".\src-common\SpirexGL.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\SpirexScr.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\texture.h"
# End Source File
# Begin Source File

SOURCE=".\src-win\VersionInfo.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\src-win\res\BL_no.ico"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\BL_yes.ico"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\blank.cur"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\BR_no.ico"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\BR_yes.ico"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\Spirex.ico"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\TL_no.ico"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\TL_yes.ico"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\TR_no.ico"
# End Source File
# Begin Source File

SOURCE=".\src-win\res\TR_yes.ico"
# End Source File
# End Group
# End Target
# End Project
