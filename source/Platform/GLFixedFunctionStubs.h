#pragma once

// GLFixedFunctionStubs.h
// No-op stubs for OpenGL 1.x fixed-function pipeline calls that don't exist in GLES2.
// These allow legacy rendering code to compile on Android without changes.
// Actual rendering is handled by the OpenGLESRenderBackend shader pipeline.

#if defined(__ANDROID__)

// Primitive mode constants (not in GLES2)
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif
#ifndef GL_QUAD_STRIP
#define GL_QUAD_STRIP 0x0008
#endif

// Matrix mode constants (not in GLES2)
#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif
#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif
#ifndef GL_TEXTURE
#define GL_TEXTURE 0x1702
#endif

// Legacy capability constants
#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif
#ifndef GL_FOG
#define GL_FOG 0x0B60
#endif
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif
#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif
#ifndef GL_NORMALIZE
#define GL_NORMALIZE 0x0BA1
#endif
#ifndef GL_COLOR_MATERIAL
#define GL_COLOR_MATERIAL 0x0B57
#endif
#ifndef GL_CLAMP
#define GL_CLAMP 0x2900
#endif

// glAlphaFunc reference constant
#ifndef GL_GREATER
#define GL_GREATER 0x0204
#endif
#ifndef GL_GEQUAL
#define GL_GEQUAL 0x0206
#endif

// Immediate mode rendering (no-op)
inline void glBegin(unsigned int /*mode*/) {}
inline void glEnd() {}
inline void glVertex2f(float /*x*/, float /*y*/) {}
inline void glVertex3f(float /*x*/, float /*y*/, float /*z*/) {}
inline void glVertex3fv(const float* /*v*/) {}
inline void glVertex2i(int /*x*/, int /*y*/) {}
inline void glTexCoord2f(float /*s*/, float /*t*/) {}
inline void glTexCoord2fv(const float* /*v*/) {}
inline void glNormal3f(float /*nx*/, float /*ny*/, float /*nz*/) {}
inline void glNormal3fv(const float* /*v*/) {}
inline void glColor3f(float /*r*/, float /*g*/, float /*b*/) {}
inline void glColor4f(float /*r*/, float /*g*/, float /*b*/, float /*a*/) {}
inline void glColor3fv(const float* /*v*/) {}
inline void glColor4fv(const float* /*v*/) {}
inline void glColor3ub(unsigned char /*r*/, unsigned char /*g*/, unsigned char /*b*/) {}
inline void glColor4ub(unsigned char /*r*/, unsigned char /*g*/, unsigned char /*b*/, unsigned char /*a*/) {}
inline void glColor3ubv(const unsigned char* /*v*/) {}
inline void glColor4ubv(const unsigned char* /*v*/) {}

// Matrix stack (no-op)
inline void glMatrixMode(unsigned int /*mode*/) {}
inline void glLoadIdentity() {}
inline void glPushMatrix() {}
inline void glPopMatrix() {}
inline void glTranslatef(float /*x*/, float /*y*/, float /*z*/) {}
inline void glRotatef(float /*angle*/, float /*x*/, float /*y*/, float /*z*/) {}
inline void glScalef(float /*x*/, float /*y*/, float /*z*/) {}
inline void glMultMatrixf(const float* /*m*/) {}
inline void glLoadMatrixf(const float* /*m*/) {}
inline void glOrtho(double /*left*/, double /*right*/, double /*bottom*/, double /*top*/, double /*zNear*/, double /*zFar*/) {}
inline void glFrustum(double /*left*/, double /*right*/, double /*bottom*/, double /*top*/, double /*zNear*/, double /*zFar*/) {}
inline void gluPerspective(double /*fovy*/, double /*aspect*/, double /*zNear*/, double /*zFar*/) {}
inline void gluLookAt(double /*eyeX*/, double /*eyeY*/, double /*eyeZ*/, double /*centerX*/, double /*centerY*/, double /*centerZ*/, double /*upX*/, double /*upY*/, double /*upZ*/) {}

