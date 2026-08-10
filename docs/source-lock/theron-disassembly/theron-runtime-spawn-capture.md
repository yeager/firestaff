# Theron runtime spawn-consumer capture

## 2026-08-10 — authenticated VCE-register TIA witness

The real US Mednafen Main-RAM loader sidecar now exposes the first source-owned
TIA separately from the generic transfer count. In the external capture the
loader executes HuC6280 `$2286` at physical `$1f0286` and transfers exactly
`$0080` bytes from `$c800` to VCE register destination `$0404`; the matching
RTS is at `$228d` / `$1f028d`. The parser records this as
`vce_tia_coordinates_verified` and the regression test asserts the exact
coordinates.

This is authentic destination/control evidence for the VCE path. It does not
prove which source bytes are palette entries, bitmap pixels, tiles or HUD
elements, and therefore does not unlock palette rendering, tile mapping or
semantic publication. Those still require a byte-exact source-data join in the
same execution.

The Mednafen capture path emits a separate `spawn_consumer_read` receipt. It
is limited to reads of the RAM-loaded helper targets `$5D64/$5D6A` and
instruction fetches inside the byte-verified `$C96B-$CA69` and
`$CC4C-$CD13` US Track 02 consumer bodies.

Those boundaries come from `theron-us-rng-consumers.asm`, including the
authenticated Track 02 BIN offsets and FNV-1a hashes. The receipt preserves
logical and physical addresses, reader PC, reader physical PC, and boundary
flags. It does not infer RNG state, spawn count, monster type, or return-value
ownership. Those remain blocked until one real capture proves the bank
mapping, helper callees, register state, and return contract together.

The output is written beside the live trace as `.spawn-consumer` and is
included in the transition receipt as `spawn_consumer_reads`. A zero count is
valid evidence that the capture did not reach the disassembly-owned consumer;
host-generated data must not replace it.

## 2026-08-09 — fresh cold-start transport receipt

The context-bound instrumented Mednafen build was replayed from a clean boot
against the authenticated US Track 02 medium (`f23601102138f87c33025877767ebf76`)
and System Card 3.0 (`ff1a674273fe3540ccef576376407d1d`). The run delivered six
checked scripted PCE input events and observed 161 raw sector spans, two
byte-exact CD-to-RAM origin receipts, 32 game-main-RAM `$E009` dispatches and
65,536 bounded main-RAM consumer samples. The spawn-register sidecar contained
16 `$4644` preconsumer entries and 64 `$4667` helper entries.

The same execution contained no `$B0E5` entry, no `$C96B`/`$CC4C` dynamic
consumer return, no `spawn_consumer_read`, no RNG-code window and no identified
target read. This is positive evidence for the authenticated transport and
helper path only. It does not publish an RNG value, creature spawn, AI,
combat, generator, loot, T700 or T900 rule. The separate savestate replay was
kept out of this receipt because it had no CD-to-RAM event; joining the two
would manufacture a semantic witness.

## 2026-08-09 — context-bound ADPCM patch order

The build helper now renders the visible `FIRESTAFF_PATCH_BLANK_CONTEXT` tokens
in `mednafen_1.32.1_theron_adpcm_fifo_ram_trace_context.patch` and applies the
result before the main-RAM consumer patch. It uses source context around
`ADPCM_ClocksToNextEvent`, `read_1808`, `PCECD_Read` and `ADPCM_Run`; the older
line-only ADPCM patch is retained as historical provenance but is not part of
the build chain. A clean 1.32.1 source tree compiled successfully with the real
SDL2 runtime on the external disk. This fixes capture-build integrity only: an
ADPCM receipt still proves transport/origin evidence, not sound ownership or
RNG, creature, AI, T700 or T900 semantics.

## 2026-08-09 — authenticated Track 02 teleporter handoff correction

