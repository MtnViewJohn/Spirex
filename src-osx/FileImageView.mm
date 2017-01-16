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

#import "FileImageView.h"

@implementation FileImageView

+ (NSString*) resolvePath: (NSString*) path
{
	if (!path || path.length == 0 || path.absolutePath)
		return path;
	
	return [[NSBundle bundleForClass: [self class]] pathForImageResource: path];
}


- (void)awakeFromNib
{
	mPath = @"";
	mDirPath = @"";
	mOtherImages = [[NSMutableArray alloc] initWithCapacity: 10];
	
	[self unregisterDraggedTypes];
	[self registerForDraggedTypes:
		@[NSFilenamesPboardType]];
	[self updateImageList];
}

- (void)dealloc
{
	[mPath release];
	[mDirPath release];
	[mOtherImages release];
	[super dealloc];
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    NSPasteboard *pboard = [sender draggingPasteboard];

    if ( [pboard.types containsObject:NSFilenamesPboardType] ) {
        NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
        NSUInteger numberOfFiles = files.count;
		if (numberOfFiles != 1)
			return NSDragOperationNone;
    }
	
    return [super draggingEntered: sender];
	// should we check the type here, saying we only do link?
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    NSPasteboard *pboard = [sender draggingPasteboard];

    if ( [pboard.types containsObject:NSFilenamesPboardType] ) {
        NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
		if (files.count != 1)
			return NO;
		id path = files[0];
		if (![path isKindOfClass: [NSString class]])
			return NO;
			
		[mPath release];
		mPath = [path retain];
    }
	else {
		return NO;
	}
	
    BOOL result = [super performDragOperation: sender];
	if (result)
		[self updateImageList];
	return result;
}

- (IBAction)imageStepperChanged:(id)sender;
{
	[self setPath: mOtherImages[mStepper.intValue]];
	[self sendAction: self.action to: self.target];
} 

- (IBAction)defaultImage:(id)sender
{
	[self setPath: @"Spirex"];
	[self sendAction: self.action to: self.target];
}

- (IBAction)clearImage:(id)sender
{
	[self setPath: @""];
	[self sendAction: self.action to: self.target];
}

- (NSString*) path
{
	return mPath;
}

- (void) setPath: (NSString*) path
{
	NSString* oldPath = mPath;
	if (!path || path.length == 0) {
		mPath = @"";
		[self setImage: NULL];
	}
	else {
		mPath = [path retain];
		self.image =	[[[NSImage alloc]
				initByReferencingFile: [[self class] resolvePath: mPath]]
			autorelease];
	}
	[oldPath release];
	[self updateImageList];
}

- (void) setEnabled: (BOOL)flag
{
	if (flag) {
		[self setPath: mPath];
		mStepper.enabled = mOtherImages.count > 0;
	}
	else {
		[self setImage: NULL];
		[mStepper setEnabled: NO];
	}
	super.enabled = flag;
	mDefaultButton.enabled = flag;
	mClearButton.enabled = flag;
}


- (void) updateImageList
{
	NSString* path = [[self class] resolvePath: mPath];
	
	if (path.length == 0) {
		[mDirPath release];
		mDirPath = @"";
		[mOtherImages removeAllObjects];
		mStepper.minValue = 0.0;
		mStepper.maxValue = 0.0;
		mStepper.intValue = 0;
		mStepper.increment = 1.0;
		[mStepper setEnabled: NO];
		return;
	}

	NSString* dirPath = path.stringByDeletingLastPathComponent;
	if ([mDirPath isEqualToString: dirPath])
		return;
	[mDirPath release];
	mDirPath = [dirPath retain];

	[mOtherImages removeAllObjects];
	
	NSArray* fileTypes = [NSImage imageFileTypes];
	
	NSDirectoryEnumerator *direnum
		= [[NSFileManager defaultManager] enumeratorAtPath: dirPath];

	while (NSString *fileName = [direnum nextObject]) {
		[direnum skipDescendents]; // never recurse
		if ([fileTypes containsObject: fileName.pathExtension]) {
			[mOtherImages addObject:
				[dirPath stringByAppendingPathComponent: fileName]];
		}
	}
	
	mStepper.minValue = 0.0;
	mStepper.maxValue = mOtherImages.count - 1;
	mStepper.intValue = static_cast<int>([mOtherImages indexOfObject: path]);
	mStepper.increment = 1.0;
	[mStepper setEnabled: YES];
}

@end