// Lighting (no-op)
inline void glLightf(unsigned int /*light*/, unsigned int /*pname*/, float /*param*/) {}
inline void glLightfv(unsigned int /*light*/, unsigned int /*pname*/, const float* /*params*/) {}
inline void glLightModelfv(unsigned int /*pname*/, const float* /*params*/) {}
inline void glMaterialf(unsigned int /*face*/, unsigned int /*pname*/, float /*param*/) {}
inline void glMaterialfv(unsigned int /*face*/, unsigned int /*pname*/, const float* /*params*/) {}
inline void glColorMaterial(unsigned int /*face*/, unsigned int /*mode*/) {}
inline void glShadeModel(unsigned int /*mode*/) {}

// Fog (no-op)
inline void glFogf(unsigned int /*pname*/, float /*param*/) {}
inline void glFogfv(unsigned int /*pname*/, const float* /*params*/) {}
inline void glFogi(unsigned int /*pname*/, int /*param*/) {}

// Alpha func (no-op)
inline void glAlphaFunc(unsigned int /*func*/, float /*ref*/) {}

// Texture environment (no-op)
inline void glTexEnvf(unsigned int /*target*/, unsigned int /*pname*/, float /*param*/) {}
inline void glTexEnvi(unsigned int /*target*/, unsigned int /*pname*/, int /*param*/) {}
inline void glTexEnvfv(unsigned int /*target*/, unsigned int /*pname*/, const float* /*params*/) {}

// Client-side arrays (no-op for legacy-style)
inline void glVertexPointer(int /*size*/, unsigned int /*type*/, int /*stride*/, const void* /*pointer*/) {}
inline void glTexCoordPointer(int /*size*/, unsigned int /*type*/, int /*stride*/, const void* /*pointer*/) {}
inline void glColorPointer(int /*size*/, unsigned int /*type*/, int /*stride*/, const void* /*pointer*/) {}
inline void glNormalPointer(unsigned int /*type*/, int /*stride*/, const void* /*pointer*/) {}
inline void glEnableClientState(unsigned int /*array*/) {}
inline void glDisableClientState(unsigned int /*array*/) {}

#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY 0x8074
#endif
#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif
#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY 0x8076
#endif
#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY 0x8075
#endif

// Display lists (no-op)
inline unsigned int glGenLists(int /*range*/) { return 0; }
inline void glNewList(unsigned int /*list*/, unsigned int /*mode*/) {}
inline void glEndList() {}
inline void glCallList(unsigned int /*list*/) {}
inline void glDeleteLists(unsigned int /*list*/, int /*range*/) {}

#ifndef GL_COMPILE
#define GL_COMPILE 0x1300
#endif
#ifndef GL_COMPILE_AND_EXECUTE
#define GL_COMPILE_AND_EXECUTE 0x1301
#endif

// Selection / feedback (no-op)
inline void glInitNames() {}
inline void glPushName(unsigned int /*name*/) {}
inline void glPopName() {}
inline void glLoadName(unsigned int /*name*/) {}
inline int glRenderMode(unsigned int /*mode*/) { return 0; }
inline void glSelectBuffer(int /*size*/, unsigned int* /*buffer*/) {}

#ifndef GL_SELECT
#define GL_SELECT 0x1C02
#endif
#ifndef GL_RENDER
#define GL_RENDER 0x1C00
#endif

// Misc legacy
inline void glRectf(float /*x1*/, float /*y1*/, float /*x2*/, float /*y2*/) {}
inline void glPixelStorei(unsigned int /*pname*/, int /*param*/) {}
inline void glPolygonMode(unsigned int /*face*/, unsigned int /*mode*/) {}
inline void glLineWidth(float /*width*/) {}
inline void glPointSize(float /*size*/) {}

#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif
#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif
#ifndef GL_POINT
#define GL_POINT 0x1B00
#endif

// Texture gen (no-op)
inline void glTexGeni(unsigned int /*coord*/, unsigned int /*pname*/, int /*param*/) {}
inline void glTexGenfv(unsigned int /*coord*/, unsigned int /*pname*/, const float* /*params*/) {}

#ifndef GL_TEXTURE_GEN_S
#define GL_TEXTURE_GEN_S 0x0C60
#endif
#ifndef GL_TEXTURE_GEN_T
#define GL_TEXTURE_GEN_T 0x0C61
#endif

