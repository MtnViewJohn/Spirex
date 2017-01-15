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

#include "Spirex.h"
#include "SpirexGeom.h"
#include "SpirexGL.h"
#include "myWindows.h"
#include "SaverSettings.h"
#include "SaverSettingsWin32.h"
#include "RingBuffer.h"
#include "Point3D.h"
#include "Debug.h"
#include "myGl.h"
#include "texture.h"
#include <math.h>
#include <stdlib.h>
//#include <GdiPlus.h>

using namespace Gdiplus;


static const int TimeDelayPerCurve = 8;
static const double PI = 3.1415926535897932384626433832795;
static const double FullCircle = 2 * PI;
static const double MaxFixedSphereAngleRate = FullCircle / 20;
static const double MaxMovingSphereAngleRate = FullCircle / 35;
static const float  LineWidthBy2 = 0.01F;
static inline int mymin(int a, int b) { return a < b ? a : b; }

HANDLE Spirex::CurrentGfxOwner = NULL;

static DWORD Cvt2ARGB(const ColorRGB& c)
{
  return RGB(c.red * 255, c.green * 255, c.blue * 255) | 0xff000000;
}

void Spirex::NextStep()
{
  mGeom.NextStep();
}



void Spirex::Render(int rate)
{
	RECT rc;
  if (GetClipBox(mHdc, &rc) == NULLREGION) 
  	return;

    checkGLError();

    SpirexGL::Render(rate, mGeom);

    SwapBuffers(mHdc);
    mRenderCount++;

    checkGLError();
}

void Spirex::Clear()
{
  Debug(mHwnd, "Setting Spirex erase flag.");
  mEraseScreen = true;
}

Spirex::Spirex(const SaverSettings& settings, int sizex, int sizey, HWND hwnd)
: mScreenCenter(sizex / 2.0F, sizey / 2.0F, 0.0F),
  mScale(mymin(sizex, sizey) / 2.0F),
  mSettings(settings),
  mGeom(settings, mScale, mScreenCenter),
  mHdc(NULL),
  mHwnd(hwnd),
  mEraseScreen(true),
  mRenderCount(0),
  pixelsAreSetup(false)
{
	if (CurrentGfxOwner == NULL)
		CurrentGfxOwner = CreateMutex(NULL, FALSE, NULL);
	
  mRect.top = mRect.left = 0;
  mRect.right = sizex;
  mRect.bottom = sizey;
}

Spirex::~Spirex()
{
#if DEBUG
	HGLRC hGLRC = wglGetCurrentContext();
  if (hGLRC)
    Debug(mHwnd, "Deleting a Spirex object while OpenGL context is still alive.");
#endif
}

void Spirex::NewSaverSettings(const SaverSettings& settings)
{
  bool decreaseCurves = settings.mCurveCount < mSettings.mCurveCount;
  bool thicknessChange =	(settings.mThickLines  	 !=	mSettings.mThickLines);
  bool new3DEnv = 	lstrcmpi(settings.getTextureStr(), mSettings.getTextureStr());
  bool new3DMode = 	(settings.mMode != mSettings.mMode) ||
                    (settings.mPoints != mSettings.mPoints);

  if (new3DEnv)
  	DestroyGfx();
  	
 	if (mSettings.mName)
 		Debug(mHwnd, mSettings.mName);

  mSettings = settings;

  mGeom.NewSaverSettings(settings);

  if (new3DMode)
    SpirexGL::InitMode(mGeom);

  if (new3DEnv) 
  	SetupGfx();
}

void Spirex::SetupGfx()
{
  GetGfxMutex();
  mHdc = GetDC(mHwnd);
	HGLRC hGLRC = wglGetCurrentContext();
	setupPixelFormat();		// try to setup pixels early and unconditionally
  if (hGLRC == NULL) {
    glFlush();	// dummy call to force loading of opengl32.dll
    if (setupPixelFormat() || ((hGLRC = wglCreateContext(mHdc)) == NULL) ||
      !wglMakeCurrent(mHdc, hGLRC))
    {
      // we failed to setup OpenGL
      DebugWin();
      FatalExit(1);
    } else {
      SetupTexture();
      SpirexGL::InitGL(mGeom, mScreenCenter.x * 2.0F, mScreenCenter.y * 2.0F);
      SelectObject(mHdc, GetStockObject(SYSTEM_FONT));
      wglUseFontBitmaps(mHdc, 0, 256, 1000);
      glListBase(1000);
    }
  } else mEraseScreen = true;
	ReleaseMutex(CurrentGfxOwner);
}

