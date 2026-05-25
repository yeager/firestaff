# DM2 V1 Viewport Rendering Audit

## Sources
- SKULLWIN C++: c_gui_vp.cpp (main viewport rendering), c_gfx_blit.h, c_gfx_main.cpp
- SKULLWIN Data: data_dm2_dm/GRAPHICS.DAT
- SKULL.ASM disassembly (skproject)
- DM1 reference: docs/dm1-v1-dungeon-audit/viewport_*.md

---

## 1. Viewport Architecture

DM2 has a single 3D dungeon viewport rendered to a backbuffer (320x136 visible)
and blitted to the full screen buffer (ORIG_SWIDTH x ORIG_SHEIGHT).

The viewport zone system (c_gui_vp.cpp) maps named zones on the screen:
- Ceiling area (top band)
- Wall area (side walls, forward wall)
- Floor area (bottom band)
- Sprite overlay zones

DM2 distinguishes two viewport rendering modes:
- **Dungeon mode**: raycast first-person 3D view (identical structure to DM1)
- **Outdoor mode**: landscape renderer with sky gradient (DM2-specific)

---

## 2. Dungeon Viewport — Same Structure as DM1

The dungeon viewport is divided into the same three bands as DM1:

| Zone        | Lines    | Height |
|-------------|----------|--------|
| Ceiling     | 0–27     | 28 px  |
| Wall area   | 28–65    | 38 px  |
| Floor       | 65–135   | 70 px  |
| **Total**   | 0–135    | ~136px |

DM1 reference (dm1-v1-dungeon-audit/viewport_dims.md):
- Ceiling: G1012 auc_Frame_Ceiling = { 0, 223, 0, 28, 112, 29 }
- Wall area: lines 28–65
- Floor: lines 66–135

DM2 uses the same band boundaries for the dungeon viewport.
The DM2 source also uses these named zones in c_gui_vp.cpp for
DM2_blit_specialeffects() calls with RG1R, RG2R, RG3R region queries.

---

## 3. Backbuffer Architecture

From c_gfx_main.cpp:
- backbuffer_w = 0xe0 (224 pixels wide)
- backbuffer_h = 0x88 (136 pixels tall)
- gfxsys.bitmapptr: viewport rendering surface

DM2 allocates the backbuffer at this resolution, renders the dungeon viewport
to it, then blits to the full screen via _specialblit() or blitter.blit().

The backbuffer size 224x136 matches the viewport display resolution after
any aspect-ratio correction. The screen scroll buffer is ORIG_SWIDTH x
ORIG_SHEIGHT (the full 320x200 DOS VGA screen buffer).

---

## 4. Expanded UI Viewport Areas

DM2 surfaces more of the screen for UI compared to DM1:

- Champion panel (bottom strip) — expanded for new DM2 stats/classes
- Shop interface — new in DM2, no DM1 equivalent
- Outdoor minimap — DM2-only map pane
- Dialogue panel — wider, supports more text (14 chars/line in DM2 vs DM1)

DM2_viewport_region constants in c_gui_vp.cpp:
- RG1R, RG2R, RG3R: dungeon viewport regions (ceiling, walls, floor)
- RG52p: dialogue picture region
- Extended GUI draw functions: DM2_DRAW_DIALOGUE_PICT

---

## 5. Outdoor Viewport (DM2-Only)

DM2 adds an entirely different viewport mode for outdoor areas:

The outdoor state machine (DM2_V1_OutdoorState):
- sky_texture_field: sky gradient background
- ground_texture_field: perspective ground plane
- building_texture_field: exterior walls
- tree_texture_field: foliage sprites
- weather: clear/rain/fog/storm
- time_of_day: ambient light modifier (0=full, 8=dark)

Outdoor rendering path:
1. Render sky gradient (top of screen)
2. Render ground texture tile (bottom of screen)
3. Render building/tree sprites at screen depth
4. Overlay weather effect (16-bit → 8-bit via blitline_48)
5. Darkness layer based on time_of_day

DM1: no outdoor areas at all.
DM2 outdoor viewport: no equivalent in any first-person dungeon game prior.

---

## 6. Depth Rendering

DM2 renders dungeon squares in front-to-back order:
- Nearest tiles (row 0) rendered last (on top)
- Farthest tiles (row 7/8) rendered first (background)
- Same painter's algorithm as DM1 for floor and ceiling tiles
- Sprite depth: each creature/overlay rendered at correct wall-row depth

Tie-breaking when two squares have the same distance:
- Left-to-right column processing (same as DM1)
- Wall strips at same depth composited in left-to-right order

---

## 7. Key Differences from DM1 Viewport

| Feature             | DM1 PC 3.4           | DM2 DOS EN              |
|---------------------|----------------------|-------------------------|
| Ceiling band        | 28 lines             | 28 lines                |
| Wall band          | 37 lines             | 37 lines (approx)       |
| Floor band         | 70 lines             | 70 lines                |
| Total viewport     | ~136 lines           | ~136 lines              |
| Dungeon mode       | Yes                  | Yes                     |
| Outdoor mode       | No                   | Yes (new)               |
| Shop UI            | No                   | Yes (overlay)           |
| Champion panel     | Basic                | Expanded (stats/classes)|
| Minimap            | No                   | Yes (outdoor)           |
| Weather overlay    | No                   | Yes (outdoor)           |
| Backbuffer size    | 224x136              | 224x136 (same)          |
