#include "app/renderer.h"
#include "app/gl_loader.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace pdg {

namespace {

const char* kVS = R"(
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
uniform mat4 u_proj;
uniform mat4 u_model;
out vec2 v_uv;
void main() {
    gl_Position = u_proj * u_model * vec4(a_pos, 0.0, 1.0);
    v_uv = a_uv;
}
)";

const char* kFS = R"(
#version 330 core
in vec2 v_uv;
out vec4 frag;
uniform sampler2D u_tex;
uniform vec4 u_tint;
void main() {
    frag = texture(u_tex, v_uv) * u_tint;
}
)";

GLuint compile(GLenum stage, const char* src, const char* tag) {
    GLuint sh = pdggl_CreateShader(stage);
    pdggl_ShaderSource(sh, 1, &src, nullptr);
    pdggl_CompileShader(sh);
    GLint ok = 0;
    pdggl_GetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        pdggl_GetShaderInfoLog(sh, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[renderer] %s shader compile failed:\n%s\n",
                     tag, log);
        pdggl_DeleteShader(sh);
        return 0;
    }
    return sh;
}

}  // namespace

SpriteRenderer::~SpriteRenderer() { Shutdown(); }

bool SpriteRenderer::Init() {
    GLuint vs = compile(GL_VERTEX_SHADER, kVS, "vertex");
    GLuint fs = compile(GL_FRAGMENT_SHADER, kFS, "fragment");
    if (!vs || !fs) return false;

    m_program = pdggl_CreateProgram();
    pdggl_AttachShader(m_program, vs);
    pdggl_AttachShader(m_program, fs);
    pdggl_LinkProgram(m_program);
    GLint linked = 0;
    pdggl_GetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[2048];
        pdggl_GetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[renderer] program link failed:\n%s\n", log);
        pdggl_DeleteShader(vs);
        pdggl_DeleteShader(fs);
        pdggl_DeleteProgram(m_program);
        m_program = 0;
        return false;
    }
    pdggl_DeleteShader(vs);
    pdggl_DeleteShader(fs);

    m_locProj  = pdggl_GetUniformLocation(m_program, "u_proj");
    m_locModel = pdggl_GetUniformLocation(m_program, "u_model");
    m_locTex   = pdggl_GetUniformLocation(m_program, "u_tex");
    m_locTint  = pdggl_GetUniformLocation(m_program, "u_tint");

    // Unit quad centered on origin so model matrix can do center-translate.
    // pos.xy + uv.xy interleaved. UV (0,0) at TL because stb-loaded textures
    // are top-down and OpenGL's "first row in memory" lines up with sampling
    // coord v=0 — net effect: same orientation as ImGui::Image.
    const float verts[] = {
        // TL
        -0.5f, -0.5f,   0.0f, 0.0f,
        // TR
         0.5f, -0.5f,   1.0f, 0.0f,
        // BL
        -0.5f,  0.5f,   0.0f, 1.0f,
        // BR
         0.5f,  0.5f,   1.0f, 1.0f,
    };

    pdggl_GenVertexArrays(1, &m_vao);
    pdggl_BindVertexArray(m_vao);

    pdggl_GenBuffers(1, &m_vbo);
    pdggl_BindBuffer(GL_ARRAY_BUFFER, m_vbo);
    pdggl_BufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    pdggl_VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void*)0);
    pdggl_EnableVertexAttribArray(0);
    pdggl_VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void*)(2 * sizeof(float)));
    pdggl_EnableVertexAttribArray(1);

    pdggl_BindVertexArray(0);
    return true;
}

void SpriteRenderer::Shutdown() {
    if (m_vbo)     { pdggl_DeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao)     { pdggl_DeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_program) { pdggl_DeleteProgram(m_program); m_program = 0; }
}

void SpriteRenderer::Begin(int vpW, int vpH) {
    // Top-left origin orthographic: x in [0..W], y in [0..H] (y down).
    std::memset(m_proj, 0, sizeof(m_proj));
    if (vpW <= 0 || vpH <= 0) return;
    float W = (float)vpW, H = (float)vpH;
    m_proj[0]  =  2.0f / W;
    m_proj[5]  = -2.0f / H;
    m_proj[10] = -1.0f;
    m_proj[12] = -1.0f;
    m_proj[13] =  1.0f;
    m_proj[15] =  1.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    pdggl_UseProgram(m_program);
    pdggl_UniformMatrix4fv(m_locProj, 1, GL_FALSE, m_proj);
    pdggl_Uniform1i(m_locTex, 0);
    pdggl_BindVertexArray(m_vao);
}

void SpriteRenderer::DrawSprite(unsigned int tex,
                                float cx, float cy,
                                float w,  float h,
                                float rot,
                                float r, float g, float b, float a) {
    float c = std::cos(rot);
    float s = std::sin(rot);
    float model[16] = {
         c * w,  s * w, 0.0f, 0.0f,
        -s * h,  c * h, 0.0f, 0.0f,
         0.0f,   0.0f,  1.0f, 0.0f,
         cx,     cy,    0.0f, 1.0f,
    };
    pdggl_UniformMatrix4fv(m_locModel, 1, GL_FALSE, model);
    pdggl_Uniform4f(m_locTint, r, g, b, a);

    pdggl_ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void SpriteRenderer::End() {
    pdggl_BindVertexArray(0);
    pdggl_UseProgram(0);
}

}  // namespace pdg
