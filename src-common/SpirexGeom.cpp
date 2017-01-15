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


#include "SpirexGeom.h"
#include "RingBuffer.h"
#include "SaverSettings.h"
#include "Point3D.h"
//#include "Debug.h"
#include "myGl.h"
#include <math.h>
#include <stdlib.h>

static const int TimeDelayPerCurve = 8;
static const double PI = 3.1415926535897932384626433832795;
static const double FullCircle = 2 * PI;
static const double MaxFixedSphereAngleRate = FullCircle / 20;
static const double MaxMovingSphereAngleRate = FullCircle / 35;
static const float  LineWidthBy2 = 0.01F;
static const double	radiusMax = 1.0;
static const double radiusMax23 = 2.0 / 3.0;
static const int	TicksPerCount = 20;
static const int	Radius4Max = 3;


static inline int RandomBoolean(int p)
	// returns a random boolean that is true 1/p of the time
{
	return !(rand() % p);
}

static inline int RandomInteger(int lower, int upper )
// return a random integer in the interval [lower, upper)
{
	return rand() % (upper - lower) + lower;
}

static inline double RandomInterval(double magnitude)
// returns a random number in the interval [-magnitude, magnitude]
{
	return ((double)rand() / RAND_MAX) * 2.0 * magnitude - magnitude;
}

static inline double RandomBiasInterval(double magnitude, double bias)
// returns a random number in the interval [-magnitude, magnitude]
// 0 < bias < 1 biases away from zero, bias > 1 biases toward zero
{
  double res = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
	return ((res < 0) ? -pow(-res, bias) : pow(res, bias)) * magnitude;
}

static inline double RandomInterval(double lower, double upper)
// returns a random number in the interval [lower, upper]
{
	return ((double)rand() / RAND_MAX) * (upper - lower) + lower;
}


void SpirexGeom::newColorDest() {
	// new start = old destination
	mHueStart = (mHueFinish + 360) % 360;
	mSatStart = mSatFinish;

	// select new destination color
	mHueFinish = mHueStart + RandomInteger(-180, 180);
	mSatFinish = RandomInteger(0, 101);

	// determine number of steps to get to the target
	mColorCount = 0;
	mColorSteps = RandomInteger(20, 220);
}

template <class T>
static inline T interpolate(T start, T end, double fraction)
{
	return (T)(start + (end - start) * fraction);
};

