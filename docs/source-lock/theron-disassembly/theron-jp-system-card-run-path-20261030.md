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

## Bounded follow from the authentic `$6148` fetch

A further cold PCE Fast reference capture used authentic Japanese Rev. 1
Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0
(MD5 `ff1a674273fe3540ccef576376407d1d`), with scripted `run@480:300` input
and no save state. The optional code-follow trace begins at logical PC `$6148`
and records instruction bytes, physical fetch addresses, MPR registers, and a
shared event sequence for up to 256 rows. The completion marker is emitted on
the subsequent instruction-fetch hook, so it confirms that execution advanced
beyond the final recorded instruction. The verifier reported
`status=complete rows=256`; the trace contains no cap or truncation marker.

The same run captured 32 complete TIA transfers (2,048 byte rows) and 128
v2 zero-page accesses. The TIA transfer verifier passed three consecutive
runs. It requires an explicit `space=zp` field and validates that logical
`$2002/$2003` accesses resolve through MPR slot 1 (MPR1=`$F8`) while
preserving offsets `$02/$03`. The trace SHA-256 values are:

- Code-follow trace: `32dc1fde9e78dd6f3c21a61a5a7f5117d80fb2393f1bab26ec2643773048e1d4`.
- TIA trace: `98d75b1f9ef1771a42d8fb661be0f19b8ecb24b9a77b5349df887cbae1bcefe7`.
- Transition receipt: `5e53a6a9243d2556fba3414b7dc48cd6286a70b5446a849b87c750a493e94bc0`.

The scripted event was observed by the System Card poll at `$E4C5`; there was
no non-System-Card poll. The receipt reports zero CD IRQ callbacks, zero
authenticated CD-to-RAM receipts, and no gameplay transition. Thus this capture
validates code-follow completion and the corrected MPR1 trace encoding, but
does not establish a complete loader, native handoff, or gameplay parity.
Raw traces and media remain private on TRV2 under
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-code-follow-v6-cold-20261030/`.

### Observed control flow in the 256-row follow

The first recorded instruction at `$6148` is `JSR $6109`. The observed
subroutine executes `LDA #$EA` and three `STA` instructions to `$40D9-$40DB`,
then returns. The caller executes `SEI`, initializes the stack with
`LDX #$FF` / `TXS`, enables interrupts with `CLI`, writes `$40` and `$60` to
`$1C/$1D` (pointer `$6040`), and jumps to `$40DC`. The following bytes decode
as `CLY; LDA ($1C); ASL A; TAX; JMP ($410D,X)`. The captured path later visits
`$4319`, calls `$41B9`, and executes indirect dispatch through `$410D` again.
These opcode names and modes match the Mednafen 1.32.1 HuC6280 table in
`src/pce/dis6280.cpp`: `JMP` at line 123, `TAX` at 130, `CLY` at 242,
`LDA` at 195, and `STA` at 208.

The trace subsequently enters the low physical `$04xx` window at logical
`$E4xx` with MPR7=`$00`, and the transition log records the controller poll at
`$E4E1`; scripted input is read at `$E4C5`. This confirms return to the System
Card input loop after the observed indirect dispatch. The targeted source-chain
verifier below matches the two indexed table bytes and the three fetched `$4319`
bytes to authentic Track 02 reads and their immediately following `$EA9C`
writer-PC records. It does not observe guest writes from every PC and therefore
does not prove exclusive memory provenance. It also does not establish a game
IRQ, complete loader transfer, or entry into the native game loop. Continue
from these byte matches into a game-owned input poll.

## Correlate the `$410D,X` target with executed code

The optional PCE Fast observer instruments the existing low/high `RdMem`
results in the HuC6280 opcode `$7C` handler, then records the resolved target
before `SetPC`. It is restricted to `$40E1`, where the v8 trace fetched
`7C 0D 41`; it performs no extra emulated memory reads. The observer stores the
table base, X index, effective logical address, both physical mappings, bytes,
MPR snapshot, and shared event sequence. The verifier only treats rows whose
sequence falls within the bounded 256-row `$6148` code-follow window as
correlatable.

