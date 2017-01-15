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

#include "SaverSettingsWin32.h"
#include "SaverSettings.h"
#include "myWindows.h"
#include <regstr.h>
#include <shlwapi.h>

#pragma warning ( push )
#pragma warning ( disable : 4005 )
#include <shlobj.h>
#pragma warning ( pop )

#include "VersionInfo.h"
#include "Debug.h"
#include <stddef.h>
#include <stdlib.h>
#include "resource.h"


unsigned long	SaverSettingsWin32::PasswordDelay;   // in seconds
unsigned long	SaverSettingsWin32::MouseThreshold;  // in pixels
bool 			SaverSettingsWin32::MuteSound;
bool			SaverSettingsWin32::Randomize;
bool			SaverSettingsWin32::Check4Updates;
int 			SaverSettingsWin32::LevelOfDetail;
int 			SaverSettingsWin32::DisableHotCorner;
FILETIME		SaverSettingsWin32::LastUpdateCheck;
HWND			SaverSettingsWin32::ConfigWindow;

#define REGSTR_PATH_PLUSSCR (REGSTR_PATH_SETUP "\\Screen Savers")
#define REGSTR_PATH_CONFIG  ("Software\\OzoneSoft\\Spirex Saver")
#define REGSTR_PATH_TMPCONFIG  ("Software\\OzoneSoft\\Spirex Saver\\TempArea")
#define APPDATA_PATH_CONFIG  ("OzoneSoft\\Spirex Saver")
#define REGSTR_PATH_PRESETS  ("Software\\OzoneSoft\\Spirex Saver\\Saved Settings")

struct ModeName
{
	const char* mName;
	SaverSettings::RenderMode mMode;
};
static const ModeName NameList[] = {
	{ "Original2D", SaverSettings::Original2D},
	{ "OriginalCounterRotate2D", SaverSettings::OriginalCounterRotate2D},
	{ "Original2DWith3DGeometry", SaverSettings::Original2DWith3DGeometry},
	{ "Curves", SaverSettings::Curves},
	{ "Spheres", SaverSettings::Spheres},
	{ "PointModeSpheres", SaverSettings::PointModeSpheres},
	{ "Disks", SaverSettings::Disks},
	{ "Cubes", SaverSettings::Cubes},
	{ "Wrapped Cubes", SaverSettings::WrappedCubes},
	{ "Squares", SaverSettings::Squares},
	{ "Conics", SaverSettings::Conics},
	{ "Cylinders", SaverSettings::Cylinders},
	{ "Toroids", SaverSettings::Toroids},
	{ "Teapots", SaverSettings::Teapots}
};
static const int NameListCount = sizeof(NameList) / sizeof(NameList[0]);

static const int CurrentVersion = 10;


void SaverSettingsWin32::InitDefaults()
{
	mSettings.mVersion = CurrentVersion;
	mSettings.InitDefaults();
	mSettings.setTexture(IDT_SPIREX);
}

SaverSettingsWin32::SaverSettingsWin32()
{
	mSettings.mVersion = CurrentVersion;
	mSettings.setTexture(IDT_SPIREX);
}

SaverSettingsWin32::SaverSettingsWin32(const SaverSettingsWin32& oldSettings)
:	mSettings(oldSettings.mSettings)
{
	mSettings.mVersion = CurrentVersion;
}


SaverSettingsWin32& SaverSettingsWin32::operator=(const SaverSettingsWin32& oldSettings)
{
	if (this == &oldSettings) return *this;
	mSettings = oldSettings.mSettings;
	return *this;
}

