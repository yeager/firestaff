# Nexus V1 Parity Testing Guide

> **Historical planning snapshot — not current production status.** This
> document predates the real-retail Nexus corpus, source receipts and Saturn
> capture gates. Its statements such as “No end-to-end test exists” and “all
> scaffolding” must not be read as current facts. For the current inventory,
> see [`NEXUS_STRICT_FIDELITY_INVENTORY.md`](NEXUS_STRICT_FIDELITY_INVENTORY.md),
> [`NEXUS_STALE_CLAIM_AUDIT.md`](NEXUS_STALE_CLAIM_AUDIT.md), `TODO.md` and
> `DONE.md`. Format parsing is evidence; runtime parity still requires an
> authenticated Saturn/Mednafen consumer capture.

## Overview

Dungeon Master Nexus (1998) is a 3D polygon remake of DM1, exclusive to Sega Saturn, Japanese only.
Testing Nexus parity = verifying the Firestaff Nexus V1 implementation matches the original Saturn disc behavior at each phase.

The current configured CTest inventory contains 184 Nexus V1/V2/M11 tests.
Against the authenticated external corpus, 173 pass and 11 tests requiring
private Saturn captures or saves are skip-safe. The older 304/14 figures are
historical registration counts, not the current verification result.
This guide is a test plan plus a statement of what is and is not admitted to
production. A green parser test is not a Saturn presentation or gameplay
parity claim.

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
- Parse the authenticated LEV00.DGN-LEV15.DGN corpus; LEV00 is title/entrance
  data and LEV01-LEV15 are the retail dungeon-level assets. Production
  gameplay remains closed until the Saturn LEV01 start pose and consumer are
  authenticated.
- Verify dungeon header: level count, map dimensions, square types
- Compare parsed map against DM1 dungeon.dat for same dungeon layout
- Verify texture loading from VDP1 format (4bpp/8bpp paletted, 15-bit RGB, SH2 big-endian)
- Verify model format: quad list, vertex coordinates, texture references
- Verify champion data: name, stats, inventory structure
- Verify monster data: type, stats, behavior flags, drop tables
- Verify text extraction: Shift-JIS dungeon names, monster names, inscriptions to UTF-8

**Pass criteria:** All 16 dungeon levels parse without error. Current evidence
proves the real DGN grid and bounded Structure1F/2/3 records; Saturn mesh,
texture placement and text consumers remain capture-gated.

---

### Phase 3 -- Core World Model

**How to test:**
- Load LEV01, then place the party only from an authenticated Saturn start
  receipt; do not infer the entry position from the first walkable cell.
- Verify party position/direction matches original game
- Walk through 10 squares: verify tile collision matches original
- Open a door: verify animation, sound, state change
- Trigger a sensor: verify event fires at correct location
- Verify timers advance at correct rate (60 Hz tick, same as DM1)
- Hash world state after 1000 ticks: record as canonical

**Pass criteria:** Host mechanics are deterministic and source-bounded. A
claim of no desync versus Saturn requires an authenticated same-session
capture; the current suite does not make that claim.

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

**Pass criteria:** Source receipts and no-draw boundaries pass. Pixel parity,
VDP1/VDP2 placement and draw order remain open until original Saturn capture
evidence is available.

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

**Pass criteria:** Implemented host mechanics pass deterministic regression
tests. Exact Nexus timing, action ownership, sound feedback and Saturn parity
remain unclaimed where no source or capture evidence exists.

---

### Phase 6 -- Save/Load

**How to test:**
- Save game: record save file
- Load: verify party position, inventory, champion state, dungeon state
- Save during combat, load, verify combat resumed
- Save with projectile in flight, load, verify projectile resolved
- Round-trip: save, quit, load, compare world state hash

**Pass criteria:** Firestaff-native save round-trips are covered. Saturn
8-KB memory-card compatibility remains open until an authentic save is found
and decoded.

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

| Area | Current status |
| --- | --- |
| Provenance | Authenticated external retail corpus is loaded without repacking; ISO/CUE and hash gates are tested. |
| Runtime profile | Nexus launcher and M11 handoff boundaries are regression-tested. |
| Data formats | DGN, DMDF/MNS, PRS3/BPK, FACE, ITEM, SMAP, FONT and SLEV/SAL bounded receipts are tested. |
| World model | Host movement, actions, saves and deterministic mechanics are tested; Saturn start pose is not yet bound. |
| Rendering | Source intake and no-draw receipts are tested; Saturn VDP1/VDP2 consumer and pixel parity remain open. |
| Audio | Authentic SAL/MAP provenance and bounded DataID 0 PCM diagnostics are tested; original selector, SDDRVS/SCSP ownership and playback remain open. |
| Verification | 173/173 executed Nexus tests pass against the external corpus; 11 private capture/save gates are skip-safe. |

The external visual baseline now includes clean window-captured Mednafen
witnesses for the authentic Nexus logo and intro sequence. These are reference
frames only; no menu, LEV01 or Saturn-save claim is made from them.

---

## How to Start Testing (Immediate Steps)

1. Use `/Volumes/Extern-disk/FirestaffUserData/data/nexus` as the external
   authenticated corpus; never generate replacement game data when a real
   member exists.
2. Build the Nexus library and run the CTest selection with `NEXUS_DATA_DIR`
   pointing at that corpus.
3. Treat skipped private Saturn-capture tests as open evidence gates, not as
   passing parity evidence.
4. Add a source/capture receipt before promoting any new consumer, pose,
   texture, audio selector or event semantic.

---

## Reference: DM1 comparison boundary

Nexus is related to DM1, but DM1 parity is not proof of Nexus behavior. Use
DM1/ReDMCSB only for comparative hypotheses; promote Nexus behavior only from
Nexus bytes, disassembly or an authenticated Saturn capture. In particular,
do not infer Nexus start pose, event ownership, save format, sound selectors or
rendering semantics from DM1.

The **only** differences from DM1 are:
- 3D polygon rendering (not 2D sprites)
- Different texture/model assets
- CD audio instead of PC speaker/AdLib
- Japanese text (Shift-JIS)
- Saturn controller (same input semantics as keyboard/mouse)

Everything else is DM1 under the hood.
