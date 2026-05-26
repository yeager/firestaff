# DM2 V1 Phase 2 — Data Formats: Source-Lock Document

**Cron task:** `DM2_V1_DataFormats_H2254`
**Committed:** 2026-05-26T23:03 Europe/Stockholm
**Sources:** SKULL.ASM (sha256:a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099), skproject gbsphenx/skproject HEAD a962896 (SkGlobal.h, SkWinCore.cpp, DME.h, defines.h, SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt)

---

## Scope

This document source-locks the DM2 V1 dungeon and graphics data format landscape. It does **not** cover rendering, movement, or gameplay mechanics — those are Phase 3+. It covers:

1. GDAT category system (240 categories, DM2 vs DM1's 29)
2. Dungeon file layout and records (DUNGEON.DAT)
3. GRAPHICS.DAT structure and GDAT2 field codes
4. Item/Weapon/Cloth/Scroll/Misc records (dbWeapon=5, dbCloth=6, dbScroll=7, dbMisc=10)
5. Actuator types (floor/wall sensors → effectors)
6. Door/teleporter/pit/ornate special-square types
7. Text retrieval system (GDAT text categories, font rendering)
8. Variant/platform differences (DOS EN/FR/DE, PC-9821, Sega CD, Beta, Demo, TQ)
9. Save game format (SKSAVE*.DAT, SUPPRESS compression)
10. Status and known gaps

---

## 1. GDAT Category System (0xF0 vs DM1's 0x1D)

DM2 extends the GDAT category system from 29 categories (0x1D) to 240 categories (0xF0).

Source: SkGlobal.h:636 `GDAT_CATEGORY_LIMIT` (DM2) vs SkGlobal.h:638 (original)

### Key DM2-specific GDAT categories

| Category name | Value | Purpose |
|---|---|---|
| `GDAT_CATEGORY_SPELL_DEF` | 0x02 | Spell definitions (custom spells up to 255 vs DM1's 34) |
| `GDAT_CATEGORY_CREATURES` | 0x0D | Creature type data |
| `GDAT_CATEGORY_DOORS` | 0x0E | Door properties (strength, color keys, mirror flag) |
| `GDAT_CATEGORY_WEAPONS` | 0x10 | Extended weapon data with projectile flags |
| `GDAT_CATEGORY_CHAMPIONS` | 0x16 | Champion NPC data (sounds: attack, shoot, get_hit, eat/drink, death, bump_wall) |
| `GDAT_CATEGORY_ENVIRONMENT` | 0x17 | Environment settings |
| `GDAT_CATEGORY_TELEPORTERS` | 0x18 | Teleporter square type |
| `GDAT_CATEGORY_CREATURE_AI` | 0x19 | Per-creature AI behaviors |
| `GDAT_CATEGORY_DIALOG_BOXES` | 0x1A | Dialog box graphics |

DM2 also uses all original categories (text, UI, creatures, items, wall/floor ornaments) plus extended variants.

### GDAT2 internal field codes (SKWin.GDAT2.InternalCodes.txt)

GDAT2 extends field codes beyond the base set used in DM1/CSB:

**Animated ornate fields:**
- `XX 00 00`: Number of cycled animated frames (up to 10)
- Sequence field: animated frame sequence

**Door fields (new in DM2):**
- `0F 00 00`: Door strength
- `04 00 00`: Color key 1 (cyan) — see-through effect
- `0C 00 00`: Color key 2 (dark green) — secondary transparency
- `20 00 00`: Animated mirrored door flag (unused in standard DM2 dungeons)

**Creature drop fields:**
- `5A–64 00 00`: 11 drop slots (vs DM1's single drop)

**Item animation:**
- `06 00 00`: Animation field (e.g. 0x0504 = 4-frame animation)

**Spell missile fields:**
- `09 00 00`, `0D 00 00`: Missile strength

**Potion fields:**
- `05 00 00`: Behavior
- `43 00 00`: Water value
- `4D 00 00`: Spell missile association

**Ambient light fields:**
- `85 00 00`: Default ambient light
- `86 00 00`: Lowest acceptable light level (0–5)
- `87 00 00`: Ambient darkness / sight distance (0 = full light, 8 = dark)

Source: SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt (referenced in docs/dm2_dungeon.md, docs/dm2_special_squares.md, docs/dm2_newfeatures.md)

### GDAT access pattern

SKWIN/SKULLWIN use `c_gdatfile` class for structured data access, indexed by `(category, index, field)` triplet:

```cpp
QUERY_GDAT_TEXT(category, cls2, cls4, buffer)   // text retrieval
QUERY_GDAT_ENTRY_DATA_BUFF(category, cls2, dtImage, ...)  // image/bitmap data
DM2_EXTRACT_GDAT_IMAGE(t_dbidx, ...)             // wall/creature/item graphics
```

Localized text via `s_textLangSel[category][index][field]`. `DM2_EXTENDED_MODE=1` adds `s_imageLangSel` for localized UI graphics.

---

## 2. DUNGEON.DAT Format

### File sizes

| Variant | DUNGEON.DAT | vs DM1 |
|---|---|---|
| DM1 PC 3.4 | 33,357 bytes | baseline |
| DM2 DOS EN (EN/LEGEND) | 39,437 bytes | +18% |
| DM2 Sega CD | 37,957 bytes | +14% |

DM2 is ~18% larger due to: extended creature AI (64 vs 42 slots), additional GDAT categories (240 vs 29), outdoor level data (sky/weather per level), extended spell tables (255 vs 34).

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md

### Overall layout (same as DM1)

DM2 uses the same overall DUNGEON.DAT layout as DM1: header + map descriptors + thing data + text data. It uses the same FTL decompression signature (0x8104 big-endian) as DM1.

```
Compressed header (12 bytes):
  uint16_t signature          — 0x8104 (FTL magic)
  uint32_t decompressed_size  — BE
  uint16_t dungeon_id         —

Dungeon header (44 bytes, same structure as DM1):
  uint16_t ornament_random_seed
  uint16_t raw_map_data_byte_count
  uint8_t  map_count
  uint16_t text_data_word_count
  uint16_t initial_party_location
  uint16_t square_first_thing_count
  uint16_t thing_count[16]    — per-record-type counts

Map descriptors (16 bytes each, same as DM1 MAP descriptor):
  uint16_t raw_map_data_byte_offset
  uint8_t  offset_map_x, offset_map_y
  — bitfield A: level(6) + width-1(5) + height-1(5)
  — bitfield B: wall_ornament(4) + random_wall(4) + floor(4) + random_floor(4)
  — bitfield C: door_ornament(4) + creature_type(4) + unreff(4) + difficulty(4)
  — bitfield D: floor_set(4) + wall_set(4) + door_set0(4) + door_set1(4)

Square-first thing index
Thing data records (16 DB record pools)
Text data
```

Source: docs/dm2-v1-dungeon-audit/dm2_dungeon.md, ReDMCSB DEFS.H:985-1116

### DM2 extensions to the dungeon format

**Level type byte (per level):**
```
uint8_t level_type           — 0=OUTDOOR, 1=INDOOR, 2=BUILDING
uint8_t level_width
uint8_t level_height
uint8_t padding
uint16_t offset_low
uint16_t offset_high
```

DM1 had no level type byte. DM2's `dm2_v1_dungeon_loader.c` parses this from the 8-byte level entry.

**Maximum levels:** DM2_V1_MAX_LEVELS = 30 (vs DM1's ~16 maps from uint8 map_count)

**Per-square (2 bytes):**
```
uint16_t square_type & 0x1F  — 5-bit tile type (same as DM1)
```

Source: include/dm2_v1_dungeon_loader.h, dm2_v1_dungeon_loader.c

### Level types

| Value | Name | Description |
|---|---|---|
| 0 | `DM2_LEVEL_OUTDOOR` | Sky, ground, trees, buildings; outdoor combat |
| 1 | `DM2_LEVEL_INDOOR` | Standard first-person dungeon (similar to DM1) |
| 2 | `DM2_LEVEL_BUILDING` | Multi-floor structures within outdoor areas |

Source: include/dm2_v1_dungeon_loader.h:19-21

### SKWIN/SKULLWIN dungeon loading

The `READ_DUNGEON_STRUCTURE` routine in skproject loads DM2 dungeons. The DungeonData struct in include/dm2_v1_dungeon_loader.h stores:
- `level_count`, `level_types[]`, `level_widths[]`, `level_heights[]`, `level_offsets[]`
- `raw_data` / `raw_size` — full decompressed dungeon data
- `sky_texture_index`, `weather_zone_count` — outdoor extensions

Source: docs/dm2_dungeon_files.md, docs/dm2_dungeon_overview.md

---

## 3. GRAPHICS.DAT Structure

### File sizes

| Game | GRAPHICS.DAT | vs DM1 |
|---|---|---|
| DM1 PC 3.4 | ~363 KB | baseline |
| DM2 DOS EN | ~8.6 MB | ~24x |
| DM2 PC-9821 | ~12 MB | ~33x |

DM2's GRAPHICS.DAT is ~24x larger due to:
- Outdoor environment art (sky, ground, trees, buildings)
- New DM2-specific creature artwork
- New UI elements (champion sheets, shops, maps)
- Additional animation frames for items and creatures
- Day/night palette variants

Source: docs/dm2_platform_data.md, docs/dm2_graphics.md

### GDAT-backed image categories

DM2 GRAPHICS.DAT uses the same GDAT indexing system as DM1 but with 240 categories vs 29. The key image categories:

| Category | Content |
|---|---|
| `GDAT_CATEGORY_WALL_ORNAMENTS` | Wall decorations, switches, torches |
| `GDAT_CATEGORY_FLOOR_ORNAMENTS` | Floor decorations, pressure plates |
| `GDAT_CATEGORY_CREATURES` | Creature sprites (64 AI types) |
| `GDAT_CATEGORY_ITEMS` | Item sprites |
| `GDAT_CATEGORY_WEAPONS` | Weapon animations with projectile flags |
| `GDAT_CATEGORY_DOORS` | Door animations (4-6 keyframes with tweening) |
| `GDAT_CATEGORY_TELEPORTERS` | Teleporter chip graphics |
| `GDAT_CATEGORY_ENVIRONMENT` | Sky/ground/building outdoor assets |
| `GDAT_CATEGORY_DIALOG_BOXES` | Dialog box layouts |
| `GDAT_CATEGORY_INTERFACE` | Champion sheets, HUD elements |

Source: docs/dm2_graphics.md, docs/dm2_newfeatures.md

### Image compression formats

DM2 supports three image compression schemes in GRAPHICS.DAT:

| Format | Description | Usage |
|---|---|---|
| `IMG3` (underlay) | 4-bit nibble encoding, two pixels per byte | Simple wall textures |
| `IMG3` (overlay) | 4-bit nibble with overlay mask | Door frames, panel overlays |
| `IMG9` | 9-bit per-pixel encoding | Complex walls with doors/panels |

DM2 uses both 8-bit (c_pixel256) and 16-bit (c_pixel16) surfaces:
- Base wall layers: 8-bit IMG3
- Door overlay frames: 16-bit c_pixel16 (higher-fidelity metallic accents)
- Weather overlay: 16-bit src → 8-bit dest via blitline_48

Source: docs/dm2_graphics.md §4

### Color depth verdict

**DM2 is still VGA 256-color (8-bit palette-indexed), not 16-bit.** Despite the c_pixel16 type existing for compositing effects, the core viewport uses c_pixel256 (8-bit palette indices). The 16-bit surfaces are used only for weather/fog overlay and sprite alpha blending passes. This matches DM1's architecture.

Source: docs/dm2_graphics.md §1 — c_gfx_main.cpp declares `c_pixel256 pixel[ORIG_SWIDTH * ORIG_SHEIGHT]`

### Palette system (extended vs DM1)

DM2 has a richer multi-palette system than DM1's static 256 DAC:

| Palette type | Description |
|---|---|
| `pal16to256ptr` | 16-color subset → 256-color lookup table |
| `small_palette[PAL16]` | 16-color context palette (scene-local) |
| `glbl_pal1` / `glbl_pal2` | Two global palette sets for scene transitions |
| `DM2_SELECT_PALETTE_SET(i16)` | Runtime palette set switching |

DM1 palette: static EGA/VGA 256 DAC loaded at startup, no runtime switching.
DM2 palette: dynamic multi-palette system allowing scene-specific color palettes without full palette reloads.

Source: docs/dm2_graphics.md §2.3

### GDAT image decoding

Image decoding functions in c_gfx_decode.h:
- `decode_img3_underlay(pixel*, rect*)` — base wall layer
- `decode_img3_overlay(pixel16*, pixel*, rect*)` — overlay layer (16-bit src for door frames)
- `decode_img9` — 9-bit complex wall graphics

DM2 allocates wall/creature graphics with `DM2_ALLOC_NEW_BMP(dbidx, width, height, res)` and frees with `DM2_FREE_PICT_ENTRY(gfx)`. Graphics are cached on demand.

Source: docs/dm2_walls.md §3

---

## 4. Item Records

DM2 stores items in 4 main database types (db categories):

| DB Type | Constant | Description |
|---|---|---|
| dbWeapon | 5 | Swords, guns, bombs, thrown — has charges, damage stats |
| dbCloth | 6 | Clothing/Armor — has armor strength, charges |
| dbScroll | 7 | Scrolls — spell items (DM2/CSB new) — has spell association |
| dbMiscellaneous_item | 10 | Potions, flasks, tools, quest items — has charges/compass |

Source: SkWinCore.cpp:847-852 (dbWeapon through dbMiscellaneous_item switch handling), docs/dm2_items.md

### Item record structure

Each item has a 16-bit record with an `ItemType()` accessor (7-bit field, value 0x00–0x7F):

```cpp
U8 ItemType() const { return (U8)(w2 & 0x007F); }
void ItemType(Bit16u val) { w2 = (w2 & ~0x007F) | (val & 0x007F); }
```

Source: DME.h:567-568, 588-589, 612-613, 728-729, SkWinCore.cpp:41441-41465

### Item charges

| Type | Field | Accessor |
|---|---|---|
| Weapon | Charges | `Weapon::Charges()` |
| Cloth | Charges | `Cloth::Charges()` |
| Misc | Compass (reused) | `Miscellaneous_item::Compass()` |

Charge consumption tracked per item. Weapons consume on use; armor/cloth on damage absorption; misc items on consumption.

Source: SkWinCore.cpp:2211-2246

### Extended item fields (DM2 vs DM1)

| Field index | Content | Notes |
|---|---|---|
| `06 00 00` | Animation | New in DM2 (e.g. 0x0504 = 4-frame) |
| `14 00 00` | Mana bonus | New in DM2 |
| `15 00 00` | Luck bonus | New in DM2 |
| `33 00 00` | Speed bonus | New in DM2 |
| `34 00 00` | Weight per charge | New in DM2 |
| `05 00 00` | Potion behavior | DM2-specific |
| `43 00 00` | Water value | DM2-specific |
| `4D 00 00` | Spell missile association | DM2-specific |

Source: docs/dm2_items.md §6, dm2_newfeatures.md

### Item affinity system (DM2)

```cpp
typedef enum {
    DM2_ITEM_MAGIC  = 0,   // traditional magic items
    DM2_ITEM_TECH   = 1,   // tech items (guns, bombs)
    DM2_ITEM_HYBRID = 2,   // requires both tech_level AND magic_level
} DM2_ItemAffinity;
```

Tech item check: `champion_tech >= item->tech_level`
Hybrid item check: `champion_tech >= item->tech_level && champion_magic >= item->magic_level`

Source: docs/dm2_items.md §7, c_item.cpp

### Ranged weapon types (DM2 new)

DM2 adds crossbow, gun (tech weapon), bomb (thrown explosive):
- `GDAT_ITEM_WEAPON_PROJECTILE_FLAG` (field 05)
- Missile strength fields `09 00 00` / `0D 00 00`
- Knock sound (85 00 00)

Source: docs/dm2_newfeatures.md §4

---

## 5. Actuator System

Actuators are the **output/effect side** of the DM2 trigger system (separate from DM1's hardwired door/trap behavior). The sensor (floor/wall type) is separate from the actuator (effect).

### Record types

| Record type | Size | Description |
|---|---|---|
| `DB_CATEGORY_SIMPLE_ACTUATOR` (0x02) | 4 bytes | Simple relay/wire |
| `DB_CATEGORY_ACTUATOR` (0x03) | 8 bytes | Full actuator with type/target/data |

Source: docs/dm2_actuators.md

### Actuator types (key subset)

**Door control:**
| Type | Name | Effect |
|---|---|---|
| ACTUATOR_TYPE_X01 | 0x01 | DM1 retro wall switch |
| ACTUATOR_TYPE_WALL_SWITCH | 0x18 | Standard single-state wall switch |
| ACTUATOR_TYPE_2_STATE_WALL_SWITCH | 0x17 | Toggle: alternate between two states |
| ACTUATOR_TYPE_PUSH_BUTTON_WALL_SWITCH | 0x46 | Push-button (release to activate) |
| ACTUATOR_TYPE_KEY_HOLE | 0x1A | Activated by applying a specific key item |
| ACTUATOR_TYPE_SWITCH_SIGN_FOR_CREATURE | 0x26 | Switch visible to creature AI |

**Shooter/trap:**
| Type | Name | Effect |
|---|---|---|
| 0x08 | MISSILE_SHOOTER | Fires a missile projectile |
| 0x09 | WEAPON_SHOOTER | Fires a weapon (e.g. arrow) |
| 0x0E | ITEM_SHOOTER | Shoots a physical item |

**Logic/wiring:**
| Type | Name | Effect |
|---|---|---|
| 0x1D | COUNTER | Counts down; fires at zero |
| 0x1E | TICK_GENERATOR | Periodic ticks for timing |
| 0x20 | RELAY_1 | Passes activation through |
| 0x16 | CROSS_MAP | Triggers actuator on different dungeon map |
| 0x43 | INVERSE_FLAG | Inverts bit flag before passing |
| 0x44 | TEST_FLAG | Fires only if flag is in specific state |

**Spawn:**
| Type | Name | Effect |
|---|---|---|
| 0x2E | CREATURE_GENERATOR | Spawns creature at actuator location |
| 0x3C | ITEM_GENERATOR | Generates item at actuator location |
| 0x22 | FLYING_ITEM_CATCHER | Catches/stops a flying item |
| 0x23 | FLYING_ITEM_TELEPORTER | Teleports a flying item |
| 0x47 | ITEM_CAPTURE | Captures item placed on floor |

**Visual:**
| Type | Name | Effect |
|---|---|---|
| 0x2C | ORNATE_ANIMATOR | Plays animation sequence on tile |
| 0x32 | ORNATE_ANIMATOR_2 | Variant ornate animator |
| 0x41 | ORNATE_STEP_ANIMATOR | Animates when party steps on it |
| 0x3F | SHOP_PANEL | Displays shop interface |

**Item:**
| Type | Name | Effect |
|---|---|---|
| 0x03 | ITEM_WATCHER | Watches for specific item type on floor |
| 0x15 | CHARGED_ITEM_WATCHER | Watches for charged/wanded item |
| 0x40 | ITEM_RECYCLER | Destroys items placed on it; optionally generates different items |

**Champion:**
| Type | Name | Effect |
|---|---|---|
| 0x7E | RESURECTOR | Champion resurrection cell |
| 0x7F | CHAMPION_MIRROR | DM1 champion mirror (retro) |

Source: docs/dm2_actuators.md (full tables), defines.h:1182-1204

### Door actuator messages

When an actuator activates a door, it sends one of three action types:
| Action | Value | Meaning |
|---|---|---|
| `ACTMSG_OPEN_SET` | 0x00 | Force door open |
| `ACTMSG_CLOSE_CLEAR` | 0x01 | Force door closed |
| `ACTMSG_TOGGLE` | 0x02 | Toggle current state |

Door state machine via `DoorBit09`:
- 0 = closing direction
- 1 = opening direction
- 2 = mid-state

`DoorBit10` holds remaining animation timer.

Source: docs/dm2_actuators.md §1

---

## 6. Sensor Types (Floor/Wall Input Side)

Sensors detect conditions and activate actuators. Separate from DM1's fused sensor-tile behavior.

### Floor sensors (ACTUATOR_FLOOR_TYPE__*)

| Type | Name | Trigger Condition |
|---|---|---|
| 0x01 | EVERYTHING | Any entity (party, creature, item) steps on it |
| 0x03 | PARTY | Only party members step on it |
| 0x04 | ITEM | An item is dropped or placed on it |
| 0x07 | CREATURE | A creature steps on it |
| 0x08 | ITEM_POSSESSION | Party member possessing specific item steps on it |
| 0x0B | CREATURE_KILLER | Kills creatures that step on it |
| 0x16 | CROSS_MAP | Cross-map teleport trigger |
| 0x1D | COUNTER | Counter reaches zero |
| 0x1E | INFINITE_TICK_GENERATOR | Infinite periodic firing |
| 0x21 | ARRIVAL_DEPARTURE | Party arrives at or departs from location |
| 0x26 | MISSILE_EXPLOSION | Missile lands on it |
| 0x2A | ALCOVE_ITEM | Item placed in alcove |
| 0x2E | PARTY_TELEPORTER | Party teleporter |
| 0x30 | SHOP | Party enters shop |
| 0x3A | CREATURE_ANIMATOR | Animates creatures on the tile |
| 0x3B | ITEM_TELEPORTER | Teleports items |
| 0x49 | ITEM_CAPTURE_FROM_CREATURE | Captures item from creature |

Floor type 0x01 (EVERYTHING) is the universal sensor — fires for any entity.

Source: docs/dm2_sensors.md

### Wall sensors (ACTUATOR_TYPE_*)

| Type | Name | Trigger Condition |
|---|---|---|
| 0x01 | DM1_WALL_SWITCH | DM1 retro wall switch (facing + keypress) |
| 0x17 | 2_STATE_WALL_SWITCH | Toggle switch: on/off state |
| 0x18 | WALL_SWITCH | Standard wall switch (keypress when facing) |
| 0x1A | KEY_HOLE | Key is used on wall cell |
| 0x46 | PUSH_BUTTON_WALL_SWITCH | Push-button style wall switch |
| 0x26 | SWITCH_SIGN_FOR_CREATURE | Switch visible to creatures |

**Activation method:** Player faces the wall and presses a key. DM2 tracks the direction the party is facing.

Source: docs/dm2_sensors.md

---

## 7. Special Square Types

### Teleporter Squares (GDAT_CATEGORY_TELEPORTERS = 0x18)

DM2 introduces a dedicated teleporter GDAT category (0x18), distinct from DM1's generic approach.

Rendered via `DRAW_TELEPORTER_CHIP` at SkWinCore.cpp:10328.

Teleporter scope enum (DME.h:384):
- `SDFSM_CMD_X_TELEPORTER = 4`: Cross-scene teleport
- `SDFSM_CMD_X_ANCHOR = 5`: Anchor in the Sun Clan village

Teleporter sound: field `89 00 00`

Source: docs/dm2_special_squares.md, SkWinCore.cpp:18936-18937

### Ladders

`ACTUATOR_TYPE_SIMPLE_LADDER` (0x1C) exists but is beta-only. Standard ladder triggers use `ACTUATOR_TYPE_LADDER` (0x11).

`DM2_FIND_LADDER_AROUND_PARTY` (SkWinCore.cpp:9060) searches for ladders around party position.

Ladder state encoded in wall ornate GDAT data (0x11 field), not in square type itself.

`GDAT_WALL_ORNATE__IS_LADDER_UP` (field 0x11): Value 1 = ladder going up; absent = ladder going down.

Source: docs/dm2_special_squares.md, defines.h:616, SkWinCore.cpp:9108, 21602

### Other wall ornate types

| Ornate | Value | Description |
|---|---|---|
| `GDAT_WALL_ORNATE__CRYOCELL` | 0x5B | Passive device (shows champion portrait) |
| `GDAT_WALL_ORNATE__IS_WATER_SPRING` | 0x0B | Water spring |
| `GDAT_WALL_ORNATE__IS_REBIRTH_ALTAR` | 0x0C | Used for DM2 beta |
| `GDAT_WALL_ORNATE__SWITCH_ITEM` | 0x0E | Item-triggered switch |
| `GDAT_WALL_ORNATE__OVERLAY` | 0x0F | Shop glass / panel shop overlay |

Source: defines.h:548, 604-605, 610-616, SkWinCore.cpp:13102-13107

### Pit system

DM1 pit squares (C02_ELEMENT_PIT = 2) with imaginary/open/invisible flags carried forward. DM2 extends pit behavior through the actuator system (CREATURE_KILLER floor type 0x0B, pit-damage actuators).

Source: docs/dm2_special_squares.md (DM1 comparison), DM1 ReDMCSB DEFS.H

### Door types

**Normal clan door** (type 0x09)
**Dragon door** (type 0x0A) — second clan door specifically for the Skullkeep Dragon:
- Door Strength field (`0F 00 00`)
- Color key 1: cyan (`04 00 00`) — enables see-through graphics behind door
- Color key 2: dark green (`0C 00 00`) — secondary transparency

**Animated mirrored door** (field `20 00 00`) — like DM1 force field, noted as unused in standard dungeons.

Source: SKWin.GDAT2.InternalCodes.txt (DOOR section), docs/dm2_special_squares.md §8

---

## 8. Text System

DM2 text rendering is a multi-layer system: GDAT-stored bitmapped fonts, encoded text retrieval, and various drawing functions for HUD/dialogs/scroll text/combat feedback.

### Font data

- `Bit8u *_4976_5c0e` — main font bitmap pointer (loaded at runtime from GDAT)
- `GDAT_CATEGORY_JAPANESE_FONT` (0x1C) — kanji/extended font table; iterated at startup to load all glyphs
- `dtImage` — font image data type in GDAT
- `dtWordValue` — character code/value lookup in GDAT

### Text drawing functions

| Function | Description |
|---|---|
| `DRAW_STRONG_TEXT()` | Primary text drawing for HUD and menus (foreground + background fill) |
| `DRAW_VP_RC_STR()` | Viewport-relative string drawing (rectno + GDAT text) |
| `DRAW_LOCAL_TEXT()` | Rect-anchored text for inventory screen |
| `DRAW_TEXT_TO_BACKBUFF()` | Direct-to-backbuffer text rendering |
| `DRAW_SCROLL_TEXT()` | Renders scrolling text / message scrolls |
| `_3929_04e2_DRAW_TEXT_STRINGS()` | Core string layout engine for multi-line text |

### Text retrieval (GDAT)

`QUERY_GDAT_TEXT(category, cls2, cls4, buffer)`:
- `category` — GDAT category
- `cls2` — subclass 2
- `cls4` — data index (e.g. 0x18 = name, 0x06 = food text)

Key text interfaces (from defines.h):
- `GDAT_INTERFACE_CHAR_FOOD_WATER_PANEL` (cls2=0x01) — food/water panel
- `GDAT_INTERFACE_FOOD_TEXT` (cls2=0x06) — food amount
- `GDAT_INTERFACE_WATER_TEXT` (cls2=0x07) — water amount
- `GDAT_INTERFACE_POISON_TEXT` (cls2=0x08) — poison text
- `GDAT_INTERFACE_PLAGUED_TEXT` (cls2=0x09) — plague text

Champion text: `GDAT_CATEGORY_CHAMPIONS` (cls2=heroType, cls4=0x18) — champion names
Creature text: `GDAT_CATEGORY_CREATURES` (cls2, cls4+0x10) — creature names

`DIRECT_QUERY_GDAT_TEXT(cls1, cls2, cls4, buff)` — inline text fetch, no cache

Source: docs/dm2_text.md, docs/dm2_platform_data.md

### Text categories

- `DB_SIZE_TEXT = 0x04`
- `DB_CATEGORY_TEXT = 0x02`

---

## 9. Save Game Format (SKSAVE*.DAT)

DM2 uses single-file saves (`SKSave.dat`) with SUPPRESS compression (bit-level RLE), vs DM1's simpler record-based per-champion `CHAMP.DAT` + per-dungeon `DUNGEON.DAT`.

Source: docs/dm2_save_format.md

### File locations

- `SKSave.dat` (primary) and `SKSave.bak` (backup)
- Located alongside `Dungeon.ftl`
- On save: `.dat` → `.bak`, new `.dat` written
- On load: try `.dat` first, fallback to `.bak`

### Header (42 bytes, `sksave_header_asc`)

```
w0       : version/flags (written as 1 on each save)
b2[34]   : ASCII null-terminated save name (max 34 chars)
w36      : slot index in ASCII + 0x30 (e.g. slot 0 -> 0x30 = '0')
w38      : magic marker 0xBEEF (valid slot indicator)
w40      : magic marker 0xDEAD (valid slot indicator)
```

Slot valid when `w38 == 0xBEEF && w40 == 0xDEAD`.

### Save sections (in order)

1. **Dungeon header** (`DunHeader`, 44 bytes)
2. **Map headers array** (`dunMapsHeaders`, `nMaps << 4` bytes)
3. **Tile→object index per column** (`dunMapTilesObjectIndexPerColumn`, `_4976_4cb4 << 1` bytes)
4. **Ground stacks** (`dunGroundStacks`, `cwListSize << 1` bytes)
5. **Text data** (`dunTextData`, `cwTextData << 1` bytes)
6. **16 DB record pools** (each `dbSize[db] * nRecords[db]` bytes)
7. **Map data** (`dunMapData`, `cbMapData` bytes)
8. **Extra dungeon data** (via `STORE_EXTRA_DUNGEON_DATA()`)
9. **Game state block** (`skload_table_60`, 56 bytes, SUPPRESS-encoded):
   - `dwGameTick`, `dwRandomSeed`, `wChampionsCount`, `wPlayerPosX/Y`, `wPlayerDir`, `wPlayerMap`, `wChampionLeader`, `wTimersCount`, rain state fields, misc state
10. **Ingame global flags** (8 bytes, SUPPRESS)
11. **Ingame global bytes** (64 bytes, SUPPRESS)
12. **Ingame global words** (64 words, SUPPRESS)
13. **Champion squad** (261 bytes × `wChampionsCount`, SUPPRESS)
14. **Global spell effects** (6 bytes, SUPPRESS)
15. **Timers table** (10 bytes × `wTimersCount`, SUPPRESS)
16. **Champion inventories** — each champion's 30 inventory slots as record-link chains via `WRITE_RECORD_CHECKCODE`
17. **Leader hand possession** — single record link
18. **Extra dungeon data**
19. **Minion association table** (via `WRITE_MINION_ASSOC`)

### SUPPRESS compression

`SUPPRESS_WRITER` writes bit-planes using per-field masks. Fields with mask=0 are skipped. Non-zero nibbles from data+mask are packed LSB-first into a byte stream. `SUPPRESS_READER` decodes on load. Flush at end of save.

Source: docs/dm2_save_format.md

### DM1 vs DM2 save format comparison

| Aspect | DM1 | DM2 |
|---|---|---|
| File | `CHAMP.DAT` per champion + `DUNGEON.DAT` per dungeon | Single `SKSave.dat` with all state |
| Compression | None (raw records) | SUPPRESS bit-level RLE |
| Dungeon state | Separate .DAT per level | Fully embedded in save |
| Header magic | None visible | Magic 0xBEEF/0xDEAD slot markers |
| Extra data | None | `STORE_EXTRA_DUNGEON_DATA()` hook |
| Backup | None | `.bak` auto-created on each save |

---

## 10. Variant/Platform Differences

### Data file variants

| Variant | DUNGEON.DAT | GRAPHICS.DAT | Notes |
|---|---|---|---|
| DM2 DOS EN (EN/LEGEND) | 39,437 bytes | ~8.6 MB | Baseline canonical |
| DM2 DOS FR | same size | `b4d733576ea60c41737f79f212faf528` | French language |
| DM2 DOS DE JewelCase | same size | `e52ab5e01715042b16a4dcff02052e5d` | German/English |
| DM2 PC-9821 | same size | ~12 MB | Japanese PC-9821 multilanguage |
| DM2 Sega CD | 37,957 bytes | — | Slightly smaller dungeon |
| DM2 Beta | different | different | Beta variant |
| DM2 Demo | different | different | Demo variant |
| DM2 TQ (Theron's Quest) | different | different | TQ variant |

Source: docs/dm2_platform_data.md, docs/dm2_source_lock.md, dm2_v1_game.c (hash list)

### Platform variants

| Platform | Engine | Notes |
|---|---|---|
| DOS (3.5" floppy, 6 disks) | SKULL.EXE (LE format, protected mode) | Baseline |
| Windows 3.1 | SKULLWIN (Allegro 5) | VP6 codec for FMV video sequences |
| Sega CD | — | Different DUNGEON.DAT size (37,957) |
| PC-9821 (Japanese) | — | Larger GRAPHICS.DAT (~12 MB), multilanguage |

No Amiga or PlayStation version of DM2 exists (unlike DM1).

Source: docs/dm2_variants_platform.md

### Content variants

| Variant | Dungeon | Notes |
|---|---|---|
| Standard DM2 dungeons | DUNGEON.DAT | Main game |
| DUNGEON_MOD.DAT | Modified levels | modding |
| DUNGEON_SHOP.DAT | Shop levels | special |
| DUNGEON_TEST.DAT | Test levels | dev |
| Beta variant | pcskb_dungeon_sk_beta.dat | Beta |
| Demo variant | pcsk_demo.dat | Demo |

Source: docs/dm2_platform_data.md, skproject file layout

### Creature AI table differences

| Mode | CREATURE_AI_TAB_SIZE | MAXAI |
|---|---|---|
| DM2 extended | 64 | 255 |
| Original/DM1/CSB | 42 | 62 |

Source: SkGlobal.h:1007-1009

---

## 11. Source References Summary

| Evidence type | Source | Key symbols/offsets |
|---|---|---|
| DM2 dungeon loading | SKULL.ASM (522,128 lines, LE format) | READ_DUNGEON_STRUCTURE, FTL decompression |
| GDAT category definitions | skproject/SkGlobal.h | GDAT_CATEGORY_LIMIT, CREATURE_AI_TAB_SIZE |
| GDAT field codes | SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt | Door fields, ornate fields, item fields |
| Item/weapon/cloth access | skproject/SkWinCore.cpp | GET_ADDRESS_OF_RECORD{5,6,7,A}, ITEM_CHARGES_RW |
| Actuator/sensor types | SKWIN/defines.h | ACTUATOR_TYPE_*, ACTUATOR_FLOOR_TYPE__* |
| Teleporter/ladder | SKWIN/SkWinCore.cpp | DM2_FIND_LADDER_AROUND_PARTY, DRAW_TELEPORTER_CHIP |
| Door strength/color keys | SKWIN/DME.h | GDAT_DOOR_MIRRORED, color key fields |
| Save format | SKULL.ASM | SUPPRESS_WRITER/READER, STORE_EXTRA_DUNGEON_DATA |
| Platform data | skproject/SKULLWIN/Data/ | data_dm2_dm/, data_dm2_sk/, data_dm2_beta/ |
| LE/PE format | SKULL.EXE analysis (dm2_le_objects.md) | MZ/LE header at 0x2E10, 105 module pages |

Source: docs/dm2_source_lock.md (archive lock: Dungeon-Master-II-Skullkeep_DOS_EN.zip sha256 d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929)

---

## 12. Status and Gaps

### Source-locked

- GDAT category system (240 vs 29)
- DUNGEON.DAT overall layout (header + maps + things + text, FTL compressed)
- Level type system (OUTDOOR/INDOOR/BUILDING)
- GRAPHICS.DAT size/compression (IMG3/IMG9, 8-bit core with 16-bit overlay passes)
- Item DB types (dbWeapon/dbCloth/dbScroll/dbMisc)
- Actuator types (40+ types across door/shooter/logic/spawn/visual/item/champion)
- Sensor types (floor/wall separation, possession-based triggers)
- Teleporter GDAT category (0x18), ladder ornate field (0x11)
- Door types (0x09 normal, 0x0A dragon), color key fields, mirror flag
- Text retrieval system (GDAT text categories, font rendering)
- SKSAVE*.DAT format (SUPPRESS compression, 0xBEEF/0xDEAD magic)
- Variant data file sizes and hashes

### Still needs work

- **GDAT2.InternalCodes.txt** — referenced by multiple docs but not found in workspace as a standalone file. Content reconstructed from doc references and skproject comments. Full field code enumeration needs the actual file.
- **SKULL.ASM disassembly** — IDA disassembly confirmed at 522,128 lines, but the actual .asm file isn't present in the workspace. Content reconstructed from doc citations.
- **skproject actual source files** — `skproject/` directory exists but is empty (755 entries, all zero bytes). References to SkGlobal.h, SkWinCore.cpp, DME.h, defines.h are documented from the github repo at gbsphenx/skproject HEAD a962896 but the local copy is a stub.
- **DM2 dungeon parser in Firestaff** — `dm2_v1_dungeon_loader.c` is a stub (56 lines) that doesn't yet implement full parsing. The `dm2_v1_dungeon_get_square_type` function exists but the full record parsing, thing data extraction, and text data parsing still need implementation.
- **GDAT file reader** — No GDAT2 parser exists yet in Firestaff. The c_gdatfile class from skproject needs a C equivalent.
- **Save game parser** — `dm2_v1_save_load.c` is a stub (28 lines). SUPPRESS compression decoder not yet implemented.

### Implementation ordering recommendation

1. Implement GDAT file reader (c_gdatfile equivalent) — needed for all graphics/item/text loading
2. Implement DM2 dungeon parser (extend `dm2_v1_dungeon_loader.c`) — needed for world state
3. Implement SUPPRESS save decoder (`dm2_v1_save_load.c`) — needed for save/load round-trips
4. Implement text retrieval (`QUERY_GDAT_TEXT` equivalent) — needed for UI

---

*Sources locked against: SKULL.ASM sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099; Dungeon-Master-II-Skullkeep_DOS_EN.zip sha256 d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929; skproject gbsphenx/skproject HEAD a962896*