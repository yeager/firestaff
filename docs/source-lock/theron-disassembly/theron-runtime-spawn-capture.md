# Theron runtime spawn-consumer capture

## 2026-08-13 — RAM/VDC-replayens VDC/VCE-pair är screen-space-admitted

Den externa replayen `theron-vdc-ram.exXuQu` har en komplett 64 KiB VDC- och
1 KiB VCE-snapshot. Pairens FNV-1a-identiteter är VRAM `087da136` och VCE
`5376a91b`; de är nu upptagna i den kända capture-listan för
`theron_v1_vram_trace_load_known_capture_files()`.

Detta öppnar endast återgivning av den autentiserade native screen-space-
bilden. I samma session skriver `$CB22` nollor över `$2600–$2EFF` och läser
sedan tillbaka området. Den observationen klassificerar inte bytes som level,
object, square, HUD, T700 eller T900, så alla sådana semantiska grindar är
fortsatt stängda.

## 2026-08-13 — `$2600`-läsning bevaras i parsern

Parsern för `main_ram_consumer` nollställer inte längre
`target_2600_bytes_present` efter att ha läst sidecar-raderna. Ett lokalt
MPR-bundet extern-disk-spår (`mpr.trace.main-ram-consumer`, MD5
`12f470ef2c38febd9b2c9699dad3b4cb`) innehåller en sammanhängande läsning från
`$2600` och rapporteras därför som `target_2600=present` i parser-only-läge.
Detta är fortfarande en adress-/körningsproveniens, inte en identifiering av
recordtypen eller en öppning av object-, T700- eller T900-semantik.

## 2026-08-13 — target-window receipt skiljer initiering från runtime

Consumer-receipten behåller nu också antal target-läsningar, icke-nollvärden
och distinkta reader-PC:er. Combat-replayens sidecar
`live.trace.main-ram-consumer` (MD5 `4d9da34dd8a0042dc302449af78c54cc`)
har 19 läsningar i `$271B–$279F`, tre icke-nollvärden och 19 reader-PC:er.

Detta är en bättre kandidat för framtida source join än den rena `$CB22`-
initieringen, men capturen startar från ett autoload-save, har inga CD/FIFO-
receipts och saknar därför fortfarande bevis för level/object- eller
T700/T900-ägarskap. Semantisk publicering förblir blockerad.

## 2026-08-11 — authentic user save reaches the source RNG consumer path

The fresh Mednafen run using the operator's `TQUS...sav` (not a synthetic
state) used the authenticated US Track 02 MODE1/2048 payload and System Card.
Its receipt observed 25 CD IRQ callbacks, 161 raw-sector bindings, 2
byte-exact CD-to-RAM origin receipts, 11 loader TII transfers, 31 `$4644`
preconsumer observations, 96 `$4667` helper observations and 3,072 bounded
RNG-consumer samples across six complete windows. The capture also emitted all
15 scripted PCE input events and `transition=observed`.

It still contained zero `$B0E5` regular-spawn entries and zero creature-record
joins. The result is authenticated execution evidence for the original RNG
consumer path, not proof of the RNG return owner or spawn semantics; AI,
attack/damage, loot, generators, T700 and T900 remain fail-closed.

## 2026-08-11 — RNG capture is bounded per source window

The reproducible Mednafen capture patch now limits both samples within a RNG
window and the number of complete source windows. The launcher validates and
forwards `THERON_CAPTURE_RNG_CONSUMER_WINDOW_LIMIT` (1–1024), and records the
chosen value in the transition receipt. A smoke capture against the
authenticated US Track 02/System Card with the limit set to 4 emitted
`rng_consumer_window_limit=4`, 662 rows and 133,594 bytes; it did not produce a
dynamic CD/RAM or creature join. The receipt remains negative evidence and is
kept on the external disk; no RNG, spawn, AI, T700 or T900 semantics are
promoted from it.

## 2026-08-11 — corrected capture replay isolates a bank-overlay `$B0E5` hit

The external-disk replay `theron-capture-20260812-state-replay-fixed` used the
corrected non-invasive Mednafen binary (`1ec797bb7d1aea4d756521686d7b0c36`),
the authenticated US Track 02 MODE1/2048 payload
(`ceb02343868f80cec899e9b239aff2da`), the authenticated System Card and the
existing user-owned SRAM. The video path remained clean and the raw trace was
kept outside GitHub.

