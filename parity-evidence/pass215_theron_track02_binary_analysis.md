# Pass 215 — Theron's Quest Track 02 Binary Analysis

**Date:** 2026-08-01
**Source:** Hash-verified US Track 02 BIN (MD5: f23601102138f87c33025877767ebf76)
**Method:** Direct binary analysis of MODE1/2352 raw sector image

## Track 02 Layout

The US Track 02 BIN is 8,104,992 bytes = 3,446 raw sectors × 2,352 bytes.
Extracting the 2,048-byte user-data payload from each sector yields a
7,057,408-byte logical stream (6,892 KiB).

### Region Map

| Region | User-Data Offset | Size | Content |
|--------|-----------------|------|---------|
| Block 0 | 0x000000–0x03FFFF | 256 KiB | Empty (all zeros) |
| Block 1 | 0x040000–0x07FFFF | 256 KiB | System Card BIOS + credits (58 KiB data at +0x30800) |
| Block 2 | 0x080000–0x0BFFFF | 256 KiB | Graphics data (227 KiB nonzero) |
| Block 3 | 0x0C0000–0x0FFFFF | 256 KiB | Graphics data (225 KiB nonzero, compressed) |
| Block 4 | 0x100000–0x13FFFF | 256 KiB | Graphics data (227 KiB nonzero) |
| Block 5 | 0x140000–0x17FFFF | 256 KiB | Graphics data (226 KiB nonzero) |
| Block 6 | 0x180000–0x1BFFFF | 256 KiB | Padding (all 0xFF) |
| Code+Data | 0x1C0000–0x3FFFFF | 2,304 KiB | HuC6280 executable + embedded data tables |
| Dense Data | 0x400000–0x57FFFF | 1,536 KiB | Graphics/tile banks (98–99% nonzero) |
| Code+Data | 0x580000–0x67FFFF | 1,024 KiB | Code + data (34–88% density) |
| Empty | 0x680000–0x6BAFFF | 240 KiB | Empty (all zeros) |

The first seven 256 KiB regions match the existing `THERON_TRACK02_QUEST_BLOCK_COUNT`
constant but only blocks 2–5 contain substantive graphics data. Block 1 is the
Hudson Soft System Card BIOS code and development credits. Block 6 is 0xFF padding.

### Block 3 Compression Signature

Block 3 has an unusually high frequency of byte 0x14 (195 occurrences in the
first 4 KiB) alongside patterns like `C1`, `C5`, `C7` prefix bytes followed by
8-byte payloads. This suggests a custom run-length or LZ-style compression format
used for tile/sprite data. The exact decompression algorithm requires HuC6280
disassembly of the graphics driver routine.

## Executable Code Area (0x1C0000–0x680000)

The post-block area contains HuC6280 machine code with embedded data tables.
Observable opcodes include STA/LDA/INY/RTS patterns consistent with 65C02-family
instruction encoding. The code area is duplicated: content at 0x1C0000 appears
again at 0x200000, likely for CD error recovery or bank-switching redundancy.

### Descriptor Tables

Three instances of the 9-word stride descriptor table (0x0020, 0x0420, ..., 0x2020)
appear at user-data offsets 0x622C06, 0x624C06, and 0x626D84. These are embedded
within executable code — the byte immediately before the first instance (at
0x622C05) is 0x60 (RTS), confirming the descriptor sits at the tail of a
subroutine.

## Extracted Text Tables

### Item Names (US Localization)

Two item name tables were found. The first (at UD 0x1D9737) contains 63 items
matching the DM1 item set. The second (at UD 0x21A08E) contains 66 items including
Theron-unique items:

| Index | Name | Notes |
|-------|------|-------|
| 0 | COMPASS | Standard DM1 item |
| 1 | ILLUMULET | Standard DM1 item |
| 6 | VORPAL BLADE | **Theron-unique** |
| 7 | THE RETALIATOR | **Theron quest item** |
| 12 | ROCK | **Theron-unique** (replaces FURY in DM1 table) |
| 14 | STAFF OF MANAR | **Theron-unique** |
| 22 | MITHRAL AKETON | **Theron-unique** (replaces TORSO PLATE) |
| 30 | MITHRAL MAIL | **Theron-unique** (replaces LEG PLATE) |
| 38 | EKKHARD CROSS | **Theron-unique** (replaces FOOT PLATE) |
| 39 | MAGICAL BOX | Standard DM1 item |
| 41 | RABBIT'S FOOT | **Theron-unique** (replaces ROPE position) |
| 44 | CHEST | Standard DM1 item |
| 45 | OPEN CHEST | Standard DM1 item |
| 62 | CORN | **Theron-unique** food item |
| 63 | DRUMSTICK | **Theron-unique** food item |

### DM1-Compatible Item Name Table (UD 0x1D9737)

