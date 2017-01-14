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

#include "SaverWin.h"
#include "myWindows.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "resource.h"
#include "Debug.h"
#include "Spirex.h"
#include "SpirexGL.h"
#include "SaverSettingsWin32.h"
#include <Mmsystem.h>
#include <Pbt.h>
#include "VersionInfo.h"
#include "FileCheck.h"
#include <shellapi.h>

#define WM_USER_SETTINGS	(WM_USER + 100)		// WPARAM = ptr to new settings
#define WM_USER_START		(WM_USER + 101)
#define WM_USER_STOP		(WM_USER + 102)
#define WM_USER_CLEAR		(WM_USER + 103)
#define WM_USER_TICK		(WM_USER + 104)		// WPARAM = AnimateCount

static void Histogram();
static BOOL CALLBACK  UpdateDialogProc(HWND , UINT , WPARAM , LPARAM );

UINT SaverWin::mMMTimerID = NULL;
SaverWin* SaverWin::SaverWinList = NULL;
bool  SaverWin::ReallyClose = false;
long  SaverWin::SaverCount = 0;
long SaverWin::mIsDialogActive = 0;
ATOM SaverWin::ClassAtom = 0;
bool SaverWin::ThereIsAnUpdate = false;

static bool VerifyPassword(HWND hwnd)
// Under NT, we return TRUE immediately. This lets the saver quit, and the
// system manages passwords. Under '95, we call VerifyScreenSavePwd. This
// checks the appropriate registry key and, if necessary, pops up a verify
// dialog.
{
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof(osv);
	GetVersionEx(&osv);
	if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT) return true;

	HINSTANCE hpwdcpl = ::LoadLibrary("PASSWORD.CPL");
	if (hpwdcpl == NULL) {
		Debug("Unable to load PASSWORD.CPL. Aborting");
		return true;
	}
	typedef BOOL (WINAPI *VERIFYSCREENSAVEPWD)(HWND hwnd);
	VERIFYSCREENSAVEPWD VerifyScreenSavePwd;
	VerifyScreenSavePwd = (VERIFYSCREENSAVEPWD)GetProcAddress(hpwdcpl,
			"VerifyScreenSavePwd");
	if (VerifyScreenSavePwd == NULL) {
		Debug("Unable to get VerifyPwProc address. Aborting");
		FreeLibrary(hpwdcpl);
		return true;
	}
	Debug("About to call VerifyPwProc");
	bool bres = (bool)VerifyScreenSavePwd(hwnd);
	FreeLibrary(hpwdcpl);
	return bres;
}


void SaverWin::Animate(int steps)
// Animate must be called an average of every 30msec. If the calls are bursty
// then it will skip rendering.
{
	if (false) {
		Histogram();
	}

	while (steps-- > 0) {
		mAnimator->NextStep();
		mAnimator->NextStep();
	}


	DWORD now = GetTickCount();

	if ((now - mLastCount) >= (DWORD)1000) {
		mFrameRate = 1000.0 * mRenderCount / (now - mLastCount);
		Debug(mHwnd, "Animate step.");
		mRenderCount = 0;
		mLastCount = now;
	}

	if ((now - mLastRender) >= (DWORD)10) {
		mAnimator->Render(mFrameRate);
		mRenderCount++;
		mLastRender = now;
		int delta = GetTickCount() - now;
		if (delta > 60) {
			char buf[100];
			wsprintf(buf, "Rendering took a long time (%d).", delta);
			Debug(mHwnd, buf);
		}
	}
}

void SaverWin::StartAnimation()
{
	if (!mHwnd) {
		Debug(mHwnd, "Received a start before the window was created.");
		return;
	}

	PostMessage(mHwnd, WM_USER_START, 0, 0);
}

void SaverWin::StopAnimation()
{
	if (!mHwnd) {
		Debug(mHwnd, "Received a stop before the window was created.");
		return;
	}

	PostMessage(mHwnd, WM_USER_STOP, 0, 0);
}

void SaverWin::NewSaverSettings(const SaverSettingsWin32& newSettings)
{
	if (!mHwnd) {
		Debug(mHwnd, "Received a settings before the window was created.");
		return;
	}

	SaverSettingsWin32 *nextSettings = new SaverSettingsWin32(newSettings);
	nextSettings->PowerAwareness(mSaverMode == ScreenSaver);
	PostMessage(mHwnd, WM_USER_SETTINGS, (WPARAM)nextSettings, 0);
}