The source-bound object loader now follows `DMBUILDER6/src/dms.h:98-108` for
the Track 02 teleporter word: `ldest` is bits 8..13 of the second word, with
the two top bits reserved. The prior decoder read the low six bits. The loader
also emits `THERON_OBJTYPE_DOOR` and `THERON_OBJTYPE_TELEPORTER` instead of
the unrelated low compatibility IDs. Runtime resolution accepts a valid
source-owned coordinate destination without requiring a second object record;
an unbound destination teleporter, invalid level, wall or out-of-range square
still fails closed. The real US AKUTUBA M0 record `(0,0) -> (2,3)` is covered
by the data-backed dungeon-loader test. This does not open RNG, AI, T700 or
T900 semantics.

`theron_v1_mednafen_spawn_consumer_trace_parse_file()` is the admission gate
for this sidecar. It rejects malformed headers, non-contiguous sequences,
non-bank coordinates, mismatched boundary flags and unrelated reads. Even a
ready receipt has `semantic_publication_allowed == 0`.

The companion `.spawn-registers` sidecar is versioned as
`source=mednafen-pce-instrumented-spawn-registers-v3`. It records A/X/Y/SP/P,
MPR0, the selected instruction-page MPR (`mpr_pc`), the disassembly-relevant
`$B3-$BB` RAM bytes, logical PC and physical PC at `$4644`, `$4667`, `$B0E5`,
`$C96B` and `$CC4C` boundary samples. The v3 sidecar additionally marks
`spawn_entry_b0e5=1` exactly at the disassembly's regular-spawn entry `LB0E5`;
the strict runtime parser requires that entry in the same run as the consumer
windows; the execution-only parser may omit it. This is still an entry
observation only and does not publish a spawn. The parser requires
`physical_pc == (mpr_pc << 13) | (pc & $1FFF)` and rejects the old unversioned
sidecar format. This prevents a logical PC copied into `physical_pc` from
being mistaken for authentic HuC6280 bank provenance.

`theron_v1_mednafen_spawn_register_trace_parse_execution_window_file()` is a
deliberately weaker admission path for an execution-only receipt. It requires
both byte-locked consumer windows, contiguous records, valid bank coordinates
and matching boundary flags, but does not require the `$4644` preconsumer or
`$4667` helper. A fresh external-disk state-autoload with the hash-verified US
Track 02 medium produced 2,048 v2 samples: 2,035 in `$C96B-$CA69` and 13 in
`$CC4C-$CD13`, with neither `$4644` nor `$4667` observed. The strict parser
therefore rejects it as a complete spawn capture, while the execution-only
parser records the authentic window entry without publishing RNG, creature,
AI, loot or T700 semantics. Its transition receipt still reports zero
game-owned CD-sector reads, so no later semantic gate is opened by this
capture.

A separate authenticated new-game replay on the same US Track 02 medium
produced 87 v2 register samples, including 16 `$4644` preconsumer entries and
64 `$4667` helper entries. It also observed 161 raw Track 02 sector reads and
2,048 ADPCM FIFO reads, but no `$C96B` samples and no `spawn_consumer_read`
RAM receipts. The two captures must not be merged: doing so would manufacture
a spawn record from separate executions.

The corresponding v3 instrumented replay preserves those observations and
adds the exact `LB0E5` marker. It still observed no `$B0E5` entry in that run;
the strict parser and correlation gate therefore reject it. This is the
intended result: raw sectors and preconsumer/helper windows are not enough to
publish a T900 spawn record.

The corrected startup replay `run@8:60,i@480:30,i@900:30` was then repeated
against the same hash-verified US Track 02 medium. Its input receipt contains
10,145 samples, with Button I on PCE wire bit `0x0001` and Run on `0x0008`;
the capture also reached 161 raw sectors and 215 MPR-bound register samples.
It still produced no `$B0E5` sample, no game-owned dynamic CD read, and no
dynamic return from the `$C96B/$CC4C` consumer windows. The result is a
verified negative handoff, not a spawn record, and cannot be combined with a
different execution to fabricate one.

