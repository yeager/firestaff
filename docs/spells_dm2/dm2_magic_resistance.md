# DM2 V1 Magic Resistance — Source Locked

**Source:** `skproject/SKWIN/SkWinCore.cpp` lines 51585 (APPLY_CREATURE_POISON_RESISTANCE)
**Source:** `skproject/SKWIN/SkWinCore.cpp` lines 522-523 (ANTI_MAGIC, ANTI_FIRE item bonuses)
**Source:** `skproject/SKWIN/SkWinCore.cpp` lines 42426-42550 (shield enchantment checks)

## Shield Enchantments as Magic Resistance

DM2 has three shield enchantment types that act as spell/fire resistance:

```cpp
// defines.h:273-275
#define ENCHANTMENT_FIRE_SHIELD     0
#define ENCHANTMENT_SPELL_SHIELD    1
#define ENCHANTMENT_PARTY_SHIELD    2
```

These are applied via the following spells:
- Fire Shield (FUL BRO NETA) → ENCHANTMENT_FIRE_SHIELD
- Spell Shield (YA IR) → ENCHANTMENT_SPELL_SHIELD
- Party Shield (YA IR, 2 runes) → ENCHANTMENT_PARTY_SHIELD (whole party)

### How Shield Resistance Works

```cpp
// SkWinCore.cpp:42426
if (glbChampionSquad[play].enchantmentAura == ENCHANTMENT_PARTY_SHIELD) { // == 2

// SkWinCore.cpp:42544
if (champion->enchantmentAura == ENCHANTMENT_SPELL_SHIELD)  // == 1
if (champion->enchantmentAura == ENCHANTMENT_FIRE_SHIELD)    // == 0
```

Shield enchantment stored in `champion->enchantmentAura` (0-2) and `champion->enchantmentPower` (level).

```cpp
// SkWinCore.cpp:20007 (PROCEED_ENCHANTMENT_SELF)
// enchantment applied to mask of champions (bitmask)
// power accumulates across multiple castings
// timer manages duration
```

### Shield Enchantment Mechanics

- Duration: Tick-based timer (ttyEnchantment)
- Power: Accumulates per cast (up to ~0x32 threshold before diminishing returns)
- Party Shield: applies to entire party at once (mask = 0x0F)
- Individual shields: apply to single champion only

## Anti-Magic and Anti-Fire Item Bonuses

DM2 items can provide resistance through item bonuses:

```cpp
// SkWinCore.cpp:522
if (bonus == GDAT_ITEM_BONUS_ANTI_MAGIC)  return "ANTI-MAGIC";
if (bonus == GDAT_ITEM_BONUS_ANTI_FIRE)   return "ANTI-FIRE";
```

These are item modifier bonuses applied via `RETRIEVE_ITEM_BONUS()`:
- `GDAT_ITEM_BONUS_ANTI_MAGIC`: Reduces magical damage/effects
- `GDAT_ITEM_BONUS_ANTI_FIRE`: Reduces fire damage

## Poison Resistance

```cpp
// SkWinCore.cpp:51585
Bit16u SkWinCore::APPLY_CREATURE_POISON_RESISTANCE(ObjectID recordLink, Bit16u iPoisonDamage)
```

Used when creatures are hit by poison:
- Called for enemy creatures at `SkWinCore.cpp:18350`
- Called for target creature at `SkWinCore.cpp:27418`

```cpp
// SkWinCore.cpp:52127
((bp0c->w24 & 0x1000) != 0 && _4976_4b7a != 1) 
    ? 0 
    : (bp16 + APPLY_CREATURE_POISON_RESISTANCE(bp2e, glbPoisonAttackDamage))
```

Creature flag `w24 & 0x1000` indicates poison immunity (bypassed only if special condition).

## Spell Reflector Cloud

```cpp
// SkWinCore.cpp:838
if (recordLink == OBJECT_EFFECT_REFLECTOR) return "REFLECTOR";
```

Spell Reflector (ZO BRO ROS) creates `OBJECT_EFFECT_REFLECTOR` cloud (oFF8E).
Cloud energy: `BETWEEN_VALUE(21, ((bp06 << 1) +4) * (power +2), 255)`

When a spell hits the reflector cloud, it is reflected back.

## Invisibility as Magic Defense

```cpp
// SkWinCore.cpp:10434
(glbGlobalSpellEffects.Invisibility != 0) ? 0x09 : (bp0a +5),
```

Invisibility (OH EW SAR) modifies how creatures engage — some ignore invisible targets.

## Global Spell Effects as Resistance

```cpp
// SkWinCore.cpp:30907
DEBUG_HELP_WRITER("Global Spell Effects", &glbGlobalSpellEffects, 6, 1);
```

`glbGlobalSpellEffects` struct contains:
- Light (light level modifier)
- Invisibility (targeting defense)
- AuraOfSpeed (combat speed boost)
- SeeThruWalls (mapping)
- FreezeCounter (slow effect from cold)

These global effects provide indirect magical defense.

## DM1 vs DM2 Magic Resistance

| Feature | DM1 | DM2 |
|---------|-----|-----|
| Spell Shield | Yes | Yes |
| Fire Shield | No | Yes (new) |
| Party Shield | Yes | Yes |
| Anti-Magic item bonus | No | Yes (new) |
| Anti-Fire item bonus | No | Yes (new) |
| Spell Reflector | No | Yes (new) |
| Poison Resistance | Yes | Yes (same system) |
| Invisibility defense | Yes | Yes |

Key DM2 additions: Fire Shield, Anti-Magic/Anti-Fire item bonuses, Spell Reflector.

