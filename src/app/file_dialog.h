#pragma once

#include <string>
#include <vector>

namespace pdg {

// Returns the selected path, or empty if the user cancelled.
// `filterDescription` shown in dialog (e.g. "Image files");
// `filterPatterns` is a semicolon list (e.g. "*.png;*.jpg;*.jpeg;*.bmp;*.tga").
std::string OpenFile(const char* filterDescription,
                     const char* filterPatterns);

// Multi-select variant.
std::vector<std::string> OpenFiles(const char* filterDescription,
                                   const char* filterPatterns);

}  // namespace pdg
