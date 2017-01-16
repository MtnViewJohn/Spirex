/*
 ** License Applicability. Except to the extent portions of this file are
 ** made subject to an alternative license as permitted in the SGI Free
 ** Software License B, Version 1.1 (the "License"), the contents of this
 ** file are subject only to the provisions of the License. You may not use
 ** this file except in compliance with the License. You may obtain a copy
 ** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
 ** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
 **
 ** http://oss.sgi.com/projects/FreeB
 **
 ** Note that, as provided in the License, the Software is distributed on an
 ** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
 ** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
 ** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
 ** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 **
 ** Original Code. The Original Code is: OpenGL Sample Implementation,
 ** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
 ** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
 ** Copyright in any portions created by third parties is as indicated
 ** elsewhere herein. All Rights Reserved.
 **
 ** Additional Notice Provisions: The application programming interfaces
 ** established by SGI in conjunction with the Original Code are The
 ** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
 ** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
 ** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
 ** Window System(R) (Version 1.3), released October 19, 1998. This software
 ** was created using the OpenGL(R) version 1.2.1 Sample Implementation
 ** published by SGI, but has not been independently verified as being
 ** compliant with the OpenGL(R) version 1.2.1 Specification.
 **
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "myGl.h"
#include "myGlut.h"

#ifndef GLU_ERROR
#define GLU_ERROR                          100103
#endif

/* Make it not a power of two to avoid cache thrashing on the chip */
#define CACHE_SIZE  240

#undef  PI
#define PI        3.14159265358979323846


struct GLUquadric {
    GLint       normals;
    GLboolean   textureCoords;
    GLint       orientation;
    GLint       drawStyle;
    GLvoid      (*errorCallback)( GLint );
};

