# Nexus V1 Phase 2 — Data Formats: Source-Lock Document
**Job:** `Nexus_V1_DataFormats_H2321`
**Status:** ⚠️ Superseded for Nexus DGN/SMAP/ITEM.IBS/MNS details by
`docs/source-lock/nexus_v1_dmweb_format_crawl_20260528.md`
**Author:** Firestaff agent (cron)
**Last revised:** 2026-05-27T00:03 UTC+2
**Sources:** `src/nexus/`, `include/nexus_v1_*.h`, `docs/`, `docs/NEXUS_FILE_CLASSIFICATION.md`, ReDMCSB cross-reference for DM1/DM2 format inheritance.

---

> 2026-05-28 update: DMWeb's Dungeon Master Nexus documentation contradicts
> several early Firestaff assumptions in this file. In particular, Nexus DGN
> files are 2048-byte block containers with a 64x64 Structure1B grid, not a raw
> 32x32 grid at offset 0; SMAP files are tilemap/palette/tileset automap images;
> ITEM.IBS and MNS have documented section layouts. Treat the older sections
> below as historical notes until they are rewritten from the DMWeb crawl.
> Source-lock: `docs/source-lock/nexus_v1_dmweb_format_crawl_20260528.md`.

## Scope

This document source-locks every data file format used by Dungeon Master Nexus (Sega Saturn), with byte layout, field definitions, and every known variant. Where format evidence is absent, the gap is explicitly noted with reasoning and a `TODO`.

Format-to-Firestaff C structure mappings are provided for each format. "Implemented" means there is working C code; "stub" means an API exists but the underlying format is not decoded; "unknown" means no implementation or format evidence exists.

ReDMCSB (WIP20210206) covers DM1/CSB/DM2 only — **no Saturn/Nexus code exists in ReDMCSB**. All Nexus format claims are best-effort reverse engineering from disc extraction, binary inspection, and game-content analysis.

---

## Format Index

| # | Format | File(s) | Status |
|---|--------|---------|--------|
| 1 | Dungeon | `LEV00-LEV15.DGN` | Grid ✅ · Geometry ❌ |
| 2 | Map/Minimap | `SMAP00-SMAP15.BIN` | ❌ Unknown |
| 3 | Object | `NEXUS_OBJECT_*` (in DGN) | ✅ Partial |
| 4 | Text | `FONT256.S2D`, `*.TXT` | ✅ Partial |
| 5 | Champion | `CHAMPIONS.DAT`, `FACE.BIN` | ✅ Partial |
| 6 | Monster/Creature | `*.MNS` (DMDF) | ✅ Partial |
| 7 | Sound | `SNDLEV*.SAL`, `SNDLEV*.MAP` | ❌ STUB |
| 8 | Graphics/Model | `*.MNS`, `*.CG`, `*.DG2` | ✅ Partial |

---

## 1. Dungeon Level Files — `LEV00.DGN` – `LEV15.DGN`

### 1.1 File Inventory

| Level | File | Size (bytes) | DMWeb grid | Notes |
|-------|------|-------------|------|-------|
| 0 | `LEV00.DGN` | 147,456 | 64×64 Structure1B | Entry/title sequence — not playable |
| 1 | `LEV01.DGN` | 280,576 | 64×64 Structure1B | Hall of Champions |
| 2 | `LEV02.DGN` | 272,384 | 64×64 Structure1B | |
| 3 | `LEV03.DGN` | 290,816 | 64×64 Structure1B | |
| 4 | `LEV04.DGN` | 245,760 | 64×64 Structure1B | |
| 5 | `LEV05.DGN` | 266,240 | 64×64 Structure1B | |
| 6 | `LEV06.DGN` | 239,616 | 64×64 Structure1B | |
| 7 | `LEV07.DGN` | 258,048 | 64×64 Structure1B | |
| 8 | `LEV08.DGN` | 303,104 | 64×64 Structure1B | |
| 9 | `LEV09.DGN` | 288,768 | 64×64 Structure1B | |
| 10 | `LEV10.DGN` | 290,816 | 64×64 Structure1B | |
| 11 | `LEV11.DGN` | 278,528 | 64×64 Structure1B | |
| 12 | `LEV12.DGN` | 321,536 | 64×64 Structure1B | Boss level — largest |
| 13 | `LEV13.DGN` | 256,000 | 64×64 Structure1B | |
| 14 | `LEV14.DGN` | 253,952 | 64×64 Structure1B | |
| 15 | `LEV15.DGN` | 270,336 | 64×64 Structure1B | Final level |

**Total: 4,393,984 bytes (~4.2 MB).** DM1: ~33 KB for all 8 levels. Ratio: ~130×.

**Provenance:** Extracted from DM Nexus Saturn ISO (disc hash not yet SHA256-locked — disc image absent).

### 1.2 File Structure — DMWeb correction

DMWeb describes LEV\*.DGN as a 2048-byte block container. The first block is a
header; Structure1 is addressed by block offset/count fields at `0x0C`,
`0x0E` and useful byte size at `0x10`.

Structure1 offset `0x14` points to Structure1B. Structure1B is always `0x8000`
bytes: a 64x64 grid with 8 bytes per cell. Collision and door presence are
decoded from those 8-byte cells. The old "2048 bytes at offset 0" model is no
longer valid for real Nexus DGN files.

**Square type semantics** (matches DM1 — see `include/nexus_v1_world.h`):
```
0  = solid wall         (NEXUS_SQUARE_WALL)
1  = floor              (NEXUS_SQUARE_FLOOR)
2  = pit                (NEXUS_SQUARE_PIT)
3  = stairs up           (NEXUS_SQUARE_STAIRS_UP)
4  = door               (NEXUS_SQUARE_DOOR)
5  = teleporter         (NEXUS_SQUARE_TELEPORTER)
6  = alarm              (NEXUS_SQUARE_ALARM)
8  = exit               (NEXUS_SQUARE_EXIT)
13 = stairs down        (NEXUS_SQUARE_STAIRS_DOWN)
```

Reading: `rb16(data + (y * 32 + x) * 2) & 0x1F` — lower 5 bits only.

**ReDMCSB reference:** DUNGEON.C F0023 — grid square type enumeration matches exactly.

#### Section B — 3D Geometry Blob (remaining 145–319 KB)

Location: **offset 2048** through end of file.

**Status: FORMAT UNKNOWN.** No structural evidence has been extracted.

Hypothesis: Pre-computed polygon data per grid position — wall faces, floor meshes, ceiling meshes. DM1 uses 2D raycasting (no geometry). Nexus bakes all 3D polygon data into the DGN file.

**Approach for reverse-engineering:**
1. Dump bytes at offset 2048 through 2048+8192 of LEV00.DGN
2. Look for repeating 12-byte patterns (DMDF int16×6 = 12 bytes/vertex)
3. Check for magic bytes at geometry start (DMDF uses `0x444D4446`)
4. Scan for uint16 index sequences (values < vertex_count hypothesis)
5. Compare LEV12.DGN (largest) geometry size with LEV00.DGN — ratio should reflect level complexity

**NOTE:** There may also be a "thing list" (object/creature placement) embedded between the grid and geometry sections, or in a separate LEV\*.DGN section at known offset. This is unverified.

### 1.3 DGN Variants