The replay emitted 36 address hits at logical `$B0E5`, but every retained hit
was the bank overlay with `A=$2C` or `A=$85`; there were zero valid category
`A=0..3` entries, zero `$4644` preconsumer edges, zero `$4667` helper edges and
zero target/creature joins. The replay also stopped after 12 of its 15 scripted
input events, so its transport receipt is incomplete. This is explicit
negative evidence, not a spawn or RNG result; no AI, combat, loot, generator,
T700 or T900 semantics are promoted.

## 2026-08-11 — non-invasive capture build preserves correct video and transport

The external-disk replay `theron-capture-20260811-fixed-hooks` used the
rebuilt capture binary after removing the pre-execution CPU operand reads that
had corrupted the Mednafen picture. The same authenticated US CUE, MODE1/2048
Track 02 payload (`ceb02343868f80cec899e9b239aff2da`) and System Card
(`ff1a674273fe3540ccef576376407d1d`) were used. The pixel-exact startup image
was correct, so this build is suitable for further capture work.

The transition receipt records 25 CD IRQ callbacks, 161 raw-sector bindings,
2 byte-exact CD-to-RAM origin receipts, 32 `$E009` dispatches, 65,536 bounded
main-RAM consumer reads, 4,096 spawn-consumer reads and 1,536 bounded
RNG-consumer samples. Host RUN/Button-I input was delivered through the real
SDL2/Cocoa route.

This run still contains zero valid `$B0E5` regular-spawn entries, zero target
write ownership and no creature-record join. It therefore does not open RNG
return ownership, creature AI, attack/damage/loot, generator timing, T700 or
T900 semantics. The external raw trace remains outside GitHub and the
production gates stay fail-closed.

## 2026-08-11 — real SDL2 SRAM replay reaches authenticated gameplay transport

The external-disk replay `theron-capture-20260811-real-sdl2-sram` used the
directly linked SDL2 2.30.9 runtime, the authenticated US Track 02 CUE
(`ceb02343868f80cec899e9b239aff2da` for the MODE1/2048 Track 02 payload), the
System Card MD5 `ff1a674273fe3540ccef576376407d1d`, and the operator-owned HUBM
SRAM. Host RUN/Button-I input was observed by the instrumented emulator.

The transition receipt records 105526 input transactions, 25 CD IRQ callbacks,
175 raw-sector bindings, 4 authenticated CD-to-RAM origin receipts, 32 game
`$E009` dispatches, 2048 ADPCM FIFO/RAM bytes and 1536 bounded RNG-consumer
samples. This is stronger transport/runtime evidence than the prior
save-state replay and is retained outside GitHub.

The session still recorded zero `$B0E5` address hits, so it did not reach the
regular-spawn entry. The capture therefore does not prove monster category,
RNG return ownership, creature AI, attack/damage/loot, generator timing, T700
statistics or T900 object rules. Those production semantics remain
fail-closed; no values are synthesized from the transport receipt.

## 2026-08-11 — ADPCM FIFO/RAM pairs are now byte- and sequence-bound

The CD-state intake now requires every authenticated
`pce_cd_fifo_read transport=adpcm` row to be followed by a matching
`pce_cd_adpcm_ram_write` row with the same source LBA, source offset, FIFO
sequence, ADPCM address and byte value. An incomplete capture is rejected;
the accepted receipt exposes `adpcm_transport_pair_verified`. This proves
transport integrity only. It does not identify a decoded sample, channel
start, sound ID or gameplay event owner, so `theron_v1_play_sound()` remains
fail-closed.

## 2026-08-11 — authenticated witness-to-creature bridge remains capture-gated

`theron_v1_creature_apply_spawn_consumer_witness()` now connects a complete
`$B0E5-$B1EB` register witness to an already admitted live creature, but only
when the creature carries the same authenticated regular-spawn category. It
copies the witness receipt's HP, attack and defense and calls no host RNG.
Static category-4 records, category-unknown creatures, incomplete witnesses
and unauthenticated captures remain rejected. This is an integration boundary,
not proof that a current capture contains the missing spawn event; the runtime
semantic gate stays closed until the real same-session RNG return, target write
and creature consumer are joined.

## 2026-08-11 — authenticated save replay observes RNG edges but no spawn

