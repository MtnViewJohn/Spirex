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

#import "SettingsSheet.h"
#import "SpirexScreenSaverView.h"
#import "FileImageView.h"
#include "SaverSettings.h"
#include <limits>

static NSString* keyCurrentSettings = @"currentSettings";
static NSString* keyUserPresets     = @"userPresets";
static NSString* keyRandomPreset    = @"randomPreset";

static NSString* keyCurveCount      = @"curveCount";
static NSString* keyCurveLength     = @"curveLength";
static NSString* keySpeed           = @"speed";
static NSString* keyEvolution       = @"evolution";
static NSString* keyThickLines      = @"thickLines";
static NSString* keyInColor         = @"inColor";
static NSString* keyFixed           = @"fixed";
static NSString* keyPoints          = @"points";
static NSString* keyTriAxial        = @"triAxial";
static NSString* keyMode            = @"mode";
static NSString* keyTexturePath     = @"texturePath";

// older keys
static NSString* keyCounterRotate   = @"counterRotate";
static NSString* key3DGeometry      = @"3DGeometry";
static NSString* key3DRender        = @"3DRender";
static NSString* key3DMode          = @"3DMode";

static bool
getDictUnsignedInt(NSDictionary* dict, NSString* key, unsigned int& value)
{
    id obj = dict[key];
    if (obj && [obj respondsToSelector: @selector(unsignedIntValue)]) {
        value = [obj unsignedIntValue];
        return true;
    }
    return false;
}

static bool
getDictBool(NSDictionary* dict, NSString* key, bool& value)
{
    id obj = dict[key];
    if (obj && [obj respondsToSelector: @selector(boolValue)]) {
        value = [obj boolValue];
        return true;
    }
    return false;
}

static bool
getDictString(NSDictionary* dict, NSString* key, NSString*& value)
{
    id obj = dict[key];
    if (obj && [obj isKindOfClass: [NSString class]]) {
        value = obj;
        return true;
    }
    return false;
}

static NSDictionary*
modeDictionary()
{
    static NSDictionary* modeDict = 0;
    
    if (!modeDict) {
        modeDict = [@{@"Original2D": [NSNumber numberWithInt: SaverSettings::Original2D],
            @"OriginalCounterRotate2D": [NSNumber numberWithInt: SaverSettings::OriginalCounterRotate2D],
            @"Original2DWith3DGeometry": [NSNumber numberWithInt: SaverSettings::Original2DWith3DGeometry],
            @"Curves": [NSNumber numberWithInt: SaverSettings::Curves],
            @"Spheres": [NSNumber numberWithInt: SaverSettings::Spheres],
            @"PointModeSpheres": [NSNumber numberWithInt: SaverSettings::PointModeSpheres],
            @"Disks": [NSNumber numberWithInt: SaverSettings::Disks],
            @"Cubes": [NSNumber numberWithInt: SaverSettings::Cubes],
            @"Squares": [NSNumber numberWithInt: SaverSettings::Squares],
            @"Conics": [NSNumber numberWithInt: SaverSettings::Conics],
            @"Cylinders": [NSNumber numberWithInt: SaverSettings::Cylinders],
            @"Toroids": [NSNumber numberWithInt: SaverSettings::Toroids],
            @"UtahTeapots": [NSNumber numberWithInt: SaverSettings::Teapots],
            @"WrappedCubes": [NSNumber numberWithInt: SaverSettings::WrappedCubes]} retain];
    }
    return modeDict;
}

@implementation SettingsSheet

+ (NSDictionary*)buildDictFromSettings: (const SaverSettings&) settings
{
    NSDictionary* modeDict = modeDictionary();
    NSArray* modes =
        [modeDict allKeysForObject: [NSNumber numberWithInt: settings.mMode]];
    id modeObj = (modes.count == 1)
        ? modes[0]
        : [NSNumber numberWithInt: settings.mMode];
    
    return @{keyCurveCount: @(settings.mCurveCount),
        keyCurveLength: @(settings.mCurveLength),
        keySpeed: @(settings.mAngleChangeRate),
        keyEvolution: @(settings.mEvolutionRate),
        keyThickLines: @(settings.mThickLines),
        keyInColor: @(settings.mInColor),
        keyFixed: @(settings.mFixed),
        keyPoints: @(settings.mPoints),
        keyTriAxial: @(settings.mTriAxial),
        keyMode: modeObj,
        keyTexturePath: @(settings.getTextureStr())};
}

