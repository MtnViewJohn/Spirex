// Spirex Screensaver
// ---------------------
// Copyright (C) 2001, 2002, 2003 John Horigan
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// John Horigan can be contacted at john@glyphic.com or at
// John Horigan, 1209 Villa St., Mountain View, CA 94041-1123, USA
//
//
// portion of this file are
// (c) 1998 Lucian Wischik. You may do whatever you want with this code, without restriction.

#ifndef INCLUDED_FILECHECK
#define INCLUDED_FILECHECK

#include "myWindows.h"

#define WM_APP_NEW_VERSION	(WM_APP + 102)

#define FileCheck_No_Internet (100)
#define FileCheck_Cannot_Reach_Site (101)
#define FileCheck_No_New_Version (102)
#define FileCheck_New_Version (103)
#define FileCheck_Abandoned (104)
#define FileCheck_Cannot_Request_File (105)
#define FileCheck_Cannot_Reach_File (106)

void FileCheck(HWND hwnd, const char* site, const char* file);
void StopFileCheck();

#endif //INCLUDED_FILECHECK