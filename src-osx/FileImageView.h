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

/* FileImageView */

#import <Cocoa/Cocoa.h>

@interface FileImageView : NSImageView
{
    IBOutlet NSStepper* mStepper;
    IBOutlet NSButton*  mDefaultButton;
    IBOutlet NSButton*  mClearButton;
    
    NSString* mPath;
    NSString* mDirPath;
    NSMutableArray* mOtherImages;
}

+ (NSString*) resolvePath: (NSString*) path;

- (IBAction)defaultImage:(id)sender;
- (IBAction)clearImage:(id)sender;

- (IBAction)imageStepperChanged:(id)sender;

@property (copy) NSString *path;

- (void) updateImageList;

@end
