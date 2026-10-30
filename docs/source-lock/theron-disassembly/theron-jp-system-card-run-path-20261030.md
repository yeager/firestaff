# Theron's Quest JP System Card Run path trace — 2026-10-30

## Purpose

Trace a cold start through the authentic Japanese System Card 3.0 controller
path using the native PCE Fast core in Mednafen 1.32.1. This is emulator
reference evidence only; it does not add an emulator dependency to Firestaff.

## Authentic inputs

- Japanese Rev. 1 Track 02: `TQJP02.bin`, MD5
  `b7afb338ad31be1025b53f9aff12d73a`.
- System Card 3.0: 262,656-byte image, MD5
  `ff1a674273fe3540ccef576376407d1d`.
- Cold replay: `run@480:300` on input slot 0. Replay lengths count scripted
  input updates and are not guaranteed wall-clock durations. No save state was
  loaded.
- Private TRV2 capture:
  `/home/trv2/firestaff-theron-evidence/capture/theron-jp-run-port0-route-cold-20261030/`.

## Trace instrumentation

`mednafen_1.32.1_theron_system_card_controller_dispatch_trace.patch` adds an
opt-in trace at the System Card controller branch and its reset/Run path. It
records the current and edge controller bytes, enabled-pad mask, `$E7`, vector
target, selected BIOS PCs, and registers. RAM values are read through the
emulator's mapped pages without calling the instrumented guest-read wrapper,
so the observations do not add false consumer-read events. Output is capped.

The complete Theron Mednafen patch set, including this trace patch, applied to
an isolated source copy. A serial `-j1` build succeeded on TRV2. The Linux
build skipped the script's macOS-only SDL runtime linkage check.

## Dynamic observations

The cold path entered System Card `$E0F3` and called `$C860`. At `$C8C7`, the
CPU accumulator held `$08`, matching the authentic disassembly's `CMP #$08`
Run branch at `$C8C7-$C8D2`. The trace then returned from `$C860` with `$A=$00`
at `$E10C`, entered `$E118/$E11B`, and later executed at `$4000` with MPR6
values `$84` and `$6B`. The capture therefore proves the Run menu branch and
subsequent code execution, but does not identify the `$4000` code as Theron
gameplay.

The same cold run recorded 63,488 source-bound CD data-port reads and 85
CD-RAM writes. The existing verifier
`verify_theron_jp_stage2_bank80_candidate_trace.py` passed: two 15-byte windows
from authentic Track 02 at LBAs 4521 and 4589 were each read at `$EA99` and
immediately written byte-equal to bank `$80` at `$EA9C`. This binds those 30
bytes to the disc and target writes; it does not establish the full loader or
all CD-RAM provenance.

The transition receipt remains `missing`: the capture records zero CD IRQ
callbacks, zero authenticated CD-to-RAM receipts, zero game `$E009` dispatches,
and no game-owned controller poll. Do not claim game launch or parity from
this BIOS/code-loader trace.

## Private trace hashes

- Controller and BIOS path trace SHA-256:
  `6e8ac6859c7ed19a3a28907dc79dd4536ab1c6f5e5c0add5c86e649c56dbc254`.
- Scripted-input receipt SHA-256:
  `e5b55779ee71aa31f7a9611dbcab4298a464b883978b20d1dfedf6fee319561c`.
- Transition receipt SHA-256:
  `c8b3091d885cfb8627be2df01f0463361fba3be40560388799f8ba3294e05306`.

## Remaining evidence needed

Continue from the authenticated bank `$80` writes: bind the following source
bytes and transfer into the active game loader, capture the first game-owned
controller poll and its resulting state change, then compare native Firestaff
input and rendering against that authenticated route. Keep save-state captures
separate from cold-boot loader evidence.
