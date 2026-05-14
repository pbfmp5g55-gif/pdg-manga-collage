#include "app/gl_loader.h"

#include <cstdio>
#include <GLFW/glfw3.h>

PFN_glCreateShader            pdggl_CreateShader            = nullptr;
PFN_glShaderSource            pdggl_ShaderSource            = nullptr;
PFN_glCompileShader           pdggl_CompileShader           = nullptr;
PFN_glGetShaderiv             pdggl_GetShaderiv             = nullptr;
PFN_glGetShaderInfoLog        pdggl_GetShaderInfoLog        = nullptr;
PFN_glDeleteShader            pdggl_DeleteShader            = nullptr;
PFN_glCreateProgram           pdggl_CreateProgram           = nullptr;
PFN_glAttachShader            pdggl_AttachShader            = nullptr;
PFN_glLinkProgram             pdggl_LinkProgram             = nullptr;
PFN_glGetProgramiv            pdggl_GetProgramiv            = nullptr;
PFN_glGetProgramInfoLog       pdggl_GetProgramInfoLog       = nullptr;
PFN_glDeleteProgram           pdggl_DeleteProgram           = nullptr;
PFN_glUseProgram              pdggl_UseProgram              = nullptr;
PFN_glGetUniformLocation      pdggl_GetUniformLocation      = nullptr;
PFN_glUniform1i               pdggl_Uniform1i               = nullptr;
PFN_glUniform1f               pdggl_Uniform1f               = nullptr;
PFN_glUniform2f               pdggl_Uniform2f               = nullptr;
PFN_glUniform4f               pdggl_Uniform4f               = nullptr;
PFN_glUniformMatrix4fv        pdggl_UniformMatrix4fv        = nullptr;
PFN_glGenVertexArrays         pdggl_GenVertexArrays         = nullptr;
PFN_glDeleteVertexArrays      pdggl_DeleteVertexArrays      = nullptr;
PFN_glBindVertexArray         pdggl_BindVertexArray         = nullptr;
PFN_glGenBuffers              pdggl_GenBuffers              = nullptr;
PFN_glDeleteBuffers           pdggl_DeleteBuffers           = nullptr;
PFN_glBindBuffer              pdggl_BindBuffer              = nullptr;
PFN_glBufferData              pdggl_BufferData              = nullptr;
PFN_glVertexAttribPointer     pdggl_VertexAttribPointer     = nullptr;
PFN_glEnableVertexAttribArray pdggl_EnableVertexAttribArray = nullptr;
PFN_glActiveTexture           pdggl_ActiveTexture           = nullptr;

namespace {
template <typename T>
T fetch(const char* name, bool& ok) {
    T fn = reinterpret_cast<T>(glfwGetProcAddress(name));
    if (!fn) {
        std::fprintf(stderr, "[pdggl] missing GL entry: %s\n", name);
        ok = false;
    }
    return fn;
}
}  // namespace

bool pdgglLoad() {
    bool ok = true;
    pdggl_CreateShader            = fetch<PFN_glCreateShader>("glCreateShader", ok);
    pdggl_ShaderSource            = fetch<PFN_glShaderSource>("glShaderSource", ok);
    pdggl_CompileShader           = fetch<PFN_glCompileShader>("glCompileShader", ok);
    pdggl_GetShaderiv             = fetch<PFN_glGetShaderiv>("glGetShaderiv", ok);
    pdggl_GetShaderInfoLog        = fetch<PFN_glGetShaderInfoLog>("glGetShaderInfoLog", ok);
    pdggl_DeleteShader            = fetch<PFN_glDeleteShader>("glDeleteShader", ok);
    pdggl_CreateProgram           = fetch<PFN_glCreateProgram>("glCreateProgram", ok);
    pdggl_AttachShader            = fetch<PFN_glAttachShader>("glAttachShader", ok);
    pdggl_LinkProgram             = fetch<PFN_glLinkProgram>("glLinkProgram", ok);
    pdggl_GetProgramiv            = fetch<PFN_glGetProgramiv>("glGetProgramiv", ok);
    pdggl_GetProgramInfoLog       = fetch<PFN_glGetProgramInfoLog>("glGetProgramInfoLog", ok);
    pdggl_DeleteProgram           = fetch<PFN_glDeleteProgram>("glDeleteProgram", ok);
    pdggl_UseProgram              = fetch<PFN_glUseProgram>("glUseProgram", ok);
    pdggl_GetUniformLocation      = fetch<PFN_glGetUniformLocation>("glGetUniformLocation", ok);
    pdggl_Uniform1i               = fetch<PFN_glUniform1i>("glUniform1i", ok);
    pdggl_Uniform1f               = fetch<PFN_glUniform1f>("glUniform1f", ok);
    pdggl_Uniform2f               = fetch<PFN_glUniform2f>("glUniform2f", ok);
    pdggl_Uniform4f               = fetch<PFN_glUniform4f>("glUniform4f", ok);
    pdggl_UniformMatrix4fv        = fetch<PFN_glUniformMatrix4fv>("glUniformMatrix4fv", ok);
    pdggl_GenVertexArrays         = fetch<PFN_glGenVertexArrays>("glGenVertexArrays", ok);
    pdggl_DeleteVertexArrays      = fetch<PFN_glDeleteVertexArrays>("glDeleteVertexArrays", ok);
    pdggl_BindVertexArray         = fetch<PFN_glBindVertexArray>("glBindVertexArray", ok);
    pdggl_GenBuffers              = fetch<PFN_glGenBuffers>("glGenBuffers", ok);
    pdggl_DeleteBuffers           = fetch<PFN_glDeleteBuffers>("glDeleteBuffers", ok);
    pdggl_BindBuffer              = fetch<PFN_glBindBuffer>("glBindBuffer", ok);
    pdggl_BufferData              = fetch<PFN_glBufferData>("glBufferData", ok);
    pdggl_VertexAttribPointer     = fetch<PFN_glVertexAttribPointer>("glVertexAttribPointer", ok);
    pdggl_EnableVertexAttribArray = fetch<PFN_glEnableVertexAttribArray>("glEnableVertexAttribArray", ok);
    pdggl_ActiveTexture           = fetch<PFN_glActiveTexture>("glActiveTexture", ok);
    return ok;
}
