# Dungeon Master Nexus V1 — New Features vs DM1/DM2/CSB

## Sources
- `src/nexus/nexus_v1_engine.c`, `nexus_v1_rasterizer.c`, `nexus_v1_viewport.c`
- `src/nexus/nexus_v1_dmdf_model.c`, `nexus_v1_math3d.c`, `nexus_v1_iso_reader.c`
- `docs/NEXUS_FILE_CLASSIFICATION.md`, `docs/NEXUS_PLAN.md`

---

## Overview

Dungeon Master Nexus (1998) is a **3D polygon remake** of Dungeon Master I. Every
visual and audio system is new vs DM1/DM2/CSB, while game logic (combat, spells,
movement, champion advancement) is derived from DM1.

| Feature               | DM1/DM2/CSB                     | Nexus V1                           |
|-----------------------|---------------------------------|------------------------------------|
| Rendering             | 2D sprite blitting              | 3D polygon rasterizer (software)   |
| Geometry              | None (walls drawn per-square)   | Per-level 3D mesh embedded in DGN  |
| Creature models       | 2D sprite sheets                | DMDF 3D .MNS polygon models        |
| Dungeon format        | DUNGEON.DAT (33-39 KB)          | LEV00-15.DGN (148-321 KB each)     |
| Per-level CD audio    | None                            | 8 Red Book Audio tracks (tracks 2-9)|
| Text/charset          | ASCII                           | 256-char font incl. Shift-JIS JP   |
| Platform              | Amiga/ST/DOS                   | Sega Saturn SH-2 (big-endian)      |
| Save format           | Binary slot files              | Saturn SRAM (exact format TBD)     |
| FMV                   | None                            | 3 AVI cutscenes (34+28+39 MB)      |
| Viewport distance    | 2 squares (DM1)                | 4 squares, first-person 3D          |
| Minimap               | ASCII chart                     | SMAP00-15.BIN binary (17-30 KB)    |
| Sound effects         | Global SND.GAM                  | Per-level SNDLEV*.SAL (290-460 KB) |
| Level scripts         | None                            | SLEV*.BIN (2-12 KB per level)      |

---

## 1. 3D Polygon Renderer (VDP1-Inspired Software Rasterizer)

Nexus replaces the 2D sprite engine entirely with a **software triangle rasterizer**
backed by the Sega Saturn VDP1 instruction set design:

- **Edge-function triangle rasterizer** in `nexus_v1_rasterizer.c` — software
  fallback for VDP1-style quad rendering; runs in SH-2 environment, ported to x86
- **3D math pipeline** — vec3/mat4 in `nexus_v1_math3d.c`: perspective projection,
  view transformation, model transforms
- **Z-buffer** — per-pixel depth for correct wall occlusion in first-person view
- **Texture mapping** — perspective-correct UV mapping via VDP1-like quad set
- **Framebuffer** — 320x200 internal resolution; Saturn VDP2 handles
  background layers and compositing

Source: `nexus_v1_rasterizer.c` (edge function), `nexus_v1_math3d.c` (mat4).

---

## 2. First-Person 3D Viewport

`nexus_v1_viewport.c` implements the first-person dungeon view:

