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

#include "Configure.h"
#include "myWindows.h"
//#include <GdiPlus.h>
#include <commdlg.h>
#include <prsht.h>
#include "resource.h"
#include "SaverSettingsWin32.h"
#include "Debug.h"
#include "SaverWin.h"
#include "LinkWin.h"
#include <stdlib.h>
#include <commctrl.h>
#include <ctype.h>
#include <string.h>
#include "VersionInfo.h"
#include "Texture.h"
#include <shellapi.h>
#include <Shlobj.h>

#include <pshpack1.h>
typedef struct DLGTEMPLATEEX
{
  WORD dlgVer;
  WORD signature;
  DWORD helpID;
  DWORD exStyle;
  DWORD style;
  WORD cDlgItems;
  short x;
  short y;
  short cx;
  short cy;
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;
#include <poppack.h>

#include <vector>
#include <string>
#include <algorithm>
#include <memory>

using namespace Gdiplus;


static SaverSettingsWin32 Configuration;
static SaverWin* PreviewWin = 0;
static std::unique_ptr<Texture> Thumbnail;
static LinkWin* URLcontrol;
static bool ControlsValid;
static OPENFILENAME Ofn;				// common dialog box structure
static char szFile[MAX_PATH + 1];		// buffer for file name
static char szFileName[MAX_PATH + 1];
static char InitDir[MAX_PATH + 1];
static HINSTANCE hInst;
static HWND PropSheetHwnd;
static HWND Cancelbutton = NULL;
static HWND PreviewButton;
static HWND PresetCombo = 0;
static HWND PresetList = 0;
static WNDPROC OrigSheetProc;

static void GetSettingsFromDlg(HWND hwnd, SaverSettingsWin32& settings);
static void SetDlgFromSettings(HWND hwnd, const SaverSettingsWin32& settings);
static void ReadPresetsRegistry(HWND hwnd, bool combo = true);

static BOOL CALLBACK ConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK AdvConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SheetSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND CreatePropSheet(HINSTANCE hInst, HWND hwnd);
static void SetupPreview();

#define ID_APPLY_NOW                    0x3021

struct ModeName
{
  char* mName;
  SaverSettings::RenderMode mMode;
};
static const ModeName NameList[] = {
  { "Original 2D", SaverSettings::Original2D},
  { "2D w/counter rotation", SaverSettings::OriginalCounterRotate2D},
  { "2D w/3D geometry", SaverSettings::Original2DWith3DGeometry},
  { "3D Curves", SaverSettings::Curves},
  { "3D Spheres", SaverSettings::Spheres},
  { "3D Disks", SaverSettings::Disks},
  { "3D Cubes", SaverSettings::Cubes},
  { "3D Wrapped Cubes", SaverSettings::WrappedCubes},
  { "3D Squares", SaverSettings::Squares},
  { "3D Conics", SaverSettings::Conics},
  { "3D Cylinders", SaverSettings::Cylinders},
  { "3D Toroids", SaverSettings::Toroids},
  { "3D Utah Teapots", SaverSettings::Teapots}
};
static const int NameListCount = sizeof(NameList) / sizeof(NameList[0]);

static void ChangeToolTip(HWND hwnd, HWND TTHWND, int control, const char* tip, bool getEditBox);


static void GetSettingsFromDlg(HWND hwnd, SaverSettingsWin32& settings)
{
  SaverSettingsWin32::Randomize = (IsDlgButtonChecked(hwnd, IDC_RANDOM) == BST_CHECKED);

  settings.mSettings.mThickLines = (IsDlgButtonChecked(hwnd, IDC_THICK) == BST_CHECKED);
  settings.mSettings.mFixed = (IsDlgButtonChecked(hwnd, IDC_FIXED) == BST_CHECKED);
  settings.mSettings.mPoints = (IsDlgButtonChecked(hwnd, IDC_POINTS) == BST_CHECKED);
  settings.mSettings.mTriAxial = (IsDlgButtonChecked(hwnd, IDC_TRIAXIAL) == BST_CHECKED);
  settings.mSettings.mInColor = (IsDlgButtonChecked(hwnd, IDC_COLOR) == BST_CHECKED);
  settings.mSettings.mCurveCount = SendDlgItemMessage(hwnd, IDC_COUNT, TBM_GETPOS, 0, 0);
  settings.mSettings.mCurveLength = SendDlgItemMessage(hwnd, IDC_LENGTH, TBM_GETPOS, 0, 0);
  settings.mSettings.mAngleChangeRate = SendDlgItemMessage(hwnd, IDC_RATE, TBM_GETPOS, 0, 0);
  settings.mSettings.mEvolutionRate = 101 - SendDlgItemMessage(hwnd, IDC_EVOLUTION,
    TBM_GETPOS, 0, 0);

  int modenum = SendDlgItemMessage(hwnd, IDC_MODE, CB_GETCURSEL, 0, 0);
#if DEBUG
  if ((modenum == CB_ERR) || (modenum >= NameListCount))
    Debug(hwnd,"Invalid mode control return.");
#endif
  settings.mSettings.mMode = NameList[modenum].mMode;

}

static void GetAdvSettingsFromDlg(HWND hwnd)
{
  SaverSettingsWin32::Check4Updates = 
    (IsDlgButtonChecked(hwnd, IDC_CHECK_UPDATES) == BST_CHECKED);
	
  SaverSettingsWin32::LevelOfDetail = SendDlgItemMessage(hwnd, IDC_DETAIL, TBM_GETPOS, 0, 0);
}

static void SetDlgFromSettings(HWND hwnd, const SaverSettingsWin32& settings)
{
  ControlsValid = false;  // tell the dialog proc that the controls are in flux
  SendDlgItemMessage(hwnd, IDC_COUNT, TBM_SETPOS,
    (WPARAM)TRUE, (LPARAM) (LONG) settings.mSettings.mCurveCount);
  SendDlgItemMessage(hwnd, IDC_LENGTH, TBM_SETPOS,
    (WPARAM)TRUE, (LPARAM) (LONG) settings.mSettings.mCurveLength);
  SendDlgItemMessage(hwnd, IDC_RATE, TBM_SETPOS,
    (WPARAM)TRUE, (LPARAM) (LONG) settings.mSettings.mAngleChangeRate);
  SendDlgItemMessage(hwnd, IDC_EVOLUTION, TBM_SETPOS,
    (WPARAM)TRUE, (LPARAM) (LONG) (101 - settings.mSettings.mEvolutionRate));
  CheckDlgButton(hwnd, IDC_THICK, settings.mSettings.mThickLines ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hwnd, IDC_FIXED, settings.mSettings.mFixed ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hwnd, IDC_POINTS, settings.mSettings.mPoints ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hwnd, IDC_TRIAXIAL, settings.mSettings.mTriAxial ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hwnd, IDC_COLOR, settings.mSettings.mInColor ? BST_CHECKED : BST_UNCHECKED);

  for (int i = 0; i < NameListCount; i++)
    if (NameList[i].mMode == settings.mSettings.mMode) {
      SendDlgItemMessage(hwnd, IDC_MODE, CB_SETCURSEL, i, 0);
      break;
    }

  CheckDlgButton(hwnd, IDC_RANDOM, SaverSettingsWin32::Randomize ? BST_CHECKED : BST_UNCHECKED);

  PostMessage(hwnd, WM_APP, 0, 0);  // tell the dialog proc that the controls are stable
}

static void SetAdvDlgFromSettings(HWND hwnd)
{
  CheckDlgButton(hwnd, IDC_CHECK_UPDATES, SaverSettingsWin32::Check4Updates ? BST_CHECKED : BST_UNCHECKED);

  SendDlgItemMessage(hwnd, IDC_DETAIL, TBM_SETPOS,
    (WPARAM)TRUE, (LPARAM) (LONG) SaverSettingsWin32::LevelOfDetail);
}

static void UpdateTextureThumbnail(HWND hwnd)
{
  RECT trect;
  
  Thumbnail.reset();

  GetClientRect(GetDlgItem(hwnd, IDC_TEXTURE), &trect);

  if (Configuration.mSettings.usesTexture()) 
    Thumbnail = std::make_unique<Texture>(Configuration.mSettings.getTextureStr(), Texture::WinBitmap, 
      trect.right - trect.left + 1, trect.bottom - trect.top + 1);

  BOOL bres = InvalidateRect(GetDlgItem(hwnd, IDC_TEXTURE), NULL, TRUE);
}

static void ReadPresetsRegistry(HWND presetWin, bool combo)
{
  SaverSettingsWin32::PresetIter presets;

  if (combo)
    SendMessage(presetWin, CB_RESETCONTENT, 0, 0);
  else {
    ListView_DeleteAllItems(presetWin);
    ListView_SetExtendedListViewStyle(presetWin, LVS_EX_CHECKBOXES);
  }

  LVITEM lvI;
  lvI.mask = LVIF_TEXT;
  lvI.iItem = 32767;
  lvI.iSubItem = 0;

  while (presets.nextPreset())
    if (combo)
      SendMessage(presetWin, CB_ADDSTRING, 0, (LPARAM)presets.currentPresetName());
    else {
      lvI.pszText = presets.currentPresetName();
      SaverSettingsWin32 preset;
      if (preset.ReadPreset(lvI.pszText)) {
        int i = ListView_InsertItem(presetWin, &lvI);
      }
    }
}

static void UpdatePresetLists(bool isAdd, char* name) 
{
  if (PresetCombo) {
    if (isAdd) {
      int i = SendMessage(PresetCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)name);
      if (i == CB_ERR)
        SendMessage(PresetCombo, CB_ADDSTRING, 0, (LPARAM)name);
    } else {
      int i = SendMessage(PresetCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)name);
      if (i != CB_ERR)
        SendMessage(PresetCombo, CB_DELETESTRING, i, 0);
      if (SendMessage(PresetCombo, CB_GETCOUNT, 0, 0) == 0)
        ReadPresetsRegistry(PresetCombo, true);
    }
  }

  if (PresetList) {
    if (isAdd) {
      LVFINDINFO lvFI;
      lvFI.flags = LVFI_STRING;
      lvFI.psz = name;
      LVITEM lvI;
      lvI.mask = LVIF_TEXT;
      lvI.iItem = 32767;
      lvI.iSubItem = 0;
      lvI.pszText = name;
      SaverSettingsWin32 preset;
      if (preset.ReadPreset(lvI.pszText) && ListView_FindItem(PresetList, -1, &lvFI) == -1) {
        int i = ListView_InsertItem(PresetList, &lvI);
      }
    } else {
      LVFINDINFO lvFI;
      lvFI.flags = LVFI_STRING;
      lvFI.psz = name;
      int i = ListView_FindItem(PresetList, -1, &lvFI);
      if (i != -1)
        ListView_DeleteItem(PresetList, i);
      if (ListView_GetItemCount(PresetList) == 0)
        ReadPresetsRegistry(PresetList, false);
    }
  }
}

