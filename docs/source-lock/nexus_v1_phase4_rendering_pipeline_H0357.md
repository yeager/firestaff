# Nexus V1 Phase 4 — Rendering Pipeline
**Job:** `Nexus_V1_Phase4_RenderingPipeline_0357`
**Status:** Source-format receipts verified; Saturn presentation remains capture-gated
**Author:** Firestaff agent (cron)
**Last revised:** 2026-08-06T12:00 UTC+2

---

## Scope

Source-lock the retained Nexus source-format receipts and the explicit no-draw
boundaries around wall/floor/object/creature/projectile/UI/title presentation,
palette/texture/model handling, and unsupported 3D assets. Saturn presentation
is not claimed until the required VDP1/VDP2 capture evidence exists.

---

## 1. Files Created/Modified

| File | Role |
|------|------|
| `include/nexus_v1_palette.h` | Palette/texture header (BGR555, texture atlas) |
| `src/nexus/nexus_v1_palette.c` | Palette load/expand (STONE.BIN, BGR555->RGBA) |
| `include/nexus_v1_rasterizer.h` | Rasterizer header (V1 320×200 fb, primitives, dungeon calls) |
| `src/nexus/nexus_v1_rasterizer.c` | Probe-only CPU rasterizer; retail links `nexus_v1_rasterizer_runtime_noop.c` until VDP1 capture |
| `include/nexus_v1_ui_surfaces.h` | UI/title surface header |
| `src/nexus/nexus_v1_ui_surfaces.c` | UI surface loader + blitter |

---

## 2. Palette System — `nexus_v1_palette.{c,h}`

### 2.1 Saturn VDP1 Palette Format

**Source:** Saturn VDP1 SDK — Color RAM (32 KB, 16-bit BGR555 × 256 entries).

BGR555 layout: `1R RRRG GGGGB BBBBB` (16 bits). Each 5-bit channel spans 0–31 in hardware; expanded to 8-bit (0–255) in software via left-shift 3, then OR with top 2 bits = `(x << 3) | (x >> 2)`.

**Source-lock implementation:** `bgr555_to_rgba()` in `nexus_v1_palette.c`.

### 2.2 STONE.BIN — Image-local palettes

**Source:** DMWeb `DecodeRawPPpp` and the retail corpus — STONE.BIN is 4,400
bytes containing eight 550-byte `pp` records. Each record is a 32×32 4bpp
image with its own 16-entry big-endian palette.

`nexus_palette_load_stone()` is retained as a blocked legacy entry point. It
does not reinterpret a retail `STONE.BIN` as a synthetic 256-entry global
palette. Callers must use the source-owned STONE pp receipt and record decoder.

The old slot classification was inferred metadata and is not a retail source
mapping; it is not used by production.

### 2.3 Texture Atlas

`Nexus_Texture` records: `data`, `w`, `h`, `pal_start`, `pal_count`, `source_file`, `label`.

`nexus_texture_load_from_surface()` validates dimensions and the complete
source span before allocating. Short, negative, or overflow-prone input is
rejected without zero-padding or publishing partial texture pixels.

**Source:** ReDMCSB PALETTE.C — DM1 palette load/apply (equivalent mechanism).

---

## 3. Rasterizer — `nexus_v1_rasterizer.{c,h}`

### 3.1 Framebuffer

**Source:** Saturn VDP1 Programmer's Guide — `G0296_puc_Bitmap_Viewport` (320×200 indexed bitmap). Maps to `Nexus_Framebuffer`: `[320×200] uint8_t color_buffer`, `float z_buffer[320×200]` (near=0.1, far=100.0), `uint32_t palette[256]` (RGBA expanded), `int clear_color`.

**Source-lock init:** `nexus_fb_init()` leaves the palette unbound. Only an
authenticated source/capture route may populate it through
`nexus_fb_set_palette()`.

**Source:** ReDMCSB DRAWVIEW.C F2172 — viewport blit to screen memory (`0xe12000` base, 224-pixel wide lines, 136-pixel height, screen-relative). DM1 draws into `G0296_puc_Bitmap_Viewport`, blits to `G0348_Bitmap_Screen` via F2172.

### 3.2 Camera

**Source:** ReDMCSB DUNGEON.C — party camera placing (FOV 60°, aspect 320/200=1.6).

`g_cam_dir[4]`: North=`(0,0,-1)`, East=`(1,0,0)`, South=`(0,0,+1)`, West=`(-1,0,0)`.