On authentic Japanese Rev. 1 Track 02 (MD5
`b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`), the `run@480:300` cold capture emitted 40
dispatch rows. Exactly one occurred within the code-follow window. At event
102677, PC `$40E1` used base `$410D` and X=`$24`, read `$19/$43` from logical
`$4131/$4132`, mapped through MPR2=`$80` to physical `$100131/$100132`, and
resolved `$4319`. The next code-follow fetch, event 102678, was logical
`$4319` at physical `$100319`, matching the computed destination. Three
consecutive verifier runs reported one in-window match and 39 dispatch rows
outside the follow window; those later rows were not used to infer targets.

The transition receipt still reports zero CD IRQ callbacks, zero aggregate
authenticated CD-to-RAM receipts, and no non-System-Card input poll. The
separate offline verifier `scripts/verify_theron_jp_indirect_jump_source_chain.py`
checks the authentic Track 02 file by size and MD5, verifies each source offset
and byte, joins each shared read event to its immediately following byte-equal
`$EA9C` RAM write, and requires that write to be the latest traced `$EA9C` store
at the destination before the consumer. `verify_read_rows` confirms the read PC
is `$EA99`. The writer trace only records writes from `$EA9C`, so unobserved
writes from other guest PCs can still affect the destination; the match is not
exclusive causal provenance. On this capture it verified:

| Source read / RAM write events | Track 02 raw file offsets | Guest physical bytes |
| --- | --- | --- |
| 31301 / 31302, 31303 / 31304 | 2,876,817–2,876,818 | `$100131-$100132`: `19 43` |
| 32277 / 32278, 32279 / 32280, 32281 / 32282 | 2,877,305–2,877,307 | `$100319-$10031B`: `A5 1C 48` |

The computed `$4319` destination and its first three fetched bytes match these
source reads and latest logged `$EA9C` writes in this capture. This does not
establish exclusive provenance, a complete loader, native handoff, or gameplay
parity. Raw media and traces remain private on TRV2
at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-indirect-jump-v8-cold-20261030/`.

- Indexed indirect-jump trace SHA-256: `f4d5775622ac1b71aca7a222c3b742a160ed1ce7239a81fb7135444d062a0050`.
- System Card code-follow trace SHA-256: `de1866c771e40f3c8dc0029b61c7142b5f47585618d6d2ba7629cb7f8baca5e9`.
- TIA trace SHA-256: `1e9c420ec9dda89e8db75ed3169c36c3fb32fb560b7d5e292ded127129490b7c`.
- Transition receipt SHA-256: `b1c282cd31302ec407894ca6818e21d996bef0900c53f99cb5da69b23759c01b`.

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

## 2026-10-30 — authentic v9 indexed-dispatch follow capture

A cold PCE Fast reference run used the authentic Japanese Rev. 1 Track 02
(MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`), with scripted `run@480:300` on input
port 0 and no save state. The bounded follow observer recorded up to 64
instruction fetches per `$40E1` indexed dispatch, with a 64-window and
4,096-row limit. The instrumented Mednafen 1.32.1 build compiled on TRV2 with
PCE Fast enabled and `-j1`; capture output and trace data stayed on TRV2.

The direct verifier accepted 40 dispatch rows and matched one target to the
earlier bounded `$6148` code-follow. The new sidecar contained 2,560 rows in
40 windows; every window reached its 64-instruction cap, with no open or
skipped windows. The direct, follow, and source-chain verifiers each passed
three runs against the authentic capture and Track 02. A cap is not a return
or handoff marker.

MAME `unidasm -arch h6280 -basepc <fetch-PC> -count 1` decoded each actual
fetch row independently. The first `$41C5` window executes `BSR $41B9`, then
`INY`/`LDA ($1C),Y` operations that update the `$1C/$1D` cursor, followed by a
`JMP $40E4` path that advances that cursor and branches back to `$40CC`.
Another observed target at `$4214` reads through `($1C),Y` and stores values
at `$4EC1`, `$4D7B`, and `$3009`. This is consistent with a record or byte
stream walk; that purpose is an inference, not a source- or game-ownership
claim. The bounded runtime excerpt is
[`theron-jp-indirect-jump-follow-v9.asm`](theron-jp-indirect-jump-follow-v9.asm).

