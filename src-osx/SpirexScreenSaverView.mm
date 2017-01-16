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

#import "SpirexScreenSaverView.h"
#import "Spirex3DView.h"
#import "SettingsSheet.h"
#import "Updater.h"

#include "SaverSettings.h"
#include "SpirexGeom.h"


@implementation SpirexScreenSaverView

+ (void)initialize
{
	srand(static_cast<unsigned>(time(0)));
}

- (instancetype)initWithFrame:(NSRect)frame
{
	return [self initWithFrame: frame isPreview: YES];
}

- (instancetype)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview
{
	mGeom = 0;
	m3DView = NULL;
	mIsPreview = isPreview;
	mReconfigDate = NULL;
	mSheet = NULL;
    mUpdater = nil;
	
    self = [super initWithFrame:frame isPreview:isPreview];
    if (!self) return self;

	SaverSettings nullSettings;
	mGeom = new SpirexGeom(nullSettings);

	[self setAutoresizesSubviews: YES];

	m3DView = [[Spirex3DView alloc]
				initWithFrame: self.bounds andGeometry: mGeom]; 
	[self addSubview: m3DView];

	self.animationTimeInterval = 0.030;
	
	[self readConfiguration];

    mUpdater = [[[Updater alloc]
        initWithTarget: self andAction: @selector(showInfo:)] retain];
    [mUpdater checkForUpdate: FALSE];

    return self;
}

- (void)dealloc
{
	delete mGeom;
	if (mReconfigDate)  [mReconfigDate release];
    if (mUpdater)       [mUpdater release];
    
	[super dealloc];
}

- (void)readConfiguration
{
	[self newSettings:
		[SettingsSheet configurationFromDefaults: [SpirexScreenSaverView defaults]
				isPreview: mIsPreview]];
}

- (void)newSettings: (const SaverSettings&)newSettings
{
	mGeom->NewSaverSettings(newSettings);

	[m3DView settingsChanged];
	[m3DView setNeedsDisplay: YES];

	NSTimeInterval reconfigRate = mIsPreview ? 30.0 : (5.0 * 60.0);
	if (mReconfigDate) [mReconfigDate release];
	mReconfigDate =
		[[NSDate dateWithTimeIntervalSinceNow: reconfigRate] retain];
}

- (void)animateOneFrame
{
	if ([mReconfigDate compare: [NSDate date]] == NSOrderedAscending)
		[self readConfiguration];
	
	mGeom->NextStep();
	mGeom->NextStep();

	[m3DView setNeedsDisplay: YES];
}

- (BOOL)hasConfigureSheet
{
    return YES;
}

- (NSWindow*)configureSheet
{
	if (!mSheet)
        if (![NSBundle loadNibNamed:@"SettingsDrawer" owner:self]) {
            NSLog(@"Failed to load the config sheet");
            return nil;
        }

    return [mSheet showSheetWithSettings: mGeom->mSettings];
}

+ (NSUserDefaults*) defaults
{
	return [Updater defaults];
}

- (void)showInfo: (id)sender;
{
    NSString* spirexInfo =
        [[NSBundle bundleForClass: [self class]]
            pathForResource: @"SpirexInfo" ofType: @"app"];
    [[NSWorkspace sharedWorkspace] launchApplication: spirexInfo];
}


@end