The follow-up clean replay used
`run@8:60,i@480:30,i@900:30,i@1320:30,i@1800:30`. The five scripted events
were independently verified as Run=`0x0008` and Button I=`0x0001`; the trace
contained 5,943 input samples, 161 raw Track 02 sectors and 87 MPR-bound
spawn-register samples. It still contained no `$B0E5` entry, no game-owned
dynamic CD read and no dynamic consumer return. This is stronger input-path
evidence, but it remains a negative semantic handoff and cannot publish RNG,
spawn, creature, AI, loot, T700 or T900 behavior.

The sidecar remains an execution snapshot only; it does not turn any register
or RAM byte into an RNG value or spawn record.

## 2026-08-09 — bounded long-window capture and clean cold-start join

The instrumented external Mednafen build now accepts
`FIRESTAFF_THERON_SPAWN_REGISTER_SAMPLE_LIMIT`. The capture script supplies
65536 by default, with a hard upper bound of 1048576; the patch-level default
remains 2048. This only prevents a diagnostic sidecar from stopping before a
later execution edge and does not relax any parser or semantic gate.

An authenticated `.mc0` replay reached `$B0E5` 50 times and entered `$5D64`
with a complete 512-step raw execution window. It had no CD→RAM receipt, so
those observations are not joined to a level/object payload and do not open
RNG or spawn semantics.

A separate clean US cold-start replay verified the full transport boundary:
Track 02 MD5 `f23601102138f87c33025877767ebf76`, System Card MD5
`ff1a674273fe3540ccef576376407d1d`, 161 raw-sector spans, 2 authenticated
CD→RAM-origin receipts, 32 game-main-RAM `$E009` dispatches, 18 `$4644`
preconsumer entries and 64 `$4667` helper entries. The same run had no
`$B0E5`, no `$C96B/$CC4C` dynamic return and no RNG sidecar. The two runs are
kept separate; merging them would fabricate a semantic spawn witness.

## 2026-08-09 — stack-derived RNG return-boundary instrumentation

The US RNG-consumer sidecar now retains the HuC6280 stack snapshot at entry
(`entry_sp`), the return PC reconstructed from the two stack bytes
(`return_pc`) and a `return_boundary` flag when the restored stack and PC
match. The bounded execution window is 512 instructions, rather than 192,
so a real return cannot be discarded merely because the helper body is longer
than the earlier diagnostic window. The C receipt exposes this only as
`return_boundary_seen`; `semantic_publication_allowed` remains zero and no A,
X or Y value is classified as RNG output.

The corresponding patch is
`scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch`, and the parser
regression covers both the stack-derived fields and a positive boundary
fixture. A fresh replay with the real US Track 02 medium and System Card
verified 161 raw sector spans, 32 game-main-RAM `$E009` dispatches, two
byte-exact CD-to-RAM receipts and four scripted PCE input events, but emitted
no `$5D64/$5D6A` RNG sidecar records. Therefore no return boundary, RNG
consumer, spawn, creature, AI, loot, T700 or T900 meaning is published from
that run.

## 2026-08-09 fresh cold-start replay receipt

The latest clean replay used the external instrumented Mednafen build, the
hash-verified US Track 02 medium and the external System Card. Its input plan
was `run@480:30,run@900:30,i@1320:30,run@1800:30,up@2100:120,right@2400:120`.
The trace independently verifies six scripted PCE input events, 159 raw
sector spans and 32 game-main-RAM `$E009` dispatches. The v3 register sidecar
contains 88 samples: `$4644` was entered 24 times and `$4667` 24 times, with
correct physical HuC6280 PC derivation. It contains no `$B0E5` sample, no
`$C96B`/`$CC4C` consumer boundary sample, no special `$4667` branch and no
dynamic RNG return receipt. The transition receipt now records these three
helper/preconsumer counts explicitly.

This is positive runtime evidence for the authentic preconsumer/helper path
and negative evidence for the regular-spawn/RNG contract. It does not justify
publishing monster, AI, loot, T700 or T900 semantics, and it is not merged
with any other execution.