Track 02 source verification accepted 63,488 CD data-port reads with zero
unbound reads. The `$EA9C` observer recorded 53,248 writes. Comparing fetched
bytes to the latest observed `$EA9C` write at each physical address found
4,064 matching bytes, one mismatch, and 3,615 bytes with no prior observed
`$EA9C` write. Each of the 4,064 matches had an ordered same-value `$EA99`
Track 02 read candidate within 16 shared events, but repeated values and
unobserved writer paths prevent a unique read-to-write causal claim. The
`$EA9C` observer is not an exhaustive guest-writer census.

The transition receipt remains `missing`: the run ended at its 45-second
bound after 22,308 PCE input transactions; the single scripted event was read
at `$E4C5`, with no non-System-Card poll, zero game IRQ callbacks, and zero
aggregate authenticated CD-to-RAM receipts. This capture adds authentic loader-path
and instruction evidence only; it does not establish the native handoff,
playable Theron gameplay, or parity.

Private TRV2 capture directory:
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-indirect-jump-follow-v9-cold-20261030/`.
Raw media and traces remain private and are not part of the repository.

- Main trace SHA-256: `2ee7799214c3c15d6eb0caaf9ae46833b1b0b0eb76f899ebbe8c5cf52b4c9636`.
- Indexed-dispatch trace SHA-256: `2e3c39a584751292faf3b0c3dfefae4333adc76fff314edc2f0546902101c631`.
- Per-dispatch follow trace SHA-256: `40a42cd12a895e57af4279951d1b48599683cee6cf182b75f9e9631e0e58fd47`.
- System Card code-follow trace SHA-256: `b1d15e2a79e427d9cc04cf1a065ddfe4fd6ccd9ada1eca5fab43645d111c17e6`.
- CD data-port read trace SHA-256: `a7767564ce8fca1bf8127ecea5d1602a758f44fbc5172c77e4b259e3467ab438`.
- CD-RAM writer-PC trace SHA-256: `fbaa2ceb03f20a993accb01fa317e7081429883eab73cc71a9f39ac9c2ff3de2`.
- Scripted-input receipt SHA-256: `3eb8673b3772f19acf27296a258c8e6379a0d921d8d659c0ddec88ae1b346871`.
- Transition receipt SHA-256: `b9665870a377bdb090442091f8f5a7036285e9e119eea4cba897e303d83d9a5d`.

## 2026-10-30 — authentic v10 128-fetch dispatch follow

A cold PCE Fast reference capture repeated the authentic JP Rev. 1 Track 02
(MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`) run with `run@480:300` on input port 0
and no save state. The follow observer raised its per-dispatch limit from 64
to 128 fetches (64 windows, 8,192 total rows). The PCE Fast instrumented
binary was built on TRV2 with `-j1`; raw media and traces remain on that host.

The direct-dispatch, bounded-follow, and authenticated Track 02 source-chain
verifiers each passed three consecutive runs. The direct trace has 40 rows,
one in-window next-fetch match, and 39 out-of-window rows. The follow trace has
5,120 rows across 40 windows: all 40 hit the 128-row instruction cap, none
remain open, none were skipped, and no window completed. The five source-chain
joins bind the two dispatched table bytes and three target-fetch bytes to
authentic Track 02 reads and their latest observed `$EA9C` writes. They do not
establish exclusive provenance or cover other guest writer paths.

Disassembly of the actual fetch rows for the `$41C5` target shows the pointer
cursor path entering System Card controller polling at `$E4A5-$E4DE`; the
scripted input is read at `$E4C5`. This confirms System Card polling only. The
45-second receipt reports 22,348 input transactions, zero host key events, one
scripted event read at `$E4C5`, zero non-System-Card polls, zero CD IRQ
callbacks, zero authenticated aggregate CD-to-RAM receipts, zero game
main-RAM `$E009` dispatches, and `transition=missing`. The capture therefore
does not establish game ownership, a native gameplay handoff, or playable
parity.

