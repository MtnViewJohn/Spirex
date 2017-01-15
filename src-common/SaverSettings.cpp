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

#include "SaverSettings.h"
//#include "Debug.h"



void SaverSettings::InitDefaults()
{
    mVersion = 0;
    mSize = sizeof(SaverSettings);

    mCurveCount = 16;
    mCurveLength = 50;
    mAngleChangeRate = 50;
    mEvolutionRate = 50;
    mThickLines = false;
    mInColor = true;
    mFixed = false;
    mPoints = false;
    mTriAxial = false;
    mMode = Original2D;
    mName.clear();

    clearTexture();
}

SaverSettings::SaverSettings()
{
    InitDefaults();
}

SaverSettings::SaverSettings(
    unsigned int CurveCount, unsigned int CurveLength,
    unsigned int AngleChangeRate, unsigned int EvolutionRate,
    bool  ThickLines, bool InColor, bool Fixed, bool Points, bool TriAxial,
    RenderMode Mode)
  : mVersion(0),
    mSize(sizeof(SaverSettings)),
    mCurveCount(CurveCount),
    mCurveLength(CurveLength),
    mAngleChangeRate(AngleChangeRate),
    mEvolutionRate(EvolutionRate),
    mThickLines(ThickLines),
    mInColor(InColor),
    mFixed(Fixed),
    mPoints(Points),
    mTriAxial(TriAxial),
    mMode(Mode)
{
}

SaverSettings::SaverSettings(
    unsigned int CurveCount, unsigned int CurveLength,
    unsigned int AngleChangeRate, unsigned int EvolutionRate,
    bool  ThickLines, bool InColor, bool Fixed, bool Points, bool TriAxial,
    RenderMode Mode, const char* TextureStr)
  : mVersion(0),
    mSize(sizeof(SaverSettings)),
    mCurveCount(CurveCount),
    mCurveLength(CurveLength),
    mAngleChangeRate(AngleChangeRate),
    mEvolutionRate(EvolutionRate),
    mThickLines(ThickLines),
    mInColor(InColor),
    mFixed(Fixed),
    mPoints(Points),
    mTriAxial(TriAxial),
    mMode(Mode)
{
    setTexture(TextureStr);
}

SaverSettings::SaverSettings(
    unsigned int CurveCount, unsigned int CurveLength,
    unsigned int AngleChangeRate, unsigned int EvolutionRate,
    bool  ThickLines, bool InColor, bool Fixed, bool Points, bool TriAxial,
    RenderMode Mode, const char* TexturePtr, unsigned int TextureLen)
  : mVersion(0),
    mSize(sizeof(SaverSettings)),
    mCurveCount(CurveCount),
    mCurveLength(CurveLength),
    mAngleChangeRate(AngleChangeRate),
    mEvolutionRate(EvolutionRate),
    mThickLines(ThickLines),
    mInColor(InColor),
    mFixed(Fixed),
    mPoints(Points),
    mTriAxial(TriAxial),
    mMode(Mode)
{
    setTexture(TexturePtr, TextureLen);
}


bool SaverSettings::operator==(const SaverSettings& other) const
{
    return mVersion == other.mVersion
        && mSize == other.mSize
        && mCurveCount == other.mCurveCount
        && mCurveLength == other.mCurveLength
        && mAngleChangeRate == other.mAngleChangeRate
        && mEvolutionRate == other.mEvolutionRate
        && mThickLines == other.mThickLines
        && mInColor == other.mInColor
        && mFixed == other.mFixed
        && mPoints == other.mPoints
        && mTriAxial == other.mTriAxial
        && mMode == other.mMode
        && mTexturePath == other.mTexturePath;
}

void SaverSettings::clearTexture()
{
    mTexturePath.clear();
}

void SaverSettings::setTexture(const char* TexturePtr, size_t TextureLen)
{
    if (!TexturePtr || TextureLen == 0)
        mTexturePath.clear();
    else
        mTexturePath.assign(TexturePtr, TextureLen);
}

void SaverSettings::setTexture(const char* TextureStr)
{
    if (TextureStr)
        mTexturePath.assign(TextureStr);
    else
        mTexturePath.clear();
}

const char* SaverSettings::getTextureStr() const
{
    return mTexturePath.c_str();
}

size_t SaverSettings::getTextureStrlen() const
{
    return mTexturePath.length();;
}

void SaverSettings::qualify()
{
    if (!usesTexture()) clearTexture();
    mFixed = mFixed && usesFixed();
    mTriAxial = mTriAxial && usesTriAxial();
    mThickLines = mThickLines && usesThickness();
}


