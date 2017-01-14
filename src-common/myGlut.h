
#ifndef __glut_h__
#define __glut_h__

/* Copyright (c) Mark J. Kilgard, 1994, 1995, 1996, 1998. */

/* This program is freely distributable without licensing fees  and is
   provided without guarantee or warrantee expressed or  implied. This
   program is -not- in the public domain. */

/*#define GLUTAPI */
#include "myGl.h"

/* GLUT pre-built models sub-API */
#ifdef __cplusplus
extern "C" {
#endif
/*	GLUTAPI void APIENTRY glutWireSphere(GLdouble radius, GLint slices, GLint stacks);
	GLUTAPI void APIENTRY glutSolidSphere(GLdouble radius, GLint slices, GLint stacks);
	GLUTAPI void APIENTRY glutWireCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
	GLUTAPI void APIENTRY glutSolidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
	GLUTAPI void APIENTRY glutWireCube(GLdouble size);
	GLUTAPI void APIENTRY glutSolidCube(GLdouble size); */
	void myGlutWireTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings);
	void myGlutPointTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings);
	void myGlutSolidTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings);
/*	GLUTAPI void APIENTRY glutWireDodecahedron(void);
	GLUTAPI void APIENTRY glutSolidDodecahedron(void); */
	void myGlutPointTeapot(GLdouble size);
	void myGlutWireTeapot(GLdouble size);
	void myGlutSolidTeapot(GLdouble size);
/*	GLUTAPI void APIENTRY glutWireOctahedron(void);
	GLUTAPI void APIENTRY glutSolidOctahedron(void);
	GLUTAPI void APIENTRY glutWireTetrahedron(void);
	GLUTAPI void APIENTRY glutSolidTetrahedron(void);
	GLUTAPI void APIENTRY glutWireIcosahedron(void);
	GLUTAPI void APIENTRY glutSolidIcosahedron(void); */
    void myGluCylinder(GLUquadric *qobj, GLdouble baseRadius, GLdouble topRadius, GLdouble height, GLint slices, GLint stacks);
#ifdef __cplusplus
}
#endif
        
#endif                  /* __glut_h__ */