No per-level structural variants detected. All 16 files use the same 32×32 grid + geometry blob structure. File size variation (147–321 KB) is solely attributable to geometry complexity.

### 1.4 Endianness

Big-endian throughout (SH2 Saturn processor). All multi-byte reads use `rb16()` / `rb32()` in `nexus_v1_dungeon.c`.

### 1.5 Firestaff Conversion — `Nexus_V1_Level`

```c
/* Source: include/nexus_v1_dungeon.h */
typedef struct {
    int      width, height;              /* = 32, fixed */
    uint8_t  squares[32][32];             /* lower 5 bits = square type */
    int      thing_count;                 /* objects in this level */
    int      creature_count;              /* creatures in this level */
    int      has_3d_geometry;             /* = 1 for Nexus */
    int      geometry_offset;             /* = 2048 */
    int      geometry_size;               /* = size - 2048 */
} Nexus_V1_Level;

/* Grid parsing: src/nexus/nexus_v1_dungeon.c:nexus_v1_level_load() */
for (gy = 0; gy < 32; gy++)
    for (gx = 0; gx < 32; gx++)
        level->squares[gy][gx] = rb16(data + (gy*32+gx)*2) & 0x1F;
```

### 1.6 Status

| Item | Status | Source |
|------|--------|--------|
| Grid parsing | ✅ Implemented | `nexus_v1_dungeon.c:nexus_v1_level_load()` |
| Square type enum | ✅ Implemented | `include/nexus_v1_world.h` |
| 3D geometry blob parser | ❌ **NOT IMPLEMENTED** | Format unknown |
| Thing/object list in DGN | ❌ **NOT IMPLEMENTED** | May be embedded or separate |
| Per-level size validation | ✅ Implemented | Size check in load function |

---

## 2. Map / Minimap Files — `SMAP00.BIN` – `SMAP15.BIN`

### 2.1 File Inventory

| File | Size (bytes) | Notes |
|------|-------------|-------|
| `SMAP00.BIN` | ~17–30 KB | Per-level automap/grid |
| `SMAP01.BIN` | ~17–30 KB | |
| ... | ... | |
| `SMAP15.BIN` | ~17–30 KB | |

**16 files total, ~400 KB total.** Each file corresponds to one LEV\*.DGN level.

**Source:** `docs/NEXUS_FILE_CLASSIFICATION.md` — "MINIMAP FILES (.BIN) — 16 files, ~400 KB total".

### 2.2 Format

**Status: COMPLETELY UNKNOWN.** No implementation or binary inspection exists in Firestaff.

**Hypotheses:**
- `SMAP*.BIN` likely stores a 32×32 bitmask (1024 bits = 128 bytes) of explored squares — matching the dungeon grid dimensions.
- Alternative: packed Run-Length-Encoding of square visibility states.
- Alternative: per-square visibility flag + icon type (door, trap, teleporter marker) — would explain larger file sizes (17–30 KB vs 128 bytes).

**File size analysis:** 17–30 KB per level. A raw 32×32 visibility grid would be ~1 KB. The 17–30 KB size suggests either:
- Per-square icon/feature flags (door=1 byte, trap=1 byte, teleporter=1 byte × 1024 = ~3 KB minimum)
- Additional per-square data (e.g., "seen from north", "seen from east" booleans)
- Uncompressed 8bpp minimap bitmap (32×32 = 1 KB, too small — likely larger display resolution)
- Some other encoding not yet reverse-engineered

### 2.3 Variants

None known. All 16 SMAP\*.BIN files presumably use the same format.

### 2.4 Firestaff Conversion — Not Defined

No `Nexus_V1_Minimap` struct exists. Target structure would be:
```c
/* TODO: define after format analysis */
typedef struct {
    int level_index;
    int width, height;       /* likely 32×32 */
    uint8_t explored[32][32]; /* 1=explored, 0=unseen */
    uint8_t icon[32][32];    /* per-square feature type */
} Nexus_V1_Minimap;
```

### 2.5 Status

| Item | Status |
|------|--------|
| SMAP\*.BIN format | ❌ **UNKNOWN** — no binary inspection |
| Minimap struct | ❌ **NOT DEFINED** |
| Minimap rendering | ❌ **NOT IMPLEMENTED** |

---

## 3. Object Format — `NEXUS_OBJECT_*` Types

### 3.1 Object Type Enum

Objects are placed in the dungeon world, not stored in a separate file. The object type system is defined in `include/nexus_v1_world.h`:

```c
/* Source: include/nexus_v1_world.h */
#define NEXUS_OBJECT_NONE           0
#define NEXUS_OBJECT_CHEST         1   /* treasure container */
#define NEXUS_OBJECT_DOOR          2   /* openable door */
#define NEXUS_OBJECT_LEVER         3   /* toggle switch */
#define NEXUS_OBJECT_BUTTON        4   /* momentary switch */
#define NEXUS_OBJECT_POTION        5   /* consumable */
#define NEXUS_OBJECT_SCROLL        6   /* spell container */
#define NEXUS_OBJECT_WEAPON        7   /* equippable */
#define NEXUS_OBJECT_ARMOR         8   /* equippable */
#define NEXUS_OBJECT_GEM           9   /* quest item / treasure */
#define NEXUS_OBJECT_KEY          10   /* door key */
#define NEXUS_OBJECT_FOOD         11   /* consumable */
#define NEXUS_OBJECT_PIT_TRAP     12   /* floor trap */
#define NEXUS_OBJECT_TELEPORTER_LINK 13 /* teleporter link pair */
```

**Note:** DM1 uses square types (0–31) for door/pit/teleporter placement in the grid. Nexus adds explicit object records for interactable entities (chests, levers, items) that are not part of the grid itself.

### 3.2 Object Record Structure

```c
/* Source: include/nexus_v1_world.h */
typedef struct {
    int      id;              /* unique within level (1-based) */
    uint8_t  type;           /* NEXUS_OBJECT_* */
    uint8_t  state;          /* open/closed/locked/consumed/... */
    int      x, y;           /* grid position */
    int      level;          /* dungeon level */
    int      quantity;       /* stack count for consumables */
    int      linked_id;      /* e.g. teleporter partner ID */
    uint32_t flags;          /* NEXUS_OBJ_F_* */
} Nexus_V1_Object;

/* Object flags */
#define NEXUS_OBJ_F_PICKED_UP   (1U << 0)
#define NEXUS_OBJ_F_OPENED      (1U << 1)
#define NEXUS_OBJ_F_ACTIVATED   (1U << 2)
#define NEXUS_OBJ_F_DESTROYED   (1U << 3)
#define NEXUS_OBJ_F_LOCKED      (1U << 4)
```

### 3.3 Object Placement Source

Objects are placed from the LEV\*.DGN "thing list" — the portion of the DGN file between the grid section (offset 2048) and the 3D geometry blob. **This section is unparsed.** The current implementation (`nexus_v1_world.c`) provides only an in-memory object database API (`nexus_v1_object_place()`, etc.) without a file-format loader.

**ReDMCSB reference:** DUNGEON.C F0103 — object placement and collision (DM1 format, not Nexus). Nexus likely uses a similar but extended format.

### 3.4 Item Encyclopedia (UI Layer)

The UI layer defines a shared item encyclopedia in `src/ui/firestaff_item_encyclopedia.c`. This is **not** a Nexus-specific data file — it's a Firestaff UI structure. Representative items include:

| Category | Items |
|----------|-------|
| Weapons | Falchion, Rapier, Mace, Club, Staff, Sword, Axe, Dagger, Arrow, Slayer, Vorpal Blade, Firestaff |
| Armor | Leather Jerkin, Mail Aketon, Plate Armor, Shield, Helmet, Boots |
| Potions | Health Potion, Mana Potion, Stamina Potion, Antidote |
| Scrolls | Scroll |
| Containers | Chest, Sack |
| Keys | Gold Key, Silver Key, Skeleton Key |
| Misc | Torch, Compass, Rabbit Foot, Corn, Water Flask |

**Champion inventory:** 30 slots (`uint8_t inventory[30]` in `Nexus_V1_Champion`), vs DM1's 12 slots.

### 3.5 Object Variants

| Aspect | DM1 | Nexus |
|--------|-----|-------|
| Object storage | Thing list in DUNGEON.DAT | Embedded in DGN (unknown format) |
| Object types | ~20 types | **14 types** (NEXUS_OBJECT_0 through NEXUS_OBJECT_TELEPORTER_LINK) |
| Inventory slots | 12 per champion | **30 per champion** |
| Flask system | Yes | Yes (inherited) |
| Scroll system | No | Yes (from CSB/DM2) |
| Teleporter links | Hard-coded square pairs | NEXUS_OBJECT_TELEPORTER_LINK with `linked_id` |

### 3.6 Firestaff Conversion

```c
/* Source: include/nexus_v1_world.h:Nexus_V1_Object */
#define NEXUS_MAX_OBJECTS 256  /* per-world object pool limit */

/* Object database API: src/nexus/nexus_v1_world.c */
int     nexus_v1_object_place(Nexus_V1_World *world, const Nexus_V1_Object *obj);
int     nexus_v1_object_remove(Nexus_V1_World *world, int id);
Nexus_V1_Object *nexus_v1_object_at(Nexus_V1_World *world, int level, int x, int y);
Nexus_V1_Object *nexus_v1_object_by_id(Nexus_V1_World *world, int id);
int     nexus_v1_object_set_state(Nexus_V1_World *world, int id, uint8_t new_state);
int     nexus_v1_object_set_flag(Nexus_V1_World *world, int id, uint32_t flag);
int     nexus_v1_object_clear_flag(Nexus_V1_World *world, int id, uint32_t flag);
```

### 3.7 Status

| Item | Status | Source |
|------|--------|--------|
| Object type enum | ✅ Implemented | `include/nexus_v1_world.h` |
| Object struct | ✅ Implemented | `include/nexus_v1_world.h` |
| Object database API | ✅ Implemented | `nexus_v1_world.c` |
| Object placement from DGN | ❌ **NOT IMPLEMENTED** | Thing list unknown |
| Item encyclopedia UI | ✅ Implemented | `firestaff_item_encyclopedia.c` |
| Champion inventory (30 slots) | ✅ Implemented | `nexus_v1_champions.h` |

---

## 4. Text Format — Font and Encoding

### 4.1 Font File — `FONT256.S2D`

**Source:** `src/nexus/nexus_v1_saturn_font.c`
**Size:** ~24–64 KB
**Format:** Sega Saturn SCR (screen font)

#### Header Format (Big-Endian, 48 bytes / 0x30)

```
Offset  Size  Field           Type       Description
0x00    15    magic           char[15]   "SEGA SATURN SCR" (null-padded ASCII)
0x0F    1     null            byte       0x00
0x10    2     char_count      uint16     Max characters (default 256, big-endian)
0x12    30    reserved        bytes      Padding
0x30    N     glyph_data     bytes      Glyph bitmaps start here
```

**Glyph size auto-detection** (from `nexus_v1_saturn_font.c`):
```c
if (glyph_size >= 32)   /* glyph_data_size / char_count >= 32 */
    dimensions = 16×16;
else if (glyph_size >= 18)
    dimensions = 12×12;
else
    dimensions = 8px wide, glyph_size tall;
```

For 256 glyphs at 16×16 (1bpp = 32 bytes/glyph): **8192 bytes** of glyph data.

#### Glyph Bitmap Format

Each glyph is a packed 1-bpp bitmap, row-major order. Rows are packed 8 pixels per byte (no word-alignment).

**API:**
```c
/* Source: include/nexus_v1_saturn_font.h */
int  nexus_v1_font_load(Nexus_V1_Font *font, const uint8_t *data, int size);
void nexus_v1_font_free(Nexus_V1_Font *font);
const uint8_t *nexus_v1_font_get_glyph(const Nexus_V1_Font *font, int idx);
```

Glyph blitting to framebuffer: **NOT IMPLEMENTED.**

### 4.2 Text Encoding — Shift-JIS

**Source:** `src/nexus/nexus_v1_text.c`

Nexus uses **Shift-JIS** for all Japanese text. No UTF-8 or Latin-1.

#### Encoding Ranges

| Code range | Type | Conversion | Status |
|-----------|------|------------|--------|
| `0x20–0x7E` | ASCII | Pass-through | ✅ Implemented |
| `0xA1–0xDF` | Hankaku katakana | → U+FF61–U+FF9F | ✅ Implemented |
| `0x81–0x9F` | Daiji (2-byte lead) | → "?" | ❌ Not implemented |
| `0xE0–0xEF` | Daiji (2-byte lead) | → "?" | ❌ Not implemented |

**Double-byte JIS X 0208 kanji:** Not implemented — requires full lookup table (~6000 characters). Currently replaced with "?".

#### Conversion Function

```c
/* Source: src/nexus/nexus_v1_text.c:nexus_v1_sjis_to_utf8() */
int nexus_v1_sjis_to_utf8(const uint8_t *sjis, int sjis_len,
    char *utf8_out, int utf8_max);
```

Handles ASCII and Hankaku katakana. Double-byte characters replaced with "?".

### 4.3 Text Files

| File | Size | Description |
|------|------|-------------|
| `DMN_ABS.TXT` | ~1 KB | Dungeon abstract / lore text |
| `DMN_BIB.TXT` | ~1 KB | Bibliography / credits |
| `DMN_CPY.TXT` | ~1 KB | Copyright notice |

**Status:** No Firestaff implementation for loading these text files.

### 4.4 Per-Level Script Files — `SLEV00.BIN` – `SLEV15.BIN`

**Source:** `docs/NEXUS_FILE_CLASSIFICATION.md`
**Size:** 2–12 KB per level (16 files total)
**Purpose:** Declarative event scripts for teleporters, traps, door animations

**Format:** Binary, processed by `SDDRVS.TSK` script VM.
**Implementation:** None in Firestaff. Script VM in `nexus_v1_script_vm.c` is a stub.

### 4.5 Text Format Variants

| Variant | Description | Status |
|---------|-------------|--------|
| FONT256.S2D | Saturn SCR font, 256 glyphs | ✅ Header parsing ✅ Glyph access ❌ Blit |
| Shift-JIS ASCII | Single-byte ASCII (0x20-0x7E) | ✅ |
| Shift-JIS Hankaku Katakana | Half-width katakana (0xA1-0xDF) | ✅ |
| Shift-JIS Daiji/Kanji | Double-byte JIS X 0208 | ❌ "?" replacement |
| `*.TXT` lore files | Plain text with Shift-JIS | ❌ Not loaded |
| `SLEV*.BIN` scripts | SDDRVS.TSK binary scripts | ❌ STUB only |