## 2026-08-09 extended cold-start replay

An additional cold-start replay was run from the hash-verified US Track 02 BIN
and System Card, with input delivered by Mednafen's checked-in PCE replay
producer. It produced 133 register samples in one execution, including
dynamic `$4644` and `$4667` entries while the game was being moved through the
dungeon. The observed `$4667` samples all had `$B3 & $07 != $04`, so the
special RAM-loaded branch at `L4667` was not taken. The run still had no
`$B0E5` entry and no dynamic return-value receipt.

This is useful negative evidence: a helper call is not the same thing as an
RNG result. The parser now records `helper_4667_special_branch_seen`
separately and leaves RNG, regular spawn, creature, AI, loot, T700 and T900
publication closed for this capture.

## 2026-08-09 authenticated `.mc0` replay

The external capture campaign also verified a real Mednafen state, distinct
from the HuBM SRAM files: the state is a 240,789-byte gzip file whose
decompressed payload begins with `MDFNSVST`. It was loaded against
the hash-verified US Track 02 and System Card and replayed with four scripted
PCE transactions. The resulting receipt contains 2,048 source-bound register
samples across `$C96B-$CA69` and `$CC4C-$CD13`; it contains no `$4644`,
`$4667`, or `$B0E5` sample, no game-owned CD-sector read, and no RNG-consumer
sidecar. The execution-only parser accepts the window, but the strict
correlation gate remains closed. These observations are from one authentic
run and are not merged with other captures.

The capture profile binds PCE Button I/1 to `Z` and Button II/2 to `X`.
Comma/period are not assumed as portable macOS bindings; they require an
explicit local Mednafen input-map entry. This control choice is capture
metadata only and does not claim a game-owned Button I/II semantic consumer.

## 2026-08-09 — råkodsidecar för RNG-ingången

Den instrumenterade Mednafen-kedjan skriver nu en separat `.rng-code`
sidecar när HuC6280 faktiskt börjar exekvera vid `$5D64` eller `$5D6A`.
Sidecaren har den fasta markören
`source=mednafen-pce-instrumented-rng-code-v1` och innehåller, per entry, den
logiska PC:n, den MPR-härledda fysiska PC:n samt 256 byte rå instruktionsminne.
Den hålls separat från `.rng-consumer`; den råa kodbilden är inte ett RNG-värde
och parsern får inte göra den till en spawn- eller statstabell.

Den autentiserade `.mc0`-körningen producerade följande lokala receipt:

```text
Track 02 MD5:       f23601102138f87c33025877767ebf76
System Card MD5:    ff1a674273fe3540ccef576376407d1d
Mednafen MD5:       3ee7c8df8aad7b87ef0ecc05aaa29d8d
B0E5 samples:       50
RNG window samples: 512
RNG code windows:   1 (entry $5D64, physical PC $000D1D64)
CD->RAM receipts:   0
RNG code MD5:       bb890a377b3d17e96384b15d0255c14b
```

Detta är nu byteexakt runtimebevis för vilken kod som körs vid `$5D64` i den
autentiserade körningen. Det bevisar fortfarande inte RNG-returvärdet,
bankens ägarskap, monsterrecordets konsument, AI, loot, T700 eller T900.
Körningen får därför inte släppa de semantiska grindarna, och den får inte
blandas med den separata cold-start-körningen som hade CD→RAM-receipts.

### 2026-08-09 — source-byte join för råkodfönstret

De första 64 byte i det autentiserade `$5D64`-fönstret matchar exakt i den
riktiga US Track 02-filen på sju offsetar:

```text
0x975c4, 0xe0dc4, 0x12a5c4, 0x173dc4,
0x1bd5c4, 0x206dc4, 0x2505c4
```

