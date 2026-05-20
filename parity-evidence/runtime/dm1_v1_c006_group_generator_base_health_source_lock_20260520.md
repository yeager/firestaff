# DM1 V1 C006 Group Generator Base-Health Source Lock (2026-05-20)

Scope: source-lock the creature base-health lookup consumed by the C006 floor group-generator path in `memory_runtime_dynamics_pc34_compat.c`.

ReDMCSB source evidence:

- `DEFS.H:1574-1594` defines `CREATURE_INFO`; `BaseHealth` is the eighth initializer field.
- `DUNGEON.C:668-733` defines the PC 3.4/I34E-compatible `G0243_as_Graphic559_CreatureInfo` table for creature types 0..26.
- `GROUP.C:481-548` implements `F0185_GROUP_GetGenerated`; `GROUP.C:530-533` indexes the creature-info table and computes generated HP from `BaseHealth`, the generator multiplier, and RNG jitter.
- `TIMELINE.C:912-1007` implements the C05 corridor-event processor; `TIMELINE.C:962-999` detects `C006_SENSOR_FLOOR_GROUP_GENERATOR`, resolves count/multiplier/delay, calls `F0185_GROUP_GetGenerated`, optionally disables the sensor, and schedules the C65 re-enable event.
- `TIMELINE.C:1009-1031` implements the C65 re-enable path by changing the first disabled sensor on the square back to floor sensor type C006.

Firestaff changes:

- Replaced the previous plausible `runtime_get_creature_base_health` table with exact `DUNGEON.C` PC 3.4/I34E values.
- Added focused probe invariants for jitter-free exact generator HP using type 7 (`BaseHealth=50`) and type 24 (`BaseHealth=255`).

Remaining gap: this source-locks generator health semantics inside the existing pure runtime-dynamics lane. Full C006 world insertion/runtime dispatch remains tracked as partial in `TODO.md`.