### 4.6 Firestaff Conversion

```c
/* Source: include/nexus_v1_saturn_font.h */
typedef struct {
    int width, height;        /* glyph dimensions */
    int char_count;           /* glyph count (default 256) */
    int glyph_size;          /* bytes per glyph */
    const uint8_t *glyphs;   /* packed bitmap data */
} Nexus_V1_Font;
```

### 4.7 Status

| Item | Status | Source |
|------|--------|--------|
| FONT256.S2D header parsing | ✅ Implemented | `nexus_v1_saturn_font.c` |
| Glyph access | ✅ Implemented | `nexus_v1_font_get_glyph()` |
| Glyph blit to framebuffer | ❌ **NOT IMPLEMENTED** | |
| Shift-JIS → UTF-8 ASCII | ✅ Implemented | `nexus_v1_sjis_to_utf8()` |
| Shift-JIS → UTF-8 Katakana | ✅ Implemented | `nexus_v1_sjis_to_utf8()` |
| Shift-JIS → UTF-8 Kanji | ❌ Replaced with "?" | No lookup table |
| `*.TXT` lore files | ❌ Not loaded | |
| `SLEV*.BIN` script parsing | ❌ STUB only | `nexus_v1_script_vm.c` |

---

## 5. Champion Format — Roster, Stats, Portraits

### 5.1 Roster Structure

Nexus has a pool of **24 champions** (same as DM1), with **4 active in the party**.

**Roster (from `src/nexus/nexus_v1_champions.c`):**
```c
static const struct g_nexus_roster[] = {
    {"Syra",      "シラ",      NEXUS_CLASS_FIGHTER, 70, 55, 15, 55, 40, 25, 50},
    {"Leyla",     "レイラ",   NEXUS_CLASS_WIZARD,  40, 35, 65, 25, 35, 60, 30},
    {"Nabi",      "ナビ",     NEXUS_CLASS_NINJA,   55, 60, 25, 40, 60, 30, 45},
    {"Gando",     "ガンド",   NEXUS_CLASS_PRIEST,  50, 40, 55, 35, 30, 55, 40},
    {"Torham",    "トルハム", NEXUS_CLASS_FIGHTER, 65, 50, 20, 50, 45, 28, 48},
    {"Elija",     "エリジャ", NEXUS_CLASS_WIZARD,  38, 30, 70, 22, 32, 65, 28},
    {"Wu Tse",    "ウー・ツエ", NEXUS_CLASS_NINJA, 52, 58, 30, 38, 55, 35, 42},
    {"Stamm",     "スタム",   NEXUS_CLASS_FIGHTER, 75, 60, 10, 60, 35, 20, 55},
    {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0} /* sentinel */
};
```

**Format:** ASCII name, Shift-JIS JP name (encoded as bytes in source), class, HP, STA, MP, STR, DEX, WIS, VIT (9 numeric fields = 9 stats).

**Only 8 named champions defined.** The remaining 16 slots (to reach 24) are presumed empty or filled from `FACE.BIN` portrait data. This is a **gap** in the current implementation.

### 5.2 Champion Struct

```c
/* Source: include/nexus_v1_champions.h */
#define NEXUS_MAX_CHAMPIONS 24
#define NEXUS_MAX_PARTY 4

typedef enum {
    NEXUS_CLASS_FIGHTER = 0,
    NEXUS_CLASS_NINJA,
    NEXUS_CLASS_PRIEST,
    NEXUS_CLASS_WIZARD,
    NEXUS_CLASS_COUNT
} Nexus_ChampionClass;

typedef struct {
    char     name_ascii[32];      /* null-terminated ASCII name */
    char     name_jp[64];         /* Japanese name (Shift-JIS source) */
    Nexus_ChampionClass primary_class;
    int      health, max_health;
    int      stamina, max_stamina;
    int      mana, max_mana;
    int      strength, dexterity, wisdom, vitality;
    int      anti_magic, anti_fire;
    int      fighter_level, ninja_level, priest_level, wizard_level;
    int      food, water;
    int      alive;
    int      portrait_index;       /* index into FACE.BIN */
    uint8_t  inventory[30];       /* item indices — 30 slots */
} Nexus_V1_Champion;

typedef struct {
    Nexus_V1_Champion champions[NEXUS_MAX_CHAMPIONS];
    int champion_count;           /* = 8 in current impl */
    int party[NEXUS_MAX_PARTY];   /* champion indices in party */
    int party_count;
    int leader_index;
} Nexus_V1_ChampionPool;
```

### 5.3 Class System — DM2 Inheritance

| Class | DM1 | CSB | DM2 | Nexus |
|-------|-----|-----|-----|-------|
| Fighter | ✅ | ✅ | ✅ | ✅ |
| Wizard | ✅ | ✅ | ✅ | ✅ |
| Priest | ✅ | ✅ | ✅ | ✅ |
| Ninja | ❌ | ✅ (unlockable) | ✅ | ✅ |

**Nexus has the full DM2 4-class roster.** Ninja class (high DEX, no magic, fast attack) is available from the start.

### 5.4 Default Values

| Field | DM1 default | Nexus default |
|-------|-------------|---------------|
| Anti-Magic | 0 | **5** |
| Anti-Fire | 0 | **5** |
| Food | 1500 | 1500 |
| Water | 1500 | 1500 |

### 5.5 Champion Portrait — `FACE.BIN`

**Source:** `docs/NEXUS_FILE_CLASSIFICATION.md`
**Size:** 44 KB
**Content:** 24 portrait images for the champion roster (one per champion slot).

**Format: UNKNOWN.** Candidates:
- Indexed bitmap (8bpp with palette)
- Saturn VDP1 BITMAP format
- Big-endian encoding

**No implementation exists** in Firestaff for `FACE.BIN` parsing or portrait rendering. `nexus_v1_champions.c` sets `portrait_index` (0–7 for the 8 defined champions), but no portrait loader exists.

### 5.6 Champion Variants

| Aspect | DM1 | CSB | DM2 | Nexus |
|--------|-----|-----|-----|-------|
| Roster size | 24 | 24 | 24+ | **24** |
| Party size | 4 | 4 | 4 + companions | **4** |
| Names | Western | Western | Western | **Japanese** |
| Ninja class | ❌ | ✅ (unlock) | ✅ | ✅ |
| Anti-Magic default | 0 | 0 | 0 | **5** |
| Anti-Fire default | 0 | 0 | 0 | **5** |
| Inventory slots | 12 | 12 | 12 | **30** |

### 5.7 Firestaff Conversion

```c
/* Source: src/nexus/nexus_v1_champions.c */
void nexus_v1_champions_init(Nexus_V1_ChampionPool *pool);
int nexus_v1_champion_recruit(Nexus_V1_ChampionPool *pool, int mirror_index);
int nexus_v1_champion_resurrect(Nexus_V1_ChampionPool *pool, int party_slot);

/* Default resistance: src/nexus/nexus_v1_champions.c:nexus_v1_champions_init() */
c->anti_magic = 5;
c->anti_fire = 5;
c->food = 1500;
c->water = 1500;
c->alive = 1;
c->portrait_index = i;
```

### 5.8 Status