The external-disk US Track 02 replay
`theron-capture-20260811-cocoa-save.trace.*` used the authenticated Track 02
MD5 `f23601102138f87c33025877767ebf76` and System Card MD5
`ff1a674273fe3540ccef576376407d1d`. Its transition receipt records 2,048
spawn-register samples, 12 `$4644` preconsumer observations and 50 `$4667`
helper observations, while recording zero valid `$B0E5` entries and no target
publication. The older sidecar has 35 sequence windows of 192 records and no
explicit window-limit header. The parser now infers that limit only at the
first authenticated sequence edge and accepts the older 18-field row format.
This is format compatibility, not a new gameplay assumption. The session
still does not bind a return value to a monster record or establish AI,
attack, damage, loot, generator, T700 or T900 ownership. The raw capture
remains outside GitHub and the production semantic gate stays closed.

## 2026-08-11 — same-session `$5D64` RNG consumer reaches 22 complete windows

The external-disk US Track 02 capture used the authenticated Track 02 BIN
`f23601102138f87c33025877767ebf76` and System Card
`ff1a674273fe3540ccef576376407d1d`. Its RNG sidecar contains 11,264 samples,
exactly 22 complete 512-step windows through `$5D64`; the separate 256-byte
`$5D64` code window matches the source BIN at the authenticated RNG-code
offset. This is now accepted by the multi-window parser.

The same capture has no valid `$B0E5` regular-spawn entry, no target write
ownership and no creature/T700/T900 event join. It therefore proves execution
and source provenance of the original RNG consumer only; no host RNG,
spawnstats, AI, combat, loot, generator or stat semantics are promoted from
it.

## 2026-08-11 — exact `$C96B` body retained without semantic promotion

The US Track 02 span at `$C96B-$CA69` is now retained verbatim as
`theron-us-c96b-consumer.asm`, with the authenticated BIN MD5, raw offset and
FNV-1a receipt. The disassembly proves the caller-provided `($3A)` writes,
the `$2040->$2045` transfer and the surrounding table updates. It does not
identify the pointed record or establish a creature, object, generator,
T700, T900 or RNG return contract. The runtime therefore remains fail-closed
for those semantics until one capture joins the caller, bank mapping,
register state and return boundary.

## 2026-08-11 — `$B07D` caller context narrows the next witness

The adjacent US caller window is now retained in
`theron-us-spawn-caller-window.asm` with raw offset and FNV-1a identity. It
shows four `$4644` calls before the `$B0E5` dispatch and the register fields
that cross that boundary. The observed writes to `$2980/$2990/$29A0` and
`$2A20/$2A28` remain unclassified RAM tables; no creature, T700 or T900
meaning is inferred from their addresses. A future capture must join this
caller, a valid category, the RNG return and the target write in one session.

The register sidecar now emits `caller_b07d_window=1` for the authenticated
`$B07D-$B0E4` caller range. The parser keeps this as provenance and does not
require it for older v3 sidecars; it is available to the next same-session
semantic correlation.

## 2026-08-11 — register parser separates `$B0E5` address hits from spawn categories

The register-sidecar parser now accepts the current external capture shape,
which appends `return_pc`/`caller_pc` context while retaining the v3 header and
`mpr_pc` bank coordinate. It validates that context as capture provenance and
keeps the original physical-PC check. The sidecar's `spawn_entry_b0e5=1` is
counted separately as an address hit; only A=`0..3` sets
`spawn_entry_b0e5_seen`. A=`$2C`/`$85` overlay therefore remains explicit
negative evidence instead of making the whole execution-window trace
unparseable. The strict runtime-spawn parser still rejects a run without a
valid category, preconsumer and helper edges, and no gameplay semantics are
published by this change.

## 2026-08-10 — corrected cold-start VDC/VCE pair admitted as source media

The corrected cold-start replay produced an exact raw VDC snapshot (64 KiB,
FNV-1a `4a2186a2`) and VCE snapshot (1 KiB, FNV-1a `aa11c4f2`) from the
authenticated US Track 02/System Card session. The production viewport now
admits this exact pair for source-bound tile bytes and VCE palette entries.

This is a media-bank admission only. The replay still reports no `$B0E5`
spawn entry, no dynamic RNG consumer and no identified level/object/tile
consumer, so it does not unlock square-to-tile mapping, perspective, HUD,
creature AI, combat, loot, generator, T700 or T900 semantics. The raw pair
remains on the external disk and no game data is committed to the repository.

## 2026-08-10 — complete File-select and dungeon-input replay remains pre-spawn

