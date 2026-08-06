# Reverse Engineering Index

> **Status reviewed 2026-08-06.** This index separates source-locked facts,
> real-data receipts, runtime milestones and open handoff boundaries. Counts
> are not feature-completeness claims; see the [documentation index](../DOCUMENTATION_INDEX.md).

This section documents all functions, data structures, file formats, and other technical details recovered through disassembly and reconstruction of the five Dungeon Master game engines. The information is valuable for game preservation, future reverse engineering efforts, and understanding how these classic RPGs work at the binary level.

## Per-Game Documentation

| Game | Platform | Reference Source | RE Page | Functions | Modules | Parity Evidence |
|------|----------|-----------------|---------|-----------|---------|-----------------|
| [Dungeon Master (DM1)](DM1-Reverse-Engineering) | DOS PC 3.4 | ReDMCSB | [DM1 RE](DM1-Reverse-Engineering) | 1388 F-numbered | 1558 unique IDs | 764 pass documents |
| [Chaos Strikes Back (CSB)](CSB-Reverse-Engineering) | DOS PC 3.4 | ReDMCSB + CSBWin | [CSB RE](CSB-Reverse-Engineering) | 1058 shared with DM1 | 28+ CSB-specific | 28 pass documents |
| [Dungeon Master II (DM2)](DM2-Reverse-Engineering) | DOS | skproject | [DM2 RE](DM2-Reverse-Engineering) | 97+ named | 271 source files | 58 pass documents |
| [Theron's Quest](Therons-Quest-Reverse-Engineering) | PC Engine CD | PC Engine disassembly and CD analysis | [Theron RE](Therons-Quest-Reverse-Engineering) | HuC6280 disassembly + retail media receipts | Track 02 loader and level-frame work | runtime handoff open |
| [DM Nexus](Nexus-Reverse-Engineering) | Sega Saturn | Saturn SH-2 disassembly and retail media analysis | [Nexus RE](Nexus-Reverse-Engineering) | SH-2/VDP/SCSP receipts | DGN/MNS/PRS3/SAL/MAP work | full runtime handoff open |

## Reference Sources

### ReDMCSB (DM1 and CSB)

ReDMCSB is a reconstructed C source tree for the original Dungeon Master and Chaos Strikes Back engines. It provides function identifiers (F0000–F1265+), data structure layouts, and control flow. Firestaff references 190+ individual ReDMCSB source files spanning the complete engine: dungeon management, viewport rendering, combat, champions, audio, input, save/load, and platform-specific paths.

ReDMCSB is the primary behavioral reference but is not FTL's original source. It is strong for control flow, data ownership, and platform-conditioned differences. It is **not** by itself proof of PC 3.4 binary ABI, instruction timing, checksum outcomes, or that a decoded asset is the original asset intended by a given PC34 route.

### skproject (DM2)

skproject is the reconstructed source for Dungeon Master II: Skullkeep. The two central reference files are `SkWinCore.cpp` and `SkGlobal.cpp`. The per-module sources follow the `c_*.cpp` naming pattern (56 files covering AI, combat, graphics, records, events, movement, etc.). Firestaff's DM2 symbol audit tracks 1021+ symbols, with 851 still marked `MISSING` as of the current cycle.

### No Reference Source (Theron's Quest, DM Nexus)

Theron's Quest (PC Engine CD) and DM Nexus (Sega Saturn) have no reconstructed source code available. All reverse engineering is performed directly from disc images and binary analysis. This makes their RE documentation particularly valuable for preservation — the structures and formats documented here may not exist anywhere else.

## Cross-Game Data Formats

### DUNGEON.DAT

The dungeon data format is shared (with variations) across DM1, CSB, and DM2:

| Field | Size | Description |
|-------|------|-------------|
| Header | 44 bytes | Signature (0x8104), map count, thing counts per type |
| Map descriptors | 16 bytes each | Width, height, level, allowed creature types |
| Square data | 1 byte per square | 3-bit element type + 5-bit attributes + thing list flag |
| Thing data | Variable | Linked list chains per square, typed by thing type |

**Element types** (3-bit field, bits 7-5):

| Value | Type | Description |
|-------|------|-------------|
| 0 | Wall | Solid wall, may have ornaments |
| 1 | Corridor | Open floor, walkable |
| 2 | Pit | May be open or closed, drops to level below |
| 3 | Stairs | Connects to adjacent level |
| 4 | Door | Openable barrier with frame and ornament |
| 5 | Teleporter | Instant transport to target coordinates |
| 6 | Fakewall | Appears solid but is walkable (or vice versa) |

**Thing types** (16 types, linked list per square):

| ID | Type | Max Count | Description |
|----|------|-----------|-------------|
| 0 | Door | per map | Door state, ornament, type |
| 1 | Teleporter | per map | Target map/x/y/direction, rotation |
| 2 | Text String | per map | Scroll text, wall inscriptions |
| 3 | Sensor | per map | Triggers, pressure plates, wall switches |
| 4 | Group | per map | Creature groups (up to 4 creatures) |
| 5 | Weapon | 45 types | Swords, axes, bows, ammunition |
| 6 | Armour | 57 types | Helmets, torso, legs, feet, shields |
| 7 | Scroll | per map | Readable text items |
| 8 | Potion | 20 types | Healing, mana, strength, etc. |
| 9 | Container | 3 types | Chests, packs |
| 10 | Junk | 52 types | Miscellaneous objects |
| 14 | Projectile | per map | Active thrown/shot items |
| 15 | Explosion | per map | Active explosion effects |

### GRAPHICS.DAT

Two distinct formats exist:

| Game | Format | Encoding |
|------|--------|----------|
| DM1 PC 3.4 | IMG3 | LZW-compressed indexed graphics |
| CSB | IMG1 (Amiga v3.1) | Nibble-RLE compressed |
| DM2 | GDAT | Section-based with typed records (dtPalIRGB, dtPalette16, dt07, Rect14) |

### Save File Formats

| Game | Format | Key Feature |
|------|--------|-------------|
| DM1 PC 3.4 | F0435 order | F0417/F0418 obfuscation, header + party + groups + events + timeline + tail |
| CSB | F0435 + CSBWin extensions | GAMEBLOCK1, DB11/EXPOOL tails, DSA state |
| DM2 | SKSAVE | skproject-compatible, byte-identical round-trip required |
| Theron's Quest | SRM (gzip-framed) | CRC32/ISIZE validation, atomic export |
| DM Nexus | Saturn SRAM | Bounded save slots |

## Parity Evidence System

The `parity-evidence/` directory contains the project's source-lock receipts.
The current checked-in corpus is counted by the repository rather than copied
into stale narrative numbers; each document anchors a behavioral claim to an
original source location or binary offset and a runnable verification path.
Each document follows a standard format:

```
Status: *_LOCKED
## Anchors
ReDMCSB SOURCEFILE.C:LINE — description
## Verification
./build/test_name — expected return code
```

These documents are machine-checkable receipts, not narrative documentation. A successful pass means the claimed behavior is pinned to a specific original-source line and has a passing test.

| Game | Pass Documents | Coverage Areas |
|------|---------------|----------------|
| DM1 | 764 | Viewport, movement, inventory, combat, champions, saves, startup |
| DM2 | 58 | GDAT rendering, creature AI, combat, records |
| CSB | 28 | DSA opcodes, viewport, startup, saves |
| Theron's Quest | 7 | Track 02 loader, level handoff |
| DM Nexus | 5 | DGN geometry, BPK surfaces, startup |
| Other/shared | 209 | Cross-game modules |

## How to Use This Documentation

**For preservation researchers**: The per-game RE pages document binary formats, function mappings, and data structures that may not be documented elsewhere. The Theron's Quest and Nexus pages are especially valuable since no reference source exists for those games.

**For modders**: The DUNGEON.DAT format documentation, thing type tables, and creature/item type constants provide the foundation for understanding dungeon data.

**For reimplementers**: The F-number function registry (DM1/CSB) and skproject module map (DM2) provide a complete cross-reference between the reconstructed source and Firestaff's implementation.

**For verifiers**: The parity evidence system provides 1071 independently testable claims, each anchored to a specific source location. Run `ctest --test-dir build -j4` to verify all claims.
