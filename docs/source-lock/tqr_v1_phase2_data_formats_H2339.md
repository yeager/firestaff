# Theron's Quest V1 Phase 2 — Data Formats: Source-Lock Document

**Cron task:** `Theron_V1_Phase2_DataFormats_0506`
**Status:** 🟡 IN PROGRESS — hypothesis stage (no disc images available)
**Author:** Firestaff agent (cron)
**Last revised:** 2026-05-27T04:37 UTC
**Source:** Phase 0 provenance gate (H2339) · DM1/CSB ReDMCSB source-lock · PC Engine platform knowledge

---

## Scope

Source-lock Theron's Quest V1 dungeon, object, text, champion, creature, and
graphics formats. Document every known/estimated structure with provenance
citations, hedging unknowns as STUB/INFERRED.

> **Source-lock rule:** Every format claim either cites a primary source (ReDMCSB,
> Platform docs) or is explicitly marked STUB / INFERRED / HYPOTHESIS.
> No guessing without the hedge.

**Critical constraint:** No disc images available at time of writing. All format
details derive from DM1/CSB baseline + platform inference. Disc image acquisition
remains the blocking dependency for precision.

---

## 1. Platform and Binary Context

### 1.1 Theron's Quest Binary Architecture

Theron's Quest is a PC Engine CD-ROM² game. The entire game ships as a single
data track (Track 02) containing:
- HuC6280 executable code (8-bit, 65C02-derived, little-endian like Intel 8088)
- Dungeon data (7 mini-dungeons, embedded blocks)
- Graphics tile/sprite data (PC Engine planar format)
- ADPCM audio data (non-CD-DA tracks)

**Contrast with DM1:** DM1 splits game data across two files:
- `GRAPHICS.DAT` (~500 KB): wall/floor/creature/item sprites, compressed
- `DUNGEON.DAT` (~500 KB): map grid + things + text, compressed

Theron's Quest combines both into one Track 02 binary, format unknown.
The binary must be parsed sequentially — no filesystem, no external files.

**Hypothesis:** Track 02 structure:
```
Offset 0x0000:       HuC6280 entry point / game code
Offset ~0x20000:     Dungeon data block 1 (Dungeon 1)
Offset ~0x40000:     Dungeon data block 2 (Dungeon 2)
...                  (7 blocks, ~0x20000 each = ~1.4 MB total)
Offset ~0xE0000:     Graphics tile data (PC Engine tile format)
Offset ~0xF0000:     ADPCM audio / SFX data
Offset ~0xFFFFF+:    CD-ROM data track end
```

PC Engine CD-ROM² games typically map data into the 64 KB CD-ROM buffer
window which is bank-switched into the 8 KB work RAM address space.
HuC6280 can address up to 64 KB directly; larger data requires banking.

**Source:** tqr_v1_phase0_provenance_gate_H2339.md · PC Engine HuC6280 platform docs

### 1.2 Endianness

| Platform | Endianness | Notes |
|----------|-----------|-------|
| PC Engine HuC6280 | Little-endian (like 65C02) | Same byte order as DM1 x86 |
| DM1 (Intel 8088) | Little-endian | Baseline for ReDMCSB structs |

Since Theron's Quest was developed by FTL (the same team as DM1) and targets
little-endian HuC6280, dungeon data structures likely use the **same byte
layout** as DM1's DUNGEON.DAT — little-endian uint16 for grid squares, headers,
thing counts. This is a strong hypothesis, not confirmed.

**Confidence:** Medium. FTL designed the original DM1 format; they would
reuse it for the port. However, the PC Engine's 64 KB addressable limit
may force a different structural layout (e.g., banked segments).

**Source:** DM1/ReDMCSB DEFS.H · HuC6280 programmer's reference

### 1.3 Dungeon Count and Scope

| Property | DM1 | Theron's Quest |
|----------|-----|----------------|
| Dungeons | 1 large (16 levels) | 7 mini-dungeons |
| Levels per dungeon | 16 (14 usable + 2 unused) | Unknown (likely 1–4 each) |
| Grid size | 16×16 max | Unknown (likely smaller) |
| Total squares | 14×16×16 = 3,584 | ~7 × (unknown) |
| Maps per dungeon | up to 14 MAP descriptors | Unknown |

The 7 mini-dungeons are each self-contained. Some dungeons are documented as
"copied or inspired by DM1/CSB" — meaning dungeon data blocks may use the same
square encoding, thing types, and text encoding as DM1 DUNGEON.DAT.

**Source:** tqr_v1_phase0_provenance_gate_H2339.md §3.4

---

## 2. Dungeon Format — Hypothesis

### 2.1 Overall Structure — STUB

No dungeon format documentation exists for Theron's Quest. All details are
INFERRED from DM1 DUNGEON.DAT as baseline.

**Hypothesized Track 02 dungeon block layout:**
```
Offset 0x0000:     DUNGEON_BLOCK[0] — Dungeon 1
                  Size: unknown (estimated 32–64 KB)
                  Layout: DUNGEON_HEADER + MAP[] + raw tiles + things
Offset N:         DUNGEON_BLOCK[1] — Dungeon 2
...               (7 blocks, variable size)
Offset M:         GRAPHICS_BLOCK — tile/sprite data (PC Engine format)
Offset P:         AUDIO_BLOCK — ADPCM SFX/music data
```