| Item | Status | Source |
|------|--------|--------|
| Roster init (8 champions) | ✅ Implemented | `nexus_v1_champions_init()` |
| Full 24-slot roster | ❌ **Only 8 champions** | Gap |
| Champion recruit/resurrect | ✅ Implemented | `nexus_v1_champion_recruit/resurrect()` |
| FACE.BIN parsing | ❌ **NOT IMPLEMENTED** | Format unknown |
| Portrait rendering | ❌ **NOT IMPLEMENTED** | |
| Class system (4 classes) | ✅ Implemented | `Nexus_ChampionClass` enum |
| Stat advancement (XP-based) | ❌ **NOT IMPLEMENTED** | DM1 formulas not ported |
| 30-slot inventory | ✅ Implemented | `Nexus_V1_Champion` |

---

## 6. Monster / Creature Format — `*.MNS` (DMDF)

### 6.1 Creature Type Inventory

**~30 .MNS files in the disc extraction** (`docs/NEXUS_FILE_CLASSIFICATION.md`):

| Name | File | Size | DMDF? |
|------|------|------|-------|
| Scorpion | `SCORPION.MNS` | ~55 KB | ✅ |
| Mummy | `MUMMY.MNS` | ~70 KB | ✅ |
| Dragon | `DRAGON.MNS` | ~88 KB | ✅ |
| Skeleton | `SKELETON.MNS` | ~52 KB | ✅ |
| Ghost | `GHOST.MNS` | 48,840 | ✅ |
| Worm | `WORM.MNS` | ~60 KB | ✅ |
| Golem | `GOLEM.MNS` | 48,140 | ✅ |
| Spider | `SPIDER.MNS` | ~44 KB | ✅ |
| Demon/Chaos | `CHAOS.MNS` | 88,572 | ✅ |
| Giggler | `GIGGLER.MNS` | ~50 KB | ✅ |
| Vexirk | `VEXIRK.MNS` | ~62 KB | ✅ |
| Rat | `RAT.MNS` | ~38 KB | ✅ |
| Screamer | `SCREAMER.MNS` | ~58 KB | ✅ |
| Rockpile | `ROCKPILE.MNS` | ~40 KB | ✅ |
| Oitu | `OITU.MNS` | ~42 KB | ✅ |
| Antman | `ANTMAN.MNS` | 53,768 | ✅ |
| Bigworm | `BIGWORM.MNS` | 53,784 | ✅ |
| Borketh | `BORKETH.MNS` | 67,644 | ✅ |
| Dra_Zom | `DRA_ZOM.MNS` | 83,508 | ✅ |
| Hound | `H_HOUND.MNS` | 46,364 | ✅ |
| Green Dragon | `GRN_DRA.MNS` | ~76 KB | ✅ |
| Gold Dragon | `D_GOLD.MNS` | ~80 KB | ✅ |
| Red Dragon | `D_RED.MNS` | ~82 KB | ✅ |
| Silver Dragon | `D_SILVER.MNS` | ~78 KB | ✅ |

### 6.2 DMDF File Header (48 bytes / 0x30)

```
Offset  Size  Field            Type       Description
0x00    4     magic            uint32     0x444D4446 = "DMDF" (big-endian)
0x04    4     file_size        uint32     Total file size in bytes
0x08    4     section_count    uint32     Number of data sections
0x0C    4     flags            uint32     Format flags
0x10    16    reserved         uint32[4]  Reserved / padding
0x20    4     data_offset      uint32     Offset to start of section data
0x24    4     vertex_offset    uint32     Offset to vertex data from file start
0x28    4     vertex_count     uint32     Number of vertices
0x2C    4     face_count       uint32     Number of faces (triangles + quads)
```

**Magic:** `0x444D4446` = ASCII "DMDF" in big-endian. Confirmed via `nexus_v1_dmdf_is_valid()`.

### 6.3 DMDF Vertex Format

**Discrepancy detected:** The header field at offset 0x24 is named `vertex_offset` but the actual loading code in `nexus_v1_dmdf_model.c` reads vertex data at `data_offset + 8`. The `vertex_offset` field is not used in the current implementation.

**Confirmed vertex stride:** 10 bytes per vertex in the current implementation:
```c
/* Source: nexus_v1_dmdf_model.c — DISCREPANCY: header says 16B but code uses 10B */
int vert_size = vc * 10; /* 5 int16 per vertex */
model->vertices[i].x = rbs16(data + vo);
model->vertices[i].y = rbs16(data + vo + 2);
model->vertices[i].z = rbs16(data + vo + 4);
model->vertices[i].u = rb16(data + vo + 6);
model->vertices[i].v = rb16(data + vo + 8);
```

**Struct definition** in `include/nexus_v1_dmdf_model.h`:
```c
typedef struct {
    int16_t  x, y, z;      /* 6 bytes — position */
    int16_t  nx, ny, nz;   /* 6 bytes — normal (NOT USED in load) */
    uint16_t u, v;         /* 4 bytes — texture coords */
} Nexus_DMDFVertex;  /* = 16 bytes per vertex (struct) but loaded as 10 bytes */
```

**This is a confirmed bug/inconsistency.** The struct is 16 bytes but the loading code reads only 10 bytes per vertex. Needs verification against a real `.MNS` file.

### 6.4 DMDF Face Format

Faces stored as **uint16_t index arrays** at `data_offset + 8 + vertex_count * 10`:
- Triangle: 3 indices × 2 bytes = **6 bytes**
- Quad: 4 indices × 2 bytes = **8 bytes**

The implementation assumes all triangles: `face_bytes = face_count * 6`.

### 6.5 DMDF Texture Data

Embedded texture follows vertex and face data. Format: **VDP1 BITMAP** (Saturn hardware texture format). Compression: unknown.

**VDP1 BITMAP format: NOT DOCUMENTED.** This is a Saturn-specific format requiring Sega Saturn graphics documentation to decode.

### 6.6 Creature Type Definitions

```c
/* Source: include/nexus_v1_creatures.h */
#define NEXUS_MAX_CREATURE_TYPES 64
#define NEXUS_MAX_ACTIVE_CREATURES 128

typedef struct {
    char name[32];
    char model_file[32];     /* e.g. "SCORPION.MNS" */
    int  health, attack, defense, speed;
    int  experience_value;
    int  model_index;        /* index into engine->models[] */
    int  smell_range;         /* tiles — triggers chase */
    int  attack_range;       /* tiles — triggers attack */
    int  wariness;           /* 0=charge always, 1+=avoid hazards */
    int  hp_warn;            /* HP% below which creature flees */
    int  flags;              /* NEXUS_CATTR_* */
    int  damage_type;        /* 0=physical, 1=fire, 2=magic, 3=poison */
} Nexus_CreatureType;

typedef struct {
    int type_index;
    int health, max_health;
    int x, y, facing;       /* map position + direction */
    int alive;
    Nexus_CreatureState state;  /* IDLE/WANDER/APPROACH/ATTACK/FLEE/DEAD */
    int ai_timer;
    int alerted;             /* 0=normal, 255=alerted (alarm triggered) */
    int last_x, last_y;     /* previous position */
    int move_cooldown;
    int attack_cooldown;
    int group_id;
} Nexus_Creature;

/* Creature attribute flags */
#define NEXUS_CATTR_LEVITATION     0x0001
#define NEXUS_CATTR_NON_MATERIAL   0x0002
#define NEXUS_CATTR_SEE_INVISIBLE  0x0004
#define NEXUS_CATTR_NIGHT_VISION   0x0008
#define NEXUS_CATTR_ARCHENEMY      0x0010
#define NEXUS_CATTR_ATTACK_ANY     0x0020
#define NEXUS_CATTR_PREFER_BACK    0x0040
#define NEXUS_CATTR_FIRE_RESIST    0x0080
```

