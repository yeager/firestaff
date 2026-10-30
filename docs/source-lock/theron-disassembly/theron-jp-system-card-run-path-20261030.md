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

A read-only latest-writer comparison across the same capture resolves all 46
bytes against later observed guest writes: PCs $613A, $613F, and $6144 wrote
the three bytes at $1000D9-$1000DB, while PCs $4684 and $4689 wrote the two
bytes at $10468D/$10468E. The existing opcode-fetch trace covers logical
$4000-$5FFF, so it does not show the instructions at $613A/$613F/$6144. This
accounts for the earlier mismatch without establishing which authenticated
disc bytes supplied those stores or what the resulting code does.

Private capture directory on TRV2:
/home/trv2/firestaff-theron-evidence/capture/theron-jp-run-writer-all-cdram-writes-cold-20261030/.

Trace SHA-256 values:

- CD-RAM target writes: 774be364738016734385dac9420afa95ebf01d2ffb3aa9f71dca067b1c821157.
- System Card dispatch and code fetch: 994b62a14607981414287200c9fffcad5cfc41602118db5380e543c44875a918.
- Transition receipt: c11a40eb017f43c2df869458167680fc32fc5a9f03e053de9a6bb6ae9d068ae6.

The raw media and traces remain private and are not included in Git.

## Remaining evidence needed

The extended cold capture below records the writer instructions around
$613A/$613F/$6144, but it reaches its bounded code-fetch cap and does not bind
those stores uniquely to authenticated Track 02 bytes. Trace a bounded repeat
that retains this writer path and its source reads, then establish the source
and caller before assigning game-level meaning. Continue from the authenticated
bank $80 writes into the active game loader, capture the first game-owned
controller poll and its resulting state change, and compare native Firestaff
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

## 2026-10-30 — capture later JP loader writers through `$7FFF`

A fifth 45-second cold reference capture used the authentic Japanese Rev. 1
Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`), System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`), and `run@480:300` without a save state.
The reference-only code-fetch observer now keeps a separate bounded stream for
logical `$6000-$7FFF`, independent of the existing `$4000-$5FFF` stream.

The authenticated CD reader verifier reported 63,488 Track 02-backed reads
and zero unbound reads. All 53,248 `$EA9C` writer values had preceding `$EA99`
read candidates, with zero writer values lacking a candidate; repeated byte
values make those read/write candidates non-unique. The combined code-fetch
trace contains 262,144 rows from each logical window. Both streams reached
their configured row caps. The original capture instrumentation did not mark
dropped rows, so completeness past either cap is unknown. A later observer
revision adds explicit one-shot truncation markers and the verifiers reject
traces containing them. The `$EA9C`-only latest-writer comparison retains 46
mismatching fetched bytes, as recorded for the preceding capture.

The new window records instructions in physical bank `$81` around the later
stores. At `$6138`, bytes `A9 4C` load `$4C`; `$613A` contains `8D D9 40`
(`STA $40D9`). The same pattern at `$613D/$613F` stores `$48` at `$40DA`, and
at `$6142/$6144` stores `$61` at `$40DB`. With MPR2=`$80`, these stores map to
physical `$1000D9-$1000DB`. The guest-write verifier confirms that all 228
fetched bytes at the five tracked addresses have a prior observed write with
the latest fetched value, with zero mismatches or missing bytes.

After those stores, the fetch at `$40D9` records `4C 48 61`, a `JMP $6148`
through the newly written bytes; a later fetch at `$40D9` sees `EA EA EA`.
The HuC6280 opcode mapping is checked against Mednafen 1.32.1
`src/pce/dis6280.cpp:111` (`RTS`), `:123` (`JMP`/`JSR`), `:195` (`LDA`), and
`:208` (`STA`). This establishes a short-lived code write and fetch, not the
disc-byte source, loader purpose, or a gameplay consumer.

