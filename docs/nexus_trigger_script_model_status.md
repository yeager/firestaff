# Nexus Trigger / Script Model Status

This note reconciles the older Nexus trigger docs that disagreed about
`SDDRVS.TSK`. It is intentionally a bounded status document, not a new
implementation plan.

## Current Decision

Firestaff should not describe `SDDRVS.TSK` as a proven Nexus trigger/script VM.

Verified repository evidence classifies the file as a Saturn sound driver task:

| File | Verified size | Evidence |
|------|---------------|----------|
| `SDDRVS.TSK` | 26,610 bytes | `src/engine/firestaff_known_hashes.c`, `docs/VERIFIED_HASHES.md`, `docs/source-lock/nexus_v1_phase2_data_formats_H2321.md` |
| `RHIFIX.BIN` | 5,448 bytes | `src/engine/firestaff_known_hashes.c`, `docs/VERIFIED_HASHES.md` |
| `SLEV00.BIN` ... `SLEV15.BIN` | 2,388 ... 11,660 bytes | `src/engine/firestaff_known_hashes.c`, `docs/VERIFIED_HASHES.md` |

The older "5,448-byte `SDDRVS.TSK`" statement was a file mix-up: 5,448 bytes
matches `RHIFIX.BIN`, not `SDDRVS.TSK`.

## What Is Proven

- `SDDRVS.TSK` exists in the Nexus Track 1 data set and is hash-verified at
  26,610 bytes.
- Source-lock docs and audio docs identify it as a Saturn sound driver task
  used around `SNDLEV*.SAL`, `SNDLEV*.MAP`, CD-DA, or related sound playback.
- Firestaff has `nexus_v1_script_vm` scaffolding, but it is a stub. It does
  not parse real `SDDRVS.TSK` bytes, does not decode real opcodes, and should
  not be cited as runtime parity evidence.
- `SLEV00.BIN` through `SLEV15.BIN` are real per-level supplementary files.
  They remain plausible event/level-state inputs, but their format and runtime
  owner are not proven in Firestaff.

## What Is Not Proven

- No repository evidence currently proves that `SDDRVS.TSK` stores dungeon
  trigger rules.
- No opcode table for `SDDRVS.TSK` is source-locked. Older `COND_*` and
  `ACT_*` tables in Nexus docs are hypotheses and must not be treated as
  reversed facts.
- No Firestaff probe proves that party movement, doors, traps, teleporters,
  stairs, or creature events are dispatched through a `SDDRVS.TSK` trigger VM.
- DM2 does not use `SDDRVS.TSK` for trigger scripting; its runtime trigger
  system is the actuator data model documented in `docs/dm2_triggers.md` and
  `docs/dm2_scripting.md`.

## Working Model Until Reversed

Use this wording in status docs:

> Nexus trigger/script ownership is unresolved. Firestaff currently has DGN
> grid parsing and bounded runtime scaffolding, but no proven Nexus
> trigger-script parser. `SDDRVS.TSK` is treated as a sound-driver task unless
> disassembly or a real-data probe proves a trigger role. `SLEV*.BIN` files are
> candidate per-level event data, still unparsed.

This keeps three possibilities open without over-claiming:

1. Trigger/event data may live in `SLEV*.BIN` and be interpreted by `DM.BIN`.
2. Some square semantics may be hardwired or table-driven in `DM.BIN`.
3. `SDDRVS.TSK` may still have a secondary task role, but that requires proof
   beyond filename, size, or analogy.

## Promotion Criteria

Do not promote Nexus trigger support beyond "unresolved/stub" until a future
change provides at least one of:

- a documented disassembly path from `DM.BIN` or `SDDRVS.TSK` into a real event
  dispatcher;
- a bounded parser/probe that consumes real `SLEV*.BIN` or `SDDRVS.TSK` bytes
  and identifies stable records/opcodes without invented tables;
- a real-asset runtime probe proving a specific trigger route, such as a known
  Nexus teleporter, stair transition, door event, trap, or level-completion
  event, and citing the owning data file or executable path.
