# Nexus V1 Autosave

## Status: NOT IMPLEMENTED

There is **no autosave** functionality in Nexus V1. No timer-based saves,
no level-change triggers, no event-driven persistence.

## What Code Shows

- `nexus_v1_tick()` in `nexus_v1_engine.c` — game loop tick at 55ms / 18.2 Hz.
  Currently empty (stub). No save logic present.
- `nexus_v1_load_level()` — level loader. No autosave on level transition.
- No `autosave` keyword anywhere in the Nexus source tree.

## Comparison to DM1

DM1 (the reference engine) has a similar gap — the DM1 engine in Firestaff
also lacks a save system. This is consistent with both engines being
early-phase reimplementations focused on rendering, movement, and combat.

## Future Consideration

An autosave system would logically trigger on:
- Level change (entering new dungeon floor)
- Party death/revival
- End of game session (shutdown hook)

No such hooks exist yet.