// Spirex Screensaver
// ---------------------
// Copyright (C) 2001, 2002, 2003 John Horigan
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

#include "texture.h"
#include "myWindows.h"
//#include <GdiPlus.h>
#include "myGl.h"
#include "Debug.h"
#include <stdlib.h>
#include "Configure.h"
#include "SaverSettingsWin32.h"
#include "CGdiPlusBitmap.h"

using namespace Gdiplus;

Texture::Texture(const char *fname, TextureFormat fmt, int setWidth, int setHeight)
  : mFormat(fmt),
    mHeight(0),
    mWidth(0)
{
    CGdiPlusBitmapResource bmr;
    CGdiPlusBitmap bmf;
    Bitmap* bm;
    if (*fname == '#') {
        if (!bmr.Load(fname, RT_RCDATA))
            return;
        bm = bmr.m_pBitmap;
    } else {
        WCHAR wpath[MAX_PATH];
        wsprintfW(wpath, L"%S", fname);
        if (!bmf.Load(wpath))
            return;
        bm = bmf.m_pBitmap;
    }

    int width = bm->GetWidth();
    int height = bm->GetHeight();

    if (setWidth && setHeight && (setWidth < width || setHeight < height)) {
        double scaleX = (double)setWidth / width;
        double scaleY = (double)setHeight / height;
        double scale = (scaleX < scaleY) ? scaleX : scaleY;
        int newWidth = width * scale;
        int newHeight = height * scale;
        int originX = (setWidth - newWidth) / 2;
        int originY = (setHeight - newHeight) / 2;
        std::unique_ptr<Bitmap> newBM = std::make_unique<Bitmap>(setWidth, setHeight, 
                                                                 PixelFormat24bppRGB);
        Graphics g(newBM.get());
        g.Clear(Color(0xfff4f3ee));
        Rect destRect(originX, originY, newWidth, newHeight);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.DrawImage(bm, destRect, 0, 0, width, height,
            UnitPixel, NULL, NULL, NULL);

        width = newWidth;
        height = newHeight;
        bmr.Empty(); bmf.Empty();
        bm = bmf.m_pBitmap = newBM.release();
    } else if (fmt == OpenGLDIB) {
        // resize to next power-of-two
        for (setWidth = 1; setWidth < width; setWidth <<= 1);
        for (setHeight = 1; setHeight < height; setHeight <<= 1);
        Rect destRect(0, 0, setWidth, setHeight);
        std::unique_ptr<Bitmap> newBM = std::make_unique<Bitmap>(setWidth, setHeight, 
                                                                 PixelFormat24bppRGB);
        Graphics g(newBM.get());
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.DrawImage(bm, destRect, 0, 0, width, height,
            UnitPixel, NULL, NULL, NULL);

        width = setWidth;
        height = setHeight;
        bmr.Empty(); bmf.Empty();
        bm = bmf.m_pBitmap = newBM.release();
    }


    switch (fmt) {
    case WinBitmap:
        mBM.reset(bm);
        bmf.m_pBitmap = bmr.m_pBitmap = 0;
        break;
    case OpenGLDIB: {
        BitmapData bmd;
        Rect r(0, 0, width, height);
        mBits = std::make_unique<BYTE[]>(width * height * 4);
        bmd.Width = width;
        bmd.Height = height;
        bmd.Stride = 4 * width;
        bmd.PixelFormat = PixelFormat32bppARGB;
        bmd.Scan0 = (void*)mBits.get();
        bmd.Reserved = NULL;
        if (bm->LockBits(&r, ImageLockModeRead | ImageLockModeUserInputBuf,
            PixelFormat32bppARGB, &bmd) != Ok) {
            DebugWin();
            mBits.reset();
            return;
        }
        // Convert GDI+ BGRA to OpenGL RGBA
        DWORD* bits = (DWORD*)mBits.get();
        for (int i = 0; i < height * width; i++) {
            DWORD pixel = bits[i];
            bits[i] = (pixel & 0xff00ff00) | ((pixel & 0xff) << 16) | ((pixel & 0xff0000) >> 16);
        }
        bm->UnlockBits(&bmd);
        break;
    }
    default:
        Debug("Illegal texture format.");
        return;
    }

    mWidth = width;
    mHeight = height;
}


bool Texture::Load(GLuint t_name)
{
    if (!mBits) {
        OutputDebugString("Sorry, can't read texture file...\n");
        return false;
    }

    // Make sure dimensions aren't too big.
    GLint MaxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxSize);
    if ((mWidth > MaxSize) || (mHeight > MaxSize))
        return false;

    glBindTexture(GL_TEXTURE_2D, t_name);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, mBits.get());
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, mWidth, mHeight, GL_RGBA,
        GL_UNSIGNED_BYTE, mBits.get());

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return true;
}

unsigned int Texture::GetWidth()
{
    return mWidth;
}

unsigned int Texture::GetHeight()
{
    return mHeight;
}

Bitmap* Texture::GetBM()
{
    return mBM.get();
}


