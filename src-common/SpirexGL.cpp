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

#include "SpirexGL.h"
#include "myGl.h"
#include "myGlut.h"
#include "SpirexGeom.h"
#include "SaverSettings.h"
#include "RingBuffer.h"
#include "Point3D.h"
//#include "texture.h"
#include <cmath>
#include <cstdio>

//#define Debug_Frame_Rate

int SpirexGL::LevelOfDetail = 100;

static void GLQuadMesh(bool points, int across, int up,
                       Point3D t1, Point3D p1,
                       Point3D t2, Point3D p2,
                       Point3D t3, Point3D p3,
                       Point3D t4, Point3D p4,
                       Point3D Offset = Point3D())
{
    Point3D row[50][2];
    Point3D texRow[50][2];
    Point3D rowStart;
    Point3D rowEnd;
    Point3D texRowStart;
    Point3D texRowEnd;
    
    Point3D normal = (p2 - p1) * (p3 - p1);
    normal.normalize();
    if ((across == 0) || (up == 0)) return;
    
    for (int y = 0; y <= up; y++) {
        if ((y == up) && (p2 == p4))
            break;
        rowStart = p1 + (p2 - p1) * ((double) y / (double)up);
        rowEnd = p3 + (p4 - p3) * ((double) y / (double)up);
        texRowStart = t1 + (t2 - t1) * ((double) y / (double)up) + Offset;
        texRowEnd = t3 + (t4 - t3) * ((double) y / (double)up) + Offset;
        if (y) {
            glBegin(points ? GL_POINTS : GL_QUAD_STRIP);
            glNormal3fv((GLfloat*)&normal);
        }
        for (int x = 0; x <= across; x++) {
            row[x][y & 1] = rowStart + (rowEnd - rowStart) * ((double) x / (double)across);
            texRow[x][y & 1] = texRowStart + (texRowEnd - texRowStart) * ((double) x / (double)across);
            if (y) {
                if (!points) glTexCoord3fv((GLfloat*)&(texRow[x][1 - (y & 1)]));
                glVertex3fv((GLfloat*)&(row[x][1 - (y & 1)]));
                if (!points) glTexCoord3fv((GLfloat*)&(texRow[x][y & 1]));
                glVertex3fv((GLfloat*)&(row[x][y & 1]));
            }
        }
        if (y)
            glEnd();
    }
    
    if (p2 == p4) {
        glBegin(points ? GL_POINTS : GL_TRIANGLES);
        glNormal3fv((GLfloat*)&normal);
        if (!points) glTexCoord3fv((GLfloat*)&texRowStart);
        glVertex3fv((GLfloat*)&rowStart);
        if (!points) glTexCoord3fv((GLfloat*)&t2);
        glVertex3fv((GLfloat*)&p2);
        if (!points) glTexCoord3fv((GLfloat*)&texRowEnd);
        glVertex3fv((GLfloat*)&rowEnd);
        glEnd();
    }
}