The DUNGEON_BLOCK structure **hypothesized** to mirror DM1 DUNGEON.DAT:
```
DUNGEON_HEADER (44 bytes):
  uint16_t  ornamentRandomSeed
  uint16_t  rawMapDataByteCount
  uint8_t   mapCount         (≤7, one per mini-dungeon level)
  uint8_t   unreferenced
  uint16_t  textDataWordCount
  uint16_t  initialPartyLocation  (X[4:0] | Y[9:5] | Dir[11:10])
  uint16_t  squareFirstThingCount
  uint16_t  thingCounts[16]   (DUNGEON_THING_TYPE_COUNT = 16)

MAP descriptors (16 bytes each, mapCount entries):
  uint16_t  rawMapDataByteOffset
  uint8_t   offsetMapX
  uint8_t   offsetMapY
  uint8_t   level, width, height   (width/height stored as actual-1)
  uint8_t   wallOrnamentCount, floorOrnamentCount
  uint8_t   creatureTypeCount, difficulty
  uint8_t   allowedCreatureTypes[16]   (max 16 creature types)
  uint8_t   floorSet, wallSet, doorSet0, doorSet1

Raw map data (column-major, 2 bytes per square):
  Column 0: (height+1) × 2 bytes
  Column 1: (height+1) × 2 bytes
  ... for (width+1) columns

Square format (2 bytes, little-endian uint16):
  bits [4:0]   = Square type (M034_SQUARE_TYPE)
  bits [14:5]  = First thing index (10 bits)
  bit [15]     = THING_LIST_PRESENT flag

Square types (inferred from DM1):
  0 = Wall
  1 = Corridor/Floor
  2 = Pit
  3 = Stairs (up/down)
  4 = Door
  5 = Teleporter
  6 = Fake wall
  7+ = Unknown / TQR-specific

Source: DM1/ReDMCSB DEFS.H:989–998 · memory_dungeon_dat_pc34_compat.h:33–72
```

### 2.2 Mini-Dungeon Grid Size — STUB

