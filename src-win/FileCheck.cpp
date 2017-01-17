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

#include "FileCheck.h" 
#include "myWindows.h"
#include <iphlpapi.h>
#include <shlwapi.h>
#include <Wininet.h>
#include <stdio.h>
#include "VersionInfo.h"
#include <vector>

static HWND MsgHwnd = NULL;
static std::vector<char> URL2Check;
static const char* Site2Check;
static const char* File2Check;
static HANDLE CheckThread = NULL;
static HINTERNET hInternetSession = NULL;
static HINTERNET hURL = NULL;
//static HINTERNET hInternetConnect = NULL;

static void GiveUp(DWORD exitCode)
{
    URL2Check.clear();
    CheckThread = NULL;
    if (hURL)
        InternetCloseHandle(hURL);
    //if (hInternetConnect)
        //InternetCloseHandle(hInternetConnect);
    if (hInternetSession)
        InternetCloseHandle(hInternetSession);
    ExitThread(exitCode);
}

static DWORD WINAPI CheckThreadEntry(LPVOID)
{
    MIB_IPFORWARDTABLE* pft;
    DWORD               dwTableSize = 0;
    BOOL                bHasDefaultRoute = FALSE;

    GetIpForwardTable(NULL, &dwTableSize, FALSE);

    std::vector<BYTE> pftbuf(dwTableSize, '\0');
    pft = reinterpret_cast<MIB_IPFORWARDTABLE*>(pftbuf.data());

    if (GetIpForwardTable(pft, &dwTableSize, TRUE) == NO_ERROR) {
        for (unsigned int nIndex = 0; nIndex < pft->dwNumEntries; nIndex++) {
            if (pft->table[nIndex].dwForwardDest == 0) {
                // Default route to gateway
                bHasDefaultRoute = TRUE;
                break;
            }
        }
    }
    pftbuf.clear();

    if (!bHasDefaultRoute)
        GiveUp(FileCheck_No_Internet);

    if (!MsgHwnd)
        GiveUp(FileCheck_Abandoned);

    hInternetSession = InternetOpen(
        "Spirex Version Checker",       // agent
        INTERNET_OPEN_TYPE_PRECONFIG,   // access
        NULL, NULL, 0);                 // defaults
    if (hInternetSession == NULL)
        GiveUp(FileCheck_No_Internet);

    if (!MsgHwnd)
        GiveUp(FileCheck_Abandoned);

#if 0
    hInternetConnect = InternetConnect(hInternetSession, Site2Check,
        INTERNET_DEFAULT_HTTP_PORT, "", "", INTERNET_SERVICE_HTTP, 0, 0);
    if (hInternetConnect == NULL)
        GiveUp(FileCheck_Cannot_Reach_Site);

    if (!MsgHwnd)
        GiveUp(FileCheck_Abandoned);

    hURL = HttpOpenRequest(hInternetConnect, NULL, File2Check, NULL,
        NULL, foo, INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (hURL == NULL)
        GiveUp(FileCheck_Cannot_Request_File);

    if (!MsgHwnd)
        GiveUp(FileCheck_Abandoned);

    if (!HttpSendRequest(hURL, NULL, 0, NULL, 0))
        GiveUp(FileCheck_Cannot_Request_File);

    //Retreive status code
    DWORD statusCode, bufSize = sizeof(DWORD), index;
    if (!HttpQueryInfo(hURL, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
        &statusCode, &bufSize, &index))
        statusCode = 0;

    if (statusCode != 404)
        GiveUp(FileCheck_No_404);
#endif

    DWORD timeout = 15000;  // 15 sec timeout
    InternetSetOption(hInternetSession, INTERNET_OPTION_CONNECT_TIMEOUT,
        (LPVOID)(&timeout), sizeof(DWORD));
    InternetSetOption(hInternetSession, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT,
        (LPVOID)(&timeout), sizeof(DWORD));
    InternetSetOption(hInternetSession, INTERNET_OPTION_CONTROL_SEND_TIMEOUT,
        (LPVOID)(&timeout), sizeof(DWORD));
    InternetSetOption(hInternetSession, INTERNET_OPTION_DATA_SEND_TIMEOUT,
        (LPVOID)(&timeout), sizeof(DWORD));
    InternetSetOption(hInternetSession, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,
        (LPVOID)(&timeout), sizeof(DWORD));

    hURL = InternetOpenUrl(hInternetSession, URL2Check.data(), NULL, 0,
        INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (hURL == NULL)
        GiveUp(FileCheck_Cannot_Reach_Site);

    if (!MsgHwnd)
        GiveUp(FileCheck_Abandoned);

    char buf[256];
    DWORD bytesRead;
    if (!InternetReadFile(hURL, buf, 255, &bytesRead))
        GiveUp(FileCheck_Cannot_Reach_File);

    int v1, v2, v3, v4;
    buf[255] = '\0';
    if (sscanf(buf, "SpirexWindowsVersion=%d.%d.%d.%d", &v1, &v2, &v3, &v4) != 4)
        GiveUp(FileCheck_Cannot_Reach_File);

    if (v1 < VersionNum1)
        GiveUp(FileCheck_No_New_Version);
    if ((v1 == VersionNum1) && (v2 < VersionNum2))
        GiveUp(FileCheck_No_New_Version);
    if ((v1 == VersionNum1) && (v2 == VersionNum2) && (v3 < VersionNum3))
        GiveUp(FileCheck_No_New_Version);
    if ((v1 == VersionNum1) && (v2 == VersionNum2) &&
        (v3 == VersionNum3) && (v4 <= VersionNum4))
        GiveUp(FileCheck_No_New_Version);

    if (!MsgHwnd)
        GiveUp(FileCheck_Abandoned);

    PostMessage(MsgHwnd, WM_APP_NEW_VERSION, 0, 0);
    GiveUp(FileCheck_New_Version);

    return 0;   // never reached
}

void FileCheck(HWND hwnd, const char* site, const char* file2check)
{
    if (MsgHwnd)
        return;

    MsgHwnd = hwnd;
    DWORD URLSize = strlen(site) + strlen(file2check) + 100;
    if (URL2Check.size() <= URLSize)
        URL2Check.resize(URLSize + 1);
    if (UrlCombine(site, file2check, URL2Check.data(), &URLSize, 0) != S_OK) {
        URL2Check.clear();
        MsgHwnd = NULL;
        return;
    }

    Site2Check = site;
    File2Check = file2check;

    CheckThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CheckThreadEntry,
        NULL, 0, NULL);
}

void StopFileCheck()
{
    if (CheckThread)
        MsgHwnd = NULL;     // signal to stop checking
}

