# Theron PCE Fast Candidate Movement-State Write Observer — 2026-10-30

## Purpose and scope

The capture-only Mednafen observer records writes to candidate movement-state
bytes `$2040/$2041`, including instruction bytes and physical PC mapping. Their
gameplay meaning is not source-confirmed. The observer does not change
Firestaff's game runtime or establish Theron movement parity. An authentic JP
capture identifies an accepted transfer to these bytes at logical PC `$C1FC`:

```asm
tii $20B4,$2040,$0002
```

The hook is in `scripts/instrument_mednafen_theron_pce_fast_movement_context_trace.py`.
For ordinary `WrMem()` writes it requires MPR1 `$F8` and destination
`$2040/$2041`, logging them as `movement_state_write`. Direct zero-page writes
are recorded only when MPR1 is `$F8`,
the mapping used to derive their physical addresses. Instruction bytes are read
from PCE Fast's `FastPageR`, resolving the MPR slot separately for each byte.
A specific TII row additionally requires opcode PC `$C1FC`, `IBM_TII`, MPR1
`$F8`, and a destination in the candidate state range.
These hooks cover their instrumented `WrMem()` and selected direct zero-page
store/RMW paths; they are not a census of every possible physical RAM writer.

## Source audit

This observer was checked against the isolated Mednafen 1.32.1 source used for
the reference build:

- `src/pce_fast/huc6280.cpp`, `BMT_TII` (line 1256 in the instrumented tree)
  calls `WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src))` before incrementing the
  source and destination. The helper therefore sees the addresses and source
  byte for the transfer currently being committed.
- `src/pce_fast/huc6280.cpp`, `WrMem` (lines 767–787), resolves the destination
  with `MPR[A >> 13]` and calls the mapped writer after the observer. The
  observed candidate state destinations use MPR1 `$F8`.
- The CPU latches `TheronTraceInstructionPC` before decode (lines 1481–1482).
  A yielded TII resumes at `continue_the_TII` (line 1424), before the next
  opcode can replace that latch. The post-operand live PC is `$C203`; it must
  not label the writes.
- The PCE Fast BaseRAM writer in `src/pce_fast/pce.cpp` (lines 463–470) stores
  the passed byte at the mapped BaseRAM offset.

These source facts validate the hook's observation point and address
calculation. The runtime capture below supplies a positive movement-state write
observation; cold CD handoff and active map identity remain unresolved.

## Authentic reference capture