void SaverWin::Clear()
{
	if (!mHwnd) {
		Debug(mHwnd, "Received a clear before the window was created.");
		return;
	}

	PostMessage(mHwnd, WM_USER_CLEAR, 0, 0);
}

void SaverWin::PrepareToExit(HWND hwnd)
{
	if (mAnimator && mAnimate)
		mAnimator->DestroyGfx();
	mAnimate = false;
	KillTimer(hwnd, IDTM_RANDOM_PRESET);
	KillTimer(hwnd, IDTM_ANIMATE);
}


LRESULT CALLBACK SaverWin::SaverWindowProc(	HWND hwnd, UINT msg,
											WPARAM wParam, LPARAM lParam)
{
	// If you have a problem that's really not going away, put a debug in here:
	//if ( /*msg != WM_TIMER && */ msg < WM_USER_TICK)
		//Debug(hwnd, MessageName(msg, wParam, lParam));
	// This will make a log of every single message that gets sent to the window.

	switch (msg) {
		case WM_CREATE: {
			height = ((LPCREATESTRUCT)lParam)->cy;
			width = ((LPCREATESTRUCT)lParam)->cx;
			mHwnd = hwnd;
			
			mAnimator = new Spirex(mSettings.mSettings, width, height, hwnd);
			GetCursorPos(&mInitCursorPos);
			mLastRender = mInitTime = GetTickCount();

			if (!mMMTimerID) {
				mWTimerID = SetTimer(hwnd, IDTM_ANIMATE, 30, NULL);
				if (!mWTimerID)
					CloseSaverWindow();
			}

			// Setup timer to do random presets change
			int randomPresetPeriod = (mSaverMode == Preview) ?
				20 * 1000 : 5 * 60 * 1000;
			mPresetTimerID = SetTimer(hwnd, IDTM_RANDOM_PRESET, randomPresetPeriod, NULL);
			PostMessage(mHwnd, WM_USER_START, 0, 0);

			if ((mSaverMode == ScreenSaver) && SaverSettingsWin32::Time2CheckUpdate())
				FileCheck(hwnd, URLString, "SpirexWindowsVersion.txt");

			return 0;
		}
		case WM_USER_SETTINGS: {
			Debug(hwnd, "WM_USER_SETTINGS");
			// Copy the new settings and then delete them
			SaverSettingsWin32 *settings = (SaverSettingsWin32 *)wParam;
			if (mOrigScrMode < SaverSettingsWin32::ResolvedMode)
				mScrMode = SaverSettingsWin32::ResolvedMode;
			else 
				mScrMode = mOrigScrMode;
		 	settings->mSettings.m3DRender = mScrMode != SaverSettingsWin32::DisplayNo3D;
#if 0
// Yay! I fixed the "GDI doesn't draw bug" on switching from OpenGL to GDI.
// I can comment out this code.
	   	if (!settings->m3DRender && mSettings.m3DRender && 
    			(mSaverMode != Preview)) {
				Debug("WM_USER_SETTINGS: sending close");
				CloseSaverWindow();			// I hate doing this
				return TRUE;
    	}
#endif // 0
			mSettings = *settings;
			delete settings;
    	SpirexGL::LevelOfDetail = SaverSettingsWin32::LevelOfDetail;
			mAnimate = false;
			mAnimator->NewSaverSettings(mSettings.mSettings);
			mAnimate = true;
			return TRUE;
		}
		case WM_USER_START:
			Debug(hwnd, "WM_USER_START");
			mLastTick = mLastRender = mInitTime = GetTickCount();
			if (mAnimate) {
			  Debug(hwnd, "Extra User start message.");
			  return TRUE;
			}
			mAnimator->SetupGfx();
			mAnimate = true;
			return TRUE;
		case WM_USER_STOP:
			Debug(hwnd, "WM_USER_STOP");
			if (!mAnimate) {
			  Debug(hwnd, "Extra User stop message.");
			  return TRUE;
			}
			mAnimate = false;
			mAnimator->DestroyGfx();
			return TRUE;
		case WM_USER_CLEAR:
			Debug(hwnd, "WM_USER_CLEAR");
  	  mAnimator->Clear();
  	  return TRUE;
		case WM_USER_TICK: {
		  // remove any ticks that have gotten stacked up
    	MSG msg;
			while (PeekMessage(&msg, NULL, WM_USER_TICK, WM_USER_TICK, PM_REMOVE))
			  if (LOWORD(msg.message) == WM_QUIT) {
			    PostQuitMessage(0);   // Put back any WM_QUITs, unlikely (impossible?)
			    break;                //   but just in case...
			  }
			    
			if (mAnimate) {
				Animate(1);
			}
			return TRUE;
		}
		case WM_APP_NEW_VERSION:
			ThereIsAnUpdate = true;
			return TRUE;
		case WM_PAINT: {
			// We don't actually repaint anything, we just erase what's there
			// and let the animator loop continue.
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
		  mAnimator->Clear();
      EndPaint(hwnd, &ps);
		  return 0;
    }
		case WM_ACTIVATEAPP: {
			if (DEBUG || (mSaverMode == Preview) || mIsDialogActive || wParam)
				break;
			for (SaverWin* saver = SaverWinList; saver; saver = saver->mNextSaverWin)
				if (saver->mThreadID == (DWORD)lParam) {
					Debug(hwnd, "Got a WM_ACTIVATEAPP:inactivate from one of my windows!");
					//MessageBeep(MB_ICONEXCLAMATION);
					SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING);
					return 0;
				}
			Debug(hwnd, "WM_ACTIVATEAPP: about to inactive window, so sending close");
			CloseSaverWindow();
			break;
		}
		case WM_ACTIVATE: {
			if ((mSaverMode == Preview) || mIsDialogActive || 
					(LOWORD(wParam) != WA_INACTIVE) || DEBUG) 
				break;
			// It is OK to be inactivated by another Spirex window
			// (multi-monitor support)
			WINDOWINFO wi;
			wi.cbSize = sizeof(WINDOWINFO);
			if (lParam) {
				if (GetWindowInfo((HWND)lParam, &wi) && (wi.atomWindowType == ClassAtom))
					break;
				char buf[200];
				wsprintf(buf, "Window 0x%x with class atom 0x%xinactivated this window (class atom = 0x%x).", 
					lParam, wi.atomWindowType, ClassAtom);
				Debug(hwnd, buf);
			} else {
				DWORD timeDelta = GetTickCount() - mInitTime;
				if (timeDelta < 5000) {
					Debug(hwnd, "Early WM_ACTIVATE:WA_INACTIVE from a null window (ignoring).");
					//MessageBeep(MB_OK);
					SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING);
					break;
				}
			}
			Debug(hwnd, "WM_ACTIVATE: about to inactive window, so sending close");
			CloseSaverWindow();
			return FALSE;
		}
		case WM_NCACTIVATE:
			break;
		case WM_TIMER:
			if (mWTimerID && ((UINT_PTR)wParam == mWTimerID)) {
				if (!mAnimate)
					return 0;

				if (false) {
					Histogram();
				}

				DWORD now = GetTickCount();

				// handle large delays
				if ((now -  mLastTick) > (DWORD)1500)
					mLastTick = now;

				DWORD steps = (now - mLastTick) / (DWORD)30;

				mLastTick += steps * 30;

				Animate(steps);
				return 0;
			} else {
				if ((UINT_PTR)wParam != IDTM_RANDOM_PRESET)
					return 1;	// just in case there are other timers

				if (SaverSettingsWin32::Randomize) {
					SaverSettingsWin32 temp;
					temp.ReadRandomPreset(!mSettings.mSettings.m3DRender);
					NewSaverSettings(temp);
				}
				
				return 0;	// this is our timer but we do nothing
			}	// for mWTimerID fall through to tick handler
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_KEYDOWN:
			if ((mSaverMode != Preview) && !mIsDialogActive) {
				Debug("WM_BUTTONDOWN: sending close");
				CloseSaverWindow();
			}
			return TRUE;
		case WM_MOUSEMOVE:
			if ((mSaverMode != Preview) && !mIsDialogActive && !DEBUG) {
				POINT pt;
				GetCursorPos(&pt);
				int dx = pt.x - mInitCursorPos.x;
				if (dx < 0) dx = -dx;
				int dy = pt.y - mInitCursorPos.y;
				if (dy < 0) dy = -dy;
				if (dx > (int)SaverSettingsWin32::MouseThreshold ||
					dy > (int)SaverSettingsWin32::MouseThreshold) {
					Debug("WM_MOUSEMOVE: gone beyond threshold, sending close");
					CloseSaverWindow();
				}
			}
			return TRUE;
		case (WM_POWERBROADCAST):
			switch (wParam) {
				case (PBT_APMSUSPEND):
					CloseSaverWindow();
					break;
				case (PBT_APMPOWERSTATUSCHANGE):
					SaverSettingsWin32 temp = mSettings;
					NewSaverSettings(temp);			// re-evaluate settings for new power state
					break;
			}
			return TRUE;
		case (WM_SYSCOMMAND):
			if (mSaverMode == ScreenSaver) {
				if ((wParam & 0xFFF0) == SC_SCREENSAVE) {
					Debug("WM_SYSCOMMAND: gobbling up a SC_SCREENSAVE to stop a new saver from running.");
					return FALSE;
				}
				if ((wParam & 0xFFF0) == SC_CLOSE && !DEBUG) {
					Debug("WM_SYSCOMMAND: gobbling up a SC_CLOSE");
					return FALSE;
				}
				if ((wParam & 0xFFF0) == SC_MONITORPOWER && lParam == 2) {
					Debug(hwnd, "Stopping because monitor powered off.");
					CloseSaverWindow();
					break;		// let system blank screen
				}
			}
			break;
		case (WM_CLOSE):
			if (mSaverMode != ScreenSaver) {
				PrepareToExit(hwnd);
				DestroyWindow(hwnd);
				return FALSE;
			}
			if (ReallyClose && !mIsDialogActive) {
				Debug("WM_CLOSE: maybe we need a password");
				bool CanClose = true;
				if (GetTickCount() - mInitTime > 1000 * SaverSettingsWin32::PasswordDelay) {
					InterlockedIncrement(&mIsDialogActive);
					CanClose = VerifyPassword(hwnd);
					InterlockedDecrement(&mIsDialogActive);
					GetCursorPos(&mInitCursorPos);
				}
				if (CanClose) {
					Debug("WM_CLOSE: doing a DestroyWindow");
					PrepareToExit(hwnd);
					DestroyWindow(hwnd);
				} else {
					Debug("WM_CLOSE: but failed password, so doing nothing");
				}
			}
			return FALSE;
			// so that DefWindowProc doesn't get called, because it would just
			// DestroyWindow
			break;
		case (WM_DESTROY):
			// Make sure the resources are freed.
			PrepareToExit(hwnd);
			mAnimate = false;
			mHwnd = NULL;
			OutputDebugString("Destroying window.\n");
			PostQuitMessage(0);
			Debug("POSTQUITMESSAGE from WM_DESTROY!!");
			return FALSE;
		case WM_NCDESTROY:
			delete mAnimator;
			mAnimator = 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void SaverWin::CloseSaverWindow()
{
	ReallyClose = true;
	for (SaverWin* saver = SaverWinList; saver; saver = saver->mNextSaverWin)
		if (saver->mHwnd)
			PostMessage(saver->mHwnd, WM_CLOSE, 0, 0);
}


