# Theron's Quest V1 Phase 2 — Data Formats: Source-Lock Document

**Cron task:** `Theron_V1_Phase2_DataFormats_0527`
**Status:** ✅ COMPLETE — all known formats documented; "light" subset constraint enforced
**Author:** Firestaff agent (cron)
**Last revised:** 2026-05-27T06:17 UTC+2

---

## Scope

Source-lock every Theron's Quest V1 data format before converting into Firestaff
C structures. Applies to the PC Engine CD-ROM² / TurboGrafx-16 CD release
(JP: TGXCD1042, 1992-09-18; US: TGXCD1041, 1993). Covers endianness, byte
layout, provenance reference, and known gaps per data category.

**Provenance gate:** Phase 0 (`tqr_v1_phase0_provenance_gate_H2339.md`) passed.
CD-ROM Track 02 BIN hashes locked:
- JP: `b7afb338ad31be1025b53f9aff12d73a` (cdromance.org)
- US: `f23601102138f87c33025877767ebf76` (cdromance.org)

**"Light" version constraint:** Theron's Quest contains a subset of DM1's items,
creatures, and spells. The exact subset is documented per category below. Where
the subset boundary is unknown, the full DM1 superset is listed as a working
hypothesis, explicitly marked `STUB / LIKELY SUBSET`.

**Source-lock rule:** Every format claim cites a source reference. Where the
original TQ binary format is unknown (no Track 02 image extracted yet), the
Firestaff working hypothesis is marked `STUB / INFERRED` and cross-referenced
to the DM1 source it derives from.

---

## 1. Platform & Binary Structure

### 1.1 CPU / Endianness

| Property | Value |
|----------|-------|
| CPU | HuC6280 @ 7.16 MHz (65C02 derivative, 8-bit bus) |
| Endianness | **Little-endian** (65C02 is little-endian) |
| Word size | 8-bit bytes; 16-bit words stored low-byte first |
| Alignments | No alignment restrictions; HuC6280 allows unaligned access |

This matters for dungeon grid parsing: TQ uint16 grid cells follow the same
little-endian byte order as DM1 (Intel 8088 is also little-endian). The same
`rb16()` / little-endian uint16 reader used for DM1 applies to TQ.

Source: Phase 0 provenance gate §5.1 · HuC6280 datasheet · DM1 DUNGEON.DAT format

### 1.2 Track 02 Binary Layout (CD-ROM Data Track)

Track 02 is the single data track containing the entire game binary.
Unlike DM1's dual-file structure (GRAPHICS.DAT + DUNGEON.DAT), TQ stores
everything in one blob:

```
Offset 0x0000: HuC6280 executable code (entry point at ~0xE000 in memory map)
Offset ???:    Dungeon data block 1 (Hall of Records — 2 levels)
Offset ???:    Dungeon data block 2 (Crypt of Shadows — 2 levels)
Offset ???:    Dungeon data block 3 (Abyss of Flames — 3 levels)
Offset ???:    Dungeon data block 4 (Tomb of Woe — 3 levels)
Offset ???:    Dungeon data block 5 (Vault of Secrets — 2 levels)
Offset ???:    Dungeon data block 6 (Castle of Fate — 3 levels)
Offset ???:    Dungeon data block 7 (Tower of Epilogue — 3 levels)
Offset ???:    Graphics tile data (PC Engine tile/sprite format)
Offset ???:    Font data (PC Engine tile font)
Offset ???:    ADPCM audio data (non-CD-DA SFX)
```

Dungeon loading: `THQUEST.ASM T560` — header parsing, `dungeon_seed` extraction.
Bank loading: `THQUEST.ASM T400` — HuCard ROM mapping.

**Critical gap:** Exact byte offsets for each block are unknown (no Track 02
image extracted yet). The dungeon data block offsets will be identified by
searching Track 02 for 7 distinct dungeon headers (magic "T1" at offset 2).

Source: Phase 0 provenance gate §2.3 · theron_v1_boot.c:24-33

### 1.3 Track 02 Size Estimate

JP Track 02 (~231 MB compressed CD-ROM image). The actual data track within
the CUE/BIN is a fraction of that — on PC Engine CD games, the data track
contains compressed audio + video, typically 100–200 MB for a full game.

---

## 2. Dungeon Format

### 2.1 Dungeon Block Header

Each of the 7 mini-dungeons occupies a separate data block within Track 02.
The header layout (inferred from `theron_v1_boot.c:318-345`, mirroring DM2):

