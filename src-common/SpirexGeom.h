// Spirex Screensaver
// ---------------------
// Copyright (C) 2001-2017 John Horigan
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

#ifndef INCLUDED_SPIREXGEOM
#define INCLUDED_SPIREXGEOM

#include "SaverSettings.h"
#include "RingBuffer.h"
#include "Point3D.h"
#include "Color.h"

typedef RingBuffer<double>      DoubleRingBuffer;
typedef RingBuffer<ColorRGB>    RBGColorRingBuffer;
typedef RingBuffer<ColorHSB>    HSBColorRingBuffer;
typedef RingBuffer<Point3D>     Point3DRingBuffer;

class SpirexGeom
{
private:
    // These are some geometry constants precomputed for convenience
    double AngleDelay;
    
    // Current geometry data. The basic geometry of Spirex3D is that of a
    // curve traced by a point on a rotating, moving sphere. The center of this
    // sphere is a point on a rotating fixed sphere.
    double mFixedSphereAngle1;
    double mFixedSphereAngle2;
    double mMovingSphereAngle1;
    double mMovingSphereAngle2;
    
    // The AngleDelay biases the curves towards one direction of rotation.
    // How boring! We want random biasing and we want the center and outer
    // circles to have independant biasing.
    double FixedSphereBias, FixedSphereBiasStart, FixedSphereBiasFinish;
    double MovingSphereBias, MovingSphereBiasStart, MovingSphereBiasFinish;
    
    // Geometry interpolation data, current radii and rates of change of angles
    // slew from a start to a finish position linearly over a random number of
    // steps.
    double mFixedSphereAngle1RateStart,    mFixedSphereAngle1RateFinish;
    double mFixedSphereAngle2RateStart,    mFixedSphereAngle2RateFinish;
    double mMovingSphereAngle1RateStart,   mMovingSphereAngle1RateFinish;
    double mMovingSphereAngle2RateStart,   mMovingSphereAngle2RateFinish;
    double mFixedSphereRadiusStart,        mFixedSphereRadiusFinish;
    double mMovingSphereRadiusStart,       mMovingSphereRadiusFinish;
    double mMovingSphereRadius2Start,      mMovingSphereRadius2Finish;
    double mMovingSphereRadius3Start,      mMovingSphereRadius3Finish;
    double mMovingSphereRadius4Start,      mMovingSphereRadius4Finish;
    
    unsigned int    mGeometryCount, mGeometrySteps;
    
    // Geometry buffers
    DoubleRingBuffer mFixedSphereAngle1Buffer,  mMovingSphereAngle1Buffer;
    DoubleRingBuffer mFixedSphereAngle2Buffer,  mMovingSphereAngle2Buffer;
    DoubleRingBuffer mFixedSphereRadiusBuffer,  mMovingSphereRadiusBuffer;
    DoubleRingBuffer mMovingSphereRadius2Buffer,  mMovingSphereRadius3Buffer;
    DoubleRingBuffer mMovingSphereRadius4Buffer;
    
    // Current color data. The current color slews linearly from a start to a
    // finish position linearly over a random number of steps.
    int mHueStart,  mHueFinish;
    int mSatStart,  mSatFinish;
    
    int mColorCount, mColorSteps;
    
    HSBColorRingBuffer mColorBuffer;
    
    // variables used for slewing curve counts and lengths
    int mCurrentCurveSlew, mTargetCurveSlew;
    unsigned int mTargetLength;
    
    // scaling parameters for 2D render mode
    Point3D         mScreenCenter;
    float           mScale;
    
public:         // remove when splitting spirex classes
    void init();
private:        // remove when splitting spirex classes
    
    void nextColor();
    void newColorDest();
    
    void nextGeometry();
    void newGeometryDest();
    
    void plotPoint(double FixedSphereAngle1,    double FixedSphereAngle2,   double FixedSphereRadius,
                   double MovingSphereAngle1,   double MovingSphereAngle2,  double MovingSphereRadius,
                   double MovingSphereRadius2,  double MovingSphereRadius3, double MovingSphereRadius4,
                   ColorHSB color, int curve);
    
    
public:
    SaverSettings mSettings;
    
    Point3DRingBuffer mPointRingBufferArray[SaverSettings::MaxCurveCount];      // stored drawn vertices
    Point3DRingBuffer mNormalRingBufferArray[SaverSettings::MaxCurveCount];     // stored drawn vertices
    Point3DRingBuffer mWidthRingBufferArray[SaverSettings::MaxCurveCount];      // stored drawn vertices
    DoubleRingBuffer  mRadius1RingBufferArray[SaverSettings::MaxCurveCount];    // stored radii
    DoubleRingBuffer  mRadius2RingBufferArray[SaverSettings::MaxCurveCount];    // stored radii
    DoubleRingBuffer  mRadius3RingBufferArray[SaverSettings::MaxCurveCount];    // stored radii
    DoubleRingBuffer  mRadius4RingBufferArray[SaverSettings::MaxCurveCount];    // stored radii
    RBGColorRingBuffer mColorRingBufferArray[SaverSettings::MaxCurveCount];     // stored vertex colors
    
    Point3D mMovingSphereCenter[SaverSettings::MaxCurveCount];
    
    SpirexGeom(const SaverSettings& mSettings, float mScale,
               Point3D mScreenCenter);
    SpirexGeom(const SaverSettings& mSettings);
    ~SpirexGeom();
    void NewSaverSettings(const SaverSettings& settings);
    void NextStep();
private:
    SpirexGeom(const SpirexGeom&);
    SpirexGeom& operator=(const SpirexGeom&);
    // not implemented, can't be assigned or copied
};

#endif  //INCLUDED_SPIREXGEOM
