# DM2 V1 Magic Items — Source Locked

**Source:** `skproject/SKWIN/SkGlobal.cpp` lines 514-530 (GDAT categories)
**Source:** `skproject/SKWIN/SkWinCore.cpp` lines 14068-14198 (item consumption)
**Source:** `skproject/SKWIN/SkWinCore.cpp` lines 517-548 (item bonus names)
**Source:** `skproject/SKWIN/SkGlobal.cpp` lines 966-1011 (potion spell definitions)

## Magic Item Categories

DM2 defines magic items across GDAT categories:

```cpp
// SkGlobal.cpp:519-524
GDAT_CATEGORY_SCROLLS,   // 0x12 — scrolls
GDAT_CATEGORY_POTIONS,   // 0x13 — potions
GDAT_CATEGORY_CLOTHES,   // 0x11 — rings, amulets, boots, gloves, etc.
GDAT_CATEGORY_MISCELLANEOUS, // 0x15 — wands, rods, etc.
```

Item ID ranges per category:
```cpp
// SkGlobal.cpp (glbActivationItemRangePerDB)
scro: 0x1FC       (1 entry, scroll type in item)
poti: 0x180-0x1AF (48 potion entries)
weap: 0x000-0x07F (128 weapon entries)  
clot: 0x080-0x0FF (128 clothing entries)
ches: 0x1E0-0x1FB (28 container entries)
misc: 0x100-0x17F (128 misc entries — wands, rods)
```

## Scrolls

Scrolls in DM2 are identified by category but the spell they cast is determined by the scroll's item type (sub-class). When used, a scroll invokes `CAST_SPELL_PLAYER` with the appropriate spell definition.

```cpp
// SkWinCore.cpp:41450
return bp04->castToScroll()->ItemType();  // spell cast from scroll item type
```

## Potions

DM2 potions are created by spell-casting (flask consumed), not found pre-made. Potions are identified by category and type:

```cpp
// SkWinCore.cpp:51816
U8 potionType = bp04->PotionType();
```

Potion power formula: `(RAND() & 15) + (power * 40)`

Potion types (from spell indices):
- Health Potion (VI)
- Mana Potion (ZO BRO RA)
- Stamina Potion (YA)
- Strength Potion (FUL BRO KU)
- Dexterity Potion (OH BRO ROS)
- Vitality Potion (YA BRO NETA)
- Wisdom Potion (YA BRO DAIN)
- Shield Potion (YA BRO)
- Anti Venin (VI BRO)

## Magic Clothing (Rings, Amulets, etc.)

DM2 clothing can have item bonuses applied:

```cpp
// SkWinCore.cpp:522-548
if (bonus == GDAT_ITEM_BONUS_ANTI_MAGIC)   return "ANTI-MAGIC";
if (bonus == GDAT_ITEM_BONUS_ANTI_FIRE)    return "ANTI-FIRE";
if (bonus == GDAT_ITEM_BONUS_MANA)          return "MANA";
if (bonus == GDAT_ITEM_BONUS_LUCK)          return "LUCK";
if (bonus == GDAT_ITEM_BONUS_STRENGTH)      return "STRENGTH";
if (bonus == GDAT_ITEM_BONUS_DEXTERITY)     return "DEXTERITY";
if (bonus == GDAT_ITEM_BONUS_WISDOM)        return "WISDOM";
if (bonus == GDAT_ITEM_BONUS_VITALITY)      return "VITALITY";
if (bonus == GDAT_ITEM_BONUS_LIGHT)         return "LIGHT";
if (bonus == GDAT_ITEM_BONUS_WALK_SPEED)    return "SPEED";
```

Skill-specific bonuses:
```cpp
if (bonus == GDAT_ITEM_BONUS_FIGHTER)       return "FIGHTER";
if (bonus == GDAT_ITEM_BONUS_NINJA)          return "NINJA";
if (bonus == GDAT_ITEM_BONUS_PRIEST)         return "PRIEST";
if (bonus == GDAT_ITEM_BONUS_WIZARD)         return "WIZARD";
if (bonus == GDAT_ITEM_BONUS_WIZ_FIRE)       return "(W1)FIRE";
if (bonus == GDAT_ITEM_BONUS_WIZ_AIR)        return "(W2)AIR";
if (bonus == GDAT_ITEM_BONUS_WIZ_EARTH)      return "(W3)EARTH";
if (bonus == GDAT_ITEM_BONUS_WIZ_WATER)      return "(W4)WATER";
```

## Wands and Rods

Wands/rods are in the miscellaneous category (0x100-0x17F). They have charges and provide spell-like effects.

```cpp
// SkWinCore.cpp:2309
// Money per charge (value/charge system)
QUERY_GDAT_DBSPEC_WORD_VALUE(si, GDAT_ITEM_BONUS_MONEY_PER_CHARGE);
```

## Item Bonus Application

```cpp
// SkWinCore.cpp:5309-5390 (BOOST_ATTRIBUTE)
iBonusModifier = RETRIEVE_ITEM_BONUS(di, GDAT_ITEM_BONUS_MANA, bp0a, si);
// manaMax modified by bonus
// attributes, skills, light all can be boosted
```

## New in DM2 vs DM1

DM2 introduces:
- **ANTI-MAGIC item bonus** — new resistance bonus type
- **ANTI-FIRE item bonus** — new resistance bonus type
- **Mana item bonus** — increases max mana
- **Speed item bonus** — increases walk speed
- **Light item bonus** — increases light level

DM2 removes:
- Magic Footprints scroll (YA BRO ROS)
- Petrify scroll (YA VEN SAR)

