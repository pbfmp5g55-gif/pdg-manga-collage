#include "panel_editor.h"
#include "app/file_dialog.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

// MinGW's <GL/gl.h> only declares GL 1.1; supply post-1.1 enums we use.
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

namespace pdg {

namespace fs = std::filesystem;

PanelEditor::PanelEditor() = default;

PanelEditor::~PanelEditor() {
    freeGLTex();
}

bool PanelEditor::LoadFromPath(const std::string& path) {
    int w = 0, h = 0, comp = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &w, &h, &comp, 4);
    if (!pixels) {
        std::snprintf(m_status, sizeof(m_status),
                      "load failed: %s — %s", path.c_str(), stbi_failure_reason());
        return false;
    }

    freeGLTex();
    m_src.w = w;
    m_src.h = h;
    m_src.rgba.assign(pixels, pixels + (size_t)w * h * 4);
    stbi_image_free(pixels);

    m_loadedPath = path;
    m_crop = CropRect{0, 0, w, h};

    fs::path stem = fs::path(path).stem();
    std::snprintf(m_outName, sizeof(m_outName), "%s.png", stem.string().c_str());
    std::snprintf(m_status, sizeof(m_status), "loaded %dx%d (%d ch)", w, h, comp);

    uploadGLTex();
    return true;
}

void PanelEditor::OnDrop(int count, const char** paths) {
    if (count > 0 && paths && paths[0]) LoadFromPath(paths[0]);
}

void PanelEditor::uploadGLTex() {
    freeGLTex();
    if (m_src.w <= 0 || m_src.h <= 0) return;
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_src.w, m_src.h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_src.rgba.data());
    m_src.gl_tex = tex;
}

void PanelEditor::freeGLTex() {
    if (m_src.gl_tex) {
        GLuint t = m_src.gl_tex;
        glDeleteTextures(1, &t);
        m_src.gl_tex = 0;
    }
}

std::vector<uint8_t> PanelEditor::applyAlpha(int* outW, int* outH) const {
    int x = std::clamp(m_crop.x, 0, m_src.w);
    int y = std::clamp(m_crop.y, 0, m_src.h);
    int w = std::clamp(m_crop.w, 1, m_src.w - x);
    int h = std::clamp(m_crop.h, 1, m_src.h - y);
    *outW = w;
    *outH = h;

    std::vector<uint8_t> dst((size_t)w * h * 4, 0);

    for (int row = 0; row < h; ++row) {
        const uint8_t* sp = m_src.rgba.data() + ((size_t)(y + row) * m_src.w + x) * 4;
        uint8_t* dp = dst.data() + (size_t)row * w * 4;
        for (int col = 0; col < w; ++col, sp += 4, dp += 4) {
            uint8_t r = sp[0], g = sp[1], b = sp[2], a = sp[3];
            dp[0] = r; dp[1] = g; dp[2] = b; dp[3] = a;

            switch (m_alphaMode) {
                case AlphaMode::Keep: break;
                case AlphaMode::WhiteKey: {
                    int dr = 255 - r, dg = 255 - g, db = 255 - b;
                    if (dr <= m_whiteTol && dg <= m_whiteTol && db <= m_whiteTol) {
                        dp[3] = 0;
                    }
                    break;
                }
                case AlphaMode::BlackThreshold: {
                    int luma = (int)(0.299f * r + 0.587f * g + 0.114f * b + 0.5f);
                    if (luma <= m_threshold) {
                        dp[0] = 0; dp[1] = 0; dp[2] = 0;
                        dp[3] = 255;
                    } else {
                        dp[3] = 0;
                    }
                    break;
                }
            }
        }
    }
    return dst;
}

