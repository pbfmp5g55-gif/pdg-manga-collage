#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace pdg {

enum class AlphaMode : int {
    Keep = 0,         // leave source alpha as-is
    WhiteKey = 1,     // pixels close to white become transparent
    BlackThreshold = 2, // pixels darker than threshold become opaque, rest transparent
};

struct LoadedImage {
    int w = 0;
    int h = 0;
    std::vector<uint8_t> rgba;  // tightly packed, 4 bytes per pixel
    unsigned int gl_tex = 0;     // 0 = not uploaded
};

struct CropRect {
    int x = 0, y = 0;
    int w = 0, h = 0;
};

class PanelEditor {
public:
    PanelEditor();
    ~PanelEditor();

    PanelEditor(const PanelEditor&) = delete;
    PanelEditor& operator=(const PanelEditor&) = delete;

    bool LoadFromPath(const std::string& path);
    void OnDrop(int count, const char** paths);

    // Renders the editor ImGui window. Caller controls visibility.
    void Draw(bool* p_open = nullptr);

private:
    void uploadGLTex();
    void freeGLTex();
    std::vector<uint8_t> applyAlpha(int* outW, int* outH) const;
    bool saveCropped();

    LoadedImage  m_src;
    CropRect     m_crop;
    AlphaMode    m_alphaMode = AlphaMode::WhiteKey;
    int          m_threshold = 240;     // 0..255, used by BlackThreshold
    int          m_whiteTol = 16;       // 0..128, used by WhiteKey
    std::string  m_loadedPath;
    char         m_tag[64] = "character";
    char         m_outName[128] = "";
    char         m_status[256] = "";
};

}  // namespace pdg
