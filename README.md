# pdg-manga-collage

Manga + photo chance-based collage generator for personal prints (flyers, T-shirts).

Spiritual sibling of [ps1-vj-mix](https://github.com/pbfmp5g55-gif/ps1-vj-mix) and [ps1-primitive-vj](https://github.com/pbfmp5g55-gif/ps1-primitive-vj). Reuses **libvj** (Filter / AutoMode / Preset Bank / MIDI / per-prim glitch axes) but swaps the live PS1 primitive stream for a static image source: photo backgrounds + manga panel cut-outs, composited with chance-driven layout, then exported at print resolution.

## Status

**M1**: window + ImGui skeleton (this commit). Builds and opens a 1280×720 window with a placeholder panel.

## Roadmap

| M | Scope |
|---|-------|
| M1 | window + ImGui + build skeleton |
| M2 | in-tool panel editor (rect crop + 3-way alpha: white-key / black-threshold / flood-fill), tag system, project asset folder |
| M3 | 4-layer composition (BG photo / photo glitch / manga collage / overlay) + libvj effects per layer |
| M4 | manga-specific effects (halftone dither, 集中線 generator, 1-bit / 2-bit threshold, palette reduction) |
| M5 | high-resolution FBO export (4K–8K PNG) + T-shirt template (印刷可能 area overlay, palette-reduced silkscreen output) |
| M6 (任意) | seed bank, preset saver, MIDI exploration UI |

## Build

```sh
git submodule update --init --recursive
cmake -S . -B build
cmake --build build --config Release
./build/pdg-collage  # or build\Release\pdg-collage.exe on MSVC
```

Requires CMake ≥ 3.20, a C++17 compiler, and OpenGL 3.3.

## License

MIT (see `LICENSE`). libvj submodule is also MIT.