void 
myGluCylinder(GLUquadric *qobj, GLdouble baseRadius, GLdouble topRadius,
              GLdouble height, GLint slices, GLint stacks)
{
    GLint i,j;
    GLfloat sinCache[CACHE_SIZE];
    GLfloat cosCache[CACHE_SIZE];
    GLfloat sinCache2[CACHE_SIZE];
    GLfloat cosCache2[CACHE_SIZE];
    GLfloat sinCache3[CACHE_SIZE];
    GLfloat cosCache3[CACHE_SIZE];
    GLfloat angle;
    GLfloat zLow, zHigh;
    GLfloat sintemp, costemp;
    GLfloat length;
    GLfloat deltaRadius;
    GLfloat zNormal;
    GLfloat xyNormalRatio;
    GLfloat radiusLow, radiusHigh;
    int needCache2, needCache3;
    
    if (slices >= CACHE_SIZE) slices = CACHE_SIZE-1;
    
    if (slices < 2 || stacks < 1 /*|| baseRadius < 0.0 || topRadius < 0.0 ||
     height < 0.0*/) {
         //gluQuadricError(qobj, GLU_INVALID_VALUE);
         return;
     }
    
    /* Compute length (needed for normal calculations) */
    deltaRadius = baseRadius - topRadius;
    length = sqrt(deltaRadius*deltaRadius + height*height);
    if (length == 0.0) {
        //gluQuadricError(qobj, GLU_INVALID_VALUE);
        return;
    }
    
    /* Cache is the vertex locations cache */
    /* Cache2 is the various normals at the vertices themselves */
    /* Cache3 is the various normals for the faces */
    needCache2 = needCache3 = 0;
    if (qobj->normals == GLU_SMOOTH) {
        needCache2 = 1;
    }
    
    if (qobj->normals == GLU_FLAT) {
        if (qobj->drawStyle != GLU_POINT) {
            needCache3 = 1;
        }
        if (qobj->drawStyle == GLU_LINE) {
            needCache2 = 1;
        }
    }
    
    zNormal = deltaRadius / length;
    xyNormalRatio = height / length;
    
    for (i = 0; i < slices; i++) {
        angle = 2 * PI * i / slices;
        if (needCache2) {
            if (qobj->orientation == GLU_OUTSIDE) {
                sinCache2[i] = xyNormalRatio * sin(angle);
                cosCache2[i] = xyNormalRatio * cos(angle);
            } else {
                sinCache2[i] = -xyNormalRatio * sin(angle);
                cosCache2[i] = -xyNormalRatio * cos(angle);
            }
        }
        sinCache[i] = sin(angle);
        cosCache[i] = cos(angle);
    }
    
    if (needCache3) {
        for (i = 0; i < slices; i++) {
            angle = 2 * PI * (i-0.5) / slices;
            if (qobj->orientation == GLU_OUTSIDE) {
                sinCache3[i] = xyNormalRatio * sin(angle);
                cosCache3[i] = xyNormalRatio * cos(angle);
            } else {
                sinCache3[i] = -xyNormalRatio * sin(angle);
                cosCache3[i] = -xyNormalRatio * cos(angle);
            }
        }
    }
    
    sinCache[slices] = sinCache[0];
    cosCache[slices] = cosCache[0];
    if (needCache2) {
        sinCache2[slices] = sinCache2[0];
        cosCache2[slices] = cosCache2[0];
    }
    if (needCache3) {
        sinCache3[slices] = sinCache3[0];
        cosCache3[slices] = cosCache3[0];
    }
    
    switch (qobj->drawStyle) {
        case GLU_FILL:
            /* Note:
             ** An argument could be made for using a TRIANGLE_FAN for the end
             ** of the cylinder of either radii is 0.0 (a cone).  However, a
             ** TRIANGLE_FAN would not work in smooth shading mode (the common
             ** case) because the normal for the apex is different for every
             ** triangle (and TRIANGLE_FAN doesn't let me respecify that normal).
             ** Now, my choice is GL_TRIANGLES, or leave the GL_QUAD_STRIP and
             ** just let the GL trivially reject one of the two triangles of the
             ** QUAD.  GL_QUAD_STRIP is probably faster, so I will leave this code
             ** alone.
             */
            for (j = 0; j < stacks; j++) {
                zLow = j * height / stacks;
                zHigh = (j + 1) * height / stacks;
                radiusLow = baseRadius - deltaRadius * ((float) j / stacks);
                radiusHigh = baseRadius - deltaRadius * ((float) (j + 1) / stacks);
                
                glBegin(GL_QUAD_STRIP);
                for (i = 0; i <= slices; i++) {
                    switch(qobj->normals) {
                        case GLU_FLAT:
                            glNormal3f(sinCache3[i], cosCache3[i], zNormal);
                            break;
                        case GLU_SMOOTH:
                            glNormal3f(sinCache2[i], cosCache2[i], zNormal);
                            break;
                        case GLU_NONE:
                        default:
                            break;
                    }
                    if (qobj->orientation == GLU_OUTSIDE) {
                        if (qobj->textureCoords) {
                            glTexCoord2f(1 - (float) i / slices,
                                         (float) j / stacks);
                        }
                        glVertex3f(radiusLow * sinCache[i],
                                   radiusLow * cosCache[i], zLow);
                        if (qobj->textureCoords) {
                            glTexCoord2f(1 - (float) i / slices,
                                         (float) (j+1) / stacks);
                        }
                        glVertex3f(radiusHigh * sinCache[i],
                                   radiusHigh * cosCache[i], zHigh);
                    } else {
                        if (qobj->textureCoords) {
                            glTexCoord2f(1 - (float) i / slices,
                                         (float) (j+1) / stacks);
                        }
                        glVertex3f(radiusHigh * sinCache[i],
                                   radiusHigh * cosCache[i], zHigh);
                        if (qobj->textureCoords) {
                            glTexCoord2f(1 - (float) i / slices,
                                         (float) j / stacks);
                        }
                        glVertex3f(radiusLow * sinCache[i],
                                   radiusLow * cosCache[i], zLow);
                    }
                }
                glEnd();
            }
            break;
        case GLU_POINT:
            glBegin(GL_POINTS);
            for (i = 0; i < slices; i++) {
                switch(qobj->normals) {
                    case GLU_FLAT:
                    case GLU_SMOOTH:
                        glNormal3f(sinCache2[i], cosCache2[i], zNormal);
                        break;
                    case GLU_NONE:
                    default:
                        break;
                }
                sintemp = sinCache[i];
                costemp = cosCache[i];
                for (j = 0; j <= stacks; j++) {
                    zLow = j * height / stacks;
                    radiusLow = baseRadius - deltaRadius * ((float) j / stacks);
                    
                    if (qobj->textureCoords) {
                        glTexCoord2f(1 - (float) i / slices,
                                     (float) j / stacks);
                    }
                    glVertex3f(radiusLow * sintemp,
                               radiusLow * costemp, zLow);
                }
            }
            glEnd();
            break;
        case GLU_LINE:
            for (j = 1; j < stacks; j++) {
                zLow = j * height / stacks;
                radiusLow = baseRadius - deltaRadius * ((float) j / stacks);
                
                glBegin(GL_LINE_STRIP);
                for (i = 0; i <= slices; i++) {
                    switch(qobj->normals) {
                        case GLU_FLAT:
                            glNormal3f(sinCache3[i], cosCache3[i], zNormal);
                            break;
                        case GLU_SMOOTH:
                            glNormal3f(sinCache2[i], cosCache2[i], zNormal);
                            break;
                        case GLU_NONE:
                        default:
                            break;
                    }
                    if (qobj->textureCoords) {
                        glTexCoord2f(1 - (float) i / slices,
                                     (float) j / stacks);
                    }
                    glVertex3f(radiusLow * sinCache[i],
                               radiusLow * cosCache[i], zLow);
                }
                glEnd();
            }
            /* Intentionally fall through here... */
        case GLU_SILHOUETTE:
            for (j = 0; j <= stacks; j += stacks) {
                zLow = j * height / stacks;
                radiusLow = baseRadius - deltaRadius * ((float) j / stacks);
                
                glBegin(GL_LINE_STRIP);
                for (i = 0; i <= slices; i++) {
                    switch(qobj->normals) {
                        case GLU_FLAT:
                            glNormal3f(sinCache3[i], cosCache3[i], zNormal);
                            break;
                        case GLU_SMOOTH:
                            glNormal3f(sinCache2[i], cosCache2[i], zNormal);
                            break;
                        case GLU_NONE:
                        default:
                            break;
                    }
                    if (qobj->textureCoords) {
                        glTexCoord2f(1 - (float) i / slices,
                                     (float) j / stacks);
                    }
                    glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i],
                               zLow);
                }
                glEnd();
            }
            for (i = 0; i < slices; i++) {
                switch(qobj->normals) {
                    case GLU_FLAT:
                    case GLU_SMOOTH:
                        glNormal3f(sinCache2[i], cosCache2[i], 0.0);
                        break;
                    case GLU_NONE:
                    default:
                        break;
                }
                sintemp = sinCache[i];
                costemp = cosCache[i];
                glBegin(GL_LINE_STRIP);
                for (j = 0; j <= stacks; j++) {
                    zLow = j * height / stacks;
                    radiusLow = baseRadius - deltaRadius * ((float) j / stacks);
                    
                    if (qobj->textureCoords) {
                        glTexCoord2f(1 - (float) i / slices,
                                     (float) j / stacks);
                    }
                    glVertex3f(radiusLow * sintemp,
                               radiusLow * costemp, zLow);
                }
                glEnd();
            }
            break;
        default:
            break;
    }
}
