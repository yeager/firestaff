# Dungeon Master Nexus V1 Overview — Source-Locked Audit

## Sources

- Sega Saturn CD image (CUE/BIN), Japanese version, T-9111G V1.003 (1998-02-03)
- Track 1 (133 MB): MODE1/2352 — game data (ISO 9660 filesystem)
- Tracks 2-9: Red Book Audio CD music
- `src/nexus/nexus_v1_engine.c`, `nexus_v1_game.c`, `nexus_v1_iso_reader.c`
- `include/nexus_v1_engine.h`, `include/nexus_v1_game.h`
- `docs/NEXUS_PLAN.md`, `docs/NEXUS_FILE_CLASSIFICATION.md`
- `src/nexus/nexus_v1_rasterizer.c`, `nexus_v1_viewport.c`

---

## What is Dungeon Master Nexus?

Dungeon Master Nexus (DM Nexus, 1998) is the fourth and **final game** in the
Dungeon Master series. It is an **exclusive Sega Saturn title, Japanese only**,
produced by FTL Games / Athena. It is a **3D polygon remake of Dungeon Master I**,
using the original DM1 dungeon layout (16 levels) with all-new Saturn-native
3D graphics, per-level CD audio, and a reimagined Japanese champion roster.

---

## How Nexus Differs from DM1, CSB, and DM2

| Aspect           | DM1 (1987)             | CSB (1991)               | DM2 (1993)                  | Nexus (1998)                    |
|------------------|------------------------|--------------------------|-----------------------------|----------------------------------|
| Platform         | Amiga, Atari ST, DOS   | Amiga, Atari ST, Mac     | DOS, PC-9821, Sega CD       | **Sega Saturn only**             |
| Language         | EN                     | EN                       | EN/JP                       | **JP only**                     |
| Graphics         | 2D sprites             | 2D sprites               | 2D sprites                  | **3D polygons**                 |
| Renderer         | Sprite blitting        | Sprite blitting          | Sprite blitting             | **Software rasterizer (VDP1)**  |
| Dungeon data     | DUNGEON.DAT (33 KB)    | DUNGEON.DAT (same)       | DUNGEON.DAT (39 KB)         | **LEV00-15.DGN (147-321 KB each)** |
| Level count      | 10 levels              | 10 levels                | 10 indoor + outdoor zones  | **16 levels**                   |
| Engine origin    | DM1 engine             | DM1 engine (patched)     | New Skullkeep engine        | **DM1 logic + new Saturn 3D**   |
| Source available | ReDMCSB (decompilation)| Partial                  | SKULL.ASM (IDA), skproject  | **No disassembly**               |
| CD audio         | None                   | None                     | Red Book Audio              | **8 CD audio tracks (2-9)**     |
| Champion names   | Western (Thor, Sara…)  | Western                  | Western                     | **Japanese (Syra, Leyla, Nabi…)** |
| Champion count   | 24                     | 24                       | ~20                         | **24**                          |
| Creature models  | 2D sprites             | 2D sprites               | 2D sprites                  | **DMDF 3D (.MNS files)**        |
| Magic system     | 16 spells              | 16 spells + Chaos magic  | 34 original + 255 custom    | **DM1 magic + alignment**       |
| 3D files         | None                   | None                     | None                        | **DMDF model format**           |

---

## Architecture: DM1 Logic + Saturn 3D

Nexus runs **DM1 game logic** (combat, spells, movement, AI, inventory,
champion advancement) but renders via a **full 3D polygon pipeline**:

- `nexus_v1_rasterizer.c` — edge-function triangle rasterizer, 320×200 framebuffer
- `nexus_v1_viewport.c` — first-person dungeon viewport, 4-square view distance
- `nexus_v1_dmdf_model.c` — loads DMDF 3D creature models from .MNS files
- `nexus_v1_math3d.c` — vec3/mat4 3D math, perspective projection
- `nexus_v1_saturn_font.c` — 256-char font including Japanese Shift-JIS

The ISO reader (`nexus_v1_iso_reader.c`) parses **MODE1/2352 CD sectors**
with ISO 9660 filesystem and CUE/BIN disc image support.

---

## Dungeon Data

- **LEV00.DGN – LEV15.DGN**: 16 level files, 147–321 KB each
  (vs DM1's ~33 KB total for 10 levels)
- Extra size is **embedded 3D geometry** (wall/floor polygon meshes per level)
- Per-level sound banks: SNDLEV00-15.SAL (290–460 KB each)
- Per-level script: SLEV00-15.BIN (2–12 KB)
- Per-level minimap: SMAP00-15.BIN (17–30 KB)

---

## Status: SOURCE-LOCKED

No SH2 disassembly available. All code is inferred from DM1 equivalents,
DMDF model reverse-engineering from .MNS files, and DGN file analysis.
The dungeon format is not ReDMCSB-based and has not been fully decoded.
