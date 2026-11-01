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

The authentic USA standard-PCE save-state map comparison is documented in the
[US state-map source lock](../source-lock/theron-us-authentic-standard-pce-state-map-binding-20261101.md).
It verifies saved CD-RAM page mapping only; it does not replace a cold capture
or prove the active map and tile consumer.

The cold JP reference route needed a four-second RUN hold at the System Card
prompt. A separate authentic capture confirms four-second Button I holds on
NEW GAME and FILE 1 enter the Akutuba story. Its `.mc0` snapshot is not in-game
BRAM; the captured 2 KiB BRAM remains identical to the known empty JP slot.
Native Firestaff hold timing, US title-action timing, and positive JP progress
saving remain unverified. See the
[source-locked title-action capture](../source-lock/theron-jp-four-second-title-action-capture-20261031.md)
and [empty-BRAM boundary](../source-lock/theron-original-backup-ram-body-layout-2026-09-23.md).

The latest visual follow-up remained at the PC Engine Super CD-ROM System
splash after a PID-targeted four-second RUN attempt. The PCE input trace
contained only released-button samples, and the wrapper rejected the run
because the System Card status trace had no `$1800` reads. An earlier cold
attempt did record 15 PID-bound presses and active-low RUN/I at poll `$E4C5`,
but no verified `$E11D` handoff, game-owned poll, CD IRQ, authenticated
CD-to-RAM receipt, or LBA 3674 read. Neither capture establishes whether the
sector is used by a successful game load. See the
[capture report](../source-lock/theron-jp-lba3674-pid-input-cold-capture-20261101.md).

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

The live-capture wrapper enforces an aggregate output budget of 1 GiB by
default. `THERON_CAPTURE_MAX_OUTPUT_BYTES` can set a positive limit from 1 MiB
through 1 TiB. The watchdog totals regular files with the configured trace
prefix and any separately configured `$23AD` trace; if the budget is exceeded,
it stops only that capture process group, leaves a `.output-budget-exceeded`
receipt, and rejects the package. This byte cap complements the wall-clock and
per-trace row limits; it does not make a truncated capture admissible.

Greatstone's Theron material concerns CSBWin custom-dungeon conversion and is
not PC Engine retail game data. DMWeb supplies edition and manual context.
Neither reference substitutes for authentic PCE input, state or display
evidence.