void SpirexGL::Render(int rate, const SpirexGeom& geom)
{
    const SaverSettings& settings = geom.mSettings;
    double LODfactor = (double)LevelOfDetail / 100.0;
    int facets = 15;
    
    glPointSize((settings.mThickLines && !settings.mFixed) ? 2.0F : 1.0F);
    if (settings.mMode <= SaverSettings::Original2DWith3DGeometry) {
        glClear(GL_COLOR_BUFFER_BIT);
        
        glLineWidth(settings.mThickLines ? 2.0 : 1.0);
        
        for (unsigned int curve = 0; curve < settings.mCurveCount; curve++) {
            glBegin(settings.mPoints ? GL_POINTS : GL_LINE_STRIP);
            for (int n = settings.mCurveLength-1; n >= 0; n -= 1) {
                Point3D point = geom.mPointRingBufferArray[curve][n];
                ColorRGB colorRGB = geom.mColorRingBufferArray[curve][n];
                
                glColor3fv((GLfloat*)&colorRGB);
                glVertex2fv((GLfloat*)&point);
            }
            glEnd();
        }
        
        return;
    }
    
    
    GLfloat txtMatrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    
    if (settings.mPoints)
        glClear(GL_COLOR_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (unsigned int curve = 0; curve < settings.mCurveCount; curve++) {
        if (settings.mMode == SaverSettings::Curves) {
            glMatrixMode(GL_TEXTURE);
            glPushMatrix();
            txtMatrix[4] = settings.mTriAxial ? geom.mRadius4RingBufferArray[curve].front() : 0;
            txtMatrix[13] = settings.mTriAxial ? geom.mRadius3RingBufferArray[curve].front() : 0;
            txtMatrix[1] = settings.mTriAxial ? geom.mRadius2RingBufferArray[curve].front() : 0;
            txtMatrix[12] = settings.mTriAxial ? geom.mRadius1RingBufferArray[curve].front() : 0;
            glLoadMatrixf(txtMatrix);
            glBegin(settings.mPoints ? GL_POINTS : GL_QUAD_STRIP);
            for (unsigned int i = 0; i <= settings.mCurveLength; i++) {
                ColorRGB color = geom.mColorRingBufferArray[curve][i];
                Point3D norm = geom.mNormalRingBufferArray[curve][i];
                Point3D pDelta = geom.mWidthRingBufferArray[curve][i];
                Point3D p = geom.mPointRingBufferArray[curve][i];
                Point3D p2 = p + pDelta;
                p = p - pDelta;
                
                glColor3fv((GLfloat*)&color);
                glNormal3fv((GLfloat*)&norm);
                if (!settings.mPoints) glTexCoord2f(i * 0.05f, 0.0f);
                glVertex3fv((GLfloat*)&p);
                if (!settings.mPoints) glTexCoord2f(i * 0.05f, 1.0f);
                glVertex3fv((GLfloat*)&p2);
            }
            glEnd();
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
        } else {
            GLUquadricObj* pQuadric = 0;
            
            glPushMatrix();
            
            Point3D norm = geom.mNormalRingBufferArray[curve].front();
            
            Point3D position;
            float radius, radius2, radius3, radius4;
            radius4 = geom.mRadius4RingBufferArray[curve].front();
            float txtOffset = geom.mRadius3RingBufferArray[curve].front();
            if (settings.mFixed) {
                position = geom.mPointRingBufferArray[curve].front();
                if (settings.mTriAxial) {
                    radius =  fabs(geom.mRadius1RingBufferArray[curve].front()) + 0.001f;
                    radius2 = fabs(geom.mRadius2RingBufferArray[curve].front()) + 0.001f;
                    radius3 = fabs(geom.mRadius3RingBufferArray[curve].front()) + 0.001f;
                    if (radius3 < radius)
                        radius3 = (radius3 / radius) * (settings.mThickLines ? 0.05 : 0.025);
                    else
                        radius3 = (radius / radius3) * (settings.mThickLines ? 0.05 : 0.025);
                    
                    if (radius2 < radius)
                        radius2 = (radius2 / radius) * (settings.mThickLines ? 0.05 : 0.025);
                    else
                        radius2 = (radius / radius2) * (settings.mThickLines ? 0.05 : 0.025);
                    
                    radius  = settings.mThickLines ? 0.05 : 0.025;
                } else {
                    radius = settings.mThickLines ? 0.05 : 0.025;
                    radius2 = radius3 = radius;
                }
            } else {
                position = geom.mMovingSphereCenter[curve];
                //radius = (position - geom.mPointRingBufferArray[curve].get(0)).length();
                radius = geom.mRadius1RingBufferArray[curve].front();
                if (settings.mTriAxial) {
                    radius2 = geom.mRadius2RingBufferArray[curve].front();
                    radius3 = geom.mRadius3RingBufferArray[curve].front();
                } else {
                    radius2 = radius3 = radius;
                }
                facets = (int)(120 * radius);
                facets = (facets < 15) ? 15 : facets;
                facets = (facets > 30) ? 30 : facets;
                facets >>= (settings.mThickLines ? 1 : 0);
            }
            facets *= LODfactor;
            
            glTranslatef(position.x, position.y, position.z);
            glRotatef(90.0F, norm.x, norm.y, norm.z);
            ColorRGB color = geom.mColorRingBufferArray[curve].front();
            glColor3fv((GLfloat*)&color);
            
            switch (settings.mMode) {
                case SaverSettings::Teapots:
                    radius  *= 0.5f;   // scale teapot to fit within equivalent sphere
                    radius2 *= 0.5f;
                    radius3 *= 0.5f;
                    glScalef(fabs(radius), fabs(radius2), fabs(radius3));
                    if (settings.mPoints) {
                        myGlutPointTeapot(1.0);
                    } else {
                        glMatrixMode(GL_TEXTURE);
                        glPushMatrix();
                        txtMatrix[4] = settings.mTriAxial ? radius4 : 0;
                        txtMatrix[13] = settings.mTriAxial ? txtOffset : 0;
                        txtMatrix[0] = txtMatrix[5] = (radius * radius2 * radius3 >= 0) ? 1 : -1;
                        glLoadMatrixf(txtMatrix);
                        myGlutSolidTeapot(1.0);
                        glPopMatrix();
                        glMatrixMode(GL_MODELVIEW);
                    }
                    break;
                case SaverSettings::Spheres:
                    glScalef(fabs(radius), fabs(radius2), fabs(radius3));
                    txtMatrix[0] = txtMatrix[5] = (radius * radius2 * radius3 >= 0) ? 1 : -1;
                case SaverSettings::Disks:
                case SaverSettings::Conics:
                case SaverSettings::Cylinders:
                    pQuadric = gluNewQuadric();
                    if (settings.mPoints) {
                        gluQuadricDrawStyle(pQuadric, GLU_POINT);
                        gluQuadricNormals(pQuadric, GLU_NONE);
                        gluQuadricTexture(pQuadric, GL_FALSE);
                    } else {
                        gluQuadricDrawStyle(pQuadric, GLU_FILL);
                        gluQuadricNormals(pQuadric, GLU_SMOOTH);
                        gluQuadricTexture(pQuadric, GL_TRUE);
                        glMatrixMode(GL_TEXTURE);
                        glPushMatrix();
                        txtMatrix[4] = settings.mTriAxial ? radius4 : 0;
                        txtMatrix[13] = settings.mTriAxial ? txtOffset : 0;
                        glLoadMatrixf(txtMatrix);
                    }
                    if (settings.mMode == SaverSettings::Spheres) {
                        gluSphere(pQuadric, 1.0F, facets, facets);
                    } else if (settings.mMode == SaverSettings::Disks) {
                        if (!settings.mTriAxial)
                            radius2 = 0.0f;
                        if (fabs(radius2) > fabs(radius)) {
                            radius3 = radius2;
                            radius2 = radius;
                            radius = radius3;
                        }
                        gluDisk(pQuadric, fabs(radius2), fabs(radius), facets, facets / 3);
                    } else {
                        if (!settings.mTriAxial)
                            radius2 = radius3 = radius;
                        if (settings.mMode == SaverSettings::Conics)
                            myGluCylinder(pQuadric, (radius), (radius2), (radius3), facets, facets / 3);
                        else
                            myGluCylinder(pQuadric, fabs(radius), fabs(radius2), fabs(radius3), facets, facets / 3);
                    }
                    if (!settings.mPoints) {
                        glPopMatrix();
                        glMatrixMode(GL_MODELVIEW);
                    }
                    break;
                case SaverSettings::Cubes:
                case SaverSettings::WrappedCubes:
                    facets /= 4;
                    if (facets < 2) facets = 2;
                    radius *= -0.65f;   // scale cube to fit within equivalent sphere
                    radius2 *= -0.65f;
                    radius3 *= -0.65f;
                    
                    glMatrixMode(GL_TEXTURE);
                    glPushMatrix();
                    txtMatrix[4] = settings.mTriAxial ? radius4 : 0;
                    txtMatrix[13] = settings.mTriAxial ? txtOffset : 0;
                    glLoadMatrixf(txtMatrix);
                    
                    if (settings.mMode == SaverSettings::WrappedCubes) {
                        if (radius < 0.0)
                            glCullFace(GL_FRONT);
                        else
                            glCullFace(GL_BACK);
                        
                        Point3D top = Point3D(radius,  radius2,  radius3);
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.00f, 0.67f, 0.00f), Point3D(-radius,  radius2,  radius3),
                                   Point3D(0.00f, 1.00f, 0.00f), top,
                                   Point3D(0.33f, 0.67f, 0.00f), Point3D( radius,  radius2, -radius3),
                                   Point3D(0.33f, 1.00f, 0.00f), top);
                        
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.33f, 0.67f, 0.00f), Point3D(radius,  radius2, -radius3),
                                   Point3D(0.33f, 1.00f, 0.00f), top,
                                   Point3D(0.67f, 0.67f, 0.00f), Point3D(radius, -radius2,  radius3),
                                   Point3D(0.67f, 1.00f, 0.00f), top);
                        
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.67f, 0.67f, 0.00f), Point3D( radius, -radius2,  radius3),
                                   Point3D(0.67f, 1.00f, 0.00f), top,
                                   Point3D(1.0f, 0.67f, 0.00f),  Point3D(-radius,  radius2,  radius3),
                                   Point3D(1.0f, 1.00f, 0.00f),  top);
                        
                        Point3D bottom = Point3D(-radius,  -radius2,  -radius3);
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(1.167f, 0.33f, 0.00f), Point3D(-radius,  radius2, -radius3),
                                   Point3D(1.167f, 0.00f, 0.00f), bottom,
                                   Point3D(0.833f, 0.33f, 0.00f), Point3D(-radius, -radius2,  radius3),
                                   Point3D(0.833f, 0.00f, 0.00f), bottom);
                        
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.833f, 0.33f, 0.00f), Point3D(-radius, -radius2,  radius3),
                                   Point3D(0.833f, 0.00f, 0.00f), bottom,
                                   Point3D(0.50f, 0.33f, 0.00f),  Point3D( radius, -radius2, -radius3),
                                   Point3D(0.50f, 0.00f, 0.00f),  bottom);
                        
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.50f, 0.33f, 0.00f),  Point3D( radius, -radius2, -radius3),
                                   Point3D(0.50f, 0.00f, 0.00f),  bottom,
                                   Point3D(0.167f, 0.33f, 0.00f), Point3D(-radius,  radius2, -radius3),
                                   Point3D(0.167f, 0.00f, 0.00f), bottom);
                        
                        
                        if (radius > 0.0)
                            glCullFace(GL_FRONT);
                        else
                            glCullFace(GL_BACK);