bool PanelEditor::saveCropped() {
    if (m_src.rgba.empty()) {
        std::snprintf(m_status, sizeof(m_status), "no image loaded");
        return false;
    }
    if (m_outName[0] == '\0') {
        std::snprintf(m_status, sizeof(m_status), "filename empty");
        return false;
    }

    int w = 0, h = 0;
    auto pixels = applyAlpha(&w, &h);

    fs::path outDir = fs::path("assets") / "manga" / m_tag;
    std::error_code ec;
    fs::create_directories(outDir, ec);

    fs::path outPath = outDir / m_outName;
    int ok = stbi_write_png(outPath.string().c_str(), w, h, 4,
                            pixels.data(), w * 4);
    if (!ok) {
        std::snprintf(m_status, sizeof(m_status),
                      "write failed: %s", outPath.string().c_str());
        return false;
    }
    std::snprintf(m_status, sizeof(m_status),
                  "saved %s (%dx%d)", outPath.string().c_str(), w, h);
    return true;
}

void PanelEditor::Draw(bool* p_open) {
    ImGui::SetNextWindowSize(ImVec2(720, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Panel Editor", p_open)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Open image...")) {
        std::string p = OpenFile("Image files",
                                 "*.png;*.jpg;*.jpeg;*.bmp;*.tga");
        if (!p.empty()) LoadFromPath(p);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(or drag onto window)");

    if (m_src.gl_tex == 0) {
        if (m_status[0]) {
            ImGui::Separator();
            ImGui::TextWrapped("%s", m_status);
        }
        ImGui::End();
        return;
    }

    ImGui::Text("Source: %s  (%d x %d)", m_loadedPath.c_str(), m_src.w, m_src.h);
    ImGui::Separator();

    ImGui::Text("Crop");
    ImGui::DragInt("x", &m_crop.x, 1.0f, 0, m_src.w);
    ImGui::DragInt("y", &m_crop.y, 1.0f, 0, m_src.h);
    ImGui::DragInt("w", &m_crop.w, 1.0f, 1, m_src.w);
    ImGui::DragInt("h", &m_crop.h, 1.0f, 1, m_src.h);
    if (ImGui::Button("Reset crop to full")) {
        m_crop = CropRect{0, 0, m_src.w, m_src.h};
    }

    ImGui::Separator();
    ImGui::Text("Alpha mode");
    int mode = (int)m_alphaMode;
    const char* modes[] = {
        "Keep source alpha",
        "White-key (white -> transparent)",
        "Black threshold (dark -> opaque silhouette)",
    };
    if (ImGui::Combo("##alpha_mode", &mode, modes, IM_ARRAYSIZE(modes))) {
        m_alphaMode = (AlphaMode)mode;
    }
    if (m_alphaMode == AlphaMode::WhiteKey) {
        ImGui::SliderInt("white tolerance", &m_whiteTol, 0, 128);
    } else if (m_alphaMode == AlphaMode::BlackThreshold) {
        ImGui::SliderInt("luma threshold", &m_threshold, 0, 255);
    }

    ImGui::Separator();
    ImGui::Text("Save");
    ImGui::InputText("tag",      m_tag,     IM_ARRAYSIZE(m_tag));
    ImGui::InputText("filename", m_outName, IM_ARRAYSIZE(m_outName));
    ImGui::TextDisabled("output: assets/manga/<tag>/<filename>");
    if (ImGui::Button("Save cropped + alpha PNG")) {
        saveCropped();
    }

    if (m_status[0]) {
        ImGui::Separator();
        ImGui::TextWrapped("%s", m_status);
    }

    ImGui::Separator();
    ImGui::Text("Preview (source, no alpha overlay)");
    float maxW = ImGui::GetContentRegionAvail().x;
    float scale = (m_src.w > 0) ? std::min(1.0f, maxW / (float)m_src.w) : 1.0f;
    ImVec2 size((float)m_src.w * scale, (float)m_src.h * scale);
    ImGui::Image((ImTextureID)(intptr_t)m_src.gl_tex, size);

    ImGui::End();
}

}  // namespace pdg