The external-disk run used the complete raw MODE1/2352 US CUE and the
original input sequence `Run` → `Button I` → held movement. Track 02 was
authenticated and the transition receipt recorded 28 byte-exact CD-to-RAM
origin receipts, 32 `$E009` dispatches and the bounded main-RAM consumer
window. The session still produced zero `$B0E5` entries, zero
`spawn_consumer_read` rows, zero `$C96B/$CC4C` RNG returns and zero identified
target reads/writes. It proves the menu/input and loader route only; no RNG,
creature AI, combat, loot, generator, T700 or T900 rule may be promoted.

The companion receipts remain outside GitHub on `/Volumes/Extern-disk` and
contain no game-data payload in the repository.

## 2026-08-10 — raw CUE/save-state replay reaches rejected `$B0E5` overlay

The external-disk replay used the complete raw MODE1/2352 US CUE together with
the authentic Mednafen save-state. The transition receipt verified Track 02
MD5 `f23601102138f87c33025877767ebf76` and counted 30 executions at `$B0E5`.
Every observed entry carried A=`$2C` or A=`$85`; none carried the
disassembly-defined regular-spawn categories `0..3`. The parser therefore
classifies these as same-address overlays and reports zero semantic spawn
entries. This is a useful negative runtime witness, not permission to derive
an RNG result, creature record, AI action, loot event, generator transition,
T700 stat update or T900 object rule.

The same experiment was also attempted against the archive's cooked
MODE1/2048-byte Track 02 projection. That run produced no authenticated
CD-to-RAM origin receipt and is explicitly excluded from source evidence.

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

The transition receipt now also includes `vdc_io_writes`, counted from the
side-effect-free VDC-port witness. Admission requires a positive count in
addition to the exact 64 KiB VDC-VRAM and 1 KiB VCE snapshots. This prevents a
header-only VDC sidecar from looking like a complete source-screen capture;
the field remains transport provenance and does not publish a VDC destination
or text/BAT/square/HUD semantic owner.

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

## 2026-08-11 — bounded combat replay remains transport-negative

En ny extern-disk replay använde den autentiserade US-CUE:n och samma
Mednafen-savestate med 18 frame-bundna PCE-händelser: rörelse, svängningar och
Button I. Körningen avslutades normalt efter den bounded timeouten och gav
VDC/VCE-snapshots, men ingen spelägd CD→RAM-originreceipt:

```text
track02_md5=f23601102138f87c33025877767ebf76
system_card_md5=ff1a674273fe3540ccef576376407d1d
scripted_pce_input_events=18
authenticated_cd_ram_receipts=0
spawn_consumer_reads=0
spawn_entry_b0e5_address_hits=50
spawn_entry_b0e5_samples=0
rng_consumer_samples=0
transition=missing
```

De 50 råa `$B0E5`-adresspassagerna bar samma ogiltiga A-värden `$2C`/`$85`
som tidigare och är därför inte regular-spawn-calls. Denna körning öppnar
inte RNG, monsterstats, AI, attack/skada, loot, generator, T700 eller T900.
Mednafen, savestate, System Card och alla råtracefiler ligger kvar på
extern-disken och är inte committade.

## 2026-08-11 — consumer register witness

The external instrumented Mednafen build now records A/X/Y/SP/P alongside
each bounded main-RAM consumer read. The authenticated replay produced
65,536 consumer rows. In the observed `$C96B` window, reads from `$20F8/$20F9`
returned stable bytes while A/Y changed across iterations; this binds the
register context to the read, but does not by itself prove whether the bytes
are RNG state, a pointer table, or a creature record. The corresponding
`$CC4C` rows likewise remain execution-window evidence until a same-session
return boundary and source-owned destination are joined.

The register-expanded sidecar and binary remain on the external disk. No
RNG, monster, AI, loot, generator, T700 or T900 semantics are published from
this witness.

## 2026-08-11 — stack/caller witness for the rejected `$B0E5` overlay

The external instrumented Mednafen binary was rebuilt to retain the HuC6280
stack return word at every traced `$B0E5` entry. The authenticated US Track 02
save-state/CUE run produced 50 entries, all with A=`$2C` or A=`$85` rather than
the regular-spawn category domain 0..3. It produced no `$4667`, `$5D64` or
`$5D6A` entry during the same run. The observed return words were `$0002` and
`$3F3F`; neither is an authenticated game-code caller for the disassembly
span.

