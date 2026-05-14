#pragma once

#include <cstdint>

namespace pdg {

// Minimal textured-quad renderer. Each DrawSprite call is a separate draw
// (no batching yet) — fine for M3 since we draw on the order of tens of
// sprites per frame, not thousands.
class SpriteRenderer {
public:
    SpriteRenderer() = default;
    ~SpriteRenderer();

    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;

    bool Init();
    void Shutdown();

    void Begin(int viewportW, int viewportH);
    void DrawSprite(unsigned int tex,
                    float cx, float cy,    // center position (px, top-left origin)
                    float w,  float h,     // size (px)
                    float rotation,         // radians, clockwise
                    float tintR, float tintG, float tintB, float tintA);
    void End();

private:
    unsigned int m_program = 0;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int m_locProj  = -1;
    int m_locModel = -1;
    int m_locTex   = -1;
    int m_locTint  = -1;
    float m_proj[16]{};
};

}  // namespace pdg
