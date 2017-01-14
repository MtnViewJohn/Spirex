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

#import "Spirex3DView.h"
#import "FileImageView.h"

#include "SpirexGeom.h"
#include "SpirexGL.h"

#import <OpenGL/glext.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/CGLContext.h>
#include <OpenGL/CGLCurrent.h>


@implementation Spirex3DView

+ (NSOpenGLPixelFormat *)defaultPixelFormat
{
	NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFAAllRenderers,
        NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)24,
		NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16,
        (NSOpenGLPixelFormatAttribute)0
    };
    return [[[NSOpenGLPixelFormat alloc]
				initWithAttributes:attributes]
					autorelease];
}

- (id) initWithFrame: (NSRect)frame andGeometry: (SpirexGeom*)geometry
{
	mPreparedGL = NO;
	mPreparedGeom = NO;
	mGeom = geometry;
	mTexturePath = @"";
	mTexture[0] = 0;
	mReshaped = NO;
	
	self = [super initWithFrame: frame
				pixelFormat: [[self class] defaultPixelFormat]];
	if (!self) return self;
	
	return self;
}

- (void) dealloc
{
	[mTexturePath release];
	[super dealloc];
}

- (void) reshape
{
	mPreparedGL = NO;
	mReshaped = YES;
}

- (void) update
{
	[super update];
	mPreparedGL = NO;
}

- (void) settingsChanged
{
	mPreparedGeom = NO;
}

- (void) setupTexture
{
	if (mGeom->mSettings.getTextureStrlen() == 0) {
		if (mTexture[0]) {
			glDeleteTextures(1, &mTexture[0]);
			mTexture[0] = 0;
			[mTexturePath release];
			mTexturePath = @"";
		}
		return;
	}
	
	NSString* path
		= [NSString stringWithCString: mGeom->mSettings.getTextureStr()
                             encoding: NSUTF8StringEncoding];
	path = [[FileImageView class] resolvePath: path];
	if (mTexture[0]  &&  [mTexturePath isEqualToString: path])
		return;
	[mTexturePath release];
	mTexturePath = @"";

    if (![self loadBitmap: path intoIndex:0]) 
        return;

	if (CGLGetCurrentContext ()) { 
		if (!mTexture[0])
			glGenTextures(1, &mTexture[0]);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, mTexture[0]);
			// consider GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE_EXT

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
        glTexImage2D(GL_TEXTURE_2D, 0, 3, mTexSize[0].width,
                     mTexSize[0].height, 0, mTexFormat[0],
                     GL_UNSIGNED_BYTE, mTexBytes[0]);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, mTexSize[0].width,
                     mTexSize[0].height, mTexFormat[0],
                     GL_UNSIGNED_BYTE, mTexBytes[0]);
	}
    free((void*)mTexBytes[0]);

	mTexturePath = [path retain];
}

/*
 * The NSBitmapImageRep is going to load the bitmap, but it will be
 * setup for the opposite coordinate system than what OpenGL uses, so
 * we copy things around.
 */
- (BOOL) loadBitmap:(NSString *)filename intoIndex:(int)texIndex
{
    BOOL success = FALSE;
    NSBitmapImageRep *theImage;
    NSInteger bitsPPixel, bytesPRow;
    unsigned char *theImageData;
    int rowNum, destRowNum;
    
    theImage = [NSBitmapImageRep imageRepWithContentsOfFile:filename ];
    if(theImage != nil) {
        bitsPPixel = [theImage bitsPerPixel];
        bytesPRow = [theImage bytesPerRow];
        if (bitsPPixel == 24)        // No alpha channel
            mTexFormat[texIndex] = GL_RGB;
        else if (bitsPPixel == 32)   // There is an alpha channel
            mTexFormat[texIndex] = GL_RGBA;
        mTexSize[texIndex].width = [theImage pixelsWide];
        mTexSize[texIndex].height = [theImage pixelsHigh];
        mTexBytes[texIndex] = (char*)calloc(bytesPRow * mTexSize[texIndex].height, 1);
        if (mTexBytes[texIndex] != NULL) {
            success = TRUE;
            theImageData = [theImage bitmapData];
            destRowNum = 0;
            for (rowNum = mTexSize[texIndex].height - 1; rowNum >= 0;
                rowNum--, destRowNum++ )
            {
                // Copy the entire row in one shot
                memcpy(mTexBytes[texIndex] + (destRowNum * bytesPRow),
                       theImageData + (rowNum * bytesPRow),
                       bytesPRow);
            }
        }
    }
    
    return success;
}

- (void) drawRect: (NSRect)rect
{
	NSOpenGLContext* ctx = [self openGLContext];	
	[ctx makeCurrentContext];

	if (!mPreparedGL) {
		GLint swapInt = 1;
		[ctx setValues: &swapInt forParameter: NSOpenGLCPSwapInterval];
		
		NSRect bounds = [self bounds];
		
        SpirexGL::InitGL(*mGeom, bounds.size.width, bounds.size.height);
            
		[self setupTexture];
		
		mPreparedGL = YES;
		mPreparedGeom = YES; // InitGL constructor calls InitMode
        SpirexGL::LevelOfDetail = 150;
	}
	if (!mPreparedGeom) {
		SpirexGL::InitMode(*mGeom);
		[self setupTexture];

		mPreparedGeom = YES;
	}
	
	glBindTexture(GL_TEXTURE_2D, mTexture[0]);
	SpirexGL::Render(-42, *mGeom);

	if (mReshaped) {
		// if resizing, let the window system's flush take care of it
		glFlush();
		mReshaped = NO;
		return;
	}
	
	[ctx flushBuffer];
}

@end
