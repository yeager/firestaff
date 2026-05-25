# DM2 V1 Spell System — Source Locked

**Source:** `skproject/SKWIN/SkGlobal.cpp` lines 966-1011 (dSpellsTable)
**Source:** `skproject/SKWIN/SkWinCore.cpp` lines 17521-17670 (CAST_SPELL_PLAYER)
**Source:** `skproject/SKWIN/SkGlobal.h` lines 37-55 (constants)

## Total Spell Count

DM2 V1 defines **34 spells** in the base table (`MAXSPELL_ORIGINAL = 34`).

```cpp
// SkGlobal.h:41
#define MAXSPELL_ORIGINAL    34
#define MAXSPELL_CUSTOM      255   // extended custom spell mode
```

The dSpellsTable has 34 entries (index 0-33). Extended mode (GDAT-loaded) supports up to 255.

## Spell Table Structure

Each SpellDefinition entry: `{ runes (3sym), difficulty, skillReq, spellValue }`

```cpp
// SkGlobal.cpp:966
{MkssymVal(s2YA  ,s3IR  ,s4DAIN),0x04,0x0F,0x3823 }, // Spell Shield
```

- Byte 04: spell difficulty (0-6)
- Byte 05: required skill (0x0F=Wizard, 0x11=Priest, etc.)
- Bytes 06-07: spell type (low 4 bits) + spell produced result (upper bits)

## Spell Types

```cpp
// SkGlobal.h:50-53
#define SPELL_TYPE_POTION    1  // requires empty flask
#define SPELL_TYPE_MISSILE   2  // shoots magical projectile
#define SPELL_TYPE_GENERAL   3  // enchantments, light, etc.
#define SPELL_TYPE_SUMMON    4  // summons a creature minion
```

## DM2 vs DM1 Spell Count

| Game | Base Spells |
|------|-------------|
| DM1  | ~30 (approx) |
| DM2  | 34          |

DM2 adds: Push, Pull, Attack/GUARD/U-Haul Minions, Spell Reflector.

## Spell List (all 34)

Index | Runes         | Name
------|---------------|----------------------------------
0     | OH IR RA      | Long Light
1     | DES IR SAR    | Darkness
2     | YA IR         | Spell Shield (Party)
3     | OH EW SAR     | Invisibility
4     | YA IR (2sym)  | Magical Shield
5     | FUL           | Light
6     | OH EW DAIN    | Aura of Wisdom
7     | OH EW ROS     | Aura of Dexterity
8     | FUL BRO NETA  | Fire Shield
9     | OH EW NETA    | Aura of Vitality
10    | OH EW KU      | Aura of Strength
11    | OH IR ROS     | Aura of Speed
12    | ZO BRO ROS    | Spell Reflector
13    | YA EW (2sym)  | Magical Marker (item creator)
14    | OH VEN        | Poison Cloud (0D/07)
15    | OH KATH RA    | Lightning (0D/02)
16    | FUL IR        | Fireball (0D/00)
17    | FUL BRO KU    | NP: STR Potion (13/07)
18    | DES EW        | Antimatter (0D/03)
19    | DES VEN       | Poison Bolt (0D/06)
20    | ZO            | Open/Close Door (0D/04)
21    | YA BRO        | NP: Shield Potion (13/0C)
22    | YA            | NP: Stamina Potion (13/0B)
23    | YA BRO DAIN   | NP: Wisdom Potion (13/08)
24    | YA BRO NETA   | NP: Vitality Potion (13/09)
25    | VI            | NP: Health Potion (13/0E)
26    | VI BRO        | NP: Anti Venin (13/0A)
27    | OH BRO ROS    | NP: Dexterity Potion (13/06)
28    | ZO BRO RA     | NP: Mana Potion (13/0D)
29    | ZO EW KU      | Attack Minion (0F/31) — SUMMON
30    | ZO EW NETA    | Guard Minion (0F/34) — SUMMON
31    | ZO EW ROS     | U-Haul Minion (0F/35) — SUMMON
32    | OH KATH KU    | Push (0D/09)
33    | OH KATH ROS   | Pull (0D/0A)

## DM1 Spells Removed in DM2

These DM1/NEXUS spells do NOT appear in DM2:
- YA BRO ROS: Magic Footprints
- YA VEN SAR: Petrify
- VI BRO NETA: Restore Health
- VI BRO RA: Restore Health for Party
- OH EW RA: See Through Walls (re-implemented in DM2 EXTENDED_MODE only)
- ZO KATH RA: ZoKathRa Spell

## Spell Casting Mechanics

From `SkWinCore.cpp:17521` (CAST_SPELL_PLAYER):

1. **Difficulty check**: `bp0e = (ref->w6_a_f() * (power + 18)) / 24`
2. **Cast chance**: Wizard ability +15 vs difficulty. Failed cast: skill penalty applied, spell fails.
3. **Skill decay**: `ADJUST_SKILLS(player, ref->requiredSkill, bp0c << (bp08 - bp06))`
4. **Cooldown**: `ADJUST_HAND_COOLDOWN(player, bp0e, 2)` after successful cast