Private TRV2 capture directory:
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-indirect-jump-follow-v10-cold-20261030/`.

- Indexed-dispatch trace SHA-256: `449d1ec9fbac23ae12d4dd375f93dbadafe11b5eacd1b0b4665bd8aac5d5235e`.
- Per-dispatch follow trace SHA-256: `994cebab24840df76f8efb66c4f9d00e1ec11acfdff681cd2bc1bfb7d292cab1`.
- System Card code-follow trace SHA-256: `5f80ab6f9b3bc61ecb1e1a42981411363333b46c5d1106bbc65b3a8454e53ba5`.
- CD data-port read trace SHA-256: `01c6c6b1854ce89fab4874dc3e786f00b582f194a652fbc82d74c7d8f8e3b07c`.
- CD-RAM writer-PC trace SHA-256: `30c3645c50e05078a80ab7ad20572d0617dd61e0ee4bf0bb7e90c0b9a28175fe`.
- Scripted-input receipt SHA-256: `256e04200c239c15443ccfc87aaa2a977ded1d30961918b63f730a413c28b917`.
- Transition receipt SHA-256: `d2613970df2cbf32952a728827bd08596abc5006eab4b2f860dd788e87e2c5f7`.

All raw traces and original media remain private and are not included in Git.

## 2026-10-30 — authentic v11 256-fetch dispatch follow

A further cold PCE Fast reference capture used the same authentic Japanese Rev.
1 Track 02 and System Card 3.0, `run@480:300` on input port 0, and no save
state. The follow observer now allows up to 256 fetches per window and 16,384
total rows across 64 windows. The TRV2 build applied the complete instrumented
patch set to an isolated Mednafen 1.32.1 source copy, enabled PCE Fast, and
compiled with `-j1`.

The direct-dispatch, bounded-follow, and authenticated source-chain verifiers
each passed three consecutive runs. The direct trace contains 40 rows, one
match inside the bounded `$6148` code-follow, and 39 out-of-window rows. The
follow sidecar has 10,030 rows across 40 windows: 12 windows ended at the next
`$40E1` dispatch after 235–248 fetches, 28 reached the 256-fetch cap, and no
window remained open or was skipped. In the first window, the actual fetches
pass from `$41C5` through System Card polling at `$E4A5-$E4DE`, read the single
scripted event at `$E4C5`, and return to the `$40E1` dispatcher. The next
dispatch targets vary among the observed windows. This confirms the traced
control-flow return to the indexed dispatcher, not a game-owned handoff.

The five source-chain joins remain limited to the initial two dispatch-table
bytes and three `$41C5` instruction bytes; they match authentic Track 02 reads
and the latest observed `$EA9C` writes. They do not bind every fetched byte or
exclude other guest writers. Track 02 validation accepted 63,488 source-bound
CD data-port reads and zero unbound reads; the `$EA9C` observer recorded 53,248
writes. The transition receipt reports 22,348 input transactions, zero host
key events, one scripted event read at `$E4C5`, zero non-System-Card polls,
zero CD IRQ callbacks, zero authenticated aggregate CD-to-RAM receipts, zero
game main-RAM `$E009` dispatches, and `transition=missing`. Thus the run still
does not establish game ownership, native gameplay handoff, or parity.

Private TRV2 capture directory:
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-indirect-jump-follow-v11-cold-20261030/`.

- Indexed-dispatch trace SHA-256: `94ab5079cd00b290fcb0668731d0314005a6eb77977d6629af2efe0d10e638d3`.
- Per-dispatch follow trace SHA-256: `366224e3cb6b70c9190c84be5f1283da40cb99a21d4d90f58f109894147369d0`.
- System Card code-follow trace SHA-256: `b95b064c8f48985b5ab62ab93a4c2bf4088729772f8c21c56a6ee1fd2db8e9d0`.
- CD data-port read trace SHA-256: `4f20ea110a67c3334d9322d88a07b540f37f812cbc567966cdbf94bb0e038381`.
- CD-RAM writer-PC trace SHA-256: `c23643c27b9b53ae0211ca572644a3fb1eaaf1f05551aa1f22f4ac624f349334`.
- Scripted-input receipt SHA-256: `256e04200c239c15443ccfc87aaa2a977ded1d30961918b63f730a413c28b917`.
- Transition receipt SHA-256: `0a565deb86da810f9e3418a842a1c799b491ee6da1a87e1e595f38b9a1b246fe`.

Raw traces and original media remain private and are not included in Git.

## 2026-10-30 — v11 bounded RAM-loop source candidates

