# CSB V1 — New Creatures (Source-Lock Audit)

**Audit date:** 2026-05-25
**Sources:** ReDMCSB DEFS.H:1339-1366 · CSBWin AsciiDump.cpp:306-344 · CSBWin Monster.cpp:15,200 · CSBWin Attack.cpp · CSB lineage Objects.h:153-183

---

## Monster Type Inventory

Both DM1 and CSB share **C027_CREATURE_TYPE_COUNT = 27** (0x00–0x1a).
The shared enum order is identical between games.

### Complete Creature Table

| Index | Hex  | Name (DEFS.H)              | Name (CSBWin AsciiDump) | Notes                        |
|-------|------|----------------------------|-------------------------|------------------------------|
| 0     | 0x00 | C00_CREATURE_GIANT_SCORPION | Scorpion                | Shared                       |
| 1     | 0x01 | C01_CREATURE_SWAMP_SLIME    | Slime_Devil             | Shared                       |
| 2     | 0x02 | C02_CREATURE_GIGGLER        | Giggler                 | Shared                       |
| 3     | 0x03 | C03_CREATURE_WIZARD_EYE     | Flying_Eye               | Shared                       |
| 4     | 0x04 | C04_CREATURE_PAIN_RAT       | Hellhound               | Shared                       |
| 5     | 0x05 | C05_CREATURE_RUSTER         | —                       | Shared                       |
| 6     | 0x06 | C06_CREATURE_SCREAMER       | Screamer                | Shared                       |
| 7     | 0x07 | C07_CREATURE_ROCK_ROCKPILE  | Rock_Pile               | Shared                       |
| 8     | 0x08 | C08_CREATURE_GHOST_RIVE     | Rive                    | Shared                       |
| 9     | 0x09 | C09_CREATURE_STONE_GOLEM    | Stone_Golem             | Shared                       |
| 10    | 0x0a | C10_CREATURE_MUMMY          | Mummy                   | Shared                       |
| 11    | 0x0b | C11_CREATURE_BLACK_FLAME    | Black_Flame             | Shared                       |
| 12    | 0x0c | C12_CREATURE_SKELETON       | Skeleton                | Shared                       |
| 13    | 0x0d | C13_CREATURE_COUATL         | Couatl                  | Shared                       |
| 14    | 0x0e | C14_CREATURE_VEXIRK         | Vexirk                  | Shared                       |
| 15    | 0x0f | C15_CREATURE_MAGENTA_WORM   | Worm                    | Shared                       |
| 16    | 0x10 | C16_CREATURE_TROLIN_ANTMAN | Ant_Man                  | Shared                       |
| 17    | 0x11 | C17_CREATURE_GIANT_WASP     | Muncher                 | Shared                       |
| 18    | 0x12 | C18_CREATURE_ANIMATED_ARMOUR | Deth_Knight            | Shared (naming diff only)   |
| 19    | 0x13 | C19_CREATURE_MATERIALIZER_ZYTAZ | Zytaz               | Shared                       |
| 20    | 0x14 | C20_CREATURE_WATER_ELEMENTAL | Water_Elemental        | Shared                       |
| 21    | 0x15 | C21_CREATURE_OITU           | Oitu                    | Shared                       |
| 22    | 0x16 | C22_CREATURE_DEMON          | Demon                   | Shared                       |
| 23    | 0x17 | C23_CREATURE_LORD_CHAOS     | Lord_Chaos              | Shared                       |
| 24    | 0x18 | C24_CREATURE_RED_DRAGON     | Dragon                  | Shared                       |
| 25    | 0x19 | —                           | "25"                    | **CSB-only, unused/placeholder** |
| 26    | 0x1a | C26_CREATURE_GREY_LORD      | Grey_Lord               | **CSB-only, new named boss** |

Sources: DEFS.H:1339–1366 · AsciiDump.cpp:306–344 · CSBWin Monster.cpp:200 (`ASSERT(...<27...)`)

---

## Analysis

### DM1 Creatures
25 types at indices 0–24. All carried forward into CSB unchanged.

### CSB-Only Creatures

**mon_25 (0x19):** Unused/placeholder. Assigned in `CreateMonster()` at Monster.cpp:200
(`ASSERT(DB4A3->monsterType() < 27,"monType")`), meaning the engine allows 0–26.
No attack data, no graphic loading, no spawn entries found. Likely a removed design slot.

**Grey Lord (0x1a):** Genuine new content.
- Named in CSBWin AsciiDump.cpp:44 and Objects.h:181
- Combat data in Attack.cpp:2423 (monster type assignment)
- Chaos.cpp: dedicated attack byte sequences
- DEFS.H:1679 — Grey Lord is a `C5_ATTACK_MAGIC` source (same category as Lord Chaos,
  Lord Order, Zytaz, Vexirk, FlyingEye)
- Grey Lord proximity check in `IsLordChaosHere()` widened to include Grey Lord

### New Creature Verdict
**Exactly 1 new named creature type in CSB: Grey Lord (0x1a).**
One unused placeholder slot (0x19) also present.

---

## Source Citations

| File | Lines | Content |
|------|-------|---------|
| ReDMCSB DEFS.H | 1339–1366 | C00–C26_CREATURE_* enum + C027_CREATURE_TYPE_COUNT |
| ReDMCSB DEFS.H | 1679 | C5_ATTACK_MAGIC sources including Grey Lord |
| CSBWin AsciiDump.cpp | 306–344 | MonsterName() — full 0x00–0x1a name table |
| CSBWin Monster.cpp | 200 | `ASSERT(monsterType < 27)` |
| CSBWin Attack.cpp | 2423 | Grey Lord monster type assignment |
| CSB lineage Objects.h | 153–183 | CSB creature type objects/definitions |