`g_cam_right[4]`: North=`(1,0,0)`, East=`(0,0,-1)`, South=`(-1,0,0)`, West=`(0,0,+1)` (cross(dir, up=+Y) in XZ plane).

View matrix from `m4_look_at()`, projection from `m4_perspective(fov=60, aspect=1.6, near=0.1, far=100)`. Combined into `view_proj` for single-transform projections.

### 3.3 Triangle Primitive

**Source:** ReDMCSB BLIT.C F0132 — 2D triangle rasterization.

`edge_fn()` (signed area) barycentric weights. Back-face culling: `area <= 0` → discard. Raster scan over scissored bounding box. Z-buffer compare: `zfrag < z_buffer[idx] && zfrag > 0` → write.

### 3.4 Textured Triangle

**Source:** Saturn VDP1 Programmer's Guide — `CMD_POLYGON` with `vertex UV` fields.

Affine UV (no perspective divide — Saturn style): `u_screen = w0*v0.uv + w1*v1.uv + w2*v2.uv` (barycentric weights from screen space). UV scaled by `(int)(u * tex_w) & (tex_w - 1)` for pixel lookup. Power-of-two textures use bitwise AND (fast modulo); non-POT sizes use integer % for safety.

The raster primitive is retained for source study and isolated fixture tests.
Production DGN/MNS presentation does not fall back to flat shading when texture,
CLUT, placement, or command-order provenance is absent; it remains no-draw.

### 3.5 Wall Rendering — `nexus_draw_wall()`

**Source:** ReDMCSB DUNGEONENTRANCE.C F0108 — wall quad for squares type 0 (wall).

`wall_quad_verts()` places 4 vertices of the wall face quad: floor-level Y=0, ceiling Y=1. Grid corner table per wall_dir (0=N/z-, 1=E/x+, 2=S/z+, 3=W/x-).

**UV conventions:** `(0,1)=top-left`, `(1,1)=top-right`, `(1,0)=bottom-right`, `(0,0)=bottom-left` of wall face texture. UV origin at lower-left (`Y=0`) of wall, matching Saturn VDP1 texture coordinate system.

**Depth:** Z from view matrix (camera-local). Back-facing walls auto-culled by triangle winding.

### 3.6 Floor/Ceiling — `nexus_draw_floor()`

**Source:** ReDMCSB DUNGEON.C F0108 floor/ceiling branch.

Floor: grid (x,z) to (x+1,z+1) at Y=0. Ceiling: same XZ at Y=1. Each drawn as a quad pair. UVs: floor normal (+Y), ceiling flipped.

### 3.7 Door States — `nexus_draw_door()`

The API is retained for gameplay state, but is currently no-draw. The old
DM1-shaped CLOSED/OPEN/LOCKED geometry and palette-14 claim are not Saturn
Nexus evidence. A real door route requires captured VDP1 commands, CLUT,
animation frame and placement tied to the Nexus DGN/MNS owner.

### 3.8 Creature Billboard — `nexus_raster_billboard()` + `nexus_raster_creature_billboard()`

`nexus_project_model_vert()` and `nexus_raster_billboard()` remain geometry
helpers, but `nexus_raster_creature_billboard()` is no-draw. DM1
CHAMPDRW.C/F0403 projection, inferred gameplay flags and a host-supplied
texture do not establish the Saturn VDP1 command, CLUT, placement or
DMDF/MNS owner. No placeholder billboard is admitted.

### 3.9 Projectile Rendering — `nexus_raster_projectile()`

The API remains as a source-study boundary, but is no-draw. The former DM1
OBJECT.C/DDA palette and primitive claims are not Nexus Saturn capture
evidence. Real effects require the Saturn VDP1 command stream, CLUT,
placement and SLEV/SAL/SFX ownership.

---

## 4. UI / Title Surfaces — `nexus_v1_ui_surfaces.{c,h}`

### 4.1 Surface Files

| File | Size | Dimensions | Palette offset | Source |
|------|------|------------|----------------|--------|
| `TITLE.CG` | 164 KB | 320×200 | 64–127 | title screen |
| `WARNING.BIN` | 99 KB | 320×200 | 160–191 | disclaimer |
| `GAMEOVER.BIN` | 101 KB | 320×200 | 128–191 | game over |
| `STABG.BIN` | 52 KB | 320×200 or 320×52 | 0–15 | status area bg |
| `FACE.BIN` | 44 KB | 24×48×48 strip | 192–207 | champion portraits |
| `FONT256.S2D` | 24 KB | (already in `nexus_v1_saturn_font.c`) | — | Saturn font |