void SpirexGeom::newGeometryDest() {
	// compute new start. This is an interpolation because a previewer might
	// trigger a new geometry destination before we reach the old destination.
	double fraction = (double)mGeometryCount / (double)mGeometrySteps;
	mFixedSphereAngle1RateStart = interpolate(mFixedSphereAngle1RateStart, mFixedSphereAngle1RateFinish, fraction);
	mFixedSphereAngle2RateStart = interpolate(mFixedSphereAngle2RateStart, mFixedSphereAngle2RateFinish, fraction);
	mMovingSphereAngle1RateStart = interpolate(mMovingSphereAngle1RateStart, mMovingSphereAngle1RateFinish, fraction);
	mMovingSphereAngle2RateStart = interpolate(mMovingSphereAngle2RateStart, mMovingSphereAngle2RateFinish, fraction);
	mFixedSphereRadiusStart = interpolate(mFixedSphereRadiusStart, mFixedSphereRadiusFinish, fraction);
	mMovingSphereRadiusStart = interpolate(mMovingSphereRadiusStart, mMovingSphereRadiusFinish, fraction);
  mMovingSphereRadius2Start = interpolate(mMovingSphereRadius2Start, mMovingSphereRadius2Finish, fraction);
  mMovingSphereRadius3Start = interpolate(mMovingSphereRadius3Start, mMovingSphereRadius3Finish, fraction);
  mMovingSphereRadius4Start = interpolate(mMovingSphereRadius4Start, mMovingSphereRadius4Finish, fraction);
	FixedSphereBiasStart = fmod(interpolate(FixedSphereBiasStart, FixedSphereBiasFinish, fraction), FullCircle);
	MovingSphereBiasStart = fmod(interpolate(MovingSphereBiasStart, MovingSphereBiasFinish, fraction), FullCircle);

	// compute new destination randomly
	double AngleRate = 	(mSettings.mAngleChangeRate * 0.01) * RandomInterval(MaxFixedSphereAngleRate);
	double Angle = RandomInterval(0.0, FullCircle);
	mFixedSphereAngle1RateFinish = AngleRate * cos(Angle);
	mFixedSphereAngle2RateFinish = AngleRate * sin(Angle);

	AngleRate =	(mSettings.mAngleChangeRate * 0.01) * RandomInterval(MaxMovingSphereAngleRate);
	Angle = RandomInterval(0.0, FullCircle);
	mMovingSphereAngle1RateFinish = AngleRate * cos(Angle);
	mMovingSphereAngle2RateFinish = AngleRate * sin(Angle);

	mFixedSphereRadiusFinish =  RandomInterval(radiusMax23);
	double outerRadiusMax = radiusMax - fabs(mFixedSphereRadiusFinish);
	mMovingSphereRadiusFinish =  RandomBiasInterval(outerRadiusMax, 0.9);
  mMovingSphereRadius2Finish =  RandomBiasInterval(outerRadiusMax, 0.9);
  mMovingSphereRadius3Finish =  RandomBiasInterval(outerRadiusMax, 0.9);
  if (RandomBoolean(5)) 
	  mMovingSphereRadius4Finish =  RandomInteger(-Radius4Max, Radius4Max);

	// determine number of steps to get to the destination
	mGeometryCount = 0;
	mGeometrySteps = RandomInteger(0, 15 * mSettings.mEvolutionRate) + 25;

	if (mSettings.mMode >= SaverSettings::Original2DWith3DGeometry) {
		FixedSphereBiasFinish = FixedSphereBiasStart;
		MovingSphereBiasFinish  = MovingSphereBiasStart;

		// Bias changes that happen too fast are kind of ugly so we only allow
		// them if the step count is in the upper half of the possible range.
		if ((mGeometrySteps > (7 * mSettings.mEvolutionRate)) && RandomBoolean(5)) {
			FixedSphereBiasFinish += RandomInterval(PI);
			MovingSphereBiasFinish  += RandomInterval(PI);
		}
	} else {
		FixedSphereBiasFinish = FixedSphereBiasStart < PI/2.0 ? 0.0 : PI;
		MovingSphereBiasFinish  = MovingSphereBiasStart  < PI/2.0 ? 0.0 : PI;

		// ditto
		if (mGeometrySteps > (7 * mSettings.mEvolutionRate)) {
			if (RandomBoolean(5)) FixedSphereBiasFinish *= -1.0;
			if (mSettings.mMode != SaverSettings::OriginalCounterRotate2D)
				MovingSphereBiasFinish = FixedSphereBiasFinish;
			else if (RandomBoolean(5))
				MovingSphereBiasFinish *= -1.0;
		}
	}


//	Debug("New geometry destination");
}


void SpirexGeom::nextColor() {
	mColorCount++;
	double fraction = (double)mColorCount / (double)mColorSteps;
	float nextHue, nextSat;

	if (mSettings.mInColor) {
		nextHue = fmod(interpolate(mHueStart, mHueFinish, fraction) + 360.0F, 360.0F);
		nextSat = interpolate(mSatStart, mSatFinish, fraction);
	} else {
		nextHue = nextSat = 0;
	}
	ColorHSB next(nextHue, nextSat, 1.0F);
	mColorBuffer.add(next);

	if (mColorCount >= mColorSteps) newColorDest();
}

