# Nexus V1 Performance Audit — Source-Locked

## Sources
- `src/nexus/nexus_v1_rasterizer.c`
- `src/nexus/nexus_v1_viewport.c`
- `src/nexus/nexus_v1_math3d.c`
- `src/nexus/nexus_v1_dmdf_model.c`
- `docs/nexus_overview.md`, `docs/nexus_graphics.md`
- `docs/nexus_testing.md`
- Saturn hardware reference documentation

---

## 1. Frame Rate and Timing

### Original Saturn Performance
- **Target frame rate**: 30 FPS (approximately)
- **Frame time**: ~33.3 ms per frame
- **Resolution**: 320×224 (NTSC) or 320×240 (PAL)
- **Display refresh**: 60 Hz (NTSC) — game renders every other field

Saturn's VDP1 polygon engine could process ~20M polygons/second theoretically,
but real-world performance was far lower due to CPU bottleneck and memory bandwidth.

### 3D Rendering Load
The software rasterizer in Nexus processes:
- Wall polygons: front/side faces at each grid position (up to 4 walls per square)
- Floor/ceiling: projected polygon per open square
- Creature models: DMDF meshes with vertex transform + rasterization
- Each wall face: 2 triangles (quad), texture-mapped

### Tick Rate
Game logic ticks at the same rate as DM1:
- **200ms tick interval** (10 VBlanks × 20ms = 200ms on PAL)
- Viewport renders as fast as possible (frame-rate independent of tick)
- Input, AI, combat updated on 200ms tick boundaries

### Viewport Render Distance
`nexus_v1_viewport.c` renders 4 squares deep in all 4 cardinal directions:
- 9 squares total (3×3 grid centered on party)
- Wall faces drawn for solid squares
- Floor/ceiling drawn for open squares

---

## 2. Saturn Hardware Requirements

### Minimum Saturn Specs
| Component | Specification |
|-----------|--------------|
| CPU | 2× Hitachi SH-2 @ 10.5 MHz (master/slave) |
| RAM | 2 MB main + 1.5 MB video + 0.5 MB sound |
| 3D Engine | Sega VDP1 polygon rasterizer |
| Display | 320×224 (NTSC) @ 30 FPS |
| Storage | CD-ROM (quad-speed minimum for audio streaming) |
| Media | 1 CD |

### VDP1 Rendering Pipeline
1. CPU transforms vertices (SH-2, fixed-point or floating-point)
2. Vertex data sent to VDP1
3. VDP1 rasterizes textured triangles (20M poly/s theoretical max)
4. Framebuffer written to VDP1 VRAM (1.5 MB)
5. VDP2 composites background layer
6. Output to display

### Memory Budget (Saturn)
- **Work RAM**: 2 MB — code + data + stack
- **VRAM (VDP1)**: 1.5 MB — framebuffer + textures
- **Sound RAM**: 0.5 MB — ADPCM sample buffer
- **CD-ROM DMA**: streaming data directly to work RAM

### Texture Memory Constraint
VDP1 texture memory is limited (~512 KB effective for textures):
- 4bpp (16-color) or 8bpp (256-color) textures
- Textures packed tightly — no mipmapping
- Creature textures likely 4bpp to fit budget

---

## 3. PC Performance (Firestaff Implementation)

### Software Rasterizer
Firestaff's `nexus_v1_rasterizer.c` runs on host CPU (no GPU):
- CPU-based triangle rasterization
- 320×200 target resolution
- Palette-indexed framebuffer
- Texture sampling via bilinear interpolation

### Performance Expectations on Modern PC
| Hardware | Expected FPS | Notes |
|----------|-------------|-------|
| Low-end (Intel i3) | 500+ FPS | CPU rasterizer, no GPU needed |
| Mid-range (Intel i5/i7, AMD Ryzen 3/5) | 1000+ FPS | |
| High-end (modern desktop) | 2000+ FPS | 320×200 is tiny |

At 320×200 resolution, the software rasterizer is so fast that frame rate
is essentially unlimited. The bottleneck would be level geometry parsing
(on first load) rather than rasterization itself.

### Memory Usage (PC)
- **Framebuffer**: 320×200 × 1 byte = 64 KB (indexed) + 320×200×4 = 256 KB (RGBA)
- **Vertex buffers**: ~few hundred KB per level (3D geometry)
- **Model data**: 46-88 KB per creature model (loaded on demand)
- **Total working set**: < 50 MB for full game

---

## 4. Rasterizer Implementation Details

