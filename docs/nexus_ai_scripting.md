# Nexus V1 Level Script AI - SDDRVS.TSK vs DM1 Hardwired Sensors

## Summary
Nexus V1 has NO level script system. No script VM, no SDDRVS.TSK equivalent, no trigger-based events.
Nexus V1 is data-driven: DGN files + MNS files + hardcoded behavior.
DM1 also has no script VM - its scripts are hardwired sensor/action pairs. SDDRVS/TSK is DM2.

## 1. What is SDDRVS.TSK?
Task/script system from Dungeon Master II (Skullkeep).
Provides: trigger conditions (door opened, switch pressed, creature killed, timer expired),
actions (spawn, set door, play sound, award XP, cast spell), conditionals (if/else, counters, delays),
persistence (state saved across save/load).

## 2. Nexus V1 Scripting Reality
Zero script files in Nexus V1 codebase.
All events hardcoded: nexus_v1_game.c, nexus_v1_dungeon.c.
No door event system, no timer triggers, no level-completion triggers.
data/ holds DM1-format dungeons only.

## 3. DM1 Scripting - Hardwired Sensor/Action Pairs
DM1 has rich hardwired behavior per creature type, no script VM.
Sensor inputs (all hardwired): smell range, vision/LOS, door sound, HP threshold, champion slot.
Action outputs (all hardwired): melee attack, projectile, move, flee, wander, steal.

## 4. DM2 Task System (SDDRVS.TSK)
DM2 task system operates at dungeon level: tasks tagged to squares,
events: ON_ENTRY, ON_EXIT, ON_CAST, ON_KILL, ON_TIMER,
actions: SPAWN, TELEPORT, SET_BLOCK, PLAY_SOUND, WIN_GAME.
Source: https://github.com/gbsphenx/skproject

## 5. Gap
| Feature             | Nexus V1  | DM1 (hardwired) | DM2 (SDDRVS) |
|---------------------|-----------|------------------|--------------|
| Level scripts       | None      | None             | Full task    |
| Conditional spawn   | None      | None             | ON_ENTRY     |
| Door/switch events  | None      | None             | Event-cond-action |
| Timer triggers      | None      | None             | Delay loops  |

## 6. What Nexus Needs
1. Task file format (.TSK or embedded in DGN)
2. Task VM - condition evaluator + action dispatcher
3. Triggers: square-entry, exit, creature-death, timer, door-open
4. Actions: spawn, teleport-party, set-block, play-sound, win/lose
5. Persist task state in save file