The authentic v11 trace also captures 165 instruction-fetch snapshots whose
valid bytes overlap logical `$40D2-$40E3`, mapped to physical `$1000D2-$1000E3`.
`verify_theron_jp_ram_loop_source_candidates.py` first validates the complete
indexed-dispatch/code-follow sidecars, authenticates all source-valid CD reads
against the Japanese Rev. 1 Track 02 image, validates the all-PC target-write
rows and `$EA9C` writer rows, then compares each fetched byte with the latest
earlier address-scoped write. For `$EA9C` writes, it checks whether a source-
valid `$EA99` read of the same value occurred in the prior 16 shared events,
using the candidate window from the existing writer verifier.

Three repeated runs against the private TRV2 capture reported the same
classification:

- All 18 physical addresses were observed in 495 valid byte slots over 165
  fetch rows.
- All 495 byte slots matched the latest address-scoped writer record, with no
  missing writes or byte mismatches. The all-PC target-write trace covers
  `$1000D9-$1000DB`; the other 15 bytes use the `$EA9C`-only writer trace.
- 95 slots matched non-`$EA9C` writers at `$1000D9-$1000DB`. The other 400
  matched `$EA9C` writes and had same-value, source-valid `$EA99` Track 02 read
  candidates within 16 shared events. There were 390 unique and 10 ambiguous
  candidates across those fetch slots.
- All 165 fetch rows matched their address-scoped latest writes; 111 rows had
  all in-range bytes joined to CD-read candidates.

The all-PC trace resolves the earlier `$EA9C`-only mismatches at `$1000D9-$1000DB`.
In the same follow window, the fetched bytes at `$6109` are `A9 EA` (`LDA #$EA`),
then `$610B/$610E/$6111` fetch `8D D9 40`, `8D DA 40`, and `8D DB 40`, which
decode as stores to `$40D9`, `$40DA`, and `$40DB`. The target-write trace records
these stores at shared events 103466, 103469, and 103472, all writing `$EA`
through MPR2=`$80` to physical `$1000D9-$1000DB`. `$6114` fetches `RTS` and
execution returns to `$432B`. This confirms the observed runtime writes and
control flow; it does not establish the routine's owner or connect it to
gameplay.

The offline verifier identifies one complete execution of this helper in the
v11 capture. Its 12 executed instruction bytes all match earlier `$EA9C`
writer-PC stores and each has a source-valid Track 02 `$EA99` read candidate
within 16 shared events. Eight byte slots have one candidate and four have
multiple same-value candidates. The three helper stores are independently
confirmed by the all-PC target-write trace. This binds executed bytes to
authentic disc-read candidates while keeping repeated-value ambiguity and
caller ownership explicit.

The v11 all-PC target-write trace covered only three of the 18 addresses;
the `$EA9C` writer trace was not exhaustive for the other 15. Candidate read
matches use the same 16-event window as the existing writer verifier, and
repeated values can leave multiple reads as candidates. Raw captures and
original media remain private on TRV2.

## 2026-10-30 — v12 expanded RAM-loop target-write trace

A new cold reference run used authentic Japanese Rev. 1 media and System Card
3.0, without loading a save state. The instrumented target-write trace records
all observed PCs for the full physical `$1000D2-$1000E3` range. The updated
focused verifiers report 2–6 writes for each address; 40 fetched bytes at
`$1000D9-$1000DB` all have a prior matching write, with no mismatches or
missing writes. Across the 165-row/$495-slot loop fetch trace, all latest
address-scoped writes match and the source-candidate counts remain 400
`$EA9C`-matched slots (390 unique, 10 ambiguous) and 111 fully source-candidate
backed rows. `$1000E3` had two writes, both from `$EA9C`.

The capture wrapper exited 124 with `BLOCKED: dynamic receipts absent`,
reporting zero host keys, IRQs, and authenticated CD-RAM receipts. These bounded
traces do not establish that scripted RUN input reached the game, a gameplay
transition, exclusive RAM provenance, game ownership, or parity. Raw captures
and original media remain private on TRV2.

The v12 trace hashes are:

- CD-RAM target writes: `574444b87cc9466c60f3a70b9aa63b0e493fb627e58297ff74a9598d386af5e4`.
- System Card dispatch: `77abcd4b77b3b4e27ae5a8749793cca825befa2d82893208c474b4e1c94bbebb`.
- Indirect-jump follow: `f6f72b60e0b9848f024040689400aa9490565462e0f3de493b2b31ad3eba94aa`.
- CD data-port reads: `5e0aa54c08ab68abce1703b3b98a78dccc1363af493bc50b7b59135163767c31`.