void SaverSettingsWin32::ReadGeneralRegistry(bool temp)
{
	LONG res;
	HKEY skey;
	DWORD valtype, valsize, val;
	FILETIME time;

	SaverSettingsWin32::PasswordDelay = 15;
	SaverSettingsWin32::MouseThreshold = 50;
	SaverSettingsWin32::MuteSound = true;
	SaverSettingsWin32::ConfigWindow = NULL;
	SaverSettingsWin32::Randomize = false;
	SaverSettingsWin32::Check4Updates = true;
	SaverSettingsWin32::LevelOfDetail = 100;
	SaverSettingsWin32::DisableHotCorner = -1;
	SaverSettingsWin32::LastUpdateCheck.dwLowDateTime = 0;
	SaverSettingsWin32::LastUpdateCheck.dwHighDateTime = 0;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_PLUSSCR, 0,
			KEY_ALL_ACCESS, &skey);

	if (res == ERROR_SUCCESS) {
		valsize = sizeof(val);
		res = RegQueryValueEx(skey, "Password Delay", 0, &valtype,
				(LPBYTE)&val, &valsize);
		if (res == ERROR_SUCCESS && valtype == REG_DWORD)
			SaverSettingsWin32::PasswordDelay = val;

		valsize = sizeof(val);
		res = RegQueryValueEx(skey, "Mouse Threshold", 0, &valtype,
				(LPBYTE)&val, &valsize);
		if (res == ERROR_SUCCESS && valtype == REG_DWORD)
			SaverSettingsWin32::MouseThreshold = val;

		valsize = sizeof(val);
		res = RegQueryValueEx(skey, "Mute Sound", 0, &valtype,
				(LPBYTE)&val, &valsize);
		if (res == ERROR_SUCCESS && valtype == REG_DWORD)
			SaverSettingsWin32::MuteSound = val != 0;

		RegCloseKey(skey);
	}

	res = RegOpenKeyEx(HKEY_CURRENT_USER, temp ? REGSTR_PATH_TMPCONFIG : REGSTR_PATH_CONFIG, 0,
			KEY_ALL_ACCESS, &skey);

	if (res == ERROR_SUCCESS) {
		valsize = sizeof(val);
		res = RegQueryValueEx(skey, "Randomize", 0, &valtype,
				(LPBYTE)&val, &valsize);
		if (res == ERROR_SUCCESS && valtype == REG_DWORD)
			SaverSettingsWin32::Randomize = val != 0;

		valsize = sizeof(val);
		res = RegQueryValueEx(skey, "CheckForUpdates", 0, &valtype,
				(LPBYTE)&val, &valsize);
		if (res == ERROR_SUCCESS && valtype == REG_DWORD)
			SaverSettingsWin32::Check4Updates = (val != 0) && !temp;

		valsize = sizeof(time);
		res = RegQueryValueEx(skey, "LastUpdateCheck", 0, &valtype,
				(LPBYTE)&time, &valsize);
		if (res == ERROR_SUCCESS && valtype == REG_BINARY && valsize == 8)
			SaverSettingsWin32::LastUpdateCheck = time;

		valsize = sizeof(val);
		res = RegQueryValueEx(skey, "LevelOfDetail", 0, &valtype,
				(LPBYTE)&val, &valsize);
		if (res == ERROR_SUCCESS && valtype == REG_DWORD && val >= 50 && val <= 200)
			SaverSettingsWin32::LevelOfDetail = val;

		valsize = sizeof(val);
		res = RegQueryValueEx(skey, "DisableHotCorner", 0, &valtype,
				(LPBYTE)&val, &valsize);
		if ((res == ERROR_SUCCESS) && (valtype == REG_DWORD) && (val >= 0) && (val <= 4))
			SaverSettingsWin32::DisableHotCorner = val - 1;

		RegCloseKey(skey);
	}
}

void SaverSettingsWin32::WriteGlobalSettings(bool temp)
{
	LONG res;
	HKEY skey;
	DWORD valsize, val;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, temp ? REGSTR_PATH_TMPCONFIG : REGSTR_PATH_CONFIG, 0,
			KEY_ALL_ACCESS, &skey);
	if (res != ERROR_SUCCESS) return;

	valsize = sizeof(val);
	val = Randomize ? 1 : 0;
	res = RegSetValueEx(skey, "Randomize", 0, REG_DWORD, (BYTE *)(&val), valsize);
	val = Check4Updates ? 1 : 0;
	res = RegSetValueEx(skey, "CheckForUpdates", 0, REG_DWORD, (BYTE *)(&val), valsize);
	val = (DWORD)LevelOfDetail;
	res = RegSetValueEx(skey, "LevelOfDetail", 0, REG_DWORD, (BYTE *)(&val), valsize);
	val = (DWORD)(DisableHotCorner + 1);	// add 1 to convert -1 .. 3 to (unsigned) 0 .. 4
	res = RegSetValueEx(skey, "DisableHotCorner", 0, REG_DWORD, (BYTE *)(&val), valsize);

	RegCloseKey(skey);
}

