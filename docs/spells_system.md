# DM1 V1 - Spell System Source Lock
Audit date: 2026-05-25 | Source: ReDMCSB WIP20210206

## Source Files (in order of authority)
| File      | Functions        | Lines          |
|-----------|------------------|----------------|
| MENU.C    | F0409, F0412     | 44-76, 1633-2041 |
| SYMBOL.C  | F0399, F0400     | 10-27          |
| CASTER.C  | F0394-F0398      | -              |
| DEFS.H    | spell constants  | 2932-2941      |

---

## 1. How Many Spells Exist in DM1 V1?

25 spells defined in G0487_as_Graphic560_Spells[25] (MENU.C:50-76).

---

## 2. Spell Symbol System - 4 Steps x 6 Symbols = 24 Runes

| Step | Name      | Symbols                  | Role                    |
|------|-----------|--------------------------|-------------------------|
| 0    | Power     | Lo, Um, On, Ee, Pal, Mon | Spell tier / power (1-6) |
| 1    | Element   | Ya, Vi, Oh, Ful, Des, Zo | Base element            |
| 2    | Class     | Ven, Ew, Kath, Ir, Bro, Gor | Sub-class / modifier  |
| 3    | Alignment | Ku, Ros, Dain, Neta, Ra, Sar | Final refinement     |

Character encoding: 96 + 6*step + index -> ASCII characters 97-120.

Symbol packing: 4 bytes, MSB-first. Spells with MSB=0 compare only lower
3 bytes (power symbol optional).

---

## 3. Spell Table Structure (DM1_Spell, MENU.C:50-76)

  typedef struct {
      uint32_t symbols;       // packed 4-byte symbol sequence
      uint8_t  baseRequiredSkillLevel;
      uint8_t  skillIndex;     // DM1_SKILL_XXX
      uint16_t attributes;    // spell kind, type, duration, rune count
  } DM1_Spell;

Attributes field decode:
- Bits [15:12] = spell kind (projectile, area, party, self, etc.)
- Bits [11:8]  = spell type
- Bits [7:0]   = duration or rune-count metadata

---

## 4. Complete Spell List

| #  | Name                  | Symbols      | Req | Skill    | Attrs   |
|----|-----------------------|-------------|-----|----------|---------|
|  0 | SHIELD                | Ya Ir        | 2   | DEFEND   | 0x7843  |
|  1 | MAGIC FOOTPRINTS      | Ya Bro Ros   | 1   | EARTH    | 0x4863  |
|  2 | INVISIBILITY          | Oh Ew Sar    | 3   | AIR      | 0xB433  |
|  3 | POISON CLOUD          | Oh Ven      | 3   | WATER    | 0x6C72  |
|  4 | THIEVE'S EYE          | Oh Ew Ra    | 3   | EARTH    | 0x8423  |
|  5 | LIGHTNING BOLT        | Oh Kath Ra  | 4   | AIR      | 0x7822  |
|  6 | LIGHT                 | Oh Ir Ra    | 4   | AIR      | 0x5803  |
|  7 | TORCH                 | Ful         | 1   | FIRE     | 0x3C53  |
|  8 | FIREBALL              | Ful Ir      | 3   | FIRE     | 0xA802  |
|  9 | STRENGTH POTION       | Ful Bro Ku  | 4   | HEAL     | 0x3C71  |
| 10 | FIRE SHIELD           | Ful Bro Neta| 4   | DEFEND   | 0x7083  |
| 11 | WEAKEN NONMATERIAL    | Des Ew      | 1   | EARTH    | 0x5032  |
| 12 | POISON BOLT           | Des Ven    | 1   | WATER    | 0x4062  |
| 13 | DARKNESS              | Des Ir Sar  | 1   | DEFEND   | 0x3013  |
| 14 | OPEN DOOR             | Zo          | 1   | AIR      | 0x3C42  |
| 15 | SHIELD POTION         | Ya Bro      | 2   | DEFEND   | 0x64C1  |
| 16 | STAMINA POTION        | Ya          | 2   | HEAL     | 0x3CB1  |
| 17 | WISDOM POTION         | Ya Bro Dain | 4   | HEAL     | 0x3C81  |
| 18 | VITALITY POTION       | Ya Bro Neta | 4   | HEAL     | 0x3C91  |
| 19 | HEALTH POTION         | Vi          | 1   | HEAL     | 0x80E1  |
| 20 | CURE POISON POTION    | Vi Bro      | 1   | HEAL     | 0x68A1  |
| 21 | DEXTERITY POTION      | Oh Bro Ros  | 4   | HEAL     | 0x3C61  |
| 22 | MANA POTION           | Zo Bro Ra   | 3   | PRIEST   | 0xFCD1  |
| 23 | POISON POTION         | Zo Ven      | 2   | WATER    | 0x7831  |
| 24 | ZOKATHRA              | Zo Kath Ra  | 0   | WIZARD   | 0x3C73  |

---

## 5. Skill System

Four base skill categories: WIZARD, PRIEST, WARRIOR, ROGUE (DEFS.H).

Sub-skills: FIRE, WATER, EARTH, AIR (WIZARD); HEAL, DEFEND (PRIEST).

Spell lookup uses the skill index from the spell table to check
champion skill level.

---

## 6. Source Evidence

  MENU.C:44      G0485_aauc_Graphic560_SymbolBaseManaCost[4][6]
  MENU.C:49      G0486_auc_Graphic560_SymbolManaCostMultiplier[6]
  MENU.C:50-76   G0487_as_Graphic560_Spells[25]
  MENU.C:1685-1705 F0409_MENUS_GetSpellFromSymbols
  DEFS.H:2932-2941 spell/failure constants