// Minimal GL 3.3 loader for pdg-manga-collage. Adapted from ps1-vj-mix.
// Only the entry points we actually use end up in here; GLAD is too much
// machinery for this scope.

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#ifndef GL_VERSION_2_0
typedef char GLchar;
#endif
typedef ptrdiff_t GLsizeiptr_compat;

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER       0x8892
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW        0x88E4
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW       0x88E8
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER    0x8B30
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER      0x8B31
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS     0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS        0x8B82
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0           0x84C0
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE      0x812F
#endif

#ifdef _WIN32
#define PDGGL_APIENTRY __stdcall
#else
#define PDGGL_APIENTRY
#endif

typedef GLuint (PDGGL_APIENTRY *PFN_glCreateShader)(GLenum);
typedef void   (PDGGL_APIENTRY *PFN_glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*);
typedef void   (PDGGL_APIENTRY *PFN_glCompileShader)(GLuint);
typedef void   (PDGGL_APIENTRY *PFN_glGetShaderiv)(GLuint, GLenum, GLint*);
typedef void   (PDGGL_APIENTRY *PFN_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void   (PDGGL_APIENTRY *PFN_glDeleteShader)(GLuint);
typedef GLuint (PDGGL_APIENTRY *PFN_glCreateProgram)(void);
typedef void   (PDGGL_APIENTRY *PFN_glAttachShader)(GLuint, GLuint);
typedef void   (PDGGL_APIENTRY *PFN_glLinkProgram)(GLuint);
typedef void   (PDGGL_APIENTRY *PFN_glGetProgramiv)(GLuint, GLenum, GLint*);
typedef void   (PDGGL_APIENTRY *PFN_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void   (PDGGL_APIENTRY *PFN_glDeleteProgram)(GLuint);
typedef void   (PDGGL_APIENTRY *PFN_glUseProgram)(GLuint);
typedef GLint  (PDGGL_APIENTRY *PFN_glGetUniformLocation)(GLuint, const GLchar*);
typedef void   (PDGGL_APIENTRY *PFN_glUniform1i)(GLint, GLint);
typedef void   (PDGGL_APIENTRY *PFN_glUniform1f)(GLint, GLfloat);
typedef void   (PDGGL_APIENTRY *PFN_glUniform2f)(GLint, GLfloat, GLfloat);
typedef void   (PDGGL_APIENTRY *PFN_glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void   (PDGGL_APIENTRY *PFN_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void   (PDGGL_APIENTRY *PFN_glGenVertexArrays)(GLsizei, GLuint*);
typedef void   (PDGGL_APIENTRY *PFN_glDeleteVertexArrays)(GLsizei, const GLuint*);
typedef void   (PDGGL_APIENTRY *PFN_glBindVertexArray)(GLuint);
typedef void   (PDGGL_APIENTRY *PFN_glGenBuffers)(GLsizei, GLuint*);
typedef void   (PDGGL_APIENTRY *PFN_glDeleteBuffers)(GLsizei, const GLuint*);
typedef void   (PDGGL_APIENTRY *PFN_glBindBuffer)(GLenum, GLuint);
typedef void   (PDGGL_APIENTRY *PFN_glBufferData)(GLenum, GLsizeiptr_compat, const void*, GLenum);
typedef void   (PDGGL_APIENTRY *PFN_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void   (PDGGL_APIENTRY *PFN_glEnableVertexAttribArray)(GLuint);
typedef void   (PDGGL_APIENTRY *PFN_glActiveTexture)(GLenum);

extern PFN_glCreateShader            pdggl_CreateShader;
extern PFN_glShaderSource            pdggl_ShaderSource;
extern PFN_glCompileShader           pdggl_CompileShader;
extern PFN_glGetShaderiv             pdggl_GetShaderiv;
extern PFN_glGetShaderInfoLog        pdggl_GetShaderInfoLog;
extern PFN_glDeleteShader            pdggl_DeleteShader;
extern PFN_glCreateProgram           pdggl_CreateProgram;
extern PFN_glAttachShader            pdggl_AttachShader;
extern PFN_glLinkProgram             pdggl_LinkProgram;
extern PFN_glGetProgramiv            pdggl_GetProgramiv;
extern PFN_glGetProgramInfoLog       pdggl_GetProgramInfoLog;
extern PFN_glDeleteProgram           pdggl_DeleteProgram;
extern PFN_glUseProgram              pdggl_UseProgram;
extern PFN_glGetUniformLocation      pdggl_GetUniformLocation;
extern PFN_glUniform1i               pdggl_Uniform1i;
extern PFN_glUniform1f               pdggl_Uniform1f;
extern PFN_glUniform2f               pdggl_Uniform2f;
extern PFN_glUniform4f               pdggl_Uniform4f;
extern PFN_glUniformMatrix4fv        pdggl_UniformMatrix4fv;
extern PFN_glGenVertexArrays         pdggl_GenVertexArrays;
extern PFN_glDeleteVertexArrays      pdggl_DeleteVertexArrays;
extern PFN_glBindVertexArray         pdggl_BindVertexArray;
extern PFN_glGenBuffers              pdggl_GenBuffers;
extern PFN_glDeleteBuffers           pdggl_DeleteBuffers;
extern PFN_glBindBuffer              pdggl_BindBuffer;
extern PFN_glBufferData              pdggl_BufferData;
extern PFN_glVertexAttribPointer     pdggl_VertexAttribPointer;
extern PFN_glEnableVertexAttribArray pdggl_EnableVertexAttribArray;
extern PFN_glActiveTexture           pdggl_ActiveTexture;

bool pdgglLoad();