void SpirexGeom::nextGeometry() {
	mGeometryCount++;
	double fraction = (double)mGeometryCount / (double)mGeometrySteps;

	mFixedSphereAngle1 = fmod(interpolate(mFixedSphereAngle1RateStart, mFixedSphereAngle1RateFinish, fraction) +
		mFixedSphereAngle1, FullCircle);
	mFixedSphereAngle2 = fmod(interpolate(mFixedSphereAngle2RateStart, mFixedSphereAngle2RateFinish, fraction) +
		mFixedSphereAngle2, FullCircle);
	mMovingSphereAngle1 = fmod(interpolate(mMovingSphereAngle1RateStart, mMovingSphereAngle1RateFinish, fraction) +
		mMovingSphereAngle1, FullCircle);
	mMovingSphereAngle2 = fmod(interpolate(mMovingSphereAngle2RateStart, mMovingSphereAngle2RateFinish, fraction) +
		mMovingSphereAngle2, FullCircle);

	FixedSphereBias = interpolate(FixedSphereBiasStart, FixedSphereBiasFinish, fraction);
	MovingSphereBias = interpolate(MovingSphereBiasStart, MovingSphereBiasFinish, fraction);

	mFixedSphereAngle1Buffer.add(mFixedSphereAngle1);
	mFixedSphereAngle2Buffer.add(mFixedSphereAngle2);
	mMovingSphereAngle1Buffer.add(mMovingSphereAngle1);
	mMovingSphereAngle2Buffer.add(mMovingSphereAngle2);
	mFixedSphereRadiusBuffer.add(interpolate(mFixedSphereRadiusStart, mFixedSphereRadiusFinish, fraction));
	mMovingSphereRadiusBuffer.add(interpolate(mMovingSphereRadiusStart, mMovingSphereRadiusFinish, fraction));
  mMovingSphereRadius2Buffer.add(interpolate(mMovingSphereRadius2Start, mMovingSphereRadius2Finish, fraction));
  mMovingSphereRadius3Buffer.add(interpolate(mMovingSphereRadius3Start, mMovingSphereRadius3Finish, fraction));
  mMovingSphereRadius4Buffer.add(interpolate(mMovingSphereRadius4Start, mMovingSphereRadius4Finish, fraction));

	if (mGeometryCount >= mGeometrySteps) newGeometryDest();
}

void SpirexGeom::NextStep()
{
	if (mTargetLength >= mSettings.mCurveLength) {
		nextColor();
		nextGeometry();

		if (mCurrentCurveSlew != mTargetCurveSlew) {
			if (mCurrentCurveSlew < mTargetCurveSlew)
				mCurrentCurveSlew++;
			else
				mCurrentCurveSlew--;

			AngleDelay = FullCircle / ((double)mCurrentCurveSlew / (double)TicksPerCount);
			mSettings.mCurveCount = mCurrentCurveSlew / TicksPerCount;
		}

		for (int curve = 0; curve < SaverSettings::MaxCurveCount; curve++) {
			int timeDelay = curve * TimeDelayPerCurve;
			plotPoint(mFixedSphereAngle1Buffer.get(timeDelay) +
			  curve * AngleDelay * cos(FixedSphereBias),
			  mFixedSphereAngle2Buffer.get(timeDelay) +
			  curve * AngleDelay * sin(FixedSphereBias),
			  mFixedSphereRadiusBuffer.get(timeDelay),
			  mMovingSphereAngle1Buffer.get(timeDelay) +
			  curve * AngleDelay * cos(MovingSphereBias),
			  mMovingSphereAngle2Buffer.get(timeDelay) +
			  curve * AngleDelay * sin(MovingSphereBias),
			  mMovingSphereRadiusBuffer.get(timeDelay),
	      mMovingSphereRadius2Buffer.get(timeDelay),
	      mMovingSphereRadius3Buffer.get(timeDelay),
	      mMovingSphereRadius4Buffer.get(timeDelay),
			  mColorBuffer.get(timeDelay), curve);
		}
	}

	if (mTargetLength != mSettings.mCurveLength) {
		if (mTargetLength < mSettings.mCurveLength)
			mSettings.mCurveLength--;
		else
			mSettings.mCurveLength++;

		for (int curve = 0; curve < SaverSettings::MaxCurveCount; curve++) {
			mPointRingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
			mWidthRingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
			mNormalRingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
			mColorRingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
		}

	}

}