#if 0
                        glBegin(settings.mPoints ? GL_POINTS : GL_TRIANGLES);
                        glNormal3f(0.0f, 1.0f, 0.0f);
                        glTexCoord2f(0.000f, 0.67f); glVertex3f(-radius,  radius2,  radius3);
                        glTexCoord2f(0.167f, 0.33f); glVertex3f(-radius,  radius2, -radius3);
                        glTexCoord2f(0.333f, 0.67f); glVertex3f( radius,  radius2, -radius3);
                        
                        glNormal3f(0.0f, 0.0f, -1.0f);
                        glTexCoord2f(0.167f, 0.33f); glVertex3f(-radius,  radius2, -radius3);
                        glTexCoord2f(0.500f, 0.33f); glVertex3f( radius, -radius2, -radius3);
                        glTexCoord2f(0.333f, 0.67f); glVertex3f( radius,  radius2, -radius3);
                        
                        glNormal3f(1.0f, 0.0f, 0.0f);
                        glTexCoord2f(0.333f, 0.67f); glVertex3f( radius,  radius2, -radius3);
                        glTexCoord2f(0.500f, 0.33f); glVertex3f( radius, -radius2, -radius3);
                        glTexCoord2f(0.667f, 0.67f); glVertex3f( radius, -radius2,  radius3);
                        
                        glNormal3f(0.0f, -1.0f, 0.0f);
                        glTexCoord2f(0.500f, 0.33f); glVertex3f( radius, -radius2, -radius3);
                        glTexCoord2f(0.833f, 0.33f); glVertex3f(-radius, -radius2,  radius3);
                        glTexCoord2f(0.667f, 0.67f); glVertex3f( radius, -radius2,  radius3);
                        
                        glNormal3f(0.0f, 0.0f, 1.0f);
                        glTexCoord2f(0.667f, 0.67f); glVertex3f( radius, -radius2,  radius3);
                        glTexCoord2f(0.833f, 0.33f); glVertex3f(-radius, -radius2,  radius3);
                        glTexCoord2f(1.000f, 0.67f); glVertex3f(-radius,  radius2,  radius3);
                        
                        
                        glNormal3f(-1.0f, 0.0f, 0.0f);
                        glTexCoord2f(1.000f, 0.67f); glVertex3f(-radius,  radius2,  radius3);
                        glTexCoord2f(0.833f, 0.33f); glVertex3f(-radius, -radius2,  radius3);
                        glTexCoord2f(1.167f, 0.33f); glVertex3f(-radius,  radius2, -radius3);
                        
                        glEnd();
