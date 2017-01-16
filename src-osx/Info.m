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

#import "Info.h"
#import "Updater.h"
#import <ScreenSaver/ScreenSaver.h>

// keys in the update dictionary fetch from the server
static NSString* keyCheckForUpdates	= @"checkForUpdates";


@implementation Info

- (void) awakeFromNib
{
    NSBundle* bundle = [NSBundle bundleForClass: [self class]];
	mDefaults = [[Updater defaults] retain];

    NSDictionary* info = [bundle infoDictionary];
    [mVersion setStringValue: [NSString stringWithFormat: @"%@ (v%@)",
        [info objectForKey: @"CFBundleShortVersionString"],
        [info objectForKey: @"CFBundleVersion"] 
        ]];
        
    BOOL checkForUpdate = 
        [mDefaults objectForKey: keyCheckForUpdates] == nil
        || [mDefaults boolForKey: keyCheckForUpdates];
        
    [mCheckForUpdates setState: checkForUpdate ? NSOnState : NSOffState];
    
    
    float areaHeight = [mUpdateArea frame].size.height;
    mInfoFrameBig = [mInfoPanel frame];
    mInfoFrameSmall = mInfoFrameBig;
    mInfoFrameSmall.size.height -= areaHeight;
    mInfoFrameSmall.origin.y += areaHeight;

    [mUpdateArea retain];
    [mUpdateArea removeFromSuperview];
    [mInfoPanel setFrame: mInfoFrameSmall display: TRUE];

    mUpdater = [[[Updater alloc]
        initWithTarget: self andAction: @selector(showUpdateArea:)] retain];
    [mUpdater checkForUpdate: TRUE];
}

- (void) dealloc
{
    if (mUpdateArea)    [mUpdateArea release];
    if (mDefaults)      [mDefaults release];
    if (mUpdater)       [mUpdater release];

    [super dealloc];
}

- (void) showUpdateArea: (id) sender;
{
    [mInfoPanel setFrame: mInfoFrameBig display: TRUE animate: TRUE];
    [[mInfoPanel contentView] addSubview: mUpdateArea];
}



- (void) goToWebsite: (id) sender
{
    [[NSWorkspace sharedWorkspace] openURL:
        [NSURL URLWithString: @"https://www.ozonehouse.com/Spirex/"]];
}

- (void) changeCheckForUpdate: (id) sender
{
	BOOL checkForUpdates = [mCheckForUpdates state] == NSOnState;
    
    BOOL oldCheckForUpdate = 
        [mDefaults objectForKey: keyCheckForUpdates] == nil
        || [mDefaults boolForKey: keyCheckForUpdates];
    if (oldCheckForUpdate == checkForUpdates)
        return;
        
    [mDefaults setBool: checkForUpdates forKey: keyCheckForUpdates];
    
    [mUpdater checkForUpdate: FALSE];
}

- (void) remindLater: (id) sender
{
    [NSApp terminate: sender];
}

- (void) downloadNow: (id) sender
{
    [[NSWorkspace sharedWorkspace] openURL: [mUpdater downloadURL]];
    [NSApp terminate: self];
}

- (void) windowWillClose: (NSNotification*) aNotification
{
    [NSApp terminate: self];
}



@end
