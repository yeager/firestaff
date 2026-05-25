# DM2 V1 — New Creatures vs DM1

**Source-locked to:** skproject/SKWIN/SkWinCore.cpp:741-810 (getAIName), SkGlobal.h:1007-1012, DME.h:1505-1538

---

## 1. Expanded AI Table

DM2 extends the creature AI table from **42 entries (DM1)** to **64 entries (DM2)**, with MAXAI increasing from 62 to 255 in extended mode.

Source:
- SkGlobal.h:1007 `#define CREATURE_AI_TAB_SIZE 64` (DM2)
- SkGlobal.h:1009 `#define CREATURE_AI_TAB_SIZE 42` (original/DM1/CSB)
- SkGlobal.h:163 `#define MAXAI 255` (DM2) vs `#define MAXAI 62` (original)
- SkWinCore.cpp:243 `memcpy(dAITable, dAITableGenuine, MAXAI * 36)`

The `dAITable` (AIDefinition array) is pre-initialized from `dAITableGenuine` (hard-coded static table), then optionally overwritten by GDAT in extended mode via `EXTENDED_LOAD_AI_DEFINITION` (SkWinCore.cpp:233-400).

---

## 2. DM2 AI Index Table (0x00–0x3E)

Full list from `getAIName(U8 ai)` at SkWinCore.cpp:741–810:

### 0x00–0x0F

| Index | Name | DM1 Equivalent? |
|---|---|---|
| 0 | TREE (PILLAR) | New (environment) |
| 1 | LABORATORY TABLE | New (environment) |
| 2 | ???? | Unknown |
| 3 | BUSH | New (environment) |
| 4 | PILLARS / ROD (PILLAR) | New (environment) |
| 5 | STALAGMITE (PILLAR) | New (environment) |
| 6 | BOULDER | New (environment) |
| 7 | FOUNTAIN | New (environment) |
| 8 | OBELISKS / TOMBS | New |
| 9 | WOOD TABLE (TABLE) | New (environment) |
| 10 | MAGICK CAULDRON | New |
| 11 | SKULL BRAZIER | New |
| 12 | TRADING TABLE | New (merchant/shop) |
| 13 | SCOUT MINION (ALLY) | **NEW — companion system** |
| 14 | ATTACK MINION (ALLY) | **NEW — companion system** |
| 15 | CARRY MINION (ALLY) | **NEW — companion system** |

### 0x10–0x1F

| Index | Name | DM1 Equivalent? |
|---|---|---|
| 16 | FETCH MINION (ALLY) | **NEW — companion system** |
| 17 | GUARD MINION (ALLY) | **NEW — companion system** |
| 18 | U-HAUL MINION (ALLY) | **NEW — companion system** |
| 19 | THORN DEMON | New |
| 20 | OBELISK (PASSABLE) | New |
| 21 | VORTEX | New |
| 22 | FLAME ORB | New |
| 23 | CAVERN BAT (BAT) | DM1 bat equivalent |
| 24 | GLOP | New |
| 25 | ROCKY | New |
| 26 | GIGGLER | DM1 equivalent (stealer) |
| 27 | THICKET THIEF | New |
| 28 | TIGER STRIPED WORM (WORM) | New |
| 29 | TREANT (TREE GORGON) | New |
| 30 | LORD DRAGOTH | **NEW — primary antagonist** |
| 31 | DRU TAN | New |

### 0x20–0x2F

| Index | Name | DM1 Equivalent? |
|---|---|---|
| 32 | CAVE IN | New (trap) |
| 33 | MERCHANTS | **NEW — NPC/shop system** |
| 34 | DRAGOTH MINION (EVIL) | **NEW — Dragoth sub-type** |
| 35 | TOWER BAT (BAT) | DM1 bat equivalent |
| 36 | ARCHER GUARD | New (ranged guard) |
| 37 | MAGICK REFLECTOR (MACHINE) | New |
| 38 | POWER CRYSTAL (MACHINE) | New |
| 39 | EVIL FOUNTAIN (FOUNTAIN) | DM1 fountain variant |
| 40 | SPIKED WALL / FLOOR SPIKES | DM1 spike equivalent |
| 41 | SPECTRE (GHOST) | DM1 ghost equivalent |
| 42 | VEG MOUTH (DIGGER WORM) | New |
| 43 | EVIL ATTACK MINION (EVIL) | New |
| 44 | AXEMAN | DM1 axeman equivalent |
| 45 | CAVERN / STONE TABLE / WALL HOLE? | New |
| 46 | MUMMY | New |
| 47 | VOID DOOR (MACHINE) | New |

