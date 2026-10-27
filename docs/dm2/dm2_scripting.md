# DM2 V1 Scripting: SDDRVS.TSK and DM1 Hardwired Sensors vs DM2 Data-Driven Actuators

## Important Correction: What is SDDRVS.TSK?

SDDRVS.TSK is **NOT** a trigger/sensor scripting file. It is the **sound driver task** (26 KB, SHA256 68890ee4a49fd0c341bc3f0a48643e4db4b175df0d7dacfeb88306340052b6) — a Nexus/DM1 executable module that handles sound playback. It belongs to the Nexus/DM1 remake project, not DM2 trigger system.

The trigger/task asked about SDDRVS.TSK as a possible script driver for DM2 triggers — this was a misattribution.

## DM1 Sensor Scripting Model

DM1 had **no runtime scripting for triggers**. Sensor behavior was hardwired in the game binary:

- **Wall switches**: Compiled code checked is player facing wall, is there a switch tile, activate — switch type was baked into binary
- **Pressure plates**: Similar hardwired logic per floor tile type
- **Bitfield triggers**: Used a bitfield array where specific dungeon events set/read bits — simple flag system, not scripting
- **SLEV00-15.BIN files**: Level script/event data — contained dungeon layout and event flags, but logic interpreting them was hardwired in DM.BIN

DM1 had a simple **event flag system** stored in SLEV*.BIN files — flag words per dungeon level that could be set/cleared by actions. This was the closest DM1 got to data-driven events, but read by hardwired code in DM.BIN.

## DM2 Scripting: No Script Files for Runtime Triggers

DM2 has **no SDDRVS.TSK equivalent for trigger scripting**. The trigger system is purely **data-driven** via actuator record configuration in the dungeon data file.

### What GDED Has Instead

GDED (the DM2 dungeon editor) has configuration/scripting-like features:

#### 1. Descriptor CSV Files

GDED loads Descriptor*.csv files that define:
- **Main category / Sub-category** — how elements are organized in editor tree
- **Type codes** — what an element does (spell, creature, item, etc.)
- **Options** — available options per element type
- **Programming** — possibly Forth script fragments for editor

The CDM2DescItem and CDM2DescriptCtx classes parse these CSV descriptor files.

#### 2. Forth Interpreter in GDED

GDED includes a minimal **Forth interpreter** (ForthInt.h / ForthInt.cpp):
- Stack-based language with integer operations
- Supports comparison (=) and custom words via Process() override
- Used by CDM2EntForth to evaluate descriptor matching conditions

CDM2DescriptCtx::FindItem() and FindItem2() use this to match dungeon entries against descriptor rules — **editor UI feature**, not runtime game logic.

#### 3. Dynamic Selection Rules

CDM2DescItem::fDynSel (dynamic selection) — when HasProg() returns true, the descriptor has a Forth script that determines whether an element matches. This drives editor features like highlighting all floor tiles adjacent to water — editor-side only.

### The Actual DM2 "Scripting"

DM2 equivalent of scripting is the **actuator wiring graph** built in GDED:

1. Place an actuator record on a floor or wall cell
2. Set its **floor type** (what triggers it: PARTY, ITEM, CREATURE, etc.)
3. Set its **actuator type** (what happens: SHOOTER, DOOR_OPEN, CREATURE_GENERATOR, etc.)
4. Optionally wire it to a **counter** or **relay** chain for multi-step logic
5. Optionally wire it to a **CROSS_MAP** actuator to span dungeons

This is a **declarative data model**, not a procedural script. There is no script text file that defines when player steps on plate A, fire projectile B after 500ms.

## Why No Scripting?

The absence of a scripting language in DM2 runtime was a design choice: the actuator type enumeration was rich enough to cover all dungeon puzzle/event needs without requiring general-purpose programming. Complexity is in the **combination** of actuators, not in custom code.

Compare to DM1 where SLEV00-15.BIN contained event flags and flag-trigger logic decoded by hardwired code — DM2 made this explicit in the data model but kept the execution engine fixed.

## Data Flow Summary

Dungeon Data File (DUNGEON.DAT)
  |-- Tile records (floor/wall/door/object)
  |-- Actuator records (dbActuator category, 8 bytes each)
  |       |-- actuator type (from ACTUATOR_TYPE_* enum)
  |       |-- target (coordinates / record ID)
  |       |-- floor type (from ACTUATOR_FLOOR_TYPE__*)
  |       |-- wall type (for wall actuators)
  |-- GDAT graphics / ornate data
  |-- Descriptor CSV (editor only, not loaded at runtime)

No .TSK or script file is read at runtime for trigger logic. All event logic is encoded in actuator records and the fixed c_tim_proc.cpp execution engine.

## GDED Forth vs DM1 Event Flags

| Aspect | DM1 SLEV/BIN Event Flags | DM2 Actuator Data Model |
|--------|------------------------|------------------------|
| Storage | Separate level script files | In DUNGEON.DAT actuator records |
| Logic | Hardwired in DM.BIN | Fixed execution engine in SKULLWIN |
| Modification | Hex edit SLEV files | GDED UI |
| Flexibility | Very limited (on/off flags) | Rich actuator types + relay chains |
| Cross-level | Not possible | CROSS_MAP actuator |