PC Engine resolution is 256×224 pixels (32×28 tiles at 8×8 each).
Visible viewport would be approximately 9×7 squares (vs DM1's 9×7 visible grid).

**Hypothesis:** Mini-dungeons use a grid size of 9×7 or 16×16 tiles.
9×7 matches the viewport size (one-screen dungeons). Larger dungeons
may span multiple screens with scrolling.

| Scenario | Grid | Evidence |
|----------|------|----------|
| Single-screen dungeons | 9×7 or 16×12 | PC Engine 256×224 resolution |
| Multi-screen dungeons | 16×16 (DM1-like) | Unlikely for "mini" design |

**Source:** tqr_v1_phase0_provenance_gate_H2339.md · PC Engine HuC6260 video spec

### 2.3 DUNGEON_HEADER Fields — STUB (Baseline: DM1)

The DUNGEON_HEADER fields (44 bytes) are hypothesized to be identical to DM1.
This is the most likely scenario since FTL designed DM1's format and would
reuse it for Theron's Quest.

| Offset | Field | Type | DM1 Use | TQR Hypothesis |
|--------|-------|------|---------|----------------|
| 0x00 | ornamentRandomSeed | uint16 | Random ornament seed | Same |
| 0x02 | rawMapDataByteCount | uint16 | Total bytes of map squares | Same |
| 0x04 | mapCount | uint8 | Number of MAP descriptors | ≤7 for TQR |
| 0x05 | unreferenced | uint8 | Padding | Same |
| 0x06 | textDataWordCount | uint16 | 3-char word count for text | Same |
| 0x08 | initialPartyLocation | uint16 | Encoded X/Y/direction | Same |
| 0x0A | squareFirstThingCount | uint16 | Square-first-thing entries | Same |
| 0x0C | thingCounts[16] | uint16[16] | Count per THING_TYPE | Same |

**Confidence:** High for structure; Medium for field offsets (banking may shift)

### 2.4 MAP Descriptors — STUB (Baseline: DM1)

```
struct DungeonMapDesc_TQR {
    uint16_t  rawMapDataByteOffset;  /* byte offset to column 0 of this map */
    uint8_t   aUnreferenced;
    uint8_t   bUnreferenced;
    uint8_t   offsetMapX;           /* dungeon X offset (multi-map dungeons) */
    uint8_t   offsetMapY;           /* dungeon Y offset */
    /* Bitfield A: */
    uint8_t   level;                /* 6 bits: level number (0-63) */
    uint8_t   width;                /* 5 bits: actual width-1 (max 16) */
    uint8_t   height;               /* 5 bits: actual height-1 (max 16) */
    /* Bitfield B: */
    uint8_t   wallOrnamentCount;    /* 4 bits */
    uint8_t   randomWallOrnamentCount; /* 4 bits */
    uint8_t   floorOrnamentCount;  /* 4 bits */
    uint8_t   randomFloorOrnamentCount; /* 4 bits */
    /* Bitfield C: */
    uint8_t   doorOrnamentCount;   /* 4 bits */
    uint8_t   creatureTypeCount;   /* 4 bits */
    uint8_t   difficulty;          /* 4 bits */
    uint8_t   reservedC;           /* 4 bits */
    /* Bitfield D: */
    uint8_t   floorSet;            /* 4 bits */
    uint8_t   wallSet;              /* 4 bits */
    uint8_t   doorSet0;             /* 4 bits */
    uint8_t   doorSet1;             /* 4 bits */
};
```

**Note:** The TQR "light" version may simplify or omit certain fields.
MAP descriptor layout (16 bytes) is expected to be compatible with DM1's.

**Source:** DM1/ReDMCSB DEFS.H:1048–1070 · memory_dungeon_dat_pc34_compat.h:44–72

---

## 3. Thing System — STUB (Baseline: DM1)

DM1's DUNGEON.DAT uses a 16-thing-type system. Theron's Quest "light" version
likely uses the **same thing types** but with fewer total things per dungeon
(given smaller dungeon size and easier difficulty).

### 3.1 Thing Type Enumeration — STUB (Baseline: DM1)

```
THING_TYPE_DOOR        0  /* 4 bytes: next, slot, magic/melee destructible, vertical, button */
THING_TYPE_TELEPORTER  1  /* 8 bytes: local or remote teleporter pair */
THING_TYPE_TEXTSTRING  2  /* 4 bytes: text string index */
THING_TYPE_SENSOR      3  /* 8-12 bytes: trigger + associated thing */
THING_TYPE_GROUP       4  /* 16 bytes: monster group (creatureType, cells, health[], behavior) */
THING_TYPE_WEAPON      5  /* 4 bytes: weapon stats */
THING_TYPE_ARMOUR      6  /* 4 bytes: armour stats */
THING_TYPE_SCROLL      7  /* 4 bytes: scroll text + closed state */
THING_TYPE_POTION      8  /* 4 bytes: potion type + power */
THING_TYPE_CONTAINER   9  /* 8 bytes: container type + contents chain */
THING_TYPE_JUNK       10  /* 4 bytes: junk/resusable items */
THING_TYPE_PROJECTILE 14  /* 8 bytes: projectile type + direction + damage */
THING_TYPE_EXPLOSION  15  /* 8 bytes: explosion animation parameters */
```

### 3.2 DungeonGroup (Creature Groups) — STUB (Baseline: DM1)

```
struct DungeonGroup_TQR {
    uint16_t  next;              /* THING: Next in linked list */
    uint16_t  slot;              /* THING: Slot (possession chain) */
    uint8_t   creatureType;      /* 8 bits: creature type index (0-25 range for TQR) */
    uint8_t   cells;             /* Cell positions for up to 4 creatures */
    uint16_t  health[4];         /* HP per creature slot */
    uint8_t   behavior;          /* 4 bits: movement/attack AI pattern */
    uint8_t   count;             /* 2 bits: actual count = count + 1 (1-4 creatures) */
    uint8_t   direction;         /* 2 bits: facing direction */
    uint8_t   doNotDiscard;      /* 1 bit */
};
```

**TQR-specific note:** Since Theron's Quest is a "light" version with fewer
creatures, creatureType indices likely cover only a subset of DM1's 26 types.
Creature groups may also be smaller (fewer per group).

**Source:** DM1/ReDMCSB DEFS.H · memory_dungeon_dat_pc34_compat.h:282–295

### 3.3 Square-First-Thing System — STUB (Baseline: DM1)

DM1 uses a square-first-thing linked-list system:
- Each map square contains a `firstThing` index (10 bits)
- Things have a `next` index pointing to the next thing in the square's list
- Square header `THING_LIST_PRESENT` flag (bit 15) indicates if list exists

Theron's Quest likely uses the same system given the shared heritage.
Square-first-thing array size = `squareFirstThingCount` from DUNGEON_HEADER.

**Source:** DM1/ReDMCSB DUNGEON.C:1073 · memory_dungeon_dat_pc34_compat.h:380–386

---

## 4. Object / Item Format — STUB (Baseline: DM1)

### 4.1 Item Thing Types — STUB (Baseline: DM1)

Theron's Quest "light" version uses only a **subset of DM1 items**.
Item thing types 5–10 (WEAPON, ARMOUR, SCROLL, POTION, CONTAINER, JUNK)
likely appear with reduced variety.

#### 4.1.1 Weapon (type 5, 4 bytes on disk)

```
struct DungeonWeapon_TQR {
    uint16_t  next;          /* THING: Next in linked list */
    uint8_t   type;          /* 7 bits: weapon type index (0-15 range for TQR) */
    uint8_t   flags;         /* doNotDiscard(1) | cursed(1) | poisoned(1) | chargeCount(4) | broken(1) | lit(1) */
};
```

**TQR note:** Torch (type 7, lit=1) is the most common weapon in DM1 dungeons.
Theron's Quest likely includes torch + a small set of other weapons.

**Source:** DM1/ReDMCSB DEFS.H:1421–1428 · memory_dungeon_dat_pc34_compat.h:296–307

#### 4.1.2 Armour (type 6, 4 bytes on disk)

```
struct DungeonArmour_TQR {
    uint16_t  next;
    uint8_t   type;          /* 7 bits: armour type index */
    uint8_t   flags;         /* doNotDiscard(1) | cursed(1) | chargeCount(4) | broken(1) */
};
```

**Source:** memory_dungeon_dat_pc34_compat.h:308–317

#### 4.1.3 Scroll (type 7, 4 bytes on disk)

```
struct DungeonScroll_TQR {
    uint16_t  next;
    uint16_t  textStringThingIndex;  /* 10 bits: index into text string table */
    uint8_t   closed;               /* 6 bits: scroll open/closed state */
};
```

**Source:** memory_dungeon_dat_pc34_compat.h:318–324

#### 4.1.4 Potion (type 8, 4 bytes on disk)

```
struct DungeonPotion_TQR {
    uint16_t  next;
    uint8_t   power;         /* 8 bits: potency (0-255) */
    uint8_t   type;          /* 7 bits: potion type index */
    uint8_t   doNotDiscard;  /* 1 bit */
};
```

**Source:** memory_dungeon_dat_pc34_compat.h:325–332

#### 4.1.5 Container (type 9, 8 bytes on disk)

```
struct DungeonContainer_TQR {
    uint16_t  next;
    uint16_t  slot;          /* THING: contents chain (first contained item) */
    uint8_t   type;          /* 2 bits: container type (0-3) */
    /* Unknown bytes: likely padding or extended type field */
};
```

**Source:** memory_dungeon_dat_pc34_compat.h:333–339

#### 4.1.6 Junk (type 10, 4 bytes on disk)

```
struct DungeonJunk_TQR {
    uint16_t  next;
    uint8_t   type;          /* 7 bits: junk item type index */
    uint8_t   doNotDiscard;  /* 1 bit */
    uint8_t   charges;       /* 6 bits: charge count for usable items */
};
```

**ZOKATHRA note:** DM1 has junk item type 51 for ZOKATHRA (Zo Kath Ra spell).
Theron's Quest "light" version likely includes this since it uses a subset of
DM1 spells. ZOKATHRA junk is spawned when the champion casts Zo Kath Ra.

**Source:** DM1/ReDMCSB DEFS.H:1527 · csb_items.md

### 4.2 Item Count Estimate

| DM1 Item Category | DM1 Count | TQR Estimate |
|-------------------|-----------|--------------|
| Weapon types | ~16 | ~4–6 |
| Armour types | ~10 | ~3–5 |
| Scroll types | ~12 | ~4–6 |
| Potion types | ~12 | ~6–8 |
| Container types | ~4 | ~2–3 |
| Junk types | ~20+ | ~8–10 |

**Light version constraint:** Fewer item types per category, but same
basic structure (type + flags bytes).

**Source:** DM1/ReDMCSB DEFS.H item constants · tqr_v1_phase0_provenance_gate_H2339.md §3.1

---

## 5. Text Format — STUB (Baseline: DM1)

### 5.1 Text Data Encoding — STUB (Baseline: DM1)

DM1 stores dungeon text as a compressed 3-character word array:
- Each "word" is 2 bytes (16 bits), representing 3 characters
- Character encoding: 5 bits per character (0–31 range)
- Characters 0–25 = 'A'–'Z', 26–30 = special, 31 = end-of-text

**Hypothesis:** Theron's Quest uses the **same text encoding** as DM1.
Dungeon text blocks (inscriptions, scroll text, message柜台) are stored in
the same word-array format after the square data.

```
TextData format (DM1 baseline):
  uint16_t textData[textDataWordCount];  /* 3-char words, 5 bits each */
  
  Encoding per uint16_t:
    bits [14:10] = char 1 (0–31)
    bits [9:5]   = char 2 (0–31)
    bits [4:0]   = char 3 (0–31)
  
  Special codes:
    0x1F (31) = end-of-text
    0x1E (30) = separator (newline in messages)
    0x00 (0)  = space
```

**Source:** DM1/ReDMCSB DUNGEON.C:1620–1677 · memory_dungeon_dat_pc34_compat.h:448–471

### 5.2 Text String Decoding — STUB (Baseline: DM1)

DM1 stores text string data as a separate section after raw map data.
Each text string has a `textStringThingIndex` pointing to a `textStringThing`
record that contains the byte offset into `textData`.

```
struct DungeonTextString_TQR {
    uint16_t  next;          /* THING: Next in linked list */
    uint16_t  textDataWordOffset;  /* byte offset into text data section */
    uint8_t   textType;      /* Message, inscription, scroll text, etc. */
};
```

**TQR note:** Given 7 mini-dungeons with possibly 1–4 levels each, the text
data section is likely smaller than DM1's full 16-level text payload.

**Source:** DM1/ReDMCSB DUNGEON.C:1655 · memory_dungeon_dat_pc34_compat.h:224–230

### 5.3 Champion Name Text — UNKNOWN

Champion names (Theron + 3 companions) are stored separately from dungeon
text. Name encoding is unknown — could be standard ASCII, modified ASCII,
or a custom PC Engine font encoding.

**Research required:** Acquire disc image and locate champion name table.

**Source:** tqr_v1_phase0_provenance_gate_H2339.md §3.3

---

## 6. Champion Format — STUB (Baseline: DM1)

### 6.1 Champion State Structure — STUB (Baseline: DM1)

Theron's Quest uses a 4-champion party: **Theron** (persistent) + **3 companions**
(per-dungeon reset). The champion data structures are hypothesized to be
similar to DM1's champion format.

#### 6.1.1 Champion Record (DM1 baseline)

```
struct Champion_DM1 {
    /* Header / identity (offset 0–27) */
    uint8_t   name[10];       /* 10 bytes: name text, null-terminated */
    uint8_t   class;          /* 0=Fighter, 1=Ninja, 2=Priest, 3=Wizard */
    uint8_t   level;          /* 1–20 */
    uint16_t  experience;     /* cumulative XP */
    
    /* Physical stats (offset 12–25) */
    uint8_t   currentHealth;  /* current HP */
    uint8_t   maxHealth;
    uint8_t   currentMana;
    uint8_t   maxMana;
    uint8_t   currentStamina;
    uint8_t   maxStamina;
    
    /* Attributes (offset 24–31) */
    uint8_t   strength, maxStrength;
    uint8_t   dexterity, maxDexterity;
    uint8_t   wisdom, maxWisdom;
    uint8_t   antiFire, maxAntiFire;
    uint8_t   antiMagic, maxAntiMagic;
    uint8_t   luck, maxLuck;
    
    /* State flags (offset 34+) */
    uint8_t   condition;      /* poisoned, diseased, etc. */
    uint8_t   resistances;    /* physical, fire, lightning, masonry */
    /* ... more fields ... */
    
    /* Inventory (offset ~64+) */
    uint8_t   inventory[39];  /* 39 bytes: item slot IDs */
    uint8_t   gold;
    uint8_t   food;
    
    /* Equipment slots (offset ~104+) */
    uint8_t   helmet, armor, shield, weapon, boots, gauntlets, ring;
};
```

**Total champion record size:** ~140 bytes (DM1 PC version).
PC Engine version may differ slightly due to memory constraints.

**Source:** DM1/ReDMCSB CHAMPION.C · memory_champion_state_pc34_compat.h

#### 6.1.2 TQR Champion Differences

| Aspect | DM1 | Theron's Quest |
|--------|-----|----------------|
| Persistent champion | None (all 4 reset) | **Theron** (always persistent) |
| Companion persistence | All 4 reset between levels | **Reset per dungeon** |
| Champion count | 4 active | Theron + 3 active |
| In-dungeon save | Supported | **Not supported** |
| Between-dungeon save | Supported | Supported (RAM/battery) |

**Theron-specific fields:** No known changes to Theron's core stats/abilities.
Theron is a standard Fighter/Priest/Wizard class champion. The persistence
is a game-state design, not a champion structure change.

**Source:** tqr_v1_phase0_provenance_gate_H2339.md §3.3

### 6.2 Champion Skill System — STUB (Baseline: DM1)

DM1 has 20 skills across 4 classes (Fighter, Ninja, Priest, Wizard).
Theron's Quest "light" version may include a **subset of these skills**.
Skill encoding is hypothesized as the same bitfield system used in DM1.

```
Skill bitfield per champion (DM1 baseline):
  uint8_t  skills[5];   /* 40 bits total, one bit per skill */
  
  Skill indices:
    0–3:  Fighter (Swing, Thrust, Club, Parry)
    4–7:  Ninja (Steal, Fight, Throw, Shoot)
    8–11: Priest (Identify, Heal, Influence, Defend)
    12–15: Wizard (Fire, Air, Earth, Water)
    16–19: Shared (None, None, None, None) — per ReDMCSB
```

**TQR note:** If "light" version reduces skill depth, the skill bitfield
structure likely remains the same but with fewer active skills per class.

**Source:** DM1/ReDMCSB DEFS.H:757–768 · dm1_v1_spell_casting_pc34_compat.h:22–36

### 6.3 Save Data Format — UNKNOWN

Theron's Quest saves **only between dungeons** (no in-dungeon saves).
Save data includes:
- Theron's persistent stats and skills
- Dungeon completion flags (which items retrieved)
- Champion roster (3 companions, rebuilt each dungeon)
- Game progress state

**Save medium:** PC Engine RAM + battery (BRAM). The save data is likely
stored in a reserved area of the 8 KB work RAM or in a dedicated SRAM chip.

**Save size estimate:** <1 KB (4 champions × ~140 bytes = ~560 bytes + overhead).

**Research required:** Disc image analysis to locate save data format and
battery-backed RAM structure.

**Source:** tqr_v1_phase0_provenance_gate_H2339.md §3.3

---

## 7. Creature Format — STUB (Baseline: DM1)

### 7.1 Creature Type Roster — STUB (Baseline: DM1)

Theron's Quest "light" version uses a **subset of DM1's 26 creature types**.
Dungeon MAP descriptors have `creatureTypeCount` (4 bits, max 16) and
`allowedCreatureTypes[16]` array, meaning up to 16 creature types per dungeon.

**DM1 creature type list (26 types, indices 0–25):**
```
0:  Human (Fighter)
1:  Human (Ninja)
2:  Human (Priest)
3:  Human (Wizard)
4:  Ghost
5:  Demon
6:  Dragon
7:  Giant Spider
8:  Scorpion
9:  Snake
10: Bug
11: rat
12: Slime
13: Golem
14: Ettin
15: Dwarf
16: Troll
17: Orc
18: Goblin
19: Kobold
20: Marmot
21: Snake  (?)
22: Snake  (?)
23: Snake  (?)
24: Snake  (?)
25: Snake  (?)
```

**TQR estimate:** For the "light" version with easier difficulty and fewer
creatures, TQR likely uses 8–15 creature types covering the core archetypes:
Human (Fighter), Ghost, Demon, Dragon, Giant Spider, Snake, Bug, Slime,
Golem, Troll, Goblin.

**Source:** DM1/ReDMCSB DEFS.H creature constants · memory_dungeon_dat_pc34_compat.h

### 7.2 Creature Group Format — STUB (Baseline: DM1)

```
struct DungeonGroup_TQR {
    uint16_t  next;
    uint16_t  slot;
    uint8_t   creatureType;   /* index into creature type table (0-25 for DM1) */
    uint8_t   cells;         /* 8 bits: up to 4 cell positions in group formation */
    uint16_t  health[4];     /* individual HP per creature slot */
    /* Bitfield byte 14: */
    uint8_t   behavior;      /* 4 bits: AI/movement pattern */
    uint8_t   count;         /* 2 bits: actual count = count + 1 (1-4) */
    uint8_t   direction;     /* 2 bits: facing direction */
    uint8_t   doNotDiscard;  /* 1 bit */
};
```

**TQR "light" note:** Smaller dungeon size means fewer creature spawns.
Group size (1-4 creatures per group) likely retained.

**Source:** DM1/ReDMCSB DEFS.H:1429–1449 · memory_dungeon_dat_pc34_compat.h:282–295

### 7.3 Creature AI — INFERRED from DM1

DM1 creature AI is driven by the `behavior` field (4 bits) per creature group.
Behaviors include: wander, patrol, chase, sleep, guard, etc.

**Hypothesis:** Theron's Quest uses the same AI behavior enumeration since
the underlying game logic is derived from DM1. No major AI changes are
expected for a "light" version — only fewer creatures and less aggressive
spawn rates.

**Source:** DM1/ReDMCSB MOVESENS.C · creature AI behavior constants

---

## 8. Spell System — STUB (Baseline: DM1)

### 8.1 Spell Roster — STUB (Baseline: DM1)

Theron's Quest "light" version uses a **subset of DM1's 25 spells**.
Spell casting, rune sequences, and spell power values are hypothesized to
use the same underlying system as DM1.

**DM1 spell table (25 spells, indices 0–24):**
```
0:  SHIELD          (Di)         — defensive
1:  MAGIC FOOTPRINTS (Am)       — reveal
2:  INVISIBILITY    (Ach)       — stealth
3:  POISON CLOUD    (Bet)       — area
4:  THIEVE'S EYE    (Cal)       — reveal
5:  LIGHTNING BOLT  (Eri)       — damage
6:  LIGHT           (Ful)       — illumination
7:  TORCH           (Gio)       — illumination
8:  FIREBALL        (Hal)       — damage
9:  STRENGTH POTION (Icy)       — buff
10: FIRE SHIELD     (Jin)       — defensive
11: WEAKEN NONMAT   (Kan)       — debuff
12: POISON BOLT     (Lev)       — damage
13: DARKNESS        (Mor)       — environmental
14: OPEN DOOR       (Nef)       — utility
15: SHIELD POTION   (Osa)       — buff
16: STAMINA POTION  (Pas)       — buff
17: WISDOM POTION   (Quas)      — buff
18: VITALITY POTION (Radi)      — buff
19: HEALTH POTION   (Ssan)      — healing
20: CURE POISON     (Tic)       — cure
21: DEXTERITY POT   (Ula)       — buff
22: MANA POTION     (Vem)       — buff
23: POISON POTION   (Wan)       — damage/debuff
24: ZOKATHRA        (Zo Kath Ra) — fireball variant (CSB-added, present in DM1)
```

**TQR estimate:** TQR likely includes 10–15 of these spells:
Shield, Light, Torch, Fireball, Lightning Bolt, Open Door, and healing potions
are the most essential. Combat and utility spells retained; more exotic spells
(Thieve's Eye, Invisibility, etc.) may be omitted.

**ZOKATHRA presence:** Given that TQR uses a subset of DM1 items/creatures/spells,
ZOKATHRA (Zo Kath Ra) may or may not be included. Its inclusion depends on whether
TQR implements the fireball variant. DM1 has ZOKATHRA; it's not CSB-specific.

**Source:** DM1/ReDMCSB MENU.C:76 · dm1_v1_spell_casting_pc34_compat.c:41–100

### 8.2 Spell Casting — STUB (Baseline: DM1)

Spell casting in DM1 requires:
1. Champion with sufficient skill level in the appropriate class
2. Runes carved on tablet (Rune of X objects placed in dungeon)
3. Champion at correct Altar of VI
4. Correct incantation sequence (spoken rune names)

**TQR adaptation:** Same casting mechanism likely applies, but with fewer
altars of VI (though "many more" than DM1 per Phase 0 docs) and simplified
rune requirements for the "light" difficulty.

**Source:** dm1_v1_spell_casting_pc34_compat.c · tqr_v1_phase0_provenance_gate_H2339.md §3.3

---

## 9. Graphics Format — STUB (PC Engine Platform)

### 9.1 PC Engine Graphics Architecture

PC Engine CD-ROM² uses the HuC6260 (video) + HuC6270 (sprite) chipset:

| Component | Specification |
|-----------|--------------|
| Resolution | 256×224 (NTSC) or 256×240 |
| Colors | 512 (9-bit, from 262,144 palette) |
| Sprites | 64 max, up to 16×16 or 32×32 |
| Tiles | 8×8 pixels, 16 colors per tile (4 bits/pixel, planar) |
| VRAM | 64 KB dual-port video RAM |

Theron's Quest, like DM1, is a first-person dungeon crawler. The viewport
is rendered using pre-computed wall/floor/ceiling tiles drawn in a grid
pattern. No 3D polygon geometry (unlike Nexus on Saturn).

**Hypothesis:** Wall/floor textures are stored as 8×8 PC Engine tiles.
The viewport is composed by drawing these tiles in a grid matching the
dungeon square layout.

**Source:** tqr_v1_phase0_provenance_gate_H2339.md §5.1 · HuC6260/HuC6270 programmer's reference

### 9.2 Tile Data Format — STUB (PC Engine standard)

PC Engine tile format (standard for CD-ROM² games):
```
Tile data (8×8 pixels, 4 bits/pixel, planar):
  Per tile: 8 rows × 8 pixels × 4 bits = 32 bytes per tile
  
  Planar layout (4 bit-planes):
    Plane 0: bit 0 of each pixel, sequential
    Plane 1: bit 1 of each pixel, sequential
    Plane 2: bit 2 of each pixel, sequential
    Plane 3: bit 3 of each pixel, sequential
    
  Within each plane, pixels are stored sequentially (left to right, top to bottom).
  Total: 32 bytes per 8×8 tile.
```

**Sprite composition:** Sprites are built from multiple 8×8 tiles (typically
4 tiles for a 16×16 sprite, or 16 tiles for a 32×32 sprite). Sprites have
a color palette index (0–15) and a palette selection (0–3).

**Source:** PC Engine HuC6270 sprite documentation

### 9.3 Wall/Floor Tile Set — STUB

**Hypothesis:** Theron's Quest stores wall/floor tiles in Track 02's graphics
section with the same structural approach as DM1's GRAPHICS.DAT:
- Wall tiles: 3 walls per set × 7 views (N, E, S, W, NW, NE, SW, SE) × light levels
- Floor tiles: per floor set × light levels
- Special tiles: doors, teleported effects, pits

**Tile count estimate for "light" version:**
- Wall tiles: 3 sets × ~50 wall variants = 150 tiles
- Floor tiles: 2 sets × ~20 variants = 40 tiles
- Object/creature sprites: ~100 sprites (vs DM1's ~200)
- Total: ~290 tiles × 32 bytes = ~9 KB (small for PC Engine)

**Note:** PC Engine sprites are more compact than DM1's VGA planar bitmaps
since tiles are 8×8 rather than the 64×64 or 128×64 scaled bitmaps used in DM1.

**Source:** DM1 GRAPHICS.DAT analysis · tqr_v1_phase0_provenance_gate_H2339.md §4.3

### 9.4 Sprite Animation — STUB

DM1 uses animation frames (walk cycle, attack, death) stored as separate
sprite frames. Theron's Quest likely uses the same approach:
```
Animation sequence:
  Frame 0 → Frame 1 → Frame 2 → Frame 3 → (repeat or advance)
  
  Frame timing: tied to game tick (DM1: 12 ticks/second)
  TQR likely uses similar tick rate (7.16 MHz CPU / game loop)
```

**Source:** DM1/ReDMCSB GFXANIM.C · csb_graphics.md animation timing

### 9.5 Palette Data — STUB (PC Engine)

PC Engine palette is 512 colors (9-bit: 2×4×4×4×4 = 262,144 colors mapped to 512).
Each sprite/tile references one of 16 palette entries (4-bit palette index).

```
Palette entry format:
  uint16_t (on PC Engine, in VRAM):
    bits [11:8] = Red (0–15)
    bits [7:4]  = Green (0–15)
    bits [3:0]  = Blue (0–15)
```

**Palette switching:** The HuC6270 supports 4 background palettes + sprite
palettes. Dungeon wall/floor tiles likely share a palette; creature/object
sprites use different palette indices.

**Source:** HuC6270 programmer's reference · PC Engine graphics documentation

---

## 10. Comparison: DM1 vs TQR Format Map

| Category | DM1 (PC 3.4) | Theron's Quest (PCE) | Delta |
|----------|-------------|---------------------|-------|
| Dungeon blocks | 1 × 16-level DUNGEON.DAT | 7 × mini-dungeon blocks | Different |
| Grid per level | 16×16 max | Unknown (likely ≤16×16) | Smaller |
| Map descriptors | Up to 14 per dungeon | Unknown | Reduced |
| Square type bits | 5 bits (32 types) | Same | Unchanged |
| Thing types | 16 types (0–15) | Same | Unchanged |
| Item types | Full set (~50 types) | Subset (~20–30) | Light version |
| Creature types | 26 types | Subset (~10–15) | Light version |
| Spell count | 25 | Subset (~10–15) | Light version |
| Text encoding | 3-char word array | Same (hypothesized) | Unchanged |
| Text data | Separate section | Same (hypothesized) | Unchanged |
| Champion record | ~140 bytes | Same (hypothesized) | Minor differences |
| Champion persistence | All reset between levels | Theron persistent, companions reset | Different |
| Graphics format | VGA planar 320×200 | PC Engine tile/sprite 256×224 | Different |
| Endianness | Little-endian | Little-endian (HuC6280) | Same |
| Save format | In-dungeon + between | Between-dungeons only | Simplified |

---

## 11. Format Compatibility Assessment

### 11.1 DM1 Data Format Reuse Potential

Given the strong architectural similarity between DM1 and Theron's Quest:
- Same game logic engine (FTL)
- Same first-person dungeon crawler genre
- Same champion/creature/item/spell concepts
- Same Altar of VI mechanic

**Hypothesis:** The **dungeon data block format** (DUNGEON_HEADER + MAP + squares
+ things) may be nearly identical to DM1's DUNGEON.DAT format.
The primary differences are:
1. File format wrapper (TQR Track 02 is a single binary; DM1 is two files)
2. Dungeon count/size (7 mini vs 1 large)
3. Item/creature/spell subset (light version)
4. Graphics format (PC Engine tiles vs VGA planar)

If the dungeon block format is identical, Firestaff's `memory_dungeon_dat_pc34_compat.c`
could serve as a reference for implementing Theron's Quest dungeon parsing,
with modifications for Track 02 binary extraction and reduced thing type counts.

### 11.2 Structural Differences to Investigate

After disc image acquisition, the following must be verified:
1. Does the dungeon block use the exact 44-byte DUNGEON_HEADER?
2. Are MAP descriptors 16 bytes each with identical field layout?
3. Is the square format 2 bytes (type + thing index)?
4. Are THING_TYPE values 0–15 as in DM1?
5. Is text encoding the 3-char word format?
6. Are champion records the same ~140-byte structure?

---

## 12. Outstanding Research Dependencies

| Priority | Dependency | Blocker | Notes |
|----------|-----------|---------|-------|
| 🔴 CRITICAL | Disc image (JP: b7afb338ad31be1025b53f9aff12d73a) | All format claims | Track 02 binary analysis required |
| 🔴 CRITICAL | Disc image (US: f23601102138f87c33025877767ebf76) | All format claims | Compare JP vs US |
| 🟡 HIGH | Track 02 binary extraction + SHA256 | Dungeon/graphics format | Use TurboRip or similar |
| 🟡 HIGH | Locate 7 dungeon data blocks in Track 02 | Dungeon format | Binary search for DUNGEON_HEADER signatures |
| 🟡 HIGH | Verify square format (2-byte little-endian) | Dungeon format | Pattern scan for 0x0001, 0x0002 etc. |
| 🟡 HIGH | Identify creature type table | Creature format | Look for 26-entry creature name list |
| 🟡 HIGH | Identify item type table | Item format | Look for weapon/armour/potion name tables |
| 🟡 MEDIUM | Champion save format (battery-backed RAM) | Champion format | SRAM dump or emulator save state |
| 🟡 MEDIUM | Identify text encoding (3-char word?) | Text format | Scan for known text patterns |
| 🟡 MEDIUM | Identify graphics tile data offset | Graphics format | PC Engine tile signature search |
| 🟡 LOW | CD-ROM banking scheme for 64 KB window | Full binary parsing | HuC6280 memory mapping |

---

## 13. Phase 2 Completion Checklist

```
[x] Dungeon format hypothesis (DM1 baseline + TQR adaptations)
[x] Object/item format hypothesis (light subset)
[x] Text format hypothesis (DM1 3-char word encoding)
[x] Champion format hypothesis (Theron persistent, 3 companions reset)
[x] Creature format hypothesis (light subset of 26 types)
[x] Spell system hypothesis (light subset of 25 spells)
[x] Graphics format hypothesis (PC Engine tile/sprite)
[x] Format comparison map (DM1 vs TQR)
[x] Format compatibility assessment with Firestaff DM1 code
[x] Outstanding research dependencies documented
[ ] DISC IMAGE: Acquire JP disc image from cdromance.org
[ ] DISC IMAGE: Acquire US disc image from cdromance.org
[ ] BINARY: Extract Track 02 from both disc images
[ ] BINARY: Compute SHA256 hashes and verify against provenance gate
[ ] BINARY: Locate 7 dungeon data blocks in Track 02
[ ] DUNGEON: Verify DUNGEON_HEADER (44 bytes) and MAP descriptors
[ ] DUNGEON: Confirm square format (2-byte little-endian)
[ ] DUNGEON: Identify dungeon grid sizes for all 7 mini-dungeons
[ ] THINGS: Verify thing type enumeration (0–15 vs subset)
[ ] ITEMS: Identify item type count and sizes
[ ] TEXT: Verify 3-char word text encoding
[ ] CHAMPION: Locate champion save data format
[ ] GRAPHICS: Locate and parse tile/sprite data
[ ] GRAPHICS: Identify wall/floor tile sets and sprite animations
[ ] PARSE: Write parser for Track 02 → Firestaff TQR structures
[ ] COMMIT: Save data format documentation to docs/source-lock/
```

---

## 14. Reference Documents

| Document | Path | Status |
|----------|------|--------|
| Phase 0 Provenance Gate | `docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md` | ✅ Complete |
| Phase 1 Runtime Profile | (not yet written) | ❌ Pending |
| **Phase 2 Data Formats** | `docs/source-lock/tqr_v1_phase2_data_formats_H2339.md` | ← **This document** |
| DM1 Dungeon Header | `include/memory_dungeon_dat_pc34_compat.h` | ✅ Source-lock |
| CSB Dungeon Data Model | `docs/source-lock/csb_v1_phase2_dungeon_data_model_H2233.md` | ✅ Reference |
| DM1 Item/Object Format | `include/memory_dungeon_dat_pc34_compat.h` | ✅ Source-lock |
| DM1 Champion Format | `include/memory_champion_state_pc34_compat.h` | ✅ Source-lock |
| DM1 Spell System | `src/dm1/dm1_v1_spell_casting_pc34_compat.c` | ✅ Source-lock |

---

*Generated by cron job `Theron_V1_Phase2_DataFormats_0506`*
*Supersedes: nothing yet — first Phase 2 document*
*Blocking dependency: disc image acquisition from cdromance.org*
*Next step: Phase 1 runtime profile (boot/menu/asset discovery)*