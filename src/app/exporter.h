#pragma once

#include <string>

namespace pdg {

class ExportPanel {
public:
    // Renders ImGui controls. Sets `requestExport` to true on click;
    // caller is expected to perform the actual save after the next
    // frame's composite finishes.
    void Draw();

    bool ShouldExport()      const { return m_request; }
    void ConsumeExport()           { m_request = false; }
    const std::string& Path() const { return m_path; }

    // Writes the bottom-up RGBA buffer at (w, h) to the configured
    // path as a PNG (top-down). Returns true on success and updates
    // status text.
    bool SavePNG(const unsigned char* rgba_bottom_up, int w, int h);

    void SetStatus(const char* msg);

private:
    char        m_filename[256] = "exports/snapshot.png";
    bool        m_request = false;
    std::string m_path;
    char        m_status[256] = "";
};

}  // namespace pdg