void CALLBACK SaverWin::MMTimeProc(UINT uID, UINT uMsg, DWORD , DWORD dw1, DWORD dw2)
// Multimedia timer callback. Executes in MM timer thread.
{
	static int Flag = 0;
	if (!Flag) {
		char buf[200];
		wsprintf(buf, "MM timer thread is 0x%X\n", (int)GetCurrentThreadId());
		OutputDebugString(buf);
		Flag = 1;
	}

	if (uID != mMMTimerID) return;	// just in case there are other timers

	if (false) {
		Histogram();
	}

	for (SaverWin* saver = SaverWinList; saver; saver = saver->mNextSaverWin)
		if (saver->mHwnd)
			PostMessage(saver->mHwnd, WM_USER_TICK, 0, 0);
}


static LRESULT CALLBACK SaverWindowProc(HWND hwnd, UINT msg,
		WPARAM wParam, LPARAM lParam)
{
	// If you have a problem that's really not going away, put a debug in here:
	//Debug(MessageName(msg));
	// This will make a log of every single message that gets sent to the window.
	SaverWin* sw;

	if (msg == WM_CREATE) {
		// A pointer to the SaverWin was stashed in the CreateWindow user data.
		// Extract it and stash it in the window user data for subsequent use.
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		sw = (SaverWin*)(cs->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)sw);
	} else {
		// Grab the previously stashed SaverWin pointer from the window user data.
		sw = (SaverWin*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (sw)
		return sw->SaverWindowProc(hwnd, msg, wParam, lParam);
	else
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

static ATOM RegisterSaverWinClass(HINSTANCE instHandle)
{
	WNDCLASS wc;
	HCURSOR blankCursor = LoadCursor(instHandle, MAKEINTRESOURCE(ID_BLANK_CURSOR));
	HICON spirexIcon = LoadIcon(instHandle, MAKEINTRESOURCE(ID_SPIREX_ICON));

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;   // OpenGL requires CS_OWNDC
	wc.lpfnWndProc = SaverWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instHandle;
	wc.hIcon = spirexIcon;
	wc.hCursor = blankCursor;
	wc.hbrBackground = NULL; 
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "ScrClass";
	ATOM classAtom = RegisterClass(&wc);
	if (classAtom == 0)
		DebugWin();
	return classAtom;
}

DWORD WINAPI SaverWin::StartThread(LPVOID SaverWinPtr)
{
	InterlockedIncrement(&SaverCount);

	SaverWin *ThisWin = (SaverWin*)SaverWinPtr;
	DWORD res = ThisWin->StartWindow();

	// The last SaverWin thread must kill the main thread
	if (InterlockedDecrement(&SaverCount) == 0) {
		// Retrieve resources used by saver
		Terminate();
		// Pop-up update dialog if there is an update
		if (ThereIsAnUpdate) {
			DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_UPDATE), NULL,
				UpdateDialogProc, 0);
		}
		// Kill parent thread
		while (PostThreadMessage(ThisWin->mParentThread, WM_QUIT, 0, 0) == 0)
		  Sleep(100);
	}

	return res;
}

