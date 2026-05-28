# Nexus V1 Phase 2 — Data Formats: Source-Lock Document

**Cron task:** `Nexus_V1_Phase2_DataFormats_0424`
**Status:** ⚠️ SUPERSEDED for DGN/SMAP/ITEM.IBS/MNS details by
`docs/source-lock/nexus_v1_dmweb_format_crawl_20260528.md`
**Author:** Firestaff agent (cron)
**Last revised:** 2026-05-27T03:45 UTC

---

## Scope

Source-lock every Nexus V1 format landscape before converting into Firestaff
C structures. Applies to all 16 dungeon levels (LEV00–LEV15.DGN) on the Sega
Saturn release (T-9111G V1.003, 1998-02-03, Japan only). Covers endianness,
byte layout, Provenance Gate reference, and known gaps per data category.

> **Source-lock rule:** Every format claim cites a source file or section in
> `(src|docs|include)/nexus/` or `docs/NEXUS_FILE_CLASSIFICATION.md`. Where
> the original Saturn format is unknown, the Firestaff stub or working
> hypothesis is explicitly marked STUB / UNKNOWN / INFERRED.

> **2026-05-28 correction:** DMWeb's Dungeon Master Nexus documentation
> contradicts the early Firestaff DGN hypothesis below. Real DGN files are
> 2048-byte block containers with a 64x64 Structure1B grid. Treat raw 32x32
> offset-0 claims in this document as historical notes only.

---

## 1. Game / Platform Identification

| Field | Value |
|-------|-------|
| Platform | Sega Saturn (SH-2 big-endian) |
| Release | 1998-02-03, Japan only |
| Product ID | T-9111G V1.003 |
| Media | CD-ROM, CUE/BIN (9 tracks: 1×MODE1/2352 data + 8×CD-DA audio) |
| CPU | 2× Hitachi SH-2 @ ~10.5 MHz, **big-endian** |
| 3D | Sega VDP1 polygon rasterizer (software on SH-2) |
| Audio | Red Book CD-DA (tracks 2–9) + ADX/SEGA PCM SFX |
| Languages | Japanese only |

All multi-byte values on disc are **big-endian** (SH2 byte order). Firestaff
PC builds (x86/ARM, little-endian) use `rb16()` / `rb32()` byte-swapping helpers
inline in each parser. Source: `nexus_v1_dmdf_model.c`, `nexus_v1_dungeon.c`,
`nexus_v1_iso_reader.c`.

---

## 2. Image / ISO Structure

### 2.1 Disc Image Layout

| Track | Type | Content |
|-------|------|---------|
| 1 | MODE1/2352 | ISO 9660 filesystem: game data, 133 MB |
| 2–9 | CD-DA Audio | Red Book Audio music, 2 levels per track |

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`, `nexus_v1_engine.c`

### 2.2 ISO 9660 Reader (`nexus_v1_iso_reader.c/h`)

Key constants (`include/nexus_v1_iso_reader.h`):
```
NEXUS_ISO_SECTOR_SIZE  = 2352  (MODE1 full sector)
NEXUS_ISO_DATA_OFFSET = 16    (user data starts at byte 16)
NEXUS_ISO_DATA_SIZE   = 2048  (user data bytes per sector)
NEXUS_ISO_MAX_FILES   = 1024
```

`Nexus_ISOReader` provides:
- `nexus_iso_open()` / `nexus_iso_open_cue()` — open BIN or CUE
- `nexus_iso_find()` — case-insensitive file lookup by name
- `nexus_iso_read_file()` — read entire file (spanning sectors)
- `nexus_iso_read_file_chunk()` — read byte range within a file
- `nexus_iso_is_nexus()` — checks for `DM.BIN` + `LEV00.DGN` in root

Reading strategy:
1. Read PVD at sector 16 (`pvd[0]==1 && memcmp(pvd+1,"CD001",5)==0`)
2. Parse root directory (LBA at `pvd[158]`, size at `pvd[166]`)
3. Recurse directory records (ISO 9660 directory record format)
4. Strip version suffix `;1` from filenames

### 2.3 Disc File Manifest

137 files extracted. Key groups:

| Group | Files | Total | Description |
|-------|-------|-------|-------------|
| LEV\*.DGN | 16 | 4.1 MB | Dungeon levels |
| \*.MNS | 30 | 1.5 MB | 3D creature models |
| SNDLEV\*.SAL | 16 | ~5.4 MB | Per-level SFX banks |
| SNDLEV\*.MAP | 16 | ~1.1 KB | SFX event maps |
| DMV\*.AVI | 3 | 101 MB | FMV cutscenes |
| SLEV\*.BIN | 16 | ~120 KB | Level script files |
| SMAP\*.BIN | 16 | ~400 KB | Minimap data |
| DM.BIN | 1 | 542 KB | Main game executable |
| TITLE.BIN | 1 | 110 KB | Title screen |
| TITLE.CG | 1 | 164 KB | Title color graphics |
| FONT256.S2D | 1 | 24 KB | 256-char Saturn font |
| ITEM.IBS | 1 | 98 KB | Item icon set |
| FACE.BIN | 1 | 44 KB | Champion face portraits |
| STABG.BIN | 1 | 52 KB | Status bar background |
| 0DMSTRT.BIN | 1 | 39 KB | Startup data |
| SDDRVS.TSK | 1 | 26 KB | Saturn sound driver |

Source: `docs/NEXUS_FILE_CLASSIFICATION.md` (137-file inventory)

### 2.4 Signature Files (Provenance Gate)

Presence of these files confirms a valid Nexus disc image:

| File | Purpose | `nexus_iso_find()` reference |
|------|---------|------------------------------|
| `DM.BIN` | Main executable/data | ✅ checked in `nexus_iso_is_nexus()` |
| `LEV00.DGN` | Level 0 dungeon data | ✅ checked in `nexus_iso_is_nexus()` |
| `SDDRVS.TSK` | Sound driver | presence confirms audio capability |
| `FONT256.S2D` | Font | presence confirms Japanese text |
| `SNDLEV00.SAL` | Level 0 SFX | presence confirms per-level audio |

SHA256 hash lock: **NOT SET** — no disc image in `.firestaff/data/nexus/`.
All further format claims are best-effort from extracted file analysis.

---

## 3. Dungeon Level Format — LEV\*.DGN

### 3.1 File Sizes

| Level | File | Size (bytes) | Grid |
|-------|------|-------------|------|
| 0 | LEV00.DGN | 147,456 | 32×32 |
| 1 | LEV01.DGN | 280,576 | 32×32 |
| 2 | LEV02.DGN | 272,384 | 32×32 |
| 3 | LEV03.DGN | 290,816 | 32×32 |
| 4 | LEV04.DGN | 245,760 | 32×32 |
| 5 | LEV05.DGN | 266,240 | 32×32 |
| 6 | LEV06.DGN | 239,616 | 32×32 |
| 7 | LEV07.DGN | 258,048 | 32×32 |
| 8 | LEV08.DGN | 303,104 | 32×32 |
| 9 | LEV09.DGN | 288,768 | 32×32 |
| 10 | LEV10.DGN | 290,816 | 32×32 |
| 11 | LEV11.DGN | 278,528 | 32×32 |
| 12 | LEV12.DGN | 321,536 | 32×32 |
| 13 | LEV13.DGN | 256,000 | 32×32 |
| 14 | LEV14.DGN | 253,952 | 32×32 |
| 15 | LEV15.DGN | 270,336 | 32×32 |

**Total: ~4.3 MB**. DM1 DUNGEON.DAT is ~33 KB — Nexus is **~130× larger**
because 3D polygon geometry is pre-baked into each DGN file rather than
computed at runtime.

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`, `docs/nexus_data.md`