### 0x30–0x3E

| Index | Name | DM1 Equivalent? |
|---|---|---|
| 48 | DARK VEXIRK (VEXIRK) | **NEW — Vexirk race** |
| 49 | EVIL GUARD MINION (ENEMY) | **NEW** |
| 50 | SKELETON | DM1 skeleton equivalent |
| 51 | AMPLIFIER (MACHINE) | **NEW — fireball attack** |
| 52 | WOLF | New |
| 53 | PIT GHOST (GHOST) | DM1 ghost variant (invisible) |
| 54 | DOOR GHOST (GHOST) | DM1 ghost variant |
| 55 | VEXIRK KING (VEXIRK) | **NEW — Vexirk boss** |
| 56 | ? OBELISK LIKE ? | Unknown |
| 57 | AXEMAN THIEF | New |
| 58 | FLYING CHEST | New |
| 59 | BARREL | New |
| 60 | PEDISTAL (PILLAR) | New |
| 61 | GHOST | DM1 ghost equivalent |
| 62 | EVIL ATTACK MINION (EVIL) | Same as index 43 |

---

## 3. Summary: Truly New Creature Categories

### Companion/Minion System (DM2 unique)
- Indices 13–18: Ally minions (Scout, Attack, Carry, Fetch, Guard, U-Haul)
- Indices 34, 43, 49, 62: Evil minion variants

### Boss/Elite Creatures
- Index 30: **LORD DRAGOTH** — primary antagonist, final boss
- Index 34: **DRAGOTH MINION (EVIL)** — Dragoth sub-type
- Index 55: **VEXIRK KING** — Vexirk boss

### Environment/Object Creatures (DM1 had nothing equivalent)
- Indices 0–12: Environmental objects (pillars, boulders, fountains, tables, cauldrons, braziers)
- Indices 19–22: Dynamic objects (Thorn Demon, Vortex, Flame Orb)
- Indices 37–38: Magical machines (Reflector, Power Crystal)
- Index 47: Void Door
- Index 56: Unknown obelisk variant

### New Monster Types
- **Thorn Demon** (19): Unknown attack pattern
- **Glop** (24): Unknown
- **Rocky** (25): Unknown
- **Thicket Thief** (27): Stealth-based
- **Tiger Striped Worm** (28): Worm-type
- **Treant** (29): Tree Gorgon
- **Dru Tan** (31): Unknown
- **Dark Vexirk** (48): Vexirk race
- **Wolf** (52): Canine
- **Mummy** (46): Undead
- **Vexirk King** (55): Elite Vexirk

---

## 4. Comparison with DM1 Creature List

DM1's 42 creature types (from ReDMCSB DATA.C creature tables) include: Giggler, Golem, Grond, Heavy Weapon, Hound, Hunchback, Wolf, Wraith, Zombie, plus environmental/objects. DM2 adds:
- Entire companion/minion system
- Lord Dragoth / Dragoth Minion hierarchy
- Vexirk race (Dark Vexirk, Vexirk King)
- Environmental machines (Reflector, Power Crystal, Amplifier)
- New monster archetypes (Treant, Thorn Demon, Veg Mouth, Mummy, Dru Tan, Wolf)

---

## STATUS: SOURCE-LOCKED

**Primary sources:**
- skproject/SKWIN/SkWinCore.cpp:741-810 (getAIName)
- skproject/SKWIN/SkGlobal.h:1007-1012 (CREATURE_AI_TAB_SIZE)
- skproject/SKWIN/DME.h:1505-1538 (AIDefinition struct)
