# DM2 V1 New Spells — Source Locked

**Source:** `skproject/SKWIN/SkGlobal.cpp` lines 966-1011
**Source:** `skproject/SKWIN/SkWinCore.cpp` lines 17521-17670

## New in DM2 (not present in DM1)

DM2 introduces the following spells that did not exist in DM1:

### Minion Summoning Spells (Indices 29-31)

All are SUMMON type (type 4), creating party-controlled minions.

| Index | Runes      | Name          | Creature ID | Notes |
|-------|------------|---------------|-------------|-------|
| 29    | ZO EW KU   | Attack Minion | 0x31        | Combat minion |
| 30    | ZO EW NETA | Guard Minion  | 0x34        | Defensive minion |
| 31    | ZO EW ROS  | U-Haul Minion | 0x35        | Pickup/carry minion |

**Minion Mechanics** (`SkWinCore.cpp:17524-17602`):
- Summon power: `((RAND02() + (bp06 << 1)) * power) / 6`
- Minion type determined by SpellCastIndex
- If summon fails (map full), creates dust cloud instead (oFFA8)
- U-Haul minion (0x35): searches for existing U-Haul, destroys if found (only one at a time)
- Spawns in front of party: `(glbPlayerDir + 2) & 3`, at party position

### Push and Pull Spells (Indices 32-33)

Missile-type spells (type 2) for creature repositioning.

| Index | Runes         | Name | ObjectID | Notes |
|-------|---------------|------|----------|-------|
| 32    | OH KATH KU    | Push | oFF89    | Push target away |
| 33    | OH KATH ROS   | Pull | oFF8A    | Pull target toward |

### Spell Reflector (Index 12)

**Runes:** ZO BRO ROS | **Type:** GENERAL | **Effect:** Creates oFF8E reflector cloud

```cpp
// SkWinCore.cpp:17762
case 14:    // Spell reflector : oFF8E
    CREATE_CLOUD(
        OBJECT_EFFECT_REFLECTOR, 
        BETWEEN_VALUE(21, ((bp06 << 1) +4) * (power +2), 255),
        glbPlayerPosX,
        glbPlayerPosY,
        255
    );
```

Creates a cloud at party position that reflects incoming spells.

### Fire Shield (Index 8)

**Runes:** FUL BRO NETA | **Type:** ENCHANTMENT | **EnchantmentType:** `ENCHANTMENT_FIRE_SHIELD` (0)

New shield type — DM1 had only Spell Shield. Fire Shield provides anti-fire protection.

### Magical Marker (Index 13)

**Runes:** YA EW | **Type:** GENERAL | **Effect:** Creates item from GDAT

```cpp
// SkWinCore.cpp:17808
case 15:    // Magical Marker => Item creator!
    bp1a = QUERY_GDAT_ENTRY_DATA_INDEX(0x0d, 0x0f, dtWordValue, 0x42);
    // In GDAT: missile 0x0F, entry 0x42 = item 0x013E (YaEw marker)
```

Item ID is GDAT-driven (entry 0x42 of missile category). If leader hand is empty, item goes to hand; otherwise drops on floor.

## Potion Spells New to DM2 (Indices 17, 21-28)

DM2 introduces 10 NP (Non-Power) potion spells using POTION type (1):

| Index | Runes          | Potion Type | Effect |
|-------|----------------|-------------|--------|
| 17    | FUL BRO KU     | STR Potion  | +Strength (power*40 + RAND) |
| 21    | YA BRO         | Shield Potion | |
| 22    | YA             | Stamina Potion | |
| 23    | YA BRO DAIN    | Wisdom Potion | |
| 24    | YA BRO NETA    | Vitality Potion | |
| 25    | VI             | Health Potion | |
| 26    | VI BRO         | Anti Venin  | |
| 27    | OH BRO ROS     | Dexterity Potion | |
| 28    | ZO BRO RA      | Mana Potion  | |

Potion power: `(RAND() & 15) + (power * 40)`

## DM1 Spells NOT in DM2

- Magic Footprints (YA BRO ROS) — removed
- Petrify (YA VEN SAR) — removed  
- Restore Health (VI BRO NETA) — removed, potions take this role
- Restore Health Party (VI BRO RA) — removed
- See Through Walls (OH EW RA) — removed (re-implemented in EXTENDED_MODE only)
- ZoKathRa (ZO KATH RA) — removed

