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

#import "Updater.h"
#import <ScreenSaver/ScreenSaver.h>
#include "SystemConfiguration/SCNetwork.h"
#include "SystemConfiguration/SCNetworkReachability.h"
#include <libkern/OSAtomic.h>

// keys in the update dictionary fetch from the server
static NSString* DefaultModuleName = @"com.ozonehouse.spirex";
static NSString* keyCheckForUpdates	= @"checkForUpdates";
static NSString* keyLastUpdateCheck = @"lastUpdateCheck";

static NSString* urlUpdate
    = @"https://www.ozonehouse.com/Spirex/SpirexMacOSXVersion.xml?v=%d&t=%1.0f";
static const char* updateServer
    = "www.ozonehouse.com";
    
// keys in the update dictionary fetch from the server
static NSString* keyBundleVersion = @"CFBundleVersion";
static NSString* keyDownloadURL = @"SpirexDownloadURL";

static const NSTimeInterval checkingInterval = 7*24*60*60;
    // one week in seconds

static int32_t instanceCounter = 0;

@implementation Updater

- (id) initWithTarget: (id)target
    andAction:(SEL)action;
{
    mID = OSAtomicAdd32Barrier(1, &instanceCounter);
    mUpdateInfo = nil;
    mTarget = [target retain];
    mAction = action;
    mDefaults = [[Updater defaults] retain];
    
    mThisVersion =
        [[[[NSBundle bundleForClass: [self class]] infoDictionary]
            objectForKey: keyBundleVersion] intValue];
    if (mThisVersion == 0 || mThisVersion == INT_MIN || mThisVersion == INT_MAX)
        mThisVersion = 0;

    return self;
}

- (void) dealloc
{
    if (mUpdateInfo)    [mUpdateInfo release];
    if (mTarget)        [mTarget release];
    if (mDefaults)      [mDefaults release];
    
    OSAtomicAdd32Barrier(-1, &instanceCounter);
    [super dealloc];
}

+ (NSUserDefaults*) defaults
{
    return [ScreenSaverDefaults defaultsForModuleWithName: DefaultModuleName];
}


- (void) checkForUpdate: (BOOL) alwaysCheck;
{
    if (mID != 1)
        return;
        
    if (mUpdateInfo) {
        [self completeCheck];
        return;
    }
    
    [mDefaults synchronize];
    NSObject* lastUpdateCheck = [mDefaults objectForKey: keyLastUpdateCheck];
    NSDate* lastUpdateCheckDate = nil;
    if ([lastUpdateCheck isKindOfClass: [NSString class]]) {
        NSDateFormatter *dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
        dateFormatter.dateFormat = @"yyyy-MM-dd HH:mm:ss ZZZ";
        lastUpdateCheckDate = [dateFormatter dateFromString: (NSString*)lastUpdateCheck];
    } else if ([lastUpdateCheck isKindOfClass: [NSDate class]])
        lastUpdateCheckDate = (NSDate*)lastUpdateCheck;
    
    if (lastUpdateCheckDate) {
        mSinceLastCheck = - [lastUpdateCheckDate timeIntervalSinceNow];
    } else {
        mSinceLastCheck = -1;
    }

    if (!alwaysCheck) {
        BOOL checkForUpdate = 
            [mDefaults objectForKey: keyCheckForUpdates] == nil
            || [mDefaults boolForKey: keyCheckForUpdates];
        if (!checkForUpdate) return;
        
        if (0 <= mSinceLastCheck  &&  mSinceLastCheck < checkingInterval)
            return;
    }
        
    bool networkUp = false;
    SCNetworkConnectionFlags flags;
    Boolean ok;
    SCNetworkReachabilityRef target = SCNetworkReachabilityCreateWithName(NULL, updateServer);
    ok = SCNetworkReachabilityGetFlags(target, &flags);
    CFRelease(target);
    if (ok) {
        networkUp =
                !(flags & kSCNetworkFlagsConnectionRequired)
             &&  (flags & kSCNetworkFlagsReachable);
        // See "Technical Note TN1145 Living in a Dynamic TCP/IP Environment
        // http://developer.apple.com/technotes/tn/tn1145.html
    }
    if (!networkUp) return;

    NSMachPort* updatePort = [[NSMachPort alloc] init];
    [updatePort setDelegate: self];
    [updatePort
        scheduleInRunLoop: [NSRunLoop currentRunLoop]
        forMode:(NSString *)kCFRunLoopCommonModes];

    [self retain];
    [NSThread detachNewThreadSelector:
        @selector(runGetUpdateThread:) toTarget: self withObject: updatePort];
}

- (void) runGetUpdateThread: (id) updatePort
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    NSString* requestURL =
        [NSString stringWithFormat: urlUpdate, mThisVersion, mSinceLastCheck];

    mUpdateInfo =
        [[NSDictionary dictionaryWithContentsOfURL:
                [NSURL URLWithString: requestURL]] retain];
    [updatePort sendBeforeDate: [NSDate date]
            components: nil from: nil reserved: 0];
    
    [pool release];
}

- (void) handleMachMessage: (void*) msg
{
    [self completeCheck];
    [self autorelease];
}

- (void) completeCheck;
{
    [mDefaults
        setObject: [NSDate date]
        forKey: keyLastUpdateCheck];
    [mDefaults synchronize];
        
    int updateVersion =
        [[mUpdateInfo objectForKey: keyBundleVersion] intValue];

    if (mThisVersion == 0 || mThisVersion == INT_MIN || mThisVersion == INT_MAX
    ||  updateVersion == 0 || updateVersion == INT_MIN || updateVersion == INT_MAX
    ||  mThisVersion >= updateVersion)
        return;

    [mTarget performSelector: mAction withObject: self];
}


- (NSURL*) downloadURL
{
    return [NSURL URLWithString: [mUpdateInfo objectForKey: keyDownloadURL]];
}




@end