### 4.2 Saturn VDP1 Surface Format

**Source:** Saturn VDP1 SDK — `CMD_BITMAP` (8-bit indexed CLUT mode). Row-major, stride = w bytes (no padding, Saturn row stride = w).

`nexus_ui_surface_load()`:
- rejects `NULL` data, invalid dimensions, and any source shorter than `w*h`.
- never zero-pads, publishes partial pixels, or creates a gray placeholder.
- may retain verified source pixels for receipts and inspection only; a
  successful load is not proof of Saturn VDP2 placement or a drawable screen.
- SEGA header chk (first 4 bytes `"SEGA"`), skip 16-byte offset if present (for TITLE.CG).

### 4.3 Presentation Boundary

The public blit/render/remap/darken helpers are intentionally no-op in the
production Nexus route. The ReDMCSB `BLIT.C` F0132 copy is a historical source
reference, not evidence that Nexus uses the same framebuffer ownership.
Authentic presentation requires captured VDP1 commands, VDP2 destination and
CLUT selection, plus ordering/timing tied to the relevant title/HUD surface.

Until that evidence is available, missing or unproven presentation data is
reported as a receipt/status and produces no visible fallback.

### 4.4 FACE.BIN Layout

24 portraits, 48×48 pixels each (or nearest power-of-2). Horizontal strip: entry `N` at offset `N × 48 × 48`. `nexus_ui_load_face_single()` handles per-entry extraction with diagnostic on short data.

---

## 5. DMDF Model Bridge

`nexus_v1_dmdf_model.c` parses DMDF `.MNS` files (big-endian, magic `"DMDF"`).

The geometry helpers are retained as source-study boundaries. The creature
billboard route is no-draw until Saturn VDP1 command/CLUT/placement and
DMDF/MNS ownership are captured. When a specific `.MNS` file is missing or
corrupt, logging and receipt-only diagnostics are allowed; a gray placeholder
or other visible fallback would be synthetic data and is not permitted.

Full DMDF → rasterizer vertex pipeline (proper geometry, not just billboard) is scheduled for Nexus V2 Phase 2 (graphics pipeline).

---

## 6. V2 Pipeline Integration

`nexus_v2_render_pipeline.c` bridges V1 → V2:

```
V1_fb.color_buffer[320×200]  (indexed)
  ↓ EPX upscale (nexus_v2_epx_upscale)
  ↓ palette → RGBA conversion
  ↓ bilinear smooth (V2.1 optional)
  ↓ dynamic lighting (V2.2)
  ↓ fog + AO (V2.2)
  ↓ particles (V2.2)
  → SDL present RGBA buffer
```

`nexus_v2_pipeline_render()` handles all three modes (V1 OFF / V2.1 UPSCALED / V2.2 ENHANCED). V1 mode bypasses upscaling: direct palette → RGBA per pixel.

---

## 7. Source References

| ReDMCSB File | Function | Use |
|-------------|----------|-----|
| `DRAWVIEW.C` | F2172 | Viewport blit to screen |
| `DRAWVIEW.C` | F1082–F1095 | Viewport blit lines |
| `DUNGEON.C` | F0108 | Wall/floor/ceiling square draw |
| `DUNGEON.C` | F0107 | Door panel visual states |
| `BLIT.C` | F0132 | Blit rect primitive |
| `OBJECT.C` | F0823 / F0841–F0843 | Projectile firing |
| `CHAMPDRW.C` | F0403 | Creature billboard projection |
| `PALETTE.C` | — | DM1 palette load/expand |
| `CEDTINCK.C` | — | CEDT font/text rendering |
| PANEL.C | F0120–F0125 | Panel element drawing |
| DEFS.H | `C200_HEIGHT_SCREEN=200` | Screen dimensions |

---

## 8. Compliance with Phase 4 Mandate

✅ Source-format parsers and bounded receipts — implemented and validated
⏳ Saturn VDP1/VDP2 presentation — capture-gated
⏳ DGN mesh/material/creature/door/projectile runtime rendering — capture-gated
✅ Unsupported or missing/corrupt data fails closed; no generated placeholder is admitted
✅ Source citations and bounds checks are retained at the implementation boundary