63-item table in the primary code area. Includes the full DM1 plate armor set
(TORSO PLATE, LEG PLATE, ARMET, FOOT PLATE), TORCH, EYE OF TIME, FURY, GOLD
COIN, and RUBY KEY. Does NOT include TQ-unique items (VORPAL BLADE, MITHRAL
gear, etc.). This represents the DM1 item ID namespace used by the dungeon
engine's internal object system.

### Three Item Table Variants

TQ contains three distinct item name tables representing different ID namespaces:

1. **DM1-compatible** (63 items at UD 0x1D9737): DM1 PC 3.4 item set with plate
   armor, TORCH, EYE OF TIME, FURY, GOLD COIN, RUBY KEY. Used by internal
   dungeon object system.

2. **TQ-unique** (66 items at UD 0x21A08E): Production item set with 11
   Theron-unique items (VORPAL BLADE, STAFF OF MANAR, MITHRAL AKETON/MAIL,
   EKKHARD CROSS, RABBIT'S FOOT, ROCK, CORN, DRUMSTICK, CHEST/OPEN CHEST).

3. **Quest block copy** (~80 items at UD 0x099517): Superset combining DM1 full
   item set (WATERSKIN, WATER, FLAMITT, FALCHION, SWORD, GHI, Lyte armor,
   WOODEN SHIELD, SHIELD DEFIANT, BOULDER, BREAD, CHEESE, SCREAMER SLICE,
   IRON/GOLD/RA KEY) with some TQ items. This appears to be the original DM1
   PC 3.4 table carried over unchanged.

### Level Names

15 level names (LEVEL 1 through LEVEL 15) at UD 0x27423B, each as fixed-width
null-terminated ASCII strings. This confirms the same 15-level depth structure
as DM1.

### Quest Retrieval Messages

Seven quest completion messages at UD 0x27713D:

1. "THERON has retrieved the Shield Defiant."
2. "THERON has retrieved the Taza Boots."
3. "THERON has retrieved the Taza Poleyn."
4. "THERON has retrieved the Soulcage."
5. "THERON has retrieved the Taza Armour."
6. "THERON has retrieved the Tazahelm."
7. "THERON has retrieved the Retaliator."

### Save/Load UI Strings

- "WHICH FILE DO YOU PLAY?" (UD 0x2770E9)
- "WHICH FILE DO YOU LOAD?" (UD 0x277102)
- "FILE_1", "FILE_2", "FILE_3" (UD 0x27711B)
- "YES", "NO" (UD 0x277133)

### Status Strings

- "POISONED", "BROKEN", "CURSED", "AND" (UD 0x1C65DF)
- "HEALTH", "STAMINA", "MANA" (UD 0x1CFD85)
- "WAKE UP", "GAME FROZEN" (UD 0x1C2E1D)
- "RESURRECTED" (UD 0x1C6E72)
- "LOAD  KG" (UD 0x1CB3AC — weight display)

### Hand-to-Hand/Movement Actions (UD 0x1DEEC1)

5-entry table: PUNCH, KICK, WAR CRY, STAB, CLIMB DOWN.

### Combat/Spell Action Names (UD 0x1DEEE4)

30-entry table: FREEZE LIFE, HIT, SWING, STAB, THRUST, JAB, PARRY, HACK,
BERZERK, FIREBALL, DISPELL, CONFUSE, LIGHTNING, DISRUPT, MELEE, X, INVOKE,
SLASH, CLEAVE, BASH, STUN, SHOOT, SPELLSHIELD, FIRESHIELD, HEAL, CALM,
LIGHT, SPIT, BRANDISH, THROW.

### Champion Classes (UD 0x1C9A32)

FIGHTER, NINJA, PRIEST, WIZARD.

### Skill Level Names (UD 0x1C9B6B)

NEOPHYTE, NOVICE, APPRENTICE, JOURNEYMAN, CRAFTSMAN, ARTISAN, ADEPT, EXPERT,
then 6 MASTER variants with prefix glyphs 0x60-0x65 (custom font rank icons),
then ARCHMASTER (16 entries total).

### Stat Names (UD 0x1C9B15)

STRENGTH, DEXTERITY, WISDOM, VITALITY, ANTI-MAGIC, ANTI-FIRE.

### Resource Names (UD 0x1CFD85)

HEALTH, STAMINA, MANA.

### Combat Strings (UD 0x1C9AE3)

"HEADS.", "TAILS." (Theron-unique coin flip mechanic), "CAN'T REACH",
"NEED AMMO".

### UI Interaction Messages (UD 0x1C9A4E)

" NEEDS MORE PRACTICE WITH THIS ", " MUMBLES A MEANINGLESS SPELL.",
" NEEDS AN EMPTY FLASK IN HAND FOR POTION.", " SPELL.",
" JUST GAINED A ", " LEVEL!", "IT COMES UP " (coin flip prefix).

### Container/Compass/Status UI Strings

Flask states (UD 0x1C9C5B): "(EMPTY)", "(ALMOST EMPTY)", "(ALMOST FULL)",
"(FULL)".

Compass (UD 0x1C9C7F): "PARTY FACING", "NORTH", "EAST", "SOUTH", "WEST".

Weight display (UD 0x1C9CA2): "WEIGHS", " KG.".

Light source (UD 0x1C9CAE): "(BURNT OUT)".

Item attributes (UD 0x1C65D4): "CONSUMABLE", "POISONED", "BROKEN", "CURSED",
", ", " AND ".

System messages: "WAKE UP" (UD 0x1C2E1D), "GAME FROZEN" (UD 0x1C2E25),
"RESURRECTED." (UD 0x1C6E72).

Hall of Champions (UD 0x1CBBBC): "GO AWAY AND RESURRECT THERON".

### Creature Type Names (UD 0x2741EF)

7-entry creature name table in the second code area. Each entry is 7 bytes
of fixed-width ASCII (space-padded), separated by 0x01, final entry
terminated with 0x00. These are Theron-unique creature types, distinct from
DM1/CSB creatures:

| Index | Name | Notes |
|-------|------|-------|
| 0 | AKUTUBA | Theron-unique |
| 1 | DRATOR | Theron-unique |
| 2 | FORMIC | Theron-unique |
| 3 | SARMON | Theron-unique |
| 4 | SHADO | Theron-unique |
| 5 | THIEF | Theron-unique |
| 6 | DEMON | Shared with DM1 |

Immediately after: "GAME SPEED" options menu label (UD 0x274228), followed
by level names at UD 0x27423B.

### Experience Threshold Table (UD 0x1DA890)

64-entry word table (little-endian uint16). Monotonically increasing values
from 0 to 214. Likely maps to 4 skill classes × 16 skill levels (matching
the 16 skill level names at UD 0x1C9B6B and 4 champion classes at UD
0x1C9A32).

Preceded at UD 0x1DA870 by class base-stat parameters:
- UD 0x1DA870: 0, 0, 60, 50, 256, 256, 256, 256 (words)
- UD 0x1DA880: 3, 3, 3, 3, 0, 10, 54, 90 (words)

### Dungeon Lore / Quest Narrative (UD 0x27613E–0x276CCB)

7 multi-line narrative blocks describing the backstory and quest objective
for each dungeon.  Text lines are separated by 0x01, blocks terminated by
0x00.  Each dungeon corresponds to one creature type and one quest item:

| Dungeon | Creature | Quest Item | Key Locations/NPCs |
|---------|----------|------------|---------------------|
| 0 Ak-Tu-Ba | AKUTUBA | Shield Defiant | Floating fortress, Mummies, sorcerer Alaphalon |
| 1 Drator's Tower | DRATOR | Taza Boots | Cult of Deaths, warrior/wizard |
| 2 Formicia | FORMIC | Taza Poleyn | Underground, Trolins, Ya-Brodin monastery |
| 3 Sarmon's Lair | SARMON | Soulcage | Evil wizard spirit, Brotherhood of Enlightenment |
| 4 Shadodan's Den | SHADO | Taza Armour | Ancient dragon, witch curse |
| 5 Village of Thieves | THIEF | Tazahelm | Swamps of Nordoor, Gigglers |
| 6 Demon's Gate | DEMON | Retaliator | Demon Sargoth, the sword |

### Save Overwrite UI (UD 0x2751D3)

"THAT FILE ALREADY EXISTS!" warning, "REPLACE" / "NO" confirmation buttons.

### System Card Credits (Block 1)

Japanese Shift-JIS text at Block 1 offset +0x30800 identifies the Hudson Soft
development team:

- PRODUCER: 中本伸一 (Shinichi Nakamoto)
- DIRECTOR: 野沢勝広 (Katsuhiro Nozawa)
- CD-ROM SIMULATOR: 本迫芳夫 (Yoshio Honosako)
- BIOS MAIN CODE, CD-PLAYER: 小林敬樹 (Takaki Kobayashi)
- PSG DRIVER: 岩淵貴幸 (Takayuki Iwabuchi)
- GRAPHIC DRIVER: 岩崎啓真 (Keima Iwasaki)
- GRAPHIC DRIVER SUB: 及川克之 (Katsuyuki Oikawa)
- Copyright: "1988 Sep. Written by TAKAKI KOBAYASHI"
- "PC Engine CD-ROM SYSTEM / Copyright HUDSON SOFT / NEC Home Electronics,Ltd."

## Non-Claims

This analysis does NOT establish:

- Tile/material semantics for any quest block graphics data
- The compression format used in blocks 3–5
- Level grid cell value meanings (the byte-faithful grid is proven but
  square values remain semantically unresolved)
- Object table format or location
- Champion stat record format
- The relationship between the two item tables (DM1-compatible vs Theron-unique)
- Creature attribute records (HP, attack, defense, speed — names are proven)
- Shop price table source offsets
