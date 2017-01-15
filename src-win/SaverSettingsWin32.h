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

#ifndef INCLUDED_SAVERSETTINGSWIN32
#define INCLUDED_SAVERSETTINGSWIN32

#include "myWindows.h"
#include <regstr.h>
#include "SaverSettings.h"
#include <vector>

class SaverSettingsWin32
{ 
public:
	SaverSettings   mSettings;
	static bool     Randomize;
	static bool     Check4Updates;
	static FILETIME LastUpdateCheck;
	static int      LevelOfDetail;
	static int      DisableHotCorner;
	
	
	
	static unsigned long    PasswordDelay;   // in seconds
	static unsigned long    MouseThreshold;  // in pixels
	static bool             MuteSound;
	static HWND             ConfigWindow;
	
	SaverSettingsWin32();
	SaverSettingsWin32(const SaverSettingsWin32&);
	SaverSettingsWin32& operator=(const SaverSettingsWin32&);


	static void ReadGeneralRegistry(bool temp);
	static void WriteGlobalSettings(bool temp);
	static bool DeleteAllSettings();
	bool ReadCurrentSettings(bool temp);
	void WriteCurrentSettings(bool temp);
	void ReadRandomPreset();
	bool ReadPreset(const char* presetName);
	void WritePreset(const char* presetName);
	bool ComparePreset(const char* presetName) const;
	void PowerAwareness(bool isScreenSaver);
	static void InitPresetsRegistry();
	static void DeletePreset(const char* presetName);
	static bool Time2CheckUpdate();
	
	bool operator==(const SaverSettingsWin32& other) const;

	class PresetIter
	{
	public:
		bool nextPreset();
		inline char* currentPresetName() { return mBuffer.data(); }
		PresetIter();
		~PresetIter();
		DWORD mPresetCount;
		
	private:
		HKEY	mPresetKey;
		DWORD	mCurrentSetting;
		DWORD mCurrentIndex;
		std::vector<char>	mBuffer;
		void	InitIter();
		
		PresetIter(const PresetIter&);
		PresetIter& operator=(const PresetIter&);
			// not implemented, can't be assigned or copied
	};
	
		
	friend class SaverSettingsWin32::PresetIter;
	
private:
	void InitDefaults();
	bool ReadConfig(const char* path, const char* valueName);
	void WriteConfig(const char* path, const char* valueName);

};


#endif // INCLUDED_SAVERSETTINGSWIN32