## 2026-10-30 — v13 extended cold RUN trace

Repeated the same cold Japanese Rev. 1/System Card 3.0 capture with
`run@480:300`, extending the wall limit to 180 seconds and raising the CD read
and writer trace limits to 1,048,576. The scripted event was applied at frame
480 with mask `0008`; the System Card read it at `$E4C5` (shared event 1202),
and the scripted-input receipt marked the controller poll boundary verified.
This is positive input-delivery evidence only.

The longer capture still had 0 CD IRQ callbacks, 0 non-System-Card polls, 0
authenticated CD-RAM receipts, 0 game `$E009` dispatches, and
`transition=missing`. It recorded 63,488 source-bound CD data-port reads, the
same sequence as v12, so increasing the capture duration did not yield further
observed CD data-port activity. The System Card sidecar records `$C860` once and
`$C8C7/$C8C9/$C8D2` after input, plus 207 controller polls at `$E4E1`, but has
no `$C897` row or instruction-level `$C8C4` CD-status wait witness. The
dedicated wait verifier therefore remains unsatisfied. The available code-
follow still contains 256 rows; this run does not establish that execution left
the System Card or entered gameplay. The next source target is the BIOS
post-poll branch and its CD-status checks. The raw capture and original media remain
private on TRV2 at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-post-bios-wait-v13-180s-cold-20261030/`.

The v13 trace hashes are:

- Transition receipt: `7e8091e01bc7d5bc2c80ba76a4329ed8525564171db95fcdf155cba5efd902c9`.
- Scripted-input receipt: `bf48c22366285ec05e6475ca7b06e4dcd25ff31e44193e7afc00c38162c65d78`.
- System Card dispatch: `7ba48345ee331ac5dff52c8f7c71017d8fc4606fd92472a666adb85d028fb75d`.
- Indirect-jump follow: `6ffd5a41c196ead9fcc272ec91014df6ec1d147f0f8289d5d5b694b4700a55e5`.
- CD-RAM target writes: `cbf1878c6c8b2506e4d4fa07c67d0045cc3dbeffff4cca1a5708b45b2e0573ce`.
- CD data-port reads: `5e0aa54c08ab68abce1703b3b98a78dccc1363af493bc50b7b59135163767c31`.

## 2026-10-30 — v14 source-guided RUN/SELECT dispatch

The authentic System Card 3.0 code in
`theron-system-card3-controller-poll-e4b0.asm` shows the `$E4E1-$E4FB`
callback condition: input delta `$04` and current state `$0C`, followed by
`JMP ($2284)`. A new cold replay held RUN at mask `$08` for 300 frames, then
pressed SELECT at mask `$04` for one frame while RUN remained held:
`run@480:300,select@600:1`. The scripted-input receipt confirms both events
were applied and read at `$E4C5`. The runtime trace records `$E4FB` with
state=`$0C`, delta=`$04`, vector `$E000`, then BIOS bootstrap PCs `$E0F3`,
`$E109`, and `$C860`. The scripted-input receipt and bounded RAM-loop source
verifier both passed. The RAM-loop report covers 78 fetch rows and 234 byte
slots across all 18 physical addresses; every slot matches its latest prior
address-scoped write, and 56 rows are fully joined to source-read candidates.
This is direct evidence that the System Card dispatch condition and vector
were reached, plus bounded loader-write evidence.

The 60-second capture still reports 0 CD IRQ callbacks, 0 non-System-Card
polls, 0 authenticated CD-RAM receipts, 0 game `$E009` dispatches, and
`transition=missing`. It records 51,200 source-bound CD data-port reads. The
post-vector trace re-enters the BIOS bootstrap; it does not prove that control
reached the game or that Theron gameplay began. A source-bound disassembly of
the callback vector and bootstrap is now recorded in
`theron-system-card3-bootstrap-e0f3.asm`. The vector resolves to `$E0F3`; its
`$E109` loop calls `$C860`, invokes `$F0A9` and the `$E14C` routine, then either
configures MPR6 and jumps to `$4000`, or calls `$CB03/$CB54` and retries `$C860`.
The excerpt identifies the branch and memory operations but does not name
their semantics. Runtime evidence still lacks the branch values needed to
prove which route v14 took, and `$4000` has not been identified as game code.
Raw media and trace files remain private on TRV2 at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-run-select-edge-v14-60s-cold-20261030/`.