#else
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.333f, 0.67f, 0.00f), Point3D( radius,  radius2, -radius3),
                                   Point3D(0.167f, 0.33f, 0.00f), Point3D(-radius,  radius2, -radius3),
                                   Point3D(0.000f, 0.67f, 0.00f), Point3D(-radius,  radius2,  radius3),
                                   Point3D(0.167f, 0.33f, 0.00f), Point3D(-radius,  radius2, -radius3));
                        
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.333f, 0.67f, 0.00f), Point3D( radius,  radius2, -radius3),
                                   Point3D(0.500f, 0.33f, 0.00f), Point3D( radius, -radius2, -radius3),
                                   Point3D(0.167f, 0.33f, 0.00f), Point3D(-radius,  radius2, -radius3),
                                   Point3D(0.500f, 0.33f, 0.00f), Point3D( radius, -radius2, -radius3));
                        
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.667f, 0.67f, 0.00f), Point3D( radius, -radius2,  radius3),
                                   Point3D(0.500f, 0.33f, 0.00f), Point3D( radius, -radius2, -radius3),
                                   Point3D(0.333f, 0.67f, 0.00f), Point3D( radius,  radius2, -radius3),
                                   Point3D(0.500f, 0.33f, 0.00f), Point3D( radius, -radius2, -radius3));
                        
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.667f, 0.67f, 0.00f), Point3D( radius, -radius2,  radius3),
                                   Point3D(0.833f, 0.33f, 0.00f), Point3D(-radius, -radius2,  radius3),
                                   Point3D(0.500f, 0.33f, 0.00f), Point3D( radius, -radius2, -radius3),
                                   Point3D(0.833f, 0.33f, 0.00f), Point3D(-radius, -radius2,  radius3));
                        
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(1.000f, 0.67f, 0.00f), Point3D(-radius,  radius2,  radius3),
                                   Point3D(0.833f, 0.33f, 0.00f), Point3D(-radius, -radius2,  radius3),
                                   Point3D(0.667f, 0.67f, 0.00f), Point3D( radius, -radius2,  radius3),
                                   Point3D(0.833f, 0.33f, 0.00f), Point3D(-radius, -radius2,  radius3));
                        
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(1.167f, 0.33f, 0.00f), Point3D(-radius,  radius2, -radius3),
                                   Point3D(0.833f, 0.33f, 0.00f), Point3D(-radius, -radius2,  radius3),
                                   Point3D(1.000f, 0.67f, 0.00f), Point3D(-radius,  radius2,  radius3),
                                   Point3D(0.833f, 0.33f, 0.00f), Point3D(-radius, -radius2,  radius3));
