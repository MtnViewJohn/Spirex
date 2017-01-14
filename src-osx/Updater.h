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

#import <Foundation/Foundation.h>

@interface Updater : NSObject<NSMachPortDelegate> {
    int             mID;
    NSDictionary*   mUpdateInfo;
    
    id              mTarget;
    SEL             mAction;
    NSUserDefaults* mDefaults;
    
    int             mThisVersion;
    NSTimeInterval  mSinceLastCheck;
}

- (id)initWithTarget: (id)target
    andAction: (SEL)action;
- (void)dealloc;

- (void)checkForUpdate: (BOOL)alwaysCheck;
- (NSURL*)downloadURL;
+ (NSUserDefaults*) defaults;

// private
- (void)runGetUpdateThread: (id)args;
- (void)handleMachMessage: (void*)msg;
- (void)completeCheck;

@end
