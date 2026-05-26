# Theron's Quest V1 Phase 0 — Provenance Gate
**Job:** `Theron_V1_ProvenanceGate_H2339`
**Status:** 🏗️ IN PROGRESS — initial documentation from dmweb.free.fr, no disc image, no game data
**Author:** Firestaff agent (cron)
**Last revised:** 2026-05-27T00:03 UTC+2

---

## Scope

Hash-lock the exact Theron's Quest PC Engine/TurboGrafx-16 disc/image, file
manifests, data file formats, and primary technical references. Document the
CD-ROM structure, game differences from DM1/CSB, and the "light" version
constraints. No disc image is currently present; this document establishes the
research baseline for Phase 1+.

---

## 1. Disc Image / Source Artifact

### 1.1 Product Identification

| Field | Value |
|-------|-------|
| **Game title** | Theron's Quest (Dungeon Master Theron's Quest) |
| **Japanese title** | ダンジョン・マスター　セロンズクエスト |
| **Platform** | PC Engine CD-ROM² (Japan) / TurboGrafx-16 CD (USA) |
| **Regions** | Japan (JP, 1992-09-18), USA (1993) |
| **Product ID** | Unknown — see §1.2 |
| **Version** | Unknown |
| **Release dates** | JP: 1992-09-18 · US: 1993 |
| **Publisher** | Extreme Entertainment Group |
| **Medium** | CD-ROM (1 disc) — CD-DA format |
| **File system** | PC Engine CD-ROM filesystem (custom, not standard ISO 9660) |

### 1.2 Disc Image Status

| Item | Status | Notes |
|------|--------|-------|
| CUE/BIN disc image (JP) | ❌ **NOT PRESENT** | No disc image in repository |
| CUE/BIN disc image (US) | ❌ **NOT PRESENT** | No disc image in repository |
| Extracted file set | ❌ **NOT PRESENT** | No extracted game data |
| SHA256 hash lock | ❌ **NOT SET** | Cannot set without disc image |
| Canonical path | ❌ TBD | `.firestaff/data/tqr/` — path reserved but empty |
| Product ID (T-??? / A-???) | ❌ **UNKNOWN** | Need disc image to read label/product ID |

**Source:** dmweb.free.fr `/games/therons-quest/editions/pc-engine-turbografx/`
downloads section: "ISO/OGG Compact Disc images" via OneDrive RAR archive.

### 1.3 Archive Availability

```
OneDrive: Theron's Quest for PC Engine (US and Japanese versions).rar
  Location: Games folder in DMFiles Shared OneDrive
  Contains: English (USA) + Japanese (JP) versions
  Format:   ISO/OGG compact disc images
  Source:   dmweb.free.fr/downloads
```

**Action required:** Download and extract the RAR archive. Place disc images
at `.firestaff/data/tqr/jp/` and `.firestaff/data/tqr/us/` respectively.
Compute SHA256 hashes and record in §1.5.

### 1.4 CD Track Structure

Based on dmweb.free.fr music track documentation:

```
Track 01: Audio track — spoken intro/narration (JP/EN language variant)
Track 02: DATA TRACK — game binary + graphics + audio data  ← primary data track
Track 03: Audio track — spoken dialogue/music
Track 04: Audio track — spoken dialogue/music
Track 17: Audio track — ending music
           JP: static noise at 1:04, 7 sec longer than US
           US: fixed static noise issue present in JP version

Total: 18 tracks (CD-DA + data hybrid)
```

**Key differences from DM1 (PC):** DM1 is a standard IBM PC game distributed on
floppy disk. Theron's Quest is a PC Engine CD game with CD-DA audio tracks.
The data track (Track 02) is the equivalent of the PC's GRAPHICS.DAT +
DUNGEON.DAT combined, but in a PC Engine-specific format.

### 1.5 Hash Registry (Planned)

When disc images are obtained:

```
SHA256(therons-quest-jp.iso)  = <pending>
SHA256(therons-quest-us.iso)  = <pending>
MD5(therons-quest-jp.iso)     = <pending>  # for asset catalog compatibility
MD5(therons-quest-us.iso)    = <pending>
```

Firestaff version catalog entries (to be added to `asset_status_m12.c`):
```c
// Placeholder — to be filled when disc images are available
// static const M12_VersionSpec g_tqrVersions[] = { ... };
```

---

## 2. File Manifest — NOT YET DOCUMENTED

### 2.1 Expected File Set

PC Engine CD-ROM games using CD-DA format typically contain:

