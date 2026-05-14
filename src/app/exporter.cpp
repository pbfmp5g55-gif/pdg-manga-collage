#include "app/exporter.h"

#include <imgui.h>
#include <stb_image_write.h>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>

namespace pdg {

namespace fs = std::filesystem;

void ExportPanel::Draw() {
    ImGui::SetNextWindowSize(ImVec2(360, 180), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Export")) { ImGui::End(); return; }

    ImGui::TextWrapped("Saves the current preview (post-fx applied) "
                       "to a PNG. High-resolution / T-shirt template "
                       "land in M5.5.");

    ImGui::InputText("path", m_filename, IM_ARRAYSIZE(m_filename));
    if (ImGui::Button("Save preview as PNG")) {
        m_path    = m_filename;
        m_request = true;
    }
    if (m_status[0]) {
        ImGui::Separator();
        ImGui::TextWrapped("%s", m_status);
    }
    ImGui::End();
}

void ExportPanel::SetStatus(const char* msg) {
    if (!msg) { m_status[0] = '\0'; return; }
    std::snprintf(m_status, sizeof(m_status), "%s", msg);
}

bool ExportPanel::SavePNG(const unsigned char* rgba_bottom_up,
                          int w, int h) {
    if (!rgba_bottom_up || w <= 0 || h <= 0) {
        SetStatus("invalid pixel buffer");
        return false;
    }
    if (m_path.empty()) {
        SetStatus("export path empty");
        return false;
    }

    // glReadPixels gives us bottom-up; PNG wants top-down.
    std::vector<unsigned char> flipped((size_t)w * h * 4);
    int rowBytes = w * 4;
    for (int row = 0; row < h; ++row) {
        std::memcpy(flipped.data() + (size_t)row * rowBytes,
                    rgba_bottom_up + (size_t)(h - 1 - row) * rowBytes,
                    rowBytes);
    }

    fs::path out(m_path);
    std::error_code ec;
    if (out.has_parent_path()) fs::create_directories(out.parent_path(), ec);

    int ok = stbi_write_png(out.string().c_str(), w, h, 4,
                            flipped.data(), rowBytes);
    if (!ok) {
        char msg[256];
        std::snprintf(msg, sizeof(msg), "write failed: %s",
                      out.string().c_str());
        SetStatus(msg);
        return false;
    }
    char msg[256];
    std::snprintf(msg, sizeof(msg), "saved %s (%dx%d)",
                  out.string().c_str(), w, h);
    SetStatus(msg);
    return true;
}

}  // namespace pdg