#endif
                        glCullFace(GL_BACK);
                    } else {
                        // Front Face 1
                        //glColor3f(1.0f, 0.0f, 0.0f);
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(1.0f, 1.0f, 0.0f), Point3D( radius, -radius2,  radius3),
                                   Point3D(0.0f, 1.0f, 0.0f), Point3D( radius,  radius2,  radius3),
                                   Point3D(1.0f, 0.0f, 0.0f), Point3D(-radius, -radius2,  radius3),
                                   Point3D(0.0f, 0.0f, 0.0f), Point3D(-radius,  radius2,  radius3));
                        // Back Face 2
                        //glColor3f(0.0f, 1.0f, 0.0f);
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(1.0f, 0.0f, 0.0f), Point3D(-radius,  radius2, -radius3),
                                   Point3D(1.0f, 1.0f, 0.0f), Point3D( radius,  radius2, -radius3),
                                   Point3D(0.0f, 0.0f, 0.0f), Point3D(-radius, -radius2, -radius3),
                                   Point3D(0.0f, 1.0f, 0.0f), Point3D( radius, -radius2, -radius3));
                        // Top Face 3
                        //glColor3f(0.0f, 0.0f, 1.0f);
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(0.0f, 0.0f, 0.0f), Point3D(-radius,  radius2,  radius3),
                                   Point3D(1.0f, 0.0f, 0.0f), Point3D( radius,  radius2,  radius3),
                                   Point3D(0.0f, 1.0f, 0.0f), Point3D(-radius,  radius2, -radius3),
                                   Point3D(1.0f, 1.0f, 0.0f), Point3D( radius,  radius2, -radius3));
                        // Bottom Face 4
                        //glColor3f(0.0f, 1.0f, 1.0f);
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(1.0f, 1.0f, 0.0f), Point3D(-radius, -radius2, -radius3),
                                   Point3D(0.0f, 1.0f, 0.0f), Point3D( radius, -radius2, -radius3),
                                   Point3D(1.0f, 0.0f, 0.0f), Point3D(-radius, -radius2,  radius3),
                                   Point3D(0.0f, 0.0f, 0.0f), Point3D( radius, -radius2,  radius3));
                        // Right face 5
                        //glColor3f(1.0f, 0.0f, 1.0f);
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(1.0f, 0.0f, 0.0f), Point3D( radius, -radius2, -radius3),
                                   Point3D(1.0f, 1.0f, 0.0f), Point3D( radius,  radius2, -radius3),
                                   Point3D(0.0f, 0.0f, 0.0f), Point3D( radius, -radius2,  radius3),
                                   Point3D(0.0f, 1.0f, 0.0f), Point3D( radius,  radius2,  radius3));
                        // Left Face 6
                        //glColor3f(1.0f, 1.0f, 0.0f);
                        GLQuadMesh(settings.mPoints, facets, facets,
                                   Point3D(1.0f, 0.0f, 0.0f), Point3D(-radius, -radius2,  radius3), // Bottom Right Of The Texture and Quad
                                   Point3D(1.0f, 1.0f, 0.0f), Point3D(-radius,  radius2,  radius3), // Top Right Of The Texture and Quad
                                   Point3D(0.0f, 0.0f, 0.0f), Point3D(-radius, -radius2, -radius3), // Bottom Left Of The Texture and Quad
                                   Point3D(0.0f, 1.0f, 0.0f), Point3D(-radius,  radius2, -radius3));// Top Left Of The Texture and Quad
                    }
                    
                    glPopMatrix();
                    glMatrixMode(GL_MODELVIEW);
                    
                    break;
                case SaverSettings::Toroids:
                    radius =  geom.mRadius1RingBufferArray[curve].front() * 0.7;
                    radius2 = geom.mRadius2RingBufferArray[curve].front() * radius;
                    radius3 = geom.mRadius3RingBufferArray[curve].front();
                    radius4 = geom.mRadius4RingBufferArray[curve].front();
                    if (radius2 < 0.0)
                        radius3 = -radius3;
                    
                    radius = fabs(radius);
                    radius2 = fabs(radius2);
                    radius3 *= 4.0;
                    
                    if (!settings.mTriAxial) {
                        radius2 = radius * 0.5;
                        radius3 = 0.5;
                        radius4 = 0.0;
                    }
                    
                    if (settings.mPoints) {
                        myGlutPointTorus(radius2, radius, facets, facets);
                    } else {
                        glMatrixMode(GL_TEXTURE);
                        glPushMatrix();
                        txtMatrix[13] = radius3;
                        txtMatrix[1] = radius4;
                        glLoadMatrixf(txtMatrix);
                        myGlutSolidTorus(radius2, radius, facets, facets);
                        glPopMatrix();
                        glMatrixMode(GL_MODELVIEW);
                    }
                    
                    break;
                case SaverSettings::Squares:
                    facets /= 4;
                    if (facets < 2) facets = 2;
                    radius2 = geom.mRadius2RingBufferArray[curve].front();
                    radius3 = geom.mRadius3RingBufferArray[curve].front();
                    if (!settings.mTriAxial) {
                        radius2 = radius3 = 0.0f;
                    } else {
                        radius2 *= 4.0f; radius3 *= 4.0f;
                        glMatrixMode(GL_TEXTURE);
                        glPushMatrix();
                        txtMatrix[4] = radius4;
                        txtMatrix[1] = txtOffset;
                        glLoadMatrixf(txtMatrix);
                    }
                    
                    GLQuadMesh(settings.mPoints, facets, facets,
                               Point3D(0.0f, 0.0f, 0.0f), Point3D(-radius, -radius,  0.0f),
                               Point3D(0.0f, 1.0f, 0.0f), Point3D(-radius,  radius,  0.0f),
                               Point3D(1.0f, 0.0f, 0.0f), Point3D( radius, -radius,  0.0f),
                               Point3D(1.0f, 1.0f, 0.0f), Point3D( radius,  radius,  0.0f),
                               Point3D(radius2, radius3, 0.0f));
                    
                    if (settings.mTriAxial) {
                        glPopMatrix();
                        glMatrixMode(GL_MODELVIEW);
                    }
                    
                    break;
                default:        // never reached, only here to suppress warnings
                    break;
            }
            glPopMatrix();
            if (pQuadric) gluDeleteQuadric(pQuadric);
        }
    }
    
