# Nexus V1 Scripting Status

This page summarizes the current scripting/trigger evidence and defers the
`SDDRVS.TSK` question to `docs/nexus_trigger_script_model_status.md`.

## Current Status

Nexus trigger scripting is unresolved in Firestaff.

- `SDDRVS.TSK` is verified at 26,610 bytes and is currently classified as a
  Saturn sound driver task.
- The older 5,448-byte `SDDRVS.TSK` claim was a mix-up with `RHIFIX.BIN`.
- `SLEV00.BIN` through `SLEV15.BIN` are real per-level supplementary files.
  Firestaff now validates their shared 36-byte SH-2 entry spine and records
  bounded literal/RTS/call-shape receipts for the complete real 16-file
  corpus. The task bodies remain opaque; no event rule or action meaning is
  assigned.
- `src/nexus/nexus_v1_script_vm.c` is a receipt/profile boundary for the real
  task shape plus an isolated provisional rule-table test API. It is not a
  source-locked Nexus script dispatcher.

## DM1 and DM2 Baseline

DM1 has no runtime scripting language. Its dungeon behavior is hardwired in
tile-type dispatch and sensor/action code.

DM2 is data-driven but still not script-driven. Its runtime trigger behavior is
encoded in actuator records and executed by fixed runtime handlers. Relay,
counter, and cross-map actuator wiring provide sequencing without a general
script VM.

## Nexus Open Questions

The remaining Nexus work is to identify the real owner of event behavior:

1. `SLEV*.BIN` may contain per-level event or state records interpreted by
   `DM.BIN`.
2. Some square semantics may be hardwired or table-driven inside `DM.BIN`.
3. DGN geometry or metadata may carry some trigger attachment state.
4. `SDDRVS.TSK` may have a secondary role beyond sound, but that is not proven.

## Implementation Boundary

Until a future disassembly, parser, or real-data route probe proves the model,
Firestaff should describe Nexus trigger support as:

> DGN grid parsing and gameplay scaffolding exist; the real Nexus
> trigger/script model is not decoded. `SLEV*.BIN` files have a bounded,
> source-verified SH-2 task-entry profile but no decoded event ownership, and
> `SDDRVS.TSK` remains classified as a sound-driver task.

Do not publish opcode tables, rule counts, or `.TSK` trigger claims as facts
without a corresponding source-lock artifact.

## Promotion Criteria

Promote this status only when a future change provides one of:

- a documented disassembly path from `DM.BIN` or a Saturn task into a real
  event dispatcher;
- a bounded parser/probe that consumes real `SLEV*.BIN` or `SDDRVS.TSK` bytes
  and identifies stable records without invented opcode names;
- a real-asset runtime probe proving a specific route, such as a teleporter,
  stair transition, door event, trap, or level-completion trigger, and citing
  the owning data/executable path.
