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
#define _CRT_SECURE_NO_DEPRECATE 1
//#include "Debug.h"
#include <string.h>



void SaverSettings::InitDefaults()
{
	mVersion = 0;
	mSize = sizeof(SaverSettings);

	mCurveCount = 16;
	mCurveLength = 50;
	mAngleChangeRate = 50;
	mEvolutionRate = 50;
	mThickLines = false;
	m3DRender = true;
	mInColor = true;
	mFixed = false;
  mPoints = false;
  mTriAxial = false;
	mMode = Original2D;
	mName = 0;

	clearTexture();
}

SaverSettings::SaverSettings()
	: mTextureLen(0), mTexturePtr(0)
{
	InitDefaults();
}

SaverSettings::~SaverSettings()
{
	clearTexture();
	delete[] mName;
}

SaverSettings::SaverSettings(const SaverSettings& oldSettings) :
    mVersion(oldSettings.mVersion),
    mSize(sizeof(SaverSettings)),
    mCurveCount(oldSettings.mCurveCount),
    mCurveLength(oldSettings.mCurveLength),
    mAngleChangeRate(oldSettings.mAngleChangeRate),
    mEvolutionRate(oldSettings.mEvolutionRate),
    mThickLines(oldSettings.mThickLines),
    m3DRender(oldSettings.m3DRender),
    mInColor(oldSettings.mInColor),
    mFixed(oldSettings.mFixed),
    mPoints(oldSettings.mPoints),
    mTriAxial(oldSettings.mTriAxial),
    mMode(oldSettings.mMode),
    mName(0),
    mTextureLen(0),
    mTexturePtr(0)
{
	setTexture(oldSettings.mTexturePtr, oldSettings.mTextureLen);
	if (oldSettings.mName) {
		mName = new char[strlen(oldSettings.mName) + 1];
		strcpy(mName, oldSettings.mName);
	}
}

SaverSettings& SaverSettings::operator=(const SaverSettings& oldSettings)
{
	if (this == &oldSettings) return *this;

    mVersion = oldSettings.mVersion;
    mSize = sizeof(SaverSettings);
    mCurveCount = oldSettings.mCurveCount;
    mCurveLength = oldSettings.mCurveLength;
    mAngleChangeRate = oldSettings.mAngleChangeRate;
    mEvolutionRate = oldSettings.mEvolutionRate;
    mThickLines = oldSettings.mThickLines;
    m3DRender = oldSettings.m3DRender;
    mInColor = oldSettings.mInColor;
    mFixed = oldSettings.mFixed;
    mPoints = oldSettings.mPoints;
    mTriAxial = oldSettings.mTriAxial;
    mMode = oldSettings.mMode;

	if (oldSettings.mName) {
		mName = new char[strlen(oldSettings.mName) + 1];
		strcpy(mName, oldSettings.mName);
	} else {
		delete[] mName;
		mName = 0;
	}
	
	setTexture(oldSettings.mTexturePtr, oldSettings.mTextureLen);

	return *this;
}

SaverSettings::SaverSettings(
	unsigned int CurveCount,	 	unsigned int CurveLength,
	unsigned int AngleChangeRate,	unsigned int EvolutionRate,
	bool  ThickLines, bool InColor, bool Fixed, bool Points, bool TriAxial,
  RenderMode Mode) :
    mVersion(0),
    mSize(sizeof(SaverSettings)),
    mCurveCount(CurveCount),
    mCurveLength(CurveLength),
    mAngleChangeRate(AngleChangeRate),
    mEvolutionRate(EvolutionRate),
    mThickLines(ThickLines),
    m3DRender(true),
    mInColor(InColor),
    mFixed(Fixed),
    mPoints(Points),
    mTriAxial(TriAxial),
    mMode(Mode),
    mName(0),
    mTextureLen(0),
    mTexturePtr(0)
{
}

SaverSettings::SaverSettings(
	unsigned int CurveCount,	 	unsigned int CurveLength,
	unsigned int AngleChangeRate,	unsigned int EvolutionRate,
	bool  ThickLines, bool InColor, bool Fixed, bool Points, bool TriAxial,
  RenderMode Mode, const char* TextureStr) :
	mVersion(0),
	mSize(sizeof(SaverSettings)),
	mCurveCount(CurveCount),
	mCurveLength(CurveLength),
	mAngleChangeRate(AngleChangeRate),
	mEvolutionRate(EvolutionRate),
	mThickLines(ThickLines),
	m3DRender(true),
	mInColor(InColor),
	mFixed(Fixed),
  mPoints(Points),
  mTriAxial(TriAxial),
	mMode(Mode),
	mName(0),
	mTextureLen(0),
	mTexturePtr(0)
{
	setTexture(TextureStr);
}

SaverSettings::SaverSettings(
	unsigned int CurveCount,	 	unsigned int CurveLength,
	unsigned int AngleChangeRate,	unsigned int EvolutionRate,
	bool  ThickLines, bool InColor, bool Fixed, bool Points, bool TriAxial,
  RenderMode Mode, const char* TexturePtr, unsigned int TextureLen) :
	mVersion(0),
	mSize(sizeof(SaverSettings)),
	mCurveCount(CurveCount),
	mCurveLength(CurveLength),
	mAngleChangeRate(AngleChangeRate),
	mEvolutionRate(EvolutionRate),
	mThickLines(ThickLines),
	m3DRender(true),
	mInColor(InColor),
	mFixed(Fixed),
    mPoints(Points),
    mTriAxial(TriAxial),
	mMode(Mode),
	mName(0),
	mTextureLen(0),
	mTexturePtr(0)
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
        && m3DRender == other.m3DRender
        && mInColor == other.mInColor
        && mFixed == other.mFixed
        && mPoints == other.mPoints
        && mTriAxial == other.mTriAxial
        && mMode == other.mMode
        && mTextureLen == other.mTextureLen
        && (((mTexturePtr == 0) && (other.mTexturePtr == 0))
            || ((mTexturePtr != 0) && (other.mTexturePtr != 0)
                && !memcmp(mTexturePtr, other.mTexturePtr, mTextureLen)));
}

void SaverSettings::clearTexture()
{
	delete[] mTexturePtr;
	mTextureLen = 0;
	mTexturePtr = 0;
}

void SaverSettings::setTexture(const char* TexturePtr, size_t TextureLen)
{
	if (TextureLen <= 0  ||  TextureLen != mTextureLen) {
		clearTexture();
	}
	if (TextureLen > 0  &&  TexturePtr) {
		if (!mTexturePtr) {
			mTextureLen = TextureLen;
			mTexturePtr = new char[mTextureLen];
		}
		memcpy(mTexturePtr, TexturePtr, mTextureLen);
	}
}

void SaverSettings::setTexture(const char* TextureStr)
{
	setTexture(TextureStr, TextureStr ? (strlen(TextureStr)+1) : 0);
}

const char* SaverSettings::getTextureStr() const
{
	return mTexturePtr ? mTexturePtr : "";
}

size_t SaverSettings::getTextureStrlen() const
{
	return mTexturePtr ? strlen(mTexturePtr) : 0;
}

void SaverSettings::qualify()
{
    if (!usesTexture(true)) clearTexture();
    mFixed = mFixed && usesFixed(true);
    mTriAxial = mTriAxial && usesTriAxial(true);
    mThickLines = mThickLines && usesThickness(true);
}