void SpirexGeom::plotPoint(double FixedSphereAngle1, double FixedSphereAngle2, double FixedSphereRadius,
  double MovingSphereAngle1,  double MovingSphereAngle2,  double MovingSphereRadius,
  double MovingSphereRadius2, double MovingSphereRadius3, double MovingSphereRadius4, 
  ColorHSB colorHSB, int curve)
{
	Point3D newHead;
	Point3D here, normal;

	// compute next point
	if (mSettings.mMode > SaverSettings::OriginalCounterRotate2D) {
		double irad = sin(FixedSphereAngle1) * FixedSphereRadius;
		double orad = sin(MovingSphereAngle1);

		normal.z = cos(MovingSphereAngle1);
		normal.x = cos(MovingSphereAngle2) * orad;
		normal.y = sin(MovingSphereAngle2) * orad;

		here.z = cos(FixedSphereAngle1) * FixedSphereRadius;
		here.x = cos(FixedSphereAngle2) * irad;
		here.y = sin(FixedSphereAngle2) * irad;
		mMovingSphereCenter[curve] = here;
		here = here + normal * MovingSphereRadius;

		if (!mSettings.usesPolygons())
		{
			// project the point onto the screen
			newHead.x = (here.x * 1.5 * radiusMax) / (2.0 * radiusMax - here.z);
			newHead.y = (here.y * 1.5 * radiusMax) / (2.0 * radiusMax - here.z);
		}
	} else {
		newHead.x = FixedSphereRadius * cos(FixedSphereAngle1) +
						  MovingSphereRadius  * cos(MovingSphereAngle1);
		newHead.y = FixedSphereRadius * sin(FixedSphereAngle1) +
						  MovingSphereRadius  * sin(MovingSphereAngle1);
	}

	// in 2-D/3-D mode do cheesy pseudo-3D using brightness
	if (mSettings.mMode == SaverSettings::Original2DWith3DGeometry) {
		colorHSB.bright = (here.z / radiusMax + 1.0F) * 0.375F + 0.25F;
		if (colorHSB.bright < 0.0F) colorHSB.bright = 0.0F;
		if (colorHSB.bright > 1.0F) colorHSB.bright = 1.0F;
	}

	// construct an RBG color from the HSB color
	ColorRGB colorRGB(colorHSB);

	// store point
	if (mSettings.usesPolygons()) {
		Point3D previousPoint = mPointRingBufferArray[curve].get(0);
		Point3D width = normal * (here - previousPoint);
		float scaler = (LineWidthBy2 * (mSettings.mThickLines ? 2.0F : 1.0F)) /
			width.length();

		if (scaler < 1000000.0F) {
			width = width * scaler;
		} else {
			width = mWidthRingBufferArray[curve].get(0);
		}

		mPointRingBufferArray[curve].add(here);
		mWidthRingBufferArray[curve].add(width);
		mNormalRingBufferArray[curve].add(normal);
		mColorRingBufferArray[curve].add(colorRGB);
    mRadius1RingBufferArray[curve].add(MovingSphereRadius);
    mRadius2RingBufferArray[curve].add(MovingSphereRadius2);
    mRadius3RingBufferArray[curve].add(MovingSphereRadius3);
    mRadius4RingBufferArray[curve].add(MovingSphereRadius4);

	} else {
		mPointRingBufferArray[curve].add(newHead);
		mColorRingBufferArray[curve].add(colorRGB);
	}
}


SpirexGeom::SpirexGeom(const SaverSettings& settings, float scale, Point3D center)
:	mScreenCenter(center),
	mScale(scale),
	mSettings(settings)
{
	init();
}

SpirexGeom::SpirexGeom(const SaverSettings& settings)
:	mScale(1.0f),	// use default constructor for mScreenCenter
	mSettings(settings)
{
    mSettings.qualify();
	init();
}

SpirexGeom::~SpirexGeom()
{
}