static void SetPresetList(HWND presetWin, const char* presetName)
{
  SendMessage(presetWin, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) presetName);
}

static void InitPresetList(HWND presetWin, const SaverSettingsWin32& current)
{
  SaverSettingsWin32::PresetIter presets;

  while (presets.nextPreset())
    if (current.ComparePreset(presets.currentPresetName())) {
      SetPresetList(presetWin, presets.currentPresetName());
      return;
    }
}

static char* GetPresetText(HWND presetWin)
{
  static char* buffer;
  static int length = 0;

  if (!length) {
    length = 2;
    buffer = new char[length];
  }

  int currentLength = SendMessage(presetWin, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);
  if (currentLength >= length) {
    delete buffer;
    length = currentLength + 1;
    buffer = new char[length];
  }

  SendMessage(presetWin, WM_GETTEXT, (WPARAM)length, (LPARAM)buffer);

  return buffer;
}


static void ChopTrailingSpace(char* buf)
{
  for(int i = strlen(buf); isspace(buf[--i]); )
    buf[i] = '\0';				// chop off trailing space
}


static void MaybeEnableSave(HWND hwnd)
{
  char *buf = GetPresetText(GetDlgItem(hwnd, IDC_PRESETS));
  EnableWindow(GetDlgItem(hwnd, IDC_PRESETS), !SaverSettingsWin32::Randomize);
  if ((strlen(buf) == 0) || SaverSettingsWin32::Randomize) {
    EnableWindow(GetDlgItem(hwnd, IDC_DELETE), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_SAVE), FALSE);
  } else {
    ChopTrailingSpace(buf);

    SaverSettingsWin32 temp;
    if (temp.ReadPreset(buf)) {
      EnableWindow(GetDlgItem(hwnd, IDC_DELETE), TRUE);
      if (temp == Configuration)
        EnableWindow(GetDlgItem(hwnd, IDC_SAVE), FALSE);
      else
        EnableWindow(GetDlgItem(hwnd, IDC_SAVE), TRUE);
    } else {
      EnableWindow(GetDlgItem(hwnd, IDC_DELETE), FALSE);
      EnableWindow(GetDlgItem(hwnd, IDC_SAVE), TRUE);
    }
  }
  bool enableFixed = Configuration.mSettings.usesFixed() && !SaverSettingsWin32::Randomize;
  bool enableThick = Configuration.mSettings.usesThickness() && !SaverSettingsWin32::Randomize;
  bool enableLength = Configuration.mSettings.usesLength() && !SaverSettingsWin32::Randomize;
  bool enableTexture = Configuration.mSettings.usesTexture() && !SaverSettingsWin32::Randomize;
  bool enableTriAxial = Configuration.mSettings.usesTriAxial() && !SaverSettingsWin32::Randomize;
  EnableWindow(GetDlgItem(hwnd, IDC_THICK), enableThick);
  EnableWindow(GetDlgItem(hwnd, IDC_FIXED), enableFixed);
  EnableWindow(GetDlgItem(hwnd, IDC_POINTS), !SaverSettingsWin32::Randomize);
  EnableWindow(GetDlgItem(hwnd, IDC_TRIAXIAL), enableTriAxial);
  EnableWindow(GetDlgItem(hwnd, IDC_LENGTH), enableLength);
  EnableWindow(GetDlgItem(hwnd, IDT_LENGTH), enableLength);
  EnableWindow(GetDlgItem(hwnd, IDC_TEXTURE_BROWSE), enableTexture);
  EnableWindow(GetDlgItem(hwnd, IDC_TEXTURE_CLEAR), enableTexture);
  EnableWindow(GetDlgItem(hwnd, IDC_TEXTURE_UD), enableTexture);
  EnableWindow(GetDlgItem(hwnd, IDC_COUNT), !SaverSettingsWin32::Randomize);
  EnableWindow(GetDlgItem(hwnd, IDT_COUNT), !SaverSettingsWin32::Randomize);
  EnableWindow(GetDlgItem(hwnd, IDC_EVOLUTION), !SaverSettingsWin32::Randomize);
  EnableWindow(GetDlgItem(hwnd, IDT_EVOLUTION), !SaverSettingsWin32::Randomize);
  EnableWindow(GetDlgItem(hwnd, IDC_RATE), !SaverSettingsWin32::Randomize);
  EnableWindow(GetDlgItem(hwnd, IDT_RATE), !SaverSettingsWin32::Randomize);
  EnableWindow(GetDlgItem(hwnd, IDC_COLOR), !SaverSettingsWin32::Randomize);
  EnableWindow(GetDlgItem(hwnd, ID_BOX1), !SaverSettingsWin32::Randomize);
  EnableWindow(GetDlgItem(hwnd, IDT_TEXTURE), enableTexture);
  EnableWindow(GetDlgItem(hwnd, IDC_MODE), !SaverSettingsWin32::Randomize);
  
  static bool lastEnableTexture = true;
  if (lastEnableTexture != enableTexture) {
    UpdateTextureThumbnail(hwnd);
    lastEnableTexture = enableTexture;
  }
}