Det är `0x975c4 + n*0x49800`, `n = 0..6`, i den hashverifierade US-filen
`TQUS02.bin` (MD5 `f23601102138f87c33025877767ebf76`). Parsern verifierar nu
hela 256-byte-fönstret mot dessa källkopior, tillsammans med sidecar-header,
PC-fält, 8 104 992-byte filstorlek och hexlängd. Detta är en källbyte-join,
inte ett påstående om vilken kopia som var mappad i körningen.

RNG-returvärde, caller, spawnkategori, creature, AI och T900-regler är fortsatt
stängda tills den autentiserade körningen visar deras verkliga konsumentkedja.

### 2026-08-09 — samma cold-start, fortfarande ingen spawnretur

En ny bounded cold-start med samma autentiserade US Track 02, System Card och
instrumenterade Mednafen gav i en och samma session:

```text
authenticated_cd_ram_receipts=256
game_main_ram_e009_dispatches=26
spawn_preconsumer_4644_samples=33
spawn_helper_4667_samples=96
spawn_entry_b0e5_samples=0
rng_consumer_samples=0
rng_code_windows=0
```

Detta är ett kombinerat transport-/förkonsumentbevis, men inte ett positivt
RNG- eller spawnbevis. `$4644`/`$4667` med `B3=$FF` visar inte den specialgren
som disassemblyn kräver, och utan `$B0E5` i samma session får ingen RNG-,
creature-, AI-, generator-, T700- eller T900-semantik öppnas.

## 2026-08-09 fresh cold-start window trace

The same real US CUE/BIN and external System Card were replayed with the
instrumented Mednafen build extended with bounded physical reads of the
control window `$1F01F7-$1F01FB`, parameter window `$1F01E5-$1F01E7`, and game
window `$1F1000-$1F1007`. This run delivered one host Run key through the
Quartz route and recorded 40 raw sector spans, 10 SCSI read commands, 25 CD
IRQ callbacks, seven game-main-RAM `$E009` dispatches, nine `$4644`
preconsumer entries and 32 `$4667` helper entries. The window sidecar also
recorded one eight-byte read of the game window by physical PC `$002B22`;
there were no parameter-window reads.

The receipt is useful because it proves the authentic CD-to-main-RAM-to-code
route and identifies the next consumer boundary. It still has no `$B0E5`
entry, no `$C96B/$CC4C` dynamic return, no byte-exact RNG result and no
source-owned T700/T900 read. The sidecar therefore remains transport/runtime
evidence only. It must not be converted into RNG, creature AI, attack, loot,
generator timing or stat semantics, and it is not merged with the `.mc0`
save-state run above.

## 2026-08-10 — same-session scripted dungeon-state replay

The operator-created Mednafen state was replayed in a fresh instrumented
process against the authenticated US raw BIN and System Card. The replay used
one execution and this checked input plan:

```text
i@60:5,up@120:10,run@240:2,i@360:5,ii@480:5,left@600:10,right@720:10
```

The resulting receipt is retained outside the repository at
`/Volumes/Extern-disk/theron-dungeon-capture-20260810/` and binds:

```text
track02_md5=f23601102138f87c33025877767ebf76
system_card_md5=ff1a674273fe3540ccef576376407d1d
autoload_state_md5=f17f377df210b4a3ae904a13fb85a7f0
scripted_pce_input_events=7
input_transactions=10764
spawn_register_samples=2048
authenticated_cd_ram_receipts=0
spawn_consumer_reads=0
rng_consumer_samples=0
transition=missing
```

This is a negative same-session result: the replay did not reach a game-owned
CD-sector consumer or a monster/object consumer. The 2,048 register samples
are execution-window evidence only and cannot authorize RNG, spawn, creature
AI, attack, damage, loot, generator, T700 or T900 semantics. The capture is not
joined with another run and no BIOS, BIN/CUE member or state file is tracked.

## 2026-08-10 — JP raw-BIN startup attempt