The capture again ended with no CD IRQ callback, authenticated CD-to-RAM
receipt, or game-owned input poll (`transition=missing`). It is emulator and
disassembly evidence only; it does not prove game launch or native parity.
Raw traces, game media, and firmware remain private on TRV2 at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-code-fetch-6000-window-cold-20261030/`.

- CD data-port reads SHA-256: `27f420ec0232b425b38d43975e16fe9c1f536281585672656aa4906b0e233641`.
- CD-RAM target writes SHA-256: `0d52667a9376624099a87f4036b7e8384c681674253150b2ca7710f2ffb36228`.
- CD-RAM writer-PC trace SHA-256: `03105bd07dadc5c781411adcf3b8ec70417a5a51c8fedc36875da6bd72a54487`.
- System Card dispatch and code fetches SHA-256: `841f76db4cc9b6911edde3380048bde1b2f19c9f7e8b902cdd15908b2f815fc8`.
- Transition receipt SHA-256: `65b7b35d440243940299616d04f530c947727b31df839b38d3d386ca5252ef5c`.

## 2026-10-30 — verify code-fetch truncation markers

A repeat 45-second cold capture used the same authentic JP Rev. 1 Track 02
(MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`), with no save state and the observer
revision that emits one-shot overflow markers. Each logical code window
recorded 262,144 rows, then emitted exactly one marker on the first subsequent
fetch: `$4000-$5FFF` at event 389,967 and `$6000-$7FFF` at event 652,194.
Both source-bound and target-fetch verifiers rejected the trace as incomplete.
The earlier capture's exact-cap row counts alone could not distinguish a full
trace from dropped rows; this run confirms both windows continue beyond their
caps under the same cold route.

