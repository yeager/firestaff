# Theron reference-capture runbook

This page records the supported boundaries for local Theron reference
captures. Firestaff itself runs Theron in its native C/SDL runtime; Mednafen
and disassembly tools are diagnostic references only. Keep authentic game
media, System Card firmware, save states, captures, emulator builds and trace
sidecars outside the repository.

## Current status

Use the [capture-readiness record](THERON_CAPTURE_READINESS.md) for the
current native/runtime boundary and source-locked capture evidence. The
[real-data inventory](THERON_REALDATA_INVENTORY.md) lists the verified Track
02/19 identities and loader constraints. The
[input guide](theron_input.md) documents Firestaff's native keyboard and
mouse controls.

The cold JP reference route needed a four-second RUN hold at the System Card
prompt, followed by ordinary short menu/story presses. The captured 2 KiB
BRAM matched the known empty JP slot. This does not verify a long hold on the
selected title-menu action, positive JP progress saving, or native gameplay
parity. See the [source-locked empty-BRAM boundary](../source-lock/theron-original-backup-ram-body-layout-2026-09-23.md).

## Capture requirements

- Start from authentic, hash-verified regional media and retain the CUE/BIN
  ownership and sector mapping used by the capture.
- Use an isolated emulator profile and output directory. Never point captures
  at Firestaff's save directory or modify source media.
- Record whether the run is cold or loaded from a save state. State loading
  bypasses the CD-to-RAM transfer that populated data already present in the
  saved state. Assess any later observed transfer separately.
- Keep input scripts, guest reads, RAM/VRAM/VDC/VCE observations and presented
  frames in one session when making an action or renderer claim. Label an
  unjoined observation as a boundary, not parity evidence.
- Do not derive production tile mappings, hitboxes, movement semantics or
  screenshots from synthetic fixtures or a blocked-wall state.

## Diagnostic tooling

Capture tooling may build instrumented Mednafen outside the repository and
run bounded trace verifiers. The resulting binary and traces are not runtime
dependencies and do not prove game semantics by themselves. Consult the
dated reports linked from [capture readiness](THERON_CAPTURE_READINESS.md)
for exact options, hashes and result scope; machine-specific retired paths
are intentionally not reproduced here.

Greatstone's Theron material concerns CSBWin custom-dungeon conversion and is
not PC Engine retail game data. DMWeb supplies edition and manual context.
Neither reference substitutes for authentic PCE input, state or display
evidence.