### 3.2 Layout: Two Sections

Each DGN file contains:

**Section 1 — Grid (first 2048 bytes for 32×32 fixed grid):**
```
Offset 0–1:   uint16_t width    (big-endian, extracted via rb16())
Offset 2–3:   uint16_t height   (big-endian, extracted via rb16())
Offset 4+:    grid entries (uint16_t LE per cell, lower 5 bits = tile type)
```

Layout variant detection (from `nexus_v1_dungeon.c`):

* **Layout A:** bytes 0–1 = `width`, bytes 2–3 = `height` (big-endian u16) —
  header is 4 bytes, grid data starts at offset **4**
* **Layout B:** raw 32×32 grid starts at offset **0** (DM1-style)

Parsing at offset `grid_offset` reads uint16 per cell (`rb16() & 0x1F`).
Source: `nexus_v1_dungeon.c:nexus_v1_level_load()`, doc comment citing
`DUNGEON.C F0001` from ReDMCSB.

**Section 2 — Post-grid geometry blob (all remaining bytes):**
- Pre-computed wall polygon data per grid position
- Floor/ceiling mesh vertices per open square
- Per-square mesh identifiers (wall type, door state, stairs variant)
- **NOT YET PARSED** — `nexus_v1_dungeon.c` sets `has_3d_geometry = 1` and
  records `geometry_offset` / `geometry_size` as a gap marker

### 3.3 Square Type Encoding

Square types use the **same 5-bit scheme as DM1** (proven by hex analysis of
DGN data patterns and the mask `& 0x1F` in `nexus_v1_level_get_square()`):

| Value | Name | Behavior |
|-------|------|----------|
| 0 | `NEXUS_SQUARE_WALL` | Blocks movement; wall geometry rendered |
| 1 | `NEXUS_SQUARE_FLOOR` | Normal passage; floor + ceiling rendered |
| 2 | `NEXUS_SQUARE_STAIRS_DN` | Descend one level |
| 3 | `NEXUS_SQUARE_STAIRS_UP` | Ascend one level |
| 8 | `NEXUS_SQUARE_DOOR` | 3D door geometry, scripted open/close via SDDRVS.TSK |
| 9 | `NEXUS_SQUARE_TELEPORT` | Scripted teleport |
| 10 | `NEXUS_SQUARE_TELEPORT2` | Intra-level teleport |
| 11 | `NEXUS_SQUARE_TELEPORT3` | Level-wide teleport |
| 12 | `NEXUS_SQUARE_ALARM` | Alarm trap (creatures chase) |
| 13 | `NEXUS_SQUARE_CHUTE` | Chute/trapdoor (party falls) |
| 14 | `NEXUS_SQUARE_EXIT` | Dungeon exit |
| 15–31 | — | Other/extended |

Square semantics (door/teleporter/trap) are handled by **SDDRVS.TSK scripting**
rather than hardwired type codes (unlike DM1 where types 6/7/9/10/11 are
compile-time dispatched via `F0267/F0268`). Source: `docs/nexus_squares.md`,
`nexus_v1_squares.c`.

### 3.4 Core Structures

`include/nexus_v1_dungeon.h`:
```c
#define NEXUS_MAX_MAP_SIZE 64

typedef struct {
    int width, height;
    uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE]; /* decoded Structure1B passability */
    int thing_count;
    int creature_count;
    int has_3d_geometry;
    int geometry_offset;   /* byte offset after Structure1B */
    int geometry_size;    /* bytes remaining after grid section */
} Nexus_V1_Level;

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index);
int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y);
```

### 3.5 Dungeon Format Status

| Sub-format | Status | Evidence |
|-----------|--------|---------|
| Grid section | ✅ Parsed (Layout A & B) | `nexus_v1_dungeon.c` |
| 3D geometry blob | ❌ NOT PARSED | Gap marker set; `has_3d_geometry` signals future work |
| Thing list (items) | ❌ NOT PARSED | No thing data extraction in current source |
| Creature placement | ❌ NOT PARSED | No per-level creature spawn records |

---

## 4. Map / Map-Supplementary Format — SMAP\*.BIN, SLEV\*.BIN

### 4.1 Minimap Files — SMAP\*.BIN

- 16 files: `SMAP00.BIN` through `SMAP15.BIN` (17–30 KB each)
- Per-level minimap/automap images displayed by the in-game map command
- **Format: UNKNOWN.** Likely a raw bitmap (2bpp or 4bpp, palette-encoded)
  matching the Saturn VDP2 background layer format
