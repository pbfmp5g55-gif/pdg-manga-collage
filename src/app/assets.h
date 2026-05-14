#pragma once

#include <string>
#include <vector>

namespace pdg {

struct AssetItem {
    std::string  path;     // absolute or working-dir-relative path
    std::string  display;  // short name for UI
    std::string  tag;      // empty for photo, "<tag>" for manga
    int          w = 0;
    int          h = 0;
    unsigned int tex = 0;  // 0 = not uploaded yet
};

class AssetLibrary {
public:
    AssetLibrary() = default;
    ~AssetLibrary();

    AssetLibrary(const AssetLibrary&) = delete;
    AssetLibrary& operator=(const AssetLibrary&) = delete;

    // Scan assets/photos/ and assets/manga/<tag>/ relative to CWD.
    void Scan();
    void Clear();

    std::vector<AssetItem>&       photos()       { return m_photos; }
    std::vector<AssetItem>&       manga()        { return m_manga; }
    const std::vector<AssetItem>& photos() const { return m_photos; }
    const std::vector<AssetItem>& manga()  const { return m_manga; }

    // Loads the texture lazily and returns its GL id; 0 if load failed.
    unsigned int Tex(AssetItem& item);

private:
    std::vector<AssetItem> m_photos;
    std::vector<AssetItem> m_manga;
};

}  // namespace pdg
