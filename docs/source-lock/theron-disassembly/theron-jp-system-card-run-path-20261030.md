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

## Follow-up transfer and code-fetch capture

A second cold PCE Fast reference capture used the same hash-verified Japanese
Rev. 1 Track 02 and System Card 3.0, with run@480:300 and no save state. The
instrumented Mednafen build included bounded PC $EA9C writer records and
instruction-byte snapshots for execution in $4000-$5FFF. The new verifier
checked all source reads against the authentic Track 02 bytes and joined
writer events by the shared diagnostic sequence.

The verifier reported 63,488 source-bound CD data-port reads and zero
unbound reads. All 53,248 PC $EA9C CD-RAM writes had an ordered/value
candidate match to a recent PC $EA99 source read. Repeated bytes can make
these joins ambiguous; they do not prove unique read-to-write causality. It
recorded 262,144 code-fetch rows; 262,121 rows were fully backed by prior
traced writes, with 786,386 matching bytes and 46 bytes differing from the
latest traced write to their address.
The 46 mismatching bytes span 23 fetch rows, five physical addresses in banks
$80 and $82, and three logical PCs. Each address has a prior $EA9C write,
but no earlier traced $EA9C write at that address has the fetched value.
An untraced later writer or mapping/trace semantics could explain this; the
capture cannot distinguish them. The physical addresses were $1000D9 (2
bytes), $1000DA (1), $1000DB (1), $10468D (21), and $10468E (21); logical
PCs were $40D7 (1), $40D9 (3), and $468C (42). Slot-2 MPR was $80 at the
first three addresses and $82 at the last two, with no observed mapping
change. The PC $EA9C-only writer observer did not capture other guest writers. The follow-up below traces guest writes at these five addresses; neither capture proves a complete transfer. The capture was blocked with exit 124 and still had no game IRQ,
authenticated CD transfer receipt, game-owned input poll, or gameplay
transition. It provides loader and execution provenance only, not parity.

Private capture directory on TRV2:
/home/trv2/firestaff-theron-evidence/capture/theron-jp-run-writer-opcode-cold-20261030/.
Trace SHA-256 values:

- CD writer trace: efc717b6842e4a7c5d28717c339878cc0b258ce6e2ac207b05768f0d5d50e0ef.
- CD data-port reads: e5d4506ad3e9cb1b1fa5ffafe7670f6566132ea4ec9f98f82aebbada2258218f.
- System Card dispatch and code fetches: cd8d07bf81c47beac46e2a314bb169e8cc0c0fe1ddf4a1c115ab158e82a80651.
- Transition receipt: 35c1dfc18874283cc96dc1994ce1eeb4f89621b90cb12e08c92ba3abdee110b6.

The raw media and trace files remain private and are not part of the repository.

## Follow-up guest-write and instruction capture

A second cold capture used the same authenticated Japanese Rev. 1 Track 02
(MD5 b7afb338ad31be1025b53f9aff12d73a) and System Card 3.0, again with
run@480:300 on input slot 0 and no save state. It ran for 45 seconds and
remained blocked before the game IRQ, authenticated CD transfer receipt, and
gameplay transition.

The target-write observer was extended to include the five previously
mismatching physical bytes in banks $80 and $82, using the existing PCE Fast
HuCRAMWrite/HuCRAMWriteCDSpecial guest-write path. This includes HuC6280
block-move writes routed through PCEWrite; it is not a claim that every
hardware-side DMA path is observed. The trace recorded 6 writes at each of
$1000D9-$1000DB and 33 at each of $10468D/$10468E. Of those, 76 writes came
from PCs other than $EA9C. The bounded target-write trace contained 170
records total.

The new verifier scripts/verify_theron_jp_fetch_target_writes.py correlated
the shared event sequence of target writes and instruction-byte fetches. It
found 228 fetched bytes at the five addresses: all 228 had a prior observed
write whose latest value matched; there were zero bytes without a prior
observed write and zero latest-value mismatches. The observed code also
explains why the earlier $EA9C-only comparison was incomplete:

- At $1000D9-$1000DB, a TII block move at $4055 used source $40D9 and
  destination $60D9 while both windows mapped to bank $80, so those writes
  aliased the same physical bytes. Later writes placed $4C 48 61 there and
  rewrote them to $EA before subsequent fetches.
- At $104684, the fetched instruction bytes were 8D 8D 46 (STA $468D); at
  $104689, they were 8D 8E 46 (STA $468E). Those instructions wrote $E0 and
  $47 to $10468D/$10468E. The later fetch at $10468C was E3 E0 47,
  identifying $E3 as the HuC6280 TIA opcode and the two observed bytes as the
  start of its $47E0 source operand. The opcode name and block-move behavior
  are cross-checked against Mednafen 1.32.1 src/pce/dis6280.cpp and
  src/pce/huc6280.cpp.
