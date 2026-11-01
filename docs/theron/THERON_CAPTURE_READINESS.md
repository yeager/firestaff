# Theron Capture Readiness

**Current summary: 2026-11-01 (updated after the r4 cold reference attempt).** This is the maintained evidence overview for
Theron native runtime and reference captures. Historical source-lock reports
retain their original capture dates and are not additional current status
summaries. Start with [project status](../PROJECT_STATUS.md), the
[Theron TODO](../../TODO-theron.md), and the [documentation index](../DOCUMENTATION_INDEX.md).

The latest authenticated JP state-only replay used five scripted controller
events and verified each event reached a controller read. It recorded zero
spawn-consumer/register entries, zero RNG samples, and no dungeon transition;
it does not establish movement or spawn semantics. Explicit state-only PCE
Fast autoload captures skip the cold-start System Card hook and trace checks,
as well as the combined CD/controller/HuC6260, raw-sector, input/IRQ, and
authenticated CD-to-RAM handoff checks. They still require a verified
autoload receipt and the remaining applicable trace schemas. The shell
regression checks the conditional System Card hook gates. See the
[runtime spawn capture report](../source-lock/theron-disassembly/theron-runtime-spawn-capture.md).

The new native `--debug` receipt's modified engine and decoder translation
units compiled in an isolated TRV2 Debug configuration. The full application
has not been linked and no authentic runtime log has been captured, so this
adds no gameplay evidence. Its planned fields and remaining acceptance checks
are listed in the [debug receipt note](../source-lock/theron-track02-spawn-debug-receipt-20261101.md).

The PCE Fast reference wrapper now supports the exact source-sector filter
`THERON_CAPTURE_PCE_FAST_EXACT_SOURCE_LBA=3674`. It gates CD data-port rows and
FIFO-origin RAM receipts before their trace quotas; CD FIFO origin tracking
continues across all sectors to preserve byte attribution. This is capture
tooling only. No accepted authentic capture has exercised the filter, and
LBA 3674 has not yet been joined to a game-owned active-map or tile consumer.
See the [filter implementation and verification note](../source-lock/theron-jp-akutuba-exact-lba-capture-tool-20261101.md).
The data-port verifier accepts the authenticated raw BIN and MODE1/2048 ISO
projection, and can require complete LBA 3674 user-byte coverage for the
descriptor/map span; see that note for the exact command and fixture boundary.

The current Mednafen 1.32.1 r4 binary was rebuilt from patched source against
standalone SDL 2.32.10; isolated startup and linked-runtime version checks
passed. The latest visual follow-up used authentic JP media and a
PID-targeted four-second RUN attempt, but the PCE input trace recorded only
released-button samples. The window remained at the PC Engine Super CD-ROM
System splash, decision/status traces had no rows, and the wrapper rejected
the attempt because the status trace contained no `$1800` reads. This run did
not establish title-menu readiness or a game handoff. An earlier cold run
recorded all 15 requested host presses and active-low RUN/I samples at System
Card poll `$E4C5`, but did not verify the `$E11D` bootstrap branch or a
game-owned poll. That run also recorded zero CD IRQs, zero authenticated
CD-to-RAM receipts, and no LBA 3674 data-port rows before timing out. The
exact-LBA filter remains unexercised on an accepted game load. See the
[PID-input capture report](../source-lock/theron-jp-lba3674-pid-input-cold-capture-20261101.md),
[current TODO](../../TODO-theron.md), and
[exact-LBA tool note](../source-lock/theron-jp-akutuba-exact-lba-capture-tool-20261101.md).

An earlier headless capture did hit the decision trace’s 2,048-row cap. The
earlier PID-input capture did not: its decision verifier accepted 88 rows
across six BIOS PCs, while its separate System Card status trace reached its
65,536-row cap and recorded an explicit truncation marker. The visual
follow-up's status trace had no rows. None of these cold attempts establishes
a game handoff or an active-map source read.

Everything below this maintained summary is a dated capture and tooling
runbook. Historical results retain their original scope and may refer to
earlier diagnostic paths; they do not supersede the current parity boundaries
above or the linked project status and TODO pages.

An authentic US standard-PCE save-state comparison maps all four US Akutuba
arrays from the hash-verified Track 02 projection into `HuC.SysCardRAM`.
Saved MPR3 selects physical page `$74` in CPU window `$6000–$7fff`; this
establishes the saved guest mapping, not which resident map is active. Two
older movement-trace read addresses align with map-0 source bytes under that
mapping, but the trace did not preserve MPR at each read. The active selector,
pose-bound tile consumer, and render update are still unbound. See the
[US state map binding](../source-lock/theron-us-authentic-standard-pce-state-map-binding-20261101.md).

The current native regression build also passed eight selected US/JP
real-media and input CTests in three consecutive loops (24 executions):
regional Track 02 source checks, JP fresh start, US BRAM Continue, JP
later-dungeon runtime, and both regions' keyboard/SDL input routes. The longer
SDL route includes mouse-panel and touch input, movement, wait, champion
selection, and the US item interaction sequence. These tests establish those
bounded routes only; they do not close the gameplay-parity gaps below. A separate native macOS Debug run of the US event-loop suite passed three consecutive times against hash-verified authentic Track 02 (MD5 `f23601102138f87c33025877767ebf76`; 348.18 seconds total); see the [run receipt](../source-lock/theron-native-sdl-input-real-media-20261101.md). A newer authentic US cold Drator replay exercised the independent instruction
window and observed `$C852` → `$C85E` `JSR $5039` → `$5039` in order. Its
first five `$C852` 512-byte windows match US Track 02 at raw offset `$A3AD2`;
the live `$5039` bytes (`44 05 86`) differ from the old static stage-two
candidate. The wrapper rejected the package because live `$45C3` windows did
not match retail Track 02 and multiple trace quotas were reached. See the
[ordered call-window report](../source-lock/theron-disassembly/theron-us-c852-runtime-call-capture-20261101.md). A separate authentic JP
PCE Fast save-state-only run exercised the `$3879` trace patch after the
wrapper accepted the JP MODE1/2048 Track 02 identity. The shared trace still
reached 8,192 rows before recording `$C852`; it recorded JP `$C85E` as opcode
`$A6` at physical `$0D285E`, with no `$5039` execution. Disabling the optional
`$44D2` detail did not prevent the broad shared cap. The new independent
`<trace>.c852-runtime-window` sidecar remained at zero rows and the receipt
reported `not_observed`, which is the expected result when no `$C852` hit
occurs. It records the first `$C852` plus 128 following instruction dispatches
with PC/opcode/operands/register/MPR state, independent of the broad quota. The
wrapper accepts the JP MODE1/2048 ISO identity `62a39bbf43415c9739c41c2481080a49`. Set
`FIRESTAFF_THERON_DISABLE_44D2_TRACE=1` to omit that optional detail when
reproducing the capture. This state-only JP result does not prove a cold handoff or gameplay semantics;
the US call path is covered by the separate bounded trace described above. An earlier local US attempt lacked intervening authentic tracks and was
rejected at Track 03. A later capture used the complete archived US track set
and reached the Drator route; see the ordered call-window report for its
source-validation failure and trace limits.

For the separate T900 item-progression gap, the real-data quest-name regression
now lists five ground-linked records whose type index matches the local
quest-name index, across US/JP dungeons 3, 6 and 7. These source locations are
capture targets only; they do not establish artifact identity or a T900
consumer. See the
[ground-item index report](../source-lock/theron-t900-ground-item-index-candidates-20261101.md).