The receipt again reports zero CD IRQ callbacks, zero authenticated CD-to-RAM
receipts, zero game-main-RAM `$E009` dispatches, zero loader TII transfers, and
`transition=missing`. The marker behavior improves capture completeness
reporting; it adds no evidence of game launch or gameplay and does not prove
native parity. Raw traces, game media, and firmware remain private on TRV2 at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-code-fetch-cap-marker-cold-20261030/`.

- System Card dispatch and code-fetch trace SHA-256: `4badfc34dfddf0786cc6021712d726f084810ad0e92179ff7cc33ce159e89437`.
- CD data-port read trace SHA-256: `27f420ec0232b425b38d43975e16fe9c1f536281585672656aa4906b0e233641`.
- CD-RAM target-write trace SHA-256: `2d6c2df36a271a654605bd3cbf133baa0849017925caa486f39a29596c6a4461`.
- Transition receipt SHA-256: `c8fe028fb07f052c2e64217c071fcb188baa62f5aa6d096ce32664f85c4e2b84`.

## Targeted v3 code-fetch rerun

A further cold PCE Fast capture used the authentic JP Rev. 1 Track 02 and
System Card 3.0, with no save state. Its v3 dispatch trace records only
logical PCs `$40D9`, `$6138`, `$613A`, `$613D`, `$613F`, `$6142`, and `$6144`;
the narrower scope avoids the overflow caused by the earlier broad
`$4000-$5FFF` and `$6000-$7FFF` windows. The v3 dispatch trace contains 37
basic fetch rows and 6 extended fetch rows, with no explicit code-fetch
truncation marker.

The source-bound verifier authenticated 262,144 Track 02 data-port reads and
reported zero unbound reads. It correlated 131,072 `$EA9C` writer records
against those reads; 106,496 had an ordered value candidate, while 24,576 did
not. Among 43 executed fetch rows, 126 bytes matched the latest captured
`$EA9C` write, 3 differed, and 42 rows were fully backed by prior captured
writes. A separate guest-write comparison found 111 fetched bytes at
`$1000D9-$1000DB`; all had a prior matching guest write and all 37 target
fetch rows were fully backed. The older `$10468D/$10468E` candidate addresses
were not fetched in this targeted scope and are reported as zero fetches.
These bounded joins do not prove a unique disc-to-write transfer or gameplay
handoff.

The capture replay applied all 11 scheduled input events, but the wrapper
stopped as `BLOCKED` when the default 1,048,576-row input trace filled before
the last event had a matching controller-port read. The data-read and
`$EA9C`-writer traces also reached their configured row caps. Therefore the
fetch trace is complete for its targeted rows, while the surrounding input
and source chronology is bounded and incomplete. This run still establishes
no game-owned input poll, game IRQ, authenticated CD-to-RAM receipt,
`$E009` dispatch, or gameplay transition. It adds trace evidence only, not
native parity.

The focused v3 verifier test covers accepted complete v3 rows, v2/v3
truncation rejection, and absent candidate addresses. The four private trace
SHA-256 values are, in order, dispatch, `$EA9C` writers, data-port reads, and
guest target writes:

- `5a9352107570c865c8e0b015d9d5817a247ee543b067815f9c889881c1c4b2d2`
- `8449ed86a2bed54efa7f8f65dab3eabcfbc66e601101939a55365bff249c96db`
- `a5949ffd7299a9454e4c538b67920edfddb92c071e9a6d4dbc1bae7c15895f5d`
- `42bc77ea7b18ec603533ec317928864c4b8472312a13770208d22304e301a1a2`

The original media and raw traces remain private on TRV2; they were not
copied into the repository.

## 2026-10-30 — targeted cold consumer and poll trace v4

A further cold PCE Fast reference capture used the same hash-verified Japanese
Rev. 1 Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0
(MD5 `ff1a674273fe3540ccef576376407d1d`). No save state was loaded. The
instrumentation narrowed code fetches to the five candidate boundaries
`$40D9`, `$42AB`, `$42B2`, `$44C1`, `$44D2` and the seven writer PCs
`$6138/$613A/$613D/$613F/$6142/$6144/$6148`. The input trace budget was raised
to 8,388,608 reads; the CD read, CD-RAM target-write and writer-PC limits were
each raised to 1,048,576.

The trace contains 15,351 consumer-boundary fetch rows plus seven extended
writer fetches (15,358 total), with no code-fetch truncation marker. Counts
were 37 at `$40D9`, 6,144 each at `$42AB/$42B2`, 1,511 at `$44C1`, and 1,515
at `$44D2`; each extended writer PC occurred once. The source-bound verifier
accepted 483,328 reads from authentic Track 02 with zero unbound reads. All 327,680
observed `$EA9C` writer bytes had ordered, value-matching read candidates.
Across the five previously tracked target addresses, 111 fetched bytes had a
preceding matching observed guest write, with zero missing or mismatching
latest writes. The broader fetch/write comparison found 46,071 matching bytes,
three bytes differing from the latest observed writer, and 15,357 fully backed
fetch rows. The guest-write observer is bounded to the hooked write paths; it
is not an exhaustive bus-writer census.

All 11 scripted input events were followed by a System Card poll read at
`$E4C5`; no non-System-Card poll was observed. The capture ended at its 250
second timeout and the transition receipt remains `missing`: zero CD IRQs,
zero authenticated CD-to-RAM receipts, zero `$E009` dispatches, and no gameplay
transition. The main-RAM consumer observer reached its own 65,536-sample cap.
This closes the input-delivery timing gap for the scripted cold replay and
extends bounded writer/fetch evidence. It does not prove that `$42AB/$42B2` or
the extended writer code is game-owned, establish a complete CD transfer, or
demonstrate native Theron's Quest parity.

The temporary capture directory on TRV2 was
`/dev/shm/theron-system-card-targeted-v4-cold-20261030/`; it was removed after
verification and recording these hashes. Raw media and trace files are not in
Git.

- Dispatch/code-fetch trace SHA-256:
  `8ec140737bd07dab0e0da005d5469b998b4c141117aec75ecccbd46bcdbe63d7`.
- Scripted input receipt SHA-256:
  `0db985e4c82ae234134f74655e53e98756c9da6d990a5b01a87fe08927eb8068`.
- Transition receipt SHA-256:
  `5bb329081e17d14defb4d2fa6e31c5c550e47005367afa34d23eeba7003a59e4`.
- Source-bound CD read trace SHA-256:
  `d0758a82fb9bd02668116e5b339b84f18834307413e32ad5c8a3f8022e35262f`.
- CD-RAM writer-PC trace SHA-256:
  `61e9addccbed7772c1c9b6d756ebcd79864fad0c02fab174e63bc0b5382a6335`.
- CD-RAM target-write trace SHA-256:
  `ab1edc5fb6456e243857c727b6cef04f64c3c287997e9b6269cf57829d2fc123`.
- Input trace SHA-256:
  `679ac83c4334b21e9f5e4a7ee3b7fe610daa1d640df5f9515253eaf1fd79ff79`.

The raw media and traces remain private and are not included in Git.