+ (SaverSettings)readSettingsFromDict: (NSDictionary*)dict
{
    SaverSettings settings;
    
    getDictUnsignedInt(dict, keyCurveCount,     settings.mCurveCount);
    getDictUnsignedInt(dict, keyCurveLength,    settings.mCurveLength);
    getDictUnsignedInt(dict, keySpeed,          settings.mAngleChangeRate);
    getDictUnsignedInt(dict, keyEvolution,      settings.mEvolutionRate);
    getDictBool(dict, keyThickLines,            settings.mThickLines);
    getDictBool(dict, keyInColor,               settings.mInColor);
    getDictBool(dict, keyFixed,                 settings.mFixed);
    getDictBool(dict, keyPoints,                settings.mPoints);
    getDictBool(dict, keyTriAxial,              settings.mTriAxial);
    
    NSDictionary* modeDict = modeDictionary();
    
    bool oldMode = false;
    
    id modeObj = dict[keyMode];
    if (!modeObj) {
        modeObj = dict[key3DMode];
        oldMode = true;
    }
    NSNumber* modeNum = modeDict[modeObj];
    if (!modeNum) modeNum = modeObj;
    if (modeNum  &&  [modeNum respondsToSelector: @selector(intValue)]) {
        settings.mMode = (SaverSettings::RenderMode)modeNum.intValue;
    }

    if (oldMode) {
        bool counterRotate;
        bool geometry3D;
        bool render3D;
    
        getDictBool(dict, keyCounterRotate, counterRotate);
        getDictBool(dict, key3DGeometry,    geometry3D);
        getDictBool(dict, key3DRender,      render3D);
        
        if (geometry3D) {
            if (!render3D)
                settings.mMode = SaverSettings::Original2DWith3DGeometry;
        } else {
            if (counterRotate)
                settings.mMode = SaverSettings::OriginalCounterRotate2D;
            else
                settings.mMode = SaverSettings::Original2D;
        }
    }
    
    if (settings.mMode == SaverSettings::PointModeSpheres) {
        settings.mMode = SaverSettings::Spheres;
        settings.mPoints = true;
    }
    
    NSString* textureString = 0;
    getDictString(dict, keyTexturePath, textureString);
    settings.setTexture(textureString.UTF8String);

    return settings;
}

+ (void)writeCurrentSettings: (const SaverSettings&) settings
            toDefaults: (NSUserDefaults*)defaults
{
    [defaults setObject: [self buildDictFromSettings: settings]
        forKey: keyCurrentSettings];
    [defaults synchronize];
}

+ (BOOL)readCurrentSettings: (SaverSettings&) settings
            fromDefaults: (NSUserDefaults*)defaults
{
    NSDictionary* dict = [defaults dictionaryForKey: keyCurrentSettings];
    if (!dict) return NO;
    
    settings = [self readSettingsFromDict: dict];
    return YES;
}

+ (NSDictionary*) userPresetsDictionaryFromDefaults: (NSUserDefaults*) defaults
{
    return [defaults dictionaryForKey: keyUserPresets];
}

+ (NSDictionary*) standardPresetsDictionary
{
    NSBundle* bundle = [NSBundle bundleForClass: [self class]];
    NSString* presetsPath =
                [bundle pathForResource: @"Presets" ofType: @"plist"];
    NSData* presetsData = [NSData dataWithContentsOfFile: presetsPath];
    NSError* error = 0;
    id presetsObj = [NSPropertyListSerialization
        propertyListWithData: presetsData
        options: NSPropertyListImmutable
        format: NULL
        error: &error];
    if (!presetsObj || ![presetsObj isKindOfClass: [NSDictionary class]]) {
        if (error) [error release];
        presetsObj = NULL;
    }
    return (NSDictionary*)presetsObj;
}

+ (SaverSettings) configurationFromDefaults: (NSUserDefaults*) defaults
                        isPreview: (BOOL) isPreview
{
    SaverSettings currentSettings;
    BOOL hasCurrent
        = [SettingsSheet readCurrentSettings: currentSettings
                fromDefaults: defaults];
    if (hasCurrent  &&  ![defaults boolForKey: keyRandomPreset])
        return currentSettings;
        
    NSDictionary* presets
        = [self userPresetsDictionaryFromDefaults: defaults];
    if (!presets  ||  presets.count == 0)
        presets = [self standardPresetsDictionary];
        
    int index = SSRandomIntBetween(0, static_cast<int>(presets.count - 1));
    
    NSDictionary* preset
        = presets[presets.allKeys[index]];
    
    return [self readSettingsFromDict: preset];
}

