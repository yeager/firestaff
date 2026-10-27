# Nexus V1 Dungeon Design Audit

## 1. Level Architecture

**16 authenticated DGN files: LEV00.DGN – LEV15.DGN**

`LEV00.DGN` is the title-sequence entrance asset, not the first playable
dungeon. The retail game starts in the Hall of Champions represented by
`LEV01.DGN`; Firestaff keeps LEV00 available for source inspection but does
not use it to manufacture a gameplay start position.

| Level | Size (bytes) | File |
|-------|-------------|------|
| LEV00 | 147,456 | 64×64 Structure1B title/entrance cells |
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

**Grid size:** 64×64 Structure1B cells per level. Each cell is an 8-byte
big-endian record. The old 32×32 description was a DM1-shaped placeholder and
must not be used for Nexus parsing.

## 2. Level File Format (DGN)



The Structure1B cell is retained as a Saturn-specific source record. Firestaff
does not reinterpret its fields as DM1 square types merely because some bits
are convenient to inspect.

**Why so large?** The DGN files contain embedded 3D geometry (wall polygon data, floor/ceiling mesh vertices) rather than computing it procedurally — unlike DM1 which used a 2D grid + pre-baked sprite rendering.

## 3. Level Scripts / Events

Nexus V1 ships a separate `SLEV00.BIN`–`SLEV15.BIN` task corpus (2,388 to
11,660 bytes in the mounted retail revision), not one 5,448-byte script file.

**Key difference vs. DM1:**
- DM1: sensors hardwired in game loop logic (compile-time behavior)
- Nexus: Saturn task candidates whose event-dispatch/consumer is still opaque

Firestaff currently admits the real Structure1B grid and bounded post-grid
Structure2/Structure3 receipts across the 16 authenticated levels. Structure3
face ownership, Saturn transform/material semantics and VDP1 command order are
still capture-gated. SLEV task bytes are profiled as real SH-2 candidates, but
the event dispatcher and script execution are not enabled.

## 4. Comparison: Nexus vs. DM1 Dungeon Design

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| Levels | 8 (D0–D7) | 16 (LEV00–LEV15) |
| Grid | Variable 16–30 | Fixed 64×64 Structure1B |
| File size (all levels) | ~33 KB | ~4.3 MB |
| Level format | Inline in EXE | Standalone DGN files |
| 3D geometry | None (2D sprites) | Baked into DGN files |
| Sensors | Hardwired game loop | Runtime owner not verified |
| Overworld | None | Not verified; no authenticated map file or runtime consumer |
| Teleporters | Hardwired square types | Runtime consumer not verified |

## 5. 3D Geometry in DGN Files

DGN geometry sections store pre-computed polygon data (vertices + face indices) for:
- Wall front/side faces at each grid position
- Floor and ceiling mesh candidates per square
- Door/teleporter geometry candidates

The files contain bounded geometry/material candidates, but the runtime
consumer still has to be joined to an authenticated Saturn VDP1 capture. A
host projection or software rasterizer is not evidence of original parity.

**Note:** LEV12 is the largest level file at 321,536 bytes and LEV00 is the
smallest at 147,456 bytes. File size alone does not prove tutorial, boss or
start-pose semantics; the retail start selector remains capture-gated.