- `nexus_iso_reader.c` can read these files but no parser exists
- **Status:** STUB — read via `nexus_iso_read_file()`, no parsing

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`

### 4.2 Level Script Files — SLEV\*.BIN

- 16 files: `SLEV00.BIN` through `SLEV15.BIN` (2–12 KB each)
- Per-level declarative event rules processed by the **SDDRVS.TSK script VM**
- Controls: teleporters, trap triggers, door animations, level transitions,
  creature spawns, and other scripted behaviors
- **Format: UNKNOWN.** Declarative event-rule format (likely record-based:
  condition → actor → effect). Unlike DM1's bytecoded `SQUARE.C` sensors
  (F0355/F0356), Nexus uses a table-driven rule system
- SDDRVS.TSK (26 KB Saturn sound driver task) runs the script VM

Key difference vs DM1:
- DM1: sensors hardwired at compile time (`SQUARE.C`, `SENSORS.C`)
- Nexus: declarative scripts in SLEV\*.BIN, processed by SDDRVS.TSK at runtime
- **Implication:** Nexus dungeon behavior is far more designer-friendly
  (no recompile needed for new dungeons), which explains the 16-level
  depth vs DM1's 8

Source: `docs/NEXUS_FILE_CLASSIFICATION.md` (SLEV\*.BIN listed),
`docs/nexus_squares.md` (§3), `nexus_v1_engine.c` (no TSK references found)

### 4.3 Format Status

| Sub-format | Status | Evidence |
|-----------|--------|---------|
| SMAP\*.BIN minimap | ❌ NOT PARSED | Raw read available, no parser |
| SLEV\*.BIN scripts | ❌ NOT PARSED | No SDDRVS.TSK VM in codebase |
| SDDRVS.TSK driver | ❌ NOT REVERSED | 26 KB binary, no disassembly |

---

## 5. Object / Item Format

###  Item Encyclopedia (`include/nexus_v1_inventory.h`)

Nexus uses the **same item system as DM1**, inherited from the shared
`firestaff_item_encyclopedia.c`. Item indices and stat definitions are
identical to DM1 PC 3.4.

**Item categories:**
```c
typedef enum {
    NEXUS_ITEM_WEAPON    = 0,
    NEXUS_ITEM_ARMOR     = 1,
    NEXUS_ITEM_POTION    = 2,
    NEXUS_ITEM_SCROLL    = 3,
    NEXUS_ITEM_CONTAINER = 4,
    NEXUS_ITEM_MISC      = 5,
    NEXUS_ITEM_KEY       = 6,
    NEXUS_ITEM_SPELL_RUNE= 7,
    NEXUS_ITEM_COUNT     = 8
} Nexus_ItemCategory;
```

**Item definition:** `Nexus_ItemDef { name, category, weight, attack, defense, flags }`

**Notable items from `nexus_v1_inventory.c`:**
- Weapon indices 0–11: Falchion, Rapier, Mace, Club, Staff, Sword, Axe,
  Dagger, Arrow, Slayer, Vorpal Blade, Firestaff
- Armor indices 20–26: Leather Jerkin, Mail Achain, Plate Armor, Shield,
  Helmet, Boots, Gauntlets
- Potions: Health (30), Mana (31), Stamina (32), Antidote (33)
- Scroll (40), Containers (50–51), Torch/Compass/Rabbit Foot/Corn/Water/Rope (60–65), Keys (70+)
- Item flags: `NEXUS_ITEMF_EQUIPPABLE`, `NEXUS_ITEMF_CONSUMABLE`,
  `NEXUS_ITEMF_STACKABLE`, `NEXUS_ITEMF_KEY`, `NEXUS_ITEMF_NO_DROP`

### 5.2 Champion Inventory Structure

Per champion (`include/nexus_v1_champions.h`):
```c
uint8_t inventory[30];    /* item indices — 30 slots */
```

30 slots vs DM1's 12-slot grid. Representation difference: Nexus may encode
equipment slots (weapon, shield, head, torso, etc.) as a flat array vs DM1's
separate slot encoding. The flat 30-slot model in `Nexus_V1_Champion` is the
in-memory structure.

Source: `include/nexus_v1_champions.h:inventory[30]`, `docs/nexus_inventory.md`

### 5.3 Floor Item Structure

```c
typedef struct {
    int item_id;    /* index into global item catalog, -1 = empty */
    int quantity;
    int x, y;       /* dungeon position */
    int on_ground;  /* 1 = on ground, 0 = in container */
} Nexus_FloorItem;
```

Source: `include/nexus_v1_inventory.h`

### 5.4 Item Format Status

| Sub-format | Status | Evidence |
|-----------|--------|---------|
| Global item catalog | ✅ DM1-style (enumerated) | `nexus_v1_inventory.c` |
| Champion inventory | ✅ In-memory (flat 30-slot) | `nexus_v1_champions.h` |
| Floor items | ✅ In-memory struct defined | `nexus_v1_inventory.h` |
| Item-on-discord format | ❌ NOT REVERSED | No .BIN item file parsed |
| Object placement (DGN) | ❌ NOT PARSED | No per-square thing loading |

---

## 6. Text Format

### 6.1 Font File — FONT256.S2D

Format: Sega Saturn SCR (Screen font resource). Header: `"SEGA SATURN SCR"`
(16 bytes, null-terminated ASCII).

**Header layout:**
```
Offset 0–15:  "SEGA SATURN SCR\0"  (16 bytes, big-endian u16 length follows)
Offset 16–17: uint16_t char_count (big-endian, max 512, default 256)
Offset 18+:    Glyph bitmap data begins here
```

**Glyph format:**
- Dimensions auto-detected based on glyph size:
  - `glyph_size >= 32` → 16×16 px
  - `glyph_size >= 18` → 12×12 px
  - otherwise → 8px wide, `glyph_size` tall
- 1 bpp bitmap per glyph (1 bit per pixel)
- For 256 glyphs at 16×16 (1 bpp = 32 bytes/glyph): 8192 bytes of glyph data

Loaded by `nexus_v1_font_load()` in `nexus_v1_engine.c`. API:
```c
int  nexus_v1_font_load(Nexus_V1_Font *font, const uint8_t *data, int size);
void nexus_v1_font_free(Nexus_V1_Font *font);
const uint8_t *nexus_v1_font_get_glyph(const Nexus_V1_Font *font, int idx);
```

Source: `include/nexus_v1_saturn_font.h`, `docs/nexus_text.md`

### 6.2 Shift-JIS Text Encoding

Nexus stores all in-game text in **Shift-JIS** (Japanese Industrial Standard).

**Encoding ranges in `nexus_v1_text.c`:**
| Range | Content | Conversion |
|-------|---------|------------|
| `0x20–0x7E` | ASCII | Pass-through (1:1) |
| `0xA1–0xDF` | Half-width katakana | → UTF-8 U+FF61–FF9F |
| `0x81–0x9F` | Shift-JIS double-byte lead | Decode as JIS X 0208 kanji |
| `0xE0–0xEF` | Shift-JIS double-byte lead | Decode as JIS X 0208 kanji |
| `0x40–0x7E`, `0x80–0xFC` | Trailing byte | Combined with lead |

**Half-width katakana mapping:** `U+FF61 = 0xA1`
```c
uint16_t cp = 0xFF61 + (b - 0xA1);   // b in range 0xA1–0xDF
utf8_out[ui++] = (char)(0xE0 | (cp >> 12));     // 3-byte UTF-8
utf8_out[ui++] = (char)(0x80 | ((cp >> 6) & 0x3F));
utf8_out[ui++] = (char)(0x80 | (cp & 0x3F));
```

Double-byte JIS X 0208 kanji: not yet implemented → replaced by `'?'`.
A full lookup table (188 valid lead-byte × 2 trailing-byte combinations)
would fill ~36 K single-byte-per-entry — implement when kanji support is needed.

Source: `src/nexus/nexus_v1_text.c` (SJIS → UTF-8 converter), `docs/nexus_text.md`

### 6.3 String Extraction

`nexus_v1_extract_strings()` scans binary data for runs of Shift-JIS bytes
(4+ consecutive printable bytes → extracted as a string run). Uses a static
buffer — returned pointers are only valid until the next call.

### 6.4 Text Rendering Status

| Sub-format | Status | Evidence |
|-----------|--------|---------|
| FONT256.S2D loading | ✅ Parsed | `nexus_v1_saturn_font.c` |
| Glyph bitmap access | ✅ API exists | `nexus_v1_font_get_glyph()` |
| SJIS → UTF-8 (ASCII+katakana) | ✅ Implemented | `nexus_v1_sjis_to_utf8()` |
| JIS X 0308 kanji | ❌ NOT IMPLEMENTED | `'?'` placeholder |
| Glyph blit to framebuffer | ❌ NOT IMPLEMENTED | No blit function |
| Text layout engine | ❌ NOT IMPLEMENTED | No layout code |
| In-game message rendering | ❌ NOT IMPLEMENTED | HUD text not wired |

### 6.5 Champion Name Storage

Champion names in `Nexus_V1_Champion` are stored as dual fields:
```c
char name_ascii[32];  /* Romanized ASCII name */
char name_jp[64];    /* UTF-8-encoded Japanese name (UTF-8, not Shift-JIS) */
```

Confirmed roster (partial, 8 Japanese champions defined in `nexus_v1_champions.c`):
Syra, Leyla, Nabi, Gando, Torham, Elija, Wu Tse, Stamm.
Full 24-champion roster not yet defined in source.

Source: `src/nexus/nexus_v1_champions.c` (static roster table)

---

## 7. Champion / Party Format

### 7.1 Champion Struct

`include/nexus_v1_champions.h`:
```c
#define NEXUS_MAX_CHAMPIONS 24
#define NEXUS_MAX_PARTY      4

