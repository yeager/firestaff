# External Reference Sources for Firestaff Parity Work

**Date:** 2026-05-24
**Status:** EXPAND_ONGOING — mer referensmaterial läggs till vid behov

---

## Quick reference: What each source is good for

| Source | URL | Best for |
|--------|-----|----------|
| Community docs | `dmweb.free.fr/community/documentation/` | **Primary reference** för all DM/CSB spelmekanik, grafikformat, creature-data, actions, attacks, dungeon-format |
| Greatstone atlas | `greatstone.free.fr/dm/g_dm.html` | Dungeon maps, graphics atlas, PC 3.4 specific data |
| ReDMCSB source | `~/.openclaw/data/firestaff-redmcsb-source/` | **Primary source code** för DM1/CSB implementation |
| skproject (DM2) | `github.com/gbsphenx/skproject` | **DM2/Skullkeep** source reference |
| CSBWin source | `github.com/BeipDev/CSBWin` | CSB implementation reference, creature/resurrect routing |
| CSB source | `github.com/zelurker/CSB` | CSB source under `src/` (secondary) |
| Dungeon Master Hints | `dmweb.free.fr/community/tools/dungeon-master-hints-disk-for-pc/` | Hints disk data |
| Solutions/maps | `dmweb.free.fr/games/dungeon-master/solutions/` | Champion names, dungeon maps |
| Champions list | `dmweb.free.fr/games/dungeon-master/solutions/champions/` | Champion data |
| Creatures list | `dmweb.free.fr/games/dungeon-master/solutions/creatures/` | Creature data |
| Original DM game | `~/.openclaw/data/firestaff-original-games/DM/` | Original DOS/PC data files |

---

## A. dmweb.free.fr — Community Documentation

**Base URL:** `http://dmweb.free.fr/community/documentation/`

This is the single richest authoritative reference for DM/CSB mechanics. ALL pages below are written and maintained by the community. They cite exact item numbers, byte offsets, and formula derivations.

### A1. Game mechanics

| Page | URL | What it covers |
|------|-----|----------------|
| **Actions and Combos** | `/dungeon-master-and-chaos-strikes-back/actions-and-combos/` | All 44 actions (Fighter, Ninja, Priest, Wizard). Tables: skill, exp, defense mod, stamina, hit prob, damage (1/32nds), fatigue. Atari ST version differences noted. |
| **Attacks and Defenses** | `/dungeon-master-and-chaos-strikes-back/attacks-and-defenses/` | 8 attack types (0 Unconditional → 7 Blast). Cause + defense factors per type. Creature attack type classification. Giggler exception documented. |
| **Creature Details** | `/dungeon-master-and-chaos-strikes-back/creature-details/` | Creature descriptors from GRAPHICS.DAT item 559. Per-creature: view index, attack sound, size, side-attack flag, back-row pref, attack-any-champion, levitation, non-material, height, drop-things, keep-thrown-sharp, see-invisible, night-vision. **Essential for creature parity.** |
| **Creature Generators** | `/dungeon-master-and-chaos-strikes-back/creature-generators/` | Health formula: `HM=0 → LEM×BH+Random(BH/4+1)`, `HM>0 → HM×BH+Random(BH/4+1)`. Sensor type in dungeon.dat. |
| **Skills and Statistics** | `/dungeon-master-and-chaos-strikes-back/skills-and-statistics/` | Skill system, stat growth, experience |
| **Items Properties** | `/dungeon-master-and-chaos-strikes-back/items-properties/` | Item properties, weapon stats, armor stats |

### A2. GRAPHICS.DAT internal documentation

| Page | URL | What it covers |
|------|-----|----------------|
| **Graphics.dat Hidden Code** | `/dungeon-master-and-chaos-strikes-back/graphics.dat-hidden-code/` | Copy protection code embedded in GRAPHICS.DAT items 21, 538, 548. COD1/COD2/COD3/COD4 formats. Atari ST, Apple IIGS, Amiga differences. MD5 hashes per hidden-code item per game version. |
| **Graphics.dat Item 558** | `/dungeon-master-and-chaos-strikes-back/graphics.dat-item-558/` | Item 558 (Atari ST only): creature coordinate definitions, cloud display coordinates, creature graphics reference table. Byte-level with hex offsets. |
| **Graphics.dat Item 559** | `/dungeon-master-and-chaos-strikes-back/graphics.dat-item-559/` | Creature descriptors — the main creature definition table. View index, attack sound ordinal, size, flags (side-attack, back-row, attack-any, levitation, non-material), height, drop-things, thrown-sharp, see-invisible, night-vision. **This is the master creature data source.** |
| **Graphics.dat Item 560** | `/dungeon-master-and-chaos-strikes-back/graphics.dat-item-560/` | Item 560 data |
| **Graphics.dat Item 561** | `/dungeon-master-and-chaos-strikes-back/graphics.dat-item-561/` | Item 561 data |
| **Graphics.dat Item 562** | `/dungeon-master-and-chaos-strikes-back/graphics.dat-item-562/` | Item 562 data |