bool SaverSettingsWin32::DeleteAllSettings()
{
	return SHDeleteKey(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG) == ERROR_SUCCESS;
}

bool SaverSettingsWin32::Time2CheckUpdate()
{
	if (!Check4Updates)
		return false;
		
	// Make sure we only do this check once per invocation
	static long AlreadyChecked = 0;
	if (InterlockedCompareExchange(&AlreadyChecked, 1, 0) != 0)
	  return false;

	SYSTEMTIME st;
	FILETIME ft;
	GetSystemTime(&st);
	if (!SystemTimeToFileTime(&st, &ft))
		return false;

	if ((ft.dwHighDateTime - LastUpdateCheck.dwHighDateTime) < 1400)
		return false;

	LastUpdateCheck = ft;

	LONG res;
	HKEY skey;
	DWORD valsize;
	FILETIME time;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG, 0,
			KEY_ALL_ACCESS, &skey);
	if (res != ERROR_SUCCESS)
		return false;

	valsize = sizeof(time);
	time = LastUpdateCheck;
	res = RegSetValueEx(skey, "LastUpdateCheck", 0, REG_BINARY, (BYTE *)(&time), valsize);

	RegCloseKey(skey);

	return true;
}

void SaverSettingsWin32::InitPresetsRegistry()
{
	LONG res;

	res = RegDeleteKey(HKEY_CURRENT_USER, REGSTR_PATH_PRESETS);

	SaverSettingsWin32 s;

	//					         crv len spd evol thick color fixed points tri-axial
	//  Mode
	SaverSettings earth(12, 50, 10, 50, false, false, false, false, false,
		SaverSettings::WrappedCubes, IDT_EARTH);
	s.mSettings = earth;
	s.mSettings.mVersion = CurrentVersion;
	s.WritePreset("Earth");

	SaverSettings spirals(16, 200, 50, 50, false, true, false, false, false,
		SaverSettings::OriginalCounterRotate2D);
	s.mSettings = spirals;
	s.mSettings.mVersion = CurrentVersion;
	s.WritePreset("Spirals");

	SaverSettings ribbons(32, 15, 33, 50, true, true, false, false, false,
		SaverSettings::Curves, IDT_SPIREX);
	s.mSettings = ribbons;
	s.mSettings.mVersion = CurrentVersion;
	s.WritePreset("Ribbons");

	SaverSettings spheres(24, 50, 15, 50, false, true, false, true, false,
		SaverSettings::Spheres);
	s.mSettings = spheres;
	s.mSettings.mVersion = CurrentVersion;
	s.WritePreset("Spheres");

	SaverSettings blobs(24, 50, 15, 50, false, true, false, false, true,
		SaverSettings::Spheres, IDT_SPIREX);
	s.mSettings = blobs;
	s.mSettings.mVersion = CurrentVersion;
	s.WritePreset("Blobs");

	SaverSettings torqued(16, 50, 15, 50, false, true, false, false, true,
		SaverSettings::Conics, IDT_EUROPA);
	s.mSettings = torqued;
	s.mSettings.mVersion = CurrentVersion;
	s.WritePreset("Torqued Ellipses");

	SaverSettings homer(12, 50, 15, 50, false, false, false, false, true,
		SaverSettings::Toroids, IDT_EARTH);
	s.mSettings = homer;
	s.mSettings.mVersion = CurrentVersion;
	s.WritePreset("Planet Homer");
}