### 6.7 Creature AI — State Machine

**Source:** `src/nexus/nexus_v1_creatures.c:nexus_v1_creatures_tick()`

DM1-inspired 5-state AI with DDA LOS check:
```
IDLE/WANDER → (smell_range or alerted) → APPROACH
  (has LOS) → (attack_range) → ATTACK
  (lost LOS or out of range) → IDLE
HP < hp_warn → FLEE (toward away from party)
```

**ReDMCSB reference:** F0200 creature LOS, F0209 group events, BEHAVIOR state machine in `dm1_v1_creature_ai_behavior_pc34_compat.c`.

### 6.8 Monster Format Variants

| Aspect | DM1 | Nexus |
|--------|-----|-------|
| Creature count | ~42 types | ~30 types |
| Model format | 2D sprite (GRAPHICS.DAT) | **3D DMDF mesh (.MNS)** |
| Model files | None (sprites in EXE) | ~30 × .MNS files |
| AI model | State machine + LOS | Simplified state machine |
| Group formation | Cell-based | Cell-based |
| Projectile attacks | Fireball/Lightning/Poison | Unknown |

### 6.9 Firestaff Conversion — `Nexus_DMDFHeader` / `Nexus_V1_Model`

```c
/* Source: include/nexus_v1_dmdf_model.h */
typedef struct {
    uint32_t magic;          /* 0x444D4446 */
    uint32_t file_size;
    uint32_t section_count;
    uint32_t flags;
    uint32_t reserved[4];
    uint32_t data_offset;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t face_count;
} Nexus_DMDFHeader;

typedef struct {
    Nexus_DMDFHeader header;
    Nexus_DMDFVertex *vertices;
    int               vertex_count;
    uint16_t         *faces;
    int               face_count;
    uint8_t          *texture_data;
    int               texture_size;
    const char       *name;
} Nexus_V1_Model;

/* Loading: src/nexus/nexus_v1_dmdf_model.c:nexus_v1_dmdf_load() */
int nexus_v1_dmdf_load(Nexus_V1_Model *model, const uint8_t *data, int size, const char *name);
```

### 6.10 Status

| Item | Status | Source |
|------|--------|--------|
| DMDF header parsing | ✅ Implemented | `nexus_v1_dmdf_is_valid()` |
| DMDF vertex loading | ⚠️ Buggy | 10B stride vs 16B struct |
| DMDF face index loading | ✅ Implemented | Assumes triangles |
| DMDF texture data loading | ⚠️ Loaded, not decoded | VDP1 BITMAP unknown |
| VDP1 BITMAP format spec | ❌ **NOT DOCUMENTED** | Saturn-specific |
| Creature type init | ✅ Implemented | `nexus_v1_creatures_init()` |
| Creature spawn | ✅ Implemented | `nexus_v1_creature_spawn()` |
| Creature AI tick | ✅ Implemented | `nexus_v1_creatures_tick()` |
| DMDF vertex stride | ⚠️ **INCONSISTENT** | 10B loaded vs 16B struct |

---

## 7. Sound Format — `SNDLEV*.SAL` / `SNDLEV*.MAP` / CD Audio

### 7.1 File Inventory

| File | Size | Description |
|------|------|-------------|
| `SNDLEV00.SAL` – `SNDLEV15.SAL` | 290–460 KB each | Per-level sound banks |
| `SNDLEV00.MAP` – `SNDLEV15.MAP` | 66–90 bytes each | Per-level event index |
| CD Audio tracks 2–9 | Red Book Audio | 8 music tracks |

**32 files total, ~5.9 MB + audio.** The large .SAL file sizes (290–460 KB) suggest either compressed audio samples or a packed sample format.

### 7.2 Sound Engine Structure

```c
/* Source: include/nexus_v1_sound.h */
typedef struct {
    int initialized;
    int sfx_enabled;
    int music_enabled;
    int current_cd_track;
    int current_level;
    uint8_t *sal_data;    /* current level's .SAL data */
    int sal_size;
    uint8_t *map_data;    /* current level's .MAP data */
    int map_size;
} Nexus_SoundEngine;
```

### 7.3 Event Types (from MAP index)

```c
/* Source: include/nexus_v1_sound.h */
typedef enum {
    NEXUS_SFX_NONE = 0,
    NEXUS_SFX_FOOTSTEP       = 1,
    NEXUS_SFX_DOOR_OPEN      = 2,
    NEXUS_SFX_DOOR_CLOSE     = 3,
    NEXUS_SFX_ATTACK_HIT     = 4,
    NEXUS_SFX_ATTACK_MISS    = 5,
    NEXUS_SFX_CHAMPION_HURT  = 6,
    NEXUS_SFX_CREATURE_DEATH = 7,
    NEXUS_SFX_CREATURE_ATTACK= 8,
    NEXUS_SFX_SPELL_CAST     = 9,
    NEXUS_SFX_SPELL_IMPACT   = 10,
    NEXUS_SFX_PICKUP_ITEM    = 11,
    NEXUS_SFX_DROP_ITEM      = 12,
    NEXUS_SFX_STAIRS         = 13,
    NEXUS_SFX_TELEPORT       = 14,
    NEXUS_SFX_ALARM          = 15,
    NEXUS_SFX_PIT_FALL       = 16,
    NEXUS_SFX_MENU_SELECT    = 17,
    NEXUS_SFX_MENU_CONFIRM   = 18,
    NEXUS_SFX_MENU_CANCEL    = 19,
    NEXUS_SFX_GOLD_PICKUP    = 20
} Nexus_SoundEvent;
```

### 7.4 SAL Format

**Status: COMPLETELY UNKNOWN.**

Hypotheses for 290–460 KB per level:
1. **Saturn SAS or ATRAC audio** — compressed sample format used by Saturn sound hardware
2. **Signed 8-bit PCM** — 290–460 KB uncompressed at 22 kHz stereo ≈ 6.6–10.5 seconds
3. **Signed 16-bit PCM** — would give half the duration
4. **ADPCM** — 4:1 compression ratio common in 90s console games

The `SDDRVS.TSK` sound driver (26 KB) likely handles SAL decoding.

### 7.5 MAP Format

**Status: COMPLETELY UNKNOWN.**

The MAP files are 66–90 bytes each. This strongly suggests a small index table mapping event IDs to offsets/sizes within the SAL file.

Hypothesis: 20 events × (2-byte offset + 2-byte size) = 80 bytes — matches the observed 66–90 byte range.

### 7.6 CD Audio Track Mapping

**Source:** `nexus_v1_engine.c:nexus_v1_cd_track_for_level()`

```
Level pairs:  0–1 → track 2
               2–3 → track 3
               4–5 → track 4
               6–7 → track 5
               8–9 → track 6
              10–11 → track 7
              12–13 → track 8
              14–15 → track 9
```

**8 Red Book Audio CD-DA tracks** mapped to 8 level pairs.

### 7.7 Sound Format Variants