The v14 trace hashes are:

- Transition receipt: `4693272b8b5bd3f846a8da98ad613d1d95033f3215861dec15ae72d371374b95`.
- Scripted-input receipt: `1f6be64bc4807d8b77c998f2dcc7891e7fe0db748900924f4fbdac5a5a31d25f`.
- System Card dispatch: `5159154afc81d4925a6284a092198a98fd22ddd27da7618da67554c8eab24b41`.
- Scripted PCE input: `c1e61dcaaf90bc616f21a70267303f1279acfea710e7f87b05cb1f96b63cff67`.

The additional trace hashes are:

- Indexed-jump dispatch trace: `94ab5079cd00b290fcb0668731d0314005a6eb77977d6629af2efe0d10e638d3`.
- System Card dispatch trace: `accfaa21d89efa1567731ad07ac8d41aabef50058e3505f40d2a48a79887afdf`.
- All-PC CD-RAM target-write trace: `819c48b493c75194f9e356c85a156718994091aa24c15f61ec1d1f775e51425d`.

The code-follow, bounded-follow, CD read, and `$EA9C` writer hashes are the
v11 values recorded above.

## 2026-10-30 — v15 bounded bootstrap and `$4000` instruction windows

Added a reference-only PCE Fast trace that independently arms a 512-fetch
window at `$E11B` and a 128-fetch window at `$4000`. Each row records the
logical PC, mapped physical PC and instruction-byte addresses, four memory
bytes with a validity mask, CPU registers, and MPR0–MPR7. The verifier checks
each window's ordering and cap independently, then validates the logical to
physical mapping for every recorded instruction byte. It does not require one
window to precede the other, since the two start PCs are armed independently.

The cold capture used authentic Japanese Rev. 1 media, System Card 3.0, and
`run@480:300,select@600:1`, without a save-state. Track 02 MD5 was
`b7afb338ad31be1025b53f9aff12d73a`; the System Card MD5 was
`ff1a674273fe3540ccef576376407d1d`. The capture produced 512 bootstrap rows
beginning at `$E11B` and 128 target rows beginning at `$4000`; the bounded
trace verifier passed. The target window maps `$4000` through MPR2=`$80` to
physical `$100000`. Its rows include a repeated instruction loop at
`$403A-$4041`; the window alone does not identify this mapped payload or its
owner.

The scripted-input receipt confirms both scheduled events were consumed at
the System Card poll `$E4C5`; it records zero non-System-Card polls. The
60-second capture ended with exit 124 and `transition=missing`, zero CD IRQs,
and zero authenticated CD-to-RAM receipts. These rows show bounded CPU
execution and address mapping only. They do not source-bind the `$4000`
payload, prove a causal BIOS handoff, identify game-owned code, or establish
gameplay. Raw traces and original media remain private on TRV2 at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-bootstrap-follow-v15-20261030/`.

Bootstrap-follow trace SHA-256:
`b05d637423d927c66318d38c8534e058dcb76fdeaec282c134a93415740a24aa`.

## 2026-10-30 — v16 instruction-start snapshots at `$4002/$4009`

A second cold reference capture used the v2 bootstrap-follow patch, which adds
`TheronTraceCDSharedEventSequence` to every row in both independently armed
instruction windows. The authentic Japanese Rev. 1 cue and Track 02 were
used with System Card 3.0; Track 02 MD5 was
`b7afb338ad31be1025b53f9aff12d73a`, System Card MD5 was
`ff1a674273fe3540ccef576376407d1d`, and no save state was loaded. Replay input
was `run@480:300,select@600:1`.

The v2 bootstrap-follow verifier accepted 512 bootstrap rows and 128 target
rows. The sidecars record `$4002` instruction-start bytes
`$73 $00 $20 $01` at event 26830, and `$4009` bytes `$73 $00 $20 $00` at
event 26846. The latest writes recorded before those samples had IDs
14509/14512/14515/14518 and 14530/14533/14536/14539. The read sidecar
independently authenticates Track 02 LBA 4521, user offsets 2–12 (raw-file
offsets 2,716,578–2,716,588); the write sidecar records matching values at
physical `$100002-$10000c`. However, v16's read/write headers do not identify
the shared event-sequence source. The former cross-sidecar verifier therefore
could not establish that these IDs came from one counter; its source-join
result is withdrawn as cross-stream proof. In addition, the bootstrap hook
samples mapped instruction bytes before the CPU opcode read, so these are
instruction-start snapshots, not CPU fetch receipts. The captured values
remain bounded evidence, but their source-to-write chronology needs a fresh
capture with explicit shared-sequence header markers.

The scripted-input receipt still records both events only at System Card
poll `$E4C5`, with zero non-System-Card polls. The transition receipt records
zero CD IRQ callbacks, zero authenticated aggregate CD-to-RAM receipts, zero
game `$E009` dispatches, zero target-range main-RAM consumer reads, and
`transition=missing`. The 60-second capture exited at its bounded timeout
with 32,280 input reads. The gameplay gap remains unresolved.

Raw capture files and original media remain private on TRV2 at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-bootstrap-follow-v16-20261030/`.
The capture summary, bootstrap-follow, CD-RAM target-write, CD data-port-read,
transition, and scripted-input SHA-256 values are:

