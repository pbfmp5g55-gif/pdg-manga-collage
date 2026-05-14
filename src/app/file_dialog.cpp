#include "app/file_dialog.h"

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

#include <cstring>
#include <string>
#include <vector>

namespace pdg {

namespace {

#ifdef _WIN32
// Build a comdlg32 filter blob: "Description\0patterns\0\0".
std::string buildFilter(const char* desc, const char* pats) {
    std::string out;
    out.append(desc ? desc : "Files");
    out.push_back('\0');
    out.append(pats ? pats : "*.*");
    out.push_back('\0');
    out.push_back('\0');
    return out;
}
#endif

}  // namespace

std::string OpenFile(const char* desc, const char* pats) {
#ifdef _WIN32
    char buf[MAX_PATH] = {0};
    std::string filter = buildFilter(desc, pats);

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrFile   = buf;
    ofn.nMaxFile    = MAX_PATH;
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&ofn)) return std::string(buf);
    return {};
#else
    (void)desc; (void)pats;
    return {};
#endif
}

std::vector<std::string> OpenFiles(const char* desc, const char* pats) {
#ifdef _WIN32
    // OFN_ALLOWMULTISELECT returns null-separated dir + filenames in the buffer.
    constexpr size_t kBuf = 32 * 1024;
    std::vector<char> buf(kBuf, 0);
    std::string filter = buildFilter(desc, pats);

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrFile   = buf.data();
    ofn.nMaxFile    = (DWORD)buf.size();
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                      OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    if (!GetOpenFileNameA(&ofn)) return {};

    // First entry is the directory; subsequent entries are filenames.
    // If only one file selected, the buffer holds the full path directly.
    std::vector<std::string> out;
    const char* p = buf.data();
    std::string dir(p);
    p += dir.size() + 1;
    if (*p == '\0') {
        // Single selection — `dir` is actually the full path.
        out.push_back(std::move(dir));
        return out;
    }
    while (*p) {
        std::string name(p);
        p += name.size() + 1;
        std::string full = dir;
        if (!full.empty() && full.back() != '\\' && full.back() != '/')
            full.push_back('\\');
        full += name;
        out.push_back(std::move(full));
    }
    return out;
#else
    (void)desc; (void)pats;
    return {};
#endif
}

}  // namespace pdg
