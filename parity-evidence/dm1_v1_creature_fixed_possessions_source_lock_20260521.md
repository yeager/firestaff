# DM1 V1 Creature Fixed Possessions Source Lock (2026-05-21)

N2-only pass for the `TODO.md` creature type-specific behavior gap.

## Source Evidence

- `ReDMCSB_WIP20210206/Toolchains/Common/Source/GROUP.C:580-609`: `F0186_GROUP_DropCreatureFixedPossessions` selects fixed-possession tables for Skeleton, Stone Golem, Trolin/Antman, Animated Armour/Deth Knight, Rock/Rockpile, Pain Rat/Hellhound, Screamer, Magenta Worm/Worm, and Red Dragon.
- `ReDMCSB_WIP20210206/Toolchains/Common/Source/GROUP.C:610-645`: fixed-possession rows honor `MASK0x8000_RANDOM_DROP`, classify object info index ranges into weapon/armour/junk thing types, mark Animated Armour drops cursed, select source/random cells, move the new thing through `F0267`, and choose metallic vs wooden thud from whether any weapon dropped.
- `ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNGEON.C:518-557`: source tables `G0245` through `G0253` define the exact fixed possessions and random entries.
- `ReDMCSB_WIP20210206/Toolchains/Common/Source/DEFS.H:1597-1604`: creature attribute bit `MASK0x0200_DROP_FIXED_POSSESSIONS` marks creature types that use the fixed-possession path.

## Firestaff Scope

- Added `F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat` as a pure source-locked resolver in the DM1 V1 creature AI behavior module.
- The resolver returns the type/item/cell/cursed/source-order payloads needed by a runtime caller to materialize drops through the existing object/move path.
- This pass does not wire full dungeon thing allocation or moving-creature delayed drop queues.

## Verification Scope

- `tests/test_dm1_v1_creature_ai_behavior_pc34_compat.c` now covers cursed Animated Armour equipment, Rockpile random boulder/rock drops, and Red Dragon steak table/random entries.
