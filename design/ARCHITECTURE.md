# pdg-manga-collage — Architecture sketch

## Goal

Generate single-frame collage images for personal print use (flyers, T-shirts), driven by chance + MIDI knobs, using a manga-panel-centric aesthetic with photo chaos as supporting texture.

This is **not a live VJ tool**. It's a parameterized still-image generator that benefits from real-time preview but ultimately exports a single high-resolution PNG / PDF.

## Inputs

- **Photo source folder** (BG / texture material)
- **Manga panel folder** — alpha PNGs cut from scans, tagged by role (BG / character / ベタ / トーン / 集中線 / 吹き出し)
- **Seed** — single 64-bit integer that drives all randomness; lock for reproducible export

## Composition (4 layers, bottom → top)

1. **BG photo layer** — 1 photo, optional blur / 1-bit threshold / palette reduction
2. **Photo glitch layer** — sampled regions of the same / other photos, libvj effects (jitter / color / UV warp)
3. **Manga panel collage layer** (the star) — N panels placed by layout strategy (grid / random / poster), per-panel rotation / scale / blend mode
4. **Overlay layer** — procedural 集中線, text, 吹き出し template

Each layer feeds primitives into the renderer the same way ps1-vj-mix does, so libvj's Params / FilterParams / AutoMode / Preset Bank / MIDI panel apply per-layer with no protocol change.

## Renderer

OpenGL 3.3 + custom shader pipeline (no fixed function). FBO render at preview resolution (1280×720) by default. Export switches to 4K / 6K / 8K FBO and reads back to PNG.

T-shirt mode: target dimensions in cm, DPI 300, palette reduction stage (ordered dither → N-color quantize → silkscreen-friendly PNG with separated channels optional).

## Reproducibility

All random draws (panel choice, position jitter, glitch dice rolls) seed from a single uint64. `seed` is a UI input. "I like this frame" → click Lock → frame and seed are saved to a project file. Re-render at any later time / resolution from that seed.

## Reused from ps1-vj-mix / libvj

- libvj `Params` (8 glitch axes), `FilterParams` (per-prim filter), `AutoMode` (LFO modulation), `FilterPresetBank` (16-slot bank with MIDI knob morph)
- `RtMidiController` for MIDI exploration during preview
- ImGui panels for parameter exposure
- Shader vocabulary: PS1-style CLUT noise, ABR sub-modes, 4×4 Bayer dither

## New for this project

- Image loader (stb_image)
- Panel editor (rect crop + 3-way alpha generation)
- Layer composition graph
- Halftone dither / 集中線 generator / palette reducer
- High-res FBO export
- T-shirt template overlay + print preset
- Seed lock / project save format