typedef struct {
    char name_ascii[32];
    char name_jp[64];
    Nexus_ChampionClass primary_class;   /* FIGHTER/NINJA/PRIEST/WIZARD */
    int health, max_health;
    int stamina, max_stamina;
    int mana, max_mana;
    int strength, dexterity, wisdom, vitality, anti_magic, anti_fire;
    int fighter_level, ninja_level, priest_level, wizard_level;
    int food, water;
    int alive;
    int portrait_index;     /* CG texture index for portrait */
    uint8_t inventory[30]; /* item indices */
} Nexus_V1_Champion;

typedef enum {
    NEXUS_CLASS_FIGHTER = 0,
    NEXUS_CLASS_NINJA   = 1,
    NEXUS_CLASS_PRIEST = 2,
    NEXUS_CLASS_WIZARD = 3,
    NEXUS_CLASS_COUNT
} Nexus_ChampionClass;
```

### 7.2 Champion Pool

```c
typedef struct {
    Nexus_V1_Champion champions[NEXUS_MAX_CHAMPIONS]; /* full 24-champion roster */
    int champion_count;
    int party[NEXUS_MAX_PARTY];                       /* indices into champions[] */
    int party_count;
    int leader_index;                                 /* party leader slot */
} Nexus_V1_ChampionPool;
```

Source: `include/nexus_v1_champions.h`, `src/nexus/nexus_v1_champions.c`

### 7.3 Champion System Status

| Sub-format | Status | Evidence |
|-----------|--------|---------|
| Champion roster (in-memory) | ✅ Parsed | `nexus_v1_champions.c` static table |
| Champion persistence | ❌ NOT IMPLEMENTED | No binary champion file parsed from disc |
| 24-champion Hall of Fame | ⚠️ PARTIAL | 8 defined, full 24 not yet |
| Party management (in-game) | ✅ API exists | `nexus_v1_champion_recruit()`, `nexus_v1_champion_resurrect()` |
| FACE.BIN portraits | ⚠️ FILE EXISTS (44 KB) | No parser exists; format unknown |

### 7.4 FACE.BIN Note

`FACE.BIN` (44 KB) in the disc manifest. Likely stores the 24 champion
portrait graphics used in the UI when selecting/championing characters.
Format unknown — likely a bitmap sheet (4-bit or 8-bit indexed) with one
portrait per champion and possibly animation frames. No parser in current
source.

---

## 8. Monster / Creature Format

### 8.1 DMDF — Dungeon Master Data Format (3D Model Format)

DMDF is the **Nexus-specific 3D model format** used for all creature models
(.MNS files). Defined in `include/nexus_v1_dmdf_model.h` and implemented in
`src/nexus/nexus_v1_dmdf_model.c`.

**Magic:** `0x444D4446` = `"DMDF"` at byte offset 0 (big-endian uint32)

**Header structure (all values big-endian, read via `rb16()`/`rb32()`):**
```
Offset  0 (4):  magic = 0x444D4446
Offset  4 (4):  file_size (uint32)
Offset  8 (4):  section_count (uint32)
Offset 12 (4):  flags (uint32)
Offset 16 (4):  [reserved]
Offset 20 (4):  [reserved]
Offset 24 (4):  [reserved]
Offset 28 (4):  data_offset   (uint32) — absolute offset of data section
Offset 32 (4):  vertex_offset (uint32) — relative to data_section start
Offset 36 (4):  vertex_count  (uint32)
Offset 40 (4):  face_count    (uint32)
```

**Vertex structure** (`Nexus_DMDFVertex`, 10 bytes each):
```
Offset +0 (2): int16_t x   (model-space X)
Offset +2 (2): int16_t y   (model-space Y)
Offset +4 (2): int16_t z   (model-space Z)
Offset +6 (2): int16_t nx  (normal X)
Offset +8 (2): int16_t ny  (normal Y)
Offset+10 (2): int16_t nz  (normal Z)
Offset+12 (2): uint16_t u  (texture U coord)
Offset+14 (2): uint16_t v  (texture V coord)
```

**Face structure:** uint16_t index arrays. Triangle = 3 indices, quad = 4.
Face byte size = `face_count × 6 bytes` (3 `rb16()` calls per face).
Total model byte size = `(vertex_count × 10) + (face_count × 6) + texture_size`.

**Known .MNS models:**
| File | Size | Name |
|------|------|------|
| ANTMAN.MNS | 53,768 | Giant Ant |
| BIGWORM.MNS | 53,784 | Giant Worm |
| BORKETH.MNS | 67,644 | Borketh (named NPC?) |
| CHAOS.MNS | 88,572 | Chaos creature |
| DRA_ZOM.MNS | 83,508 | Dragon Zombie |
| GHOST.MNS | 48,840 | Ghost |
| GOLEM.MNS | 48,140 | Stone Golem |
| H_HOUND.MNS | 46,364 | Hell Hound |
| SCORPION.MNS | — | Scorpion |
| MUMMY.MNS | — | Mummy |
| SKELETON.MNS | — | Skeleton |
| SPIDER.MNS | — | Spider |
| D_RED.MNS | — | Red Dragon |
| D_GOLD.MNS | — | Gold Dragon |
| D_SILVER.MNS | — | Silver Dragon |
| GRN_DRA.MNS | — | Green Dragon |
| MINI_DRA.MNS | — | Mini Dragon |
| RED_DRA.MNS | — | Red Dragon variant |

Source: `include/nexus_v1_dmdf_model.h`, `src/nexus/nexus_v1_dmdf_model.c`,
`docs/NEXUS_FILE_CLASSIFICATION.md`, `docs/nexus_graphics.md`

### 8.2 In-Memory Creature State

`include/nexus_v1_creatures.h`:
```c
#define NEXUS_MAX_CREATURE_TYPES  64
#define NEXUS_MAX_ACTIVE_CREATURES 128