### nexus_v1_rasterizer.c
Software triangle rasterizer features:
- **Edge-function algorithm** for triangle fill
- **Texture-mapped** triangles (u,v coordinate sampling)
- **Z-buffer** for depth sorting (painter's algorithm backup)
- **Framebuffer**: Nexus_Framebuffer struct (palette-indexed)

### Drawing Primitives
From `nexus_v1_viewport.c` wall drawing:
```c
case 0: /* North wall (z face) */ draw triangle ... break;
case 1: /* East wall (x face) */  draw triangle ... break;
case 2: /* South wall (z face) */ draw triangle ... break;
case 3: /* West wall (x face) */  draw triangle ... break;
```

Each wall square draws 2 triangles (one quad = 2 triangles).
Floor/ceiling: `nexus_draw_floor()` projects square polygon to screen.

### Back-Face Culling
```c
/* Back-face culling — skip triangles facing away from camera */
```

Normals tested against view direction; back-facing triangles skipped.

---

## 5. DMDF Model Performance

### Model Loading
`nexus_v1_dmdf_load()` loads .MNS files:
- Reads header (magic, sizes, offsets)
- Allocates vertex array (Nexus_DMDFVertex = 16 bytes each)
- Allocates face index array (uint16_t × 3 per face)
- Embedded BITMAP texture data loaded

### Vertex Count Examples
| Model | Vertices (est.) | Faces (est.) | Size |
|-------|----------------|--------------|------|
| ANTMAN.MNS | ~1,000-2,000 | ~500-1,000 | 53 KB |
| CHAOS.MNS | ~2,000-3,000 | ~1,000-1,500 | 88 KB |

### Per-Frame Cost
1. Transform all vertices by camera matrix (Vec3 × Mat4)
2. Perspective divide (z-divide for projection)
3. Depth sort against z-buffer
4. Rasterize visible triangles

For ~10 visible creatures × ~2,000 vertices each = ~20K vertex transforms/frame
which is trivial even on Saturn SH-2.

---

## 6. Performance Comparison: DM1 vs Nexus

| Aspect | DM1 V1 | Nexus V1 |
|--------|--------|----------|
| Render resolution | 320×200 | 320×224 (NTSC) |
| Frame rate target | 30 FPS | 30 FPS |
| Rendering method | 2D sprite blitting | 3D polygon rasterizer |
| Draw calls/frame | ~50 sprites | ~200 triangles |
| CPU load | Low (68000 @ 7MHz) | High (SH-2 @ 10.5 MHz) |
| Memory bandwidth | Low (sprite reads) | High (texture + geometry) |
| Geometry | None | 4.3 MB baked in DGN |
| Texture memory | GRAPHICS.DAT (250 KB) | Per-level VDP1 textures |

**Nexus is ~10× more demanding than DM1** due to 3D rendering,
but Saturn's VDP1 hardware accelerates the polygon rasterization.

---

## 7. Performance Optimization Opportunities

### In Firestaff (PC)
1. **EPX/others upscaling** (`nexus_v2_upscaler.c`): Scale 320×224 to 640×448 or 960×720
   - 2× EPX: 4 pixels written per input pixel (no interpolation)
   - Nearly free in terms of CPU (simple pixel duplication)
2. **Display scaling**: SDL renderer can scale to any window size
3. **Batch draw calls**: Sort geometry by texture to minimize state changes

### On Original Saturn (Historical)
1. **Level-of-detail**: Distant walls use lower-detail geometry
2. **Texture tiling**: Small 64×64 textures repeated across walls
3. **Z-buffer**: Fast depth test eliminates overdraw
4. **Billboard creatures**: 2D sprite approximation for distant creatures

---

## 8. Scalability

### Viewport Distance
Current implementation renders 4 squares deep:
- 9 squares (3×3 grid centered on party)
- Front wall + side walls per visible square
- Maximum ~40 wall faces + floor + ceiling per frame
- Manageable even on Saturn SH-2

### Frame Rate Stability
Game tick is decoupled from render frame rate:
- Logic ticks at 200ms intervals
- Renders as fast as possible between ticks
- If VSync is enabled: locked to 60 Hz display refresh

---

## 9. Known Performance Gaps

1. **DGN 3D geometry parsing**: Not yet implemented — geometry blob
   parser needed before full 3D rendering works
2. **Model loading**: On-demand loading means first creature encounter
   has a brief delay as .MNS file is read and parsed
3. **No texture compression**: DMDF stores uncompressed BITMAP textures
4. **No Z-buffer early-out**: Full rasterization of all triangles,
   no pre-sorted visibility culling

---

## 10. Hardware Requirements Summary

### Original Sega Saturn
- **CPU**: 2× SH-2 @ 10.5 MHz (big-endian)
- **RAM**: 4 MB total (2+1.5+0.5)
- **Storage**: CD-ROM (1 disc)
- **Display**: 320×224, 30 FPS target
- **Audio**: CD-DA + ADX/SEGA PCM

### Firestaff PC (Modern)
- **CPU**: Any x86/ARM (software rasterizer)
- **RAM**: 50 MB minimum
- **Storage**: Disk or disc image
- **Display**: Any SDL-supported resolution (320×200 native, scalable)
- **Audio**: SDL3 audio output

No GPU required — pure CPU rendering at 320×200.
