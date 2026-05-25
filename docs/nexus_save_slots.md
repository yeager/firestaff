# Nexus V1 Save Slot System

## Status: NOT IMPLEMENTED

There are **no save slots** in the Nexus V1 codebase. No slot count, no slot
selection UI, no slot enumeration functions.

## What Exists

- `NEXUS_MAX_PARTY = 4` — max active champions in party (from `nexus_v1_champions.h`)
- `NEXUS_MAX_CHAMPIONS = 24` — total champion roster size
- `Nexus_V1_GameState` — tracks current level and party position only, no slot
  references

## In Context of Original Game

The original **Dungeon Master Nexus (Saturn)** saves were stored on:
- Saturn Memory Cartridge (battery-backed RAM)
- Saturn Memory Card (Backup RAM)

The game had **no visible "save slots"** in the UI in the same style as DM1.
Saves were automatic or triggered via the memory card UI (not in-game).

## Nexus V1 Engine Position

The V1 engine currently:
- Does not define any save slot count constant
- Has no slot management struct
- Has no file-based save mechanism

## Files Examined

- `include/nexus_v1_engine.h`
- `include/nexus_v1_game.h`
- `include/nexus_v1_champions.h`
- `include/nexus_v1_dungeon.h`
- `src/nexus/nexus_v1_engine.c`
- `src/nexus/nexus_v1_game.c`
- `src/nexus/nexus_v1_champions.c`
- `src/nexus/nexus_v1_dungeon.c`