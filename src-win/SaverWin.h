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

#ifndef INCLUDED_SAVERWIN
#define INCLUDED_SAVERWIN

#include "myWindows.h"
#include "SaverSettings.h"
#include "SaverSettingsWin32.h"
#include <memory>


class Spirex;

class SaverWin
{
public:
	enum SaverMode {ScreenSaver = 0, Preview = 1};
	HWND mHwnd;
private:
	static ATOM ClassAtom;
	HWND mParentHwnd;
	RECT mRect;
	int height, width;
	SaverMode mSaverMode;
	HINSTANCE mInstHandle;
	DWORD mInitTime;    		// in ms
	POINT mInitCursorPos;
	static long  mIsDialogActive;
	static bool  ReallyClose;	// for NT, so we know if a WM_CLOSE came from us or it.
	static long SaverCount;
	static bool	ThereIsAnUpdate;
	
	bool mChangedOK;
	bool mChangedMode;
	bool EnsureMode(DWORD width, DWORD height);
	void ExitMode();
	
	static SaverWin* SaverWinList;
	SaverWin* mNextSaverWin;

	std::unique_ptr<Spirex> mAnimator;
	bool mAnimate;

	SaverSettingsWin32 mSettings;		// current settings, owned by animate thread
	bool	mOriginal3DRender;
	
	DWORD	mLastRender;	// ms
	DWORD	mLastCount;
	int		mRenderCount;
	int		mFrameRate;
	DWORD	mLastTick;

	void CloseSaverWindow();
	void Animate(int steps);
	
	static UINT mMMTimerID;
	UINT_PTR mWTimerID;
	UINT_PTR mPresetTimerID;
	static int AnimateCount;
	static void CALLBACK MMTimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
	
	static DWORD WINAPI StartThread(LPVOID SaverWinPtr);
	DWORD StartWindow();
	HANDLE mThread;
	DWORD mThreadID;
	DWORD mParentThread;
	
	void PrepareToExit(HWND hwnd);
	static void Initialize();
public:
	SaverWin(	HWND hparwnd, SaverMode mSaverMode, const SaverSettingsWin32& newSettings, 
				RECT screenRect, HINSTANCE hInst);
	static void Terminate();
	void StopAnimation();
	void StartAnimation();
	void NewSaverSettings(const SaverSettingsWin32& newSettings);
	void Clear();
	
	LRESULT CALLBACK SaverWindowProc(HWND hwnd, UINT msg, 
		WPARAM wParam, LPARAM lParam);
};


#endif //INCLUDED_SAVERWIN