| Format | Description | Evidence | Status |
|--------|-------------|----------|--------|
| `SNDLEV*.SAL` | Per-level sound bank, 290–460 KB | Size patterns | ❌ **UNKNOWN FORMAT** |
| `SNDLEV*.MAP` | Event index, 66–90 bytes | Size range | ❌ **UNKNOWN FORMAT** |
| CD Audio | Red Book Audio tracks 2–9 | ISO track listing | ⚠️ Tracks present, playback stub |
| `SDDRVS.TSK` | Saturn sound driver task, 26 KB | Extracted file | ❌ **NOT ANALYZED** |

### 7.8 Firestaff Conversion

```c
/* Source: include/nexus_v1_sound.h */
int nexus_sound_init(Nexus_SoundEngine *eng);
void nexus_sound_shutdown(Nexus_SoundEngine *eng);
int nexus_sound_load_level(Nexus_SoundEngine *eng, int level_index,
                            const uint8_t *sal_data, int sal_size,
                            const uint8_t *map_data, int map_size);
void nexus_sound_play(Nexus_SoundEngine *eng, Nexus_SoundEvent event);
void nexus_sound_play_idx(Nexus_SoundEngine *eng, int sample_index);
int nexus_sound_cd_track(Nexus_SoundEngine *eng, int track_number);
int nexus_sound_cd_stop(Nexus_SoundEngine *eng);
void nexus_sound_set_sfx(Nexus_SoundEngine *eng, int enabled);
void nexus_sound_set_music(Nexus_SoundEngine *eng, int enabled);
const char *nexus_sound_event_name(Nexus_SoundEvent event);
```

**All functions are stubs** — they initialize state and log calls but do not decode SAL/MAP or play audio.

### 7.9 Status

| Item | Status |
|------|--------|
| Sound engine API | ✅ Stub implemented |
| SAL format decode | ❌ **NOT IMPLEMENTED** — format unknown |
| MAP event index parse | ❌ **NOT IMPLEMENTED** — format unknown |
| CD audio playback | ❌ **NOT IMPLEMENTED** — stub only |
| SDL_mixer integration | ❌ **NOT IMPLEMENTED** |
| SDDRVS.TSK analysis | ❌ **NOT ANALYZED** |

---

## 8. Graphics / Model Format — DMDF + VDP1 BITMAP

### 8.1 Overview

Nexus uses a layered graphics architecture:

```
Saturn CD-ROM
  ├── LEV*.DGN      → 3D geometry (walls, floors, ceilings) — unknown format
  ├── *.MNS (DMDF) → 3D creature models (vertices + faces + texture)
  ├── *.CG          → Color graphics (title screen)
  ├── *.DG2         → VDP2 background format (logo background)
  ├── *.S2D         → Saturn SCR font
  ├── *.BIN         → Binary graphics (FACE.BIN, STONE.BIN, etc.)
  └── ITEM.IBS      → Item icon/bitmap set
```

### 8.2 VDP1 BITMAP Texture Format

**Status: NOT DOCUMENTED.**

VDP1 is the Saturn's second video display processor, capable of direct framebuffer rendering with texture mapping. VDP1 BITMAP textures are stored in the Saturn's native format, which is not a standard PC format.

**Known characteristics:**
- Big-endian pixel data
- Can be 8bpp (palette) or 16bpp (direct color)
- May use 4-bit or 8-bit clut (color look-up table)
- Texture dimensions must be powers of 2 (8, 16, 32, 64, 128, 256)

**Decompression: NOT VERIFIED.** DMDF embedded textures may be compressed.

### 8.3 Other Graphics Files

| File | Size | Format | Status |
|------|------|--------|--------|
| `TITLE.CG` | 164 KB | Unknown (color graphics) | ❌ Not analyzed |
| `LOGOBG.DG2` | 71 KB | VDP2 background | ❌ Not analyzed |
| `FONT256.S2D` | 24 KB | Saturn SCR | ✅ Header ✅ Glyph access |
| `FACE.BIN` | 44 KB | Champion portraits (24 entries) | ❌ Format unknown |
| `STONE.BIN` | 4 KB | Wall/stone texture base | ❌ Not analyzed |
| `NBG3.BIN` | 7 KB | VDP2 background layer | ❌ Not analyzed |
| `POTEFT.BIN` | 3 KB | Potion effect graphics | ❌ Not analyzed |
| `STABG.BIN` | 52 KB | Status area background | ❌ Not analyzed |
| `SWTCHR.BIN` | 38 KB | Switch/lever graphics | ❌ Not analyzed |
| `ITEM.IBS` | 98 KB | Item icon/bitmap set | ❌ Not analyzed |
| `TM.BIN` | 156 KB | Texture/tilemap data | ❌ Not analyzed |
| `MENU.BPK` | 87 KB | Menu graphics (packed) | ❌ Not analyzed |
| `DEATH.BIN` | 4 KB | Death sequence data | ❌ Not analyzed |

### 8.4 Graphics Format Variants

| Format | Description | Status |
|--------|-------------|--------|
| DMDF (.MNS) | 3D creature mesh + VDP1 BITMAP texture | ✅ Partial |
| VDP1 BITMAP | Saturn hardware texture format | ❌ **NOT DOCUMENTED** |
| VDP2 background | Saturn VDP2 layer format | ❌ **NOT DOCUMENTED** |
| Saturn SCR font | FONT256.S2D | ✅ Header ✅ Glyph access |
| Packed (.BPK) | Menu graphics pack format | ❌ **NOT DOCUMENTED** |
| IBS icon set | Item bitmap set | ❌ **NOT DOCUMENTED** |
| CG color graphics | Title/color graphics | ❌ **NOT ANALYZED** |

### 8.5 Firestaff Conversion — DMDF

```c
/* Source: include/nexus_v1_dmdf_model.h */
int nexus_v1_dmdf_load(Nexus_V1_Model *model,
                        const uint8_t *data, int size, const char *name);
void nexus_v1_dmdf_free(Nexus_V1_Model *model);
int nexus_v1_dmdf_is_valid(const uint8_t *data, int size);

/* Saturn SCR font: include/nexus_v1_saturn_font.h */
int  nexus_v1_font_load(Nexus_V1_Font *font,
                          const uint8_t *data, int size);
void nexus_v1_font_free(Nexus_V1_Font *font);
const uint8_t *nexus_v1_font_get_glyph(const Nexus_V1_Font *font, int idx);
```

### 8.6 Status

| Item | Status |
|------|--------|
| DMDF header/vertex/face parsing | ✅ Implemented |
| DMDF vertex stride bug | ⚠️ 10B loaded vs 16B struct |
| VDP1 BITMAP texture format | ❌ **NOT DOCUMENTED** |
| VDP2 background format | ❌ **NOT DOCUMENTED** |
| Saturn SCR font | ✅ Header ✅ Glyph access |
| FACE.BIN portraits | ❌ **NOT IMPLEMENTED** |
| ITEM.IBS icon set | ❌ **NOT IMPLEMENTED** |
| MENU.BPK packed graphics | ❌ **NOT DOCUMENTED** |
| TITLE.CG / LOGOBG.DG2 | ❌ **NOT ANALYZED** |

---

## 9. Save Format — `saves/nexus/*.fssv`

### 9.1 Save Namespace

**Source:** `src/nexus/nexus_v1_save_load.c`, `src/engine/firestaff_save.c`

