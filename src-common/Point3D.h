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

#ifndef INCLUDED_POINT3D
#define INCLUDED_POINT3D

#include "myGl.h"
#include <math.h>

class Point3D
{
public:
    GLfloat x, y, z;

    Point3D operator+(Point3D b) { 
        return Point3D(x + b.x, y + b.y, z + b.z); 
    }
    
    Point3D operator-(Point3D b) { 
        return Point3D(x - b.x, y - b.y, z - b.z); 
    }
    
    Point3D operator*(Point3D b) { 
        return Point3D( y * b.z - z * b.y,
                        z * b.x - x * b.z, 
                        x * b.y - y * b.x); 
    }
    
    Point3D operator*(float b) { 
        return Point3D(x * b, y * b, z * b); 
    }
    
    Point3D operator*(double b) { 
        return Point3D((float)(x * b), (float)(y * b), (float)(z * b)); 
    }
    
    Point3D operator/(float b) { 
        return Point3D(x / b, y / b, z / b); 
    }
    
    bool operator==(Point3D b) { 
        return (x == b.x) && (y == b.y) && (z == b.z);
    }
    
    

    Point3D(float x1, float y1, float z1):x(x1), y(y1), z(z1)
    { 
    }

    Point3D():x(0.0F), y(0.0F), z(0.0F)
    {
    }
    
    float length() {
        return sqrt(x * x + y * y + z * z);
    }
    
    void normalize() {
        float len = length();
        if (len < 1e-7) {
            x = y = 0.0f;   // anything will do
            z = 1.0f;
        } else {
            x = x / len;
            y = y / len;
            z = z / len;
        }
    }
    
    void set(float x1, float y1, float z1) { 
        x = x1; y = y1; z = z1; 
    }
    
    void setInt() {
        x = (int)x; y = (int)y; z = (int)z;
    }
    
};

    

#endif // INCLUDED_POINT3D