- **View distance: 4 squares** (vs DM1's 2-square visibility) — corridors
  feel more expansive, more tactical warning time
- **Wall polygon rendering** — each visible wall facet emits triangles to the
  framebuffer; no per-square sprite blitting
- **Floor/ceiling geometry** — projected triangle mesh per level with
  per-vertex UV for texture sampling
- **Door state rendering** — open/closed/destroyed reflected in 3D geometry
- **Creature 3D projection** — DMDF models projected with distance-based scaling

Source: `nexus_v1_viewport.c`.

---

## 3. DMDF 3D Creature Model Format

All creature models stored in the proprietary **DMDF format** (.MNS files):

- **30 .MNS files** in the ISO: SCORPION, MUMMY, GHOST, ROCKPILE, SCREAMER,
  RAT, WORM, OITU, GOLEM, GIGGLER, CHAOS, VEXIRK, D_RED, D_GOLD, D_SILVER,
  GRN_DRA, MINI_DRA, RED_DRA, SKELETON, DRAGON, SPIDER, and variants
- Models include idle/attack/death animation frames (exact count TBD)
- Loaded by `nexus_v1_dmdf_model.c` and cached in creature manager
- DMDF stores per-vertex position (x,y,z floats), normal vector (nx,ny,nz),
  UV coordinates, triangle index list, and texture reference per face

Source: `nexus_v1_dmdf_model.c`, `docs/NEXUS_FILE_CLASSIFICATION.md`.

---

## 4. Per-Level 3D Dungeon Geometry (LEV*.DGN)

Nexus dungeon files (LEV00-15.DGN, 148-321 KB each) are **10x larger than DM1's
DUNGEON.DAT** because they embed full 3D wall/floor/ceiling polygon meshes:

- **DMWeb DGN block container** — 2048-byte blocks; Structure1B is a
  64x64 grid with 8 bytes per cell
- **3D geometry and collision sections** — Structure1C through Structure1F and
  later render payloads are still being split out
- **Level script** — SLEV*.BIN (2-12 KB): trigger/events per level
- **Level minimap** — SMAP*.BIN (17-30 KB): binary automap data

Source: `nexus_v1_dungeon.c` (`nexus_v1_level_load`).

---

## 5. Per-Level CD Audio (Red Book Audio, Tracks 2-9)

- **8 CD audio tracks** (Red Book Audio, 44.1kHz stereo) mapped to level pairs
- Per-level sound effect banks: SNDLEV00-15.SAL (290-460 KB each)
- Sound mapping tables: SNDLEV00-15.MAP (66-90 bytes each)
- `nexus_v1_game.c` manages track selection (track 2 + level/2) and crossfade
- Audio driver: SDDRVS.TSK (26 KB)

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`, `docs/NEXUS_PLAN.md`.

---

## 6. Japanese Language Support

- **FONT256.S2D** (24 KB) — 256-entry sprite font including Japanese glyphs
- Champion roster: 8 Japanese champions with ASCII + JP Shift-JIS name
- Inscriptions, monster names, spell names in Shift-JIS
- Saturn SFC-style font renderer in `nexus_v1_saturn_font.c`

Source: `nexus_v1_champions.c` (roster), `nexus_v1_saturn_font.c`.

---

## 7. Champion Identity and Progression

- **24-member roster** with unique portraits (FACE.BIN, 44 KB)
- **Alignment system** — Fighter/Neutral, Priest/Good, Wizard/Chaos, Ninja/Neutral
- **Food/water** — 1500 units each at creation; starvation triggers death
- **Anti-magic / Anti-fire** — initially 5 each (Nexus sets defaults vs DM1's 0)
- **Resurrection** — champions killed can be resurrected at 25% max HP/STA
- **Ninja class** — added from DM2; DM1 had only Fighter/Wizard/Priest

Source: `nexus_v1_champions.c`.

---

## 8. FMV Cutscenes (Saturn AVI)

- **3 AVI files**: DMV0.AVI (34 MB), DMV1.AVI (28 MB), DMV2.AVI (39 MB)
- Intro, ending, mid-game cinematic (exact mapping TBD)
- Saturn AVI codec — not standard AVI; requires custom decoder

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`.

---

## 9. Per-Level Sound Effect Banks

- **SNDLEV00-15.SAL** (290-460 KB each) — per-level sound effects
- **SNDLEV00-15.MAP** (66-90 bytes each) — sound effect index mapping
- vs DM1's global SND.GAM which applied to all levels

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`.

---

## 10. Saturn Platform Adaptation

- **SH-2 big-endian CPU** — all binary structures byte-swapped vs x86
- **VDP1/VDP2 hardware** — VDP1: 3D polygons; VDP2: background + compositing
- **CD-ROM XA audio** — mixed-mode CD with data + Red Book Audio
- **SRAM save** — 8 KB Saturn memory backup for save games
- **Controller** — SFC-style pad; D-pad + 6 face buttons + shoulder triggers

Source: `docs/NEXUS_PLAN.md`, ISO sector analysis.

---

## Comparison Summary

| Aspect                  | DM1        | CSB        | DM2         | Nexus V1           |
|-------------------------|------------|------------|-------------|--------------------|
| Graphics engine         | 2D sprites | 2D sprites | 2D sprites | **3D polygons**   |
| Dungeon data per level  | ~33 KB     | ~33 KB     | ~39 KB      | **148-321 KB**     |
| CD audio                | None       | None       | Per-level   | **8 tracks + SFX** |
| Creature models         | 2D sprites | 2D sprites | 2D sprites  | **DMDF .MNS 3D**   |
| View distance           | 2 squares  | 2 squares  | 2 squares   | **4 squares**      |
| Champion roster         | Western    | Western    | Western     | **Japanese**       |
| FMV                     | None       | None       | None        | **3 AVI cutscenes**|
| Platform                | 16-bit HW  | 16-bit HW  | 32-bit PC   | **Sega Saturn**   |
| Level scripts           | None       | None       | None        | **SLEV*.BIN**      |
| Per-level SFX           | None       | None       | None        | **SNDLEV*.SAL**    |

---

## Status: PARTIALLY SOURCE-LOCKED

3D renderer, dungeon loader, creature manager, champion system, CD audio,
and FMV classification are derived from ISO analysis + inference from DM1.
No SH-2 disassembly confirmed. All file formats (DGN, DMDF, MNS, DGN geometry
section) are reverse-engineered and not byte-verified against actual CD data.