typedef struct {           /* Static type definition */
    char name[32];
    char model_file[32];   /* e.g. "SCORPION.MNS" */
    int health, attack, defense, speed;
    int experience_value;
    int model_index;       /* index into engine->models[] */
} Nexus_CreatureType;

typedef struct {            /* Active instance in dungeon */
    int type_index;
    int health;
    int x, y;
    int facing;           /* 0=N 1=E 2=S 3=W */
    int alive;
    int state;           /* 0=idle 1=patrol 2=chase 3=attack 4=flee */
    int ai_timer;
} Nexus_Creature;

typedef struct {
    Nexus_CreatureType types[NEXUS_MAX_CREATURE_TYPES];
    int type_count;
    Nexus_Creature active[NEXUS_MAX_ACTIVE_CREATURES];
    int active_count;
} Nexus_V1_CreatureManager;
```

AI state machine: idle → patrol → chase (when party within 3 tiles) →
attack (when party within 1 tile). Speed modifies chase tick interval.
Source: `src/nexus/nexus_v1_creatures.c`, `docs/nexus_ai_scripting.md`

### 8.3 Creature Format Status

| Sub-format | Status | Evidence |
|-----------|--------|---------|
| DMDF header/magic | ✅ Parsed | `nexus_v1_dmdf_is_valid()` |
| DMDF vertex data | ✅ Parsed | `nexus_v1_dmdf_load()` loop |
| DMDF face indices | ✅ Parsed | 3 rb16 per face triangle |
| DMDF texture data | ⚠️ Space allocated | No decompression implemented |
| Per-level creature placement | ❌ NOT PARSED | No spawn records from DGN |
| MNS file discovery | ⚠️ Manual | Listed in NEXUS_FILE_CLASSIFICATION.md |

---

## 9. Sound Format

### 9.1 CD-DA Music Tracks

Standard: **Red Book Audio CD-DA**
- Sample rate: 44.1 kHz, stereo, 16-bit
- Tracks 2–9 on the Saturn CD
- 8 tracks cover 16 levels: **2 levels per track**

| Track | Levels | File (CUE) |
|-------|--------|-----------|
| 2 | 0, 1 | Track 2 AUDIO |
| 3 | 2, 3 | Track 3 AUDIO |
| 4 | 4, 5 | Track 4 AUDIO |
| 5 | 6, 7 | Track 5 AUDIO |
| 6 | 8, 9 | Track 6 AUDIO |
| 7 | 10, 11 | Track 7 AUDIO |
| 8 | 12, 13 | Track 8 AUDIO |
| 9 | 14, 15 | Track 9 AUDIO |

Track switching implemented in `nexus_v1_game.c:nexus_v1_cd_track_for_level()`.
Actual CD-DA playback: **STUB** (logged, SDL_mixer not yet wired).

Source: `docs/nexus_audio_format.md`, `nexus_v1_engine.c`

### 9.2 Per-Level SFX Banks — SNDLEV\*.SAL

- 16 files: `SNDLEV00.SAL` through `SNDLEV15.SAL`
- Sizes: 290,000–460,000 bytes per level
- **Format: UNKNOWN.** Not PCM. Likely compressed sample format.
  Size suggests either ADX (Sega ADPCM, ~4:1 compression) or a custom
  framing format for the Saturn SCSP sound chip
- `nexus_sound_load_level()` accepts raw `sal_data` pointer but does not
  parse the stream
- **Status:** STUB — signature confirmed from file size, no decoder exists

### 9.3 Event Maps — SNDLEV\*.MAP

- 16 files: `SNDLEV00.MAP` through `SNDLEV15.MAP`
- Size: 66–90 bytes each
- **Format: UNKNOWN.** 66–90 bytes at 20 events × 2 fields (ID + offset?)
  ≈ 82 bytes suggests a table: `uint8_t event_id, uint8_t bank, uint16_t offset`
  or `uint16_t sample_offset, uint16_t size` per event
- Source: `docs/nexus_audio_format.md`, `src/nexus/nexus_v1_sound.c`

### 9.4 Sound Driver — SDDRVS.TSK

- File: `SDDRVS.TSK` (26 KB) — "Sound DRiVerS TaSK"
- Saturn SH-2 executable task/binary
- Handles: SNDLEV\*.SAL playback, CD audio track management, ADX/PCM routing
- **Status:** NOT REVERSED — no disassembly in repository

### 9.5 FMV Audio — DMV\*.AVI

| File | Size | Description |
|------|------|-------------|
| DMV0.AVI | 34 MB | Intro cutscene |
| DMV1.AVI | 28 MB | Mid-game or alternate ending |
| DMV2.AVI | 39 MB | Ending cutscene |

Format: Saturn AVI (custom proprietary codec, not standard AVI).
Audio stream embedded in AVI container. **Status:** NOT IMPLEMENTED.
Source: `docs/NEXUS_FILE_CLASSIFICATION.md`, `docs/nexus_audio_format.md`

### 9.6 Sound Event API

`include/nexus_v1_sound.h` defines a sound event enum and engine:
```c
enum {
    NEXUS_SFX_NONE = 0,
    NEXUS_SFX_FOOTSTEP = 1,
    NEXUS_SFX_DOOR_OPEN = 2,
    NEXUS_SFX_DOOR_CLOSE = 3,
    NEXUS_SFX_ATTACK_HIT = 4,
    NEXUS_SFX_ATTACK_MISS = 5,
    NEXUS_SFX_CHAMPION_HURT = 6,
    NEXUS_SFX_CREATURE_DEATH = 7,
    NEXUS_SFX_CREATURE_ATTACK = 8,
    NEXUS_SFX_SPELL_CAST = 9,
    NEXUS_SFX_SPELL_IMPACT = 10,
    NEXUS_SFX_PICKUP_ITEM = 11,
    NEXUS_SFX_DROP_ITEM = 12,
    NEXUS_SFX_STAIRS = 13,
    NEXUS_SFX_TELEPORT = 14,
    NEXUS_SFX_ALARM = 15,
    NEXUS_SFX_PIT_FALL = 16,
    NEXUS_SFX_MENU_SELECT = 17,
    NEXUS_SFX_MENU_CONFIRM = 18,
    NEXUS_SFX_MENU_CANCEL = 19,
    NEXUS_SFX_GOLD_PICKUP = 20
};
```

API: `nexus_sound_init()`, `nexus_sound_load_level()`,
`nexus_sound_play()`, `nexus_sound_play_idx()`, `nexus_sound_cd_track()`,
`nexus_sound_music_fade()`, `nexus_sound_set_sfx()`, `nexus_sound_set_music()`.

### 9.7 Sound Format Status

| Sub-format | Status | Evidence |
|-----------|--------|---------|
| CD-DA track switching | ✅ API + logic | `nexus_v1_cd_track_for_level()` |
| CD-DA playback | ❌ STUB | Logged, SDL_mixer not wired |
| SNDLEV\*.SAL decoding | ❌ STUB | Loaded raw, not parsed |
| SNDLEV\*.MAP parsing | ❌ STUB | 66-90 B event table, no decoder |
| SDDRVS.TSK disassembly | ❌ NOT DONE | 26 KB binary |
| DMV\*.AVI audio | ❌ NOT DONE | No AVI parser |

---

## 10. Graphics / Model Format

### 10.1 DMDF (Creature Models) — \*.MNS

Covered in §8.1 above. DMDF stores:
- Nexus_DMDFVertex array (int16 x,y,z + normal + uv)
- Face index arrays (uint16_t triangle/quad indices)
- Embedded BITMAP texture data (format: Saturn VDP1 compressed)

Firestaff reimplementation: loads 3D mesh into `Nexus_V1_Model`, transforms
via `nexus_v1_math3d.c`, rasterizes via `nexus_v1_rasterizer.c`.

Source: `include/nexus_v1_dmdf_model.h`, `docs/nexus_graphics.md`

### 10.2 3D Rasterizer (`nexus_v1_rasterizer.c/h`)

Software rasterizer for the Nexus viewport. Reads polygons from DMDF meshes
and rasterizes with texture-mapped triangles. Z-buffer for depth sorting.
Framebuffer: palette-indexed (Nexus_Framebuffer).

Pipeline:
1. Transform vertices with camera matrix (`nexus_v1_math3d.c`)
2. Project to 2D via perspective divide
3. Depth sort transparent faces (painter's algorithm)
4. Rasterize with texture sampling using DMDF u,v coords

Source: `docs/nexus_graphics.md`, `src/nexus/nexus_v1_rasterizer.c`

### 10.3 Math3D (`nexus_v1_math3d.c/h`)

Vector/matrix library:
```c
Vec3  { float x, y, z; }
Vec4  { float x, y, z, w; }
Matrix4x4 { float m[16]; }  /* column-major, row-indexed get/set }
```

Operations: identity, multiply, translate, rotate, transform_vec3,
transform_vec4, perspective projection.

### 10.4 UI / Title Surfaces (`nexus_v1_ui_surfaces.c/h`)

- TITLE.BIN (110 KB) — title screen background surface
- TITLE.CG (164 KB) — title screen color graphics
- LOGOBG.DG2 (71 KB) — logo background
- STABG.BIN (52 KB) — status area background
- SWTCHR.BIN (38 KB) — switch/lever graphics

Format: likely Saturn VDP2 background layers (2bpp/4bpp indexed).
No dedicated parser in current source — loaded raw via ISO reader
and treated as opaque byte buffers.

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`, `src/nexus/nexus_v1_ui_surfaces.c`

### 10.5 Item Icon Set — ITEM.IBS

- File: `ITEM.IBS` (98 KB) — item icon/bitmap set
- Likely a Saturn VDP1 BITMAP-strip of item icons
- Format: UNKNOWN — likely 2bpp or 4bpp indexed bitmap rows
- No parser in current source

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`

### 10.6 Graphics Format Status

| Sub-format | Status | Evidence |
|-----------|--------|---------|
| DMDF model loading | ✅ Header+vertices+faces | `nexus_v1_dmdf_load()` |
| DMDF texture BITMAP | ⚠️ Allocated | No decompression implemented |
| 3D rasterization | ✅ Implemented | `nexus_v1_rasterizer.c` |
| Math3D transforms | ✅ Implemented | `nexus_v1_math3d.c` |
| TITLE.BIN/CG surfaces | ⚠️ Raw read only | No VDP2 parser |
| ITEM.IBS icons | ❌ NOT PARSED | Format unknown |
| VDP1 texture decompression | ❌ NOT IMPLEMENTED | No VDP1 decompressor |
| Minimap rendering | ❌ NOT IMPLEMENTED | SMAP\*.BIN not parsed |

---

## 11. Save / Persistence Format

### 11.1 Firestaff Native Format

Nexus V1 Phase 6 implements **save/load via a Firestaff-native binary format**.
The original Saturn memory card format is undocumented — no import path exists.

Format: `NEXUS_SAVE_MAGIC = 'FNXS'`, little-endian, version 2.

**Header** (`include/nexus_v1_save.h`):
```c
#define NEXUS_SAVE_MAGIC   0x53584E46U  /* 'FNXS' */
#define NEXUS_SAVE_VERSION 2

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t header_size;
    uint32_t data_size;
    uint32_t champion_data_size;  /* v2+ */
    uint32_t world_data_size;    /* v2+ */
    uint32_t game_time;           /* accumulated ticks */
    uint32_t crc32;               /* CRC-32 of data section */
    int32_t  current_level;       /* 0–15 */
    int32_t  party_x, party_y;
    int32_t  party_dir;           /* 0=N 1=E 2=S 3=W */
    uint32_t state_hash;           /* FNV-1a world hash at save time */
    char     description[32];
} Nexus_V1_SaveHeader;
```

CRC-32 uses zlib polynomial (0xEDB88320), same as ReDMCSB SAVEHEAD.C F0429.
Game time: 32-bit accumulated tick counter (600 ticks/second, matching DM1).
World hash: 64-bit FNV-1a seeded with per-subsystem constants, same approach
as DM1 F0029 but with Nexus-specific seed values.

Platform paths: `~/.firestaff/saves/nexus/` (Unix/macOS), AppData (Windows).
Atomic write: `save.tmp` → `fsync` → rename (no partially-written saves).

Source: `src/nexus/nexus_v1_save_load.c`, `include/nexus_v1_save.h`

### 11.2 Save Data Sections (v2)

| Section | Content | Serialization |
|---------|---------|---------------|
| `champion_data` | All 24 champion structs + party state | Binary blob, `champion_data_size` |
| `world_data` | All 16 level objects + door/teleporter/stairs registry | Binary blob, `world_data_size` |

### 11.3 Slot Manager

`Nexus_V1_SaveManager` manages 8 save slots (`NEXUS_SAVE_MAX_SLOTS = 8`).
Scan on init: read first 48 bytes of each `.nxs` file and populate slot table.
No probe destroys loaded data — first 48 bytes only, non-destructive.

`nexus_v1_save_probe()`: reads header, returns static reason string for any
load failure (magic/version/CRC/unknown variant). Never returns NULL or "".

### 11.4 Deterministic State Hash (World Hash)

FNV-1a 64-bit hash of world state. Components:
- Party position (level, x, y, dir)
- Foot step counter
- Per-creature state (type, position, health, alive)
- Per-item floor state (position, item_id, quantity)
- Per-square door state (open/closed)

Source: `src/nexus/nexus_v1_world.c`, `docs/nexus_save_format.md`

### 11.5 Save Format Status

| Sub-format | Status | Evidence |
|-----------|--------|---------|
| Native save header | ✅ v2 format | `nexus_v1_save.h` |
| CRC-32 integrity | ✅ Implemented | `crc32_update()` |
| Slot manager | ✅ 8 slots + browse | `nexus_v1_save_load.c` |
| Champion serialization | ✅ Binary blob | `nexus_v1_save_load.c` |
| World serialization | ✅ Level objects + registries | `nexus_v1_save_load.c` |
| State hash (FNV-1a) | ✅ Implemented | `nexus_v1_world_hash()` |
| Atomic write (tmp+rename) | ✅ Implemented | `make_dirs()` + rename |
| Original Saturn format | ❌ NOT REVERSED | No memory card disassembly |

---

## 12. Variant Summary Table

| Format / File | Variant(s) | Endianness | Size/Record | Parser Status |
|--------------|------------|-----------|-------------|--------------|
| LEV\*.DGN grid | All 16 levels | BE block container | Structure1B: 0x8000 bytes (64×64×8) | ✅ Parsed (`nexus_v1_dungeon.c`) |
| LEV\*.DGN 3D geometry | All 16 levels | BE (mixed polygons) | variable after Structure1B | ❌ NOT PARSED |
| \*.MNS DMDF | All creature models | BE uint32/16 | 46–89 KB each | ✅ Parsed (header+verts+faces) |
| SNDLEV\*.SAL | Per level | Unknown (compressed?) | 290–460 KB | ❌ NOT PARSED |
| SNDLEV\*.MAP | Per level | Unknown | 66–90 B | ❌ NOT PARSED |
| SDDRVS.TSK | Single binary | SH-2 big-endian | 26 KB | ❌ NOT REVERSED |
| DMV\*.AVI | 3 cutscenes | Saturn AVI codec | 28–39 MB | ❌ NOT PARSED |
| SLEV\*.BIN | Per level | Unknown declarative | 2–12 KB | ❌ NOT PARSED |
| SMAP\*.BIN | Per level | Unknown bitmap | 17–30 KB | ❌ NOT PARSED |
| FONT256.S2D | Single | BE with SCR header | 24 KB | ✅ Parsed (Saturn SCR loader) |
| FACE.BIN | Single | Unknown bitmap | 44 KB | ❌ NOT PARSED |
| ITEM.IBS | Single | Unknown bitmap | 98 KB | ❌ NOT PARSED |
| TITLE.BIN | Single | Unknown surface | 110 KB | ⚠️ Raw read only |
| TITLE.CG | Single | Unknown (color graphics) | 164 KB | ⚠️ Raw read only |
| STABG.BIN | Single | Unknown surface | 52 KB | ⚠️ Raw read only |
| SWTCHR.BIN | Single | Unknown graphics | 38 KB | ⚠️ Raw read only |
| DM.BIN | Single | SH-2 big-endian | 542 KB | ⚠️ ISO-loadable, not parsed |
| 0DMSTRT.BIN | Single | Binary data | 39 KB | ⚠️ ISO-loadable, not parsed |
| Nexus save (native) | Firestaff v2 | LE binary | variable | ✅ Full R/W |
| CD-DA tracks | 8 tracks (2–9) | Red Book Audio | 44.1 kHz/stereo/16b | ⚠️ Track switch only |
| Per-level CD track map | 16 levels | — (2/track) | — | ✅ `nexus_v1_cd_track_for_level()` |

---

## 13. Big-Endian Conversion Swizzles

Every multi-byte parser uses these inline helpers (never portable memcpy):

```c
static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint16_t rb16(const uint8_t *p) { return ((uint16_t)p[0]<<8)|p[1]; }
static int16_t rbs16(const uint8_t *p) { return (int16_t)rb16(p); }
```

Used in: `nexus_v1_dungeon.c`, `nexus_v1_dmdf_model.c`, `nexus_v1_iso_reader.c`.
The ISO reader also uses `r32le()` for **little-endian** LBA reads from the
ISO PVD (ISO 9660 standard uses LSB-first sector addresses). This is the only
exception: ISO sector LBA is little-endian on disc.

---

## 14. Key Differences vs DM1 Format Landscape

| Aspect | DM1 V1 | Nexus V1 | Implication |
|--------|--------|---------|-------------|
| Dungeon file | DUNGEON.DAT (~33 KB, all levels inline) | LEV\*.DGN (4.3 MB, 130× larger) | 3D geometry pre-baked into each level |
| Sensor system | Hardwired F0267/F0268 compile-time | SDDRVS.TSK + SLEV\*.BIN declarative | New dungeons possible without recompile |
| Levels | 8 (D0–D7), variable grid 16–30 | 16 (LEV00–LEV15), fixed 32×32 | Larger dungeons, fixed buffer sizes |
| Creature models | 2D sprites in GRAPHICS.DAT | DMDF .MNS 3D polygon meshes | Full 3D rendering pipeline needed |
| Audio | SND3.DAT (global, ~28 KB PCM) | Per-level .SAL (290–460 KB each) | Memory-budget strategy differs |
| Sound driver | None (PC BIOS/AdLib) | SDDRVS.TSK (26 KB SH-2 task) | Saturn audio is task-based |
| Text encoding | ASCII | Shift-JIS | Font + encoding conversion needed |
| Overworld | None | Yes (separate map) | World model split between dungeon + outdoor |
| Language | EN | Japanese only | All text requires SJIS→UTF-8 conversion |
| CD audio | None (sequenced music) | CD-DA Red Book 8 tracks | Platform CD-audio support needed |
| FMV | None | 3× DMV\*.AVI cutscenes (101 MB) | AVI demux + codec work |

Source: `docs/nexus_data.md` DM1 comparison tables, `docs/nexus_dungeon.md`

---

## 15. Immediate Gaps (Before Phase 3+)

### Priority 1 — Geometry & Models
- **DGN 3D geometry parser** — wall/floor/ceiling mesh extraction from the
  post-grid blob. This is the single largest rendering gap. Without it,
  only the 2D grid is available (no 3D walls/floors/doors).
- **VDP1 texture decompression** — DMDF texture BITMAP data is compressed
  in the Saturn VDP1 format. No decompressor exists.

### Priority 2 — Audio
- **SNDLEV\*.SAL decoder** — needs ADX identification or PCM format detection.
  Saturn SCSP ADX is a known Sega format (4:1 ADPCM, block size 20/80 bytes).
- **SNDLEV\*.MAP parser** — 66–90 byte table, simple event→sample mapping.

### Priority 3 — Scripting
- **SDDRVS.TSK VM** — the declarative script system cannot be factored without
  disassembly of either SDDRVS.TSK or SLEV\*.BIN format. SLEV\*.BIN may be
  disassembly-friendly if it uses a simple record structure.

### Priority 4 — Text
- **Glyph blit** — single-character blit to framebuffer. FONT256.S2D is
  loaded; the rendering step (blit + layout) needs implementation.
- **JIS X 0208 kanji** — full double-byte lookup table for 6,965 kanji.

### Priority 5 — Supplementary
- **SMAP\*.BIN parser** — likely 2bpp bitmap strip. Low priority since it
  comes after dungeon rendering is functional.
- **ITEM.IBS parser** — likely a VDP1 icon strip for in-game UI.
- **FACE.BIN portraits** — important for champion selection UI.

---

## 16. Source Evidence Index

| Evidence | File | Description |
|----------|------|-------------|
| Grid loading | `src/nexus/nexus_v1_dungeon.c` | Two-layout grid parser, `rb16() & 0x1F` |
| ISO9660 reading | `src/nexus/nexus_v1_iso_reader.c` | Full PVD/directory parsing |
| DMDF parsing | `src/nexus/nexus_v1_dmdf_model.c` | Magic, header, vertices, faces |
| Font loading | `src/nexus/nexus_v1_saturn_font.c` | SCR header + glyph extraction |
| SJIS conversion | `src/nexus/nexus_v1_text.c` | ASCII + katakana UTF-8 |
| Champion roster | `src/nexus/nexus_v1_champions.c` | 8-champion roster table |
| Inventory defs | `src/nexus/nexus_v1_inventory.c` | Full item catalog (DM1-compatible) |
| Creature roster | `src/nexus/nexus_v1_creatures.c` | Type defs, AI state machine |
| Sound stub | `src/nexus/nexus_v1_sound.c` | API surface, 21 event enum entries |
| Save/load | `src/nexus/nexus_v1_save_load.c` | Full native format, CRC-32, slots |
| World hash | `src/nexus/nexus_v1_world.c` | FNV-1a 64-bit |
| Disc manifest | `docs/NEXUS_FILE_CLASSIFICATION.md` | 137-file inventory |
| Audio spec | `docs/nexus_audio_format.md` | CD-DA + SAL + MAP layout |
| Text spec | `docs/nexus_text.md` | Font + SJIS + layout gaps |
| Dungeon fmt | `docs/nexus_dungeon.md` | Level sizes + format overview |
| Square fmt | `docs/nexus_squares.md` | Type encoding + SDDRVS.TSK vs DM1 |
| Save fmt | `docs/nexus_save_format.md` | Native format + gaps |
| Audio gaps | `docs/nexus_sfx.md` | No SFX implementation documented |
| Inventory fmt | `docs/nexus_inventory.md` | 30-slot model, equipment slots |
| Phase 0 | `docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md` | Provenance gate (disc absent) |
| Phase 1 | `docs/source-lock/nexus_v1_phase1_boot_H2318.md` | Boot profile (boot profile split) |
| Phase 4 | `docs/source-lock/nexus_v1_phase4_rendering_pipeline_H0357.md` | Rasterizer + palette |
| Phase 7 | `docs/source-lock/nexus_v1_phase7_verification_suite_H0357.md` | Verification probes |
| ReDMCSB | `~/.openclaw/data/firestaff-redmcsb-source/` | Source-lock reference |