This strengthens the negative result only. It does not identify an RNG return,
monster stat, spawn event, AI, combat, loot, generator, T700 or T900 owner.
The raw trace remains on the external disk and is not committed.

## 2026-08-10 — corrected cold-start replay proves loader transport only

En korrigerad extern-disk replay skickade en kort Run-puls efter BIOS i stället
för att hålla Run aktiv genom hela uppstarten. Den autentiserade sessionen
producerade 226 råsektorspann, 254 CD→RAM-originreceipts, 32 `$E009`-
dispatchar och 11 verifierade frame-bundna PCE-inputevents. Den gav dessutom
465 registerprover vid `$4644/$4667` i samma session.

Detta är ett positivt loader-/transport-witness men inte ett gameplay-witness:
`spawn_consumer_reads=0`, `spawn_entry_b0e5=0`,
`rng_consumer_samples=0`, `target_reads=0`, `target_writes=0` och
`helper_4667_special_branch_samples=0`. `$4644/$4667` får därför inte
frikopplas från samma-sessionens saknade `$B0E5`/RNG-retur eller användas för
att uppfinna monster, AI, attack/skada, loot, generator, T700 eller T900.
Receipten finns på extern disk under
`/Volumes/Extern-disk/theron-capture-20260810-replay2-goal.transition`.

## 2026-08-10 — cold-start scripted replay stops before game-owned CD handoff

En ny extern-disk capture kördes mot den autentiserade US-CUE:n med den
instrumenterade Mednafen-binären och en frame-bunden plan med Run från frame 1,
Button I/II och rörelse. Inputtrace:n verifierade 11 planerade PCE-händelser
och 131072 inputtransaktioner. Körningen gav dock ingen autentiserad
CD→RAM-originreceipt (`authenticated_cd_ram=0`, `raw_sector_spans=0`) och bara
tre IRQ2-callbacks innan den bounded timeouten. Den är därför ett negativt
transport-witness: den bevisar inte level/object-consumer, RNG-retur, spawn,
creature-AI, attack/skada, loot, generator, T700 eller T900. Råtrace och
snapshots finns endast på extern disk under
`/Volumes/Extern-disk/theron-capture-20260810-replay-goal`.

## 2026-08-10 — source-bound VDC/VCE capture allow-list

Produktionsvägen accepterar nu de kompletta snapshotpar som faktiskt har
verifierats på extern disk, i stället för att bara acceptera ett äldre par.
Allow-listan är stängd och jämför hela filernas FNV-1a-identitet:

```text
US legacy capture       VRAM f11c6b2a  VCE ea83f117
US dungeon capture      VRAM 5c830cc2  VCE 6fb303b5
US interactive capture  VRAM 4f15b98c  VCE 71cc9b11
JP startup capture      VRAM 8ae1e419  VCE 4e48c361
US cold-start capture   VRAM 1a37c99b  VCE 71cc9b11
```

Alla par kräver fortfarande exakta 64 KiB VRAM- och 1 KiB VCE-filer. De ger
endast source-bound BAT/tile/palett-replay och M11-presentering av den fångade
skärmen. Ingen post i allow-listan bevisar square-to-tile-mappning,
perspektiv, HUD-/objektägare, monster, RNG eller T700/T900-semantik.
# 2026-08-10 — complete US CUE/19-track runtime capture

The external-disk capture was repeated with the complete user-supplied US
layout from the archive: `TQUS.cue`, decoded audio tracks 01/03/04–18, and
`TQUS02.iso` reconstructed exactly as the archive's `Decode.bat` specifies
(`TQUS19.iso` followed by `TQUS02End.iso`). No BIOS, disc image, audio track or
raw trace is stored in the repository.

Mednafen reported the real 19-track TOC and consumed Track 02 at LBA 3234.
The same session produced 159 raw CD-sector reads, 88 spawn-register samples,
17 `$4644` entries and 64 `$4667` entries, but zero valid `$B0E5` entries,
zero `$C96B/$CC4C` RNG windows, zero spawn-consumer reads and zero target
writes. The complete CUE therefore strengthens media/runtime transport
coverage but does not bind a gameplay consumer. RNG, dynamic spawn, creature
AI, combat, loot, generator timing, T700 and T900 remain fail-closed.

## 2026-08-10 — autoload replay remains pre-gameplay

