#include "app/compositor.h"
#include "app/assets.h"
#include "app/file_dialog.h"
#include "app/renderer.h"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <random>

namespace pdg {

namespace fs = std::filesystem;

namespace {
constexpr float kTwoPi = 6.28318530718f;

// Copy `srcPaths` into `assets/<subdir>/`, creating the dir if needed.
// Returns the count of successful copies.
int importInto(const std::vector<std::string>& srcPaths,
               const std::string& subdir) {
    fs::path dir = fs::path("assets") / subdir;
    std::error_code ec;
    fs::create_directories(dir, ec);
    int n = 0;
    for (auto& src : srcPaths) {
        fs::path s(src);
        fs::path d = dir / s.filename();
        fs::copy_file(s, d, fs::copy_options::overwrite_existing, ec);
        if (!ec) ++n;
    }
    return n;
}
}

void Compositor::generate(AssetLibrary& lib, int W, int H) {
    m_panels.clear();
    if (lib.manga().empty() || m_panelCount <= 0) return;

    std::mt19937_64 rng(m_seed);
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);
    std::uniform_int_distribution<int> pick(0, (int)lib.manga().size() - 1);

    for (int i = 0; i < m_panelCount; ++i) {
        PanelPlacement p;
        p.mangaIndex = pick(rng);
        p.cx       = u01(rng) * (float)W;
        p.cy       = u01(rng) * (float)H;
        float sJitter = 1.0f + (u01(rng) * 2.0f - 1.0f) * m_scaleVar;
        p.scale    = std::max(0.05f, m_panelScale * sJitter);
        p.rotation = (u01(rng) * 2.0f - 1.0f) * m_rotVarTurns * kTwoPi;
        p.alpha    = 1.0f;
        m_panels.push_back(p);
    }
}

void Compositor::RegenerateIfDirty(AssetLibrary& lib, int W, int H) {
    if (!m_dirty) return;
    generate(lib, W, H);
    m_dirty = false;
}

void Compositor::Render(SpriteRenderer& r, AssetLibrary& lib,
                        int W, int H) {
    r.Begin(W, H);

    // BG layer: 1 photo, aspect-fit centered.
    if (m_bgIdx >= 0 && m_bgIdx < (int)lib.photos().size()) {
        auto& bg = lib.photos()[m_bgIdx];
        unsigned int tex = lib.Tex(bg);
        if (tex && bg.w > 0 && bg.h > 0) {
            float iar = (float)bg.w / (float)bg.h;
            float var = (float)W / (float)H;
            float tw, th;
            if (iar > var) { tw = (float)W; th = tw / iar; }
            else           { th = (float)H; tw = th * iar; }
            r.DrawSprite(tex, W * 0.5f, H * 0.5f, tw, th, 0.0f,
                         1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    // Manga collage layer.
    for (auto& p : m_panels) {
        if (p.mangaIndex < 0 || p.mangaIndex >= (int)lib.manga().size()) continue;
        auto& m = lib.manga()[p.mangaIndex];
        unsigned int tex = lib.Tex(m);
        if (!tex || m.w <= 0 || m.h <= 0) continue;
        float tw = (float)m.w * p.scale;
        float th = (float)m.h * p.scale;
        r.DrawSprite(tex, p.cx, p.cy, tw, th, p.rotation,
                     1.0f, 1.0f, 1.0f, p.alpha);
    }

    r.End();
}

void Compositor::Draw(AssetLibrary& lib) {
    ImGui::SetNextWindowSize(ImVec2(380, 480), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Compose")) { ImGui::End(); return; }

    if (ImGui::Button("Rescan assets/")) lib.Scan();
    ImGui::SameLine();
    ImGui::TextDisabled("(%zu photos, %zu manga)",
                        lib.photos().size(), lib.manga().size());

    if (ImGui::Button("+ photos...")) {
        auto picks = OpenFiles("Image files",
                               "*.png;*.jpg;*.jpeg;*.bmp;*.tga");
        if (!picks.empty()) {
            int n = importInto(picks, "photos");
            std::fprintf(stderr, "[compose] imported %d photos\n", n);
            lib.Scan();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("+ manga (no edit)...")) {
        auto picks = OpenFiles("Image files",
                               "*.png;*.jpg;*.jpeg;*.bmp;*.tga");
        if (!picks.empty()) {
            int n = importInto(picks, "manga/character");
            std::fprintf(stderr, "[compose] imported %d manga panels\n", n);
            lib.Scan();
        }
    }

    ImGui::Separator();
    ImGui::Text("Background");
    const char* bgName = (m_bgIdx >= 0 && m_bgIdx < (int)lib.photos().size())
                            ? lib.photos()[m_bgIdx].display.c_str()
                            : "<none>";
    if (ImGui::BeginCombo("##bg", bgName)) {
        if (ImGui::Selectable("<none>", m_bgIdx < 0)) m_bgIdx = -1;
        for (int i = 0; i < (int)lib.photos().size(); ++i) {
            bool sel = (m_bgIdx == i);
            if (ImGui::Selectable(lib.photos()[i].display.c_str(), sel))
                m_bgIdx = i;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();
    ImGui::Text("Manga collage");
    bool changed = false;
    {
        int seedLo = (int)(m_seed & 0xFFFFFFFFu);
        if (ImGui::InputInt("seed (low32)", &seedLo, 1, 100,
                            ImGuiInputTextFlags_EnterReturnsTrue)) {
            m_seed = (m_seed & 0xFFFFFFFF00000000ull) | (uint32_t)seedLo;
            changed = true;
        }
    }
    if (ImGui::Button("Reroll seed")) {
        std::random_device rd;
        m_seed = ((uint64_t)rd() << 32) ^ rd();
        changed = true;
    }
    ImGui::SameLine();
    ImGui::Text("0x%016llx", (unsigned long long)m_seed);

    if (ImGui::SliderInt("panels",        &m_panelCount,    0, 64))   changed = true;
    if (ImGui::SliderFloat("scale",        &m_panelScale,   0.05f, 2.0f)) changed = true;
    if (ImGui::SliderFloat("scale var",    &m_scaleVar,     0.0f,  1.0f)) changed = true;
    if (ImGui::SliderFloat("rotation var", &m_rotVarTurns,  0.0f,  0.5f)) changed = true;

    if (ImGui::Button("Regenerate")) changed = true;

    if (changed) m_dirty = true;

    ImGui::Separator();
    ImGui::Text("Placements: %zu", m_panels.size());

    ImGui::End();
}

}  // namespace pdg