A PCE Fast replay used the authentic JP Rev. 1 Track 02 cue (MD5
`85706e7c2f658bc2792511d618dfc7a5`), System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`), and emulator-created JP state (MD5
`d5c0daa227c04d55bdf20244803c00bd`). The input plan was
`ii@470:140,up@471:120`. At frame 471 the original wrote `$2905=$03` at
`$CC99`, then cleared it at `$D39D` in frame 472. Two position writes followed
at logical `$C1FC`: `code_bytes=73b420`, block move `IBM_TII`, with
`$20B4->$2040=03` and `$20B5->$2041=03`. Destination physical addresses were
`$1F0040/$1F0041` under MPR1 `$F8`; opcode physical PC was `$0DC1FC` under MPR6
`$6E`. The TII-specific rows independently record source bytes `$03` at
`$1F00B4/$1F00B5` and matching destination commits. The final PCE Fast RAM
snapshot has `$2040/$2041=03/03`, while the loaded state had `02/03`. A
save-state reader interprets this pair as coordinates changing from `(2,3)` to
`(3,3)`, but this capture does not source-confirm that semantic interpretation.

The capture ended `BLOCKED` because save-state autoload bypassed the cold CD
IRQ and authenticated CD-to-RAM receipts. This is a positive emulator
reference capture of the original movement-state transfer, not a cold Track 02
handoff, active-map identification, collision result, or Firestaff parity
proof. Private trace SHA-256 values: input
`230b92771f5fa2201621a4721a2109ee86758e6dd0163af84ee6b4862c0643cd`, final
RAM `4f8cf26de11bd330524509f9d8ea8c52a24d0f4685d5eec31bbc6bd7dfd493f7`,
movement context
`738f6df0ea8ef0b2d513723d15c0446f3d679d497c41282a2a80500faf56242c`, and
transition receipt
`3ebe932324e4706a7f69cbe26bf4af2f2df14b81f488699bccb634319aa4dbd5`.

The save-state reader reports `(2,3)` and bank bytes `$20DA/$20DB=01/00`, but
the meaning of the candidate fields and the active JP dungeon map are
unresolved. As documented in
`../theron-pce-fast-akutuba-ram-capture-2026-10-03.md`, the same
save-state-interpreted coordinate can have different passability across
candidate maps. The capture does not identify which map or prove collision
behavior.

The instrumented binary was rebuilt on TRV2 with `-j1`. The focused
live-capture test passed three consecutive loops, as did Python syntax
validation and `git diff --check`. Private traces and original media remain on
TRV2 and are not committed.

## Cold-start scripted input replay (2026-10-30)

A separate PCE Fast capture cold-started the same authentic Japanese Rev. 1
CUE and headered System Card 3.0 with no state autoload. The CUE's Track 02
matched MD5 `b7afb338ad31be1025b53f9aff12d73a`; the runtime System Card body
matched `38179df8f4ac870017db21ebcbf53114`. A scripted Run hold and the input
plan based on the previously verified new-game/story route, followed by
`ii@13600:140,up@13601:120`, produced 13 apply receipts, each followed by a
port-0 controller read containing its scripted mask. The verifier saw two game/non-System-Card poll boundaries.

At frame 13601, the movement-context trace recorded the same two `$C1FC`
`TII $20B4,$2040,$0002` commits: source `$1F00B4/$1F00B5` contained `$03`, and
the writes reached `$1F0040/$1F0041` under MPR1 `$F8`. The new shared
`event_sequence` places the general `WrMem` rows at 1,555,276 and 1,555,278,
and their TII-specific observations at 1,555,277 and 1,555,279. These are
separate diagnostic rows for the same two physical stores, not one identifier
per CPU event. The counter only advances at enabled trace hooks; it is not a
complete CPU instruction or bus-event clock. The rows now retain all MPR0–MPR7
values, opcode MPR, and physical PC mapping.

The capture process ended with `BLOCKED: dynamic receipts absent`; it recorded
no CD IRQ, authenticated CD-to-RAM receipt, or `$E009` dispatch. Scripted masks
and the two movement-state transfer rows therefore do not establish the
verified story-to-dungeon route for this instrumented run, active map, tile
collision, visible movement, coordinate semantics, or native Firestaff parity.
The broad physical RAM write trace reached its 1,048,576-row limit and was
truncated. Private traces remain on TRV2 under
`/dev/shm/firestaff-theron-event-seq-20261030/capture/`; no media or raw trace
was copied into the repository. Movement-context trace SHA-256:
`7287556c22aaea642cd30ec9734a17c591620a59ca9d9cdc73ce93e6c68c223f`;
transition receipt SHA-256:
`f98ca3d8f12143661af792278cb75fbee8dc4297d2256da60bf2ccc6938b67a6`.

## Shared diagnostic event sequence

Movement-context v3 rows read and increment
`TheronTraceCDSharedEventSequence`, a counter shared with the PCE Fast CD and
memory trace hooks in the instrumented Mednafen build. The earlier capture's
input sidecar lacked `event_sequence`, so those guest reads could not be
ordered against movement-context rows. The corrected 2026-10-31 PCE Fast hook
records `event_sequence` on `pce_input_read`; scripted input-apply rows still
lack it. The new JP comparison therefore orders guest reads against TII
transfers using the guest-read and movement-context rows. Selected
`input_latch_write` rows emitted by this observer also carry the shared
sequence. The
`build_mednafen_theron_irq2_trace.sh` build pipeline applies
`mednafen_1.32.1_theron_pce_fast_cd_shared_event_sequence.patch` before this
instrumenter. Source audit points to `src/pce_fast/huc6280.cpp`, `WrMem`
(instrumented lines 767–787), `BMT_TII` (line 1256), and
`continue_the_TII` (line 1424): `BMT_TII` reads the source then commits through
`WrMem`, while the latched instruction PC remains the issuing TII across
yield/resume. The counter orders observed hook rows only; disabled hooks,
untraced instructions, DMA, and other uninstrumented events do not receive
sequence values. Two hook rows may describe one store and consume adjacent
values.

## Dynamic instruction context — 2026-10-31

The capture-only observer now retains a 128-record PCE Fast instruction
window ending at the `$C1FC` target opcode (127 earlier instructions plus the
target) and flushes it when the candidate TII transfer is observed.
Three state-only replays from the same authentic JP Akutuba F5 state produced
identical instruction-context hashes. They show the same bounded control-flow
path through `$C1D6` (`JSR $C14E`), return to `$C1D9`, and the conditional
sequence at `$C1E9`–`$C1FA` falling through to `$C1FC`. The dynamic history
therefore improves on the earlier missing-instruction-context limitation; it
does not provide a source-level meaning for the checked RAM fields or identify
the active map/object. See the [control-flow capture](theron-pce-fast-tii-control-flow-20261031.md)
for exact hashes, instruction rows, and provenance.

This still does not disambiguate the player-movement hypothesis from the
source-locked Track 02 teleporter-destination path. The capture remains
state-only, without authenticated cold CD handoff, active-map identity,
collision/passability evidence, a paired rendered response, or native
Firestaff parity.

## Follow-up comparison — 2026-10-31

Three authentic-state action replays and three matched controls repeat the
input-conditioned TII transfers from `$20B4` to candidate `$2040/$2041` after
Button II+UP guest reads; Button II-only controls preserve the prior bytes.
This supports a repeatable state-transfer observation, not confirmed
coordinates, collision outcome, visible movement, or native parity. All six
captures are state-only and lack authenticated CD-to-RAM provenance. See the
[review-format comparison bundle](../../../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md)
for per-run hashes and limits.

## Remaining proof

Identify the active dungeon map and adjacent tile for this JP state, then
correlate the accepted step with a cold-start authenticated Track 02 capture
and verify the same control against the exact Firestaff implementation. This
observer is verified reference tooling and a positive original
movement-state transfer capture; it does not establish the fields' gameplay
meaning or full movement parity.
