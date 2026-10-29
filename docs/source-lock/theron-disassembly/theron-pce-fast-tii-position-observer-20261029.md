# Theron PCE Fast TII Position Observer — 2026-10-29

## Purpose and scope

The capture-only Mednafen observer records the original position transfer at
logical PC `$C1FA`. It does not change Firestaff's game runtime or establish
Theron movement parity. The source-locked instruction is:

```asm
tii $20B4,$2040,$0002
```

The hook is in `scripts/instrument_mednafen_theron_pce_fast_movement_context_trace.py`.
It emits a row only when the latched opcode PC is `$C1FA`, the active block
transfer is `IBM_TII`, MPR1 is `$F8`, and the destination is `$2040` or `$2041`.
The row carries the source and destination logical/physical addresses and MPRs,
the byte passed from `RdMem()` to `WrMem()`, the latched opcode PC and MPR, and
the remaining TII byte count.

## Source audit

This observer was checked against the isolated Mednafen 1.32.1 source used for
the reference build:

- `src/pce_fast/huc6280.cpp`, `BMT_TII` (line 1256 in the instrumented tree)
  calls `WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src))` before incrementing the
  source and destination. The helper therefore sees the addresses and source
  byte for the transfer currently being committed.
- `src/pce_fast/huc6280.cpp`, `WrMem` (lines 767–787), resolves the destination
  with `MPR[A >> 13]` and calls the mapped writer after the observer. The
  position addresses use MPR1 `$F8`.
- The CPU latches `TheronTraceInstructionPC` before decode (lines 1481–1482).
  A yielded TII resumes at `continue_the_TII` (line 1424), before the next
  opcode can replace that latch. The post-operand live PC is `$C203`; it must
  not label the writes.
- The PCE Fast BaseRAM writer in `src/pce_fast/pce.cpp` (lines 463–470) stores
  the passed byte at the mapped BaseRAM offset.

These source facts validate the hook's observation point and address
calculation. A runtime row is still required to validate the byte and MPR tuple
against an actual accepted movement.

## Authentic reference capture

A 10-second PCE Fast replay used the authentic JP Rev. 1 Track 02 cue (MD5
`85706e7c2f658bc2792511d618dfc7a5`), System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`), and emulator-created JP state (MD5
`d5c0daa227c04d55bdf20244803c00bd`). The input plan was
`right@1:140,down@2:33,i@145:5`. The state loaded and exposed controller input.
At frame 145, the original wrote `$2905=$04` at `$CCD8`; it cleared the command
at frame 151. The final BaseRAM snapshot retained position `$2040/$2041=02/03`.
The bounded movement-context trace recorded no `$2040/$2041`
`runtime_state_write` rows, and the gated `$C1FA` TII observer recorded zero
rows. The capture ended `BLOCKED` because save-state autoload bypassed the cold
CD IRQ and authenticated CD-to-RAM receipts; these bounded traces do not prove
that no position writes occurred outside their recorded rows.

This is negative capture evidence only. The save state reports position
`(2,3)` and bank bytes `$20DA/$20DB=01/00`, but the active JP dungeon map is
unresolved. As documented in
`../theron-pce-fast-akutuba-ram-capture-2026-10-03.md`, the same coordinate can
have different passability across candidate maps. The absence of position-write
rows in these bounded traces must not be attributed to a specific collision or
input defect.

The capture-only binary was rebuilt on TRV2 with `-j1`; SHA-256:
`507b1024181356f410634666175cb7a7060e900e3f11d27171dc90ebfcb6214c`.
The focused live-capture test passed three consecutive loops, as did Python
syntax validation and `git diff --check`. Private traces and original media
remain on TRV2 and are not committed.

## Remaining proof

Obtain a source-bound JP state whose active dungeon map and adjacent tile are
identified, then replay a command that the original accepts. A positive capture
must show the `$2905` command, the `$C1FA` opcode PC with its physical MPR,
both ordered TII bytes from `$20B4/$20B5` to `$2040/$2041`, and the resulting
position change against an identical no-input control. Until then, this
observer is verified tooling, not movement evidence.
