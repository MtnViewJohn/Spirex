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

#ifndef INCLUDED_SPIREX
#define INCLUDED_SPIREX

#include "myWindows.h"
#include "SaverSettings.h"
#include "RingBuffer.h"
#include "Point3D.h"
#include "Color.h"
#include "SpirexGeom.h"

class Spirex
{
    Point3D	        mScreenCenter;
    RECT            mRect;
    float           mScale;
    SaverSettings   mSettings;
    SpirexGeom      mGeom;

    HDC             mHdc;
    HWND            mHwnd;
    bool            mEraseScreen;
    unsigned int    mRenderCount;



    GLuint mTexture[1]; /* Textures */
    void SetupTexture();

    void checkGLError();

    static HANDLE CurrentGfxOwner;
    bool GetGfxMutex();


    bool setupPixelFormat();
    bool pixelsAreSetup;

public:
    Spirex(const SaverSettings& mSettings, int sizex, int sizey, HWND hwnd);
    ~Spirex();
    void NewSaverSettings(const SaverSettings& settings);
    void NextStep();
    void Render(int rate);
    void SetupGfx();
    void DestroyGfx();
    void Clear();
private:
    Spirex(const Spirex&);
    Spirex& operator=(const Spirex&);
    // not implemented, can't be assigned or copied
};

#endif // INCLUDED_SPIREX