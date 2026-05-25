# Nexus V1 Parity Testing Guide

## Overview

Dungeon Master Nexus (1998) is a 3D polygon remake of DM1, exclusive to Sega Saturn, Japanese only.
Testing Nexus parity = verifying the Firestaff Nexus V1 implementation matches the original Saturn disc behavior at each phase.

No end-to-end test exists yet. The codebase is all scaffolding (Phase 0-7 all NOT DONE).

---

## What "Parity" Means for Nexus

Unlike DM1/CSB where ReDMCSB disassembly provides exact source-lock reference, **Nexus has no source code and no ReDMCSB equivalent**. Parity must be established by:

1. Extracting the Sega Saturn CD image (ISO 9660 Track 1)
2. Parsing Saturn asset formats (VDP1/VDP2 textures, SH2 big-endian)
3. Verifying dungeon loading, creature stats, champion stats, spell effects against what the original game produces on screen
4. Reference: DM1 mechanics underneath are identical -- if DM1 parity is proven, Nexus gameplay mechanics are too

---

## Testing Per Phase

### Phase 0 -- Provenance Gate

**How to test:**
- Run: python3 tools/extract_nexus_iso.py /path/to/nexus.bin
- Verify it produces a directory listing of Saturn filesystem
- Verify Track 1 ISO header is detected (MODE1/2352)
- Verify CD audio tracks (Tracks 2-9) are identified
- SHA256-lock the disc image -- no parity work starts before this

**Pass criteria:** Tool produces structured file manifest matching expected Saturn directory layout. SHA256 of disc image recorded.

---

### Phase 1 -- Runtime Profile Split

**How to test:**
- firestaff --profile nexus --data-dir /path/to/nexus_data starts without loading DM1/CSB assets
- Menu shows "Dungeon Master Nexus" entry
- Loading a non-Nexus dungeon with --profile nexus shows diagnostic error
- --profile nexus --diagnostics prints environment info (SDL version, render backend, asset roots)

**Pass criteria:** Separate asset namespace. No cross-contamination with DM1/CSB profiles.

---

### Phase 2 -- Data Formats

**How to test:**
- Parse LEV00.DGN-LEV15.DGN (dungeon levels from disc)
- Verify dungeon header: level count, map dimensions, square types
- Compare parsed map against DM1 dungeon.dat for same dungeon layout
- Verify texture loading from VDP1 format (4bpp/8bpp paletted, 15-bit RGB, SH2 big-endian)
- Verify model format: quad list, vertex coordinates, texture references
- Verify champion data: name, stats, inventory structure
- Verify monster data: type, stats, behavior flags, drop tables
- Verify text extraction: Shift-JIS dungeon names, monster names, inscriptions to UTF-8

**Pass criteria:** All 16 dungeon levels parse without error. Texture/model files decode to viewable data. Text decodes without mojibake.

---

### Phase 3 -- Core World Model

**How to test:**
- Load dungeon, place party at entry position
- Verify party position/direction matches original game
- Walk through 10 squares: verify tile collision matches original
- Open a door: verify animation, sound, state change
- Trigger a sensor: verify event fires at correct location
- Verify timers advance at correct rate (60 Hz tick, same as DM1)
- Hash world state after 1000 ticks: record as canonical

**Pass criteria:** World state hash is deterministic for given input script. No desync vs original.

---

### Phase 4 -- Rendering Pipeline

**How to test:**
- Render static scene at 320x224: verify wall/floor geometry matches screenshots from original Saturn
- Rotate camera 90 degrees: verify viewport updates correctly
- Verify creature rendering: polygon count, texture mapping, z-order
- Verify UI rendering: champion panel, inventory, spell list
- Verify title screen animation (zoom from title screen to dungeon)
- Run EPX 2x upscale: verify 640x448 output without artifacts
- Capture viewport at reference frame and compare against screenshot baseline

**Pass criteria:** Renders match original Saturn screenshots within pixel tolerance. No z-fighting. Correct draw order (floor, wall, object, creature, UI).

---

### Phase 5 -- Mechanics Parity

**How to test:**
- Movement: walk north/south/east/west, verify step timing and collision
- Click: left-click movement, right-click action, verify response
- Item: pick up item, equip to hand, use item -- verify state changes
- Door: open, close, locked state
- Pit: fall, teleport, return
- Teleporter: enter, exit at correct destination
- Champion: take damage, heal, die, resurrect, reincarnate
- Spells: cast each spell type -- verify effect on world
- Combat: engage creature, verify damage calculation, death, drops
- Creature AI: verify creature moves, attacks, retreats correctly
- Sound: verify CD audio track plays for level, sound effects fire

**Pass criteria:** All DM1 mechanics work identically in Nexus context. Combat numbers match. Timing matches.

---

### Phase 6 -- Save/Load

**How to test:**
- Save game: record save file
- Load: verify party position, inventory, champion state, dungeon state
- Save during combat, load, verify combat resumed
- Save with projectile in flight, load, verify projectile resolved
- Round-trip: save, quit, load, compare world state hash

**Pass criteria:** Save/load round-trip produces identical world state. No data loss.

---

### Phase 7 -- Verification Suite

**How to test:**
- Run all test scripts from tests/ that have Nexus-compatible fixtures
- Verify parity evidence files in parity-evidence/nexus/ match expected outputs
- Run deterministic input script: hash world state, compare to baseline
- Cross-platform hash: Ubuntu/macOS/Windows produce identical hash

**Pass criteria:** 100 percent test pass rate. Deterministic hash identical across all 3 platforms.

---

## Current Testing Status

Phase 0 (Provenance): NOT DONE -- extract_nexus_iso.py exists but not tested
Phase 1 (Runtime profile): NOT DONE -- no test
Phase 2 (Data formats): NOT DONE -- no test
Phase 3 (World model): NOT DONE -- no test
Phase 4 (Rendering): NOT DONE -- no test
Phase 5 (Mechanics): NOT DONE -- no test
Phase 6 (Save/load): NOT DONE -- no test
Phase 7 (Verification suite): NOT DONE -- no test

**No Nexus V1 tests exist in the test suite. No CTest entries for Nexus. No parity evidence directory for Nexus.**

---

## How to Start Testing (Immediate Steps)

1. **Get the disc image.** No testing possible without the Sega Saturn ISO.
2. **Run the ISO extractor:** python3 tools/extract_nexus_iso.py nexus.bin
3. **Build the Nexus static library:** cmake -B build && cmake --build build -- libfirestaff_nexus.a compiles without errors
4. **Wire Nexus into a game binary** (Phase 1) -- currently libfirestaff_nexus.a exists but is not linked into any executable
5. **Write the first integration test** once an executable exists with --profile nexus

---

## Reference: DM1 Parity as Proxy

Since Nexus is a 3D remake of DM1 with identical mechanics underneath:
- If DM1 V1 combat parity is proven -> Nexus combat mechanics are proven (same formulas, same creature stats)
- If DM1 V1 movement parity is proven -> Nexus movement mechanics are proven
- If DM1 V1 dungeon layout matches -> Nexus dungeon loading is proven

The **only** differences from DM1 are:
- 3D polygon rendering (not 2D sprites)
- Different texture/model assets
- CD audio instead of PC speaker/AdLib
- Japanese text (Shift-JIS)
- Saturn controller (same input semantics as keyboard/mouse)

Everything else is DM1 under the hood.