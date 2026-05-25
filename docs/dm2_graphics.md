# DM2 V1 Graphics Improvements vs DM1

File: /tmp/dm2_graphics.md
Audit: DM2 V1 graphics improvements, new graphics, animations, effects vs DM1
Sources:
  - SKULL.ASM (sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
  - skproject github.com/gbsphenx/skproject HEAD a962896
  - include/dm2_v1_outdoor_renderer.h
  - include/dm2_v2_viewport_renderer.h
  - src/dm2/dm2_v1_outdoor_renderer.c
  - src/dm2/dm2_v2_viewport_renderer.c
  - docs/dm2-v1-overview/dm2_technical.md
  - docs/dm2-v1-overview/dm2_newfeatures.md

---

## 1. Asset File Size Expansion

DM2 V1 has massively expanded GRAPHICS.DAT compared to DM1:

| File        | DM1 PC 3.4  | DM2 DOS EN  | Ratio  |
|-------------|-------------|-------------|--------|
| GRAPHICS.DAT | 363,417 B  | ~8.6 MB     | ~24x   |
| DUNGEON.DAT  | 33,357 B   | 39,437 B    | ~1.18x |

The ~24x increase in GRAPHICS.DAT covers:
- Outdoor environment art (sky, ground, trees, buildings)
- Extended creature artwork (new DM2-specific creatures)
- New UI elements (champion sheets, shops, maps)
- Animation frames for items and creatures

Source: docs/dm2-v1-overview/dm2_technical.md (Section 2 Asset File Sizes)
Source: SKWIN/SkGlobal.h (asset size constants)

---

## 2. New Graphics Systems (No DM1 Equivalent)

### 2.1 Outdoor Renderer (Entirely New)

DM2 adds a full outdoor landscape renderer with no equivalent in DM1.

Source: src/dm2/dm2_v1_outdoor_renderer.c

Key fields in DM2_V1_OutdoorState:
- sky_texture_field: sky backdrop texture
- ground_texture_field: ground plane texture
- building_texture_field: exterior building textures
- tree_texture_field: foliage/tree rendering
- weather: clear/rain/fog/storm weather system
- time_of_day: day/night cycle with ambient darkness (0=full, 8=dark)

Source: include/dm2_v1_outdoor_renderer.h

Outdoor rendering features:
- Sky gradient rendering (new — no DM1 equivalent)
- Ground plane with perspective
- Building exteriors, trees, environmental props
- Weather effects overlay

Source: include/dm2_v1_outdoor_renderer.h: "DM2 has outdoor areas. DM1 does not."
Source: include/dm2_v1_game.h: "DM2 has a different engine with outdoor areas, shops, NPCs."

### 2.2 Sky Gradient Field (DM2-specific)

The outdoor renderer draws a sky gradient as background:
- Gradient from horizon to zenith
- Color shifts based on time of day and weather

DM1: no sky rendering (first-person dungeon view only)
DM2: sky gradient + weather overlay

Source: include/dm2_v2_viewport_renderer.h: "sky gradient for outdoor"

### 2.3 Weather System (New in DM2)

DM2 outdoor renderer supports weather states:
- clear (default)
- rain (precipitation overlay)
- fog (reduced visibility)
- storm (wind + rain + lightning)

Weather affects:
- Sky color/tint
- Ground texture visibility
- Ambient lighting

DM1: no weather system
DM2: weather state machine per outdoor area

Source: include/dm2_v1_outdoor_renderer.h (weather field)
Source: dm2_v1_outdoor_renderer.c (weather_render)

---

## 3. Viewport Rendering Improvements

### 3.1 Dual Viewport Modes

DM2 supports two distinct renderers:

1. **Indoor first-person** (similar to DM1):
   - Wall/floor/ceiling raycasting
   - Creature display
   - Door/window effects

2. **Outdoor landscape** (new in DM2):
   - Top-down/external view
   - Sky gradient + ground plane
   - Building/tree rendering
   - Seamless indoor/outdoor transitions

Source: include/dm2_v1_outdoor_renderer.h, include/dm2_v1_game.h

### 3.2 V2 Smooth Indoor/Outdoor Blend

dm2_v2_viewport_renderer.c adds smooth blend transition between indoor and outdoor:

- V2.1: EPX upscale for indoor, sky gradient for outdoor
- V2.2: smooth indoor/outdoor blend transition (v2_anim_update indoor_outdoor_blend)
- Animation clock: v2_anim_clock_init, v2_anim_clock_v1_tick, v2_anim_clock_render_frame

Source: include/dm2_v2_viewport_renderer.h:37-39
Source: src/dm2/dm2_v2_viewport_renderer.c:31-34

---

## 4. New Visual Elements

### 4.1 Champion/Companion Panel Graphics

DM2 adds companion/champion management UI:

- Champion portrait graphics (new slot system)
- Champion equipment slots
- Champion status bars (HP, mana, food, water)
- Champion skill display

Source: src/dm2/dm2_v2_companion_ui.c (companion/champion UI rendering)
Source: include/dm2_v2_companion_ui.h

DM1: 4 fixed champions, no dynamic companion panel
DM2: companion system with dynamic champion management

### 4.2 Shop/Merchant Graphics (New in DM2)

DM2 adds shop interface with merchant NPCs:

- Merchant interaction panels
- Buy/sell UI elements
- Currency display (gold)

Source: include/dm2_v1_game.h: "DM2: entering shop"
Source: src/dm2/dm2_v1_game.c:57

### 4.3 Map/World View Graphics (New in DM2)

DM2 outdoor renderer includes world map elements:

- Town/building icons on overland map
- Road/path textures
- Terrain type indicators

DM1: no map view
DM2: overland exploration with map graphics

Source: include/dm2_v1_outdoor_renderer.h
Source: docs/overworld_discovery.md, docs/overworld_map.md

---

## 5. Graphics Format Compatibility

### 5.1 GRAPHICS.DAT Format

DM2 V1 GRAPHICS.DAT uses the same compressed 4bpp planar format as DM1:
- RLE/LZW compression per bitmap
- 4-bitplane (16 color indexed)
- VGA DAC 256-color palette

DM2 extends resource count significantly due to new assets.

Source: src/engine/firestaff_graphics_dat_reader.c
Source: src/engine/firestaff_vga_palette.c

### 5.2 DM2-Specific GRAPHICS.DAT Hashes

Source-locked DM2 GRAPHICS.DAT hashes:

| Hash                                  | Variant              |
|---------------------------------------|----------------------|
| 25247ede4dabb6a71e5dabdfbcd5907d      | PC English           |
| b4d733576ea60c41737f79f212faf528      | PC French            |
| e52ab5e01715042b16a4dcff02052e5d      | PC German/JewelCase  |

Source: src/dm2/dm2_v1_game.c:16-20

---

## 6. Status

| Component              | Source                | Status     |
|------------------------|-----------------------|------------|
| GRAPHICS.DAT size diff | dm2_technical.md:2    | documented |
| Outdoor renderer       | dm2_v1_outdoor_renderer.h | source-locked |
| Sky gradient           | dm2_v2_viewport_renderer.h:37 | source-locked |
| Weather system         | dm2_v1_outdoor_renderer.h | documented |
| V2 blend transition    | dm2_v2_viewport_renderer.c:31 | source-locked |
| Shop graphics          | dm2_v1_game.c:57      | documented |
| Companion UI           | dm2_v2_companion_ui.h | documented |
| DM2 GRAPHICS hashes    | dm2_v1_game.c:16-20   | source-locked |

STATUS: SOURCE-LOCKED
