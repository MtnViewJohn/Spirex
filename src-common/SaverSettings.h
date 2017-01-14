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

#ifndef INCLUDED_SAVERSETTINGS
#define INCLUDED_SAVERSETTINGS

#include <cstddef>

class SaverSettings
{
public:
  enum RenderMode {Original2D = 0, OriginalCounterRotate2D = 1,
    Original2DWith3DGeometry = 2, Curves = 4, Spheres = 5,
    PointModeSpheres = 3, Disks = 6, Cubes = 7, Squares = 8, Conics = 9,
    Toroids = 10, Teapots = 11, WrappedCubes = 12, Cylinders = 13};
  int				mVersion;
  int				mSize;
  unsigned int	mCurveCount;	// Spirex settings
  unsigned int	mCurveLength;
  unsigned int	mAngleChangeRate;
  unsigned int	mEvolutionRate;
  bool			mThickLines;
  bool			m3DRender;
  bool			mInColor;
  bool			mFixed;
  bool			mPoints;
  bool			mTriAxial;
  RenderMode		mMode;
  char*			mName;

  bool usesTexture(bool enableOpenGL = true) const;
  bool usesFixed(bool enableOpenGL = true) const;
  bool usesThickness(bool enableOpenGL = true) const;
  bool usesLength(bool enableOpenGL = true) const;
  bool usesPolygons() const;
  bool usesTriAxial(bool enableOpenGL = true) const;

private:
  size_t        mTextureLen;
  char* 			mTexturePtr;

public:
  enum {MaxCurveCount = 32, MaxCurveLength = 500};

  SaverSettings();
  ~SaverSettings();
  SaverSettings(const SaverSettings&);
  SaverSettings& operator=(const SaverSettings&);
  SaverSettings(	unsigned int CurveCount, unsigned int CurveLength,
    unsigned int mAngleChangeRate,  unsigned int EvolutionRate,
    bool ThickLines, bool InColor, bool Fixed, bool Points, bool TriAxial,
    RenderMode Mode);
  SaverSettings(	unsigned int CurveCount, unsigned int CurveLength,
    unsigned int mAngleChangeRate,  unsigned int EvolutionRate,
    bool ThickLines, bool InColor, bool Fixed, bool Points, bool TriAxial,
    RenderMode Mode, const char* TextureStr);
  SaverSettings(	unsigned int CurveCount, unsigned int CurveLength,
    unsigned int mAngleChangeRate,  unsigned int EvolutionRate,
    bool ThickLines, bool InColor, bool Fixed, bool Points, bool TriAxial,
    RenderMode Mode, const char* TexturePtr, unsigned int TextureLen);


  bool operator==(const SaverSettings& other) const;

  void InitDefaults();

  void			setTexture(const char* ptr, size_t len);
  void			setTexture(const char* str);
  void			clearTexture();
  void          qualify();

  const char*   getTexturePtr() const   { return mTexturePtr; }
  size_t        getTextureLen()	const	{ return mTextureLen; }
  const char*   getTextureStr() const;	// never returns null
  size_t        getTextureStrlen() const;
};

inline bool SaverSettings::usesTexture(bool enableOpenGL) const
{
  return m3DRender
    && (mMode > PointModeSpheres)
    && !mPoints
    && enableOpenGL;
};

inline bool SaverSettings::usesFixed(bool enableOpenGL) const
{
  return m3DRender
    && ((mMode >= Spheres) || (mMode == PointModeSpheres))
    && enableOpenGL;
}

inline bool SaverSettings::usesThickness(bool enableOpenGL) const
{
  return (usesFixed(enableOpenGL) && mFixed)
    || !m3DRender
    || (mMode <= Curves)
    || !enableOpenGL
    || mPoints;
};

inline bool SaverSettings::usesLength(bool enableOpenGL) const
{
  return !m3DRender
    || (mMode < PointModeSpheres)
    || (mMode == Curves)
    || !enableOpenGL;
};

inline bool SaverSettings::usesPolygons() const
{
  return m3DRender &&	(mMode > Original2DWith3DGeometry);
}

inline bool SaverSettings::usesTriAxial(bool enableOpenGL) const
{
  return m3DRender && (mMode >= PointModeSpheres) && enableOpenGL;
};

#endif // INCLUDED_SAVERSETTINGS