DWORD SaverWin::StartWindow()
{
	int cx, cy;
	char *name;

	mThreadID = GetCurrentThreadId();
	srand(DEBUG ? 8732 : (time(NULL) ^ mThreadID));

	cx = mRect.right - mRect.left;
	cy = mRect.bottom - mRect.top;
	HWND w;
	MSG msg;

  if (mScrMode != SaverSettingsWin32::DisplayNative && 
  		mScrMode != SaverSettingsWin32::DisplayNo3D &&
  		mSaverMode != Preview) {
		bool res = false;
    if (mScrMode == SaverSettingsWin32::Display800x600) 
    	res = EnsureMode(800, 600);
     else 
    	res = EnsureMode(640, 480);

    if (res) {
    	DEVMODE newDM;
    	newDM.dmSize = sizeof(DEVMODE);
      EnumDisplaySettings(mScrDev, ENUM_CURRENT_SETTINGS, &newDM);
      mRect.left = newDM.dmPosition.x;
      mRect.top = newDM.dmPosition.y;
      cx = newDM.dmPelsWidth;
      cy = newDM.dmPelsHeight;
      char buf[100];
      wsprintf(buf, "New display:%s size:%d, %d position:%d, %d depth:%d freq:%dHz flags:%x",
        mScrDev, newDM.dmPelsWidth, newDM.dmPelsHeight, 
        newDM.dmPosition.x, newDM.dmPosition.y, newDM.dmBitsPerPel,
        newDM.dmDisplayFrequency, newDM.dmDisplayFlags);
      Debug(buf);
    }
  }

	if (mSaverMode == Preview) {
		name = "SaverPreview";
 		mScrMode = SaverSettingsWin32::DisplayNative;
		w = CreateWindowEx(
			0, "ScrClass", name,
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			mRect.left, mRect.top, cx, cy,
			mParentHwnd, NULL, mInstHandle,
			(LPVOID)this);
	} else {
		DWORD exstyle, style;
		if (DEBUG) {
    	char* buf = new char[100];  // who cares about this leak?
			wsprintf(buf, "Spirex: (%d,%d) to (%d,%d)",
				mRect.left, mRect.top,
				mRect.right, mRect.bottom);
			name = buf;
			cx /= 3;
			cy /= 3;
			exstyle = 0;
			style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
   		mScrMode = SaverSettingsWin32::DisplayNative;
		} else {
			name = "Spirex";
			exstyle = WS_EX_TOPMOST;
			style = WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		}
		w = CreateWindowEx(exstyle, "ScrClass", name, style,
			mRect.left, mRect.top, cx, cy,
			NULL, NULL, mInstHandle, (LPVOID)this);
	}

	Debug(w, name);
  // If the main window cannot be created, terminate
	// the application.
	if (w == NULL) {
		DebugWin();
		return FALSE;
	}

	BOOL bRet;

	// Show the window and paint its contents.

	ShowWindow(w, SW_SHOW);
	UpdateWindow(w);

	// Start the message loop.

	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0) {
		if (bRet == -1) {
			DebugWin();
			return FALSE;
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		// invert message priority, otherwise all the posted messages can block
		// the input and close messages
		if (PeekMessage(&msg, NULL, WM_CREATE, WM_USER_STOP, PM_REMOVE)) {
		  if (LOWORD(msg.message) == WM_QUIT)
		    break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
  // Restore screen settings if we changed them
  ExitMode();

	// Return the exit code to the system.
	return msg.wParam;
}


bool SaverWin::EnsureMode(DWORD width, DWORD height)
{ 
	if (mChangedMode) 
		return false;
  mChangedMode = true; 
  mChangedOK = false;
  // We want to change to 800x600. But if we merely change, without specifying
  // a refresh rate, then some systems give us a crummy one. So, we first
  // enum the available modes and pick the best one -- the least one above 70Hz,
  // or the largest if none are above 70. Some systems return only "0" and "1"
  // for refresh rates (i.e. referring to monitor-specific presets, rather than
  // frequencies in hertz). This possibility is already covered by our heuristic.
  int leastabove70 = -1, leastabove70i = -1, greatest = -1, greatesti = -1;
  for (int i=0; ; i++)
  { 
  	DEVMODE dm; 
  	ZeroMemory(&dm,sizeof(dm)); dm.dmSize = sizeof(dm); dm.dmDriverExtra = 0;
    BOOL res=EnumDisplaySettings(NULL,i,&dm);
    if (!res) break;
    if (dm.dmPelsWidth != width || dm.dmPelsHeight != height || dm.dmBitsPerPel != 16) 
    	continue;
    if (dm.dmDisplayFrequency > 70 && (leastabove70 == -1 || 
    		(int)dm.dmDisplayFrequency < leastabove70)) {
    	leastabove70 = dm.dmDisplayFrequency; leastabove70i = i;
    }
    if ((int)dm.dmDisplayFrequency > greatest) {
    	greatest = dm.dmDisplayFrequency; greatesti = i;
    }
  }
  
  int index = leastabove70i; 
  if (index == -1) 
  	index=greatesti; 
  if (index == -1) 
  	return false;
  	
  DEVMODE dm; ZeroMemory(&dm,sizeof(dm)); dm.dmSize=sizeof(dm); dm.dmDriverExtra=0;
  EnumDisplaySettings(NULL, index, &dm);
  LONG res = ChangeDisplaySettings(&dm, CDS_TEST);
  if (res != DISP_CHANGE_SUCCESSFUL) 
  	return false;
  // Changing the screen-mode causes some mouse-move messages to be sent.
  // In order that the saver doesn't respond by quitting, we lie to it:
	InterlockedIncrement(&mIsDialogActive);
  ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	InterlockedDecrement(&mIsDialogActive);
  GetCursorPos(&mInitCursorPos);
  //
  mChangedOK=true;
  return true;
}


void SaverWin::ExitMode()
{ 
	if (!mChangedMode || !mChangedOK) 
		return;
  ChangeDisplaySettings(0,0); // this magically resizes our window
  mChangedMode = false;
}


SaverWin::SaverWin(HWND hparwnd, SaverMode Mode, const SaverSettingsWin32& newSettings, 
  RECT screenRect, HINSTANCE hInst, const char* ScrDev, 
  SaverSettingsWin32::DisplayMode ScrMode)
:	mHwnd(NULL),
	mParentHwnd(hparwnd),
	mRect(screenRect),
	mScrDev(0),
	mScrMode(ScrMode),
	mOrigScrMode(ScrMode),
	mSaverMode(Mode),
	mInstHandle(hInst),
	mChangedOK(false),
	mChangedMode(false),
	mNextSaverWin(SaverWinList),
	mAnimator(NULL),
	mAnimate(false),
	mSettings(newSettings),
	mOriginal3DRender(newSettings.mSettings.m3DRender),
	mWTimerID(0),
	mPresetTimerID(0),
	mThread(NULL),
	mThreadID(0)
{
	static bool Initialized = false;

	SaverWinList = this;

	mParentThread = GetCurrentThreadId();
	
	if (ScrDev && *ScrDev) { 
	  // If I were good I would add a destructor for this. But SaverWins only get
	  // destroyed at program exit so why bother.
	  mScrDev = new char[strlen(ScrDev) + 1];
	  strcpy(mScrDev, ScrDev);
	}
		
	mSettings.PowerAwareness(mSaverMode == ScreenSaver);
	if (mScrMode < SaverSettingsWin32::ResolvedMode)
		mScrMode = SaverSettingsWin32::ResolvedMode;
	mSettings.mSettings.m3DRender = mScrMode != SaverSettingsWin32::DisplayNo3D;

	if (SaverSettingsWin32::Randomize) {
	  bool ThreeDRender = mSettings.mSettings.m3DRender;
		SaverSettingsWin32 temp;
		temp.ReadRandomPreset(!mSettings.mSettings.m3DRender);
    mSettings = temp;
		mSettings.mSettings.m3DRender = ThreeDRender;
	}

	if (!Initialized) {
		Initialize();
		ClassAtom = RegisterSaverWinClass(mInstHandle);
		Initialized = true;
	}

	// Windows OpenGL implementation limits each thread to one OpenGL context.
	// So, each SaverWin must run in its own thread.
	mThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartThread,
		(LPVOID)this, 0, NULL);
}

void SaverWin::Terminate()
{
	if (mMMTimerID) {
		timeKillEvent(mMMTimerID);
		timeEndPeriod(10);
	}
	mMMTimerID = NULL;
}

void SaverWin::Initialize()
{
	SpirexGL::LevelOfDetail = SaverSettingsWin32::LevelOfDetail;

	if (!mMMTimerID) {
		LARGE_INTEGER Frequency;
		if (QueryPerformanceCounter(&Frequency) &&
			(timeBeginPeriod(10) == TIMERR_NOERROR)) {
			if (!mMMTimerID)
				mMMTimerID = timeSetEvent(30, 10, SaverWin::MMTimeProc,
					NULL, TIME_PERIODIC);
		}
	}

}

static BOOL CALLBACK UpdateDialogProc(HWND hwnd, UINT msg,
		WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_COMMAND:
			int id = LOWORD(wParam);
			if (HIWORD(wParam) != BN_CLICKED)
				break;
			switch (id) {
				case IDC_NOT_NOW:
					EndDialog(hwnd, 0);
					return 0;
				case IDC_STOP:
					SaverSettingsWin32::ReadGeneralRegistry(false);
					SaverSettingsWin32::Check4Updates = false;
					SaverSettingsWin32::WriteGlobalSettings(false);
					EndDialog(hwnd, 0);
					return 0;
				case IDC_GET_IT:
					ShellExecute(NULL, "open", DownloadURLString,
						NULL, NULL, SW_SHOWNORMAL);
					EndDialog(hwnd, 0);
					return 0;
			}
			return FALSE;
	}
	return FALSE;
}