En autentisk extern-disk-savestate replayades med 15 planerade PCE-händelser.
Den gav ingen autentiserad CD→RAM-receipt och ingen spelägd spawn-konsument.
De 50 `$B0E5`-adresshittarna hade alla A=`$2C`/`$85`, alltså inte en giltig
regular-spawn-kategori `0..3`. `$4644`, `$4667`, giltiga spawn-samples,
RNG-fönster och target writes var noll. Därför öppnas ingen RNG-, AI-,
combat-, loot-, generator-, T700- eller T900-semantik. Rådata stannade på
extern disk och Mednafen stängdes efter den avgränsade körningen.

## 2026-08-11 — C3A0 same-session target reads admitted as provenance

The receipt now separates the observed `$CB22` init/readback helper from
non-init target reads and counts the byte-locked `$C3A0-$C429` caller shape
independently (including non-zero and distinct reader-PC counts). This is a
provenance refinement only; it does not turn the target bytes into a level,
square, object, HUD, creature, T700 or T900 record.

The external-disk state-autoload run produced 65,536 ordered
`main_ram_consumer_read` rows. The C3A0 register window and the target sidecar
are from the same run: C3A0 reads observed main-RAM bytes at `$271e-$272b`
through the authenticated HuC6280 code bank `$0dxxxx`. The raw consumer trace
has MD5 `4d9da34dd8a0042dc302449af78c54cc` and is accepted by Firestaff's
parser after allowing the instrumenter's optional `a/x/y/sp/p` suffix fields.

This is a real C3A0-to-main-RAM observation, not a semantic record label. The
capture does not identify those bytes as creature, object, generator, T700 or
T900 data, and it has no valid regular `$B0E5` category plus complete return
contract. RNG, AI, combat, loot, generator timing and T700/T900 publication
therefore remain blocked.

## 2026-08-11 — longer savestate replay rejects a B0E5 address alias

The same authenticated US Track 02/System Card pair was replayed from the
operator's real Mednafen savestate with a longer bounded movement plan. The
session recorded 36 logical `$B0E5` address hits, but every sampled entry had
`A=$2C` or `A=$85`; neither value is a valid regular-spawn category (`0..3`).
The hits mapped to physical `$0E10E5`, while the trace contained no valid
`$B0E5` execution sample, no `$4644` preconsumer sample, and no `$4667` helper
sample. The transition receipt therefore records
`spawn_entry_b0e5_address_hits=36`, `spawn_entry_b0e5_samples=0`, and
`semantic_publication_allowed=0`.

This is explicit negative evidence for a same-address bank/overlay alias, not
an RNG or spawn witness. The invalid entries must not be promoted to monster
stats, AI, generators, T700 or T900 semantics. The raw capture remains outside
the repository; only the bounded receipt facts are documented here.
## 2026-08-11 — authenticated screen-space VDC/VCE replay admission

The bounded US Track 02/System Card replay produced a complete raw VDC VRAM
snapshot (65,536 bytes, FNV-1a `42a483ac`) and VCE snapshot (1,024 bytes,
FNV-1a `6fb303b5`). The pair was captured from the real external Track 02
session, not generated or checked into the repository. The production VRAM
loader now admits this exact pair alongside the earlier authenticated
captures, so the real VDC tile bytes and VCE palette words can be decoded and
presented as a native screen-space frame.

This receipt does not prove the missing Track 02 square-to-tile/material
consumer, level/object records, HUD ownership, creature semantics, or source
bitmap decompressor. Those routes remain fail-closed; no world or object
meaning is inferred from the BAT alone.

## 2026-08-09 — clean external screen-space replay retained

The external capture directory also contains a complete 64 KiB VRAM snapshot
with FNV-1a `a449538a` and a 1 KiB VCE snapshot with FNV-1a `ea83f117`. The
session used the authenticated US Track 02 BIN and System Card, and the raw
files remain outside GitHub. The transition receipt for this run has no
semantic loader transition, so the runtime allow-list admits this pair only
for source VDC tile decoding, atlas binding and VCE palette presentation.

It does not authorize a dungeon level, square-to-tile mapping, HUD/object
owner, creature, T700 or T900 consumer. No README screenshot is promoted from
this capture.

## 2026-08-13 — CUE/state screen-space replay retained

