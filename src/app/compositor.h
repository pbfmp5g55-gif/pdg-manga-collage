#pragma once

#include <cstdint>
#include <vector>

namespace pdg {

class AssetLibrary;
class SpriteRenderer;

struct PanelPlacement {
    int   mangaIndex = -1;  // index into AssetLibrary::manga()
    float cx = 0.0f, cy = 0.0f;
    float scale = 1.0f;
    float rotation = 0.0f;  // radians
    float alpha = 1.0f;
};

class Compositor {
public:
    void Draw(AssetLibrary& lib);                              // ImGui controls window
    void Render(SpriteRenderer& r, AssetLibrary& lib,
                int viewportW, int viewportH);                 // GL draw
    void RegenerateIfDirty(AssetLibrary& lib, int W, int H);

    void MarkDirty() { m_dirty = true; }

private:
    void generate(AssetLibrary& lib, int W, int H);

    int      m_bgIdx        = -1;
    int      m_panelCount   = 8;
    uint64_t m_seed         = 0xC011A6Eull;
    float    m_panelScale   = 0.4f;
    float    m_scaleVar     = 0.4f;     // 0..1 fraction
    float    m_rotVarTurns  = 0.08f;    // ±N turns (1 turn = 2π)
    bool     m_dirty        = true;
    std::vector<PanelPlacement> m_panels;
};

}  // namespace pdg
