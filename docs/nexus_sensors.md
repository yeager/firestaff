# Nexus V1 Sensors / Triggers Audit

## 1. DM1 Sensor Model: Hardwired in Game Loop

DM1 has NO separate script system. All game logic is hardwired in the EXE:

Hardwired sensors in DM1:
- Wall collision (square 0): blocking logic in movement handler
- Door interaction (type 8): toggle open/close on forward command
- Stairs (types 2/3): level transition on step
- Alarm trap (type 6): set all creature alertness to 255
- Chute/trapdoor (type 7): force party to next level
- Teleporter (types 9/10): instant position warp
- Pit (type 4): remove champion from party
- Pillars (type 12): block creature pathfinding
- Water (type 13): prevent movement unless champion has ROPE
- Fire (type 14): 1 HP damage per step unless champion has PROTECT

These are all compile-time behavior baked into the game loop (game.c).

## 2. Nexus Sensor Model: SDDRVS.TSK Script VM

Nexus Saturn uses a separate script file SDDRVS.TSK (5,448 bytes) for
event triggers. This is a task/script VM approach, more flexible than DM1.

Script file SDDRVS.TSK:
- Size: 5,448 bytes
- Contains declarative rules: WHEN condition THEN action
- Processed by a script virtual machine in the Nexus engine
- Allows designers to place any event on any square without type constraints

Script system advantages over DM1:
1. Multiple events per square (DM1: one type per square)
2. Conditional logic (DM1: pure boolean triggers only)
3. No compile-time constraints (DM1: type codes are baked in)
4. Events can be chained (DM1: single immediate effect)
5. Debug scripts can be hot-swapped without recompiling EXE

## 3. Sensor Comparison: DM1 vs Nexus

| Sensor Aspect     | DM1                   | Nexus V1                |
|-------------------|-----------------------|-------------------------|
| Sensor storage    | Square type byte      | SDDRVS.TSK + sq type    |
| Sensor count      | Fixed (~15 types)     | Variable (unbounded)     |
| Trigger logic     | Hardwired C code      | Script VM               |
| Conditional events| No                    | Yes (TSK conditionals)   |
| Multiple events/sq| No                    | Yes                     |
| Designer iteration| Slow (recompile)      | Fast (script edit)      |
| Teleporter logic  | Types 9-10            | SDDRVS.TSK rule         |
| Trap logic        | Types 6-7-11          | SDDRVS.TSK rule         |
| Door logic        | Type 8                | SDDRVS.TSK + geometry   |
| Level scripts     | None (inline)         | SDDRVS.TSK              |
| Creature spawn   | Hardwired level data  | Scripted spawn event    |

## 4. Firestaff Implementation Status

Current implementation in firestaff:
- nexus_v1_dungeon.c: grid parsing only, no script integration
- SDDRVS.TSK: identified as script source, but no parser implemented
- nexus_v1_engine.c: basic init plus file loading, script VM is TODO

The SDDRVS.TSK parser is the main gap in sensor/trigger support.
Script VM would need:
1. TSK file parser (opcode plus operand format to reverse-engineer)
2. Condition evaluator (square proximity, champion state, item held)
3. Action dispatcher (teleport, spawn, sound, door toggle, etc.)

## 5. Script VM Design (Target for Firestaff)

Based on the SDDRVS.TSK size (5,448 bytes), the script format is likely:

Header: version, entry point count
Entries: [condition_size, condition_data, action_size, action_data]

Conditions might include:
- PARTY_ON square coordinates
- CHAMPION_HAS item
- SQUARE_TYPE == N
- LEVEL_LOADED == N
- CREATURE_COUNT > N

Actions might include:
- TELEPORT x,y,level
- SPAWN creature_type, x,y
- SOUND track_number
- SET_SQUARE_TYPE x,y,type
- award_gold, award_item, trigger_door