// Light model constants
#ifndef GL_FLAT
#define GL_FLAT 0x1D00
#endif
#ifndef GL_SMOOTH
#define GL_SMOOTH 0x1D01
#endif
#ifndef GL_AMBIENT
#define GL_AMBIENT 0x1200
#endif
#ifndef GL_DIFFUSE
#define GL_DIFFUSE 0x1201
#endif
#ifndef GL_SPECULAR
#define GL_SPECULAR 0x1202
#endif
#ifndef GL_POSITION
#define GL_POSITION 0x1203
#endif
#ifndef GL_AMBIENT_AND_DIFFUSE
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#endif
#ifndef GL_EMISSION
#define GL_EMISSION 0x1600
#endif
#ifndef GL_SHININESS
#define GL_SHININESS 0x1601
#endif
#ifndef GL_LIGHT_MODEL_AMBIENT
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#endif

// Fog constants
#ifndef GL_FOG_MODE
#define GL_FOG_MODE 0x0B65
#endif
#ifndef GL_FOG_DENSITY
#define GL_FOG_DENSITY 0x0B62
#endif
#ifndef GL_FOG_START
#define GL_FOG_START 0x0B63
#endif
#ifndef GL_FOG_END
#define GL_FOG_END 0x0B64
#endif
#ifndef GL_FOG_COLOR
#define GL_FOG_COLOR 0x0B66
#endif
#ifndef GL_EXP
#define GL_EXP 0x0800
#endif
#ifndef GL_EXP2
#define GL_EXP2 0x0801
#endif

// Texture environment constants
#ifndef GL_TEXTURE_ENV
#define GL_TEXTURE_ENV 0x2300
#endif
#ifndef GL_TEXTURE_ENV_MODE
#define GL_TEXTURE_ENV_MODE 0x2200
#endif
#ifndef GL_MODULATE
#define GL_MODULATE 0x2100
#endif
#ifndef GL_DECAL
#define GL_DECAL 0x2101
#endif
#ifndef GL_COMBINE
#define GL_COMBINE 0x8570
#endif
#ifndef GL_COMBINE_RGB
#define GL_COMBINE_RGB 0x8571
#endif
#ifndef GL_COMBINE_ALPHA
#define GL_COMBINE_ALPHA 0x8572
#endif
#ifndef GL_ADD_SIGNED
#define GL_ADD_SIGNED 0x8574
#endif
#ifndef GL_INTERPOLATE
#define GL_INTERPOLATE 0x8575
#endif
#ifndef GL_PREVIOUS
#define GL_PREVIOUS 0x8578
#endif
#ifndef GL_PRIMARY_COLOR
#define GL_PRIMARY_COLOR 0x8577
#endif
#ifndef GL_SRC0_RGB
#define GL_SRC0_RGB 0x8580
#endif
#ifndef GL_SRC1_RGB
#define GL_SRC1_RGB 0x8581
#endif
#ifndef GL_SRC2_RGB
#define GL_SRC2_RGB 0x8582
#endif
#ifndef GL_OPERAND0_RGB
#define GL_OPERAND0_RGB 0x8590
#endif
#ifndef GL_OPERAND1_RGB
#define GL_OPERAND1_RGB 0x8591
#endif
#ifndef GL_OPERAND2_RGB
#define GL_OPERAND2_RGB 0x8592
#endif
#ifndef GL_SRC_COLOR
#define GL_SRC_COLOR 0x0300
#endif
#ifndef GL_SRC_ALPHA
#define GL_SRC_ALPHA 0x0302
#endif

// TexGen constants
#ifndef GL_S
#define GL_S 0x2000
#endif
#ifndef GL_T
#define GL_T 0x2001
#endif
#ifndef GL_TEXTURE_GEN_MODE
#define GL_TEXTURE_GEN_MODE 0x2500
#endif
#ifndef GL_SPHERE_MAP
#define GL_SPHERE_MAP 0x2402
#endif
#ifndef GL_OBJECT_PLANE
#define GL_OBJECT_PLANE 0x2501
#endif
#ifndef GL_OBJECT_LINEAR
#define GL_OBJECT_LINEAR 0x2401
#endif

#endif // __ANDROID__