- (instancetype)init
{
    self = [super init];
    if (!self) return self;
    
    return self;
}

- (void)dealloc
{
    [mDrawer close];
    [mSaverPreview release]; mSaverPreview = nil;
    
    [[NSNotificationCenter defaultCenter]
        removeObserver: self];
    [super dealloc];
}

- (void)awakeFromNib
{
    NSBundle* bundle = [NSBundle bundleForClass: [self class]];
    NSDictionary* info = bundle.infoDictionary;
    mVersionLabel.stringValue = [NSString stringWithFormat: @"Spirex v%@",
                               info[@"CFBundleShortVersionString"]
                               ];
    mCopyrightLabel.stringValue = info[@"NSHumanReadableCopyright"];
    
    mUserPresets.clear();
    mStandardPresets.clear();
    mFixed3DView = NO;

    [mSettingsCurveNumber removeAllItems];
    
    NSMenu* countMenu = mSettingsCurveNumber.menu;
    
    for (int i = 1; i <= SaverSettings::MaxCurveCount; ++i) {
        NSMenuItem* item = [[NSMenuItem alloc]
            initWithTitle: [NSString stringWithFormat: @"%d", i]
            action: NULL
            keyEquivalent: @""];
        if (!item) continue;
        
        item.tag = i;
        [countMenu addItem: item];
        [item release];
    };
}

- (NSPanel*)showSheetWithSettings: (const SaverSettings&) settings;
{
    mRandom = [[SpirexScreenSaverView defaults] boolForKey: keyRandomPreset];
    mSavedRandom.state = mRandom ? NSOnState : NSOffState;
    
    mSettings = settings;
    
    [self rebuildPresets];

    size_t presetIndex = std::numeric_limits<size_t>::max();
    
    for (size_t i = 0; i < mUserPresets.size(); ++i) {
        if (mUserPresets[i] == mSettings) {
            presetIndex = i;
            break;
        }
    }
    if (presetIndex == std::numeric_limits<size_t>::max()) {
        for (size_t i = 0; i < mStandardPresets.size(); ++i) {
            if (mStandardPresets[i] == mSettings) {
                presetIndex =
                      mUserPresets.size()
                    + (mUserPresets.empty() ? 0 : 1)
                    + i;
                break;
            }
        }
    }
        
    [mSavedSettings selectItemAtIndex: presetIndex];
    [self displaySettings];
    [self updateSavedUI];
    
    if (!mSaverPreview) {
        mSaverPreview = [[SpirexScreenSaverView alloc] initWithFrame: mSaverPane.bounds
                                                           isPreview: YES];
        [mSaverPreview isConfig];
    }
    
    [mSaverPane addSubview: mSaverPreview];

    [mSaverPreview readConfiguration];
    
    if (mPreviewTimer) {
        [mPreviewTimer invalidate];
        mPreviewTimer = nil;
    }
    
    mPreviewTimer = [NSTimer timerWithTimeInterval: mSaverPreview.animationTimeInterval
                                            target: self
                                          selector: @selector(timerAnimate:)
                                          userInfo: nil
                                           repeats: YES];
    [[NSRunLoop mainRunLoop] addTimer: mPreviewTimer forMode: NSRunLoopCommonModes];
    
    return mDrawer;
}

- (void)timerAnimate:(NSTimer *)timer
{
    [mSaverPreview animateOneFrame];
}

- (void)save
{
    [SettingsSheet writeCurrentSettings: mSettings
        toDefaults: [SpirexScreenSaverView defaults]];
    [mSaver newSettings: mSettings];
}


- (void)savedPick:(id) sender
{
    NSInteger i = mSavedSettings.indexOfSelectedItem;
    if (0 <= i  &&  i < mUserPresets.size())
        mSettings = mUserPresets[i];
    else {
        i -= mUserPresets.size();
        if (!mUserPresets.empty()) i -= 1; // the separator
        if (0 <= i  &&  i < mStandardPresets.size())
            mSettings = mStandardPresets[i];
    }
    
    [self displaySettings];
    [self updateSavedUI];
    [mSaverPreview newSettings: mSettings];
}

