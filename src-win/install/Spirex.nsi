; Spirex Screensaver
; ---------------------
; Copyright (C) 2001, 2002, 2003, 2004 John Horigan
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
; 
; John Horigan can be contacted at john@glyphic.com or at
; John Horigan, 1209 Villa St., Mountain View, CA 94041-1123, USA
;

!define VER_MAJOR 2
!define VER_MINOR 25

SetCompressor lzma

!define PRODUCT "Spirex"
!define VERSION "${VER_MAJOR}.${VER_MINOR}"

!include "MUI.nsh"

;--------------------------------
;Configuration

  !define MUI_ICON "..\\res\\Spirex.ico"
  
  !define MUI_WELCOMEFINISHPAGE_BITMAP "Spirex welcome.bmp"


  !define MUI_WELCOMEPAGE_TITLE "Spirex Screen Saver"
    
  
  !define MUI_FINISHPAGE_NOAUTOCLOSE

ShowInstDetails show

  
;General
  OutFile "Spirex${VER_MAJOR}${VER_MINOR}Install.exe"
  
  ;License page
  LicenseData "Spirex license.txt"

  
  Name "${PRODUCT}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "Spirex license.txt"
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH


  ;Language
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Spirex screen saver (required)" 
SectionIn RO

  ClearErrors
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Spirex" "UninstallString"
  IfErrors NoUninstall
  MessageBox MB_YESNO "Uninstall old version of Spirex? (Recommended)" IDNO NoUninstall
  Exec $0
  MessageBox MB_OK "Click OK when uninstall is done."
  
NoUninstall:
  
  SetOutPath $WINDIR
  SetOverwrite on
  File '..\\..\\Release\\Spirex.scr'
 
SectionEnd


Section -post

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{0A9A8E5B-1D0D-44A0-A163-8C149DCAE3F7}" \
                   "DisplayName" "Spirex screen saver"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{0A9A8E5B-1D0D-44A0-A163-8C149DCAE3F7}" \
                   "UninstallString" "$WINDIR\Spirex.scr /u"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{0A9A8E5B-1D0D-44A0-A163-8C149DCAE3F7}" \
                   "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{0A9A8E5B-1D0D-44A0-A163-8C149DCAE3F7}" \
                   "DisplayIcon" "$WINDIR\Spirex.scr,1"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{0A9A8E5B-1D0D-44A0-A163-8C149DCAE3F7}" \
                   "URLInfoAbout" "http://www.ozonehouse.com/Spirex/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{0A9A8E5B-1D0D-44A0-A163-8C149DCAE3F7}" \
                   "URLUpdateInfo" "http://www.ozonehouse.com/Spirex/download.html"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{0A9A8E5B-1D0D-44A0-A163-8C149DCAE3F7}" \
                   "Publisher" "Ozonesoft"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{0A9A8E5B-1D0D-44A0-A163-8C149DCAE3F7}" \
                   "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{0A9A8E5B-1D0D-44A0-A163-8C149DCAE3F7}" \
                   "NoRepair" 1

SectionEnd

Function .onInstSuccess
  ExecShell install '$WINDIR\Spirex.scr'
FunctionEnd




;eof
