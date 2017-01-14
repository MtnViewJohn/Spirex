// Spirex Screensaver
// ---------------------
// Copyright (C) 2004 John Horigan
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

#import <Cocoa/Cocoa.h>
@class SpirexScreenSaverView;
@class FileImageView;
#include "SaverSettings.h"
#include <vector>

@interface SettingsSheet : NSObject<NSWindowDelegate>
{
	IBOutlet NSPanel*		mDrawer;
    IBOutlet NSView*        mSaverPane;
	
	IBOutlet NSPopUpButton*	mSavedSettings;
	IBOutlet NSTextField*   mSavedName;
	IBOutlet NSButton*		mSavedSaveButton;
	IBOutlet NSButton*		mSavedDeleteButton;
	IBOutlet NSButton*		mSavedRandom;
	
	IBOutlet NSPopUpButton* mSettingsMode;
	IBOutlet NSPopUpButton* mSettingsCurveNumber;
	IBOutlet NSSlider*		mSettingsCurveLength;
	IBOutlet NSSlider*		mSettingsSpeed;
	IBOutlet NSSlider*		mSettingsEvolutionRate;
	IBOutlet NSButton*		mSettingsThick;
	IBOutlet NSButton*		mSettingsColor;
	IBOutlet NSButton*		mSettingsFixed;
	IBOutlet NSButton*		mSettingsPoints;
	IBOutlet NSButton*		mSettingsTriAxial;
    
    IBOutlet NSTextField*   mVersionLabel;
    IBOutlet NSTextField*   mCopyrightLabel;    
	
	IBOutlet FileImageView* mSettingsTexture;
	
	IBOutlet SpirexScreenSaverView*		mSaver;


	BOOL					mRandom;
	SaverSettings 			mSettings;
	
    std::vector<SaverSettings> mUserPresets;
	std::vector<SaverSettings> mStandardPresets;
	
	BOOL					mFixed3DView;
    
    SpirexScreenSaverView*  mSaverPreview;
    NSTimer*                mPreviewTimer;
}

- (id)init;
- (void)dealloc;
- (void)awakeFromNib;

- (NSPanel*)showSheetWithSettings: (const SaverSettings&) settings;

- (void)save;
- (void)timerAnimate:(NSTimer *)timer;

- (void)savedPick:(id) sender;
- (void)savedSave:(id) sender;
- (void)savedDelete:(id) sender;
- (void)savedNameChanged:(id) sender;
- (void)controlTextDidChange:(NSNotification *)aNotification;
- (void)savedRandomChanged:(id) sender;
- (IBAction) cancelSheetAction: (id) sender;
- (IBAction) okSheetAction: (id) sender;
- (void)settingsChanged:(id) sender;

- (void)rebuildPresets;
- (void)updateSavedUI;

- (void)displaySettings;
- (void)enableSettings;
- (void)fetchSettings;

+ (NSDictionary*)buildDictFromSettings: (const SaverSettings&) settings;
+ (SaverSettings)readSettingsFromDict: (NSDictionary*)dict;

+ (void)writeCurrentSettings: (const SaverSettings&) settings
			toDefaults: (NSUserDefaults*)defaults;
+ (BOOL)readCurrentSettings: (SaverSettings&) settings
			fromDefaults: (NSUserDefaults*)defaults;

+ (NSDictionary*) userPresetsDictionaryFromDefaults: (NSUserDefaults*) defaults;
+ (NSDictionary*) standardPresetsDictionary;

+ (SaverSettings) configurationFromDefaults: (NSUserDefaults*) defaults
						isPreview: (BOOL) isPreview;

@end