void SpirexGeom::init()
{
	int BufferSize = SaverSettings::MaxCurveCount * TimeDelayPerCurve;
	AngleDelay = FullCircle / mSettings.mCurveCount;
	mCurrentCurveSlew = mTargetCurveSlew = mSettings.mCurveCount * TicksPerCount;
	mTargetLength = mSettings.mCurveLength;

	ColorHSB initColor(0.0F, mSettings.mInColor ? 100.0F : 0.0F, 1.0F);
	mColorBuffer.resetBuffer(BufferSize, initColor);
	mHueFinish = (int)initColor.hue; mSatFinish = (int)initColor.sat;
	newColorDest();

	mFixedSphereRadiusBuffer.resetBuffer(BufferSize, 0.0);
	mMovingSphereRadiusBuffer.resetBuffer(BufferSize, 0.0);
	mMovingSphereRadius2Buffer.resetBuffer(BufferSize, 0.0);
	mMovingSphereRadius3Buffer.resetBuffer(BufferSize, 0.0);
	mMovingSphereRadius4Buffer.resetBuffer(BufferSize, 0.0);
	mFixedSphereAngle1Buffer.resetBuffer(BufferSize, 0.0);
	mFixedSphereAngle2Buffer.resetBuffer(BufferSize, 0.0);
	mMovingSphereAngle1Buffer.resetBuffer(BufferSize, 0.0);
	mMovingSphereAngle2Buffer.resetBuffer(BufferSize, 0.0);

	mFixedSphereRadiusFinish = mMovingSphereRadiusFinish = 0.0;
	mFixedSphereAngle1 = mMovingSphereAngle1 = mFixedSphereAngle1RateFinish =
		mMovingSphereAngle1RateFinish = mFixedSphereAngle2 = mMovingSphereAngle2 =
		mFixedSphereAngle2RateFinish = mMovingSphereAngle2RateFinish = 0.0;

	mFixedSphereRadiusStart = mMovingSphereRadiusStart = 0.0;
  mMovingSphereRadius2Start = mMovingSphereRadius3Start = 0.0;
  mMovingSphereRadius4Start = 0.0;
	mFixedSphereAngle1RateStart = mMovingSphereAngle1RateStart =
	mFixedSphereAngle2RateStart = mMovingSphereAngle2RateStart = 0.0;

	FixedSphereBias = FixedSphereBiasFinish = FixedSphereBiasStart =
		MovingSphereBiasStart = 0.0;
	MovingSphereBias = MovingSphereBiasFinish = (mSettings.mMode == SaverSettings::OriginalCounterRotate2D) ? PI : 0.0;
	mGeometryCount = 1;
	mGeometrySteps = 1;
	newGeometryDest();

	Point3D initXYZ;
	Point3D initNormal(0.0F, 0.0F, 1.0F);
	Point3D initWidth(LineWidthBy2, 0.0F, 0.0F);
	ColorRGB initRGB = initColor;
	for (unsigned int curve = 0; curve < SaverSettings::MaxCurveCount; curve++ ) {
		mPointRingBufferArray[curve].resetBuffer(SaverSettings::MaxCurveLength + 1, initXYZ);
		mWidthRingBufferArray[curve].resetBuffer(SaverSettings::MaxCurveLength + 1, initWidth);
    mRadius1RingBufferArray[curve].resetBuffer(SaverSettings::MaxCurveLength + 1, 0.0);
    mRadius2RingBufferArray[curve].resetBuffer(SaverSettings::MaxCurveLength + 1, 0.0);
    mRadius3RingBufferArray[curve].resetBuffer(SaverSettings::MaxCurveLength + 1, 0.0);
    mRadius4RingBufferArray[curve].resetBuffer(SaverSettings::MaxCurveLength + 1, 0.0);
		mNormalRingBufferArray[curve].resetBuffer(SaverSettings::MaxCurveLength + 1, initNormal);
		mColorRingBufferArray[curve].resetBuffer(SaverSettings::MaxCurveLength + 1, initRGB);
		mPointRingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
		mWidthRingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
    mRadius1RingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
    mRadius2RingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
    mRadius3RingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
    mRadius4RingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
		mNormalRingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
		mColorRingBufferArray[curve].setSize(mSettings.mCurveLength + 1);
	}
}

void SpirexGeom::NewSaverSettings(const SaverSettings& settings)
{
	int savedCurveCount = mSettings.mCurveCount;
	int savedCurveLength = mSettings.mCurveLength;

	bool newDest = 		(settings.mAngleChangeRate  != mSettings.mAngleChangeRate) ||
						(settings.mEvolutionRate 	!= mSettings.mEvolutionRate);
	bool is3Dto3D = 	settings.usesPolygons() && mSettings.usesPolygons();
	bool newMode = 		(settings.mMode 			!= mSettings.mMode) &&
						(!is3Dto3D
							|| (settings.mCurveCount != mSettings.mCurveCount)
							|| (settings.mCurveLength != mSettings.mCurveLength));
	// initialize if changing from a 3D mode to a 2D mode or a 2D mode to a 2D mode
	// or if changing from a 3D mode to a 3D mode and changing curve count or length

	mSettings = settings;
    mSettings.qualify();
	mSettings.mCurveCount = savedCurveCount;
	mSettings.mCurveLength = savedCurveLength;

	mTargetCurveSlew = settings.mCurveCount * TicksPerCount;
	mTargetLength = settings.mCurveLength;

	if (newMode) {
		mSettings.mCurveCount = settings.mCurveCount;
		mSettings.mCurveLength = settings.mCurveLength;
		init();
	} else if (newDest)
		newGeometryDest();
}

