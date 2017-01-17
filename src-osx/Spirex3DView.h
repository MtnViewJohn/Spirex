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
#import <OpenGL/gl.h>
class SpirexGeom;
class SpirexGL;

@interface Spirex3DView : NSOpenGLView {
    BOOL    mPreparedGL;
    BOOL    mPreparedGeom;
    
    SpirexGeom* mGeom;

    NSString*   mTexturePath;
    GLuint      mTextureName;
    
    BOOL        mReshaped;
    
    GLenum mTexFormat[ 1 ];   // Format of texture (GL_RGB, GL_RGBA)
    NSSize mTexSize[ 1 ];     // Width and height
    char *mTexBytes[ 1 ];     // Texture data
    GLuint mTexture[ 1 ];     // Storage for one texture
}

+ (NSOpenGLPixelFormat *)defaultPixelFormat;

- (instancetype) initWithFrame: (NSRect)frame andGeometry: (SpirexGeom*)geometry NS_DESIGNATED_INITIALIZER;
- (void) setupTexture;
- (void) settingsChanged;
//- (GLubyte *) loadBufferFromImageFile: (NSString*) path: (int*) width: (int*) height;
- (BOOL) loadBitmap:(NSString *)filename intoIndex:(int)texIndex;

- (void) drawRect: (NSRect)rect;
- (void) reshape;
- (void) update;

@end