bool SaverSettingsWin32::ReadConfig(const char* path, const char* settingName)
{
	LONG res;
	DWORD valtype, valsize;
	//char bufferChars[MAX_PATH + 1];
	union {
		char mSZ[MAX_PATH + 1];
		DWORD mDWORD;
	} buffer;
	char pathBuffer[MAX_PATH + 1];
	char valueNameBuffer[MAX_PATH + 1];

	strcpy(pathBuffer, path);
	strcat(pathBuffer, "\\");
	strcat(pathBuffer, settingName);
	mSettings.InitDefaults();
	mSettings.mVersion = mSettings.mSize = 0;

	HKEY skey;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, pathBuffer, 0, KEY_ALL_ACCESS, &skey);
	if (res != ERROR_SUCCESS)
		return false;

	valsize = MAX_PATH;

	DWORD maxNameLength, maxValueLength, valueNum;

	res = RegQueryInfoKey(skey, 0, 0, 0, 0, 0, 0,
			&valueNum, &maxNameLength, &maxValueLength, 0, 0);
	if ((res != ERROR_SUCCESS) || (valueNum <= 0) ||
		(maxNameLength > MAX_PATH) || (maxValueLength > MAX_PATH)) {
		RegCloseKey(skey);
		res = RegDeleteKey(HKEY_CURRENT_USER, path);
		if (res != ERROR_SUCCESS)
			DebugWin();
		return false;
	}

	// Iterate through all the named values, putting their contents into the
	// SaverSettings. If a named value is unknown or has the wrong data then
	// reset the mVersion member and stop. This will have the affect of deleting
	// the key and returning false.
	for (DWORD i = 0; i < valueNum; i++) {
		maxNameLength = MAX_PATH + 1;
		valsize = MAX_PATH + 1;
		res = RegEnumValue(skey, i, valueNameBuffer, &maxNameLength, NULL,
			&valtype, (BYTE*)(buffer.mSZ), &valsize);
		if (res != ERROR_SUCCESS) {
			wsprintf(buffer.mSZ, "Error trying to read value: %s", valueNameBuffer);
			Debug(buffer.mSZ);
			continue;
		}


		if (!strcmp(valueNameBuffer, "TexturePath")) {
			if (valtype != REG_SZ) {
				mSettings.mVersion = 0;
				break;
			}
			mSettings.setTexture(buffer.mSZ);
			continue;
		}
		if (!strcmp(valueNameBuffer, "3DMode")) {
			if (valtype != REG_SZ) {
				mSettings.mVersion = 0;
				break;
			}

			int i;
			for (i = 0; i < NameListCount; i++)
				if (!strcmp(NameList[i].mName, buffer.mSZ)) {
					mSettings.mMode = NameList[i].mMode;
					break;
				}

			if (i == NameListCount) {
				mSettings.mVersion = 0;
				break;
			}

      if (mSettings.mMode == SaverSettings::PointModeSpheres) {
        mSettings.mMode = SaverSettings::Spheres;
        mSettings.mPoints = true;
      }

			continue;
		}

		if (valtype != REG_DWORD) {
			mSettings.mVersion = 0;
			break;
		}

		if (!strcmp(valueNameBuffer, "Version")) {
			mSettings.mVersion = buffer.mDWORD;
			continue;
		}
		if (!strcmp(valueNameBuffer, "Size")) {
			mSettings.mSize = buffer.mDWORD;
			continue;
		}
		if (!strcmp(valueNameBuffer, "Count")) {
			mSettings.mCurveCount = buffer.mDWORD;
			continue;
		}
		if (!strcmp(valueNameBuffer, "Length")) {
			mSettings.mCurveLength = buffer.mDWORD;
			continue;
		}
		if (!strcmp(valueNameBuffer, "Speed")) {
			mSettings.mAngleChangeRate = buffer.mDWORD;
			continue;
		}
		if (!strcmp(valueNameBuffer, "ChangeRate")) {
			mSettings.mEvolutionRate = buffer.mDWORD;
			continue;
		}
		if (!strcmp(valueNameBuffer, "ThickLines")) {
			mSettings.mThickLines = buffer.mDWORD;
			continue;
		}
		if (!strcmp(valueNameBuffer, "Color")) {
			mSettings.mInColor = buffer.mDWORD;
			continue;
		}
		if (!strcmp(valueNameBuffer, "FixedSize")) {
			mSettings.mFixed = buffer.mDWORD;
			continue;
		}
    if (!strcmp(valueNameBuffer, "Points")) {
            mSettings.mPoints = buffer.mDWORD;
            continue;
    }
    if (!strcmp(valueNameBuffer, "TriAxial")) {
            mSettings.mTriAxial = buffer.mDWORD;
            continue;
    }

    if (!strcmp(valueNameBuffer, "TwoDSubset")) {
            continue;   // Recognize but ignore old setting
    }

		// The named value is unknown
		mSettings.mVersion = 0;
		break;
	}

	if ((mSettings.mVersion != CurrentVersion) ||
		// (mSettings.mSize != sizeof(SaverSettings)) ||
		(mSettings.mCurveCount == 0) ||
		(mSettings.mCurveCount > SaverSettings::MaxCurveCount) ||
		(mSettings.mCurveLength < 3) ||
		(mSettings.mCurveLength > 500) ||
		(mSettings.mAngleChangeRate == 0) ||
		(mSettings.mAngleChangeRate > 100) ||
		(mSettings.mEvolutionRate == 0) ||
		(mSettings.mEvolutionRate > 100))
	{
		RegCloseKey(skey);

		res = RegDeleteKey(HKEY_CURRENT_USER, pathBuffer);
		if (res != ERROR_SUCCESS)
			DebugWin();

		return false;
	}


	res = RegCloseKey(skey);

	if (res != ERROR_SUCCESS) {
		res = RegDeleteKey(HKEY_CURRENT_USER, path);
		if (res != ERROR_SUCCESS)
			DebugWin();

		return false;
	}

	return true;
}

