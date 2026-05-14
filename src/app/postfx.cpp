#include "app/postfx.h"
#include "app/gl_loader.h"

#include <imgui.h>

#include <cmath>
#include <cstdio>

namespace pdg {

namespace {

const char* kPostVS = R"(
#version 330 core
layout(location = 0) in vec2 a_pos;
out vec2 v_uv;
void main() {
    gl_Position = vec4(a_pos, 0.0, 1.0);
    v_uv = a_pos * 0.5 + 0.5;
}
)";

const char* kPostFS = R"(
#version 330 core
in vec2 v_uv;
out vec4 frag;
uniform sampler2D u_tex;
uniform int       u_mode;
uniform float     u_threshold;
uniform float     u_halftone_cell;
uniform float     u_halftone_angle;
uniform int       u_posterize_levels;
void main() {
    vec4 c = texture(u_tex, v_uv);
    if (u_mode == 0) {
        frag = c;
    } else if (u_mode == 1) {
        // Halftone — screen-space dots scaled by darkness.
        float lum = dot(c.rgb, vec3(0.299, 0.587, 0.114));
        float a = u_halftone_angle;
        mat2 R = mat2(cos(a), -sin(a), sin(a), cos(a));
        vec2 p = R * gl_FragCoord.xy;
        float cell = max(2.0, u_halftone_cell);
        vec2 cp = mod(p, cell) - cell * 0.5;
        float radius = (1.0 - lum) * cell * 0.55;
        float d = length(cp);
        float dotMask = step(d, radius);
        frag = vec4(vec3(1.0 - dotMask), 1.0);
    } else if (u_mode == 2) {
        // 1-bit threshold.
        float lum = dot(c.rgb, vec3(0.299, 0.587, 0.114));
        float v = step(u_threshold, lum);
        frag = vec4(vec3(v), 1.0);
    } else if (u_mode == 3) {
        // Posterize.
        float L = float(max(2, u_posterize_levels));
        frag = vec4(floor(c.rgb * L) / L, c.a);
    } else {
        frag = c;
    }
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
        std::fprintf(stderr, "[postfx] %s shader compile failed:\n%s\n",
                     tag, log);
        pdggl_DeleteShader(sh);
        return 0;
    }
    return sh;
}

}  // namespace

PostFx::~PostFx() { Shutdown(); }

bool PostFx::Init() {
    GLuint vs = compile(GL_VERTEX_SHADER, kPostVS, "vertex");
    GLuint fs = compile(GL_FRAGMENT_SHADER, kPostFS, "fragment");
    if (!vs || !fs) return false;

    m_program = pdggl_CreateProgram();
    pdggl_AttachShader(m_program, vs);
    pdggl_AttachShader(m_program, fs);
    pdggl_LinkProgram(m_program);
    GLint linked = 0;
    pdggl_GetProgramiv(m_program, GL_LINK_STATUS, &linked);
    pdggl_DeleteShader(vs);
    pdggl_DeleteShader(fs);
    if (!linked) {
        char log[2048];
        pdggl_GetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[postfx] program link failed:\n%s\n", log);
        pdggl_DeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    m_locTex              = pdggl_GetUniformLocation(m_program, "u_tex");
    m_locMode             = pdggl_GetUniformLocation(m_program, "u_mode");
    m_locThreshold        = pdggl_GetUniformLocation(m_program, "u_threshold");
    m_locHalftoneCell     = pdggl_GetUniformLocation(m_program, "u_halftone_cell");
    m_locHalftoneAngle    = pdggl_GetUniformLocation(m_program, "u_halftone_angle");
    m_locPosterizeLevels  = pdggl_GetUniformLocation(m_program, "u_posterize_levels");

    const float verts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };
    pdggl_GenVertexArrays(1, &m_vao);
    pdggl_BindVertexArray(m_vao);
    pdggl_GenBuffers(1, &m_vbo);
    pdggl_BindBuffer(GL_ARRAY_BUFFER, m_vbo);
    pdggl_BufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    pdggl_VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                              (void*)0);
    pdggl_EnableVertexAttribArray(0);
    pdggl_BindVertexArray(0);

    return true;
}

void PostFx::Shutdown() {
    destroyTargets();
    if (m_vbo)     { pdggl_DeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao)     { pdggl_DeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_program) { pdggl_DeleteProgram(m_program); m_program = 0; }
}

void PostFx::destroyTargets() {
    if (m_fbo) { pdggl_DeleteFramebuffers(1, &m_fbo); m_fbo = 0; }
    if (m_tex) {
        GLuint t = m_tex;
        glDeleteTextures(1, &t);
        m_tex = 0;
    }
    m_W = m_H = 0;
}

void PostFx::ensureSize(int W, int H) {
    if (W == m_W && H == m_H && m_fbo && m_tex) return;
    destroyTargets();
    if (W <= 0 || H <= 0) return;

    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    pdggl_GenFramebuffers(1, &m_fbo);
    pdggl_BindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    pdggl_FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, m_tex, 0);
    GLenum status = pdggl_CheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "[postfx] FBO incomplete: 0x%x\n", status);
    }
    pdggl_BindFramebuffer(GL_FRAMEBUFFER, 0);

    m_W = W;
    m_H = H;
}

void PostFx::BeginCapture(int W, int H) {
    ensureSize(W, H);
    pdggl_BindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void PostFx::EndCaptureAndDraw(int targetW, int targetH) {
    pdggl_BindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, targetW, targetH);

    pdggl_UseProgram(m_program);
    pdggl_Uniform1i (m_locTex, 0);
    pdggl_Uniform1i (m_locMode, m_mode);
    pdggl_Uniform1f (m_locThreshold, m_threshold);
    pdggl_Uniform1f (m_locHalftoneCell, m_halftoneCell);
    pdggl_Uniform1f (m_locHalftoneAngle, m_halftoneAngleDeg * 3.14159265f / 180.0f);
    pdggl_Uniform1i (m_locPosterizeLevels, m_posterizeLevels);

    pdggl_ActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);

    glDisable(GL_BLEND);
    pdggl_BindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    pdggl_BindVertexArray(0);

    pdggl_UseProgram(0);
}

void PostFx::Draw() {
    ImGui::SetNextWindowSize(ImVec2(340, 240), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Post Effect")) { ImGui::End(); return; }

    const char* modes[] = {"None", "Halftone", "1-bit threshold", "Posterize"};
    ImGui::Combo("mode", &m_mode, modes, IM_ARRAYSIZE(modes));

    switch (m_mode) {
        case 1:
            ImGui::SliderFloat("cell (px)", &m_halftoneCell, 2.0f, 40.0f);
            ImGui::SliderFloat("angle (deg)", &m_halftoneAngleDeg, 0.0f, 90.0f);
            break;
        case 2:
            ImGui::SliderFloat("luma threshold", &m_threshold, 0.0f, 1.0f);
            break;
        case 3:
            ImGui::SliderInt("levels", &m_posterizeLevels, 2, 16);
            break;
        default: break;
    }

    ImGui::End();
}

}  // namespace pdg
