# CSB V1 - Combat Changes Audit

## Source Paths
- CSB: /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Attack.cpp, Character.cpp
- CSB: /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Projectile.cpp, Magic.cpp
- DM1: ReDMCSB PROJEXPL.C, DEFS.H, GROUP.C, BugsAndChanges.htm

## Projectile Speed Normalization (CHANGE7_20_IMPROVEMENT)
- File: PROJEXPL.C (both DM1 and CSB)
- DM1 bug: Projectiles moved slower on maps other than the party map
- CSB fix: Projectiles now move at same speed on all maps
- **Only non-bug-fix combat change in CSB**

## Magical Attack Sources (UNCHANGED from DM1)

DEFS.H:1679 defines C5_ATTACK_MAGIC sources:
Grey Lord, Lord Chaos, Lord Order, Materializer/Zytaz, Vexirk, Wizard Eye/Flying Eye

All unchanged from DM1 - Grey Lord added to DM1's existing list (not new to CSB,
Grey Lord itself is the CSB addition, per csb_creatures.md).

## Grey Lord Combat Behavior (CSB-specific)
- Attack.cpp:2423: Grey Lord monster type assignment
- Attack.cpp: IsLordChaosHere() also checks for Grey Lord proximity
- Chaos.cpp: Dedicated attack byte sequences for Grey Lord
- Grey Lord is a C5_ATTACK_MAGIC attacker
- Grey Lord attack pattern data in Chaos.cpp byte arrays

## Group/Creature AI Changes

### CHANGE7_19_FIX (GROUP.C)
- Fixed BUG0_69: Group movement/teleporter handling
- Related to Lord Chaos allowed map checks

### BUG0_09, BUG0_10 (DUNGEON.C - CHANGE7_17/18)
- Dungeon square event processing bugs fixed
- Affects creature group spawning/triggering

## Save Game Combat State
- CHANGE7_29: New saved game header format
- CHANGE8_12_FIX: Save/load fixes affecting combat state

## Combat System: No New Attack Types

- No new attack categories beyond DM1's existing types
- No new weapon types
- No new armor types
- Combat resolution algorithm appears identical to DM1

## Conclusion
CSB combat changes vs DM1 are MINIMAL:
1. Projectile speed normalized across maps (only gameplay difference)
2. Grey Lord (0x1a) is a new C5_ATTACK_MAGIC creature - its combat behavior
   is new content, but uses existing attack type infrastructure
3. Bug fixes for group movement, dungeon events, save/load
No new combat mechanics, attack types, weapons, or armor.