static void DeleteChar(char *presetName, DWORD loc, DWORD& start, DWORD& end)
{
  for (DWORD i = loc; presetName[i]; i++)
    presetName[i] = presetName[i + 1];

  if (loc < start) start--;
  if (loc < end) end--;
}

static bool CleanPresetName(char *presetName, DWORD& start, DWORD& end)
{
  bool returnVal = false;
  int i;

  while (isspace(presetName[0])) {			// remove leading spaces
    DeleteChar(presetName, 0, start, end);
    returnVal = true;
  }

  for (i = 0; presetName[i];)					// remove illegal chars
    if ((presetName[i] == '\\' || !isgraph(presetName[i])) &&
      presetName[i] != ' ') {
      DeleteChar(presetName, i, start, end);
      returnVal = true;
    } else {
      i++;
    }

  return returnVal;
}

static void UpdateTextureEnable()
{
  if (Configuration.mSettings.getTextureStrlen() == 0) return;    // no texture
  if (*(Configuration.mSettings.getTextureStr()) == '#') return;  // embedded texture
  HANDLE textureFile = CreateFile(Configuration.mSettings.getTextureStr(), GENERIC_READ, 0,
  NULL, OPEN_EXISTING, 0, NULL);
  if (textureFile == INVALID_HANDLE_VALUE) {
    Configuration.mSettings.clearTexture();
  } else {
    CloseHandle(textureFile);
  }
}