An external-disk JP-only CUE was built around the hash-verified raw
`TQJP02.bin` (`b7afb338ad31be1025b53f9aff12d73a`) and run with the authenticated
System Card (`ff1a674273fe3540ccef576376407d1d`). Mednafen read 256 authentic
Track 02 sectors and emitted the normal 64 KiB VDC-VRAM and 1 KiB VCE snapshots.
The receipt nevertheless contained zero byte-exact FIFO/RAM origin records and
zero authenticated CD→RAM handoffs; the minimal data-only CUE does not prove
the complete JP CD boot route. It also produced no source-owned portrait
consumer or VDC destination binding.

The JP roster/object bytes remain valid source data, but the snapshot is not
joined with the US capture and does not authorize JP portraits, portrait IDs,
or a JP text/bitmap consumer. A complete authenticated JP CUE with its real
audio tracks and a same-run CD→RAM origin receipt is still required.

## 2026-08-09 extended `.mc0` RNG window

The external-disk replay was repeated with the same hash-verified US Track 02,
System Card, authenticated `.mc0` state and checked-in PCE replay plan, using
the new `THERON_CAPTURE_RNG_CONSUMER_SAMPLE_LIMIT=4096` bound. The sidecar
declared `rng_consumer_sample_limit=4096` and contains exactly 4,096 samples;
the raw `$5D64` window and one source-byte-correlated code window were observed.

This run is still not a semantic handoff. It recorded 50 `$B0E5` register
entries, but no `$4667` helper entry, no game-owned CD→RAM receipt, no
`$C96B/$CC4C` return boundary and no stack-derived return boundary. The
instrumentation observed `return_pc=0001` throughout this state-autoload
window, so the capture does not prove ownership of an RNG return value. The
parser and runtime admission therefore keep RNG, spawn, creature, AI, loot,
generator, T700 and T900 semantics closed. The 4,096-step file is retained
only as external-disk diagnostic provenance and is not tracked as game data.

The sidecar limit is now mandatory and bounded to 512..65,536 samples. This
prevents an unlabelled or unbounded diagnostic file from being mistaken for a
complete execution witness; it does not relax any semantic gate.

## 2026-08-10 — operator-created dungeon state, same-session spawn entry

A real Mednafen session was advanced through the authentic US startup, level
selection and dungeon entry using the hash-verified System Card and full US
CUE. The original dungeon viewport was visibly reached, and Mednafen displayed
`State 0 saved.` after the state was written. The existing state was backed up
outside the repository before replacement; no BIOS, BIN/CUE member or savestate
is tracked in GitHub.

The state was then autoloaded by a fresh instrumented Mednafen process against
the same full US CUE. Its transition receipt records:

```text
mednafen_binary_md5=92ee06fdc623703dacfb133d28e8a004
track02_md5=f23601102138f87c33025877767ebf76
system_card_md5=ff1a674273fe3540ccef576376407d1d
autoload_state_md5=f17f377df210b4a3ae904a13fb85a7f0
host_key_events=22
cd_irq_callbacks=1
spawn_entry_b0e5_address_hits=50
spawn_entry_b0e5_samples=0
rng_consumer_samples=0
rng_code_windows=0
transition=missing
```

The `autoload_state_md5` is the MD5 of the operator-created local state used
for this replay; it is provenance only, not game data committed to the
repository. The measured fact that matters is the same-session `$B0E5` entry
observation. The run did not produce a verified
`$4644`/`$4667` return owner, `$5D64/$5D6A` RNG return, game-owned CD-sector
consumer, monster record, object consumer or `$2600` consumer. Therefore RNG,
creature, AI, attack, damage, loot, generator, T700 and T900 publication
remain fail-closed.

## 2026-08-10 — operator-controlled transition receipt

The long-running external Mednafen session was allowed to complete one
operator-controlled startup-to-dungeon transition against the same authentic
US Track 02, System Card and instrumented binary. Its single-session receipt
records:

```text
mednafen_binary_md5=92ee06fdc623703dacfb133d28e8a004
track02_md5=f23601102138f87c33025877767ebf76
system_card_md5=ff1a674273fe3540ccef576376407d1d
input_transactions=131072
host_key_events=23
cd_irq_callbacks=25
raw_sector_spans=134
scsi_read_commands=38
scsi_read_sector_bindings=134
byte_exact_fifo_ram_destinations=0
adpcm_fifo_reads=2048
adpcm_ram_writes=2048
byte_exact_origin_ram_receipts=256
authenticated_cd_ram_receipts=256
game_main_ram_e009_dispatches=31
main_ram_loader_tii_transfers=11
main_ram_loader_rts=24
main_ram_loader_call_entries=5
main_ram_loader_entry_successor_next=4
main_ram_e009_enters=6
main_ram_e009_register_writes=117
main_ram_consumer_reads=65536
main_ram_target_reads=0
main_ram_target_writes=0
spawn_consumer_reads=0
spawn_register_samples=136
spawn_preconsumer_4644_samples=33
spawn_helper_4667_samples=96
spawn_helper_4667_special_branch_samples=0
spawn_entry_b0e5_samples=0
rng_consumer_samples=0
rng_code_windows=0
vdc_vram_snapshot_bytes=65536
vce_palette_snapshot_bytes=1024
transition=observed
```

This is the strongest current same-session transport receipt: authentic
Track 02 sectors reached authenticated RAM-origin records, the game entered
its `$E009` loader path, and the bounded main-RAM consumer was observed. It
does not identify a level/object/tile owner; there is no byte-exact FIFO
destination, target read, `$B0E5` regular-spawn entry, dynamic RNG return, or
source-owned T700/T900 consumer. The zero values are therefore negative
semantic evidence, not permission to substitute host-side formulas. RNG,
creature, AI, attack, damage, loot, generator, T700 and T900 remain
fail-closed. The raw receipt remains outside GitHub at
`/Users/bosse/.firestaff/cache/theron/manual-capture/out/theron.transition`.

The corresponding bounded main-RAM sidecar is approximately 8.7 MiB. The
Firestaff intake ceiling is now 16 MiB, which permits this authenticated
65,536-sample receipt to reach the existing byte/PC code-window verifier. This
is an intake correction only: `target_2600_bytes_present=0` and
`semantic_publication_allowed=0` remain mandatory.

## 2026-08-10 — external capture 3 ADPCM transport receipt

The external capture at `~/.firestaff/cache/theron/full-capture-3/theron.cd`
adds a complete, source-bound ADPCM transport witness for the authenticated
Mednafen session. It contains 2,048 `pce_cd_fifo_read transport=adpcm` rows
and 2,048 matching `pce_cd_adpcm_ram_write` rows. Each row carries Track 02
LBA 4860, source offsets, FIFO sequence numbers, ADPCM addresses and byte
values; the parser now counts both sides and rejects non-ADPCM FIFO rows in
this trace family.

This proves CD-sector/ADPCM FIFO to ADPCM-RAM transport. It does not prove a
CPU read from ADPCM-RAM, a decoded sample, a PSG/ADPCM channel start, a sound
ID, or the owner that maps a gameplay event to a sample. No event-owned audio
consumer is therefore published, and `theron_v1_play_sound()` remains
fail-closed. The real evidence tightens the gate; it does not justify a
synthetic sound mapping.

## 2026-08-10 — cold-start runtime edge capture

An additional authenticated cold-start capture against the US Track 02 BIN
recorded 102 raw-sector spans, 28 game `$E009` dispatches, two byte-exact
CD→RAM origin receipts, 17 `$4644` preconsumer observations and 64 `$4667`
helper observations. The input route was PID-bound Quartz delivery; the
capture recorded 29 host key events and an observed startup-to-loader
transition.

The same receipt recorded zero `$B0E5` regular-spawn entries, zero helper
special-branch samples (`B3 & 7 == 4`), zero `$C96B/$CC4C` RNG windows, zero
spawn-consumer reads and zero RNG return contracts. The `$4667` calls therefore
prove only that the authenticated runtime reaches the static helper edge;
they do not authorize a host RNG, creature record, AI, loot, generator,
T700 or T900 implementation. The raw capture remains outside GitHub.

