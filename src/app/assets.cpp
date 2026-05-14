#include "app/assets.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <stb_image.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>

namespace pdg {

namespace fs = std::filesystem;

namespace {

bool isImage(const fs::path& p) {
    auto ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
           ext == ".bmp" || ext == ".tga";
}

void scanDir(const fs::path& dir, const std::string& tag,
             std::vector<AssetItem>& out) {
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) return;
    for (auto& e : fs::recursive_directory_iterator(dir, ec)) {
        if (ec) break;
        if (!e.is_regular_file()) continue;
        if (!isImage(e.path())) continue;
        AssetItem item;
        item.path    = e.path().string();
        item.display = e.path().filename().string();
        item.tag     = tag;
        out.push_back(std::move(item));
    }
}

}  // namespace

AssetLibrary::~AssetLibrary() { Clear(); }

void AssetLibrary::Clear() {
    for (auto& v : {std::ref(m_photos), std::ref(m_manga)}) {
        for (auto& it : v.get()) {
            if (it.tex) {
                GLuint t = it.tex;
                glDeleteTextures(1, &t);
                it.tex = 0;
            }
        }
    }
    m_photos.clear();
    m_manga.clear();
}

void AssetLibrary::Scan() {
    Clear();

    // Photos: assets/photos/ recursive (no tag).
    scanDir(fs::path("assets") / "photos", "", m_photos);

    // Manga: assets/manga/<tag>/ — one tag per immediate subfolder.
    fs::path mangaRoot = fs::path("assets") / "manga";
    std::error_code ec;
    if (fs::is_directory(mangaRoot, ec)) {
        for (auto& tagDir : fs::directory_iterator(mangaRoot, ec)) {
            if (ec) break;
            if (!tagDir.is_directory()) continue;
            scanDir(tagDir.path(), tagDir.path().filename().string(), m_manga);
        }
    }

    std::fprintf(stderr, "[assets] scanned: %zu photos, %zu manga panels\n",
                 m_photos.size(), m_manga.size());
}

unsigned int AssetLibrary::Tex(AssetItem& item) {
    if (item.tex) return item.tex;
    int w = 0, h = 0, comp = 0;
    stbi_uc* pixels = stbi_load(item.path.c_str(), &w, &h, &comp, 4);
    if (!pixels) {
        std::fprintf(stderr, "[assets] load failed: %s\n", item.path.c_str());
        return 0;
    }
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    item.tex = tex;
    item.w = w;
    item.h = h;
    return tex;
}

}  // namespace pdg