The existing patched Mednafen capture producer replayed the authentic retail
US `TQUS.cue` with its hash-locked MODE1/2048 Track 02
(`ceb02343868f80cec899e9b239aff2da`), the local System Card
(`ff1a674273fe3540ccef576376407d1d`), and the operator's matching Mednafen
state. Its clean shutdown wrote a 65,536-byte VDC snapshot with FNV-1a
`8165c4d4` and a 1,024-byte VCE snapshot with FNV-1a `ea83f117`.

The raw files remain on the external disk and are not game data committed to
Git. The production allow-list admits this exact VDC/VCE pair only as a
screen-space tile and palette replay. The real-capture regression verified
1,057 BAT tiles, 50,455 non-zero preview pixels and 38,167 presented pixels.
It does not establish the original bitmap loader, VDC transfer provenance,
HUD ownership, a dungeon map/object interpretation, or any combat/spawn
consumer; those semantic routes remain closed.

## 2026-08-11 — MODE1/2048 CD→RAM transport witness

En ny, avgränsad körning använde den autentiserade US-distributionens riktiga
`TQUS.cue` med Track 02 i MODE1/2048 (`TQUS02.iso`, MD5
`ceb02343868f80cec899e9b239aff2da`) och den lokala System Card-proveniensen
`ff1a674273fe3540ccef576376407d1d`. Den instrumenterade Mednafen-sessionen
gav 161 råsektorspann, 51 SCSI-läsningar, 2 byte-exakta CD→RAM-receipts,
32 `$E009`-dispatches och 65 536 ordnade main-RAM-consumerläsningar. Samma
session observerade 4 096 spawn-consumerläsningar och 3 584 RNG-windowläsningar.

Detta är nu godkänt som transport/proveniens även när runtime-läsningarna är
icke-noll. De är observationer, inte semantiskt bevis. `$B0E5` hade fortfarande
0 giltiga regular-spawn-samples, och därför förblir RNG, spawn, creature-AI,
combat, loot, generatorer, T700 och T900 fail-closed. Råtrace, VCE/VRAM och
media ligger kvar på extern disk; inget BIOS eller spelmedia är incheckat.

## 2026-08-11 — directed Button-II replay still misses gameplay ownership

En ny extern replay använde samma hashverifierade US Track 02/System Card och
det lokala användarskapade `.mc0`-state:t, med riktade Button-II- och
rörelsehändelser. Mednafen accepterade den verkliga Track 02-bilden och
producerade 65 536 main-RAM-consumer-rader, 4 096 spawn-consumer-rader och
11 422 183 RNG-consumerobservationer. Den nådde däremot ingen transition och
ingen giltig `$B0E5`-execution sample: 36 adresshittar var återigen overlay-
träffar utan A=`0..3`, medan `$4644` och `$4667` saknades.

Detta är ett nytt negativt runtime-bevis, inte en grund för att fylla i
monster-AI, attack/skada, loot, generatorer, T700 eller T900. Capturen och
BIOS:et ligger kvar på extern disk; endast denna sammanfattning är i repot.

## 2026-08-11 — real US data consumer audit after directed replay

Den lokala hashverifierade `TQUS02.bin`-filen validerades mot samtliga sju
dungeon-ledgers: 2 186 placerade thing-records, 165 monsterrecords, 392
materialiserade itemförekomster och 8 source-bound US-roster-namn. Den riktiga
66-raders item-propertytabellen och palettefönstret passerade också sina
source-byte-kontroller.

Textvägen visar däremot en konkret kvarvarande consumergräns. Thing-data
dekoderas till observerade codon-strängar, men de innehåller obevisade `{}`-
kontrollkoder och publiceras därför inte av `world_load_dungeon_text()`;
testet får korrekt 0 publicerade world-strängar. Detta ska lösas med en
autentiserad US textconsumer/codetabell från disassembly/runtime-capture, inte
genom att göra den diagnostiska rådekodningen till speltext.

## 2026-08-12 — autoload combat capture remains a C96B-only witness

Den externa capture-katalogen `theron-capture-20260812-combat` använde den
autentiserade US-CD:n, System Card och det lokala Mednafen-autoload-state:t.
Mednafen bekräftar därför den riktiga Track 02-identiteten, men capturen visar
`cd_irq_callbacks=1`, `non_system_card_pcecd_reads=0`, `transition=missing` och
`main_ram_loader_tii_transfers=0`. Den är alltså inte en CD→RAM→level-handoff.