void SaverSettingsWin32::WriteConfig(const char* path, const char* settingName)
{
	LONG res;
	HKEY skey;
	DWORD disp;
	DWORD valueDWORD;
	char pathBuffer[MAX_PATH + 1];

	strcpy(pathBuffer, path);
	strcat(pathBuffer, "\\");
	strcat(pathBuffer, settingName);

	res = RegCreateKeyEx(HKEY_CURRENT_USER, pathBuffer, 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &skey, &disp);
	if (res != ERROR_SUCCESS) return;

	do {
		valueDWORD = mSettings.mVersion;
		res = RegSetValueEx(skey, "Version", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
		if (res != ERROR_SUCCESS) break;

		valueDWORD = mSettings.mSize;
		res = RegSetValueEx(skey, "Size", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
		if (res != ERROR_SUCCESS) break;

		valueDWORD = mSettings.mCurveCount;
		res = RegSetValueEx(skey, "Count", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
		if (res != ERROR_SUCCESS) break;

		valueDWORD = mSettings.mCurveLength;
		res = RegSetValueEx(skey, "Length", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
		if (res != ERROR_SUCCESS) break;

		valueDWORD = mSettings.mAngleChangeRate;
		res = RegSetValueEx(skey, "Speed", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
		if (res != ERROR_SUCCESS) break;

		valueDWORD = mSettings.mEvolutionRate;
		res = RegSetValueEx(skey, "ChangeRate", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
		if (res != ERROR_SUCCESS) break;

		valueDWORD = mSettings.mThickLines;
		res = RegSetValueEx(skey, "ThickLines", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
		if (res != ERROR_SUCCESS) break;

		valueDWORD = mSettings.mInColor;
		res = RegSetValueEx(skey, "Color", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
		if (res != ERROR_SUCCESS) break;

		valueDWORD = mSettings.mFixed;
		res = RegSetValueEx(skey, "FixedSize", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
		if (res != ERROR_SUCCESS) break;

    valueDWORD = mSettings.mPoints;
    res = RegSetValueEx(skey, "Points", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
    if (res != ERROR_SUCCESS) break;

    valueDWORD = mSettings.mTriAxial;
    res = RegSetValueEx(skey, "TriAxial", 0, REG_DWORD, (BYTE*)&valueDWORD, sizeof(DWORD));
    if (res != ERROR_SUCCESS) break;

		res = RegSetValueEx(skey, "TexturePath", 0, REG_SZ, (BYTE*)mSettings.getTextureStr(),
			mSettings.getTextureStrlen() + 1);
		if (res != ERROR_SUCCESS) break;

		int i;
		for (i = 0; i < NameListCount; i++)
			if (NameList[i].mMode == mSettings.mMode) {
				res = RegSetValueEx(skey, "3DMode", 0, REG_SZ, (BYTE*)NameList[i].mName,
					strlen(NameList[i].mName) + 1);
				if (res != ERROR_SUCCESS) return;
				break;
			}

		if (i == NameListCount)
			break;

		RegCloseKey(skey);
		return;
	} while (0);

	RegCloseKey(skey);
	res = RegDeleteKey(HKEY_CURRENT_USER, pathBuffer);
	if (res != ERROR_SUCCESS)
		DebugWin();

}

bool SaverSettingsWin32::ReadCurrentSettings(bool temp)
{
	bool res;
	if (temp) {
  	res = ReadConfig(REGSTR_PATH_TMPCONFIG, "Settings");
  	if (!res)
  		InitDefaults();
	} else {
  	res = ReadConfig(REGSTR_PATH_CONFIG, "Settings");
  	if (!res)
  		InitDefaults();
	}
	return res;
}

bool SaverSettingsWin32::ReadPreset(const char* presetName)
{
	if (ReadConfig(REGSTR_PATH_PRESETS, presetName)) {
        mSettings.mName.assign(presetName);
        return true;
	}
	return false;
}

bool SaverSettingsWin32::operator==(const SaverSettingsWin32& other) const
{
	return mSettings == other.mSettings;
}

bool SaverSettingsWin32::ComparePreset(const char* presetName) const
{
	SaverSettingsWin32 temp;

	return temp.ReadPreset(presetName) && *this == temp;
}

void SaverSettingsWin32::DeletePreset(const char* presetName)
{
	LONG res;
	char pathBuffer[MAX_PATH + 1];

	strcpy(pathBuffer, REGSTR_PATH_PRESETS);
	strcat(pathBuffer, "\\");
	strcat(pathBuffer, presetName);

	res = RegDeleteKey(HKEY_CURRENT_USER, pathBuffer);
	if (res != ERROR_SUCCESS)
		DebugWin();
}

void SaverSettingsWin32::WritePreset(const char* presetName)
{
	WriteConfig(REGSTR_PATH_PRESETS, presetName);
}

void SaverSettingsWin32::WriteCurrentSettings(bool temp)
{
	WriteConfig(temp ? REGSTR_PATH_TMPCONFIG : REGSTR_PATH_CONFIG, "Settings");
}

void SaverSettingsWin32::ReadRandomPreset()
{
	SaverSettingsWin32::PresetIter presets;
	if (presets.mPresetCount == 0) {
	  InitDefaults();
	  return;
	}
	int pick = rand() % presets.mPresetCount;

	for (int i = 0; i <= pick; i++)
		if (!presets.nextPreset()) {
			ReadCurrentSettings(false);	// this shouldn't happen
			return;					// but a little paranoia doesn't hurt
		}

	ReadPreset(presets.currentPresetName());
}

void SaverSettingsWin32::PowerAwareness(bool isScreenSaver)
{
	SYSTEM_POWER_STATUS sps;
	GetSystemPowerStatus(&sps);
}



SaverSettingsWin32::PresetIter::PresetIter()
:	mPresetCount(0),
	mPresetKey(0),
	mCurrentSetting(0),
	mCurrentIndex(0),
	mBuffer(0),
	mBufferSize(0)
{
	InitIter();
	if (mPresetCount == 0) {
		InitPresetsRegistry();
		InitIter();
		if (mPresetCount == 0) {
			// Something very bad has happened, this is the best we can do
			mPresetCount = mCurrentSetting = 1;
		}
	}
}

void SaverSettingsWin32::PresetIter::InitIter()
{
	LONG res;
	SaverSettingsWin32 s;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_PRESETS,
			0, KEY_ALL_ACCESS, &mPresetKey);
	if (res != ERROR_SUCCESS) return;

	DWORD maxNameLength, keyNum;
	res = RegQueryInfoKey(mPresetKey, 0, 0, 0, &keyNum, &maxNameLength, 0,
		0, 0, 0, 0, 0);
	if (res != ERROR_SUCCESS) return;

	mBufferSize = maxNameLength + 1;
	mBuffer = new char[mBufferSize];

  mPresetCount = 32767;
  int count = 0;
  while (nextPreset())
    count++;
  
  mPresetCount = count;
  mCurrentIndex = mCurrentSetting = 0;

  if (!mPresetCount) {
		RegCloseKey(mPresetKey);
		mPresetKey = 0;
  }
}

SaverSettingsWin32::PresetIter::~PresetIter()
{
	delete[] mBuffer;
	if (mPresetKey != NULL)
		RegCloseKey(mPresetKey);
}



bool SaverSettingsWin32::PresetIter::nextPreset()
{
	if (mCurrentSetting >= mPresetCount)
		return false;

	DWORD keyNameLength = mBufferSize;
	SaverSettingsWin32 s;

	while (RegEnumKeyEx(mPresetKey, mCurrentIndex++, mBuffer, &keyNameLength, 0, 0, 0, 0) == ERROR_SUCCESS) {
		if (s.ReadPreset(mBuffer)) {
			mCurrentSetting++;
      return true;
		}
		keyNameLength = mBufferSize;
	}

	mCurrentSetting = mPresetCount;
	return false;
}

#if 0
// The following example can be used to enable or disable the
// backup privilege. By making the indicated substitutions, you can
// also use this example to enable or disable the restore privilege 
// Use the following to enable the privilege:
//   hr = ModifyPrivilege(SE_BACKUP_NAME, TRUE);
// Use the following to disable the privilege:
//   hr = ModifyPrivilege(SE_BACKUP_NAME, FALSE);
// Use SE_RESTORE_NAME for the restore privilege.

static HRESULT ModifyPrivilege(LPCTSTR szPrivilege, BOOL fEnable)
{
  HRESULT hr = S_OK;
  TOKEN_PRIVILEGES NewState;
  LUID             luid;
  HANDLE hToken    = NULL;

  // Open the process token for this process.
  if (!OpenProcessToken(GetCurrentProcess(), 
      TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ))
  {
    Debug("Failed OpenProcessToken\n");
    return ERROR_FUNCTION_FAILED;
  }

  // Get the local unique id for the privilege.
  if ( !LookupPrivilegeValue( NULL, szPrivilege, &luid ))
  {
    CloseHandle( hToken );
    Debug("Failed LookupPrivilegeValue\n");
    return ERROR_FUNCTION_FAILED;
  }

  // Assign values to the TOKEN_PRIVILEGE structure.
  NewState.PrivilegeCount = 1;
  NewState.Privileges[0].Luid = luid;
  NewState.Privileges[0].Attributes = (fEnable ? SE_PRIVILEGE_ENABLED : 0);

  // Adjust the token privilege.
  if (!AdjustTokenPrivileges(hToken, FALSE, &NewState, sizeof(NewState), 
      NULL, NULL))
  {
    Debug("Failed AdjustTokenPrivileges\n");
    hr = ERROR_FUNCTION_FAILED;
  }

  // Close the handle.
  CloseHandle(hToken);

  return hr;
}


class StaticInitHelper
{
private:
  bool  mLoadedKeys;
  char  mFileName[MAX_PATH + 1];  
public:
  StaticInitHelper();
  ~StaticInitHelper();
};

// Doesn't work right now: StaticInitHelper SIH;

StaticInitHelper::StaticInitHelper()
: mLoadedKeys(false)
{
  mFileName[0] = '\0';

  HRESULT res2;
  res2 = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT,
    mFileName);
  
  if (res2 == S_OK) {
    PathAppend(mFileName, APPDATA_PATH_CONFIG);
    PathAppend(mFileName, "Settings");
  }
 	
  LONG res;
  HKEY skey;
 	res = RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG, 0,
		KEY_ALL_ACCESS, &skey);

	if (res == ERROR_SUCCESS) {
	  RegCloseKey(skey);
	} else if (mFileName[0]) {
  	if (ModifyPrivilege(SE_RESTORE_NAME, TRUE) != S_OK)
  	  return;
  	
  	// Why am I getting an Access Denied error here?
    res = RegLoadKey(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG, mFileName);
  	ModifyPrivilege(SE_RESTORE_NAME, FALSE);
    if (res == ERROR_SUCCESS)
      mLoadedKeys = true;
	}
}

StaticInitHelper::~StaticInitHelper()
{
  if (mLoadedKeys) {
    RegUnLoadKey(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG);
  } else {
    LONG res;
    HKEY skey;
   	res = RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG, 0,
  		KEY_ALL_ACCESS, &skey);

  	if (ModifyPrivilege(SE_BACKUP_NAME, TRUE) != S_OK)
  	  return;
  	if (res == ERROR_SUCCESS) {
  	  char DirName[MAX_PATH + 1];
  	  strcpy(DirName, mFileName);
  	  PathRemoveFileSpec(DirName);
  	  res = SHCreateDirectoryEx(NULL, DirName, NULL);
  	  BOOL res3 = DeleteFile(mFileName);
  	  res = RegSaveKey(skey, mFileName, NULL);
  	  if (res == ERROR_SUCCESS) {
    	  //res = RegCloseKey(skey);
    	  res = SHDeleteKey(skey, "");
    	  //res = RegDeleteKey(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG);
  	  }
  	}
  }
}

#endif // 0