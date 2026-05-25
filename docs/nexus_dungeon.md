# Nexus V1 Dungeon Design Audit

## 1. Level Architecture

**16 levels: LEV00.DGN – LEV15.DGN**

| Level | Size (bytes) | File |
|-------|-------------|------|
| LEV00 | 147,456 | 32×32 @ 144 bpp? |
| LEV01 | 280,576 | |
| LEV02 | 272,384 | |
| LEV03 | 290,816 | |
| LEV04 | 245,760 | |
| LEV05 | 266,240 | |
| LEV06 | 239,616 | |
| LEV07 | 258,048 | |
| LEV08 | 303,104 | |
| LEV09 | 288,768 | |
| LEV10 | 290,816 | |
| LEV11 | 278,528 | |
| LEV12 | 321,536 | largest |
| LEV13 | 256,000 | |
| LEV14 | 253,952 | |
| LEV15 | 270,336 | |

Total: ~4.3 MB of level data, vs. DM1's ~33 KB. Ratio: ~130× larger.

**Grid size:**  (32×32 squares per level), vs. DM1's variable 16–30 per level.

## 2. Level File Format (DGN)



Square values: lower 5 bits used (), matching DM1 square type semantics.

**Why so large?** The DGN files contain embedded 3D geometry (wall polygon data, floor/ceiling mesh vertices) rather than computing it procedurally — unlike DM1 which used a 2D grid + pre-baked sprite rendering.

## 3. Level Scripts / Events

Nexus V1 uses **scripted level events** loaded from  (5,448 bytes) — a separate script file, not embedded per-level like DM1's inline bytecodes.

**Key difference vs. DM1:**
- DM1: sensors hardwired in game loop logic (compile-time behavior)
- Nexus: declarative scripts processed by a virtual machine ()

The firestaff  currently implements only grid parsing; 3D geometry parsing and script VM are **TODO/future work**.

## 4. Comparison: Nexus vs. DM1 Dungeon Design

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| Levels | 8 (D0–D7) | 16 (LEV00–LEV15) |
| Grid | Variable 16–30 | Fixed 32×32 |
| File size (all levels) | ~33 KB | ~4.3 MB |
| Level format | Inline in EXE | Standalone DGN files |
| 3D geometry | None (2D sprites) | Baked into DGN files |
| Sensors | Hardwired game loop | SDDRVS.TSK scripts |
| Overworld | None | Yes (separate map) |
| Teleporters | Hardwired square types | Scripted in TSK |

## 5. 3D Geometry in DGN Files

DGN geometry sections store pre-computed polygon data (vertices + face indices) for:
- Wall front/side faces at each grid position
- Floor and ceiling meshes per square
- Door/teleporter geometry overlays

This eliminates runtime geometry computation (vs. DM1's raycasting 2D approach), at the cost of enormous file sizes.

**Note:** LEV12 is the largest level file at 321,536 bytes — likely the deepest/boss level. LEV00 at 147,456 bytes is the smallest — the tutorial/entry level.