- `trace.log`: `6153913f1be2adf6d35220e8e76198e5b3ac8d541b1db90120d80325a130a982`
- Bootstrap-follow: `9c04ba4590f6b7243c5c3420fc2d802df6f5e8858b9618ed8110a7f438e8c032`
- CD-RAM target-write: `57682ec0f4c0b8c2278d31dcb78661efa8ccb84b839f2461efb93741afe47e11`
- CD data-port-read: `25ecd7a59f0e1047ee00044c6da25dceea5258f76898f686446c2636d43eef1c`
- Transition: `595e7e056872be5f2c54f5edde75e2e0089dbc3b1ba0525d46825ad53a2a342e`
- Scripted input: `90e0e8ddea0f9951d4ad5cb408fbf03f10af0c146814a37bf642fe60e5d48eb9`

## v17 shared-sequence instruction-start snapshot check

A follow-up cold reference capture used authentic Japanese Rev. 1 Track 02
(`TQJP02.bin`, MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0
(MD5 `ff1a674273fe3540ccef576376407d1d`) with `run@480:300,select@600:1`;
no save state was loaded. The capture used the patched Mednafen 1.32.1 PCE
Fast reference core. Its CD data-port-read and CD-RAM target-write sidecar
headers explicitly name `TheronTraceCDSharedEventSequence`, allowing the
verifier to compare their shared sequence numbers to the bootstrap-follow
trace rather than assuming two independent counters are comparable.

`verify_theron_jp_stage2_instruction_snapshot_source.py` passed. It bound all
11 bytes at physical `$100002-$10000C` to authentic Track 02 bytes, required
an immediate shared-sequence CD read then equal CD-RAM write for each selected
byte, and confirmed those values were still the latest observed writes at
instruction-start snapshots for `$4002` (event 26830) and `$4009` (event
26846). The corresponding latest write events were 14509/14512/14515/14518
and 14530/14533/14536/14539. This is bounded sidecar evidence for the captured
instruction-start memory snapshots. The hook samples before the opcode fetch;
it does not record the actual opcode bus read, exclusive physical-memory
provenance, or a completed loader handoff.

The capture timed out with exit 124. Its transition receipt reports zero CD
IRQ callbacks, zero authenticated CD-to-RAM receipts, zero game `$E009`
dispatches, and no non-System-Card controller poll. Thus v17 does not prove
gameplay or native parity. The raw game media and capture files remain private
on TRV2 at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-bootstrap-follow-v17-20261030/`.

Private trace SHA-256 values:

- System Card bootstrap-follow: `9c04ba4590f6b7243c5c3420fc2d802df6f5e8858b9618ed8110a7f438e8c032`.
- CD-RAM target writes: `bbe32a4aa7f56f5fd7336053cac723a0ff6d18b6139b6f4b1d9b49a724ec8cf8`.
- CD data-port reads: `cf05804075b6a35e69ad823b9c6d3be661a936386c995397477df28f035a2506`.
- Transition receipt: `8a41d252fdb74c7ac2831a752f46e2588a8c5a1dac959e07d74d98a2a6b313ff`.

The raw media and traces are not included in Git.