static void Histogram()
{
	static int	 count = 0;
	static DWORD beginTime;
	static DWORD lastTime = 0;
	static int   buckets[21];

	CHAR		buffer[256];

	DWORD		thisTime = GetTickCount();

	if (lastTime != 0) {
		DWORD interval = (thisTime - lastTime) / 5;

		if (interval < 0)	interval = 0;
		if (interval >= 20)	interval = 20;
		buckets[interval] += 1;

		count += 1;
	}

	lastTime = thisTime;

	if (count == 500) {

		double groupTime = thisTime - beginTime;
		sprintf(buffer, "%d calls took %f sec, avg = %f ms each",
			count, groupTime / 1000.0, groupTime / count);
		Debug(buffer);

		for (int i = 0; i < 20; ++i) {
			sprintf(buffer, "%3d-%3dms : %3d : %.*s",
				i*5, i*5+4, buckets[i], (buckets[i] + 9)/10,
				"=============================================================");
			Debug(buffer);
		}

		sprintf(buffer, "  > 100ms : %3d : %.*s",
			buckets[20], (buckets[20] + 9)/10,
			"=============================================================");
		Debug(buffer);

		count = 0;
	}

	if (count == 0) {
		beginTime = thisTime;

		for (int i = 0; i < 21; ++i)
			buckets[i] = 0;
	}
}