- (void)savedSave:(id) sender
{
    NSString* name = mSavedName.stringValue;
    if (name.length == 0) return;
    
    NSDictionary* preset = [SettingsSheet buildDictFromSettings: mSettings];

    NSUserDefaults* defaults = [SpirexScreenSaverView defaults];
    NSDictionary* presets = [defaults dictionaryForKey: keyUserPresets];
    
    NSMutableDictionary* newPresets;
    if (presets)
        newPresets = [NSMutableDictionary dictionaryWithDictionary: presets];
    else
        newPresets = [NSMutableDictionary dictionaryWithCapacity: 1];
        
    newPresets[name] = preset;
    [defaults setObject: newPresets forKey: keyUserPresets];
    [defaults synchronize];

    [self rebuildPresets];
    [mSavedSettings selectItemWithTitle: name];
    [self updateSavedUI];
}

- (void)savedDelete:(id) sender
{
    NSString* name = mSavedSettings.titleOfSelectedItem;
    if (name.length == 0) return;
    
    NSUserDefaults* defaults = [SpirexScreenSaverView defaults];
    NSDictionary* presets = [defaults dictionaryForKey: keyUserPresets];
    if (!presets) return;

    NSMutableDictionary* newPresets
            = [NSMutableDictionary dictionaryWithDictionary: presets];

    [newPresets removeObjectForKey: name];
    [defaults setObject: newPresets forKey: keyUserPresets];
    [defaults synchronize];

    [self rebuildPresets];
    [mSavedSettings selectItemAtIndex: -1];
    [self updateSavedUI];
}

- (void)savedNameChanged:(id) sender
{
    [mSavedSettings selectItemAtIndex: -1];
    [self updateSavedUI];
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
    [self savedNameChanged: self];
}

- (void)savedRandomChanged:(id) sender
{
    mRandom = mSavedRandom.state == NSOnState;
    if (sender != self) {
        NSUserDefaults* def = [SpirexScreenSaverView defaults];
        [def setBool: mRandom forKey: keyRandomPreset];
        [def synchronize];
    }
    
    if (mRandom)
        [mSaverPreview readConfiguration];
    else
        [mSaverPreview newSettings: mSettings];
    [self enableSettings];
}

- (IBAction) cancelSheetAction: (id) sender {
    //  close the sheet without saving the settings
    [mSaverPreview readConfiguration];
    if (mPreviewTimer) {
        [mPreviewTimer invalidate];
        mPreviewTimer = nil;
    }
    [NSApp endSheet: mDrawer];
}

- (IBAction) okSheetAction: (id) sender {
    [self save];
    if (mPreviewTimer) {
        [mPreviewTimer invalidate];
        mPreviewTimer = nil;
    }
    [NSApp endSheet: mDrawer];
}

- (void)settingsChanged:(id) sender
{
    [self fetchSettings];
    [self enableSettings];
    [mSaverPreview newSettings: mSettings];
    
    NSInteger selectedIndex = mSavedSettings.indexOfSelectedItem;
    if (selectedIndex >= 0) {
        NSString* name = mSavedSettings.titleOfSelectedItem;
        
        if (selectedIndex > mUserPresets.size()) {
            //XXX Localize this string
            name = [NSString localizedStringWithFormat: @"Copy of %@", name];
        }
        
        mSavedName.stringValue = name;
        [mSavedName setEnabled: YES];
        [mSavedSettings selectItemAtIndex: -1];
        
        [self updateSavedUI];
    }
}


- (void)rebuildPresets
{
    NSUserDefaults* defaults = [SpirexScreenSaverView defaults];
    
    [mSavedSettings removeAllItems];

    // First, find and load up the user's presets
    do {
        mUserPresets.clear();
        
        NSDictionary* presets
            = [[self class] userPresetsDictionaryFromDefaults: defaults];
        if (!presets) break;
        
        NSArray* keys = presets.allKeys;
        keys = [keys sortedArrayUsingSelector:
            @selector(caseInsensitiveCompare:)];
            
        mUserPresets.reserve(keys.count);
        
        for (int i = 0; i < keys.count; ++i) {
            NSString* key = keys[i];
            [mSavedSettings addItemWithTitle: key];
            mUserPresets.push_back([SettingsSheet readSettingsFromDict:
                                    presets[key]]);
        };
        
        if (!mUserPresets.empty() > 0) {
            [mSavedSettings.menu addItem: [NSMenuItem separatorItem]];
        }
    } while (false);

    // Second, find and load up the standard presets
    do {
        mStandardPresets.clear();
        
        NSDictionary* presets = [[self class] standardPresetsDictionary];
        if (!presets) break;
        
        NSArray* keys = presets.allKeys;
        keys = [keys sortedArrayUsingSelector:
                        @selector(caseInsensitiveCompare:)];

        mStandardPresets.reserve(keys.count);
        
        for (int i = 0; i < keys.count; ++i) {
            NSString* key = keys[i];
            [mSavedSettings addItemWithTitle: key];
            mStandardPresets.push_back([SettingsSheet readSettingsFromDict:
                                            presets[key]]);
        };
    } while (false);
}