### A3. File formats

| Page | URL | What it covers |
|------|-----|----------------|
| **Data Files** | `/file-formats/data-files/` | Master file format reference: DMCSB1/DMCSB2/DMII header formats, endianness per file, item attributes (IMG3 width/height, SND3 audio format, TXT1/TXT2 text, FNT1 font, RAW1). GRAPHICS.DAT item size limit: 32KB. Portrait item can hold 64 (8×8) instead of 24. **Essential for asset loading parity.** |
| **Animation Script** | `/file-formats/animation-script/` | Animation scripting format |
| **Dungeon Files** | `/file-formats/dungeon-files/` | Complete dungeon file format: endianness, checksum algorithm, compression. Full table of all dungeon versions with MD5 hashes. **Use for dungeon.dat parsing parity.** |

### A4. Solutions and maps

| Page | URL | What it covers |
|------|-----|----------------|
| **Dungeon Master Hints Disk for PC** | `/community/tools/dungeon-master-hints-disk-for-pc/` | Hints disk data and tools |
| **Maps (Claude Lanthier)** | `/games/dungeon-master/solutions/maps-claude-lanthier/` | Dungeon maps |
| **Champions** | `/games/dungeon-master/solutions/champions/` | Champion roster, names, attributes |
| **Creatures** | `/games/dungeon-master/solutions/creatures/` | Creature reference list |
| **Dungeon Master (root)** | `/games/dungeon-master/` | Root game page |

---

## B. Greatstone DM Reference

**Base URL:** `http://greatstone.free.fr/dm/`

| URL | What it covers |
|-----|----------------|
| `/g_dm.html` | Master overview and provenance |
| `/db_data/dm_pc_34/dungeon.dat/dungeon.html` | **PC 3.4 dungeon maps** (plain, no graphics) |
| `/db_data/dm_pc_34/graphics.dat/graphics.dat.html` | **PC 3.4 graphics atlas** — full GRAPHICS.DAT bitmap atlas |
| `/db_data/dm_pc_34_multi/graphics.dat/graphics.dat.html` | **PC 3.4 multilingual graphics atlas** — separate EN/FR/DE variants. **CRITICAL:** PC34_MULTI has different `dungeon.dat` files per language (`dungeon.dat` EN, `dungeonf.dat` FR, `dungeong.dat` DE). Do not conflate. |

**MEMORY.md rule (2026-05-09):** All GRAPHICS.DAT/dungeon.dat comparisons must hash-lock the exact file variant before claiming parity. File name is NOT identity — SHA256 is.

---

## C. Source Code References

### C1. ReDMCSB (PRIMÄR för DM1/CSB V1)

**Archive:** `ReDMCSB_WIP20210206.7z`
**Local (N2):** `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/`
**Key files:**

| File | Size | What it covers |
|------|------|----------------|
| `DUNGEON.C` | 181,374 B | Dungeon engine source |
| `DEFS.H` | 479,889 B | Master struct definitions (G-variables, constants, structs) |
| `VIDEODRV.C` | 170,071 B | VGA video driver with full palette data |
| `DUNVIEW.C` | (in DUNGEON.C) | Viewport draw functions |
| `MOVESENS.C` | (in DUNGEON.C) | Movement and sensor processing |
| `TIMELINE.C` | (in DUNGEON.C) | Event timeline, door animation |
| `PANEL.C` | (in DUNGEON.C) | HUD/panel rendering |
| `CREATURE.C` | (in DUNGEON.C) | Creature AI and behavior |
| `CHAMPION.C` | (in DUNGEON.C) | Champion stats, food/water drain |

**Use for:** Primary source-locking of ALL DM1 V1 implementation decisions. ReDMCSB is the authoritative source for Firestaff DM1 V1 parity per MEMORY.md.

### C2. skproject (för DM2/Skullkeep)

**GitHub:** `https://github.com/gbsphenx/skproject`
**Locked HEAD:** `585ef61aea1e9b33261cfd8a9712cadbb40604b9`
**N2 mirror:** `~/.openclaw/data/firestaff-dm2-sources/skproject.git`
**Sphenx SKWin:** `https://dmbuilder.sphenxmusics.fr/skwin.php`
**Sphenx package:** `https://dmbuilder.sphenxmusics.fr/skwin/SkWinCurrent.zip`
**SKULL.ASM:** sha256 `a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099`