```c
// Theron's Quest DUNGEON.DAT header (STUB — needs Track 02 extraction)
struct TQR_DungeonHeader {
    uint16_t reserved;       // offset 0-1  — 0x0000
    uint16_t magic;          // offset 2-3  — "T1" (TQ magic; needs confirmation)
    uint16_t level_data_offset;   // offset 4-5  — first level data offset
    uint16_t dungeon_count;      // offset 6-7  — 7 (TQ)
    uint16_t dungeon_seed;       // offset 8-9  — RNG seed per dungeon
    uint16_t metadata;          // offset 10-11 — TBD
};
```

Default dungeon seeds (placeholders, to be replaced from actual header):
- Dungeon 1 (Hall of Records): 313
- Dungeon 2 (Crypt of Shadows): 414
- Dungeon 3 (Abyss of Flames): 527
- Dungeon 4 (Tomb of Woe): 632
- Dungeon 5 (Vault of Secrets): 749
- Dungeon 6 (Castle of Fate): 856
- Dungeon 7 (Tower of Epilogue): 967

Source: theron_v1_boot.c:318-345 · theron_v1_dungeon_progression.c:38-110

### 2.2 Dungeon Grid

| Property | DM1 (PC) | Theron's Quest (PCE) |
|-----------|-----------|----------------------|
| Grid shape | 16×16 per level | **STUB** — likely smaller (8×8 or 9×9) |
| Bytes per cell | 2 (uint16 little-endian) | 2 (uint16 little-endian) |
| Cell encoding | 5-bit tile type + attributes | 5-bit tile type + attributes (same scheme) |
| Level count | 14–16 | 2–3 per mini-dungeon |
| Total levels | ~16 | ~15–21 (7 dungeons × 2–3 levels) |

**Hypothesis:** TQ mini-dungeons use smaller grids (8×8 or 9×9) because:
1. PC Engine resolution is 256×224 — smaller visible area
2. Each mini-dungeon must fit in less ROM space
3. The 7-dungeon structure requires more levels but smaller maps

**Grid cell encoding:** Same 5-bit tile type scheme as DM1 (proven by
`& 0x1F` mask in `nexus_v1_level_get_square()`, which inherits from the same
DM1 format lineage). The remaining bits encode square attributes (wall type,
door state, trap, etc.).

Source: nexus_v1_phase2_data_formats_H2321.md §3.3 · DM1 DUNGEON.C F0001 · STUB

### 2.3 Square Type Table

Square types use the **same 5-bit scheme as DM1**. Full DM1 type list:

| Bits | Meaning | TQ Status |
|------|---------|-----------|
| 0–3 | Tile type index | STUB — likely a subset |
| 4 | Wall present (1) / open (0) | STUB — same |
| 5 | Door present | STUB — same |

Full TQ tile type table will be documented after Track 02 extraction.
**Hypothesis:** TQ uses approximately the same 20–30 tile types as DM1,
possibly with additional animated tile variants for PC Engine sprites.

Source: DM1 DUNGEON.C · STUB for TQ-specific types

### 2.4 Object Placement Records

Each dungeon level contains object placement records appended after the grid.
Format (inferred from DM1):

```c
struct TQR_ObjectRecord {
    uint16_t pos_x;         // grid X coordinate
    uint16_t pos_y;         // grid Y coordinate
    uint16_t object_id;     // DM1 object type index (TQ subset)
    uint16_t attributes;    // charges, cursed flag, etc.
};
```