#ifdef Debug_Frame_Rate
    static char buf[8];
    glColor3f(2.0f, 2.0f, 2.0f);
    glRasterPos2f(-1.1f, -0.9f);
    wsprintf(buf, "%3d", rate);
    glCallLists (3, GL_UNSIGNED_BYTE, buf);
#endif
}

static void setProjection(GLfloat width, GLfloat height)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /*
     ** Preserve the aspect ratio of objects in the scene.
     */
    if (width > height) {
        GLfloat aspect = width / height;
        glFrustum(-0.5F*aspect, 0.5F*aspect, -0.5F, 0.5F, 1.0F, 3.0F);
    } else {
        GLfloat aspect = height / width;
        glFrustum(-0.5F, 0.5F, -0.5F*aspect, 0.5F*aspect, 1.0F, 3.0F);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0F, 0.0F, -2.1F);
}

static void setMaterial()
{
    GLfloat matAmb[4] = { 0.15F, 0.15F, 0.15F, 1.00F };
    GLfloat matDif[4] = { 0.35F, 0.35F, 0.35F, 1.00F };
    GLfloat matSpc[4] = { 0.65F, 0.65F, 0.65F, 1.00F };
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpc);
}

#if 0
static void spotLight(GLenum light, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat lightPos[4];
    lightPos[0] = x; lightPos[1] = y; lightPos[2] = z;
    lightPos[3] = 0.00F;
    glLightfv(light, GL_POSITION, lightPos);
    
    GLfloat lightSpotTarget[4] = { 0.50F, -1.0F, 0.00F, 1.00F };
    lightSpotTarget[2] = z/2.0;
    GLfloat lightSpotDir[3];
    for (int i = 0; i < 3; ++i)
        lightSpotDir[i] = lightSpotTarget[i] - lightPos[i];
    
    glLightfv(light, GL_SPOT_DIRECTION, lightSpotDir);
    
    glLightf(light, GL_SPOT_CUTOFF, 55.0F);
    glLightf(light, GL_SPOT_EXPONENT, 90.0F);
    
    const GLfloat Intensity = 0.75F;
    
    GLfloat lightAmb[4] = { 0.10F, 0.10F, 0.10F, 1.00F };
    GLfloat lightDif[4] = { Intensity, Intensity, Intensity, 1.00F };
    GLfloat lightSpec[4] = { 1 - Intensity, 1 - Intensity, 1 - Intensity, 1.00F };
    glLightfv(light, GL_AMBIENT, lightAmb);
    glLightfv(light, GL_DIFFUSE, lightDif);
    glLightfv(light, GL_SPECULAR, lightSpec);
    
    glEnable(light);
}
#endif