void Spirex::DestroyGfx()
{
  GetGfxMutex();
	HGLRC hGLRC = wglGetCurrentContext();
	if (hGLRC != NULL) {
		if (mRenderCount & 1) {	// check if we are the primary or secondary buffer
	    SwapBuffers(mHdc);		// swap in primary buffer so GDI can draw
	    mRenderCount++;
		}
    glDeleteLists(1000, 256);
    HDC hdc = wglGetCurrentDC();
    if (!wglMakeCurrent(NULL, NULL))
      DebugWin();
		if (ReleaseDC(mHwnd, hdc) == 0)
	  	Debug(mHwnd, "ReleaseDC didn't release the DC.");  
    if (!wglDeleteContext(hGLRC))
      DebugWin();
    hGLRC = wglGetCurrentContext();
    if (hGLRC != NULL)
    	Debug("OpenGL did not go away.");
    else
    	Debug("OpenGL went away.");
  } else {
		if (ReleaseDC(mHwnd, mHdc) == 0)
	  	Debug(mHwnd, "ReleaseDC didn't release the DC.");  
  }
	ReleaseMutex(CurrentGfxOwner);
}

bool Spirex::GetGfxMutex()
// This mutex must be aquired before creating of destroying OpenGL contexts.
// The calling code must release the mutex after creating of destroying the
// context. Windows OpenGL requires threads to not create or destroy contexts
// at the same time.
{
	switch (WaitForSingleObject(CurrentGfxOwner, 20000)) {
		case WAIT_ABANDONED:
		case WAIT_OBJECT_0:
			return true;
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
			return false;
	}
	return false;
}

bool Spirex::setupPixelFormat()
{
  PIXELFORMATDESCRIPTOR pfd;
  int SelectedPixelFormat;
  BOOL retVal;
  
  if (pixelsAreSetup)
  	return false;		// only call once per window

  // set the pixel format for the DC
  ZeroMemory(&pfd, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 16;
  pfd.iLayerType = PFD_MAIN_PLANE;

  SelectedPixelFormat = ChoosePixelFormat(mHdc, &pfd);
  if (SelectedPixelFormat == 0)
    return true;

  retVal = DescribePixelFormat(mHdc, SelectedPixelFormat,





sizeof(PIXELFORMATDESCRIPTOR), &pfd);
  if (retVal == 0)
    return true;

  retVal = SetPixelFormat(mHdc, SelectedPixelFormat, &pfd);
  if (retVal != TRUE)
    return true;
  else
  	pixelsAreSetup = true;

  char buf[100];
  wsprintf(buf, "Flags = 0x%x color bits=%d\n", pfd.dwFlags, pfd.cColorBits);
  OutputDebugString(buf);

  if ((pfd.dwFlags & PFD_NEED_PALETTE) || (pfd.iPixelType == PFD_TYPE_COLORINDEX))
    return true;

  return false;
}

void Spirex::SetupTexture()
{
  if (mSettings.getTextureStrlen() != 0) {
    // Generate and load texture
    glGenTextures(1, mTexture);
    Texture *tex = new Texture(mSettings.getTextureStr(), Texture::OpenGLDIB);

    // Try to load texture. If loaded then bind it. Otherwise
    // erase the local copy of the texture path so that InitMode will
    // disable texturing.
    if (tex->Load(mTexture[0]))	{
      glBindTexture(GL_TEXTURE_2D, mTexture[0]);
    } else {
      mSettings.clearTexture();
    }

    delete tex;
  }
}


void Spirex::checkGLError()
{
  GLenum errCode;

  if (!DEBUG) return;

  while ((errCode = glGetError()) != 0) {
    static char buf[100];
    switch (errCode) {
      case GL_INVALID_ENUM:
        Debug("GL_INVALID_ENUM");
        break;
      case GL_INVALID_VALUE:
        Debug("GL_INVALID_VALUE");
        break;
      case GL_INVALID_OPERATION:
        Debug("GL_INVALID_OPERATION");
        goto endwhile;
      case GL_STACK_OVERFLOW:
        Debug("GL_STACK_OVERFLOW");
        break;
      case GL_STACK_UNDERFLOW:
        Debug("GL_STACK_UNDERFLOW");
        break;
      case GL_OUT_OF_MEMORY:
        Debug("GL_OUT_OF_MEMORY");
        break;
      default:
        wsprintf(buf, "Unknown GlError %d", errCode);
        Debug(buf);
    }
  }
endwhile:

  if (errCode) {
    FatalExit(0);
  }
}