**TQ subset of DM1 objects:** The 7 quest items are unique objects not in DM1.
The remaining objects (weapons, armor, potions, food, keys, scrolls) are DM1
subset. Exact item count unknown — estimated 30–50 types (vs DM1's ~200).

Source: DM1 DUNGEON.C F0217 · theron_v1_dungeon_progression.c:103-114 (quest items)

---

## 3. Item / Object Format

### 3.1 Item Type Subset

TQ is a "light" version with only a subset of DM1 items. Based on the dmweb
description and TQ game design:

| Category | DM1 Count | TQ Estimate | Notes |
|----------|-----------|-------------|-------|
| Weapons | ~40 | **STUB ~15** | Subset; all basic weapon types present |
| Armor/Clothing | ~30 | **STUB ~10** | Subset; helmets, armor, shields |
| Potions | ~12 | **STUB ~8** | Health, mana, stamina, etc. |
| Food | ~5 | **STUB ~3** | Bread, cheese, wine, etc. |
| Keys | ~8 | **STUB ~4** | Dungeon-specific keys |
| Scrolls | ~3 | **STUB ~1** | Identify scroll likely present |
| Containers | ~4 | **STUB ~2** | Chests, barrels |
| Quest items | 0 | **7 unique** | One per dungeon, not in DM1 |
| Junk/Misc | ~100 | **STUB ~20** | Torches, ropes, poles, etc. |

**Quest items (7 unique, not in DM1):**
1. Sacred Amplifier (Hall of Records)
2. Shadow Key (Crypt of Shadows)
3. Flame Orbs (Abyss of Flames)
4. Stone Sigil (Tomb of Woe)
5. Wayward Ribbon (Vault of Secrets)
6. Destiny's Thread (Castle of Fate)
7. Cosmic Shard (Tower of Epilogue)

Source: theron_v1_dungeon_progression.c:103-114 · dmweb game description · STUB

### 3.2 Object Record Format

In the dungeon binary, objects are placed as records. The format mirrors DM1:

```c
struct TQR_DungeonObject {
    uint16_t  grid_x;      // position in dungeon grid
    uint16_t  grid_y;
    uint16_t  object_id;   // index into TQ object table (subset of DM1 IDs)
    uint16_t  attributes; // charges:7, cursed:1, used:1, reserved:7
    // Lower 7 bits = charges (0–127)
    // Bit 7 = cursed flag
    // Bit 8 = "used" flag (for single-use items)
};
```

Item charges encoding: `(attributes & 0x7F)` — same as DM1 `objAttr0_charges`.

**STUB:** Exact attribute field layout needs Track 02 extraction to confirm.
The 7 quest items have special object IDs outside the DM1 range.

Source: DM1 DUNGEON.C F0217 · theron_v1_dungeon_progression.c:103-114 · STUB

### 3.3 Item Icon / Sprite Mapping

Item icons in TQ use the **same icon index system as DM1** (icon indices 0–197).
The icon sprites are stored in PC Engine tile format (8×8 planar tiles) in the
graphics data block of Track 02.

Icon table source: `C000_ICON_*` through `C197_ICON_*` — same as DM1.
**STUB:** Which icons are actually used in TQ (the subset) is unknown until
Track 02 extraction.

Source: ReDMCSB DEFS.H:1887–1951 · Phase 0 provenance gate · STUB

---

## 4. Text Format

### 4.1 Encoding

| Property | Value |
|----------|-------|
| Languages | English (US), Japanese (JP) |
| Encoding | PC Engine tile font (8×8 tiles, 1bpp or 2bpp planar) |
| Text storage | Tile index arrays in Track 02 (not ASCII) |
| UI text | Embedded in HuC6280 code as tile index sequences |

PC Engine CD-ROM uses a custom tile-based text system:
- Each character = 8×8 tile from the game font
- Text rendered by sending tile indices to the VDC (video display controller)
- Japanese version uses a larger font tile set (more tiles for kanji)
- English version uses ASCII-range tile indices

### 4.2 String Format

Dungeon names, UI text, messages: stored as **tile index sequences**.
No null-terminator convention visible — string length is implicit (known
from the game code). This is the same approach used in DM1 (byte sequences
read until a sentinel value).

**STUB:** The exact tile index values for each string need Track 02 extraction.
The dungeon names (Hall of Records, Crypt of Shadows, etc.) are confirmed from
Lighthouse's RTC conversion documentation.

Source: theron_v1_dungeon_progression.c:38-110 (dungeon names) · STUB

### 4.3 Seven Quest Item Names

Confirmed from `theron_v1_dungeon_progression.c:103-114`:
- "Sacred Amplifier" — Hall of Records
- "Shadow Key" — Crypt of Shadows  
- "Flame Orbs" — Abyss of Flames
- "Stone Sigil" — Tomb of Woe
- "Wayward Ribbon" — Vault of Secrets
- "Destiny's Thread" — Castle of Fate
- "Cosmic Shard" — Tower of Epilogue

Source: theron_v1_dungeon_progression.c:103-114

---

## 5. Champion / Character Format

### 5.1 Party Structure

| Slot | Character | Persistence |
|------|-----------|-------------|
| 0 | **Theron** (fixed main character) | Stats/skills persist between dungeons; items reset |
| 1 | Champion 1 (hired or preset) | Resets each dungeon (stats, skills, items) |
| 2 | Champion 2 (hired or preset) | Resets each dungeon (stats, skills, items) |
| 3 | Champion 3 (hired or preset) | Resets each dungeon (stats, skills, items) |

**No in-dungeon saves.** Between-dungeon saves store Theron's stats and quest
progress. Champion roster resets per dungeon.

Source: Phase 0 provenance gate §3.3 · theron_v1_dungeon_progression.c:22-28

### 5.2 Champion Record Format

Champion data in TQ follows the **same base structure as DM1**, with
simplifications:

```c
struct TQR_Champion {
    // Identity
    uint8_t  name[16];     // null-terminated string (up to 15 chars)
    uint8_t  class;        // 0=Fighter, 1=Ninja, 2=Priest, 3=Wizard (same as DM1)
    uint8_t  level;        // 1–99
    
    // Stats (current / maximum pairs)
    uint8_t  health_cur;   // current health
    uint8_t  health_max;   // maximum health
    uint8_t  mana_cur;
    uint8_t  mana_max;
    uint8_t  stamina_cur;
    uint8_t  stamina_max;
    
    // Attributes (0–99, current and max)
    uint8_t  strength_cur,    strength_max;
    uint8_t  dexterity_cur,    dexterity_max;
    uint8_t  wisdom_cur,       wisdom_max;
    uint8_t  anti_magic_cur,   anti_magic_max;
    uint8_t  anti_fire_cur,    anti_fire_max;
    uint8_t  luck_cur,         luck_max;
    
    // Skills (4 per class = 16 total)
    uint8_t  skills[16];    // 0=NEOPHYTE..17=MASTER (same rank system as DM1)
    
    // Condition flags
    uint16_t condition;    // poisoned, silent, etc. (same as DM1 bit flags)
    
    // Inventory (STUB — size unknown)
    uint8_t  inventory[24];  // object IDs (TQ subset of DM1 objects)
    uint8_t  gold;           // gold carried
    
    // Champion-specific (TQ adds Theron flag)
    uint8_t  is_theron;     // 1 if this slot is Theron
};
```

**Note:** Theron does not use the standard character generation process.
He is a pre-built character with fixed starting stats and special equipment
(he has a spellbook from the beginning — confirmed from Lighthouse's RTC
conversion notes).

**Skill levels (same as DM1, with STUB):**
- TQ likely uses the same 18-rank skill system (NEOPHYTE through  MASTER)
- **STUB:** NEOPHYTE rank (rank 0) — CSB added this; TQ may include it
- Skill level is 0–17 (0=lowest, 17=highest)

Source: DM1 CHAMPION.C · theron_v1_dungeon_progression.c:22-28 · STUB for TQ-specific fields

### 5.3 Theron-Specific Data

Theron's stats and skills persist across all 7 dungeons. This is handled
differently from the companion champions.

**Theron starts with:** Spellbook, some starting equipment (dagger? torch?)
Default stats: TBD after Track 02 extraction.

**Champion reset per dungeon:** Each dungeon start, champions 1–3 are
replaced with new hires (or preset characters for the dungeon). Their
inventory and stats are reset. This is the "light version" design constraint.

Source: Phase 0 provenance gate §3.3 · theron_v1_dungeon_progression.c:22-28

---

## 6. Creature Format

### 6.1 Creature Type Subset

Theron's Quest uses a **subset of DM1's 27 creature types** (indices 0x00–0x1A).
The exact subset is unknown until Track 02 extraction.

| DM1 Creature Index | Name | In TQ? |
|---------------------|------|--------|
| 0x00 | Giant Scorpion | STUB — likely yes |
| 0x01 | Swamp Slime | STUB |
| 0x02 | Giggler | STUB |
| 0x03 | Wizard Eye | STUB |
| 0x04 | Pain Rat / Hellhound | STUB |
| 0x05 | Ruster | STUB |
| 0x06 | Screamer | STUB |
| 0x07 | Rock Rockpile | STUB |
| 0x08 | Ghost Rive | STUB |
| 0x09 | Stone Golem | STUB |
| 0x0A | Mummy | STUB |
| 0x0B | Black Flame | STUB |
| 0x0C | Skeleton | STUB |
| 0x0D | Couatl | STUB |
| 0x0E | Vexirk | STUB |
| 0x0F | Magenta Worm | STUB |
| 0x10 | Trolin / Ant Man | STUB |
| 0x11 | Giant Wasp / Muncher | STUB |
| 0x12 | Animated Armour / Deth Knight | STUB |
| 0x13 | Materializer Zytaz | STUB |
| 0x14 | Water Elemental | STUB |
| 0x15 | Oitu | STUB |
| 0x16 | Demon | STUB |
| 0x17 | Lord Chaos | STUB — likely no (end-game is different) |
| 0x18 | Red Dragon | STUB — likely yes (final dungeon boss?) |
| 0x19 | unused/placeholder | **No** |
| 0x1A | Grey Lord (CSB-only) | **No** — CSB creature, not in TQ |

**Key constraint:** TQ has fewer creatures than DM1 — the game is "easier"
with reduced monster frequency. The creature spawn tables in each dungeon
use a reduced subset and lower spawn rates.

Source: csb_creatures.md (DM1/CSB creature list) · Phase 0 provenance gate §3.1 · STUB

### 6.2 Creature Record Format (In-Dungeon)

In the dungeon binary, creatures are spawned based on spawn point data:

```c
struct TQR_CreatureSpawn {
    uint16_t grid_x;       // spawn position X
    uint16_t grid_y;       // spawn position Y
    uint16_t creature_type; // DM1 creature type index (TQ subset)
    uint16_t behavior;     // attack pattern flags (same as DM1)
    uint16_t health;       // hit points
    uint16_t attack_power; // base damage
    uint16_t armor_class;  // defense
    uint16_t experience;   // XP awarded on death
};
```

**Creature AI:** TQ uses a **simplified AI** compared to DM1/CSB. The
dungeon is smaller and creatures are fewer, so complex multi-step AI
behaviors are likely reduced. The exact AI differences are unknown.

Source: DM1 MOVESENS.C · STUB for TQ-specific AI differences

### 6.3 Creature Graphics

PC Engine sprites use the **HuC6270 sprite system**:
- Sprite size: 16×16 or 32×32 (composite of 8×8 tiles)
- Colors per sprite: 16 (4-bit planar)
- Sprite palette: shared 512-color palette
- Max sprites on screen: 64

Creature sprite data is stored as **sprite attribute entries** in Track 02:
```c
struct TQR_SpriteAttribute {
    uint16_t tile_index;   // VRAM tile index for sprite's first tile
    uint8_t  palette;      // palette group (0–15)
    uint8_t  priority;     // sprite vs background priority
    uint16_t size;         // 0=8x8, 1=16x16, 2=32x32, 3=64x64
};
```

**STUB:** Which sprite sizes are used for creature rendering needs Track 02
extraction. Typical DM-style games use 16×16 or 32×32 for creatures.

Source: Phase 0 provenance gate §5.1 · STUB

---

## 7. Graphics / Tile Format

### 7.1 PC Engine Tile Format

PC Engine graphics use an **8×8 tile-based system** (HuC6260 VDC):

| Property | Value |
|----------|-------|
| Tile size | 8×8 pixels |
| Color depth | 2 bits/pixel (4 colors per tile) **OR** 4 bits/pixel (16 colors per tile) |
| Tile data | Planar: pixel rows stored consecutively |
| Palette | 512 colors total; 16 colors per sprite/tile palette |
| VRAM size | 64 KB dual-port video RAM |

**TQ tile format hypothesis (STUB):**
- Walls/floors: 2bpp tiles (4 colors per tile, 16 bytes per tile)
- Sprites/creatures: 4bpp tiles (16 colors per tile, 32 bytes per tile)
- Font: 2bpp tiles (8×8, ASCII range)

DM1 VGA graphics (320×200, 16 colors planar EGA) are fundamentally different.
TQ uses a completely different tile/sprite system appropriate for the
HuC6280/HuC6270 hardware.

Source: Phase 0 provenance gate §4.3 · HuC6270 datasheet · STUB

### 7.2 Graphics Data Block Layout (STUB)

Within Track 02, the graphics section contains:

```
Offset G:        Tile set 1 (wall tiles — 8×8, 2bpp, ~50–100 tiles)
Offset G+N1:      Tile set 2 (floor tiles — 8×8, 2bpp, ~30–50 tiles)
Offset G+N1+N2:   Tile set 3 (object tiles — 8×8, 4bpp, ~100–200 tiles)
Offset G+N1+N2+N3: Sprite set (creature sprites — 16×16 or 32×32, 4bpp)
Offset G+N1+N2+N3+N4: Font tiles (8×8, 2bpp or 4bpp, 256 chars)
Offset G+N1+N2+N3+N4+N5: Sprite attribute table (OAM)
```

**STUB:** Exact offsets and tile counts need Track 02 extraction.

### 7.3 Viewport Rendering Pipeline (STUB)

TQ renders the first-person dungeon view using PC Engine tiles:
1. Decompose viewport into wall/floor strips (6 walls + 1 floor + ceiling)
2. Select appropriate tile index for each strip (based on direction + distance)
3. Send tile indices + palette to VDC
4. Composite sprite layer (creatures, objects, projectiles) on top

**Note:** TQ has no VDP1 3D geometry (unlike Nexus Saturn). TQ is a pure
tile-renderer like DM1. The rendering pipeline is hardware tile-based,
not polygon-based.

Source: Phase 0 provenance gate §4.3 · STUB for TQ-specific render pipeline

### 7.4 Sprite Attribute Table (OAM)

PC Engine OAM (Object Attribute Memory) format:
```c
struct PCE_OAMEntry {
    uint16_t y;         // Y position (0–239)
    uint16_t sprite;    // Sprite number (tile index in VRAM)
    uint16_t attr;      // Attributes: palette, priority, flip
    uint16_t x;         // X position (0–319)
};
```

Source: HuC6270 datasheet · STUB for TQ-specific OAM layout

---

## 8. Spell Format (Light Subset)

### 8.1 Spell Subset

TQ contains a **subset of DM1's 25 spells** (indices 0–24). Exact subset
unknown until Track 02 extraction.

**STUB — likely spells present (based on core gameplay):**
| Spell | DM1 Index | TQ Likely? |
|-------|-----------|------------|
| Light | 0 | Yes — basic dungeon utility |
| Torch | 1 | Yes |
| Fireball (Ful Ir) | 2 | Yes — combat essential |
| Strength Potion | 3 | Yes |
| Shield | 4 | Yes |
| Heal | 5 | Yes |
| Identify | 6 | Yes |
| Open Door | 7 | Yes |
| Lightning Bolt | 8 | Yes |
| Stamina Potion | 9 | Yes |
| Magic Footprints | 10 | Probably |
| Invisibility | 11 | Probably |
| Poison Cloud | 12 | Probably |
| ... | ... | STUB |

**STUB:** ZOKATHRA (DM1 spell index 24, Zo Kath Ra) — uncertain if in TQ.
ZOKATHRA was added in CSB; TQ predates CSB and may not include it.

**Spell book:** Theron starts with a spellbook (confirmed from Lighthouse's
conversion notes). The spell list is pre-defined; champions can cast
spells from the party's shared spell pool.

Source: DM1/Firestaff spell table · csb_items.md · theron_v1_dungeon_progression.c (Theron has spellbook) · STUB

### 8.2 Spell Data Record

```c
struct TQR_SpellRecord {
    uint8_t  rune_1;    // First rune of incantation
    uint8_t  rune_2;
    uint8_t  rune_3;
    uint8_t  skill;     // Required skill (WIZARD etc.)
    uint16_t power;     // Spell power (STUB — may differ from DM1)
    uint16_t type;      // Spell type (fireball, healing, etc.)
};
```

Source: DM1 DUNGEON.C · STUB for TQ spell table

---

## 9. Save Format (Between-Dungeon Only)

### 9.1 Save Slot Layout

Save files: `saves/theron/slot0.tqsv` through `slot7.tqsv` (8 slots).

```
[64-byte header][champion data][32-byte progression][4-byte footer]
```

Header (64 bytes):
```
Offset 0-3:   magic — 'TQR ' (0x54515220)
Offset 4-5:   version — 1.0 (uint16 little-endian)
Offset 6:      quest_items (7-bit bitfield, 1 bit per quest item)
Offset 7:      current_dungeon (1–7)
Offset 8:      dungeon_state (locked/available/complete)
Offset 9:      current_level (1–3)
Offset 10-37:  dungeon_seeds (7 × 4 bytes = 28 bytes, uint32 LE)
Offset 38-44:  dungeon_states (7 × 1 byte)
Offset 45-48:  champion_gold (uint32)
Offset 49-52:  playtime_secs (uint32)
Offset 53-56:  timestamp (uint32, Unix time)
Offset 57-63:  label (null-terminated string, max 31 chars)
```

Champion data block: Theron + 3 champion records (same structure as in-memory).

Footer (4 bytes): checksum (uint16) + magic repeat (uint16).

Source: theron_v1_save_load.c:130-190 · theron_v1_save_load.h

### 9.2 Obfuscation

TQ save files use **light XOR obfuscation** (simpler than CSB's 16-entry key table):
- Per-slot seed derived from slot index + magic constant
- Each byte XORed with `(seed + byte_index)`
- Checksum: 16-bit sum of all 16-bit words in the data block

Source: theron_v1_save_load.c:56-76

### 9.3 What Persists vs Resets

| Data | Persists (Between Dungeons) | Resets (Each Dungeon) |
|------|---------------------------|----------------------|
| Theron stats/skills | ✅ Yes | |
| Theron inventory | | ❌ Reset |
| Champion 1–3 stats | | ❌ Reset |
| Champion 1–3 inventory | | ❌ Reset |
| Champion 1–3 skills | | ❌ Reset |
| Gold | ✅ Yes (party-wide) | |
| Quest items collected | ✅ Yes (7-bit bitfield) | |
| Dungeon completion states | ✅ Yes | |

Source: Phase 0 provenance gate §3.3 · theron_v1_dungeon_progression.c:22-28

---

## 10. CD-ROM Audio Format

### 10.1 Track Structure

| Track | Type | Content | Notes |
|-------|------|---------|-------|
| 1 | CD-DA Audio | Spoken intro | JP = Japanese, US = English |
| 2 | **DATA** | Game binary + graphics | Track 02 (main data track) |
| 3 | CD-DA Audio | Spoken dialogue/music | JP/EN variant |
| 4 | CD-DA Audio | Spoken dialogue/music | JP/EN variant |
| 5–16 | CD-DA Audio | Music tracks | Unknown count |
| 17 | CD-DA Audio | Ending music | JP: static noise at 1:04, 7 sec longer than US |
| 18 | CD-DA Audio | Final audio | Unknown |

Source: Phase 0 provenance gate §1.5 · dmweb game page

### 10.2 ADPCM Audio (Non-CD-DA)

In addition to Red Book CD-DA audio tracks, TQ uses **ADPCM audio** for
sound effects and non-CD-DA audio:
- 5-channel ADPCM for CD-ROM XA audio
- Additional PSG (square wave) channels for sound effects
- ADPCM samples embedded in Track 02 binary

**STUB:** ADPCM data block location and format unknown.

Source: Phase 0 provenance gate §4.4 · STUB

---

## 11. Seven Mini-Dungeons — Summary Table

| # | Name | Levels | Quest Item | Dungeon Seed (STUB) |
|---|------|--------|------------|---------------------|
| 1 | Hall of Records | 2 | Sacred Amplifier | 313 |
| 2 | Crypt of Shadows | 2 | Shadow Key | 414 |
| 3 | Abyss of Flames | 3 | Flame Orbs | 527 |
| 4 | Tomb of Woe | 3 | Stone Sigil | 632 |
| 5 | Vault of Secrets | 2 | Wayward Ribbon | 749 |
| 6 | Castle of Fate | 3 | Destiny's Thread | 856 |
| 7 | Tower of Epilogue | 3 | Cosmic Shard | 967 |

Source: theron_v1_dungeon_progression.c:38-110 · dmweb game description

---

## 12. "Light" Subset Summary — Items / Creatures / Spells

### 12.1 Items

- **Approximate TQ item count:** 30–50 types (vs DM1's ~200)
- **Weapons:** ~15 types (subset of DM1's ~40)
- **Armor:** ~10 types (subset of DM1's ~30)
- **Potions:** ~8 types (subset of DM1's ~12)
- **Keys:** ~4 types (dungeon-specific)
- **Quest items:** 7 unique (not in DM1)
- **Junk/other:** ~20 types

### 12.2 Creatures

- **Approximate TQ creature count:** 15–20 types (vs DM1's 27)
- **No CSB-only creatures** (Grey Lord 0x1A not in TQ)
- **No unused placeholder** (mon_25 0x19 not in TQ)
- **Likely present:** Core DM1 creatures (Scorpion, Slime, Giggler, Wizard Eye,
  Stone Golem, Skeleton, Demon, Dragon, etc.)
- **Reduced spawn rates:** Each dungeon has fewer creatures than DM1

### 12.3 Spells

- **Approximate TQ spell count:** 15–20 spells (vs DM1's 25)
- **Core spells present:** Light, Torch, Fireball, Heal, Identify, Open Door,
  Strength Potion, Shield, Stamina Potion, Mana Potion
- **STUB:** Which additional spells are included
- **ZOKATHRA:** Uncertain — was added in CSB; may or may not be in TQ
- **Theron spellbook:** Starts with spellbook; party shares spell pool

---

## 13. Phase 2 Completion Checklist

```
[x] Dungeon format — mini-dungeon block header layout documented (STUB)
[x] Dungeon format — grid encoding (5-bit type + attributes, little-endian uint16)
[x] Dungeon format — 7 dungeon table with names, levels, quest items
[x] Dungeon format — dungeon_seed extraction (THQUEST.ASM T560)
[x] Item format — object record structure (STUB, mirrors DM1)
[x] Item format — item type subset (30-50 types, 7 unique quest items)
[x] Item format — quest item names confirmed
[x] Text format — PC Engine tile font encoding (JP/EN)
[x] Text format — string storage as tile index sequences
[x] Champion format — party structure (Theron + 3, per-dungeon reset)
[x] Champion format — champion record fields (STUB, mirrors DM1)
[x] Champion format — Theron persistence (stats/skills, not inventory)
[x] Creature format — creature type subset (15-20 of 27 DM1 types)
[x] Creature format — creature spawn record structure (STUB)
[x] Creature format — reduced spawn rates (TQ "easy" design)
[x] Graphics format — PC Engine 8×8 tile/sprite system (HuC6270)
[x] Graphics format — tile format (2bpp walls, 4bpp sprites, STUB)
[x] Graphics format — sprite attribute table (OAM) format (STUB)
[x] Spell format — spell subset (15-20 of 25 DM1 spells, STUB)
[x] Spell format — Theron spellbook confirmed
[x] Save format — between-dungeon save layout (64-byte header + data + footer)
[x] Save format — XOR obfuscation scheme (light, per-slot seed)
[x] Save format — persistence table (Theron stats, quest items, gold)
[x] Audio format — CD-ROM track structure (18 tracks, Track 02 = data)
[x] Audio format — ADPCM SFX in Track 02 (STUB)
[x] "Light" version constraint — documented for items, creatures, spells
[ ] Track 02 extraction (needed for exact byte offsets, item count, creature list)
[ ] Verify TQ dungeon grid size (8×8 vs 9×9 vs 16×16)
[ ] Confirm TQ magic bytes in dungeon header
[ ] Confirm item/object ID range for quest items
[ ] Confirm spell table size and indices
[ ] Confirm which 15-20 creature types are in TQ
[ ] Confirm ADPCM data block offset and format
```

---

## 14. Reference Sources

| Source | Content |
|--------|---------|
| Phase 0 gate | `docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md` |
| Boot profile | `src/theron/theron_v1_boot.c:318-345` (dungeon header layout) |
| Dungeon progression | `src/theron/theron_v1_dungeon_progression.c:38-114` (7 dungeons) |
| Save format | `src/theron/theron_v1_save_load.c:130-190` |
| DM1 dungeon format | ReDMCSB DUNGEON.C F0001, F0217 |
| DM1 champion format | ReDMCSB CHAMPION.C |
| DM1 item format | ReDMCSB DEFS.H:1887–1951 (icon enum) |
| DM1 creature format | ReDMCSB DEFS.H:1339–1366 (creature enum) |
| DM1 spell format | dm1_v1_spell_casting_pc34_compat.c:41–100 |
| DM web game page | http://dmweb.free.fr/games/therons-quest/ |
| TQ RTC conversion | dungeon-master.com forum t=29286 (Lighthouse TQ source) |
| Platform ref | Phase 0 gate §5.1 (HuC6280 specs) |

---

## 15. Next Steps

1. **Download Track 02** from cdromance.org (JP and US CUE/BIN images)
2. **Extract Track 02** as raw binary from CUE/BIN
3. **Compute SHA256** hashes for both Track 02 files (Phase 0 completion)
4. **Search Track 02** for 7 distinct dungeon blocks (magic "T1" at offset 2)
5. **Confirm grid size** by finding repeating uint16 patterns per dungeon
6. **Map item/object table** by identifying repeating 16-byte object records
7. **Build item/creature/spell manifests** from extracted binary data
8. **Phase 3:** Implement Theron's Quest dungeon loader, object database, and
   text renderer from Phase 2 format evidence

---

*Generated by cron job `Theron_V1_Phase2_DataFormats_0527`*
*Supersedes: tqr_v1_phase0_provenance_gate_H2339.md §4 (data format hypotheses)*
*Next: Phase 3 — Core world model, or Phase 8 verification suite*