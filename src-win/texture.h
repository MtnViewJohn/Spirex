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

#ifndef INCLUDED_TEXTURE
#define INCLUDED_TEXTURE

#include "myWindows.h"
#include "myGl.h"
#include <memory>

namespace Gdiplus {
	class Bitmap;
};


class Texture 
{
public:
	enum TextureFormat { UnknownFormat = 0, WinBitmap = 1, OpenGLDIB = 2 };
	Texture(const char *name, TextureFormat fmt, int setWidth = 0, int setHeight = 0);
	~Texture() = default;
	bool Load(GLuint t_name);
	unsigned int GetWidth();
	unsigned int GetHeight();
	Gdiplus::Bitmap* GetBM();

private:
	TextureFormat mFormat;
	std::unique_ptr<Gdiplus::Bitmap>	mBM;			// data for GDI+ Bitmap texture
	std::unique_ptr<unsigned char[]>	mBits;			// data for OpenGLDIB texture
	int	mHeight, mWidth;
};



#endif //INCLUDED_TEXTURE
