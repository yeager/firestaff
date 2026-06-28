# Nexus V1 Level Script AI - SDDRVS.TSK vs DM1 Hardwired Sensors

## Summary
Current bounded status is tracked in
`docs/nexus_trigger_script_model_status.md`.

Nexus V1 has no proven `SDDRVS.TSK` trigger/script VM in Firestaff today.
`SDDRVS.TSK` is hash-verified as a 26,610-byte Saturn sound driver task, not
as a decoded trigger rule file. `SLEV00.BIN` through `SLEV15.BIN` remain
candidate per-level event data, but Firestaff has not parsed or source-locked
their trigger model.

DM1 has no script VM; its trigger behavior is hardwired sensor/action code.
DM2 also has no `SDDRVS.TSK` trigger VM; its dungeon events are data-driven
actuator records interpreted by fixed runtime code.

## 1. What is SDDRVS.TSK?
In the verified Nexus file catalog, `SDDRVS.TSK` is the Saturn sound driver
task:

- verified size: 26,610 bytes;
- verified hash: see `docs/VERIFIED_HASHES.md`;
- source-lock classification: `docs/source-lock/nexus_v1_phase2_data_formats_H2321.md`;
- audio context: `docs/nexus_audio_format.md`.

The older "5,448-byte `SDDRVS.TSK`" claim was a file mix-up with
`RHIFIX.BIN`, which is the 5,448-byte file in the verified hash catalog.

## 2. Nexus V1 Scripting Reality
Firestaff currently has Nexus trigger scaffolding, not a source-locked trigger
VM:

- `src/nexus/nexus_v1_script_vm.c` is explicitly a stub and does not parse real
  Nexus bytecode.
- `LEV*.DGN` grid parsing exists, but deeper 3D geometry and trigger/event
  ownership remain open.
- `SLEV*.BIN` files are verified real Nexus files and are plausible event or
  level-state data, but remain unparsed.
- No current probe proves a real Nexus door, trap, teleporter, stair, or
  level-completion event through a decoded script model.

## 3. DM1 Scripting - Hardwired Sensor/Action Pairs
DM1 has rich hardwired behavior per creature type, no script VM.
Sensor inputs (all hardwired): smell range, vision/LOS, door sound, HP threshold, champion slot.
Action outputs (all hardwired): melee attack, projectile, move, flee, wander, steal.

## 4. DM2 Trigger System
DM2's runtime trigger system is actuator-based, not `SDDRVS.TSK`-based.
Actuator records are stored in dungeon data and executed by fixed runtime code;
relay/counter/cross-map wiring provides puzzle sequencing without a general
script VM.

See `docs/dm2_triggers.md` and `docs/dm2_scripting.md`.

## 5. Gap
| Feature             | Nexus V1 current status | DM1 | DM2 |
|---------------------|--------------------------|-----|-----|
| Level scripts       | Unresolved; `SLEV*.BIN` unparsed | None | No script VM |
| `SDDRVS.TSK` role   | Sound driver task unless proven otherwise | N/A | N/A |
| Conditional spawn   | Not source-locked | Hardwired creature/data paths | Actuator generator |
| Door/switch events  | Not source-locked | Hardwired sensors | Actuators |
| Timer triggers      | Not source-locked | C006/countdown sensor paths | Actuator timing |

## 6. What Nexus Needs
1. Identify the real owner of trigger/event data (`SLEV*.BIN`, `DM.BIN`,
   DGN geometry records, or another task).
2. Add a bounded parser/probe only after record or opcode evidence exists.
3. Prove at least one real route: square-entry, door, stair, trap, teleporter,
   creature-death, timer, or level-completion.
4. Persist any proven event state in the Nexus save model.