static void setLights()
{
    const GLfloat AmbientIntensity = 0.40F;
    GLfloat lightModelAmb[4] = { AmbientIntensity, AmbientIntensity, AmbientIntensity, 1.00F };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmb);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    
    GLfloat light0Pos[4] = { 0.0F, 1.7F, 1.0F, 0.00F };
    glLightfv(GL_LIGHT2, GL_POSITION, light0Pos);
    
    const GLfloat Intensity = 0.35F;
    GLfloat light0Amb[4] = { 0.10F, 0.10F, 0.10F, 1.00F };
    GLfloat light0Dif[4] = { Intensity, Intensity, Intensity, 1.00F };
    GLfloat light0Spc[4] = { Intensity, Intensity, Intensity, 1.00F };
    glLightfv(GL_LIGHT2, GL_AMBIENT, light0Amb);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light0Dif);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light0Spc);
    glEnable(GL_LIGHT2);
    glDisable(GL_LIGHT0);
    
    //spotLight(GL_LIGHT1, -1.75F, 3.50F, 2.00F);
}


void SpirexGL::InitGL(SpirexGeom& geom, GLfloat width, GLfloat height)
{
    setProjection(width, height);
    setMaterial();
    setLights();
    
    InitMode(geom);
}

void SpirexGL::InitMode(const SpirexGeom& geom)
{
    const SaverSettings& settings = geom.mSettings;
    
    // common 2D and 3D settings
    glDisable(GL_DITHER);
    
    if (settings.mMode <= SaverSettings::Original2DWith3DGeometry) {
        // put shared settings in 2D modes
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        //glDisable(GL_ALPHA_TEST);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // settings that only matter in 2D
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
        
        return;
    }
    
    // put shared settings in 3D modes
    glDisable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    
    glDisable(GL_BLEND);
    
    // settings that only matter in 3D
    glDisable(GL_POLYGON_SMOOTH);
    if (settings.getTextureStrlen() > 0)
        glEnable(GL_TEXTURE_2D);
    else
        glDisable(GL_TEXTURE_2D);
    
    if (settings.mPoints) {
        glDisable(GL_DEPTH_TEST);
        //glDisable(GL_ALPHA_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
        return;
    }
    
    switch (settings.mMode) {
        case SaverSettings::Curves:
            glMaterialf(GL_FRONT, GL_SHININESS, 0.9F*128.0F);
            glMaterialf(GL_BACK, GL_SHININESS, 0.1F*128.0F);
            glShadeModel(GL_FLAT);
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
            glDisable(GL_CULL_FACE);
            break;
        case SaverSettings::Spheres:
        case SaverSettings::Teapots:
            glEnable(GL_NORMALIZE);
        case SaverSettings::Toroids:        
        case SaverSettings::Cubes:
        case SaverSettings::WrappedCubes:
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.5F*128.0F);
            glShadeModel(GL_SMOOTH);
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
            glDisable(GL_CULL_FACE);
            //glEnable(GL_ALPHA_TEST);
            //glAlphaFunc(GL_GREATER, 0.5f);
            //glEnable(GL_CULL_FACE);
            break;
        case SaverSettings::Disks:
        case SaverSettings::Conics:
        case SaverSettings::Cylinders:
        case SaverSettings::Squares:
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.5F*128.0F);
            glShadeModel(GL_SMOOTH);
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
            glDisable(GL_CULL_FACE);
            break;
        default:  // never used, just to suppress warnings
            break;
    }
}

