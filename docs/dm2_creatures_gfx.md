# DM2 V1 Creature Graphics Audit

## Sources
- SKULLWIN C++: c_gui_vp.cpp, c_gfx_blit.h, c_gfx_main.cpp
- SKULLWIN Data: data_dm2_dm/graphics.dat (creature sprite indices)
- SKULL.ASM disassembly (skproject)
- Firestaff docs: docs/dm2_ai_creatures.md, docs/dm2-v1-overview/dm2_technical.md

---

## 1. Creature Rendering Pipeline

DM2 renders creatures using the standard sprite compositing system:

1. **Depth sort**: Each creature sprite has a depth value (wall row + position
   within the row). Sprites are sorted front-to-back.
2. **Sheet extraction**: Creature sprite is extracted from GRAPHICS.DAT
   as c_pixel256 bitmap via DM2_EXTRACT_GDAT_IMAGE(t_dbidx, frames_offset)
3. **Masked blit**: Sprite is drawn with colorkey mask (transparent pixels
   skipped), using blitter.blit() with NOMASK or a mask parameter
4. **Depth compositing**: Creature is composited over wall/floor viewport pass

The DM2 creature sprite system is byte-packed indexed color (palette-indexed),
same as DM1.

---

## 2. Creature Sprite Format

From c_gfx_blit.h and c_gfx_main.cpp:
- c_pixel256* — 8-bit palette-indexed sprite data
- Colorkey (E_COL00 or transparent index) for masked blits
- Sprite bitmaps referenced by t_dbidx (graphics database index)

The creature sprite system in SKULLWIN:
- DM2 creatures have more animation frames than DM1 equivalents
- Large creatures use multi-sprite compositing (body + shadow separate)
- DM2 creature sprite sheet indices: new entries not present in DM1 GRAPHICS.DAT

DM2 extends the sprite count significantly:
- New DM2-specific monsters drawn from scratch (not in DM1 GRAPHICS.DAT)
- Extended frame counts for existing DM1 monsters (more walk/attack frames)

---

## 3. Creature GRAPHICS.DAT Locations

The ~24x increase in GRAPHICS.DAT includes new creature sprites:

| Creature Category | DM1 PC 3.4 | DM2 DOS EN | Notes |
|-------------------|------------|------------|-------|
| Core DM1 monsters | Present | Present | Palette-compatible |
| DM2 new monsters | Absent | Present | Entirely new art |
| Animation frames | 4-8 | 8-16 | More attack/defend angles |
| Champion sprites | Basic | Expanded | More class-specific frames |

Source: docs/dm2-v1-overview/dm2_technical.md (Section 2: Asset File Sizes)

---

## 4. Sprite Animation

DM2 creature animation uses tweening between keyframes for smooth motion:

### Walk Cycle
- DM1: 4-frame walk cycle (snap between keyframes)
- DM2: 8-frame walk cycle with interpolation between keyframes
- Frame interpolation done visually at render time (not in sprite data)

### Attack Frames
- DM2: Extended attack frame sequences
- Attack frame count matches DM2 creature AI extended combat moves
- More death/knockback frames for the longer combat rounds

From DM2 combat system (docs/dm2_combat_creatures.md):
- Each creature type has extended frame arrays
- Graphics.DAT sprite indices for animation frames indexed by creature_type

---

## 5. Creature Sprite Depth

DM2 creature depth sorting:
- Creatures rendered after floor and walls are complete
- Per-square depth: each creature in a square has its own depth
- Depth is calculated as: wall_row_index + y_position_within_row

From c_gui_vp.cpp, DM2_blit_specialeffects calls for sprite zones:
- RG1R/RG2R/RG3R region queries for dungeon viewport zones
- Sprite overlay passes are done after wall/ceiling/floor passes

DM1 creature depth from docs/dm1-v1-dungeon-audit:
- Per-square: creatures sorted by distance (closer on top)
- Same square: right-to-left tie-break (leftmost drawn first)

DM2 follows the same tie-breaking logic but with more granular depth
values due to the extended creature AI pathfinding.

---

## 6. Champion Sprite Rendering

Champions are rendered as sprites bound to their class/equipment:
- Champion sprite sheet: selected by class (Warrior, Paladin, etc.)
- Weapon sprite overlays: drawn separately on top of champion sprite
- Armor level affects sprite (higher armor = different overlay)
- State-based sprites: standing, walking, fighting, dead

From docs/dm2_champ_classes.md:
- Each champion class has its own sprite set
- Sprite set switching based on equipped weapon
- Class-specific visual overlays (spell effects for Wizard)

DM1 champions had 4 basic sprite states (stand, walk, fight, dead).
DM2 champions have expanded states matching new classes and mechanics.

---

## 7. Sprite Masking and Blending

DM2 sprite masking:
- colorkey-based masking (E_COL00 = transparent)
- Masked blit via blitter.blit() with alphamask parameter
- Alpha mask layer for semi-transparent sprites (spell effects)

c_gfx_blit.h alpha blend functions:
- blitline_44_ma / blitline_44_mima — 16-bit alpha blend variants
- blitline_48_ma / blitline_48_mima — 16→8-bit with alpha
- blitline_88_ma / blitline_88_mima — 8→8-bit with alpha
- blitline_88xlat_ma / blitline_88xlat_mima — 8→8-bit with palette xlat + alpha

These _ma (multiply-alpha) and _mima (multiply-inverse-alpha) variants
enable sprite transparency effects:
- Spell effect sprites: semi-transparent blue/red glow (DM2 magic system)
- Torch glow overlay: warm semi-transparent halo around light sources
- Status effect overlays: poison green, energy blue etc.

---

## 8. Particle/Sprite Effects

DM2 implements additional sprite-based effects beyond DM1:
- Wall torch flames: animated flame sprites in wall brackets
- Weather particles: rain drop sprites (blitline_48 16→8-bit overlay)
- Spell projectiles: creature-sized sprite sheets with many frames
- Smoke/mist: semi-transparent blob sprites from c_gfx_blit alpha functions

DM1 had minimal particle effects (torch flicker on status bar only).
DM2 has:
- Wall torch flames (per-wall bracket sprite animation)
- Fog overlay (rendered via blitline_48)
- Rain sprite sheet (outdoor weather)
- Environmental dust motes (outdoor)

---

## 9. Key Differences from DM1 Creature Graphics

| Feature              | DM1 PC 3.4           | DM2 DOS EN               |
|----------------------|----------------------|--------------------------|
| Sprite format        | Byte-packet indexed  | Byte-packet indexed      |
| Animation frames     | 4-8 per creature     | 8-16 per creature        |
| Walk interpolation   | None (snap)          | Interpolation (smooth)   |
| Attack frames        | 4 per creature       | 8 per creature           |
| Death frames         | 2-4                  | 6-8                      |
| Sprite masking       | Colorkey only        | Colorkey + alpha blend   |
| Alpha blending       | None                 | Yes (_ma/_mima variants) |
| Spell effect sprites | None                 | Yes (semi-transparent)   |
| Particle effects     | Minimal (bar)        | Yes (torch, rain, fog)   |
| Champion sprite sets| 4 states             | Expanded per class       |
| Weapon overlay       | Static sprite        | Class/equipment-specific |
| Weather sprites      | None                 | Rain/fog/storm           |
| New creature types   | No                   | Yes (entirely new art)   |