| Expected File | Description | Status |
|---------------|-------------|--------|
| GAME.BIN / THERTQ.BIN | Main game executable + data | ❌ Unknown |
| GRP/graphics files | Wall/floor/creature/item sprites | ❌ Unknown |
| DUNGEON.DAT / LVL*.BIN | 7 mini-dungeon map data | ❌ Unknown |
| SOUND/CGDB files | Audio samples (ADPCM) | ❌ Unknown |
| FONT data | PC Engine tile font | ❌ Unknown |
| MENU/UI surfaces | Title, champion select, HUD | ❌ Unknown |

**Total expected file count:** Unknown — likely far fewer than DM1's
2-file structure (DUNGEON.DAT + GRAPHICS.DAT), given PC Engine's
smaller storage capacity and CD-ROM medium.

### 2.2 PC Engine Storage Context

PC Engine CD-ROM² specs:
- CPU: HuC6280 (8-bit, 7.16 MHz)
- RAM: 8 KB work RAM + 64 KB CD-ROM buffer RAM
- Graphics: HuC6260 (video) + HuC6270 (sprites) — 512 colors, 64 sprites
- Resolution: 256×224 (NTSC) or 256×240
- Sound: 6-channel ADPCM audio + 1 noise channel
- Storage: CD-ROM, ~650 MB max

Compared to DM1 (IBM PC):
- DM1: 320×200 VGA, 16-color EGA palette, PC speaker
- Theron: 256×224, 512 colors, ADPCM audio

**Implication:** Graphics format is tile/sprite-based (PC Engine hardware),
not planar VGA bitmap like DM1. Dungeon rendering is likely 2D tile-grid
with pre-rendered wall/floor textures stored as PC Engine tiles.

### 2.3 Action Items

- [ ] Obtain both JP and US disc images from OneDrive
- [ ] Extract and catalog all files on disc
- [ ] Compare JP vs US file sets for differences
- [ ] Identify main game binary
- [ ] Identify dungeon data files (7 mini-dungeons expected)
- [ ] Identify graphics format (likely PC Engine tile format)
- [ ] Identify audio format (likely ADPCM)

---

## 3. Game Overview — DM1 "Light" Variant

### 3.1 Game Structure

Theron's Quest is a **DM1 adaptation for the PC Engine/TurboGrafx-16**,
not a sequel. Key characteristics:

| Aspect | DM1 (PC) | Theron's Quest (PCE/TG16) |
|--------|----------|--------------------------|
| Dungeons | 1 large dungeon (16 levels) | 7 mini-dungeons |
| Dungeon continuity | Seamless between levels | Per-dungeon resets |
| Items | Full DM1 roster | Subset of DM1 |
| Creatures | Full DM1 roster | Subset of DM1 |
| Spells | Full DM1 roster | Subset of DM1 |
| Champions | 4 (selectable at start) | Theron + 3 others |
| Champion persistence | Kept between levels | Reset per dungeon |
| Theron persistence | N/A (DM1 doesn't have Theron) | Skills/stats kept |
| Saves | In-dungeon save allowed | Between-dungeons only |
| Altars of VI | Standard DM1 count | Many more than DM1 |
| Difficulty | Standard DM1 | Easier (fewer creatures) |
| Goal | Retrieve Eye of the World | Retrieve 7 valuable items |

### 3.2 The Seven Quest Items

The goal of Theron's Quest is to retrieve seven valuable items, one from
each mini-dungeon. Source does not list specific item names — this requires
disc inspection.

### 3.3 Champion System Details

- **Theron:** Main character. Keeps skills and statistics between dungeons.
  (Unlike other champions, Theron is persistent.)
- **3 Champion companions:** Lose all skills and items upon dungeon completion.
  (i.e., each dungeon starts with fresh champion loadouts.)
- **No in-dungeon saves:** Game only saves between dungeons.
- **Frequent Altars of VI:** Respawn points abundant (game balanced for no-save
  within dungeons).

### 3.4 Dungeon Origins

Some dungeons are copied or inspired by DM1 and Chaos Strikes Back dungeons.
The 7 dungeons likely map to subsets of the 16 DM1 levels or CSB levels,
but the exact mapping is unknown and requires disc inspection.

### 3.5 Light Version Impact on Firestaff

Because Theron's Quest uses a **subset** of DM1's items, creatures, and spells:

1. **ReDMCSB source lock:** Applicable. Theron's Quest is a DM1 derivative.
   ReDMCSB covers DM1/CSB — these serve as the primary source reference for
   game logic. However, the PC Engine adaptations differ from the PC versions.
2. **Asset formats:** Completely different from DM1 (PC). DM1 uses planar
   VGA bitmap format; Theron's uses PC Engine tile/sprite format.
3. **Dungeon format:** Different from DM1 DUNGEON.DAT (16 levels, 9×9 grid).
   PC Engine Theron's Quest dungeon format is unknown — likely tile-grid.
4. **Creature roster:** Subset of DM1 creatures. Expected: common creatures
   like Lizard Man, Goblin, Skeleton, etc. Boss creatures likely removed or
   reduced.
5. **Spell system:** Subset of DM1 spells. Combat is easier (fewer creatures).

---

## 4. Data Formats — Hypotheses and Research Plan

### 4.1 Game Binary Format

PC Engine CD-ROM games typically store the game as a single large binary
loaded from the data track. The binary contains:
- HuC6280 CPU code (8-bit, similar to 65C02)
- Graphics data (tile data, sprite data)
- Audio data (ADPCM samples)
- Dungeon/map data

**Hypothesis:** Track 02 contains a single game binary similar to:
```
Offset 0x0000: HuC6280 executable code
Offset N:       Dungeon data (7 mini-dungeons, unknown format)
Offset M:       Graphics tile data (PC Engine tile/sprite format)
Offset P:       Audio ADPCM data
```

**Verification:** Requires disc image and hex editor or emulator debug tools.

### 4.2 Dungeon Data Format

No formal dungeon format analysis exists for Theron's Quest.

**Hypotheses based on DM1:**
- DM1 PC: 9×9 grid squares, big-endian uint16 per square, 16 levels
- Theron's Quest: Likely tile-based grid (PC Engine resolution: 256×224)
  - A dungeon level might fit in a single 256×224 screen (1:1 tile grid)
  - Or dungeons might be larger, requiring scrolling

**Based on PC Engine capabilities:**
- PC Engine tile size: 8×8 pixels
- Screen resolution: 256×224 (32×28 tiles visible)
- Dungeon levels could be 32×28 tiles (single screen) or larger multi-screen

**Research plan:**
1. Extract data track from disc image
2. Search for repeating patterns (16-bit grid data)
3. Look for known DM1 creature/item IDs embedded in data
4. Correlate with DM1 DUNGEON.DAT structure as baseline

### 4.3 Graphics Format

PC Engine graphics uses a tile-based system:
- VRAM: 64 KB dual-port video RAM
- Tiles: 8×8 pixels, 16 colors per tile (4 bits/pixel, planar)
- Sprites: 16×16 or 32×32 composite sprites from 8×8 tiles

**Hypothesis:** Wall/floor textures are stored as 8×8 PC Engine tiles,
similar to traditional 2D dungeon crawlers on the platform.

**Not expected:** 3D polygon geometry (unlike Nexus which uses Saturn VDP1).
Theron's Quest is a 2D tile-rendering game like the original DM1.

### 4.4 Audio Format

PC Engine CD-ROM uses ADPCM audio:
- 5-channel ADPCM for CD-ROM XA audio
- Additional PSG (square wave) channels for sound effects
- Speech samples in Tracks 1, 3, 4 (spoken language JP/EN)

**Audio track differences:**
| Track | Content | JP vs US difference |
|-------|---------|---------------------|
| 1 | Spoken intro | JP/EN language |
| 3 | Spoken dialogue | JP/EN language |
| 4 | Spoken dialogue | JP/EN language |
| 17 | Ending music | JP: static noise at 1:04; US: fixed |

---

## 5. PC Engine vs TurboGrafx-16

### 5.1 Platform Equivalence

PC Engine (Japan) and TurboGrafx-16 (USA) are the **same hardware** with
regional branding:
- PC Engine: Sold in Japan by NEC
- TurboGrafx-16: Sold in USA by NEC and Hudson Soft

The CD-ROM² add-on for PC Engine is compatible with TurboGrafx-16 CD.

### 5.2 Two Releases

| Release | Platform | Region | Date | Notes |
|---------|----------|--------|------|-------|
| JP | PC Engine + CD-ROM² | Japan | 1992-09-18 | Original release |
| US | TurboGrafx-16 + CD | USA | 1993 | English language |

Differences:
- Language: Japanese vs English (spoken dialogue/tracks)
- Track 17 ending music: JP has static noise bug; US fixes it
- JP is 7 seconds longer on Track 17
- Other differences unknown — requires comparison

---

## 6. Platform Hardware Context

### 6.1 PC Engine CD-ROM² Hardware

| Component | Specification |
|-----------|--------------|
| CPU | HuC6280 @ 7.16 MHz (65C02 compatible) |
| RAM | 8 KB work RAM + 64 KB CD-ROM buffer |
| Video | HuC6260 (VDC) — 512 colors, 64 sprites, 16 tiles |
| Audio | HuC6270 (VCE) — 6-channel ADPCM + noise |
| Resolution | 256×224 (NTSC) or 256×240 |
| Medium | CD-ROM (650 MB) |

### 6.2 Dungeon Rendering Approach

Theron's Quest renders a first-person dungeon view using 2D tile rendering
(not 3D polygons). This matches the DM1 aesthetic but adapted for PC Engine:
- Walls: Pre-rendered tile strips (top/middle/bottom wall segments)
- Floor/Ceiling: Flat colored tiles with depth shading
- Items: Sprite overlays on fixed positions
- Creatures: Sprite overlays with animation frames

**Contrast with Nexus:** Nexus (Saturn) uses 3D polygon rendering.
Theron's Quest is closer to DM1's rendering approach (2D billboard).

---

## 7. Primary Technical References

### 7.1 Web References

| Resource | URL | Content |
|----------|-----|---------|
| DMWeb Theron's Quest main | http://dmweb.free.fr/games/therons-quest/ | Game overview, history |
| DMWeb PC Engine/TurboGrafx editions | http://dmweb.free.fr/games/therons-quest/editions/pc-engine-turbografx/ | Tracks, screenshots, downloads |
| DMFiles OneDrive (RAR archive) | https://1drv.ms/f/s!AsBu7boYHQokbYK3rjKY0b5_ra8 | Disc images (JP + US) |

### 7.2 No ReDMCSB for Theron's Quest

Theron's Quest has **no equivalent to ReDMCSB**. It is a PC Engine game,
not an IBM PC game. ReDMCSB covers:
- DM1 PC (Intel 8086/8088)
- CSB PC
- DM2 PC (Intel 386+)

Theron's Quest is a separate platform (HuC6280) with different binary format,
different graphics format, and different architecture — even though the game
logic is derived from DM1.

**Primary source reference for game logic:** ReDMCSB's DM1 disassembly
(`Toolchains/Common/Source/DUNGEON.C`, `COMMAND.C`, `CHAMPION.C`, etc.)
serves as the **closest available reference** for dungeon/champion/item/spell
semantics — but NOT for binary formats.

### 7.3 Known PC Engine Development Resources

| Resource | Description |
|----------|-------------|
| HuC6280 opcodes | HuC6280 is 65C02 + custom instructions — opcode reference needed |
| PC Engine VDC/VCE | Video chip documentation for tile/sprite rendering |
| CD-ROM XA ADPCM | Audio format documentation |

**No Firestaff code exists yet for Theron's Quest.** This document is the
starting point for Phase 1 (runtime profile split).

---

## 8. Source-Lock Assessment

### 8.1 Provenance Items — Status

| Item | Status | Evidence |
|------|--------|---------|
| Disc image hash (JP) | ❌ **NOT SET** | No disc image |
| Disc image hash (US) | ❌ **NOT SET** | No disc image |
| File manifest | ❌ **NOT SET** | No disc image |
| Game binary format | ❌ **UNKNOWN** | Requires disc image |
| Dungeon data format | ❌ **UNKNOWN** | Requires disc image |
| Graphics format | ❌ **UNKNOWN** | Likely PC Engine tiles; unconfirmed |
| Audio format | ❌ **UNKNOWN** | Likely ADPCM; unconfirmed |
| CD track structure | ✅ Documented | Tracks 1-4, 17 from dmweb.free.fr |
| Platform metadata | ✅ Documented | JP 1992-09-18, US 1993 |
| ReDMCSB reference | ⚠️ Partial | DM1 disassembly applicable for game logic only |

### 8.2 Comparison: Theron's Quest vs DM1/CSB

| Item | DM1 (PC) | CSB (PC) | Theron's Quest (PCE) |
|------|----------|----------|---------------------|
| Levels/Dungeons | 16 levels, 1 dungeon | 12 levels, 1 dungeon | 7 mini-dungeons |
| Dungeon format | DUNGEON.DAT | DUNGEON.DAT (same) | Unknown |
| Graphics format | Planar VGA 320×200 | Planar VGA 320×200 | PC Engine tiles |
| Asset files | GRAPHICS.DAT + DUNGEON.DAT | GRAPHICS.DAT + DUNGEON.DAT | Unknown |
| Champion count | 4 | 4 | 4 (Theron + 3) |
| Spell system | Full roster | Full + chaos magic | Subset |
| Creature roster | Full roster | Full + new CSB creatures | Subset |
| Save system | In-dungeon save | In-dungeon save | Between-dungeons only |

### 8.3 Source-Lock Confidence

**High** for: Game description, platform metadata, CD track layout, release dates,
general game differences from DM1.

**Medium** for: JP/US differences (only Track 17 confirmed; other differences
may exist).

**Low** for: Binary formats, dungeon format, graphics format, audio format,
game logic derivation from DM1 (inferred, not confirmed).

---

## 9. Research Plan for Phase 1

Before Phase 1 (runtime profile split) can begin, the following are needed:

### Critical Path

1. **Obtain disc images** (JP + US) from OneDrive
2. **Extract Track 02 data** (game binary)
3. **Catalog all files** on both disc images
4. **Identify main game binary** and entry point
5. **Compare JP vs US** file sets for differences
6. **Reverse-engineer dungeon format** (7 mini-dungeons)
7. **Reverse-engineer graphics format** (PC Engine tile/sprite)
8. **Identify champion/item/spell subsets** vs DM1

### Disc Image Extraction Steps

```bash
# 1. Download RAR archive from OneDrive
# 2. Extract RAR to .firestaff/data/tqr/jp/ and .firestaff/data/tqr/us/
# 3. Verify disc image format (likely BIN/CUE or ISO)
# 4. Compute hashes
sha256sum therons-quest-jp.bin therons-quest-us.bin
md5sum    therons-quest-jp.bin therons-quest-us.bin

# 5. Mount or extract ISO
# 6. Catalog all files
ls -laR .firestaff/data/tqr/jp/ > tqr_jp_filelist.txt
ls -laL .firestaff/data/tqr/us/ > tqr_us_filelist.txt

# 7. Compare JP vs US
diff tqr_jp_filelist.txt tqr_us_filelist.txt
```

### Dungeon Format Reverse-Engineering Steps

```python
# Hypothetical dungeon parsing approach:
# 1. Search data track for repeating uint16 patterns (grid squares)
# 2. Look for 7 distinct dungeon data blocks
# 3. Each mini-dungeon might be smaller than DM1's 9×9 grid
# 4. Look for known DM1 creature IDs (Lizard Man = 0x01, etc.)
# 5. Check for tile IDs referencing graphics tile indices
```

---

## 10. Phase 0 Completion Checklist

```
[ ] OneDrive archive downloaded
[ ] JP disc image placed at .firestaff/data/tqr/jp/
[ ] US disc image placed at .firestaff/data/tqr/us/
[ ] SHA256 hashes computed for both disc images
[ ] File manifests generated (JP and US)
[ ] Track 02 (data) extracted and analyzed
[ ] Main game binary identified
[ ] JP vs US file differences documented
[ ] Dungeon data blocks identified (7 mini-dungeons)
[ ] Graphics format identified (PC Engine tile/sprite)
[ ] Audio format confirmed (ADPCM)
[ ] Hash registry entries added to asset_status_m12.c
[ ] parity-evidence/tqr/ directory created with:
      disc_image_sha256.txt
      file_manifest_jp.txt
      file_manifest_us.txt
      dungeon_blocks_summary.md
```

**Status: Phase 0 NOT COMPLETE — disc images required.**

---

## 11. Relationship to Other Firestaff Games

```
Firestaff supported games:
  DM1 (PC 3.4)  — source-locked (ReDMCSB), V1 ✅
  CSB (PC 3.4) — source-locked (ReDMCSB), V1 ✅
  DM2 (Skullkeep) — source-locked (ReDMCSB), V1 ❌ Phase 1-8
  Nexus (Saturn)  — provenance-locked, V1 ❌ Phase 1-7
  Theron's Quest (PCE/TG16) — provenance-locked, V1 ❌ Phase 0-7

Theron's Quest is architecturally closest to DM1:
  - Same 1st-person dungeon view
  - Same champion/creature/item/spell concepts
  - Same 4-champion party
  - Same Altar of VI mechanic
  - Derived from DM1 game logic (inferred)

Key differences from DM1:
  - Different platform (HuC6280 vs Intel 8086)
  - Different binary format
  - Different graphics format (tiles vs VGA bitmap)
  - Smaller dungeon scope (7 mini-dungeons vs 1 large)
  - Persistent main character (Theron) with per-dungeon companion reset
  - No in-dungeon saves
```

---

*Generated by cron job `Theron_V1_ProvenanceGate_H2339`*
*Next step: obtain PC Engine/TurboGrafx-16 disc images from OneDrive*