The 2026-10-31 original-game capture maps the runtime `$C852` code window to
Track 02 raw offset `$A3AD2`. The cold Drator route diagnostic reached start,
left-event and reentry, while the bounded consumer trace hit its row cap
before reentry. The wrapper rejected the package because the exhaustive BIOS
trace hit its row cap. The current next capture work is the JP active-map
source-to-consumer join above; when conducting a separate US Drator
reader-source replay, use the source-scoped BIOS mode below to bound that
replay. The existing capture also joins generated-
row publication bytes to later reads and a work-buffer copy. Enable the bounded
fetch hook with
`THERON_CAPTURE_PCE_FAST_C852_FETCH_TRACE=1`; it runs after `RdAtPC()` and
before instruction dispatch, and writes `${THERON_LIVE_TRACE_OUTPUT}.c852-fetch`
with at most 4,096 hits per observed code site. In addition to `$C852`, it
records 512-byte instruction windows beginning at the Drator row-reader
candidate PCs `$CBC6` and `$45C3`, with physical PC, MPR snapshot, registers
and `event_sequence` from the same shared counter used by the row-lifecycle
trace. These code-fetch receipts can be searched against the authenticated
Track 02 BIN to narrow repeated static occurrences; they do not prove source
ownership until the bytes match and the physical mapping is accounted for.
The `$C852` source bytes also contain a direct `JSR $5039` at `$C85E`; matching
static target bytes occur at `$5039` in the authentic US and JP stage-two
payloads. The earlier capture available when this static candidate was first
reported had no MPR2 or instruction path for the call. The latest US trace records `$C85E` and the following `$5039` dispatch with
ordered events and all MPRs, but the live target bytes do not match the static
stage-two candidate. Binding the runtime target to authentic source bytes,
then following the operand read and `$3879` path, remains open. An identity-
checked scan compared all 102 unique MPR2 `$68` instruction windows with raw
US Track 02 and Track 19 and found no contiguous 8 KiB source-page match. This
bounded scan does not rule out decoded, relocated, mixed-source or runtime-
written code. A cross-capture comparison found that none of the 102 observed
instruction windows matches the 5,064-byte `$23AD` output from a separate
authentic US run; this is not a same-session provenance join. The existing code-RAM write trace filters writer PCs to
`$1F1C00–$1F1DFF`; the reference wrapper also provides an all-writer
`WrMem()` hook for physical `$0D0000–$0D1FFF`. Set
`THERON_CAPTURE_PCE_FAST_MAIN_RAM_TARGET_WRITE_RANGE=0d0000-0d1fff`,
`THERON_CAPTURE_PCE_FAST_MAIN_RAM_TARGET_WRITE_SAMPLE_LIMIT=1048576`,
and `THERON_CAPTURE_PCE_FAST_CODE_RAM_WRITE_TRACE=1`; build with the `$23AD`
hook enabled and set `THERON_CAPTURE_HUC6280_23AD_TRACE_OUTPUT` to retain its
ordered output. The target-write sidecar includes a shared `event_sequence`
and an explicit truncation marker. The C852 patch now emits the same counter
under schema v3 and a per-run capture ID; the all-writer trace carries that ID,
configured range and sample limit. The wrapper receipt binds both sidecar hashes
and completeness counts, and the verifier requires that receipt before joining
events. Nine bounded fixtures exercise success and rejection paths; no compiled
binary or authentic cold capture has validated runtime output yet. Run
[`verify_theron_runtime_page_write_join.py`](../../scripts/verify_theron_runtime_page_write_join.py)
to compare captured fetch bytes with the latest prior physical writes; this
still leaves source-read ownership open. The exact procedure and interpretation
are in the [runtime page scan](../source-lock/theron-disassembly/theron-us-mpr2-68-runtime-page-source-scan-20261101.md),
[ordered call-window report](../source-lock/theron-disassembly/theron-us-c852-runtime-call-capture-20261101.md) and the
[static candidate report](../source-lock/theron-disassembly/theron-us-stage2-c852-call-target-5039-20261101.md).
The PCE and PCE Fast `$3879` trace-hook extension starts a bounded
128-instruction window at the first runtime `$C852` instruction and retains
the caller range, MPRs and CPU registers through later instructions that may
reach `$5039`. The complete Mednafen 1.32.1 patch-only chain applied in three
consecutive isolated regression loops; a separate patch-only pass with PCE
Fast enabled also passed. The instrumented PCE Fast emulator was built against
SDL2 2.32.10 and passed the isolated startup/runtime version check. An
authentic JP state-only run exercised the patch but hit the shared 8,192-row
cap without `$C852`; the sidecar remained header-only and the receipt reported
`not_observed`. A clean rebuild after the sidecar's independent dispatch gate
change passed isolated SDL2 2.32.10 startup validation, and a second authentic
JP state-only run again reported zero rows and `not_observed`; no `$5039`
execution was recorded. Therefore the ordered US runtime `$C852`→`$C85E`→`$5039` dispatch is now
observed in a bounded reference trace. Runtime target source ownership, data
semantics, a complete accepted handoff package and native parity remain
unproven.
Use the [US Track 02 code-window verifier](../../scripts/verify_theron_pce_fast_code_fetch_source.py)
to validate the exact media hash, opcode/window consistency, MPR-to-physical-PC
relationship and all matching raw offsets. A complete verification requires
both `$CBC6` and `$45C3` to appear and match the retail image; per-PC row
sequences must be contiguous, shared event sequences must increase, and any
truncation marker rejects the report. Multiple matching offsets remain
explicit rather than being collapsed to one source occurrence. The verifier's
regression passed three loops, and a tooling fixture using 512-byte windows
read from the hash-verified US retail BIN resolved all four `$CBC6` and three
`$45C3` matching raw offsets. These are authentic source bytes but constructed
execution rows, not a runtime capture. A later authentic run emitted live
reader windows: `$CBC6` matched retail candidates, while `$45C3` did not match
the retail image and caused the strict wrapper to reject the source report.
For an isolated US raw Track 02 replay, set both
`THERON_CAPTURE_PCE_FAST_C852_FETCH_TRACE=1` and
`THERON_CAPTURE_PCE_FAST_CODE_FETCH_SOURCE_VERIFY=1`. The capture wrapper
requires the authenticated US MODE1/2352 BIN and runs the verifier before
accepting the source report. It writes
`${THERON_LIVE_TRACE_OUTPUT}.c852-fetch-source-report` and records the report
path in the transition receipt. This strict source gate is opt-in and does
not apply to JP or MODE1/2048 ISO captures.
Correlate them with the
main-RAM consumer-read trace, which identifies data reads by reader PC and
physical mapping. `FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_RANGE` filters
physical data-read addresses, not instruction PCs.