Registertrace innehåller 65 536 samplingar och en 16-bitars sekvens-wrap, men
bara `$C96B`-fönster. Den innehåller inga `$CC4C`, `$4644`, `$4667` eller giltiga
`$B0E5`-samples. Spawn-read-tracen läser i stället bank-1F-adresser `$2072`
till `$2081` utan `$5D64/$5D6A`-mål. Den befintliga parsern avvisar därför
capturen som komplett spawn-consumer, vilket är korrekt.

Detta stärker den negativa beviskedjan men öppnar inte RNG-returägande,
monster-/generatorlogik, AI, combat, T700, T900 eller ljudhändelser. Råtrace
och BIOS ligger kvar på extern disk; inget av detta läggs i GitHub.

Capturens kompletta screen-space-snapshot är däremot nu separat admissible:
VDC-VRAM FNV-1a `411960eb` och VCE FNV-1a `6fb303b5`. Det paret går via den
source-bound VDC/VCE-presentern, men får inte tolkas som square-, objekt-, HUD-
eller monsterdata och används inte som README-screenshot.

## 2026-08-12 — bounded `$B0E5` reserve rejects the active state’s non-spawn calls

The reproducible capture patch reserves at most 256 register rows for logical
`$B0E5` after its ordinary register budget is exhausted. This avoids dense
`$C3A0`/`$CAxx` execution windows hiding every later `$B0E5` observation while
keeping the trace bounded. The 12-second isolated replay of the operator-owned
US CUE state recorded 164 retained `$B0E5` address hits in a 595 KiB register
sidecar.

Every retained entry had `A=$80` or `A=$85`; neither is in the source-locked
regular-spawn dispatch domain `0..3`. The same run had
`transition=missing` and zero authenticated CD-to-RAM receipts. It therefore
remains negative evidence only, even though its state image contains one exact
copy of the source-lock caller and `$B0E5` byte spans. The presence of source
bytes in a Mednafen state does not establish that a particular runtime call
uses the regular-spawn contract.

No RNG return ownership, creature spawn, AI, attack/damage, loot, generator
timing, T700 or T900 rule is published from this capture. The next positive
witness must still show A=`0..3` together with the caller/helper return path,
a source-owned target write and a live creature record in one execution.

## 2026-08-13 — side-effect-free HuC6280→VDC port witness

The existing patched PCE Mednafen route now has an optional bounded
`FIRESTAFF_THERON_VDC_IO_TRACE` observer. It records each original VDC-port
write before the normal VCE/VDC path receives it, with its HuC6280 logical and
MPR-derived physical program counter, timestamp, port address, value and A/X/Y
register values. The observer changes no VDC state and is capped at 65,536
records.

The isolated 12-second US CUE/System Card/state replay produced 65,536
newline-delimited records. Its primary game-code writers were physical PCs in
`$0E1Axx` and `$0E1Bxx`; the port stream included the normal MAWR/VWR select
and data sequence. This proves that the original runtime, rather than
Firestaff, drove the captured VDC destination.

It is deliberately not a source-to-VRAM admission by itself: the same replay
still had zero authenticated CD-to-main-RAM receipts and no live level
transition. In particular, a writer PC and a VDC address do not identify the
retail byte span that supplied a tile, nor assign square, object, HUD, portrait
or text semantics. Those presentation routes remain fail-closed until one
capture joins original CD/RAM data consumption and the VDC transfer in the
same execution.

Firestaff now validates this sidecar through
`theron_v1_mednafen_vdc_io_trace_parse_file()`: header, contiguous sequence,
monotonic timestamps, bounded addresses/registers and writer coordinates are
all checked. The parser remains provenance-only and does not change the
negative semantic conclusion above.
## 2026-08-13 — fresh dungeon savestate replay remains transport-negative

A new local replay from the authenticated dungeon `.mc0` state was run against
the hash-verified US Track 02 and System Card. It produced 65,756 register
samples, 256 `$B0E5` address-overlay hits, 4,096 `spawn_consumer_read` rows,
and 2,213 RNG samples. The replay produced only one CD IRQ after autoload:
zero raw sectors, zero source-backed CD-to-RAM receipts, zero valid regular
`$B0E5` categories, and zero `$4644`/`$4667` samples. The result is retained
outside GitHub at `/Volumes/Extern-disk/theron-capture-20260813-state/` and
does not authorize spawn, RNG, AI, combat, loot, T700, or T900 semantics.

The source-bound US/JP mechanics-playability probe independently passes 79/79;
that result covers the authenticated grid/loader path only and is not merged
with this negative runtime replay.
