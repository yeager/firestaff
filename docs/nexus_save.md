# Nexus V1 Save/Load System

## Status: NOT IMPLEMENTED

The Nexus V1 engine has **no save or load functionality**. There are no
`nexus_v1_save_*` or `nexus_v1_load_*` functions defined anywhere in the codebase.

## Evidence

- `include/nexus_v1_engine.h` — no save/load function declarations
- `src/nexus/nexus_v1_engine.c` — `nexus_v1_init`, `nexus_v1_load_level`,
  `nexus_v1_tick`, `nexus_v1_shutdown` only; no persistence functions
- `src/nexus/nexus_v1_game.c` — `nexus_v1_game_init`, `nexus_v1_game_load_level`
  only; no save/load

## What Exists vs. What Would Be Needed

**Exists:**
- `Nexus_V1_GameState` struct: tracks `current_level`, `party_x`, `party_y`,
  `party_dir`, `game_started`, `data_dir`
- `Nexus_V1_ChampionPool` struct: tracks champions array, party slots, leader
- `Nexus_V1_Level` struct: tracks dungeon grid, geometry

**Would be needed for a save system:**
- Serialization of `Nexus_V1_GameState`
- Serialization of `Nexus_V1_ChampionPool` (champions, inventory, food/water)
- Dungeon progress (current level, explored state)
- Party position and direction

## Engine Version

This is the **V1** implementation — a reimplementation of the Dungeon Master
/Nexus (Saturn) engine. The original Saturn game stored saves on cartridge
memory or memory card (no manual save slots in the same sense as DM1).