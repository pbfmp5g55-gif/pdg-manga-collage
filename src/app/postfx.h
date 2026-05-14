#pragma once

namespace pdg {

// Renders the scene into an off-screen color texture, then composites
// it back to the default framebuffer through a fragment shader that
// applies one of: pass-through, halftone, 1-bit threshold, posterize.
class PostFx {
public:
    PostFx() = default;
    ~PostFx();

    PostFx(const PostFx&) = delete;
    PostFx& operator=(const PostFx&) = delete;

    bool Init();
    void Shutdown();

    // Binds the FBO and (re)creates the color target at the requested
    // size if needed. Caller still owns glViewport / glClear.
    void BeginCapture(int W, int H);

    // Unbinds FBO and draws the captured texture full-screen with the
    // active effect.
    void EndCaptureAndDraw(int targetW, int targetH);

    void Draw();  // ImGui controls

    int  mode() const { return m_mode; }

private:
    void ensureSize(int W, int H);
    void destroyTargets();

    unsigned int m_fbo = 0;
    unsigned int m_tex = 0;
    int          m_W = 0, m_H = 0;

    unsigned int m_program = 0;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int          m_locTex = -1;
    int          m_locMode = -1;
    int          m_locThreshold = -1;
    int          m_locHalftoneCell = -1;
    int          m_locHalftoneAngle = -1;
    int          m_locPosterizeLevels = -1;

    int   m_mode = 0;
    float m_threshold = 0.5f;
    float m_halftoneCell = 8.0f;
    float m_halftoneAngleDeg = 45.0f;
    int   m_posterizeLevels = 4;
};

}  // namespace pdg