The JP active map is also still unbound. A scan of 106 retained 8 KiB BaseRAM
snapshots found no contiguous copy of any authenticated Akutuba map tile array;
this negative result does not cover other RAM, packed data, or a live consumer.
An offline scan of a separate authentic JP `.mc0` state found the complete
descriptor-plus-four-map source span once at inflated offset `0x6633b` (state
SHA-256 `b4f2a06788e4723110b19892f93e8ac4aef1b5c18c6358a9fe36f1baf747b068`).
The source-locked Mednafen 1.32.1 inspector maps it to the named
`HuC.ROMSpace + 0x68 * 8192` CD-RAM variable, at page `$74`, page offset
`$091d` (physical `$0E891D`). The saved MPRs do not select page `$74`, and no
contiguous copy of a map array occurs in saved BaseRAM. The state also embeds
an RGB preview and named VCE/VDC0/VRAM/SAT variables, which form an atomic
state snapshot; they are not independently joined to the active map or party
pose. All four maps are present, so the active map remains unknown. See the
[state inspector](../../tools/inspect_theron_mednafen_pce_fast_state.py) and
[source-state report](../source-lock/theron-disassembly/theron-jp-akutuba-map-source-sector-20261101.md).
The authenticated source descriptor and all four Akutuba tile arrays map
to one raw MODE1 sector: Track 02 file sector 308 / disc LBA 3674. Map 0 and
map 2 discriminating `(2,3)` bytes are at raw BIN offsets `$B10E6` and
`$B121F`. Use that sector as a capture filter only; source presence does not
establish that the original read or selected it. The repeatable source mapping
is reported in the [JP Akutuba sector trace plan](../source-lock/theron-disassembly/theron-jp-akutuba-map-source-sector-20261101.md)
and can be recomputed with the
[`identity-checked planner`](../../tools/plan_theron_jp_akutuba_map_trace.py).
For a source-only view of all four map arrays, pass `--dump-grids`; it prints
descriptor values and raw bytes in `y` rows and `x` columns without assigning
semantics to unknown fields or tiles, or choosing the runtime map.
The next cold JP capture must preserve one shared event sequence from
hash-verified Track 02 descriptor/map source reads, through System Card
transfer and any `$23AD` decode, to the game-owned active selector and a tile
read at the captured party pose. Include fetched instruction bytes, logical and
physical PCs, MPRs, disc LBA/user offset, RAM destinations, and an atomic
VDC/VCE/VRAM snapshot. At `(2,3)`, authenticated Akutuba map 0 has `0x20` and
map 2 has `0x00`; use that as a map discriminator only after the runtime pose
and tile consumer are independently bound. Compare an accepted input with a
no-input control before changing movement or viewport behavior. See the
[active-map evidence boundary](../../TODO-theron.md#2026-10-30--retained-baseram-snapshots-do-not-identify-the-active-jp-map).

For a source-scoped Drator byte-consumer replay, the wrapper also supports
`THERON_CAPTURE_SPAWN_CONSUMER_PC_FILTER=drator-row`. This PCE Fast-only mode
records reads of physical `$1F093B-$1F093D` only at the observed US reader
addresses `$CBC9`/`$0DABC9` and `$45C3`/`$0D05C3`. The versioned sidecar parser
checks the data range, logical address window, PC/address pair and sequence.
The receipt remains non-semantic. A changed physical bank intentionally
produces no accepted row until a new authentic source mapping is captured.
Use this with the System Card `run-path` filter below to keep both traces
bounded; neither filter alone establishes a complete capture or gameplay.
The focused parser, wrapper and reference-patch contract checks passed three
consecutive loops; the later authentic cold US capture used this spawn filter.
The package remained rejected because the live `$45C3` source check failed and
other traces reached their caps. See the [completed tooling entry](../../DONE-theron.md#2026-11-01--add-source-scoped-drator-row-consumer-trace-filter).
Generated-row semantics remain open.

The reference capture wrapper now supports
`THERON_CAPTURE_SYSTEM_CARD_IO_RAM_PC_FILTER=run-path`. This source-scoped mode
keeps only System Card data reads at `$EA4D`, callback writes at `$EA50`, and
the callback-result read at `$E162`; the verifier checks each logical PC and
the captured instruction bytes against the hash-verified System Card image.
It also admits the observed callback target `$2259`. The default `none` mode
retains the broader trace. The filtered receipt excludes repeated `$1800`
status-poll instructions by design. It reduces trace volume but does not prove
a completed BIOS handoff or gameplay. The later authentic US capture used this
mode; its package still failed at live `$45C3` source verification and trace
caps. A future accepted cold handoff must pass the independent callback, input
and transition verifiers.

Three consecutive loops passed the focused instrumenter, streaming-verifier
and wrapper-script regressions for this mode. These loops exercise the
instrumentation and fail-closed row validation; they do not represent a game
capture. See the [capture-tooling work-log entry](../../DONE-theron.md#2026-11-01--add-source-scoped-system-card-ioram-capture-mode) and the
[System Card RUN source lock](../source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md).

The isolated 2026-10-31 authentic US cold run reached Drator `start`,
`left-event` and `reentered-event` at level `$03`, position `$02,$03`. The
rebuilt reference binary recorded 132 `$C852` fetches at physical `$0DC852`.
The first five
512-byte runtime windows have SHA-256
`c2396321541223629083719fe542d90c5ba81ad63e85af1ba9353aa339e72351`, exactly
matching the authentic US Track 02 candidate at raw offset `$A3AD2`. The
[HuC6280 disassembly](../source-lock/theron-disassembly/theron-us-c852-a3ad2-20261031.asm)
records this 512-byte source window. Later
windows vary as execution advances. This maps the captured runtime code window
to that source occurrence; it does not prove `$C852` is the generated-row
consumer or assign semantics to the row bytes. The filtered
consumer trace filled its 1,048,576-row sample limit before route reentry; the
route later reached reentry. The trace retained a same-session byte join:
writes to physical `$1F093B-$1F093D`
(`$02,$03,$00`) at events 10,644,901, 10,644,902 and 10,644,910 are followed
by reads of those bytes at events 10,644,912–10,644,914. A later PC `$45C3`
`TII $2935,$2400,$000A` reads the same offsets and values at events
10,647,071–10,647,073. The target-write trace retained 139 rows. Callback-state
verification passed with 412,791 rows (87 reads and 412,704 writes), but the
System Card I/O/RAM trace reached its 8,388,608-row cap. Its former verifier
loaded all 8.4 million rows into a list and reached about 3.2 GB RSS before it
was stopped. The current streaming verifier processed the trace with about
18 MB RSS and correctly rejected the explicit cap marker. The wrapper
completed, but the capture package is not accepted because the trace is
truncated. Iterator-mode regression coverage passes.

The generator receipt and this cold run now prove a same-session write/read
match for row offsets 6–8 and a ten-byte copy into a work buffer. The capture
does not prove that the `$C852` fetch itself reads those bytes. Keep row bytes
semantically opaque and trace the consumer's downstream use. The
streaming I/O/RAM verifier uses bounded memory but correctly rejects this
capture's explicit truncation marker. This capture does not
close the separate native
viewport gap, which still requires a same-run map/pose-to-VDC/VCE/VRAM-to-frame
join. See the
[generator source-lock report](../source-lock/theron-generator-rng-state-2026-08-21.md)
and [active TODO](../../TODO-theron.md).

A static source search now finds two compatible US Track 02 instruction
candidates for the captured Drator row readers. The `$CBC9` trace PC fits an
absolute,Y load beginning at `$CBC6`; the `$45C3` candidate is a ten-byte TII
from `$2935` to `$2400`, including workspace offsets 6–8. Each appears in
the verifier reports four matching raw offsets for `$CBC6` and three for
`$45C3`. These constructed retail-byte windows narrow the disassembly target,
but do not prove that either window was fetched during the authentic replay,
bind a unique runtime occurrence or LBA, or assign row-field semantics. See the
[reader source-candidate report](../source-lock/theron-disassembly/theron-us-drator-row-reader-source-map-20261101.md).

The complete clean-source Mednafen 1.32.1 patch chain applied in three
consecutive loops. A fresh instrumented PCE Fast build against real SDL2 2.33.0
compiled and installed, and the isolated capture-wrapper runtime verifier
passed for both compiled and running SDL versions. This clears the former
`sdl2-compat` runtime gate. The 2026-10-31 run has since reached Drator and
proved the generated-row byte reads described above; `$C852` source mapping is
resolved for the captured code window, while downstream buffer semantics
remain open. No Drator-positioned save or state artifact is checked into the repository. Use a
separately verified authentic state or reproduce the original Drator route
with hash-verified media in a new isolated profile and evidence directory; do
not reuse an occupied capture profile.

Firestaff runs Theron in its native C/SDL runtime and skips the Firestaff Run
menu and System Card on direct launch. Authentic JP fresh-start and US
Continue routes reach loaded dungeons. The native input panel has bounded
keyboard and mouse regressions for movement, champion selection, wait, and
US pickup/reselect/drop; USE command effects remain unverified. Native JP M11
rejects the authentic empty BRAM, while positive JP progress restoration remains
untested. Current claims are bounded routes, not complete gameplay or visual
parity.

The 2026-11-01 source update admits the eight creature-pointer records and
five regular-spawn zone records from authenticated US and JP full-ISO
projections. Each decoded record matches its regional raw Track 02 BIN, and
native ISO dungeon startup now binds those records. The first five pointer
records' source-relative offsets (`$0058`, `$00D7`, `$0102`, `$0129`, `$0150`)
resolve in order to the five authenticated regular-spawn zone records in both
US and JP data; the remaining three pointer offsets are zero. This is an
authenticated static table join only. It does not establish runtime pointer
selection, category meaning, a spawn consumer, RNG, creature generation, or
combat. See the
[ISO projection source lock](../source-lock/theron-track02-spawn-record-iso-projection-20261101.md).

On 2026-10-31, a clean macOS Debug/SDL3 build passed three repetitions each of
the authentic USA and Japanese Rev. 1 native SDL input suites. These runs
verify event dispatch and bounded state changes; they do not close original
mouse-hitbox, rendered-frame or complete retail-input parity.

The Theron CUE resolver now permits same-stem `.ogg` substitutions only for
CUE-declared `.wav` audio tracks. A focused three-run regression confirms that
such an audio alias remains accepted and that an OGG cannot satisfy a missing
Track 02 data file. This protects media admission; it does not verify CD-DA
playback or audio parity.

## 2026-10-31 — matched JP state-only comparison records candidate transfers

Three PCE Fast replays from the authentic Akutuba F5 state recorded the same
Button II+UP guest reads followed by `$C1FC` TII transfers from `$20B4` into
candidate bytes `$2040/$2041`. Three matched Button II-only controls recorded
no such transfer and retained the prior values. The input-conditioned
transfer repeated, but active-map identity, position semantics, collision
behavior, and any rendered effect remain unproven. VRAM snapshots differ in
two action repetitions, and all six runs report `transition=missing` with no
authenticated CD-to-RAM receipt. This is reference evidence only; it does not
establish native gameplay parity. See the
[state-only comparison](../../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md).
The same `$C1FC` TII is associated in the source-locked teleporter analysis
with copying destination coordinates. A follow-up three-run trace records an
identical 128-instruction context into `$C1FC`, including a `$C1D6` call/return
and conditional fall-through. This bounds the dynamic path but does not
source-bind the tested fields, establish the routine's gameplay purpose, or
identify the active-map object. The movement-versus-teleporter question stays
open. See the [control-flow capture](../source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md).

## 2026-10-31 — corrected input-gated PCE Fast VRAM trace captured

The complete reference-only Mednafen trace patch chain applied in three
isolated regression loops, and the corrected instrumented PCE Fast binary
built and installed on TRV2. The callback passes guest-visible
`pce_jp_data[index]` to the gate. A bounded authentic JP Akutuba-state replay
then recorded the scripted UP at guest PC `$44C1`, port `raw=0010`, event
sequence `58758`; its first gated VRAM commit follows at shared event sequence
`58761`. The capture wrapper validated the V5 trace schema, 4,096 contiguous
commit rows, same-frame input receipt, and close-time snapshots. Its
state-only mode marks cold System Card checks not applicable because autoload
bypasses that path.

The transition receipt remains `missing`; no cold-start CD-to-RAM handoff,
map binding, semantic VRAM interpretation, visible input effect, causality,
or renderer/Firestaff comparison is established. This verifies gated trace
capture only and does not change native Theron support or parity status. See
the [source-locked capture report](../source-lock/theron-disassembly/theron-jp-pce-fast-input-gated-vram-trace-20261031.md)
and [Theron TODO](../../TODO-theron.md).

## 2026-10-31 — JP title-menu four-second Button I hold visibly advances

An isolated cold Mednafen 1.32.1 PCE Fast run used hash-verified JP Rev. 1
Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`). On the title menu, a four-second Button I
hold with NEW GAME selected opened file selection; a second four-second hold
with FILE 1 selected entered the Akutuba story screens. Twelve one-second
Button I presses with release intervals reached the first-person dungeon.
This visibly confirms the reported long hold for the JP retail title route.
A Mednafen `.mc0` save state was created in the isolated profile; it is an
emulator snapshot, not in-game BRAM. The screenshots and state remain outside
Git. The capture does not provide a joined game-owned input/CD-loader receipt,
completed stage, positive BRAM, native timing, or Firestaff parity. See the
[source-locked route and artifact hashes](../source-lock/theron-jp-four-second-title-action-capture-20261031.md).

## 2026-10-31 — native Debug direct launch bypasses Firestaff's Run menu

An isolated macOS Debug build launched the native Theron title/startup path
directly from authentic US Track 02 data without showing Firestaff's Run menu
or booting a PC Engine System Card. The `--boot-probe` receipt remained in the
startup phase with `levelLoaded=0`; this is a launch-boundary check, not a
loaded-dungeon or gameplay result. The existing JP fresh-start and US Continue
tests remain the evidence for reaching native dungeons. See the
[source-locked launch note](../source-lock/theron-native-debug-direct-launch-20261031.md).

## 2026-10-31 — authentic JP empty BRAM at native startup

The native M11 startup integration pairs authentic Japanese Rev. 1 Track 02
with the captured empty JP SRAM (BRAM). The file has a valid container and
body layout but a zero campaign byte, so M11 correctly withholds Continue and
keeps the new Akutuba route unchanged. The integration passed three TRV2
Debug loops and verifies that neither source file changes. This is a negative
empty-slot boundary only: no authentic progressed JP BRAM is available, so
positive JP Continue/save round-trip and retail JP save compatibility remain
unverified. See the [source-locked save-body report](../source-lock/theron-original-backup-ram-body-layout-2026-09-23.md)
and [`DONE-theron.md`](../../DONE-theron.md).

## 2026-10-31 — cold JP emulator route reaches Akutuba but saves no progress

An isolated Mednafen 1.32.1 PCE Fast profile booted the authentic Japanese
Rev. 1 disc from cold media, held RUN for four seconds at the System Card
prompt, selected New Game and the first file, advanced the Akutuba story
screens, and reached the first-person dungeon. The profile was kept separate
from Firestaff data. Its only 2,048-byte `.sav` output has SHA-256
`de8e415730226a1f0e39666b1ea291b6abec07bcaeb7223dc33ea01a71f89eaa` and
MD5 `dbdedb0ec809227b289c2bc5b18b9c9d`, exactly matching the already
verified empty JP BRAM. No stage-clear or progressed BRAM was produced. The
screenshots and emulator state remain in the private TRV2 Theron evidence
store and outside Git. They are reference artifacts, not Firestaff output or
parity evidence. The
cold route confirms the long RUN hold and New Game navigation, but not a
game-owned input trace, stage completion, positive save/Continue, or native
rendering parity. Continue by completing the original Akutuba stage and
validating any resulting BRAM before using it as positive-save evidence.

## 2026-10-31 — JP PCE Fast poll-to-RAM event ordering

Three isolated emulator runs replayed `up@61:120` from an authentic JP
Akutuba F5 dungeon state. The PCE Fast trace binds the scripted mask and
controller slot to the same-frame active-low guest read at `$44C1`
(`raw=0010`, `value=3e`). A further three UP runs from the same exact state
and three neutral controls compare the `$2912` main-RAM consumer: each UP run
records `$D334` reading `00`, `$D349` writing `10`, then `$D334` reading `10`;
the controls show no `$D334` or `$D349` access. The v2 verifier rejects
ambiguous module/slot/mask records and validates the shared order: poll event
2342, `$D334` read event 3000, `$D349` write event 3001, and later `$D334` read
event 3023 in each UP run. This is an event-order join, but does not prove the
poll caused the write. `$2912`'s meaning, party movement, and screen effect
remain unproven. See the [guest-read
receipt](../source-lock/theron-disassembly/theron-jp-pce-fast-scripted-input-receipt-20261031.md)
and [same-state comparison](../source-lock/theron-disassembly/theron-jp-pcefast-input-state-compare-20261031.md).

A follow-up single UP/neutral pair reproduces the `$D334` read → `$D349` write
→ `$D334` read sequence. The authenticated F5 pose faces a wall: separate UP,
neutral, and DOWN screenshots retain the same front wall, and the bounded
observer records no `$C1FC` TII or `$2040/$2041` position write. This is a
blocked-wall observation, not a gameplay or renderer result. A later matched
directional replay observed repeatable candidate-position transfers, but
active-map identification, collision semantics, and a screen join remain
open. See the [comparison](../../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md)
and the historical [blocked-wall
probe](../source-lock/theron-disassembly/theron-jp-up-input-latch-blocked-wall-probe-20261031.md).

## 2026-10-31 — authentic JP and US native routes pass a macOS Debug build

A fresh isolated macOS Debug configuration built the native Firestaff target
and its Theron real-media CTest integrations. JP fresh-start passed against
hash-verified original Track 02, and US Continue/save passed against
hash-verified US Track 02 plus the authentic Ak-Tu-Ba-complete BRAM. The tests
isolate user-data paths and check that their original media inputs remain
unchanged. This confirms bounded native startup/save routes on macOS; it does
not establish retail presentation parity, complete campaign behavior, or the
missing cold game-owned handoff. Detailed hashes and scope are in
[`DONE-theron.md`](../../DONE-theron.md).

## 2026-10-31 — current capture and renderer checkpoint

The supported Firestaff route remains native C/SDL. Authentic US Continue
evidence reaches dungeon 2, and the Japanese fresh-start route reaches a loaded
dungeon; these establish bounded host/game-data paths, not a source-bound
retail renderer. The production viewport still needs a same-run join from
authenticated Track 02 map identity and party pose through game-owned VDC/VCE
registers, VRAM/BAT and rendered output. A matched no-input reference frame is
also required to separate display animation from state-driven scene changes.

The available original-reference evidence that can be promoted today is
narrower: a Japanese dungeon capture shows host mouse motion moving the retail
hand cursor, but does not establish guest coordinates or click semantics. The
US save-state route records raw mouse input reaching guest reads and candidate
command-consumer activity, but does not bind the raw bit to Button I or to a
committed party movement or viewport change. The Japanese close-time RUN trace
ends in System Card BIOS. A newer cold US trace records one `$E009` dispatch. A follow-up validates
all three scripted inputs at System Card controller polls, but observes no non-System-Card
poll, runtime CD source-read join, or authenticated CD-to-RAM handoff. It does
not establish a game-owned handoff, title-menu action, or gameplay input
semantics. See the [returned-mask capture report](../source-lock/theron-disassembly/theron-us-cold-scripted-input-returned-mask-20261031.md).

A separate US saved-state PCE Fast run loaded its existing state and recorded
scripted input-apply events. Its shared input trace contains 1,354
short-format rows at `$44CA/$44DB`, all reporting `raw=0000`; because standard
PCE and PCE Fast instrumentation share a path and row label, these are not a
verified PCE Fast read receipt. RAM, VRAM, and SAT differ from the control
snapshot, while VCE and VDC state match; those changes are not joined to a
received input, party pose, map cell, or display consumer. This proves neither
action semantics nor game ownership and cannot supply a tile mapping. See the
[saved-state input boundary report](../source-lock/theron-disassembly/theron-us-state-scripted-input-boundary-20261031.md).

Instrumented Mednafen builds are diagnostic reference tools only; none are part
of Firestaff's shipped runtime. A successful emulator build or input-delivery
receipt does not by itself establish original-game behavior or native parity.
Do not substitute fixture tile mappings, synthetic screens, or edited save
states for these missing real-data joins. See the [Theron TODO](../../TODO-theron.md),
[source-locked capture notes](../source-lock/theron-disassembly/), and
[native platform status](../PLATFORM_STATUS.md#therons-quest) for the current
work order and detailed evidence.

## 2026-10-31 — authentic US PCE mouse-input capture boundary

The preserved US gameplay state (MD5 `f17f377df210b4a3ae904a13fb85a7f0`)
loaded in standard PCE in the earlier capture build; PCE Fast rejected its
serialization format in that build. A later instrumented PCE Fast binary
(MD5 `cdef1c8ebe6ec3248ea19559cdd13fe3`) reports a successful load of the same
state. Its scripted-input trace is still inconclusive because every guest
input-read record reports `raw=0000`; see the [saved-state input boundary
report](../source-lock/theron-disassembly/theron-us-state-scripted-input-boundary-20261031.md).
The earlier successful standard-PCE replay used authentic US Track 02 MODE1/2352 media (MD5
`f23601102138f87c33025877767ebf76`) and the verified System Card. The state
already contains dungeon-bank bytes `$20DA=01`, `$20DB=00`.

One neutral capture and three repeated captures with raw mouse-button mask
`0x01` held for 60 updates were made from that exact state and media. All three
held-input captures agree on close-time RAM and command-consumer stream. The
input trace records 60 scripted updates and 60 guest device reads. The command
consumer reads logical `$2905=$50` twice at PC `$D34D` in the instrumented
listing; across the 473 reads, these are dispatch observations, not a decoded
game action. Close-time RAM differs from neutral in 24 bytes, while VRAM, VCE,
SAT and VDC snapshots match the neutral capture. Known candidate position
bytes `$2031`, `$203F`, `$2040`, `$2041`, and pointers `$2E3B`/`$2E7B` are
unchanged. A separate motion-only probe latched hardware delta `x=-10`, but
did not change those bytes or the close-time display snapshots.

All replay validations end `transition=missing`: loading the save state skips
the original CD-to-RAM transfer, so no authenticated runtime media-origin
receipt exists. These captures establish guest mouse-read and command-consumer
activity only. They do not bind mask `0x01` to Button I, prove movement or an
item action, identify a target, establish a game-owned pose/map join, or verify
rendering/parity. The [source-locked capture report](../source-lock/theron-disassembly/theron-us-pce-mouse-held-command-capture-20261031.md)
records artifact hashes and the full evidence boundary.

Capture tooling fixes are also retained: optional BRAM arguments are now added
only when a verified filename exists, avoiding macOS Bash `nounset` failure;
the PCE Fast TIA zero-page instrumentation patch is opt-in. The isolated
patch-only pipeline passed three loops, and the instrumented PCE Fast build
passed the SDL 2.32.10 runtime check. Mednafen remains a diagnostic reference
tool and is not part of Firestaff's native C/SDL runtime.

Three fresh TRV2 patch-only runs applied the normal-PCE instruction-start
consumer trace sequence through selected-record and guest-input instrumenters.
The instruction PC and physical mapping are captured before opcode fetch, and
the fetched opcode is latched afterward. The instrumented Mednafen 1.32.1 PCE
build and install succeeded. Controller-patch dry-run and compiled mouse parser
checks each passed three loops. A subsequent cold US capture recorded one
`$E009` dispatch. A follow-up cold run validates three scripted inputs at System Card controller polls; no
game-owned poll or runtime CD source-read join was observed. This is bounded
input-delivery evidence only; native runtime evidence and parity gaps are
unchanged. See the [returned-mask capture report](../source-lock/theron-disassembly/theron-us-cold-scripted-input-returned-mask-20261031.md).

## 2026-10-31 — PCE Fast capture tooling repaired

The isolated reference-capture wrapper previously failed under macOS Bash
`nounset` when its optional authentic-BRAM argument list was empty. The wrapper
now adds that Mednafen option only when a verified BRAM filename exists. The
PCE Fast build also applies its expensive zero-page TIA tracing patch only when
the corresponding opt-in trace flag is enabled; the complete patch-only
pipeline passed three isolated loops, and a PCE Fast instrumentation build
passed the real SDL 2.32.10 runtime check.

The earlier PCE Fast build rejected the preserved US state's serialization
schema, while a later instrumented PCE Fast binary reported successful loading
of the same state. The shared input trace remains ambiguous as described above.
No game media, save state or raw capture was committed.

## 2026-10-31 — cold System Card RUN callback remains before game handoff

The authentic Japanese cold `run@480:300` capture records one System Card
`RUN` event and no `SELECT` event. Shared-sequence I/O and RAM observations
bind `$1800=$C8`, `$1801=$04`, the resulting `$2259=$04` store, and the BIOS
branch `$E166->$E176`. No `$E174`, `$E11D`, `$E130`, or `$E209` receipt or
confirmed game-owned handoff appears. This identifies the callback byte and
BIOS path only; it does not assign meaning to `$2259` or demonstrate title
menu input or gameplay. See the [source-locked report](../source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md#2026-10-31-r5-callback-and-transfer-boundary)
and [100-instruction System Card listing](../source-lock/theron-disassembly/theron-system-card3-ea3b-20261031.asm).

The native Continue and fresh-campaign evidence below is independent of this
reference-only System Card trace.

## 2026-10-31 — authentic BRAM Continue reaches native dungeon 2

The native `theron_v1_m11_real_bram_continue` integration test passed three
TRV2 loops with authentic US Track 02 and the authentic Ak-Tu-Ba-complete
Backup RAM image. It verifies source-backed campaign and Theron stat restore,
Continue focus and acceptance, next-dungeon/Soul Room progression, forcefield
admission, and three native floor moves. A separate 10-frame CLI boot-probe
with no input remained at the title; it does not exercise Continue. The native
test is not a paired retail comparison and leaves the full-screen renderer and
later quest/save paths unresolved. See the dated result in
[`DONE-theron.md`](../../DONE-theron.md).

Firestaff runs Theron's Quest in its native C/SDL runtime. Mednafen is used
only as an external reference instrument for authentic original-game behavior
and data capture; it is never part of the shipped runtime or a substitute for
native Firestaff gameplay.

## 2026-10-29 — PCE mouse motion visibly moves the original cursor

A cold authentic Japanese Rev. 1 Mednafen PCE Fast run switched port 1 to the
emulated mouse at the dungeon. Two screenshots with host-mouse positions 150
screen pixels apart differ only in a 41×51-pixel box around the original hand
cursor. This confirms visual cursor motion in the original game. It does not
identify guest coordinates or prove what a click does: the probe has no
command or state-transition receipt, and the later click's black viewport is
inconclusive. See the
[source-locked capture note](../source-lock/theron-disassembly/theron-jp-mouse-pointer-motion-20261029.md).

## 2026-10-29 — cold Japanese reference route reaches dungeon view

A fresh Mednafen 1.32.1 `pce_fast` run opened the authentic Japanese Rev. 1
disc without loading a save-state. It showed story pages, a fade and then a
first-person dungeon after six separate 250 ms Button I presses. This
supersedes the earlier r1 capture's stopping point: a dungeon screenshot is
now available, but the r2 run did not include an instrumented guest-input,
CD-to-RAM, or command receipt. A separate mouse-mode capture confirms that
host-mouse motion moves the original hand cursor; neither screenshot sequence
proves the guest coordinates or click behavior. Mednafen reported the SBI
sidecar missing, and the full headered System Card image was unavailable. The
retail manual documents D-pad cursor movement and Button I actions; it does not
document physical mouse input. These captures do not prove the transition's
source provenance, gameplay behavior, or Firestaff parity. The private
screenshots, log and hash manifest remain outside Git in the TRV2 Theron
evidence store.

The separate [cold mouse-mode Run trace](../source-lock/theron-disassembly/theron-jp-cold-mouse-run-trace-20261029.md)
records Run and Button I at the original System Card poll, but the game
handoff still lacks CD IRQ and loader receipts. Mednafen also logged the
Japanese SBI sidecar missing; its role in the missing handoff is unknown.

## 2026-10-29 — authentic Japanese stage-message advancement captured

A cold Japanese Rev. 1 reference run used original Track 02, the canonical
headerless System Card 3.0 body, and the original SBI sidecar without loading
a save-state. Three real screenshots capture the Ak-Tu-Ba stage card, its
first story page after Button I, and a later page after another short Button I
press. The active-low Button I poll value `$3E` is present in the controller
trace. This proves one visible story-page transition after controller input.
It does not show hero selection or a dungeon, and it does not establish native
parity. The full headered card image was unavailable. Private evidence,
including hashes, remains outside Git in the TRV2 Theron evidence store.

## 2026-10-26 — PCE Fast captures default to a close-time snapshot signal

The reference capture launcher now defaults PCE Fast to `TERM` unless the
caller explicitly selects another shutdown signal. In an isolated TRV2 probe
using the authentic Japanese Rev. 1 disc and preserved gameplay state, `TERM`
reached Mednafen's `CloseGame` boundary and wrote complete 8 KiB main-RAM,
64 KiB VRAM, VCE, SAT and VDC-state snapshots. The prior PCE Fast `INT`
default did not exit that older instrumented binary before the bounded force
kill and therefore produced no snapshots.

This selects a reliable reference-capture shutdown path only. It does not
assign meaning to any captured RAM byte, prove a party pose or movement, or
alter Firestaff's native runtime.

## 2026-10-26 — native launch does not expose the System Card Run menu

Firestaff's Theron launcher consumes a hash-verified Track 02 image and enters
the native C/SDL startup/runtime path. It does not boot a PC Engine System Card,
display its `RUN` menu, or require a controller-button hold to cross that
firmware screen. System Card menus and their hold timing remain reference-only
capture evidence and are not part of the shipped game flow.

The native route may still present source-backed Theron title, stage-selection,
and Soul Room states when a fresh game requires them. It may enter a loaded
dungeon only through a verified Track 02 level/party handoff. Firestaff must
not replace those states with a guessed party, position, or level merely to
shorten startup; full direct-to-dungeon launch needs an authenticated initial
party-and-level receipt.

## 2026-10-26 — five-second cold-start RUN is delivered but remains System Card-only

An isolated `pce_fast` reference capture used the authenticated Japanese Rev.
1 disc and verified System Card 3.0 with `run@9600:300`: a five-second RUN
hold beginning at reference frame 9,600. The scripted-plan and
controller-read receipts both passed. The active-low RUN mask appeared in 414
controller reads during the hold, but every witnessed poll belonged to System
Card PC `$E4C5`; no non-System-Card poll followed.

The run also produced close-time PCE Fast RAM, VRAM, VCE, SAT and VDC-state
snapshots, plus source-bound CD data-port reads. It produced no raw-sector
span, SCSI command, authenticated CD-to-RAM origin receipt, CD IRQ, or game
`$E009` dispatch. The capture helper therefore rejected it as
`transition=missing`. This proves delivery of the requested long RUN hold to
the original controller path only. It does not prove a Theron title/menu
action, game launch, dungeon admission, or native runtime parity. Raw capture
artifacts are discarded after the summary is recorded.

## Bounded cold CD-read trace options

The PCE Fast reference wrapper leaves the CD data-port trace at its historical
65,536-row all-source cap by default. A cold source-join capture may set
`THERON_CAPTURE_PCE_FAST_CD_DATA_READ_START_AT_TRACK02=1` to record only
source-valid data-port reads whose LBA falls inside the selected Track 02, and
`THERON_CAPTURE_PCE_FAST_CD_DATA_READ_TRACE_LIMIT` to choose a cap from 65,536
through 1,048,576 rows. The wrapper records both values in its transition
receipt. This filter applies only to that diagnostic sidecar; separate
CD-to-RAM, CPU, VDC and VCE traces keep their own bounds and meanings. It does
not prove a CD-to-RAM consumer or screen mapping until those receipts are
joined in the same cold session. The 2026-10-29 Japanese cold capture and its
limits are summarized in the [filtered Track 02 capture note](../source-lock/theron-disassembly/theron-jp-cold-track02-filtered-input-20261029.md).

## 2026-10-24 — authentic runtime input comparison remains inconclusive

## 2026-10-26 — frame-windowed PCE Fast replay still has no gameplay receipt

An isolated replay from the preserved authentic Japanese dungeon state used a
five-second Button I hold followed by held UP and RIGHT inputs. The replay
plan, application records, and non-System-Card controller reads agreed for all
three events. A bounded movement-context window starting immediately before
the directional events captured the original CPU's input-latch, data-read and
runtime-state activity, but no observed movement-command write followed either
direction. This is evidence of controller delivery only. It does not identify
a party coordinate, map, movement outcome, or renderer input, and does not
authorize native dynamic rendering. The private reference build and capture
artifacts were discarded after analysis; no game media or state was committed.

Three isolated PCE Fast captures on TRV2 loaded the same preserved Japanese
gameplay state, authentic Rev. 1 Track 02, and the verified System Card. A
scripted UP event and matched LEFT/RIGHT events each reached the non-System
Card controller poll at HuC6280 PC `$44D2`. The LEFT and RIGHT captures retained
identical party-RAM trace hashes and identical candidate bytes at `$2031`,
`$203F`, `$2040`, and `$2041`; no authenticated CD-to-RAM receipt or CD IRQ was
observed. The close-time VRAM hashes differed, while LEFT and RIGHT VDC state
and VCE hashes matched. With no VDC write trace or proven RAM field semantics,
those VRAM differences cannot be attributed to party movement. This confirms
input polling only; it does not establish movement, a map-to-graphics binding,
or dynamic-renderer parity.

The captures and bounded receipts were kept in a private TRV2 task cache and
removed after analysis; no game media, savestate, BIOS, or raw memory image was
added to the repository.
The reproducible capture entry point is
`scripts/capture_theron_mednafen_live_trace.sh` with
`THERON_CAPTURE_AUTOLOAD_STATE`, `THERON_CAPTURE_REPLAY_INPUT_SCRIPT`, and
`THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast` set against authentic local media.

For a narrow follow-up probe, `THERON_CAPTURE_PCE_FAST_MOVEMENT_CONTEXT_MAIN_RAM_RANGE`
accepts one inclusive physical-RAM range as `first-last` hexadecimal addresses.
It records only writes in that range with the executing PC and MPR map. The
range is disabled by default; selecting one produces provenance evidence, not
field semantics or permission to enable dynamic rendering.

The bounded movement-context sidecar records the reference frame and the full
MPR0--MPR7 map for every observed input-latch or runtime-state write. This
makes a future same-session capture auditable against its scripted controller
event and executing physical bank. The sidecar is provenance evidence only:
it does not make a RAM field semantic, identify a Track 02 source offset, or
permit Firestaff to enable dynamic rendering.

## 2026-10-24 — authenticated screen snapshot renders; world pairing remains open

Three TRV2 Firestaff `--debug --boot-probe` loops accepted authentic US Track
02 and a hash-allowlisted VDC capture bundle. Each produced identical native
320x200 source and presented frames with 35,452 non-black pixels. This proves
capture admission and screen-space rendering in Firestaff. The captured image
is a frozen authentic frame: the boot probe had `gameTick=0`, no movement, and
the original screen has not been joined to Firestaff's loaded dungeon or party
coordinates. It is not evidence of dynamic dungeon rendering or visual/gameplay
parity. The receipt and reproduction command are recorded in
[`theron_v1_authenticated_snapshot_render_receipt_20261024`](../../parity-evidence/verification/theron_v1_authenticated_snapshot_render_receipt_20261024).

The follow-up native route reached party `(2,3,dir1)` and `gameTick=17` while
presenting the same frozen frame hash. This pose match remains a candidate
only: the capture's map/level fields are not source-joined to Firestaff's
world. The production renderer still needs an authenticated live
square-to-screen/material binding before it can replace the snapshot with
state-driven gameplay. Mouse hit regions likewise remain unavailable until
that native view exposes meaningful controls.

## Earlier 2026-10-24 — native menu input reaches the dungeon, one presentation path was black

A local macOS Debug build ran Firestaff's native C/SDL application with
authentic Japanese Rev. 1 `TQJP02.bin` copied from the user's TRV2 data
directory into the ignored local game-data directory. Firestaff's own SDL
script input selected a stage, opened the Soul Room, moved its cursor to the
forcefield entry, and entered a loaded dungeon. All three repeated runs
reported `level-loaded=1`, map 0, party `(1,0)`, `THERON READY`, and nonzero
game ticks and rendered frames. The reported champion count varied between
one and two. Measured intervals reported `game-ticks-per-sec=2.80–3.00`,
`game-frames-rendered-per-sec=4.99–5.19`, and
`game-frames-presented-per-sec=4.99–5.19`. The Firestaff-presented 1600x1000
capture remained black (SHA-256
`160ee7a7f88cd678205f9aceced67995f35203c647f6fc71ec104f556bdcd733`). This
verifies native startup, input, dungeon admission, and tick/render loop
activity, but not visible playability. The scripted inputs were discrete
key-down events and did not verify a continuous four-second hold in the
native runtime. No
emulator was used to run Theron. The next parity work is to bind authentic
graphics, palette, and tile mapping into the production native renderer.
The US/JP Stage-2 `$46B8` palette callers and their regional VCE routines are
now source-bound across raw MODE1/2352 sectors, including a shared 64-byte
payload written at VCE index `$E0`. The command stream's screen association
and matching graphics/pixel route remain unproven; see the
[palette source note](../source-lock/theron-stage2-static-vce-palette-20261024.md).

An opt-in PCE Fast write trace is available for the source-bound routine window
`$1F1C00–$1F1CFF` and physical RAM `$0D0000–$0D1FFF`. Enable it with
`FIRESTAFF_THERON_PCE_FAST_CODE_RAM_WRITE_TRACE_SUPPORT=1` when building the
instrumented reference binary, then `THERON_CAPTURE_PCE_FAST_CODE_RAM_WRITE_TRACE=1`
on a PCE Fast capture. It records the CPU logical and physical write addresses,
active MPR values, instruction PC, registers, and a shared event sequence,
capped at 65,536 records. The hook reports open/write errors and capture validation
rejects missing or invalid trace receipts. This instrumentation is for reference captures only;
output remains untrusted for native rendering until matched to authentic code,
source bytes, and a later gameplay/render transition. See the
[decoded-buffer bit-reader note](../source-lock/theron-disassembly/theron-us-decoded-buffer-bit-reader-20261029.md).
Consumer reads can be bounded with
`THERON_CAPTURE_MAIN_RAM_CONSUMER_SAMPLE_LIMIT` (4,096–1,048,576; default
65,536). The default `$1F0000–$1F7FFF` consumer stream and an optional physical
range selected by `THERON_CAPTURE_PCE_FAST_MAIN_RAM_CONSUMER_RANGE` each have
their own limit, so early default-RAM reads cannot consume the selected range's
budget. Records identify `sample_class`; each stream writes its own explicit
truncation marker when a configured sample cap or the per-address cap drops a
read. Reaching the total cap alone does not mark a stream truncated. A truncated
stream is incomplete for absence or coverage claims. This range selects
physical data-read addresses only; it cannot select an instruction PC. To
observe an instruction address, use a bounded opcode-fetch hook at the CPU
fetch/dispatch boundary.

## 2026-10-29 — decoded-buffer read candidates

A follow-up cold USA reference capture verified the ordered 5,064-byte `$23AD`
output, then recorded 29,618 reads from that physical output span. All 5,064
output addresses were read after the final decoder write; 6,111 reads matched
the corresponding decoded byte across 3,130 addresses. The strongest reader
groups are physical PCs `$1042AB`, `$1042B2`, `$104287`, `$10428C`, and
`$1F1C3E`. This run used an earlier aggregate-cap trace binary, so its
per-stream counts were reconstructed from physical addresses; separate stream
caps were independently exercised in the preceding capture. The run did not
record a CD IRQ, `$E009` dispatch, or gameplay transition, and the last
scripted input lacked a controller-poll witness. These reads remain candidates
until their destination writes are traced into an authentic game-state or
rendering effect. See the [decoder bit-reader note](../source-lock/theron-disassembly/theron-us-decoded-buffer-bit-reader-20261029.md).

## Earlier 2026-10-24 — TRV2 headless native full-loop presentation check

Firestaff was built from source commit `d43df8905e22a207696f4439bde7afb02b808b53`
on TRV2 and run three times for 15 seconds through the native application
path with authentic Japanese Rev. 1 Track 02. Each run admitted the verified
Track 02 file and kept the SDL host loop active at about 62.5 loops per
second. The Debug perf receipts reported zero game ticks, zero rendered game
frames, and zero presented frames in every run. Firestaff's own presented
surface capture was 320x200 and black in all three runs. `SDL_VIDEODRIVER=dummy`
was required because the TRV2 session had no `DISPLAY` or `WAYLAND_DISPLAY`,
so no interactive window or input was verified. This is stronger than the
boot-probe-only startup receipt, but still does not establish visible native
gameplay. Firestaff reported that the later original graphics bank is not
captured and fallback visuals stay disabled. The next parity blocker is to
bind and decode authentic graphics data and then verify nonzero native game
ticks and presented frames. No emulator participated in these Firestaff runs.

## 2026-10-24 — current evidence boundary

An isolated Debug build from clean commit `2748eed3db4a3a9d7b1db2e1522b050440b3e0a2`
was built on TRV2 and exercised three times through Firestaff's native
`--debug --boot-probe` route with the authentic JP Rev. 1 Track 02 image. Each
run reported `phase=theron-runtime`, `levelLoaded=1`, party `(1,0,0)`, two
champions, 291 source objects, four loaded dungeon levels, and 455 nonzero
source tiles. The probe also reported `frames=0` and `runtimeTick=0`. This
confirms a clean native build and bounded startup/data admission; it does not
confirm interactive gameplay, dungeon drawing, or parity. The isolated source
and build directory was removed from `/dev/shm` after the three loops.

The Oct. 24 pit safety change was then built from a checksum-verified source
snapshot on TRV2. The real-media `theron_v1_track02_dungeon_loader` test passed
three loops against authentic US Track 02 MD5
`f23601102138f87c33025877767ebf76` and JP Track 02 MD5
`b7afb338ad31be1025b53f9aff12d73a`. Each loop found 18 authentic pit-target
party-actuator occurrences per edition and retriggered each 128 times; the
queue retained one equivalent pending occurrence and the target source tile
stayed unchanged. The movement guard is in Firestaff's native C path and runs before
creature combat. No authentic active creature was co-located with a pit, so
that conditional interaction remains untested. The full native Firestaff
binary was also rebuilt and launched three times with `--debug
--boot-probe --theron-native jp`; each reached `theron-runtime`, but reported
`frames=0` and `runtimeTick=0`. This demonstrates startup only, not gameplay,
pit semantics, or dungeon rendering parity.

Three current-source Firestaff `--debug` runs on TRV2 passed with authentic JP
Rev. 1 Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`) through the default
`FIRESTAFF_DATA` directory and `--theron-native jp`. Each reached
`phase=theron-runtime` with the level loaded, party `(1,0,0)`, two champions,
291 source objects and four loaded dungeon levels. This also verifies the
null-safe default-data path when no `--data-dir` argument is supplied. The
separate movement regression remains recorded below. The
`test_theron_v1_track02_dungeon_loader` real-media test also passed three
loops and reported pit-record candidates across all seven JP/US dungeons.
These checks establish Firestaff runtime admission and source-record
candidates only; they do not prove original-game movement, pit semantics or
visual parity.

An authentic JP Mednafen state was visibly in dungeon gameplay, but its active
map and party coordinates are not joined to Track 02. The latest isolated
autoload attempt produced raw VDC and RAM snapshots, but had no verified input
delivery, command-consumer receipt, or CD transition. The capture helper
therefore rejected the bundle as incomplete. An earlier host-key attempt
reached the PCE pad interface but did not prove movement. A separate US
cold-start attempt remained on the System Card screen. These are reference
diagnostics only; do not promote them to original-game parity. The pit
consumer remains blocked; see the
[pit evidence gap](../source-lock/theron-disassembly/theron-pit-gate-evidence-gap-20261024.md).

The fresh clean-source Oct. 24 `pce_fast` reference binary contains the
scripted-input hook. A bounded replay from the authentic JP state recorded
one non-System-Card controller poll but no matching replay-plan/event/apply
or game-command-consumer receipt, and the capture helper rejected the bundle.
This provides no evidence of movement. Before interpreting any later
reference capture, require plan, event, apply, and controller-read receipts
to agree. Firestaff gameplay continues to run in the native C/SDL runtime.

Next reference-capture work is to verify a controlled input with same-session
command-consumer, party-pose and active-map receipts, then bind the executing
pit query/caller to its physical bank and authentic Track 02 record. Keep game
media and private emulator artifacts on the operator's machine. Continue
implementing and validating gameplay in Firestaff's native runtime.

## Historical 2026-10-24 — title-action hold question

At that date, the user reported that starting from the original game's menu
required holding the selected action for several seconds. The later 2026-10-31
authentic JP capture confirms four-second Button I holds on NEW GAME and FILE 1
advance through the Akutuba story to the dungeon. This resolves the JP title
action observation. DMWeb identifies Button I as the selector but does not
specify the hold duration. No equivalent verified US title-menu hold capture
is recorded here; do not generalize the JP timing to US or treat it as native
Firestaff timing or gameplay parity.

On 2026-10-25, three additional isolated four-second RUN traces likewise
stopped at System Card BIOS controller reads rather than game-owned input or
Theron title/menu execution. They are retained only as operator-local negative
diagnostics and do not change the later positive JP title-action result.

Reference context: [DMWeb Theron's Quest edition records](../DMWEB_REFERENCE.md)
separate Japanese and US media and include original menu/gameplay references;
[Greatstone SCK](http://greatstone.free.fr/dm/) publishes a CSBWin custom
dungeon named "Theron's Quest Conversion," not retail PC Engine data. Neither
substitutes for an authenticated runtime capture of the menu input.

## 2026-09-25 — JP cold-start capture is System Card-only

The hash-locked authentic JP Rev. 1 cold full-disc capture has a VDC-I/O stream
that exactly replays its captured 64 KiB VRAM snapshot (30,453 VWR commits,
12,544/12,544 words matched). However, visual inspection shows the PC Engine
CD-ROM System Card screen, not Theron game graphics. Its 512 `$2600-$27FF`
reads are zero-valued BIOS `$CB22` reads, with no authenticated CD-to-RAM
receipt and no game transition. The bundle is deliberately not admitted by the
Theron production viewport, and it is not a Theron screenshot. A temporary
admission/CLI test was reverted after this source mismatch was identified.
Keep capture files operator-local; game-owned JP rendering remains blocked.

## Local media boundary

The PCE Fast reference-capture wrapper can optionally record HuC6260 VCE
color-table data writes alongside CD data-port reads, CD RAM target writes,
and CPU-port VWR commits. Set
`THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_TRACE=1` with
`THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast`; the wrapper writes the VCE stream
to `<trace>.pce-fast.vce-write` when color-table writes occur and adds it to
the shared event-order JSONL. Register 4 and 5 records identify the palette
address, old/new 9-bit value and guest instruction address. The chronology
establishes temporal order only; it does not prove causal CD-to-palette data
flow, DMA writes, tile use, or world-cell mapping. A cold Japanese Rev. 1
boot/title-menu capture now provides an authentic VCE write stream with 11,592
ordered records over all 512 palette addresses. The Firestaff event-merger
parser accepts every row. A paired real runtime screenshot still shows
`NEW GAME` selected after a four-second Run hold. The private TRV2 evidence bundle remains outside Git in the Theron evidence
store. This capture used the hash-verified canonical headerless System Card 3.0 ROM
body (`38179df8f4ac870017db21ebcbf53114`) and authentic Japanese Rev. 1 Track
02 (`b7afb338ad31be1025b53f9aff12d73a`), without an autoload state. The full
headered System Card image was unavailable and the optional SBI file was
missing. This does not verify the original firmware container or reach dungeon
gameplay, and therefore does not establish Theron gameplay parity. An older
operator-local
`theron-spawn-search-r26` bundle contains a 1,024-byte VCE endpoint snapshot,
but its receipt records zero non-System-Card CD reads. Treat it as a palette
snapshot only, not a Theron dungeon palette or source-to-tile mapping.

Theron capture uses user-supplied System Card firmware, CUE/BIN and other
original media selected from operator-local storage. None of those payloads
may be committed to GitHub. The repository check is
`scripts/verify_no_original_media_tracked.sh`, and `.gitignore` provides the
same protection for normal local work.

For an exact US Akutuba source-frame comparison, provide the five authenticated
VDC bundle paths through `THERON_VRAM_SNAPSHOT`, `THERON_VCE_SNAPSHOT`,
`THERON_VDC_STATE_SNAPSHOT`, `THERON_VDC_SAT_SNAPSHOT`, and
`THERON_VDC_IO_TRACE`. Set `THERON_VRAM_CAPTURE_BMP` to Firestaff's temporary
output path and `THERON_EXPECTED_SOURCE_BMP` to the corresponding
operator-local original-Mednafen BMP. The optional real-capture test compares
the two BMP files byte-for-byte. The verified US Akutuba frame currently
matches exactly; neither that screen-space receipt nor its local reference
promotes dungeon/gameplay semantics or a public screenshot.

**Update 2026-08-09:** an authenticated VDC/VCE pair now reaches the
production Theron boot presenter. M11 installs the captured VCE palette before
presenting the indexed frame, and the boot facade admits this source-only
screen replay without unlocking generated tiles or unproven square-to-tile
semantics. The resulting frame remains diagnostic evidence: it is not yet a
README-eligible gameplay screenshot.

**Historical capture (2026-08-12).** The fresh authentic US CUE/ISO capture used
the real System Card 3.0 and the instrumented real-SDL2 Mednafen build. It
observed 161 raw sectors, 51 SCSI read commands, 25 CD IRQ callbacks, two
authenticated CD-to-RAM receipts and 32 game-main-RAM `$E009` dispatches. It
still observed zero game-owned `$E009` data reads. This is a useful bounded
result: the transport is real, while the dungeon handoff is not yet proven.

**Fresh local replay, 2026-08-13:** using the same hash-verified US Track 02
and an operator-supplied System Card 3.0, a private replay
produced 161 raw sectors, 51 SCSI read commands, 25 CD IRQ callbacks, 161
sector bindings, 47 byte-exact FIFO-to-RAM origin receipts, 4,096
source-window spawn-consumer reads, 4,608 raw RNG-window samples, and 65,536
VDC I/O writes. `scripts/verify_theron_origin_ram_receipt.pl` passed for all
47 receipts. The spawn sidecar reaches `$CC4C`, `$4644` and `$4667`, but not a
valid `$B0E5` category; the RNG sidecar is not a `$5D64/$5D6A` source join.
The replay therefore remains transport/provenance evidence only.
The raw output remains outside GitHub in private operator storage.

### Historical runtime/status checkpoint (2026-08-13; superseded)

The external-`TMPDIR` Theron regression is green: 253 selected CTest cases
passed (247 executed, six expected capture skips). With the local authenticated
fixtures, the VRAM/VCE readiness, Main-RAM consumer, and CD-state sidecar tests
also pass. These are transport/readiness gates only; they do not promote the
missing game-owned dungeon, spawn, or RNG consumers.

An additional authenticated GUI capture now proves the real BIOS Run-to-CD
Track 02 startup handoff. Its original emulator image is kept local and is
not a public Firestaff screenshot. Its complete hashes and narrow evidence
boundary are recorded in
[`docs/source-lock/theron-authentic-track02-handoff-2026-08-08.md`](../source-lock/theron-authentic-track02-handoff-2026-08-08.md).

The same real US CD route also has a local original-emulator dungeon frame;
it is not part of the public repository or README. Firestaff's Theron
runtime uses ordinary desktop controls: arrow keys or W/A/S/D for movement,
mouse button 1/2 for Button I/II, and a free mouse pointer that never hops
between objects.

README-eligible screenshots must be rendered by Firestaff itself.

At that 2026-08-13 checkpoint, the Firestaff runtime had no
`README_ELIGIBLE` Theron row. The status at that time was:
Firestaff can hash-verify JP/US Track 02 data, lock
raw bank-anchor offsets, read the authenticated US 53-entry descriptor table
from its real MODE1 user-data offset, and reach
the M11 Theron runtime, but per-entry semantic Track 02 dungeon-table binding,
broader loader parity, runtime playability proof, and reviewed screenshot
promotion are still active work.

The 2026-08-13 external-disk cold-start capture added a bounded raw window
receipt:
the real US route reached seven game-main-RAM `$E009` dispatches, nine
`$4644` preconsumer entries and 32 `$4667` helper entries after 40 raw-sector
spans. This is positive CD→RAM→HuC6280 runtime evidence. It did not reach
`$B0E5`, a dynamic `$C96B/$CC4C` return or a source-owned T700/T900 consumer,
so it does not unlock RNG, AI, attack, damage, loot, generator timing or
statistic semantics. The full boundary is recorded in
[`docs/source-lock/theron-disassembly/theron-runtime-spawn-capture.md`](../source-lock/theron-disassembly/theron-runtime-spawn-capture.md).

The loader sidecar importer accepts the literal `\\n` record separators found
in the early local Mednafen artifact. Normalization is parser-only: the source
file is still hashed as-is, and the receipt remains limited to loader
coordinates rather than semantic publication.

The same normalization is applied to the Main-RAM consumer trace and its
code-window verifier, so a transport-escaped sidecar cannot be accepted by
the loader stage but rejected later by the consumer stage.

The 2026-08-13 authenticated replay sidecar was parser-ready but remained
initialisation-only: its `$2600-$27FF` window contains 512 `$CB22` reads, all
zero-valued, with no `$C3A0` reader. Run that sidecar through the parser-only
gate with `THERON_MEDNAFEN_MAIN_RAM_CONSUMER_PARSE_ONLY=1`; the code-window
assertion remains a separate gate because this replay did not execute the
`$2c54-$2c69` window. This prevents a transport receipt from being mistaken
for a dynamic level/object consumer.

An operator-supplied raw VDC/VCE bundle can be replayed by the explicit
diagnostic capture tool when its exact receipts are present. This is a
screen-space snapshot replay path; it is not the production dungeon viewport
and does not prove a source-bound map/pose-to-render chain, square-to-tile
perspective, creature ownership, or HUD layout. The production Theron world
viewport remains fail-closed pending that evidence.

Tests and captures recorded at this historical checkpoint:

- `tier1_strict_boot_probe`: JP canonical, JP extras, and US extras reach the
  `TQR level load` boot milestone when those data paths are present.
- `theron_v1_m11_direct_launch`: M11 consumes the hash-verified Track 02 path
  without re-walking the data root and builds the bounded Theron runtime state;
  it does not establish a source-bound rendered dungeon viewport.
- `theron_v1_raw_bin_runtime_boot`: a configured user-owned `TQUS02.bin`
  layout is launched through the actual CLI, enters the source-backed Theron
  runtime, loads the first level and forms the initial party. It is a
  real-media test and SKIPs on hosts without that copyrighted file; it does
  not assert unproven later rendering, combat or dungeon semantics.
- `theron_v1_viewport_renderer` and `theron_v1_rendering`: data-free viewport,
  palette, UI-chrome, and M11 blit contracts are covered. These contract tests
  do not establish the production world renderer or authentic visual parity.
- `theron_v1_cross_route_mechanics`: synthetic runtime mechanics now cover a
  bounded movement/interact route across explicit door opening, pools, alarms, triggers,
  teleporters, pits, post-move drain, and click-route TAKE.
- `theron_v1_runtime_screenshot_readiness`: when real Track 02 data is
  present, the verifier launches Firestaff under dummy video and records M11
  runtime-probe JSON plus source/presented BMP hash receipts. Rows that report
  deterministic fallback assets are capture-path proof, not final art or
  source-bank proof. Without an explicit authenticated VDC/VCE pair the gate
  is a successful SKIP; a black fail-closed viewport is never accepted as a
  screenshot. To inspect the diagnostic screen-space replay, supply an
  authenticated VDC/VCE pair with --vram-snapshot and --vce-snapshot. This
  does not prove a production world-render route. Missing data is also
  a successful SKIP, not a failure.
- `theron_v1_track02_bank` and `theron_v1_track02_descriptor_table`: when
  real Track 02 data is present, both probes hash-gate the bank-anchor
  offsets and the 9-word little-endian stride table shape on the US ISO
  (`0x1584`) plus all three US raw BIN anchors and all three JP raw BIN
  anchors. The probes discover configured operator-supplied `TQUS02.bin` and
  `TQJP02.bin` files, while environment overrides remain available. The
  descriptor-table decoder is
  shape-driven only: it validates
  the 9-word stride sequence (entries 0x0020..0x2020, stride 0x0400) without
  claiming per-entry semantic type, dungeon-level binding, or loader
  handoff.
- `theron_v1_track02_dungeon_loader`: authenticated dungeon-text codons are
  copied into the live world as raw source words during a real US/JP dungeon
  load. This is a lossless handoff for the future HuC6280 consumer;
  `world_dungeon_text()` still publishes zero strings while control-code
  ownership and the destination renderer remain unresolved.
- `theron_v1_mednafen_vdc_io_trace`: the optional Mednafen VDC-port sidecar
  now has a strict receipt parser for its header, contiguous sequence,
  monotonic timestamp, port/register bounds and HuC6280 writer coordinates.
  The live transition receipt also records `vdc_io_writes` and requires a
  positive write count, so a header-only sidecar cannot be admitted as a
  complete transport capture. It proves only an original CPU-to-VDC write
  witness; it does not join source bytes to VRAM or open text, BAT, square, HUD
  or object semantics.
- `theron_v1_level_descriptor`: reads all 53 six-byte descriptor records from
  authenticated US Track 02 user data at UD `0x619900` and rejects a bytewise
  mismatch against the source-locked table. This is a real-data receipt, not
  a decoder for the referenced graphics blocks and not proof of dungeon-level,
  object, tile-bank or palette semantics.
- `theron_v1_runtime_screenshot_promotion_gate`: a bounded provenance gate
  that audits every row of the readiness manifest against an explicit
  README-eligibility contract (real Firestaff runtime capture, no
  deterministic fallback assets, `TQR level load` boot marker, semantic
  Track 02 loader evidence in the probe, unique source BMP sha256 per row,
  valid 320x200 presented BMP) and locks the non-promotion state until at
  least one row is `README_ELIGIBLE` AND a human reviewer promotes it from
  tracked evidence.
- `theron_v1_runtime_screenshot_promotion_checklist`: a bounded companion
  to the promotion gate that surfaces the explicit per-row review
  checklist a human reviewer must work through before any single row may
  be promoted into `verification-screens/` or `docs/compare/`, and folds
  an optional operator-local reviewer sign-off file into the audit. The
  checklist reuses the upstream promotion-gate manifest as a contract
  source-of-truth, so a contract drift in the gate fails this gate as
  well, and locks the non-promotion state until BOTH the machine
  contract AND an explicit reviewer sign-off for at least one row are
  present. Default verdict today is `REVIEW_CHECKLIST_NO_ROW_PROMOTED`:
  every readiness row still uses deterministic fallback assets and shows
  no semantic Track 02 loader evidence, so no Theron capture may yet be
  promoted into the README even if a reviewer has signed off.
- `theron_24h_readiness`: the daily readiness rollup ties the Track 02 bank,
  save/load, cross-route mechanics, runtime screenshot, dungeon progression,
  cross-slot, and M11 launch gates into one PASS/FAIL manifest so later
  screenshot promotion can start from a known-good baseline.
- `theron_v1_srm_classifier` (probe + unit test, 2026-06-25): bounded
  Save Disk manifest that detects the gzipped-deflate body dmweb's
  `community/documentation/miscellaneous.html` and greatstone page
  reference for Theron's Quest savegames, reports a clean
  `present_count=0, recognized_count=0` ABSENT manifest on the current
  host (no real `.srm` file is staged), and accepts a real `.srm`
  when one is supplied through `FIRESTAFF_THERON_SRM_DIR`. The default
  configuration uses five disk slots matching the original Save Disk
  cartridge model. The
  classifier is the bounded real-artifact counterpart to the
  synthetic `theron_v1_save_load.c` `slotN.tqsv` in-game save
  format; the two are kept separate because the underlying save
  models are different (Save Disk cartridge vs. in-game save
  format). See `include/theron_v1_srm_classifier.h` for the
  classifier contract and `docs/DMWEB_REFERENCE.md` §6 for the
  format anchor. The bounded gzip probe can inflate synthetic
  recognized payloads, and the Firestaff-only readiness envelopes can
  restore `Theron_DungeonProgression` plus party/champion body fields
  into `Theron_V1_Party`. Real Sphenx/TQ-RTC body layout decode,
  inventory/equipment import, real `.srm` promotion, and launchable
  save conversion remain out of scope and are tracked under
  `docs/FIRESTAFF_GAP_LIST.md` A3 `Savegame format (Theron .SRM)`.

These are Firestaff readiness receipts. They govern Firestaff-rendered
screenshots, not the separately labelled original-media capture documented
above. Do not add a Firestaff Theron image to the README until the
`theron_v1_runtime_screenshot_promotion_gate` classifies at least one
readiness row as `README_ELIGIBLE` AND a human reviewer promotes that specific
row from tracked evidence into `verification-screens/`. Do not use generated,
illustrated, or mock Theron images as screenshots. A green readiness row proves
the launch/capture path is alive; it does not prove semantic Track 02
dungeon-table parity, real `.srm` interchange, broader playability, or that
the captured frame is README-eligible.

The promotion gate remains the single source of truth for whether a Firestaff
runtime Theron screenshot is eligible to be promoted into public docs. Today it reports
`NO_README_PROMOTION_PERMITTED`: every readiness row still uses
deterministic fallback assets and shows no semantic Track 02 loader
evidence, so no Theron capture may yet be promoted into the README even
if the runtime-screenshot gate is green. The original-media image is labelled
separately and does not change this Firestaff-runtime verdict.

Run the focused readiness check with:

```bash
ctest --test-dir build -R '^theron_v1_runtime_screenshot_readiness$' --output-on-failure
ctest --test-dir build -R '^theron_v1_track02_' --output-on-failure
ctest --test-dir build -R '^theron_v1_srm_classifier' --output-on-failure
```

Run the promotion-provenance check with:

```bash
ctest --test-dir build -R '^theron_v1_runtime_screenshot_promotion_gate$' --output-on-failure
ctest --test-dir build -R '^theron_v1_runtime_screenshot_promotion_checklist$' --output-on-failure
```

The generated reports live at:

- `parity-evidence/theron_v1_runtime_screenshot_readiness.md` — geometry
  and hashes only, screenshot bytes operator-local until promotion.
- `parity-evidence/theron_v1_runtime_screenshot_promotion_gate.md` — row
  classification against the eligibility contract and the current
  `NO_README_PROMOTION_PERMITTED` lock.
- `parity-evidence/theron_v1_runtime_screenshot_promotion_checklist.md` —
  per-row reviewer checklist with machine + reviewer sign-off verdicts
  and the current `REVIEW_CHECKLIST_NO_ROW_PROMOTED` lock.
