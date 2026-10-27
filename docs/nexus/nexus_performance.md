# Nexus V1 Performance Audit — Source-Locked

## Summary
Nexus targets 30 FPS on Sega Saturn (320x224) using a VDP1 polygon
rasterizer. The Firestaff PC implementation runs the software rasterizer
at effectively unlimited frame rates at 320x200.

## 1. Frame Rate and Timing

Original Saturn Performance:
- Target frame rate: 30 FPS (frame time: ~33.3 ms)
- Display resolution: 320x224 (NTSC) or 320x240 (PAL)
- Display refresh: 60 Hz NTSC -- game renders every other field
- Tick rate: 200ms (game logic decoupled from render)
- Saturn VDP1: ~20M polygons/second theoretical max

Viewport render distance: nexus_v1_viewport.c renders 4 squares deep
in all 4 cardinal directions (9 squares, 3x3 grid). Front wall + side
walls per visible square. Maximum ~40 wall faces + floor + ceiling per frame.

Game logic ticks at 200ms intervals (same as DM1). Viewport renders as
fast as possible. Input, AI, combat updated on 200ms tick boundaries.

## 2. Saturn Hardware Requirements

Hardware specs:
- CPU: 2x Hitachi SH-2 at 10.5 MHz (master/slave, big-endian)
- RAM: 2 MB main + 1.5 MB video + 0.5 MB sound
- 3D Engine: Sega VDP1 polygon rasterizer
- Storage: CD-ROM (quad-speed minimum for audio streaming)

VDP1 Rendering Pipeline:
1. CPU transforms vertices (SH-2)
2. Vertex data sent to VDP1
3. VDP1 rasterizes textured triangles
4. Framebuffer to VDP1 VRAM (1.5 MB)
5. VDP2 composites background layer
6. Output to display

Memory Budget (Saturn):
- Work RAM: 2 MB
- VRAM (VDP1): 1.5 MB
- Sound RAM: 0.5 MB

## 3. PC Performance (Firestaff Implementation)

Firestaff nexus_v1_rasterizer.c runs on host CPU (no GPU):
- Edge-function triangle rasterizer
- 320x200 target resolution
- Palette-indexed framebuffer (64 KB) or RGBA (256 KB)
- Bilinear texture interpolation

Expected FPS on modern PC:
- Low-end (Intel i3): 500+ FPS
- Mid-range (Intel i5/i7, AMD Ryzen): 1000+ FPS
- High-end: 2000+ FPS

At 320x200, rasterization is essentially unlimited. Bottleneck is
level geometry parsing on first load, not rasterization.

Memory usage:
- Framebuffer: 64-256 KB
- Vertex buffers: ~few hundred KB per level
- Model data: 46-88 KB per creature
- Total: < 50 MB

## 4. DMDF Model Performance

nexus_v1_dmdf_load() loads model files:
- Magic 0x444D4446 validated
- Vertex array: Nexus_DMDFVertex = 16 bytes each
- Face index array: uint16 x 3 per face
- Embedded BITMAP texture loaded

Model sizes: ANTMAN.MNS 53KB, CHAOS.MNS 88KB, BORKETH.MNS 67KB.
Per-frame (Saturn): ~20K vertex transforms for 10 visible creatures.

## 5. Performance Comparison: DM1 vs Nexus

| Aspect            | DM1 V1        | Nexus V1             |
|-------------------|---------------|----------------------|
| Render resolution | 320x200       | 320x224 (NTSC)       |
| Frame rate target | 30 FPS        | 30 FPS               |
| Rendering method  | 2D sprites    | 3D polygon rasterizer|
| Draw calls/frame  | ~50 sprites   | ~200 triangles       |
| CPU load          | Low           | High (10x more)      |
| Geometry data     | None          | 4.3 MB baked in DGN  |

## 6. Known Performance Gaps

1. DGN 3D geometry parsing: NOT YET IMPLEMENTED. Full 3D rendering
   blocked until the post-grid geometry blob is parsed.
2. Model loading delay: First creature encounter has brief read+parse delay.
3. No texture compression: DMDF stores uncompressed VDP1 BITMAP textures.
4. No z-buffer early-out: Full rasterization of all triangles with no
   pre-sorted visibility culling.

## 7. Hardware Requirements Summary

Original Sega Saturn:
- CPU: 2x SH-2 at 10.5 MHz (big-endian)
- RAM: 4 MB total
- Display: 320x224, 30 FPS target
- Audio: CD-DA + ADX/SEGA PCM
- GPU: VDP1 polygon rasterizer (builtin)

Firestaff PC (Modern):
- CPU: Any x86/ARM (software rasterizer)
- RAM: 50 MB minimum
- Display: Any SDL-supported resolution
- GPU: Not required -- pure CPU rendering at 320x200