- (void)updateSavedUI
{
    NSInteger i = mSavedSettings.indexOfSelectedItem;
    if (i < 0) {
        bool hasName = mSavedName.stringValue.length > 0;
        mSavedSaveButton.enabled = hasName;
        [mSavedDeleteButton setEnabled: NO];
    }
    else {
        mSavedName.stringValue = @"";
        [mSavedName setEnabled: NO];
        mSavedDeleteButton.enabled = (i < mUserPresets.size());
        [mSavedSaveButton setEnabled: NO];
    }
}


- (void)displaySettings
{
    [mSettingsMode selectItem:
        [mSettingsMode.menu itemWithTag: mSettings.mMode]];

    [mSettingsCurveNumber selectItem:
        [mSettingsCurveNumber.menu itemWithTag: mSettings.mCurveCount]];
        
    mSettingsCurveLength.intValue = mSettings.mCurveLength;
    mSettingsSpeed.intValue = mSettings.mAngleChangeRate;
    mSettingsEvolutionRate.intValue = mSettings.mEvolutionRate;
    
    mSettingsThick.state =      mSettings.mThickLines   ? NSOnState : NSOffState;
    mSettingsColor.state =      mSettings.mInColor      ? NSOnState : NSOffState;
    mSettingsFixed.state =      mSettings.mFixed        ? NSOnState : NSOffState;
    mSettingsPoints.state =     mSettings.mPoints       ? NSOnState : NSOffState;
    mSettingsTriAxial.state =   mSettings.mTriAxial ? NSOnState : NSOffState;
    
    [mSettingsTexture setPath:
        @(mSettings.getTextureStr())];
    
    [self enableSettings];
}

- (void)enableSettings
{
    mSettingsMode.enabled = !mRandom;
    mSettingsCurveNumber.enabled = !mRandom;
    mSettingsCurveLength.enabled = !mRandom && mSettings.usesLength();
    mSettingsSpeed.enabled = !mRandom;
    mSettingsEvolutionRate.enabled = !mRandom;
    mSettingsThick.enabled = !mRandom && mSettings.usesThickness();
    mSettingsColor.enabled = !mRandom;
    mSettingsFixed.enabled = !mRandom && mSettings.usesFixed();
    mSettingsPoints.enabled = !mRandom;
    mSettingsTriAxial.enabled = !mRandom && mSettings.usesTriAxial();
    mSettingsTexture.enabled = !mRandom && mSettings.usesTexture();
}

- (void)fetchSettings
{
    mSettings.mMode         = (SaverSettings::RenderMode)
                                  mSettingsMode.selectedItem.tag;
    mSettings.mCurveCount   = static_cast<int>(mSettingsCurveNumber.selectedItem.tag);
    mSettings.mCurveLength  = mSettingsCurveLength.intValue;
    mSettings.mAngleChangeRate = mSettingsSpeed.intValue;
    mSettings.mEvolutionRate   = mSettingsEvolutionRate.intValue;
    
    mSettings.mThickLines   = mSettingsThick.state == NSOnState;
    mSettings.mInColor      = mSettingsColor.state == NSOnState;
    mSettings.mFixed        = mSettingsFixed.state == NSOnState;
    mSettings.mPoints       = mSettingsPoints.state == NSOnState;
    mSettings.mTriAxial     = mSettingsTriAxial.state == NSOnState;
    
    NSString* texturePath = [mSettingsTexture path];
    if (texturePath  &&  texturePath.length > 0)
        mSettings.setTexture(texturePath.UTF8String);
    else
        mSettings.clearTexture();
}


@end