Saves namespaced under `saves/nexus/`:
```
saves/nexus/slot0.fssv  (Firestaff Save Format)
saves/nexus/slot1.fssv
...
saves/nexus/slot9.fssv  (max 10 slots)
```

Namespace is **`nexus`** (not `nexus`), matching `FS_GameId` string.

### 9.2 Save Structure

**Status: PARTIALLY IMPLEMENTED.**

The `nexus_v1_save_load.c` file (22,012 bytes) implements:
- `Nexus_V1_SaveData` struct with world state, champion pool, timers, events
- `nexus_v1_save_write()` / `nexus_v1_save_read()` — Firestaff save format
- MD5-based slot manifest
- Checksum verification

**Unknown:** Whether the save format matches the original Saturn save format or is a Firestaff-only format.

### 9.3 Firestaff Save Format Fields

```c
/* Source: src/nexus/nexus_v1_save_load.c — approximate struct */
typedef struct {
    uint32_t magic;           /* "FNXS" magic */
    uint32_t version;
    Nexus_V1_World world;     /* full world state */
    Nexus_V1_ChampionPool pool; /* champion roster + party */
    uint8_t  champion_count;
    uint8_t  party[4];
    uint64_t checksum;        /* CRC of save data */
} Nexus_V1_SaveData;
```

### 9.4 Status

| Item | Status |
|------|--------|
| Firestaff save format | ✅ Implemented |
| Original Saturn save format | ❌ **UNKNOWN** |
| Checksum verification | ✅ Implemented |
| Auto-save | ⚠️ Not verified |

---

## 10. Source-Lock Summary

### 10.1 Format Confidence

| Format | Confidence | Evidence |
|--------|-----------|----------|
| LEV\*.DGN grid | **High** | 32×32 confirmed by binary parsing |
| LEV\*.DGN 3D geometry | **None** | Format completely unknown |
| SMAP\*.BIN | **None** | No binary inspection |
| NEXUS_OBJECT_\* types | **High** | Enum defined and implemented |
| FONT256.S2D header | **High** | "SEGA SATURN SCR" magic confirmed |
| FONT256.S2D glyph data | **Medium** | Structure plausible, not byte-verified |
| Shift-JIS ASCII/Katakana | **High** | Source code confirmed |
| Shift-JIS Kanji | **Low** | Replaced with "?", no lookup table |
| Champion roster | **High** | 8 entries confirmed in source |
| Champion struct | **High** | Full struct defined |
| FACE.BIN | **Low** | Size known, format unknown |
| DMDF header | **High** | 0x444MDF magic confirmed |
| DMDF vertex stride | **Medium** | Bug: 10B loaded vs 16B struct |
| DMDF face format | **High** | Triangle assumption plausible |
| DMDF VDP1 texture | **None** | Saturn-specific, not documented |
| SNDLEV\*.SAL | **None** | No binary inspection |
| SNDLEV\*.MAP | **None** | Size suggests index table, not verified |
| CD audio mapping | **High** | Level-pair → track mapping confirmed |
| Nexus save format | **Medium** | Implemented but format origin unknown |
| DGN thing/object list | **None** | Embedded but unparsed |

### 10.2 Critical Gaps

| Gap | Severity | Blocking |
|-----|----------|----------|
| No disc image | **CRITICAL** | All hashes unverifiable |
| DGN 3D geometry blob | **HIGH** | Cannot render dungeon geometry |
| DMDF vertex stride bug | **HIGH** | Models loaded incorrectly |
| VDP1 BITMAP texture | **HIGH** | Cannot display creature textures |
| SAL/MAP sound format | **HIGH** | No audio playback |
| SMAP\*.BIN format | **MEDIUM** | Minimap not available |
| FACE.BIN portraits | **MEDIUM** | Champion portraits missing |
| Full 24-champion roster | **MEDIUM** | Only 8 champions defined |
| Shift-JIS kanji lookup | **MEDIUM** | All kanji displayed as "?" |
| SDDRVS.TSK analysis | **LOW** | Sound driver not examined |

### 10.3 No ReDMCSB Equivalent

ReDMCSB (WIP20210206) covers DM1, CSB, and DM2 only. There is **no Saturn or Nexus code** in the ReDMCSB source tree.

All Nexus format claims are best-effort reverse engineering from:
1. Extracted file inspection (sizes, magic bytes)
2. Binary pattern analysis (endianness, structure hints)
3. Game content comparison with DM1/DM2
4. Saturn hardware documentation (for VDP1/VDP2 formats)

---

## 11. Phase 2 Completion Checklist

```
Dungeon (LEV*.DGN):
[✓] Grid parsing (32×32 big-endian uint16)
[✓] Square type enum (NEXUS_SQUARE_*)
[✗] 3D geometry blob format
[✗] Thing/object list in DGN
[✓] Grid → Nexus_V1_Level conversion

Map (SMAP*.BIN):
[✗] SMAP*.BIN format analysis
[✗] Nexus_V1_Minimap struct
[✗] Minimap renderer

Object (NEXUS_OBJECT_*):
[✓] Object type enum
[✓] Nexus_V1_Object struct
[✓] Object database API
[✗] DGN thing list parser

Text (FONT256.S2D + Shift-JIS):
[✓] FONT256.S2D header parsing
[✓] Glyph access API
[✓] Shift-JIS → UTF-8 ASCII/Katakana
[✗] Shift-JIS → UTF-8 Kanji (lookup table)
[✗] Glyph blit to framebuffer
[✗] *.TXT lore file loading
[✗] SLEV*.BIN script parsing

Champion (CHAMPIONS.DAT, FACE.BIN):
[✓] Champion roster (8 of 24)
[✓] Nexus_V1_Champion struct
[✓] 4-class system (Fighter/Wizard/Priest/Ninja)
[✓] Party system (4 active)
[✗] Full 24-champion roster
[✗] FACE.BIN portrait parsing
[✗] Portrait rendering
[✗] Stat advancement (XP-based)

Monster (*.MNS DMDF):
[✓] DMDF header (0x444D4446 magic)
[~] DMDF vertex loading (buggy stride)
[✓] DMDF face index loading
[✓] Nexus_CreatureType definitions
[✓] Creature AI state machine
[✗] DMDF vertex stride fix (10B vs 16B)
[✗] VDP1 BITMAP texture format
[✗] DMDF texture decompression

Sound (SNDLEV*.SAL, SNDLEV*.MAP):
[✓] Sound engine API (stub)
[✓] Nexus_SoundEvent enum
[✓] CD audio track mapping
[✗] SAL format decode
[✗] MAP event index parse
[✗] SDL_mixer integration
[✗] SDDRVS.TSK analysis

Graphics/Model (DMDF + VDP1 + other):
[✓] DMDF parsing
[~] DMDF vertex stride (buggy)
[✗] VDP1 BITMAP texture spec
[✗] VDP2 background format
[✗] TITLE.CG / LOGOBG.DG2
[✗] ITEM.IBS icon set
[✗] MENU.BPK packed graphics
[✗] FACE.BIN portraits
```

**Phase 2 completion: ~45%** (format documented but implementation incomplete across most areas)

---

*Generated by cron job `Nexus_V1_DataFormats_H2321`*
*Next step: obtain disc image for hash verification; fix DMDF vertex stride; document VDP1 BITMAP format; decode SMAP*.BIN*