- The target observer only establishes guest writes and their event ordering
  at these five addresses. It does not bind each write causally to a unique
  disc byte, establish the full loader, or prove that the resulting transfer
  entered gameplay. The original source-bound reader/writer verifier still
  reports 46 differences when only $EA9C writes are considered.

Private capture directory on TRV2:
/home/trv2/firestaff-theron-evidence/capture/theron-jp-run-writer-all-cdram-writes-cold-20261030/.

Trace SHA-256 values:

- CD-RAM target writes: 774be364738016734385dac9420afa95ebf01d2ffb3aa9f71dca067b1c821157.
- System Card dispatch and code fetch: 994b62a14607981414287200c9fffcad5cfc41602118db5380e543c44875a918.
- Transition receipt: c11a40eb017f43c2df869458167680fc32fc5a9f03e053de9a6bb6ae9d068ae6.

The raw media and traces remain private and are not included in Git.

## Remaining evidence needed

Continue from the authenticated bank `$80` writes: bind the following source
bytes and transfer into the active game loader, capture the first game-owned
controller poll and its resulting state change, then compare native Firestaff
input and rendering against that authenticated route. Keep save-state captures
separate from cold-boot loader evidence.

## TIA byte transfers at PC `$468C`

A third cold reference capture used the authentic Japanese Rev. 1 Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`), System Card 3.0 (MD5 `ff1a674273fe3540ccef576376407d1d`), and scripted `run@480:300` input without a save state. A bounded observer at the PCE Fast `BMT_TIA` macro records the decoded block operands and each actual `RdMem` then `WrMem` pair. The source reference is `src/pce_fast/huc6280_ops.inc:543-544` (`LD_BMT(BMT_TIA)`) and `src/pce_fast/huc6280.cpp:463` (`BMT_TIA`).

The capture recorded 32 complete executions with identical decoded operands: source `$47E0`, destination `$0002`, encoded length `$0040`. Each copied 64 bytes from logical `$47E0-$481F`; source MPR slot 2 was `$82` (mapped `$1047E0-$10481F`). Writes alternated between logical `$0002` and `$0003`; destination MPR slot 0 was `$FF` (mapped `$1FE002/$1FE003`). The 2,048-row verifier passed, including source increments, destination alternation, remaining lengths, MPR slots, and mapped addresses. These are observed bus/memory values and mappings; they do not identify the destination's game-level meaning.

The run still ended before a game IRQ, an authenticated CD-to-RAM receipt, a game-owned input poll, or a gameplay transition. No TIA read in this capture is joined to an authenticated CD source receipt. This is emulator/disassembly evidence only; it does not establish a complete loader or native gameplay parity.

## TIA zero-page access trace

A fourth cold reference capture used authentic Japanese Rev. 1 Track 02 (MD5
`b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`), with Run held for four seconds and no
save state. The reference-only PCE Fast observer records logical $02/$03 reads
and writes while the System Card routine runs in $4600-$46FF. Its hooks cover
`RdMem`/`WrMem`, direct zero-page loads, stores and read-modify-write
instructions, and pointer bytes loaded by `GetIND`, `GetIX`, and `GetIY`
in Mednafen 1.32.1 `src/pce_fast/huc6280.cpp`. The unmodified 1.32.1 source
places `RdMem` at line 145, `WrMem` at 161, and `GetIND`/`GetIX`/`GetIY` at
393/405/417 respectively.

The combined capture contains 32 complete TIA transfers from $47E0 to the
alternating $0002/$0003 destinations (2,048 byte rows) and 2,240 zero-page
records. It includes direct reads at PC $466F from $02 and PC $4674 from $03.
The trace verifier confirms increasing shared event sequence values, contiguous
row and move indexes, byte-width values, logical-to-physical mappings, and
complete transfers with no truncation.

Capture path on TRV2:
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-tia-zero-page-holdrun4-x11-cold-20261030/`.

- TIA trace SHA-256: `1e0ac4ec23a330fd8646cc3eab0438378eefa2e8cbb3af91aaefb33f41b406c2`.
- Transition receipt SHA-256: `43c40e418050025779aeefa36a09cc4f442eaba6512ac6d00dca0083e4c3024f`.

The receipt reports `cd_irq_callbacks=0`,
`authenticated_cd_ram_receipts=0`, `main_ram_loader_tii_transfers=0`, and
`transition=missing`. This is System Card loader instrumentation evidence;
it does not establish game launch, gameplay, or native parity. Raw traces,
game media, and firmware remain private and are not included in Git.
