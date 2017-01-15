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

#include "LinkWin.h"
#include "myWindows.h"
#include <stdlib.h>
#include <stdio.h>
#include <shellapi.h>
#include "Debug.h"
#include <vector>

static bool Registered = false;
	

LRESULT CALLBACK LinkWin::LinkWinProc(	HWND hwnd, UINT msg, 
											WPARAM wParam, LPARAM lParam)
{ 
	// If you have a problem that's really not going away, put a debug in here:
	//Debug(hwnd, MessageName(msg));
	// This will make a log of every single message that gets sent to the window.

	switch (msg) { 
		case WM_CREATE: {
			HDC hdc = GetDC(hwnd);
			
			// set URL text color to blue w/tranparent bkgrnd
			COLORREF unvisitedcolor = RGB(0, 0, 255); // blue
			SetTextColor(hdc, unvisitedcolor);
			SetBkMode(hdc, TRANSPARENT);
			
			// Get the font of the parent dialog box and create a copy with
			// underline turned on
			HWND MyParent = GetParent(hwnd);
			HFONT ParentFont = (HFONT)SendMessage(MyParent, WM_GETFONT, 0,0);
			SelectObject(hdc, ParentFont);
			
			char typefacename[256];
			GetTextFace(hdc, 256, typefacename);
			TEXTMETRIC typefacemetric;
			GetTextMetrics(hdc, &typefacemetric);
			mLinkFontHandle = CreateFont(typefacemetric.tmHeight, 0, 0, 0, 0, 
				FALSE, TRUE, FALSE, typefacemetric.tmCharSet, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
				typefacename);
			
			SelectObject(hdc, mLinkFontHandle);
			
			ReleaseDC(hwnd, hdc);
	
			return 0;
		}
		case WM_PAINT: {
			// get the name of the control and display it as the URL
			int len = GetWindowTextLength(hwnd);
            std::vector<char> namebuf(len + 1, '\0');
			int r = GetWindowText(hwnd, namebuf.data(), len + 1);
			if (!r) Debug("Couldn't get link window name.");
			
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps); 
			TextOut(hdc, 0,0, namebuf.data(), len);
        	EndPaint(hwnd, &ps);
        	
      return TRUE;
    }
    case WM_MOUSEMOVE: {
      // Once the mouse enters we want to know when it leaves in order to 
      // handle cancelled clicks. 
      if (!mEntered) {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        tme.dwHoverTime = HOVER_DEFAULT;

        mEntered = true;
        if (TrackMouseEvent(&tme) == 0)
          DebugWin();
      }
      return 0;
    }
    case WM_MOUSELEAVE: {
      // Clear any down-click that might have happened. If there was a 
      // down-click it is now cancelled. If the user moves the mouse back
      // into the control window and up-clicks the click is still cancelled.
      mEntered = mButtonDown = false;
      return 0;
    }
    case WM_LBUTTONDOWN: {
      // A down-click happened. The up-click must happen within the 
      // control window (without the mouse ever leaving) for the URL 
      // link to occur. This is standard Window button and link behavior.
      mButtonDown = true;
      return 0;
    }
    case WM_LBUTTONUP: {
      // check if click was cancelled or if the down-click happened
      // outside the control window
      if (!mButtonDown) break;

  			// Click is not cancelled. Launch the browser window
  			char namebuf[200];
  			int r = GetWindowText(hwnd, namebuf, 200);
  			if (!r) {
  				Debug("Couldn't get link window name.");
  			} else {
  				HINSTANCE r = ShellExecute(NULL, "open", namebuf, 
  				NULL, NULL, SW_SHOWNORMAL);
  			}
  			
  			// Change the URL text color to the visited color if we haven't
  			// already done so.
  			if (!mVisited) {
  				mVisited = true;
  				COLORREF visitedcolor = RGB(204, 0, 204); // magenta
  				HDC hdc = GetDC(hwnd);
  				SetTextColor(hdc, visitedcolor);
  				ReleaseDC(hwnd, hdc);
  				InvalidateRect(hwnd, NULL, TRUE); 
  			}
        return 0;
    }
    case WM_DESTROY:
      delete this;
      return 0;  
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK LinkWin::LinkWindowProc(HWND hwnd, UINT msg, 
		WPARAM wParam, LPARAM lParam)
{ 
	// If you have a problem that's really not going away, put a debug in here:
	//Debug(MessageName(msg));
	// This will make a log of every single message that gets sent to the window.
	LinkWin* lw;
	
	if (msg == WM_CREATE) {
		// Create a new LinkWin object from info in the CreateStruct
		RECT rc = {0, 0, ((CREATESTRUCT*)lParam)->cx, ((CREATESTRUCT*)lParam)->cy};
		lw = new LinkWin(hwnd, rc, ((CREATESTRUCT*)lParam)->hInstance);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lw);
	} else {
		// Grab the previously stashed LinkWin pointer from the window user data.
		lw = (LinkWin*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (lw) 
		return lw->LinkWinProc(hwnd, msg, wParam, lParam);
	else 
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

void LinkWin::InitLinkWin(HINSTANCE instHandle) 
{
	WNDCLASS wc;
	
	HCURSOR LinkCursor = LoadCursor(0, IDC_HAND); 
	if (LinkCursor == NULL) {
		DebugWin();
		LinkCursor = LoadCursor(0, IDC_UPARROW);
	}
	if (LinkCursor == NULL) {
		DebugWin();
		LinkCursor = GetCursor(); 
	}
	
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = LinkWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instHandle;
	wc.hIcon = NULL;
	wc.hCursor = LinkCursor;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);	// standard dialog bkgrnd
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "LinkClass";
	RegisterClass(&wc);
}

LinkWin::LinkWin(HWND hparwnd, RECT screenRect, HINSTANCE hInst)
:	mInstHandle(hInst),
	mVisited(false),
	mButtonDown(false),
	mEntered(false),
	mLinkFontHandle(NULL)
{ 
}

LinkWin::~LinkWin()
{
	// In general it is a good idea to 
	if (mLinkFontHandle != NULL)
		DeleteObject(mLinkFontHandle);
}