**Use for:** DM2/Skullkeep specific source reference after Phase 0 provenance gate passes.

### C3. CSBWin (för CSB-referens)

**GitHub:** `https://github.com/BeipDev/CSBWin`
**N2 mirror:** `~/.openclaw/data/firestaff-csbwin-source/CSBWin/`
**Use for:** Secondary reference, särskilt champion resurrect/reincarnate/mouse routing. ReDMCSB is fortfarande primär för DM1 V1.

### C4. CSB source (sekundär CSB-reference)

**GitHub:** `https://github.com/zelurker/CSB`
**N2 mirror:** `~/.openclaw/data/firestaff-csb-source/CSB/`
**Source under:** `src/`
**Use for:** Secondary CSB reference only. ReDMCSB is primary.

---

## D. Original Game Data Files

### D1. DM1 PC 3.4 (SHA-låst referens)

**Canonical N2 path:** `~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/`
**Extracted data:** `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/`
**Files:**
- `DUNGEON.DAT` — dungeon content
- `GRAPHICS.DAT` — all graphics (713 indexes, 363,417 bytes, sha256: `2c3aa836...`)
- `TITLE` — title screen graphic (PC34, not `TITLE.DAT`)
- `SONG.DAT` — music

### D2. CSB data

**Status:** `BLOCKED_ON_REFERENCE` — CSB original data files not yet acquired.

### D3. DM2 data (phase 0 locked)

**Canonical N2 path:** `~/.openclaw/data/firestaff-original-games/DM/_canonical/dm2/`
**Files:**
- `Dungeon-Master-II-Skullkeep_DOS_EN.zip` (sha256 locked)
- `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` (sha256 locked)
- `SKULL.ASM` (sha256 locked)

---

## E. DMWeb Solutions Pages (Hall of Champions, Maps, Champions)

| URL | What it covers |
|-----|----------------|
| `/games/dungeon-master/solutions/champions/` | Champion names, portraits, lore |
| `/games/dungeon-master/solutions/creatures/` | Creature names, types |
| `/games/dungeon-master/solutions/maps-claude-lanthier/` | Dungeon maps |
| `/games/dungeon-master/` | Root DM page |

---

## F. Previously documented references (from PARITY_REFERENCES.md)

See `docs/parity/PARITY_REFERENCES.md` for:
- Section A: Original game data packages (A1-A4)
- Section B: Extracted artifacts (B1-B3: GRAPHICS.DAT extraction, VGA palette recovery, verification screenshots)
- Section C: Firestaff capture artifacts (C1-C3)
- Section D: Source code references (D1-D4)
- Section E: Probe and analysis scripts
- Section F: Documentation and plans

---

## G. Theron's Quest — 5th game (PC Engine/TurboGrafx-16)

**URL:** `http://dmweb.free.fr/games/therons-quest/`

**Platform:** TurboGrafx-16 / PC Engine. Japanese version released 1992-09-18, English version 1993 in USA.

**Game nature:** "Light" adaptation of Dungeon Master. Key differences:
- Only a subset of items, creatures, and spells from DM1
- 7 small dungeons (instead of one large)
- Some dungeons copied/inspired by DM1 and CSB dungeons
- Play as Theron + 3 other champions
- Champions lose skills/items upon completion of each dungeon
- Theron keeps skills/stats but loses items between dungeons
- Game only saved between dungeons (no in-dungeon saves)
- Many Altars of Vi to resurrect dead champions
- Goal: retrieve 7 valuable items, one per dungeon

**Parity status:** OUT OF SCOPE FOR V1. Firestaff V1 targets 1:1 parity with DM1, CSB, DM2 only (per MEMORY.md product contract). Theron's Quest is documented here as reference knowledge only.

---

## H. Key rules for using these sources

1. **Hash-lock before claiming:** All original file comparisons require SHA256 proof of exact file variant. PC34 ≠ PC34_MULTI ≠ Amiga ≠ Atari ST.
2. **ReDMCSB is primary for DM1/CSB V1:** All Firestaff DM1 V1 implementation decisions cite ReDMCSB source lines, not emulator observation.
3. **Greatstone is secondary:** DM web atlas is useful for visual comparison but not authoritative for source-locking.
4. **dmweb community docs are authoritative for game mechanics:** The action tables, creature descriptors, attack types, and file formats on dmweb.free.fr are community-verified and cite exact byte offsets. Use them as evidence for game-mechanic parity.
5. **skproject is primary for DM2:** After Phase 0 provenance gate, DM2 implementation uses skproject + SKULL.ASM as primary source.
6. **CSBWin is secondary for CSB:** Use CSBWin for cross-checking CSB behavior only; ReDMCSB remains authoritative for shared DM1 mechanics.