## 2026-08-10 — file-select route capture

An additional authenticated cold-start run used PID-bound Button I/Run input
through the intro and reached the original `WHICH FILE DO YOU PLAY?` screen.
Its receipt contained 176 raw-sector spans, 32 game `$E009` dispatches, four
byte-exact CD-to-RAM origin receipts, and 2,048 matching ADPCM FIFO reads and
ADPCM-RAM writes.

The same session contained zero `$B0E5` spawn entries, zero `$C96B/$CC4C`
RNG windows, zero spawn-consumer reads, and zero valid creature-category
observations. This is positive input/media/startup-to-file-select evidence,
not dungeon, RNG, creature, AI, T700 or T900 evidence. The raw capture remains
outside GitHub.

## 2026-08-10 — dungeon walk reaches an invalid `$B0E5` overlay

The authenticated US Track 02 session was driven through the real map and
dungeon viewport, then closed after the manual input phase. Its final trace
contains 50 logical `$B0E5` address hits, 34 `$4644` preconsumer samples and
99 `$4667` helper samples, with 65,536 bounded register observations.

All 50 `$B0E5` hits carried A=`$2c` or A=`$85`, never a source-defined regular
spawn category `0..3`. The helper samples had no `$B3 & 7 == 4` special branch,
and the session produced zero `spawn_consumer_read`, RNG-consumer and RNG-code
windows. The address hits are therefore an authenticated bank-overlay
diagnostic, not an original monster spawn; no creature, AI, loot, generator,
T700 or T900 semantics are opened.

## 2026-08-10 — cold-start transport and helper boundary

En ny cold-start-capture kördes från extern disk mot samma hashverifierade US
Track 02, System Card och instrumenterade Mednafen. Den autentiserade
startupkedjan gav följande kvitto i en och samma session:

```text
track02_md5=f23601102138f87c33025877767ebf76
system_card_md5=ff1a674273fe3540ccef576376407d1d
autoload_state_md5=none
input_transactions=44539
cd_irq_callbacks=25
raw_sector_spans=159
scsi_read_commands=53
scsi_read_sector_bindings=159
adpcm_fifo_reads=2048
adpcm_ram_writes=2048
byte_exact_origin_ram_receipts=2
authenticated_cd_ram_receipts=2
game_main_ram_e009_dispatches=32
main_ram_loader_tii_transfers=11
main_ram_loader_rts=24
main_ram_loader_post_rts=24
main_ram_loader_call_entries=5
main_ram_e009_enters=6
main_ram_e009_returns=5
main_ram_consumer_reads=65536
main_ram_target_reads=0
main_ram_target_writes=0
spawn_consumer_reads=0
spawn_register_samples=88
spawn_preconsumer_4644_samples=17
spawn_helper_4667_samples=64
spawn_helper_4667_special_branch_samples=0
spawn_entry_b0e5_address_hits=0
rng_consumer_samples=0
rng_code_windows=0
vdc_vram_snapshot_bytes=65536
vce_palette_snapshot_bytes=1024
transition=observed
```

Detta är ett positivt transportbevis för råsektor → autentiserad RAM-
proveniens och för `$4644`/`$4667`-hjälparkanten. Det är samtidigt ett negativt
semantikbevis: körningen nådde inte `$B0E5`, den särskilda `$B3 & 7 == 4`-grenen,
RNG-konsumenten eller någon målskrivning. Värdena får därför inte användas för
att härleda host-RNG, monsterstats, AI, strid, loot, generatorer, T700 eller
T900. Nästa witness måste nå en verklig dungeon-/spawn- eller objektaktion i
samma autentiserade session och binda returvärde, källrecord och konsument.

Råtrace, BIOS, System Card, BIN/CUE och savestate ligger kvar utanför GitHub.