static HWND InitToolTip(HWND hwnd, int control, const char* tip, bool getEditBox)
{
  HWND hwndTT;                 // handle to the tooltip control
  // struct specifying info about tool in tooltip control
  static TOOLINFO ti;
  WPARAM uid;        // for ti initialization
  static char strTT[81];
  LPTSTR lptstr = strTT;
  strncpy(strTT, tip, 80);
  strTT[80] = '\0';

  if (getEditBox) {
    COMBOBOXINFO cbi;
    cbi.cbSize = sizeof(COMBOBOXINFO);
    BOOL res = GetComboBoxInfo(GetDlgItem(hwnd, control), &cbi);
    uid = (WPARAM)cbi.hwndItem;
  } else {
    uid = (WPARAM)GetDlgItem(hwnd, control);
  }

  // CREATE A TOOLTIP WINDOW
  hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
    WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd,
    NULL, hInst, NULL);

  SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0,	0,
    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

  // INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE
  ti.cbSize = sizeof(TOOLINFO);
  ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
  ti.hwnd = hwnd;
  ti.hinst = hInst;
  ti.uId = uid;
  ti.lpszText = lptstr;

  /* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
  SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
  return hwndTT;
}

static void InitTextureUD(HWND hwnd);

static void InitCommonDialog(HWND hwnd, bool combo) 
{
  HWND ctrl = GetDlgItem(hwnd, IDC_PRESETS);
  SendMessage(ctrl, CB_LIMITTEXT, (WPARAM)256, (LPARAM)0);
  ReadPresetsRegistry(ctrl, combo);
  if (combo)
    PresetCombo = ctrl;
  else
    PresetList = ctrl;
}

static void InitDialog(HWND hwnd, LPARAM lParam)
{
  Configuration.ReadCurrentSettings(false);
  SaverSettingsWin32::ConfigWindow = hwnd;

  hInst = (HINSTANCE)lParam;

  RECT pvrc; 
  HWND ctrl = GetDlgItem(hwnd, IDC_PREVIEW);
  GetClientRect(ctrl, &pvrc);
  PreviewWin = new SaverWin(ctrl, SaverWin::Preview, Configuration, pvrc, hInst);
  
  InitCommonDialog(hwnd, true);

  ctrl = GetDlgItem(hwnd, IDC_PRESETS);
  InitPresetList(ctrl, Configuration);

  SendDlgItemMessage(hwnd, IDOK, BM_SETSTYLE,
    (WPARAM) BS_DEFPUSHBUTTON, (LPARAM) TRUE);

  for (int i = 0; i < NameListCount; i++)
    SendDlgItemMessage(hwnd, IDC_MODE, CB_ADDSTRING, 0, (LPARAM)NameList[i].mName);

  SendDlgItemMessage(hwnd, IDC_LENGTH, TBM_SETRANGE,
    (WPARAM)FALSE, (LPARAM) MAKELONG((short)3, (short)SaverSettings::MaxCurveLength));

  SendDlgItemMessage(hwnd, IDC_RATE, TBM_SETRANGE,
    (WPARAM)FALSE, (LPARAM) MAKELONG((short)1, (short)100));

  SendDlgItemMessage(hwnd, IDC_EVOLUTION, TBM_SETRANGE,
    (WPARAM)FALSE, (LPARAM) MAKELONG((short)1, (short)100));

  SendDlgItemMessage(hwnd, IDC_COUNT, TBM_SETRANGE,
    (WPARAM)FALSE, (LPARAM) MAKELONG((short)1, (short)SaverSettings::MaxCurveCount));

  char buf[100];
  wsprintf(buf, "Spirex, version %s", VersionString);
  SendDlgItemMessage(hwnd, IDT_VERSION, WM_SETTEXT, (WPARAM) 0, (LPARAM)buf);

  SendDlgItemMessage(hwnd, IDC_TEXTURE_UD, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwnd, IDT_TEXTURE), 0);

  UpdateTextureEnable();
  InitTextureUD(hwnd);

  InitToolTip(hwnd, IDC_TEXTURE_CLEAR, "Stop using a texture; Reset browse directory", false);
  InitToolTip(hwnd, IDC_TEXTURE_BROWSE, "Enable textures and choose a texture", false);
  InitToolTip(hwnd, IDC_TEXTURE_UD, "Pick another texture in the same directory as the current texture", false);
  InitToolTip(hwnd, IDC_RANDOM, "Cycle through presets at random", false);
  InitToolTip(hwnd, IDC_PREVIEW_BUTTON, "View settings in full-screen mode.", false);
  InitToolTip(hwnd, IDC_PRESETS, "The list of saved presets. Type in a new preset name", true);
  InitToolTip(hwnd, IDC_PRESETS, "Choose from the saved presets", false);
  InitToolTip(hwnd, IDC_SAVE, "Save a new preset or modify the current preset", false);
  InitToolTip(hwnd, IDC_DELETE, "Delete the current preset (Delete all to reset to Spirex defaults)", false);
  InitToolTip(hwnd, IDC_FIXED, "Use fixed-size 3D objects moving on a more complex path", false);
  InitToolTip(hwnd, IDC_COLOR, "Vary the color of curves and 3D objects (including textures)", false);
  InitToolTip(hwnd, IDC_THICK, "Use thicker 2D and 3D curves; Use larger points and fixed-size 3D objects", false);
  InitToolTip(hwnd, IDC_POINTS, "Replace curves/surfaces with points", false);
  InitToolTip(hwnd, IDC_TRIAXIAL, "Vary sphere/cube size on all three axes, disks become rings", false);
  InitToolTip(hwnd, IDC_LENGTH, "Length of 2D and 3D curves", false);
  InitToolTip(hwnd, IDC_COUNT, "Number of curves or 3D objects", false);
  InitToolTip(hwnd, IDC_EVOLUTION, "Maximum rate at which the geometry changes", false);
  InitToolTip(hwnd, IDC_RATE, "Maximum speed of curves and 3D objects", false);

  SetDlgFromSettings(hwnd, Configuration);
  PostMessage(hwnd, WM_APP, 0, 0);
  UpdateTextureThumbnail(hwnd);

  // Initialize OPENFILENAME
  ZeroMemory(&Ofn, sizeof(OPENFILENAME));
  Ofn.lStructSize = sizeof(OPENFILENAME);
  Ofn.hwndOwner = hwnd;
  Ofn.lpstrFile = szFile;
  Ofn.nMaxFile = MAX_PATH;
  Ofn.lpstrFilter = "Textures (GIF, TIFF, BMP, PNG, and JPEG files)\0*.BMP;*.JPG;*.JPEG;*.TIFF;*.TIF;*.GIF;*.PNG\0";
  Ofn.nFilterIndex = 1;
  Ofn.lpstrFileTitle = szFileName;
  Ofn.nMaxFileTitle = MAX_PATH;
  Ofn.lpstrInitialDir = InitDir;
  Ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
  
  PWSTR path = 0;
  if (SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &path) == S_OK ||
      SHGetKnownFolderPath(FOLDERID_PublicDocuments, 0, NULL, &path) == S_OK)
  {
      size_t count;
      if (wcstombs_s(&count, InitDir, MAX_PATH, path, MAX_PATH) != 0)
          InitDir[0] = '\0';
      CoTaskMemFree(static_cast<void*>(path));
  } else {
    InitDir[0] = '\0';
  }
}

static std::string TextureDir;
static std::vector<std::string> TextureList;

static void InitTextureUD(HWND hwnd)
{
  // Free the previous texture list
    TextureList.clear();
    TextureDir.clear();
    std::string name;

  // determine if texture is embedded or a file
  if ((Configuration.mSettings.getTextureStrlen() == 0) ||
      (*(Configuration.mSettings.getTextureStr()) == '#'))
  {
      TextureList.reserve(8);
      TextureList.assign({ IDT_SPIREX , IDT_ORIGAMI , IDT_MOON , IDT_MARS , IDT_FLOWER, IDT_EUROPA, IDT_EARTH, IDT_BORG});
      name.assign(Configuration.mSettings.getTextureStr());
  } else {
    // get the texture directory and create a handle to search it
    TextureDir.assign(Configuration.mSettings.getTextureStr());
    size_t dirPtr = TextureDir.rfind('\\');
    if (dirPtr == std::string::npos)
      return;

    name = TextureDir.substr(dirPtr + 1);
    TextureDir.resize(dirPtr + 1);
    TextureDir.append(1, '*');

    WIN32_FIND_DATA w32fd;
    HANDLE FindFile = FindFirstFile(TextureDir.c_str(), &w32fd);
    if (FindFile == INVALID_HANDLE_VALUE) {
      DebugWin();
      return;
    }
    TextureDir.resize(dirPtr + 1);

    do {
      // skip directories and hidden/system files
      if ((w32fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) ||
        (w32fd.dwFileAttributes == FILE_ATTRIBUTE_HIDDEN) ||
        (w32fd.dwFileAttributes == FILE_ATTRIBUTE_SYSTEM))
        continue;


      // get the extension
      char* extPtr = strrchr(w32fd.cFileName, '.');
      if (extPtr == 0)
        continue;	// skip if no extension
      else
        extPtr++;

      // skip non-image files
      if (lstrcmpi(extPtr, "gif") &&
          lstrcmpi(extPtr, "bmp") &&
          lstrcmpi(extPtr, "rgb") &&
          lstrcmpi(extPtr, "rgba") &&
          lstrcmpi(extPtr, "jpg") &&
          lstrcmpi(extPtr, "jpeg") &&
          lstrcmpi(extPtr, "tif") &&
          lstrcmpi(extPtr, "tiff") &&
          lstrcmpi(extPtr, "png") &&
          lstrcmpi(extPtr, "tga") &&
          lstrcmpi(extPtr, "cel"))
      continue;

      // Create new texture item
      TextureList.emplace_back(w32fd.cFileName);
    } while (FindNextFile(FindFile, &w32fd) || (GetLastError() != ERROR_NO_MORE_FILES));
    std::sort(TextureList.begin(), TextureList.end());
  }


  // find where the current texture is in the list
  auto whichTexture = std::find(TextureList.begin(), TextureList.end(), name);

  if (whichTexture == TextureList.end()) {
      Debug("Couldn't find current texture in list.");
      whichTexture = TextureList.begin();
  }

  // initialize the Up-Down control for the current texture directory
  SendDlgItemMessage(hwnd, IDC_TEXTURE_UD, UDM_SETRANGE, 0, (LPARAM)MAKELONG((short)TextureList.size(), (short)1));
  SendDlgItemMessage(hwnd, IDC_TEXTURE_UD, UDM_SETPOS, 0, (LPARAM)MAKELONG((short)((whichTexture - TextureList.begin()) + 1), 0));
}

static void GetNextTexture(HWND hwnd, size_t pos)
{
    std::string textureName = TextureDir;
    if (pos > 0 && pos <= TextureList.size()) {
        textureName.append(TextureList[pos - 1]);
        if (lstrcmpi(textureName.c_str(), Configuration.mSettings.getTextureStr())) {
            Configuration.mSettings.setTexture(textureName.c_str());
            PreviewWin->NewSaverSettings(Configuration);
            UpdateTextureThumbnail(hwnd);
        }
    }
}


static void CleanUp()
{
}

static BOOL HandlePresetCommands(HWND hwnd, int code, LPARAM lParam)
{
  switch (code) {
#if DEBUG
    case CBN_SETFOCUS:
    case CBN_KILLFOCUS:
      return FALSE;
      break;
#endif
    case CBN_SELENDOK: {
      int sel = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
      if (sel < 0) break;
      int length = SendMessage((HWND)lParam, CB_GETLBTEXTLEN, (WPARAM) sel, (LPARAM) 0);
      if (length == CB_ERR) break;
      char* buf = new char[length + 1];
      SendMessage((HWND)lParam, CB_GETLBTEXT, (WPARAM) sel, (LPARAM) buf);
      Debug(buf);
      Configuration.ReadPreset(buf);
      UpdateTextureEnable();
      SetDlgFromSettings(hwnd, Configuration);
      UpdateTextureThumbnail(hwnd);
      InitTextureUD(hwnd);
      PreviewWin->NewSaverSettings(Configuration);
      delete buf;
      break;
    }
    case CBN_EDITCHANGE: {
      Debug ("WM_COMMAND CBN_EDITCHANGE ");
      char *buf = GetPresetText((HWND)lParam);
      DWORD start, end;
      SendMessage((HWND)lParam, CB_GETEDITSEL, (WPARAM)&start, (LPARAM)&end);
      if (CleanPresetName(buf, start, end)) {
        SendMessage((HWND)lParam, WM_SETTEXT, 0, (LPARAM)buf);
        if (!strlen(buf)) {
          SendMessage((HWND)lParam, CB_SETCURSEL, (WPARAM) -1, 0);
          break;
        }
        SendMessage((HWND)lParam, CB_SETEDITSEL, 0, MAKELPARAM(start, end));
      }
      break;
    }
    default: {
#if DEBUG
      char buf[100];
      wsprintf(buf, "WM_COMMAND to IDC_PRESETS code=%d", code);
#endif //DEBUG
      return FALSE;
    }
  }

  return TRUE;
}

static void DoPreview()
{
  PreviewWin->StopAnimation();
  Configuration.WriteCurrentSettings(true);
  SaverSettingsWin32::WriteGlobalSettings(true);
  char cmdbuf[MAX_PATH + 20];
  if (!GetModuleFileName(NULL, cmdbuf, MAX_PATH)) {
  	DebugWin();
  	return;
  }
  lstrcat(cmdbuf, " -v");
  STARTUPINFO sui;
  PROCESS_INFORMATION pi;
  ZeroMemory(&sui, sizeof(sui));
  sui.cb = sizeof(sui);
  ZeroMemory(&pi, sizeof(pi));
  if (!CreateProcess(NULL, cmdbuf, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &sui, &pi)) {
    DebugWin();
  } else {
		WaitForSingleObject(pi.hProcess, INFINITE);
	}
  PreviewWin->StartAnimation();
}

static BOOL HandleCommands(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  int id = LOWORD(wParam);
  int code = HIWORD(wParam);
#if DEBUG
  char buf[100];
  wsprintf(buf, "WM_COMMAND with id = %d and code = %d", id, code);
  //Debug(buf);
#endif
  switch (id) {
    case IDC_SAVE: {
    	if (code != BN_CLICKED)
    		return 1;
      HWND presetWin = GetDlgItem(hwnd, IDC_PRESETS);
      char *buf = GetPresetText(presetWin);
      if (strlen(buf) == 0) break;
      Configuration.WritePreset(buf);
      UpdatePresetLists(true, buf);
      //ReadPresetsRegistry(presetWin);
      SetPresetList(presetWin, buf);
      break;
    }
    case IDC_DELETE: {
    	if (code != BN_CLICKED)
    		return 1;
      HWND presetWin = GetDlgItem(hwnd, IDC_PRESETS);
      char *buf = GetPresetText(presetWin);
      ChopTrailingSpace(buf);
      int sel;
      if (strlen(buf) == 0 || (sel = SendMessage(presetWin, CB_FINDSTRINGEXACT, 
        (WPARAM) -1, (LPARAM)buf)) == CB_ERR)
        break;
      //SendMessage(presetWin, CB_GETLBTEXT, (WPARAM) sel, (LPARAM) buf);
      SaverSettingsWin32::DeletePreset(buf);
      UpdatePresetLists(false, buf);
      //ReadPresetsRegistry(presetWin);
      break;
    }
    case IDC_TEXTURE_BROWSE:
    	if (code != BN_CLICKED)
    		return 1;
      strcpy(szFile, Configuration.mSettings.getTextureStr());
      if (szFile[0] == '#')
      	szFile[0] = '\0';
      //PreviewWin->StopAnimation();
      if (GetOpenFileName(&Ofn)) {
        Configuration.mSettings.setTexture(szFile);
        PreviewWin->NewSaverSettings(Configuration);
        InitTextureUD(hwnd);
      }
      UpdateTextureThumbnail(hwnd);
      Ofn.lpstrInitialDir = NULL;
      break;
    case IDC_TEXTURE_CLEAR:
    	if (code != BN_CLICKED)
    		return 1;
      Configuration.mSettings.clearTexture();
      InitTextureUD(hwnd);
      UpdateTextureThumbnail(hwnd);
      PreviewWin->NewSaverSettings(Configuration);
      //SetDlgFromSettings(hwnd, Configuration);
      Ofn.lpstrInitialDir = NULL; //SaverSettingsWin32::TextureRoot;
      break;
    case IDC_THICK:
    case IDC_FIXED:
    case IDC_COLOR:
    case IDC_POINTS:
    case IDC_TRIAXIAL:
    case IDC_RANDOM:
      if (ControlsValid && (code == BN_CLICKED)) {
        GetSettingsFromDlg(hwnd, Configuration);
        PreviewWin->NewSaverSettings(Configuration);
        if (id == IDC_MODE)
          UpdateTextureThumbnail(hwnd);
      } else {
        return FALSE;
      }
      break;
    case IDC_MODE:
      if (ControlsValid && (code == CBN_SELENDOK)) {
        GetSettingsFromDlg(hwnd, Configuration);
        PreviewWin->NewSaverSettings(Configuration);
      } else
        return FALSE;
      break;
    case IDC_PRESETS:
      return ControlsValid && HandlePresetCommands(hwnd, code, lParam);
    default:
      Debug("WM_COMMAND sent from some control");
      return FALSE;
  }

  return TRUE;
}

static BOOL CALLBACK ConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  //Debug(hwnd, MessageName(msg));
  switch (msg) {
    case WM_INITDIALOG:
      InitDialog(hwnd, lParam);
      ShowWindow(hwnd, SW_SHOWNORMAL);
      return TRUE;
    case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE) {
        PreviewWin->StartAnimation();
        //InitTextureUD(hwnd);
      }
      return TRUE;
    case WM_APP:
      if (!ControlsValid) {
        Debug("Setting dialog state to ControlsValid.");
      }
      ControlsValid = true;
      MaybeEnableSave(hwnd);
      return TRUE;
    case WM_COMMAND:
      if (!HandleCommands(hwnd, wParam, lParam)) return FALSE;
      break;
    case WM_DRAWITEM: {
      //Debug(hwnd, MessageName(msg));
      DRAWITEMSTRUCT* di = (DRAWITEMSTRUCT*)lParam;
      switch (wParam) {
        case IDC_PREVIEW:
          if (PreviewWin)
            PreviewWin->Clear();
          break;
        case IDC_TEXTURE: 
          if (Thumbnail) {
            Graphics g(di->hDC);
            Bitmap* bm = Thumbnail->GetBM();
            if (bm)
              g.DrawImage(bm, 0, 0, bm->GetWidth(), bm->GetHeight());
            else
              g.Clear(Color(0xfff4f3ee));
          }
          break;
        default:
          break;
      }
      return TRUE;
    }
    case WM_NOTIFY: {
      NMHDR *note = (NMHDR*)lParam;
      PSHNOTIFY *PSHnote;
      switch (note->code) {
        case PSN_KILLACTIVE:
        	Debug(hwnd, "PSN_KILLACTIVE");
          PSHnote = (PSHNOTIFY*)lParam;
          SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
          return FALSE;
        case PSN_SETACTIVE:
        	Debug(hwnd, "PSN_SETACTIVE");
          if (PreviewWin) {
            HWND ctrl = GetDlgItem(hwnd, IDC_PREVIEW);
            SetParent(PreviewWin->mHwnd, ctrl);
            PreviewWin->Clear();
          }
          PSHnote = (PSHNOTIFY*)lParam;
          return 0;
        case PSN_APPLY: 
        	Debug(hwnd, "PSN_APPLY");
          PSHnote = (PSHNOTIFY*)lParam;
          Configuration.WriteCurrentSettings(false);
          SaverSettingsWin32::WriteGlobalSettings(false);
          SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
          return TRUE;
        case PSN_RESET:
            Thumbnail.release();    // let the last one leak
        	Debug(hwnd, "PSN_RESET");
          break;
      }
      break;
    }
    case WM_HSCROLL:
    case WM_VSCROLL:
      if (ControlsValid && LOWORD(wParam) != SB_THUMBTRACK) {
        if ((HWND)lParam == GetDlgItem(hwnd, IDC_TEXTURE_UD)) {
          if (LOWORD(wParam) == SB_THUMBPOSITION) {
            size_t pos = static_cast<size_t>(HIWORD(wParam));
            GetNextTexture(hwnd, pos);
            return 0;
          }
          return TRUE;
        }
        GetSettingsFromDlg(hwnd, Configuration);
        PreviewWin->NewSaverSettings(Configuration);
      } else {
        return TRUE;	// controls not valid or SB_THUMBTRACK
      }
      break;
    default:
      return FALSE;
  }
  if (ControlsValid) MaybeEnableSave(hwnd);
    return TRUE;
}


static BOOL CALLBACK AdvConfigDialogProc(HWND hwnd, UINT msg,
		WPARAM wParam, LPARAM lParam)
{
 	static HICON yes[4] = {0, 0, 0, 0};
 	static HICON no[4] = {0, 0, 0, 0};
	switch (msg) {
    case WM_INITDIALOG: {
			for (int i = 0; i < 4; i++) {
				yes[i] = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_TL_YES + i), IMAGE_ICON, 8, 8, LR_DEFAULTCOLOR);
				if (!yes[i])
					DebugWin();
				no[i] = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_TL_NO + i), IMAGE_ICON, 8, 8, LR_DEFAULTCOLOR);
				if (!no[i])
					DebugWin();
     		SendDlgItemMessage(hwnd, IDC_TL_CORNER + i, BM_SETIMAGE, IMAGE_ICON, 
     			(LPARAM)((SaverSettingsWin32::DisableHotCorner == i) ? yes[i] : no[i]));
			}
      SendDlgItemMessage(hwnd, IDC_DETAIL, TBM_SETRANGE,
        (WPARAM)FALSE, (LPARAM) MAKELONG((short)50, (short)200));

      char buf[100];
      wsprintf(buf, "Spirex, version %s", VersionString);
      SendDlgItemMessage(hwnd, IDT_VERSION, WM_SETTEXT, (WPARAM) 0, (LPARAM)buf);

      SetAdvDlgFromSettings(hwnd);

      return TRUE;
    }
    case WM_COMMAND: {
    	int code = HIWORD(wParam);
    	int item = LOWORD(wParam);
      switch (item) {
        case IDC_GOHOME: 
        	if (code != BN_CLICKED)
        		return 1;
					ShellExecute(NULL, "open", URLString,
						NULL, NULL, SW_SHOWNORMAL);
				  PostMessage(Cancelbutton, BM_CLICK, 0, 0);
          return 0;
        case IDC_CHECK_UPDATES:
        	if (code != BN_CLICKED)
        		return 1;
          GetAdvSettingsFromDlg(hwnd);
          PreviewWin->NewSaverSettings(Configuration);
          return 0;
        case IDC_RES_BATTERY:
        case IDC_RES_ALWAYS:
        case IDC_RES_MULTI:
        	if (code != CBN_SELENDOK)
        		return 1;
          GetAdvSettingsFromDlg(hwnd);
          PreviewWin->NewSaverSettings(Configuration);
          return 0;
        case IDC_TL_CORNER:
        case IDC_TR_CORNER:
        case IDC_BL_CORNER:
        case IDC_BR_CORNER: {
        	if (code != BN_CLICKED)
        		return 1;
        	int button = item - IDC_TL_CORNER;
        	if (SaverSettingsWin32::DisableHotCorner == button) {
        		SendDlgItemMessage(hwnd, item, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(no[button]));
        		SaverSettingsWin32::DisableHotCorner = -1;
        	} else {
        		if (SaverSettingsWin32::DisableHotCorner >= 0)
	        		SendDlgItemMessage(hwnd, IDC_TL_CORNER + SaverSettingsWin32::DisableHotCorner, 
	        			BM_SETIMAGE, IMAGE_ICON, (LPARAM)(no[SaverSettingsWin32::DisableHotCorner]));
        		SendDlgItemMessage(hwnd, item, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(yes[button]));
        		SaverSettingsWin32::DisableHotCorner = button;
        	}
        	return 0;
        }
      }
      return 1;
    }
    case WM_DRAWITEM:
      //Debug(hwnd, MessageName(msg));
      if (PreviewWin)
        PreviewWin->Clear();
      return TRUE;
    case WM_HSCROLL:
    case WM_VSCROLL:
      if (LOWORD(wParam) != SB_THUMBTRACK) {
        GetAdvSettingsFromDlg(hwnd);
        PreviewWin->NewSaverSettings(Configuration);
      } else {
        return TRUE;	// SB_THUMBTRACK
      }
      break;
    case WM_NOTIFY: {
      NMHDR *note = (NMHDR*)lParam;
      PSHNOTIFY *PSHnote;
      switch (note->code) {
        case PSN_KILLACTIVE:
          PSHnote = (PSHNOTIFY*)lParam;
          SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
          return FALSE;
        case PSN_SETACTIVE: {
          PSHnote = (PSHNOTIFY*)lParam;
          HWND ctrl = GetDlgItem(hwnd, IDC_PREVIEW);
          SetParent(PreviewWin->mHwnd, ctrl);
          PreviewWin->Clear();
          return 0;
        }
        case PSN_APPLY: 
          PSHnote = (PSHNOTIFY*)lParam;
          SaverSettingsWin32::WriteGlobalSettings(false);
          SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
          return TRUE;
        case PSN_RESET:
          SetWindowLong(PropSheetHwnd, GWL_WNDPROC, (LONG) OrigSheetProc);
          Thumbnail.release();      // let the last one leak
          break;
      }
      break;
    }
	}
	return FALSE;
}

static bool OldRandom;
static SaverSettingsWin32 OldConfig;

static BOOL CALLBACK SubsetDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  //Debug(hwnd, MessageName(msg));
  switch (msg) {
    case WM_INITDIALOG:
      InitCommonDialog(hwnd, false);

      char buf[100];
      wsprintf(buf, "Spirex, version %s", VersionString);
      SendDlgItemMessage(hwnd, IDT_VERSION, WM_SETTEXT, (WPARAM) 0, (LPARAM)buf);
    
      InitToolTip(hwnd, IDC_PRESETS, "Click on a preset to see what it looks like in 2D", false);

      ShowWindow(hwnd, SW_SHOWNORMAL);
      return TRUE;
    case WM_DRAWITEM:
      //Debug(hwnd, MessageName(msg));
      if (PreviewWin)
        PreviewWin->Clear();
      return TRUE;
    case WM_NOTIFY: {
      if (wParam == IDC_PRESETS){
        if(((LPNMHDR)lParam)->code == LVN_ITEMCHANGED) {
          HWND presetWin = GetDlgItem(hwnd, IDC_PRESETS);
          int i = ListView_GetNextItem(presetWin, -1, LVNI_FOCUSED);
          if (i == -1)
            return FALSE;
          LVITEM lvI;
          lvI.iItem = i; lvI.iSubItem = 0;
          char buf[257];
          lvI.pszText = buf; lvI.cchTextMax = 256;
          lvI.mask = LVIF_TEXT;
          ListView_GetItem(presetWin, &lvI);
          if (Configuration.ReadPreset(buf))
            PreviewWin->NewSaverSettings(Configuration);
          return TRUE;
        }
        return FALSE;
      }
      NMHDR *note = (NMHDR*)lParam;
      PSHNOTIFY *PSHnote;
      switch (note->code) {
        case PSN_KILLACTIVE:
          SaverSettingsWin32::Randomize = OldRandom;
          Configuration = OldConfig;
          PreviewWin->NewSaverSettings(Configuration);
          PSHnote = (PSHNOTIFY*)lParam;
          SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
          return FALSE;
        case PSN_SETACTIVE: {
          PSHnote = (PSHNOTIFY*)lParam;
          HWND ctrl = GetDlgItem(hwnd, IDC_PREVIEW);
          SetParent(PreviewWin->mHwnd, ctrl);
          OldRandom = SaverSettingsWin32::Randomize;
          OldConfig = Configuration;
          SaverSettingsWin32::Randomize = false;
          PreviewWin->NewSaverSettings(Configuration);
          PreviewWin->Clear();
          return 0;
        }
        case PSN_APPLY: {
          LVITEM lvI;
          char buf[257];
          SaverSettingsWin32 preset;
          lvI.mask = LVIF_TEXT;
          lvI.iSubItem = 0;
          lvI.pszText = buf;
          lvI.cchTextMax = 256;
          int max = ListView_GetItemCount(PresetList);
          for (int i = 0; i < max; i++) {
            lvI.iItem = i;
            if (ListView_GetItem(PresetList, &lvI) && preset.ReadPreset(buf)) {
              preset.WritePreset(buf);
            }
          }
          PSHnote = (PSHNOTIFY*)lParam;
          SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
          return TRUE;
        }
        case PSN_RESET:
          SetWindowLong(PropSheetHwnd, GWL_WNDPROC, (LONG) OrigSheetProc);
          break;
      }
      break;
    }
	}
	return FALSE;
}


static void SetupPreview()
{
  // get the handles and the geometry info for the OK and CANCEL buttons
  HWND OKbutton = GetDlgItem(PropSheetHwnd, IDOK);
  Cancelbutton = GetDlgItem(PropSheetHwnd, IDCANCEL);
  WINDOWINFO CancelWI;
  GetWindowInfo(Cancelbutton, &CancelWI);
  RECT OkRW, CancelRW;
  GetWindowRect(OKbutton, &OkRW);
  GetWindowRect(Cancelbutton, &CancelRW);
  int width = OkRW.right - OkRW.left + 1;
  int height = OkRW.bottom - OkRW.top + 1;
  int spacing = CancelRW.left - OkRW.left + 1;
  ScreenToClient(PropSheetHwnd, (POINT*)(&OkRW));
  ScreenToClient(PropSheetHwnd, (POINT*)(&CancelRW));
  
  // Move them to make room for the new Preview button
  MoveWindow(OKbutton, OkRW.left - spacing, OkRW.top, width, height, true);
  MoveWindow(Cancelbutton, CancelRW.left - spacing, CancelRW.top, width, height, true);
  
  // Insert the new Preview button
  PreviewButton = CreateWindowEx(CancelWI.dwExStyle, "Button", "Preview", CancelWI.dwStyle , 
    CancelRW.left - 1, CancelRW.top, width, height, PropSheetHwnd, (HMENU)IDC_PREVIEW_BUTTON, hInst, NULL);
    
  // Move to just after the CANCEL button in the tab order
  SetWindowPos(PreviewButton, Cancelbutton, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOSENDCHANGING);
  
  // Give it the same dialog font as the other buttons
  HFONT dlgfont = (HFONT)SendMessage(OKbutton, WM_GETFONT, 0, 0);
  SendMessage(PreviewButton, WM_SETFONT, (WPARAM)dlgfont, TRUE);
}

static BOOL CALLBACK SheetSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  //Debug(hwnd, MessageName(msg));
  // Insert the Preview button after the property sheet has finished initializing
  // We can't use WM_INITDIALOG because the subclassing happens after message
  // was sent. 
  if (msg == WM_APP) {
    BOOL res = CallWindowProc(OrigSheetProc, hwnd, msg, wParam, lParam);
    SetupPreview();
    return res;
  }
  
  // Process Preview button clicks
  if ((msg == WM_COMMAND) && (HIWORD(wParam) == BN_CLICKED) && 
  		((HWND)lParam == PreviewButton)) {
    DoPreview();
    return 0;
  }
  
  return CallWindowProc(OrigSheetProc, hwnd, msg, wParam, lParam); 
}

static int CALLBACK RemoveContextHelpProc(HWND hwnd, UINT message, LPARAM lParam)
{
  switch (message) {
    case PSCB_INITIALIZED: {
      PropSheetHwnd = hwnd;

      OrigSheetProc = (WNDPROC) SetWindowLong(PropSheetHwnd, GWL_WNDPROC, 
        (LONG) SheetSubclassProc); 
      PostMessage(hwnd, WM_APP, 0, 0);  // tell the dialog proc that the controls are stable
        
      
      return TRUE;
    } 
    case PSCB_PRECREATE:
      // Remove the DS_CONTEXTHELP style from the dialog template
      if (((LPDLGTEMPLATEEX)lParam)->signature ==  0xFFFF){
        ((LPDLGTEMPLATEEX)lParam)->style &= ~DS_CONTEXTHELP;
      } else {
        ((LPDLGTEMPLATE)lParam)->style &= ~DS_CONTEXTHELP;
      }
      return TRUE;
  }
  return TRUE;
}

static HWND CreatePropSheet(HINSTANCE hInst, HWND hwnd)
{
  PROPSHEETPAGE psp[2];
  PROPSHEETHEADER psh;

  psp[0].dwSize = sizeof(PROPSHEETPAGE);
  psp[0].dwFlags = PSP_USETITLE;
  psp[0].hInstance = hInst;
  psp[0].pszTemplate = MAKEINTRESOURCE(DLG_CONFIG);
  psp[0].pszIcon = NULL;
  psp[0].pfnDlgProc = ConfigDialogProc;
  psp[0].pszTitle = "General";
  psp[0].lParam = 0;
  psp[0].pfnCallback = NULL;

  psp[1].dwSize = sizeof(PROPSHEETPAGE);
  psp[1].dwFlags = PSP_USETITLE;
  psp[1].hInstance = hInst;
  psp[1].pszTemplate = MAKEINTRESOURCE(DLG_ADVCONFIG);
  psp[1].pszIcon = NULL;
  psp[1].pfnDlgProc = AdvConfigDialogProc;
  psp[1].pszTitle = "Advanced";
  psp[1].lParam = 0;
  psp[1].pfnCallback = NULL;

  psh.dwSize = sizeof(PROPSHEETHEADER);
  psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_USECALLBACK | PSH_NOAPPLYNOW; 
  psh.hwndParent = hwnd;
  psh.hInstance = hInst;
  psh.pszIcon = MAKEINTRESOURCE(ID_SPIREX_ICON);
  psh.pszCaption = (LPSTR) "Spirex Configuration";
  psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
  psh.nStartPage = 0;
  psh.ppsp = (LPCPROPSHEETPAGE) &psp;
  psh.pfnCallback = RemoveContextHelpProc;

  return (HWND)PropertySheet(&psh);
}

void CreateConfigDialog(HINSTANCE hInst, HWND hwnd)
{
  ControlsValid = false;
  INITCOMMONCONTROLSEX icc;
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icc);
  
  CreatePropSheet(hInst, hwnd);
  CleanUp();
}
