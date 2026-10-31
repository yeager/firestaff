# Firestaff TODO — Theron's Quest

## Next original-game capture — bind the generated row to `$C852`

The authenticated Drator event receipt follows a generated ten-byte row
through construction, runtime-slot publication, first reads and unlink. Static
US Track 02 code at raw offset `$A1612` / logical `$C852` copies row offsets
6–8 into working fields, but the capture corpus has not shown that consumer
reading this exact row in the same event. The reference capture wrapper now
supports a bounded opcode-fetch trace at logical `$C852`; enable it with
`THERON_CAPTURE_PCE_FAST_C852_FETCH_TRACE=1`. It writes
`${THERON_LIVE_TRACE_OUTPUT}.c852-fetch`, capped at 4,096 hits, with the fetched
opcode, physical PC, all MPRs, registers and the same shared event sequence as
the row-lifecycle records. Correlate it with the existing main-RAM
consumer-read trace, which records reader PC/MPR and row data reads. Its
`FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_RANGE` option filters physical
data-read addresses; it is not a code-PC filter. A logical-PC hit in another
physical bank does not close the join. Keep row bytes opaque; do not label
them as coordinates, direction or creature fields.
This reference capture is evidence-enabling work; the native viewport still
requires a separate same-run map, pose, VDC/VCE/VRAM and rendered-frame join.
Do not reuse an occupied capture profile or evidence directory; coordinate
with its owner before using shared capture infrastructure. See the
[source-bound generator/position report](docs/source-lock/theron-generator-rng-state-2026-08-21.md)
and [current capture readiness](docs/theron/THERON_CAPTURE_READINESS.md).

## 2026-10-31 — corrected TII stack bytes still do not identify the caller

The former `guest_stack` trace read logical `$01xx` through MPR0 even though
PCE Fast stack operations use the MPR1-backed `HU_Page1`. The reference
instrumenter now snapshots the stack page through MPR1. Three repeated
authentic JP F5 state-only captures match the same trace hash and stack bytes,
but no unique source-mapped JSR return address is established. Do not use the
old `4000000040000000` value or the corrected bytes to assign gameplay purpose
to `$C1FC`. Continue with a source-mapped caller/control-flow capture; keep the
player-movement versus teleporter question open. These runs still report
`transition=missing` and provide no cold CD-to-RAM or active-map proof. See the
[caller-stack correction](docs/source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md#caller-stack-attribution-correction-2026-10-31).

## 2026-10-31 — join authentic world resources to the live VDC scene

The production viewport still has no source-derived cell-to-screen renderer.
The available native Firestaff US capture is an admitted, nonblack snapshot,
but its map/pose values only resemble candidate bytes in a separate
save-state replay; this is not a same-run world join. Other usable VRAM/VCE
captures are save-state snapshots with `transition=missing`, while the cold
loader traces either stop before the original game handoff or lack a paired
atomic VDC snapshot. Keep the production viewport fail-closed until one cold
or authenticated progressed route records, in the same execution: the exact
JP/US Track 02 identity and source-read bytes; their source-owned decompressor
and destination RAM span; the active dungeon/level descriptor and party pose;
the subsequent game-owned bitmap/tile transfer into VRAM/BAT, VCE palette and
VDC state at an atomic frame boundary; and a retail screenshot for comparison.
Then derive the square/material, perspective, HUD, text and object ownership
from those linked bytes and verify movement plus a no-input control across
repeated runs. Existing allowlisted screen captures authorize screen-space
replay only. Do not map a cell to an attractive tile by visual resemblance or
promote a save-state input trace as cold media provenance. See the
[source-to-RAM cold-loader boundary](docs/source-lock/theron-disassembly/theron-jp-cold-loader-byte-copy-capture-20261029.md),
[atomic VDC capture boundary](docs/source-lock/theron-disassembly/theron-runtime-spawn-capture.md#2026-08-13-ramvdc-replays-vdcvce-pair-is-screen-space-admitted),
[US authentic VDC comparison](docs/source-lock/theron-us-authentic-vdc-turn-pair-2026-10-25.md),
and [current capture readiness](docs/theron/THERON_CAPTURE_READINESS.md).

## 2026-10-31 — documentation follow-up

The current status pages now identify the repeated `$C1FC` instruction path
and its possible teleporter-destination role. Keep all future movement claims
blocked until the caller data and active map are source-bound, then test an
authentic reachable pose and join state change to reference output. The
current documentation index is [`docs/DOCUMENTATION_INDEX.md`](docs/DOCUMENTATION_INDEX.md);
historical gap inventories must not be treated as the active queue.

## 2026-10-31 — identify the active map and candidate position semantics

Three repeated PCE Fast state-only runs from the authentic JP Akutuba F5 state
record Button II+UP guest reads followed by `$C1FC` TII transfers from
`$20B4` to candidate bytes `$2040/$2041`; three matched Button II-only
controls retain the previous values. This is repeatable input-conditioned
state transfer, not yet a confirmed coordinate update or successful movement.
The same `$C1FC` instruction is also associated with the Track 02 teleporter
destination path. A follow-up three-run trace now records an identical bounded
instruction path through `$C1D6` (`JSR $C14E`) and return, then conditional
fall-through into `$C1FC`. That path still does not source-bind the tested RAM
fields, identify an active-map object, or establish whether this is player
movement or teleporter resolution. Next bind the fields and routine to the
active map, then establish passability/collision behavior and an original
rendered response. All state-only captures lack a cold CD handoff; variable
VRAM snapshots do not establish visible response. See the [comparison evidence](parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md),
[dynamic instruction-context capture](docs/source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md),
and [source-locked teleporter analysis](docs/source-lock/theron-disassembly/theron-runtime-spawn-capture.md#2026-08-09-authenticated-track-02-teleporter-handoff-correction).

## 2026-10-31 — input-gated PCE Fast VRAM event-order trace captured

The complete reference-only Mednafen instrumentation patch chain applied to
an isolated source copy, and the corrected instrumented PCE Fast binary built
and installed on TRV2. The authentic Japanese dungeon state records scripted
UP at guest PC `$44C1` with port `raw=0010`, while the controller device value
is `device_raw=0000`. After correcting the callback to pass
`pce_jp_data[index]`, an isolated state-only capture validated 4,096
contiguous VRAM commit rows. The first row is shared event sequence `58761`,
after the matching guest read at `58758`. The capture wrapper explicitly
reports `transition=missing`; no cold-start CD handoff is claimed. This is an
event-order receipt, not proof that UP caused a specific VRAM write, a visible
response, decoded graphics meaning, or native parity. The source-locked report
records artifact hashes and scope. Native Theron support and parity status
are unchanged. See the [capture-readiness record](docs/theron/THERON_CAPTURE_READINESS.md).

## 2026-10-31 — next JP movement capture needs a reachable authentic pose

The earlier F5 UP/neutral/DOWN replay keeps the same front wall in all three
screenshots and records no candidate-position transfer. Its `$44C1` to
`$2912` event ordering is not proof of causality or movement. A later matched
Button II+UP comparison does record repeatable TII transfers; the map, byte
semantics, collision outcome and rendered response are still unknown. Do not
infer a coordinate or passability model until those are source-bound. See the
[comparison](parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md)
and historical [blocked-wall report](docs/source-lock/theron-disassembly/theron-jp-up-input-latch-blocked-wall-probe-20261031.md).

## 2026-10-31 — direct launch bypasses Firestaff's Run menu; dungeon proof remains separate

A native macOS Debug launch with authentic US Track 02 enters Theron's
title/startup path without showing Firestaff's Run menu or booting a System
Card. Its direct-launch boot probe reports `levelLoaded=0`, so direct entry to
a loaded dungeon is not established by this check. The independently verified
JP fresh-start and US Continue routes remain the loaded-dungeon evidence. Do
not add a synthetic starting party or pose to shorten this path. See the
[source-locked launch note](docs/source-lock/theron-native-debug-direct-launch-20261031.md).

## 2026-10-31 — US saved-state scripted-input trace remains inconclusive

An existing US state file loaded successfully in an instrumented PCE Fast
reference run with the original US CUE. The run's shared trace file contains
1,354 short-format input-read rows at `$44CA/$44DB`, all with `raw=0000`, while
the separate input script log records Right/Down/Button I mask application.
The standard-PCE and PCE Fast instrumenters share the same trace path and row
label, so these rows are not a verified PCE Fast read receipt. The different
close-time RAM/VRAM/SAT snapshots are also not attributed to an action, pose,
map cell, or display consumer. Source ownership, input consumption, and a
renderer mapping remain unproven. Separate the trace streams and join a
source-backed state change to same-session VDC/VRAM writes before using this
route for parity. See the [capture report](docs/source-lock/theron-disassembly/theron-us-state-scripted-input-boundary-20261031.md).

## 2026-10-31 — cold US scripted inputs reach BIOS polls; handoff remains unjoined

The authentic US cold standard-PCE run loaded no save state and configured no
profile SRAM. Its trace records one dispatch to `$E009` from logical PC
`$2CC7` (physical `$1F0CC7`). The initial capture had a capped input trace.
The follow-up corrected the returned-value verifier and validates all three
scripted Run/Select/Button I inputs against System Card controller reads. All reads remain at BIOS PC
`$E4C8`; no non-System-Card poll, CD data-port read, or authenticated
CD-to-RAM join was observed. Treat `$E009` only as an unjoined dispatch
observation: it does not prove game ownership, title-menu action, or gameplay.
Next capture a game-owned poll and join its transition to authenticated Track 02
source data. See the [returned-mask capture report](docs/source-lock/theron-disassembly/theron-us-cold-scripted-input-returned-mask-20261031.md).

## 2026-10-31 — cold System Card RUN still has no game-owned handoff

The authentic Japanese `run@480:300` reference capture binds `$1800=$C8`,
`$1801=$04`, the callback store `$2259=$04`, and BIOS continuation
`$E166->$E176`. It records no `SELECT` event and no `$E174`, `$E11D`,
`$E130`, or `$E209` receipt. Do not infer `$2259` semantics, a title-menu
action, or game launch from this firmware path. Continue tracing until a
source-authenticated transfer reaches game-owned code and input polling; then
compare that path against native Firestaff. Evidence and bounded disassembly:
[`System Card RUN trace`](docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md#2026-10-31-r5-callback-and-transfer-boundary),
[`$EA3B` listing](docs/source-lock/theron-disassembly/theron-system-card3-ea3b-20261031.asm).

## 2026-10-31 — reference trace build passes; original-game capture remains open

The hook is now anchored to the unique instruction-opcode provenance callback,
and the RAM read provenance hunk matches the opcode-fetch guard. Three fresh
TRV2 patch-only runs applied the full Mednafen 1.32.1 trace sequence; the full
instrumented PCE build and install succeeded. The controller-patch dry-run and
compiled mouse parser regression each passed three loops. No new authentic
gameplay capture has been collected. Next capture a bounded cold/reference run
that reaches game-owned input and state consumers, then join those observations
to native behavior. Do not infer launch, action semantics, or parity from a
successful instrumented build alone.

## Remaining end-to-end work, ordered by dependency

The dated entries below retain current capture results and limits. This short
plan is the canonical completion order; historical capture notes were trimmed
from this file, while their source-lock files remain in the repository.
Use authentic original media whenever available. Synthetic traces may test
parsers or help derive authentic evidence, but must not stand in for game
assets, mechanics, or parity.

2026-10-31 native startup and item lifecycle loop:
`theron_v1_m11_real_bram_continue` now verifies a fresh US campaign through
Ak-Tu-Ba, the authentic roster/Soul Room, and source-verified level 0. Its
Continue branch separately routes to dungeon 2 and exercises movement, turning,
pickup, and drop through the M11 input API. Three TRV2 loops verify complete
source-record/property preservation and release of the inventory slot; quest
progress remains unchanged. This proves real-data native host paths, not retail
parity. Quest artifact identity, retrieval messaging, dungeon exit, and
campaign completion still require the original T900 consumer. Do not map the
parallel quest-name type indices to quest bits: authentic US/JP tables contain
repeated matching types. See `tests/test_theron_v1_track02_quest_item_names.c`
and `docs/source-lock/theron_t900_proof_2026-08-08.md`.
The pass 215/223 binary-analysis notes describe the retrieved-item strings as
text evidence only; runtime object identity and quest-state effects remain
open.

The JP counterpart, `theron_v1_m11_jp_fresh_start`, now covers the authentic
title-to-level-0 path independently. It compares all eight decoded roster
names, confirms Theron is admitted, and requires a source-verified level header
with zero quest progress. The warning-as-error build and three repeated CTest
runs passed against the hash-locked original JP BIN. This does not prove JP
Continue/save semantics, retail input parity, or full gameplay parity.

2026-10-31 save-resume follow-up: the default Continue root now uses the same
`FSP_GetUserDataDir()/saves/theron` directory as Save Game, including when
`XDG_DATA_HOME` is set. Same-path updates now require a Firestaff-written
`.firestaff-managed` sidecar bound to both the exact BRAM bytes and save path;
the writer rechecks the marker against the bytes it is about to replace.
Unmarked or stale-marker original BRAMs are rejected without replacement.
Distinct-source saves retain the alias guard and move the in-session template
to the verified managed output after a successful save.
`theron_v1_m11_real_bram_continue` passed locally against authentic US Track 02
and Ak-Tu-Ba-complete BRAM data with XDG data isolated outside the asset root.
It verifies repeated Continue/save and that an unmarked authentic BRAM at the
canonical destination remains byte-identical after a rejected update. This
validates host persistence and startup lookup, not retail hardware or PCE SRAM
parity.

2026-10-30 v11 indexed-dispatch follow trace: authentic JP Rev. 1/Track 02 and
System Card 3.0 produced 40 dispatch windows and 10,030 runtime fetch rows at
the 256-instruction cap. Twelve windows ended at the next `$40E1` dispatch
after 235-248 fetched instructions; 28 hit the cap. In the first window the
actual fetches enter System Card controller polling at `$E4A5-$E4DE`, read the
single scripted event at `$E4C5`, and return to the indexed dispatcher. The
next observed targets vary, and this does not prove game ownership. Direct-
dispatch, follow, and source-chain verifiers each passed three loops. Five
source-chain joins bind the initial dispatched table and target opcode bytes
to authentic Track 02 reads and latest observed `$EA9C` writes. Thirty-nine
dispatches remain outside the bounded `$6148` code-follow. There were no
non-System-Card polls, CD IRQ callbacks, authenticated aggregate CD-to-RAM
receipts, or game main-RAM dispatches; the 45-second capture ended with
`transition=missing`. Continue through complete guest writers and
source/consumer chains to a game-owned poll and resulting state change. v9's
independent `$EA9C` writer comparison remains historical evidence. Results and
hashes are recorded in the
[JP System Card Run source-lock note](docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md).

2026-10-30 v12 RAM-loop source-candidate analysis: a fresh cold-reference
trace's all-PC target-write observer now covers every physical loop address
from `$1000D2` through `$1000E3`. The bounded fetch trace contains 165 snapshots
and covers all 18 bytes. Across 495 observed byte slots, all match the latest
address-scoped writer record. Ninety-five slots use non-`$EA9C` writes; 400 match an
`$EA9C` write and have a source-valid `$EA99` read candidate within 16 events.
Those 400 slots have 390 unique and 10 ambiguous read candidates; 165 fetch
rows match their address-scoped latest writes, and 111 rows are fully joined
to CD-read candidates. The expanded target-write verifier reports 2–6 writes
per loop address and 40/40 fetched bytes in `$1000D9-$1000DB` backed by a
matching prior write, with no mismatches or missing writes. Address `$1000E3`
had two observed writes, both from `$EA9C`; this remains a bounded observation,
not an exclusive provenance census.
In the same v11 window, code at `$6109` executes `LDA #$EA`, then stores `$EA`
to `$40D9-$40DB` from `$610B/$610E/$6111` before returning to `$432B`. All 12
executed helper bytes match earlier `$EA9C` writes and have source-valid CD-read
candidates; eight byte slots have one matching read and four have multiple
same-value candidates. The all-PC trace confirms all three stores. The `$4328`
indirect jump and return are observed, but caller ownership remains unproven.
Keep following the 18-byte loop through complete writers and its caller until
a game-owned consumer and input/state transition are demonstrated. The offline
candidate verifier repeats these exact counts in three TRV2 runs; details and
trace hashes are in the source-lock note.

2026-10-30 v13 extended cold-run trace: the same `run@480:300` input (RUN mask
`0008`, held for 300 frames) was consumed by the System Card at `$E4C5`, but a
180-second capture still recorded no non-System-Card poll, CD IRQ, authenticated
CD-to-RAM receipt, game `$E009` dispatch, or gameplay transition. Its 63,488
source-bound CD data-port reads were byte-identical to v12 despite the longer
capture. The System Card sidecar records `$C860` once and `$C8C7/$C8C9/$C8D2`
after the input, plus 207 `$E4E1` polls, but no `$C897` row or instruction-level
`$C8C4` CD-status wait witness; the dedicated wait verifier therefore remains
unsatisfied. Continue disassembling the post-poll branch and its CD status
checks; do not infer game startup from input consumption.
Details and trace hashes are in the source-lock note.

2026-10-30 bootstrap disassembly: the authentic System Card 3.0 callback
vector resolves to `$E0F3`. The bounded listing shows the `$E109` loop calls
`$C860`, then `$F0A9` and `$E14C`; `$E14C`'s carry selects the `$4000` path
after MPR6 setup or the `$CB03/$CB54` path that retries `$C860`. The bytes
alone do not identify either path's higher-level meaning, and the `$4000`
payload remains unidentified. Obtain a source-bound v14 trace for the return
value of `$C860`, the carry/result data at `$E11B`, and mapped bytes/control
flow at `$4000` before assigning a BIOS launch interpretation. See the bounded
excerpt `docs/source-lock/theron-disassembly/theron-system-card3-bootstrap-e0f3.asm`.

2026-10-30 v14 source-guided input trace: the authentic System Card 3.0
disassembly at `$E4E1-$E4FB` dispatches only when `delta=$04` and state=`$0C`.
The cold replay held RUN (`$08`) for 300 frames, then pressed SELECT (`$04`)
for one frame while RUN remained held. The trace observed state=`$0C`, delta=`$04`
at `$E4FB` and vector=`$E000`, then returned to BIOS bootstrap `$E0F3` and
re-entered `$E109/$C860`. This confirms the System Card action branch, not a
game launch: there were still no game-owned polls, CD IRQs, authenticated
CD-to-RAM receipts, `$E009` dispatches, or gameplay transition. The next
disassembly target is the `$E000` callback/vector and its bootstrap's CD launch
condition. The bounded RAM-loop verifier also passes on v14: all 234 fetched
byte slots across 78 rows match the latest writes for all 18 addresses, with
56 rows fully joined to source-read candidates. See the source-lock note for
trace hashes and limits.

2026-10-30 v15 bootstrap windows: an authentic cold Japanese Rev. 1 capture
now verifies 512 instruction rows starting at `$E11B` and 128 independently
armed rows starting at `$4000`, with every logical/physical mapping validated.
The scripted RUN/SELECT events were consumed at System Card poll `$E4C5`, but
there were still no non-System-Card polls, CD IRQs, authenticated CD-to-RAM
receipts, or gameplay transition. The `$4000` payload remains unbound to
source bytes and its ownership is unknown. Do not treat instruction execution
or a passing trace verifier as proof of a game handoff. Continue by joining
these mapped fetches to complete source-authenticated writes and a game-owned
consumer. Details and the trace hash are in the source-lock note.

2026-10-30 v16 instruction-start snapshots at `$4002/$4009`: the captured
bytes and the initial 11-byte authentic JP Track 02 span were source-correlated,
but the v16 CD sidecar headers do not identify the shared event-sequence
source. The old verifier's cross-file event join is therefore not sufficient
evidence of common counter provenance. Its hook also samples mapped bytes
before the CPU opcode read, so describe these as instruction-start snapshots,
not CPU fetches. A revised verifier now requires shared-sequence header
markers and immediate read-to-write adjacency; an authentic capture with
those markers remains to be collected. The original run had no CD IRQ,
non-System-Card poll, or gameplay transition. Continue tracing toward a
game-owned poll and visible state change.

2026-10-31 authentic v22 run-only callback trace and BIOS disassembly: the
hash-verified Japanese Rev. 1 / System Card 3.0 replay consumed its single RUN
event at `$E4C5`. Its 8,192-row bootstrap window reached `$EC05` once and
`$E900` three times, then remained in the `$E95E/$E966` status loop until the
window cap. It records no observed instruction start at `$E209` or `$E11D`; the
independent `$4000` window is not evidence of the `$E130` launch path. The bootstrap trace
hash matches v21, while this capture's status and callback-state hashes are
recorded in the source-lock note. Three repeated MAME `unidasm` listings from
the authentic System Card BIOS now cover `$EC05`, `$E900`, and `$E1C0-$E20A`.
They identify command setup, `$1800-$1802` accesses, status comparison, and
the `$E209 SEC; RTS` table-check exit without assigning undocumented device
semantics. An opt-in bounded decision-PC sidecar can record `$EC05`, `$E900`,
`$E209/$E20A`, and `$E11D` independently of the bootstrap row cap. The 2026-10-31 r5 cold capture used both expanded observers. It verified
453,615 I/O/RAM rows across all 29 configured addresses with authentic BIOS
opcode-byte mapping, 47 decision rows across six PCs, 65,536 status rows, and
251 callback-state rows. The RUN event was consumed only by the System Card;
no `$E11D` launch branch, CD IRQ, or game-owned controller poll was observed.
The expanded receipts document BIOS activity only and do not prove game launch
or parity. The retained trace hashes and exact capture inputs are recorded in
`DONE-theron.md` and the ignored `.codex-scratch/theron-syscard-io-ram-cold-20261031/`
artifacts on the external disk.
Neither the disassembly nor the independent `$4000` window proves a game
launch or parity. Evidence and limits are in the [System Card command/status
source-lock note](docs/source-lock/theron-disassembly/theron-system-card3-ec05-e209-20261031.asm).

The v22 bootstrap rows also show the `$E14C` branch sequence: with `$2259=$00`,
`$E166` falls through to the `$226A/$226B` counter comparison; later `$EA50`
writes `$04` to `$2259`, and the next `$E166` takes `$E176`. The replay had no
scheduled SELECT event, so `$2259=$04` alone must not be labeled as SELECT
evidence. The capture did not reach `$E174`, `$E11D`, `$E209`, or `$E130` before
the row cap. The branch and value provenance are documented in the [JP System
Card Run source-lock note](docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md).

The r5 RUN-only trace now joins the branch byte to its preceding BIOS I/O:
`$EA3B` read `$1800=$C8` (event 10510), `$EA4D` read `$1801=$04` (event
10517), and `$EA50` copied `$04` to `$2259` (event 10519). `$E166` later
branched to `$E176`; no `$E174/$E11D` clear-carry launch was observed. Keep
following the authentic CD command/status path and carry checks until the BIOS
handoff reaches a source-bound game-owned consumer and gameplay transition.
Receipts and hashes are in the [JP System Card Run source-lock
note](docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md).

The bounded `$1800-$1803` I/O and `$224C-$2255`, `$2273-$2280`, `$22A4`
memory observer has now been used in the r5 capture. Its offline verifier
requires the authenticated System Card image and checks each event's access
kind, value, PC mapping, and opcode bytes against that BIOS. Continue with
source-guided tracing around `$EA79/$EA7E` and `$E95E/$E963`, then target the
`$E209/$E11D` carry path in a replay that reaches it.

A current-source native Debug run with authentic Japanese Rev. 1 Track 02
reached the dungeon runtime and recorded a left turn followed by a forward
move. Starting-state probes also recorded blocked forward and right-strafe
actions. These are Firestaff state receipts only; bind equivalent actions to
an authentic original-game state before claiming movement parity. The run used
a dummy SDL display and produced no rendered-game frame.

1. **Bind the cold loader to game-owned consumers in both regions.** Continue
   from the authentic System Card Run path and the verified Track 02 byte
   writes through the full loader, game-owned RAM, executing consumer, source
   record, and a reproducible input/state transaction. A System Card poll,
   `$E009` dispatch, or partial byte join alone does not establish a game
   handoff or playable runtime. Current trace limits are recorded in the
   [JP System Card Run source-lock note](docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md)
   and the dated trace entries below. The bounded v11 trace follows 40 indexed
   dispatches; 28 windows reach their 256-fetch cap. The first observed path
   returns from the System Card controller poll to its own dispatcher. The
   18-byte `$40D2-$40E3` RAM-loop report matches all 495 fetched byte slots to
   the latest address-scoped writes. The v12 all-PC target-write observer
   records writes across all 18 physical addresses, though `$1000E3`'s two
   captured writes both came from `$EA9C`.
   Of the 400 slots matched to `$EA9C` writes, 390 have unique and 10 have
   ambiguous source-read candidates, while 111 fetch rows are fully joined to
   CD-read candidates. The complete `$6109-$6114` helper execution and its
   three writes are now verified, but its caller and the earlier `$613A`,
   `$613F`, `$6144`, and temporary `$40D9` bytes remain unbound to a game-owned
   consumer. Continue tracing complete writers and callers until a game-owned
   poll and resulting state change are observed; there is still no game IRQ,
   aggregate authenticated CD-to-RAM receipt, or non-System-Card poll. The v22
   run-only callback reaches `$EC05/$E900`, then remains in the `$E95E/$E966`
   status loop; it still has no observed `$E209/$E11D` instruction starts or game
   handoff.
2. **Verify quest progression and original-format save transactions.** The
   authentic US `theron_v1_m11_real_bram_continue` integration test passed three
   TRV2 loops: it restores the proven Ak-Tu-Ba campaign byte and Theron
   attributes/skill experience, then exercises Continue, dungeon 2 selection,
   Soul Room champion selection, forcefield admission, and three native floor
   moves using authentic US Track 02. A no-input CLI boot-probe remains at the
   title and is not a Continue test. Extend the proven transaction to all
   regional save cases and bind later quest artifacts and their pickup, exit,
   chapter transition, and completion consumers. Keep the campaign-completion
   bit distinct from quest-item collection. The native Save Game input now
   writes the classified original-format BRAM at the between-stage boundary,
   remains blocked in a loaded dungeon, and a fresh native M11 Continue
   restores the proven campaign and champion fields from that written file.
   Continue with all regional save cases and reopen the changed save in the
   original runtime. The
   [Backup RAM layout note](docs/source-lock/theron-original-backup-ram-body-layout-2026-09-23.md)
   records the currently authenticated structure and limits.
3. **Complete source-bound presentation and event output.** Bind gameplay
   VDC/VCE/BAT state to the active map, objects, text, portraits, HUD, CDDA,
   and ADPCM/SFX consumers before implementing their production rendering or
   audio paths. Keep the screen-space capture route distinct from the live
   dungeon renderer, and do not use synthetic room fallbacks as gameplay
   evidence.
4. **Verify the complete game loop against original media.** After those
   consumers are bound, cover all seven dungeons, stairs, objects, doors and
   actuators, creatures and combat, spells, inventory, chapter progression,
   and save/resume. Compare both authentic regional editions against the
   [Theron parity definition matrix](parity-evidence/theron_v1_parity_definition_matrix.md).

## 2026-10-30 — trace all guest writes at the five fetched-code addresses

- Extended the reference-only PCE Fast target-write observer to record guest
  CPU and block-move writes at $1000D9-$1000DB and $10468D-$10468E, the five
  addresses that differed from the earlier $EA9C-only trace.
- In a second authentic JP cold run@480:300 capture, 76 writes to these
  addresses came from PCs other than $EA9C. All 228 fetched bytes at the five
  addresses had an earlier observed write with the same latest value. The new
  verifier reports zero missing writes and zero value mismatches.
- The writes at $10468D/$10468E came from STA $468D at $4684 and
  STA $468E at $4689, with accumulator values $E0/$47. The subsequent
  fetch at $468C was E3 E0 47, a TIA opcode and the start of its source
  operand. At $1000D9-$1000DB, a TII block move at $4055 had source and
  destination mappings that alias the same physical bytes; later writes
  changed the fetched bytes to $4C 48 61 and back to $EA bytes. These are
  runtime code writes, not evidence of a native gameplay handoff.
- This closes the specific writer-observer gap for the five addresses in this
  capture. The run still had no game IRQ, authenticated CD transfer receipt,
  or gameplay transition, and it does not establish loader completion or
  parity. Full remaining work is tracked in the JP source-lock note.

## 2026-10-30 — bind the JP System Card transfer to executed code

- Extended the PCE Fast reference trace with PC $EA9C CD-RAM writer records
  and instruction-byte snapshots for execution in $4000-$5FFF. The verifier
  checks the source bytes against hash-verified Japanese Rev. 1 Track 02, joins
  reads and writes by event order and value, and compares executed bytes with
  the latest traced write to each physical address.
- On a fresh cold run@480:300 capture, all 63,488 CD data-port reads were
  source-bound to authentic Track 02. All 53,248 $EA9C writes had an
  ordered/value candidate match to a recent $EA99 source read. Repeated bytes
  can make these joins ambiguous; they do not prove unique read-to-write
  causality. Of 262,144 code-fetch rows, 262,121 were fully backed by prior
  traced writes; 46 fetched bytes differed from the latest traced write.
  Those bytes span 23 fetch rows, five physical addresses in banks $80 and
  $82, and three logical PCs. Each address had a prior $EA9C write, but no
  earlier traced write matched the fetched value. A later untraced writer
  or mapping/trace semantics could explain this; the capture cannot
  distinguish them. The affected physical addresses were $1000D9 (2 bytes),
  $1000DA (1), $1000DB (1), $10468D (21), and $10468E (21); logical PCs
  were $40D7 (1), $40D9 (3), and $468C (42). Slot-2 MPR was $80 at the
  first three addresses and $82 at the last two, with no observed mapping
  change. The follow-up trace now observes guest writes at these five addresses; it does not prove a full CD-to-game transfer.
- The run remains blocked before the game-owned IRQ, authenticated CD transfer
  receipt, game input poll, and gameplay transition. Treat this as loader and
  execution provenance only; it does not establish game launch or parity.
- Capture files remain private on TRV2 under
  /home/trv2/firestaff-theron-evidence/capture/theron-jp-run-writer-opcode-cold-20261030/.
  The emulator remains reference-only.

## 2026-10-30 — trace the JP System Card Run path from authentic media

- Added an opt-in Mednafen trace for the System Card controller condition and
  its `$E000/$C860` Run path. On a hash-verified JP Rev. 1 cold start, the
  scripted Run press reached `$C8C7` with `$A=$08`, then executed the code path
  through `$E118/$E11B` and `$4000`.
- The same capture read 63,488 source-bound Track 02 bytes through the CD data
  port and wrote 85 bytes to CD RAM. The authentic-source verifier matched two
  15-byte windows from LBAs 4521 and 4589 to immediate bank `$80` writes.
- The capture has no game-owned controller poll, authenticated full loader
  receipt, or game `$E009` dispatch. It does not prove gameplay or parity.
  See [the cold System Card Run trace](docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md).
- Next: follow the remaining authentic Track 02 reads through the active game
  loader, then capture a game-owned controller poll and its resulting state
  change. Keep the native parity target open.

## 2026-10-30 — follow the BIOS carry branch to `$4000`

- Extended the reference-only PCE Fast trace to retain up to 8,192 bootstrap
  instruction-start rows and independently follow up to 4,096 contiguous
  instruction-start rows beginning at `$E11D`. The launch verifier requires
  clear carry at the authentic `BCC $E12C`, the `$E12C/$E12E` MPR setup call,
  `$E130 JMP $4000`, and the immediately following `$4000` row mapped through
  MPR2=`$04` to physical `$008000`, as derived from authentic `$E13D` code.
- The v18 and v19 authentic JP cold captures did not reach `$E11D` within the
  bounded observation. Their bootstrap rows stopped at `$E966` in the System
  Card's `$E95E` status-poll loop. v19's strict launch verifier correctly
  rejected the missing branch. Its separately armed `$4000` rows cannot be
  treated as a causal BIOS jump. Authentic `$E13D` disassembly shows the
  `$E12C` route maps MPR2=`$04` and `$4000` to physical `$008000`; MPR2=`$80`/
  physical `$100000` belongs to an independent loaded-code observation. A
  later audit found that the earlier static comparison used the wrong System
  Card file offset; corrected MPR/file offset mapping and disassembly are in
  the source-lock report.
- In v19, scripted RUN/SELECT inputs were consumed only at System Card polls;
  the receipt reports zero host-key events, CD IRQ callbacks, authenticated
  CD-to-RAM receipts, game `$E009` dispatches, and non-System-Card poll reads.
  This remains a blocked reference capture and establishes no gameplay,
  native handoff, or parity. TRV2 became unreachable during follow-up, so its
  private capture files were initially inaccessible. After reconnecting by
  IP, the v19 receipt and source were inspected; the redundant inactive v9
  reference build cache (96 MiB) was removed, while v10 and captures v18/v19
  were preserved.
- The v10 reference core's `pce_fast/pcecd.cpp::PCECD_Read()` identifies
  `$1800` status bits as BSY/REQ/MSG/CD/IO (`$80/$40/$20/$10/$08`). Of the 529
  instruction-start snapshots after `$E95E LDA $1800`, 501 were `$90`
  (BSY+CD), 26 were `$D0` (BSY+REQ+CD), and two were `$C8` (BSY+REQ+IO). These
  are A-register snapshots, not direct bus-read captures or proof of
  original-hardware signal semantics.
- The previous claim that runtime bytes at `$E966-$E972` differed from the
  System Card was incorrect: the disassembly command used a file offset 512
  bytes past the proper payload offset. The corrected BIOS listing matches
  runtime snapshots and confirms `$D0` handling plus `$B8`-masked `$98/$88/$80`
  branches to `$E98A`. Runtime instruction-start snapshots still are not
  separate opcode-bus receipts.
- v20 added the opt-in direct `$1800` SCSI-status receipt. Its verifier accepted
  50,932 authentic-media reads: `$90` 50,828 times, `$D0` 92 times, `$C8` 10
  times, and `$D8` twice. Both `$D8` events preceded the bootstrap-follow
  window; none appeared in its later status reads. The bootstrap trace entered
  `$E14C`, called `$EFF1`, and repeated `$F30D-$F313` through the row cap
  without returning to `$E11D`. It still observed no game handoff or gameplay.
- v21's authentic callback-state trace recorded 255 reads/writes and observed
  the `$2259=$04` System Card action take `$E166->$E176`, store `$02` to
  `$2269`, and call `$CC17`; it returns at `$CC96`, proceeds through `$E17C`
  and `$EC05`, then the 8,192-row window ends at `$E966` while polling
  `$1800`. It did not reach `$E209` or `$E11D`. A separate target window did
  execute `$4000` under MPR2=`$80`/physical `$100000`; its first four code
  bytes match source-bound Track 02 reads and `$EA9C` stores, but this does not
  prove the causal `$E130` jump. Authentic `$E13D` makes the strict launch
  target MPR2=`$04`/physical `$008000`, and the verifier/tests now require
  that mapping. Next trace the `$EC05/$E900` command/status exchange through
  `$E209` and `$E11D`. Preserve the distinction between reference evidence and
  native parity. Full findings and trace hashes are in the
  [source-lock report](docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md).

## 2026-10-29 — generalize authentic CD-read-to-RAM source joins

- Added `scripts/verify_theron_cd_ram_source_join.py`, which validates a paired
  read/write trace against hash-authenticated JP Track 02, then requires each
  selected RAM write to be the immediately following shared event, byte-equal
  to a System Card `$EA99` CD-port read, and mapped to the requested physical
  range. Mixed-track rows are excluded unless their receipt identifies this
  exact JP Track 02 layout. The bounded CD trace verifier now accepts up to
  the capture wrapper's documented 1,048,576-row maximum.
- The seven parser regressions passed in three local loops. On TRV2, the
  verifier also passed against the retained authentic JP cold capture
  `theron-jp-cold-joined-20261029-r2`: 30 exact source-read-to-RAM-write joins
  across 15 physical bank `$80` addresses, from source LBAs 4521 and 4589.
  It passed three consecutive runs. The verifier prints no media byte values
  and reads the authentic Track 02 only on TRV2.
- This validates a reusable source-provenance tool and its existing bank-$80
  capture. It does not establish the source of the 21 bank-$68 poll-code writes
  from the later poll-window run; that capture's raw traces were removed during
  the preceding disk cleanup. Next rerun a bounded authentic JP cold capture
  with the poll target enabled, apply this verifier to `$0D04D2–$0D04E6`, and
  capture a game-owned poll overlapping the scripted input before claiming
  an action or visible effect. Native Theron parity remains open.

## 2026-10-29 — authenticated Stage-2 colors are available; screen binding remains open

- `theron_v1_track02_load_stage2_vce_palette()` copies the authentic regional
  Stage-2 palette words into entries `$E0-$FF` of a native palette state after
  checking Track 02 identity, dispatch entry, routine bytes and payload bounds.
- Real JP and US BIN tests passed three loops on TRV2. The helper does not
  establish which screen or dungeon layer uses these colors. No viewport route
  is admitted by this data, and it must not clear the synthetic-rendering
  guard or set `palette_route_verified`.
- Next: capture a cold, source-bound execution of the palette loader and join
  its selected data to the game-owned VDC/BAT/tile state before using it for
  any screen rendering.

## 2026-10-29 — bound cold CD source tracing to Track 02

- The Mednafen PCE Fast CD data-port trace keeps its default 65,536-row
  all-source cap. For source joins it can now select source-valid reads inside
  Track 02 and raise the explicit cap up to 1,048,576 rows. The capture wrapper
  validates the options and records both settings in the transition receipt;
  filtering does not change emulated CD behavior or the independent RAM/VDC
  traces.
- The cold authentic JP run exercised the new filter and recorded 483,328
  source-valid reads from Track 02 (LBA 3,590–5,259). The `$D515` candidate at
  raw offset 643,413 was read by the System Card at `$EA99`; this is not proof
  of a copy into the game bank or execution.
- All 11 scheduled input events reached active-low controller reads at System
  Card PC `$E4C5`; no scheduled mask reached a non-System-Card poll during its
  hold. No complete bounded HuC6280 `$23AD` call or authenticated CD-to-game-RAM
  receipt was captured, and the VWR trace reached its cap. The Japanese SBI
  sidecar was absent, but this run does not prove that it caused the missing
  handoff. Native gameplay input and dungeon projection therefore remain open.
- Full trace counts, hashes, setup and limits are recorded in
  [the cold filtered capture note](docs/source-lock/theron-disassembly/theron-jp-cold-track02-filtered-input-20261029.md).
- Next: obtain a cold run that reaches the game-owned controller poll, then
  join loaded `$D515/$D588` code and pointer descriptors to the Button-I result
  and VDC destination before enabling native world clicks or projection.

## 2026-10-29 — C1DF CPU tracing is opt-in; pointer/action join remains open

- The PCE Fast C1DF instruction/register trace now follows
  `THERON_CAPTURE_POINTER_C1DF_TRACE`. With the flag off, a capture reports
  zero `pointer_pc`/`pointer_regs`/`pointer_c1df_*` rows; with it on, it records
  the requested bounded trace. The ordinary mouse-device and watched pointer
  RAM accesses remain available independently.
- The full Mednafen patch chain applied to an isolated TRV2 copy and a serial
  `-j1` PCE Fast build completed. Three loops of the focused capture regression
  passed. An A/B run used authentic JP Rev. 1 Track 02, System Card 3.0 and the
  preserved Ak-Tu-Ba state: zero C1DF rows with tracing off, 16,199 rows with
  it on. Both runs report `transition=missing` because save-state loading
  bypasses original loader provenance; this verifies trace gating only.
- Evidence hashes and limits are in
  [the C1DF trace-gate note](docs/source-lock/theron-disassembly/theron-pointer-c1df-opt-in-trace-20261029.md).
- Next: use the opt-in instruction trace in a cold source-bound run to join
  controlled mouse deltas with game-owned pointer coordinates and the original
  hover/click command result. Do not infer hitboxes from save-state runs.

## 2026-10-29 — qualify saved-state `$D39D` command-PC attribution

- A static disassembly of the `$D395` window in both authentic regional Track
  02 files shows a bounded indexed-record loop. It does not directly read
  `$2905` or mouse coordinates. USA and JP window offsets, hashes and the
  regional helper-call variants are recorded in
  [the `$D39D` source disassembly](docs/source-lock/theron-disassembly/theron-d39d-static-source-disassembly-20261029.md).
- Earlier saved-state instrumentation reports `$2905` accesses at `$D395` and
  `$D3A0`, but those static locations are `STZ $BA` and an operand byte if the
  matched retail bank is active. Keep the RAM observations; treat their PC
  attribution as unresolved until a live MPR/physical-PC/opcode trace is joined
  to authenticated CD-to-RAM provenance. The old traces do not identify a
  pointer routine, command consumer, hitbox or world-state effect.
- Three additional standard-PCE loops with `dx=4` plus Button I produced
  byte-identical command/consumer sidecars to zero-delta-plus-Button-I and the
  earlier large-delta control. The device logged the input, but guest pointer
  fields and `$2905` observations did not change. This saved-state/script route
  does not calibrate mouse motion; all runs remain transition/provenance
  blocked. See the [bounded matched-control report](docs/source-lock/theron-disassembly/theron-scripted-mouse-delta-replay-20261029.md).
- Next: capture the logical and physical PC, MPR, executed opcode bytes and
  effective address for a repeated authenticated mouse control; then resolve
  the caller and source LBA before changing direct native world-click behavior.

## 2026-10-29 — standard PCE mouse scripts honor bounded repeats; coordinate join remains open

- The Mednafen mouse-script instrumenter now inserts one shared parser into the
  standard PCE and PCE Fast input paths. It expands `*repeat`, rejects
  malformed samples, and caps expanded scripted updates at 20,000.
- The compiled parser regression and live-capture script passed three loops.
  The complete Theron Mednafen patch set applied to three isolated TRV2 source
  copies, and a serial `-j1` build compiled both PCE cores.
- A reference run with authentic USA Track 02, System Card 3.0 and the
  preserved Ak-Tu-Ba save state sent `1,0,0*3;0,0,1;0,0,0`. Its device trace
  records three horizontal input units across two latches and the Button I
  mask. This verifies script expansion and device delivery only. The save-state
  run has no authenticated CD-to-RAM receipt and reports `transition=missing`;
  it does not establish guest pointer coordinates, click targets, or native
  Theron parity. Evidence and hashes are in
  [the mouse repeat parser note](docs/source-lock/theron-disassembly/theron-mouse-input-repeat-parser-20261029.md).
- Next: join controlled mouse deltas to game-owned pointer state and the
  original command/result path in a source-bound session before implementing
  direct viewport clicks.

## 2026-10-29 — authentic mouse motion moves the Theron hand cursor; click semantics remain open

- A cold Mednafen PCE Fast run used authentic Japanese Rev. 1 media and the
  headerless System Card 3.0 body, then switched port 1 from gamepad to mouse
  at the dungeon. Two screenshots taken after host-mouse positions 150 screen
  pixels apart differ only in a 41×51-pixel box around the original hand
  cursor (708 changed pixels). This confirms visible cursor response to
  emulated PCE mouse motion; the [PCE Fast device reference](https://mednafen.github.io/documentation/pce_fast.html)
  documents the mouse port.
- This was a screenshot-only probe: it has no guest-input trace, source
  coordinate receipt, command-buffer write, target receipt or state-transition
  receipt. A later click was followed by a black viewport, so its effect is
  inconclusive. No hitbox or click mapping is justified yet. Evidence and
  limits are in
  [the pointer-motion capture note](docs/source-lock/theron-disassembly/theron-jp-mouse-pointer-motion-20261029.md).
- Next: reproduce cursor motion with bounded game-owned pointer coordinates,
  join the Button I action to its command consumer, and test clicks against
  source-backed controls before implementing direct Firestaff world clicks.

## 2026-10-29 — compressed mouse script reaches late cold-start polls; BIOS handoff remains open

- The live Mednafen mouse-script grammar now supports `dx,dy,button-mask*repeat`
  with per-token and total limits of 20,000 updates, matching the bounded
  device trace capacity. A single bounded capture can now wait through a long
  authentic boot interval without inflating the 8,191-byte script field. The
  instrumenter patch applied to clean Mednafen
  1.32.1 sources and its incremental `pce_fast` build passed.
- Three loops of accepted script forms and five malformed/bounds-rejected
  forms passed. A cold authentic Japanese Rev. 1 PCE Fast capture then traced
  Run as active-low `$37` and Button I as `$3E` at System Card poll `$E4C5`.
  The CPU stayed in the System Card through frame 10,384; no CD IRQ, game
  handoff, authenticated CD-to-RAM receipt or C1DF pointer call was observed.
  Mednafen also logged the Japanese SBI sidecar missing, which may affect the
  route but is not established as the cause.
  Details and capture hashes are in
  [`the cold mouse Run trace`](docs/source-lock/theron-disassembly/theron-jp-cold-mouse-run-trace-20261029.md).
- This confirms delivery of those mouse button bits, not a Theron menu action.
  The transition blocker remains open; use the visually confirmed cold route
  with source-bound handoff evidence before interpreting game behavior.

## 2026-10-29 — cold Japanese route reaches the first-person dungeon; input provenance remains open

- A fresh Mednafen 1.32.1 `pce_fast` run opened authentic Japanese Rev. 1 media
  and a canonical headerless System Card 3.0 body, with `autosave 0` and no
  loaded state. Four-second Button I holds advanced the cold route through
  new-game, file, stage and story steps; six separate 250 ms Button I presses
  advanced the story and the final screenshot shows a first-person dungeon.
- The screenshots are real emulator output, but this run produced no
  instrumented guest-input, CD-to-RAM, or gameplay-command receipt. Mednafen
  also reported the Japanese SBI sidecar missing. The full headered System
  Card image was unavailable. This establishes a visual route to the dungeon
  only, not source-bound transition semantics, mouse behavior, or Firestaff
  parity. The screenshot also shows the original hand cursor. The retail
  [TurboGrafx-16 manual, pp. 4–6](https://www.tg-16.com/contributors/bernie/Dungeon_Master/Dungeon_Master.pdf)
  documents D-pad cursor movement and Button I actions; it does not document
  physical mouse input. The private bundle, without media, firmware, save-state
  or raw RAM, is
  `/home/trv2/firestaff-theron-evidence/capture/theron-jp-story-to-dungeon-20261029-r2/`.
- Next: reproduce the route with bounded input and CD-to-RAM provenance, then
  bind original pointer deltas to guest pointer coordinates and the command
  consumer before changing direct dungeon-view mouse behavior.

## 2026-10-29 — source graphics chronology marks bounded-stream limits

- The source-graphics event merger now receives configured CD data-read, CD
  RAM-write, VWR, and VCE limits and records per-stream counts. Reaching any
  configured limit marks chronology as
  `possible_truncation_at_event_limit`; the capture receipt reports
  `merged_limit_reached`, so it no longer looks unqualifiedly
  complete. A stream below its limit is marked accordingly, while calls that
  supply no bounds say `not_checked`.
- The focused merger test passed with Python 3.9 and 3.13, including exact-cap
  metadata and clean rejection of non-ASCII/oversized sequence input. The
  live-capture script regression and `bash -n` passed as well. The Mednafen
  patch-application portion was not rerun, and no original-media capture was
  produced for this follow-up.

## 2026-10-29 — gate VWR capture at the first authentic Track 02 read

- The opt-in PCE Fast VWR trace mode now omits VRAM-commit records until the
  CD trace observes the first source-valid read inside Track 02. Its V4 header
  records the filtered-prefix scope, and the merged chronology identifies the
  VWR stream in `filtered_prefix_streams`. Default captures retain the
  unfiltered V3 behavior.
- The patch chain applied and built on TRV2. A cold capture with authentic
  Japanese Rev. 1 media opened the gate at source LBA 3590 (shared event
  24,768,120); its first VWR record has sequence 0 at event 24,779,401. The
  output metadata records the filtered prefix. This authenticates the gate's
  trace boundary, not graphics causality or world-cell meaning.
- The VWR stream later reached its 1,048,576-record cap, so the merged file is
  marked `possible_truncation_at_event_limit`. The capture verifier also
  timed out before observing two of eleven scheduled controller masks. No
  dungeon gameplay was confirmed. Continue with bounded input/capture work;
  do not treat this as native Theron parity.

## 2026-10-29 — cold replay confirms controller reads; BIOS handoff remains open

- The bounded input trace and its verifier now allow up to 2,097,152 records.
  Three focused capture-script regression loops passed. The previous one
  million-record ceiling truncated before the last scheduled cold-route input.
- A new no-save-state PCE Fast capture used authentic Japanese Rev. 1 Track 02
  and the headerless System Card 3.0 body. It followed the visually recorded
  route: a four-second Run hold, four four-second Button I holds, then six
  longer Button I presses to tolerate the BIOS poll gaps. All eleven scripted
  masks have apply and controller-read witnesses; the last witness is at
  System Card PC `$E4C5`, sequence 1,737,370. This verifies delivery to the
  BIOS poll only.
- The first source-valid Track 02 read is LBA 3590 at shared event 24,768,120;
  the first VWR commit is sequence 0 at event 24,779,401. The merged stream
  marks the VWR prefix as filtered and reports the 1,048,576-record VWR cap.
  Event order is temporal evidence only, not graphics causality.
- The run still reports zero CD IRQ callbacks, zero authenticated CD-to-RAM
  receipts, no non-System-Card controller poll, and `transition=missing`.
  It did not confirm dungeon gameplay. Next reproduce the visual route through
  the same host-key path while tracing controller reads and BIOS handoff; do
  not infer gameplay from button delivery or claim native Theron parity.

## 2026-10-29 — authenticate stage-message advancement; gameplay still not reached

- A cold Mednafen reference run used authentic Japanese Rev. 1 Track 02 and
  the canonical headerless System Card 3.0 body, with the original Japanese
  SBI sidecar and no save-state. Real runtime screenshots show the Ak-Tu-Ba
  stage card, its first story page after Button I, and a later story page after
  another short Button I press.
- The input trace records Button I at the System Card poll (`raw=0001`,
  active-low value `$3E`). The successive screenshots confirm that the short
  press advanced the displayed story page. The run stopped at the story text;
  it did not reach hero selection or first-person dungeon gameplay.
- The full headered System Card image was unavailable. The private evidence
  bundle, containing screenshots and compressed traces but no source media or
  BIOS, is `/home/trv2/firestaff-theron-evidence/capture/theron-menu-progression-20261029-r1/`.
  Its manifest pins the three screenshots and both traces. This establishes
  one original menu transition only; it does not establish page-advance
  semantics, dungeon admission, or native Theron parity.
- Next: continue from the authentic stage-message sequence through the Soul
  Room and into a first-person dungeon, retaining the original-media and
  controller receipts at each transition.

## 2026-10-29 — scripted mouse deltas still do not bind guest pointer targets

- Four isolated reference captures used the authentic USA Ak-Tu-Ba state
  (`f17f377df210b4a3ae904a13fb85a7f0`), USA Track 02 and System Card 3.0.
  The existing Mednafen mouse injector was given `(60,120)`, `(121,98)`, the
  sign-reversed `(−121,−98)`, and a split cumulative horizontal movement.
- The first three captures observed the original queue `$2905=$50` and hover
  `$2911=$50`, including the nominal right-column candidate. The device trace
  records deltas and latch values, not the guest's final pointer coordinates.
  The loaded state lacks Mednafen's serialized mouse fields, and its latch
  deltas have reversed signs, so these results cannot be treated as a valid
  reproduction of the previously captured `$79/$62 → $74` point.
- All captures report `transition=missing` and zero authenticated CD-to-RAM
  receipts because loading the state bypasses loader provenance. Raw traces and
  the wrapper snapshot are retained in the task-specific external-disk
  evidence folder; its README records the capture inputs and limits. The
  trace hashes and exact limits are recorded in [the scripted mouse replay
  report](docs/source-lock/theron-disassembly/theron-scripted-mouse-delta-replay-20261029.md).
- Next, bind device deltas to the game's guest pointer state in a controlled
  original-session trace, then reproduce the `$74` control before sweeping
  boundaries. Do not publish new hitboxes or mouse actions from these probes.

## 2026-10-29 — isolate Japanese pointer-table writes by input condition

- Four paired Japanese Rev. 1 PCE Fast reference captures reused one authentic
  Ak-Tu-Ba save state and instrumented binary. The zero-input and movement-only
  controls made no writes in the watched `$28FC..$2905` BaseRAM range. Button
  I-only input wrote candidate bytes at `$28FE/$28FF` and `$2902/$2903`; the
  mixed movement-plus-Button-I run also wrote `$28FC/$2900` and `$50` at
  `$2904/$2905`. Writer PCs include `$CCD8`, `$CCDD`, `$CCE2`, and `$D39D`.
- This isolates an input-linked write difference but does not identify the
  pointer table, cursor coordinates, hit target, or command semantics. All
  captures lack authenticated CD-to-RAM provenance because the save state
  bypasses cold startup. See the
  [candidate-address write isolation report](docs/source-lock/theron-disassembly/theron-jp-pointer-table-write-isolation-20261029.md).
- Next: disassemble the bounded writer windows, join the bytes to the original
  pointer descriptor and command consumer, and reproduce the evidence in a
  cold original-media session before changing native hit testing.

## 2026-10-29 — disassemble the runtime pointer writer

- A new PCE Fast build records bounded opcode windows through the active MPR
  mapping at changed candidate writes. Three patch-application loops passed,
  and the isolated Mednafen 1.32.1 build completed with `-j1` on TRV2.
- The Japanese `$CCD5..$CCEE` window matches seven copies in authentic
  `TQJP02.bin`. MAME disassembly shows a four-entry indexed store loop using
  `$2911`, `$C7` and `$C9`; the captured writes used X values 2 and 3. This is
  useful dataflow evidence, not a proven cursor or hitbox table. See the
  [runtime writer-window report](docs/source-lock/theron-disassembly/theron-jp-runtime-pointer-writer-window-20261029.md).
- The older mixed capture's `$2904/$2905` writes did not reproduce in two new
  runs, and all save-state captures lack CD-to-RAM provenance. Next, recover a
  reproducible command dispatch and `$D39D` opcode window, then bind the live
  code to a cold original-media load before changing input behavior.

## 2026-10-28 — map Theron panel clicks against presented pixel bounds

- The authenticated USA Track 02 SDL integration test failed at the first
  dungeon panel sequence. The debug receipt showed SDL clicks arriving at
  `(301,128)` while the runtime presentation target was 1280x960; the Theron
  panel hit-test normalized those already-mapped 320x200 source coordinates a
  second time and returned no action.
- The source-space hit-test fix made regular SDL panel routes pass, but the
  1280x960 edge case then reproduced the separate inverse-rounding loss: the
  visible pixel `(729,590)` arrived as source `(182,122)`, one row above the
  first host button. Added a Theron-only pre-inverse hit test against the
  floor-scaled presentation rectangles; the focused debug route now reports
  the expected logical cell center `(204,132)` and `TURN LEFT` action.
- The full `theron_v1_sdl_input_events` test passed on TRV2 against authentic
  USA Track 02 (`f23601102138f87c33025877767ebf76`). It exercises keyboard,
  Button II, standard mouse-panel movement/items, touch, and a 1280x960
  fractional-edge click. The fractional fixture uses its own resolution
  config so the other real-media routes retain their ordinary 320x224 window.
- This failure concerns Firestaff's host panel. Direct world-space mouse
  targets and complete Theron parity remain open.

## 2026-10-28 — disassemble the authentic System Card controller branch

- MAME `unidasm -arch h6280` disassembly of the authentic System Card 3.0
  (`ff1a674273fe3540ccef576376407d1d`) maps the live controller poll at
  `$E4B4/$E4C5`. It samples direction and button nibbles from `$1000`, updates
  `$2228/$222D`, then tests the per-pad mask at `$2227` and the values `$04`
  and `$0C` before dispatching through `($2284)`. The bounded excerpt and
  address mapping are in
  [the controller-poll disassembly](docs/source-lock/theron-disassembly/theron-system-card3-controller-poll-e4b0.asm).
- Fresh authentic JP Rev. 1 capt…140031 tokens truncated…tine
  entry. A full TRV2 rebuild and fresh original-media run are pending. No
  decoder or gameplay parity is established. See
  [the `$23AD` trace-boundary record](docs/source-lock/theron-disassembly/theron-23ad-trace-boundary-20261029.md).
- A fourth cold-start run reached the research generator's start receipt, but
  its experimental detector used `$AD` rather than the authentic `$A5` opcode
  for `LDA $30`; that run was stopped before interpreting a missing trace. The
  detector now searches the source-bound `$A5/$8D/$A5/$8D/$44` prefix and
  applies the full caller hash only at the candidate BSR.
- A correctly configured cold USA run used the original CUE, System Card 3.0
  and authenticated Ak-Tu-Ba Backup RAM. It reached the generator start and
  level-03 re-entry. The live caller matched at logical `$3B86` with MPR slot
  1=`$F8`, and the routine entry at `$3BAD` matched the 382-byte retail hash.
  The 20,000- and 250,000-event bounds ended before the caller return. At
  250,000 events the compressed input count had fallen from `$0A36` to `$076A`,
  leaving 1,898 bytes. A 2,000,000-event capture then recorded one full
  invocation: 921,436 lines, no truncation, the terminal RTS at logical
  `$3C29`, and trace SHA-256
  `3668f340372938c2a366237d11dd63158eb8c312e8372d0716f684eb968465bb`.
  This proves dynamic routine control flow and fetched retail bytes. The
  enclosing capture was still rejected with `transition=missing` and zero
  authenticated CD-to-game-RAM receipts, so loader provenance and gameplay
  handoff remain open. The raw trace was deleted after recording its hash and
  summary; see the [trace-boundary evidence](docs/source-lock/theron-disassembly/theron-23ad-trace-boundary-20261029.md).
- [x] Compare the authentic `$23AD` source and destination streams against the
  native decoder for the captured US resource. The original Track 02 resource
  at normalized LBA 1871, offset 6, supplied 2,614 bytes; its 5,064 runtime
  writes match the native output byte for byte. This exposed and fixed the
  pointer-table index arithmetic at `L2496`: the retail routine subtracts one
  from each token byte before indexing. A focused `$0105` boundary vector
  covers the arithmetic. Source and output hashes and exact decode counts are
  recorded in `DONE-theron.md` and the trace-boundary evidence document.
- [ ] Establish causal disc-to-game-RAM provenance and a gameplay handoff.
  The current trace proves a complete `$23AD` invocation and exact source and
  output parity for one authentic US resource, but the enclosing cold-run
  verifier still reports `transition=missing` and zero authenticated
  CD-to-game-RAM receipts. Full-level completion and other editions remain
  unproven.

## 2026-10-29 — authentic later-level decoder context remains unresolved

- Probed all seven hash-admitted US and JP Track 02 later-level spans with the
  current native `$23AD` lift in three loops. With an empty pointer-table seed,
  all 42 edition/level attempts stopped at `POINTER_TABLE`. As a diagnostic
  only, seeding the table by interpreting the compressed frame bytes as
  little-endian pointers still stopped at `DESTINATION` in 39 attempts and
  `POINTER_TABLE` in three. The probe used destination base `$6000`, which is
  established for the one separately captured invocation but not for these
  later-level spans; those failures do not prove the retail decoder fails.
- No guessed seed or generated game payload was retained. The spans pass their
  authentic media admission hashes, but the source-bound MPR map, destination
  base, pointer-table contents at call entry, and frame-chain relationship for
  these later-level calls are still unknown.
- Next: capture one of these exact admitted source spans in a cold retail run,
  recording the effective source and destination windows, pointer-table bytes
  before and during `$23AD`, and the downstream consumer. Then compare that
  invocation byte for byte before extending native support to more spans.
- A cold authentic US capture now records a complete `$23AD` invocation and
  the bounded post-return window while reaching the Drator generator route and
  its level-03 re-entry event. The joined chronology was generated and
  validated on TRV2. It contains 1,121,429 `$23AD` events, including one
  matched return and one post-return-window start; the post-return window
  reached its 200,000-event cap. The merged file correctly reports possible
  truncation because the VWR, VCE, and `$23AD` streams reached their bounds.
  Ordering is temporal evidence only and does not establish CD-to-RAM causality
  or gameplay parity. The merger initially rejected valid event names such as
  `page1_read`; the parser and regression fixture now accept digits in event
  kinds. Continue by tracing the relevant post-return RAM effects to a proven
  loader/gameplay consumer, then cover other authentic editions.

## 2026-10-29 — observe reads from the decoded output window

- The authentic `$23AD` trace writes physical `$0D0000–$0D13C7`. A bounded
  PCE Fast consumer hook can now select that physical range. The byte-join
  verifier confirms one decoder return and all 5,064 ordered destination bytes
  with SHA-256
  `9adf13522e818ee49de16ac1020a7697c3416877e6bc6baa918840d7e963f8ee`.
  The shared-sequence join found 29,618 post-decode output-range reads touching
  all 5,064 addresses; 6,111 reads matched the decoded byte across 3,130
  addresses. The updated verifier reports total span coverage separately from
  byte-equal candidates.
- A second verifier now joins post-write reads using the shared PCE Fast event
  sequence and reports equal-byte read candidates with physical PC and captured
  code bytes. It deliberately does not label candidates as game-owned or claim
  semantic use. Its synthetic boundary tests pass in three loops, and it parses
  the authentic decoder stream with the expected output digest. It now rejects
  legacy consumer traces without the shared-sequence header instead of treating
  an unobserved destination as an empty read result.
- The opt-in PCE Fast range
  `FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_CONSUMER_RANGE=0d0000-0d13c7` was applied
  and compiled in a full isolated TRV2 build with `-j1`. It remains disabled
  by default, validates ordered hexadecimal endpoints within the 2 MiB physical
  bus, and retains the existing 65,536-event cap.
- A fresh cold capture used the authentic USA Track 02 (MD5
  `f23601102138f87c33025877767ebf76`), headered System Card 3.0 and the
  authenticated USA Backup RAM. The capture script returned `BLOCKED` because
  the final scripted Button I event at frame 13,275 had no controller poll
  during its 15-frame hold; its transition receipt correctly says
  `transition=missing`. The raw decoder and consumer traces were preserved and
  can still establish the bounded byte-level observations below.
- The capture contains exactly one `$23AD` return and 5,064 ordered writes to
  physical `$0D0000–$0D13C7`, with SHA-256
  `9adf13522e818ee49de16ac1020a7697c3416877e6bc6baa918840d7e963f8ee`. The
  shared-sequence join found 29,618 reads after the final write, including
  6,111 byte-equal reads across 3,130 unique output addresses.
- The recorded reader at physical PC `$1042AB` executed bytes
  `B1 00 9D E0 47 C8 E8 B1`; these match authentic Track 02 LBA 4,241, user
  offset 683, read in the same capture. The sequence is `LDA ($00),Y`,
  `STA $47E0,X`, `INY`, `INX`, then another indirect load. It byte-copies at
  least 1,052 unique decoded-buffer bytes, all matching the output, to logical
  `$47E0,X`; the physical destination depends on the active MPR2 mapping and
  is verified by the target-write trace below. The next step is to trace later
  consumers of those copied bytes. The RAM-resident reader at `$1F1C3E`
  executed bytes `B2 2E 85 0E E6 2E D0 02 E6 2F 86 0F A5 12 D0 02`; they match
  both the final PCE Fast RAM snapshot at offset `$1C3E` and authentic Track 02
  LBA 4,230, user offset 1,086, read in that capture. These source-byte matches
  support game-code ownership for the observed readers; the precise loading
  path and the semantic role of the decoded data are not yet established.
- Disassembly of the authentic runtime snapshot around `$1F1C3E` shows a byte
  refill helper: it loads through `($2E)`, stores to `$0E`, advances the
  `$2E/$2F` pointer, stores the refill count in `$0F`, and updates `$12/$13`
  before returning to the adjacent `$1F1C34` loop. That loop rotates `$0E`,
  `$10`, and `$11` while extracting bits. The observed `$0D` reads therefore
  enter a bit-reading path. Its output destination is now observed below, but
  its gameplay structure and meaning remain unknown.
- The opt-in PCE Fast code-bank write trace now covers physical PCs
  `$1F1C00–$1F1DFF` and records writes into physical `$0D0000–$0D1FFF` with the
  logical and physical destinations, MPRs, and shared event sequence. The
  former `$1F1CFF` upper limit missed writes at `$1F1D1D`. On a new cold
  authentic USA run, the trace recorded 10,128 writes. The reusable verifier
  joined all 5,064 ordered `$23AD` output writes to an exact address/value code
  trace row one shared-sequence step later: 1,840 at `$1F1C8D`, 1,432 at
  `$1F1CE6`, and 1,792 at `$1F1D1D`. The other 5,064 code-window writes occur
  after decoder return and remain unattributed. This replaces the earlier
  incomplete 3,272-match result and its 2,248 unexplained rows.
- A bounded write trace for the observed `$1042AB` reader was then narrowed to
  physical `$1047E0–$1048DF` after the broad two-bank window reached its
  65,536-row cap before the decoder returned. The narrow authentic capture
  recorded 12,928 target writes without truncation. Post-decode `$1042AB`
  reads matched the authentic output byte at 1,052 unique source addresses;
  1,972 of 2,012 byte-equal reads were followed immediately in the shared
  event sequence by a `$1042AD` write of the same byte. Its runtime code bytes
  are `9D E0 47`, the store in `STA $47E0,X`; the recorded logical and physical
  destinations map through MPR2=`$82` to `$1047E0–$10485E`, with 64 unique
  destination addresses. This verifies a byte-copy operation from the
  decoded buffer. It does not identify the copied structure or prove a
  rendering or gameplay effect.
- The current cold capture still returned `BLOCKED`: the final scripted
  controller mask had no poll witness (`events=12/12`, `input_reads=78,518`),
  and the transition receipt says `transition=missing`. It recorded no CD IRQ,
  `$E009` dispatch, or gameplay transition. Next follow the copied bytes from
  `$47E0` through the bit-reader's later consumers to a proven game-state or
  rendering effect, and repair the replay so each held mask has a poll witness.
  Keep the adjacent `$F425` ADPCM-buffer copy separate unless its own source
  and consumer are independently established.
- The capture remained `BLOCKED`: the final scripted button event had no
  controller-poll witness during its hold and the receipt says
  `transition=missing`. No gameplay or render transition is claimed.
- Next: repair the capture plan so every scripted event is followed by a
  controller poll while its hold is active. Then separate the unmatched
  `$1F1Cxx` writes from the `$23AD` decoder output and trace the confirmed
  destination through a coherent gameplay/render transition. Keep the
  adjacent `$F425` ADPCM-buffer copy separate unless its own source and
  consumer are independently established.

- A follow-up cold US run on 2026-10-29 shortened the replay to ten events.
  It still returned `BLOCKED`: the final Button I event had no controller-port
  poll exposing its mask during its 15-frame hold (`events=10/10`,
  `input_reads=78,388`), and the transition receipt says `transition=missing`.
  Its authenticated media, System Card, and BRAM hashes match the preceding
  run; there was no autoload state. The raw `$23AD` trace contained 1,121,433
  lines and the bounded consumer trace 65,537 lines. Those raw r4 traces were
  later removed during TRV2 disk cleanup, so the candidate-read verifier cannot
  be rerun on this capture. This repeated poll gap means the replay still needs
  an input plan that demonstrably exposes every held mask before proceeding to
  semantic tracing.
- Review found that the capture wrapper accepted
  `THERON_CAPTURE_MAIN_RAM_CONSUMER_SAMPLE_LIMIT` through 1,048,576, while the
  instrumented C hook still stopped silently at 65,536. That limit is now
  configurable and has an explicit truncation receipt. A cold authentic test
  then showed that its shared budget filled with default `$1F0000–$1F7FFF`
  reads before any selected `$0D0000–$0D13C7` consumer reads were captured.
  The independent-limit correction passed patch-only application and a PCE
  Fast `-j1` build on TRV2. A cold authentic USA capture recorded exactly
  4,096 reads in each class with separate truncation markers. In a follow-up
  cold run, an earlier aggregate-cap binary recorded 53,790 reads in the
  selected `$0D0000–$0D13C7` range and 11,746 in default `$1F0000–$1F7FFF` RAM.
  The `$23AD` trace again verified one ordered 5,064-byte output against the
  expected digest. After its final write, the consumer trace read all 5,064
  output addresses 29,618 times; 6,111 reads matched decoded bytes at 3,130
  addresses. Prominent reader PCs include `$104287`, `$1042AB`, and `$1F1C3E`.
  This is byte-equal read-candidate evidence only; it does not establish the
  routines' source ownership, the downstream write effect, or gameplay use.
  The scripted input still missed a controller poll, and the capture recorded
  no CD IRQ, `$E009` dispatch, or gameplay transition. Next disassemble the
  candidate readers, trace their bounded destination writes, and join those
  writes to an authentic game-state or rendering effect. The follow-up used an
  earlier aggregate-cap binary without sample-class fields; its stream counts
  were re-derived from the physical address fields.
- A cold authentic JP Rev. 1 capture on 2026-10-29 extended the bounded CD
  RAM write trace to include runtime poll code `$0D04D2–$0D04E6`. The patch
  applied to the isolated Mednafen source, the instrumented PCE Fast build
  completed on TRV2, and the focused verifier/script checks passed three
  loops. The 300-second capture verified 483,328 source-bound Track 02 data
  reads and 64 runtime `$44D2` windows (60 with exact physical PC `$0D04D2`).
  It recorded all 21 expected poll-code bytes written in order to bank `$68`
  by PC `$EA9C`; this proves a runtime RAM write, not its source. None of the
  seven identical static Track 02 candidates was read during the capture, so
  the source-LBA-to-bank mapping remains unresolved. Eleven scripted inputs
  were consumed at System Card PC `$E4C5`, without a poll witness during their
  holds. The run recorded no CD IRQ, authenticated CD-to-RAM receipt, `$E009`
  dispatch, or gameplay transition (`transition=missing`). Do not claim that
  these input events caused the poll-code writes or that gameplay parity is
  established. Next identify the authentic source of the `$EA9C` writes and
  arrange a replay whose input masks overlap a game poll before tracing a
  visible state/render effect.


## 2026-10-29 — authenticate poll-code source and game input sample

- A new 300-second cold JP Rev. 1 capture used the original Track 02 and full
  System Card with no save-state autoload. It joins all 21 `$EA9C` writes to
  the immediately preceding `$EA99` reads at Track 02 LBA 3,625, writing the
  bytes to physical `$0D04D2–$0D04E6` with MPR2 `$68`. The 21 selected writes
  cover all 21 target addresses. This closes the earlier source-LBA gap for
  this runtime poll-code copy.
- The same replay applied all 12 scheduled inputs and the active-low UP
  direction mask was read on every frame 17,600–17,899 by non-System-Card code
  at `$44C1` (`SEL=1`, raw `$0010`, returned `$3E`). The adjacent `$44D2`
  sample is the button-bank phase and reads idle `$3F`; it must not be cited as
  the UP sample. The scripted-input verifier now attributes its witness to the
  matching read and ignores later unmatched reads when choosing the witness.
- The capture still ended `BLOCKED`: no CD IRQ, authenticated CD-to-RAM
  gameplay receipt, `$E009` dispatch, or gameplay transition was observed.
  Delivery of UP to a game-code poll and authentication of the poll-code copy
  do not prove that the input changed party state or caused a visible effect.
  Next trace the `$44C1` caller and its party-state or rendering consumer using
  the existing source-bound instrumentation; keep action and rendering parity
  open until such an effect is observed.
- `verify_theron_cd_ram_source_join.py` now filters writes by the requested
  MPR2 before validating that window. The capture also contained unrelated
  `$EA9C` writes into the same physical address window with MPR2 `$80`; those
  are excluded from the `$68` join. The filter change and the scripted-input
  witness attribution regression passed three consecutive focused test
  loops.
- Private trace SHA-256: input
  `e61e2b5b69316bd7492703ecdb90074782977273dc28880cfed97951e84d17e4`, CD
  reads `464f0e6cd97c3899c03733a342abb97818455d216b3e14ed21435398f6c78339`,
  and RAM writes
  `9530d3ee32a8acd3c2f5104851595ce48ac9faf0daacc4083fdfe418297b5cff`.
  Raw traces and original media remained on TRV2 and are not committed.

## 2026-10-29 — validate the PCE Fast TII position observer at runtime

- Added a capture-only observer for the original `$C1FA` `TII $20B4,$2040,$0002`
  position transfer. It records the latched opcode PC/MPR, source and
  destination logical/physical addresses and MPRs, byte value, and remaining
  block-transfer length. A source audit confirms that PCE Fast `BMT_TII` calls
  `WrMem(destination, RdMem(source))` and retains the latched PC across yields.
- The isolated PCE Fast object and executable rebuilt on TRV2 with `-j1`; the
  executable includes the observer. The focused live-capture test passed three
  loops, along with Python syntax validation and `git diff --check`.
- The authentic JP Rev. 1 PCE Fast replay loaded the user-created state and
  wrote command `$2905=$04`; the bounded movement-context trace recorded no
  `$2040/$2041` `runtime_state_write` rows, and the gated `$C1FA` TII observer
  produced zero rows. The position remained `$02/$03`; the bounded capture was
  also blocked before cold CD/loader receipts. These trace results do not prove
  that no position writes occurred outside the recorded rows or whether the
  movement was blocked by map geometry or another runtime condition. The active
  JP map for that state is still unresolved.
- Next bind the active map and destination tile from authentic Track 02/runtime
  evidence, then run an accepted command plus an identical no-input control.
  The 2026-10-30 cold scripted replay reached a game-owned controller poll and
  recorded the same `$C1FC` TII writes to `$2040/$2041`, but the capture still
  lacked CD IRQ, authenticated CD-to-RAM, and `$E009` handoff receipts. This
  does not prove that the verified story route or an active map was loaded in
  that run. The shared diagnostic event sequence and full MPR snapshots make
  hook ordering and physical mappings inspectable; they do not identify the
  candidate bytes' consumer or establish collision semantics. See
  `docs/source-lock/theron-disassembly/theron-pce-fast-tii-position-observer-20261029.md`.
  The initial observer gate used `$C1FA`; the follow-up below corrects it to
  `$C1FC` and records both ordered bytes. Details are in
  `docs/source-lock/theron-disassembly/theron-pce-fast-tii-position-observer-20261029.md`.

## 2026-10-30 — resolve stair-hosted actuator target consumers

- Bind each stair-hosted `FLOOR_PARTY` target category and regional raw record
  to the original actuator consumer, including SARMON `(1,9)` category 2
  (`ref=0x0803`; `feff3106` JP, `feff0902` USA).
- Do not infer mutation semantics from the data records. Capture a positive
  original-runtime event and an equivalent no-input control before enabling the
  target. Locate the current enablement site at
  `theron_v1_world_dispatch_source_actuator_events()` before implementing.

## 2026-10-30 — available Akutuba state does not reach a stair tile

- Built and ran `test_theron_v1_track02_dungeon_loader` from the current source
  on TRV2 against the installed authentic US and JP Track 02 data. The test
  passed. A temporary US source-map census found no stair-class tile on
  Akutuba map 0; the first US Akutuba stairs are on map 1 at `(8,1)` and
  `(17,8)`, with five more on map 2 and eighteen on map 3.
- Extended `test_theron_v1_track02_dungeon_map` to print a cardinal N/E/S/W
  bit mask for adjacent `OPEN` source tiles at each authenticated stair cell.
  On TRV2, the updated target built and the US and JP real-media CTests each
  passed three consecutive runs. The census found 39 US and 42 JP stair cells
  with at least one adjacent `OPEN` source tile. Akutuba map 1 `(17,8)` is a
  shared US/JP capture candidate with an adjacent `OPEN` tile to its south
  (mask bit 2). This mask reports source adjacency only; it does not establish
  a reachable route, active runtime map identity, stair direction, or target.
- The retained authentic post-Akutuba reference state
  (`f17f377df210b4a3ae904a13fb85a7f0`) is position-bound at `(2,3)` in the
  existing command captures, but it is not bound to a stair-bearing source
  tile. Do not use it to claim a stair transition or infer a destination from
  neighboring levels.
- Next obtain an authentic original-runtime state with its active Track 02
  level and a reachable stair tile bound in the same capture, then record a
  positive stair traversal and no-input control before implementing that
  specific transition. Keep all other stair routes blocked.

## 2026-10-30 — identify accepted UP+II movement-state transfer

- A corrected authentic JP Rev. 1 save-state replay used `ii@470:140,up@471:120`.
  At frame 471 the original accepted `$2905=$03` at `$CC99`, then cleared it
  at `$D39D` in frame 472. The position-write observer recorded the two actual
  transfer writes at opcode PC `$C1FC`, bytes `73b420`, `IBM_TII`:
  `$20B4->$2040=03` and `$20B5->$2041=03`, with physical opcode PC `$0DC1FC`
  and destination `$1F0040/$1F0041` under MPR1 `$F8`. The final RAM snapshot
  changed `$2040/$2041` from `02/03` to `03/03`. A save-state reader interprets
  these as coordinates `(2,3)->(3,3)`, but the fields' meaning is not yet
  source-bound.
- This capture corrected the earlier `$C1FA` opcode-PC assumption. The specific
  TII observer now keys on `$C1FC`; generic position writes also record code
  bytes through the PCE Fast `FastPageR` mapping. Direct zero-page observations
  are gated on MPR1 `$F8` to keep physical-address attribution valid.
- Patch-only validation and a full Mednafen build passed on TRV2 with `-j1`.
  The focused live-capture test passed three loops. The runtime capture remains
  `BLOCKED` for cold CD IRQ/authenticated CD-to-RAM receipts because it loads a
  save state. Active-map identity, collision evidence, Firestaff runtime
  comparison, and Theron movement parity remain open; see the source-lock note.
- A follow-up build with the corrected `$C1FC` TII gate recorded two
  `tii_candidate_state_transfer` rows with source `$1F00B4/$1F00B5`, source value `$03`,
  and matching destinations `$1F0040/$1F0041`. The hooks cover their explicit
  `WrMem()` and selected zero-page paths, not every possible physical RAM
  writer; this bounded capture does not establish an exhaustive write census.
- Do not change movement mechanics based only on this trace: it proves the
  original accepted UP+II command and changed candidate bytes, but not the
  bytes' gameplay meaning or active map. The native mapper implements the
  manual's Button II+direction contract, and authentic-data native `--debug`
  coverage for Button II+Up and Button II+Down now passes three loops. Bind the
  `$2040/$2041` consumer and JP map before claiming the emulator bytes represent
  coordinates or asserting full original movement parity.


- Follow-up capture: an authentic cold JP RUN hold (`run@480:300`) on PCE
  input index 4 reached the System Card poll at `$E4C5`, but no game-owned poll,
  CD IRQ, authenticated CD-to-RAM receipt, or `$E009` dispatch followed. This
  does not bind the observed `$42AB/$42B2` game scan to the cold launch and
  does not identify an active map. See
  `docs/source-lock/theron-disassembly/theron-jp-index4-cold-run-trace-20261030.md`.

## 2026-10-30 — retained BaseRAM snapshots do not identify the active JP map

- Reprojected hash-verified JP Rev. 1 Track 02 raw sectors to 2,048-byte user
  data by dropping each 16-byte sector header, then decoded Akutuba's four map
  arrays using `theron_v1_track02_dungeon_map_load_for_variant()` and the
  source offsets in `theron_v1_track02_dungeon_map.c`. This reproduces the
  discriminating source cells locked by
  `tests/test_theron_v1_track02_dungeon_map.c`: map 0 has `0x20` at `(2,3)`;
  map 2 has `0x00` there.
- Scanned all 106 retained, exactly 8 KiB `pce-fast.ram` snapshots against
  each map's exact contiguous tile bytes. There were no matches. Source map
  dimensions and SHA-256 values were: map 0, 6x8,
  `a7e0707986d898ecd397d92d9312d87f88e1acab8ced97bfaa1b7b0c34dc4b20`; map 1,
  19x13, `d09fcb98e731e6bb1e5ad62e523637c898ec9d0784d1cbf72672ba04c528837a`;
  map 2, 19x17,
  `12f2745d5e0baa3cadaca61d41a564df9569efe9d1556dd1b141e4a6e93ec5b2`; map 3,
  13x12, `19a50d0d5e0baa3cadaca61d41a564df9569efe9d1556dd1b141e4a6e93ec5b2`.
- This negative scan covers only contiguous copies in 8 KiB BaseRAM snapshots.
  It does not cover full physical RAM, VRAM, packed/decoded forms, or the live
  consumer, and does not prove that no map was loaded. The active-map identity
  and movement parity remain open. Next capture a confirmed cold JP gameplay
  route and follow authenticated Track 02 reads through their runtime
  destinations to the active selector and a discriminating tile consumer.

## 2026-10-30 — retry combined TIA and RAM-writer capture

- Added an optional bounded PCE Fast TIA trace output to the live-capture
  wrapper. It is disabled by default, requires the PCE Fast hook, validates the
  trace header, rejects truncation, and adds row counts to the capture receipt.
- A cold JP Rev. 1 run with authentic Track 02 and System Card 3.0 produced
  32 complete TIA transfers (2,048 byte rows), 2,240 zero-page rows, and 2,176
  writes covering the observed `$1047E0-$10481F` range. The existing TIA and
  authentic Track 02 verifiers passed. The RAM write trace was not truncated.
- The wrapper still ended `BLOCKED`: it observed no IRQ or authenticated
  CD-to-RAM receipt, so the run cannot establish a source-to-TIA provenance
  chain. The partial first attempt was also interrupted with SIGINT and lacked
  the required close-time VRAM snapshot; the corrected retry used SIGTERM and
  emitted the snapshots. The partial captures are retained privately on TRV2.
- The TIA hook is System Card execution evidence only. Game launch, native
  parity, and the complete Theron gameplay route remain unproven.

## 2026-10-30 — join TIA reads to preceding BaseRAM writes

- Added `scripts/verify_theron_tia_ram_source_trace.py` to join every TIA byte
  read to the latest strictly preceding BaseRAM write at the same physical
  address using the shared event sequence. It rejects malformed, reordered,
  truncated, unmapped, missing, or value-mismatched evidence.
- Against the retained authentic-media capture, the verifier matched all
  2,048 TIA reads with zero missing predecessors or value mismatches. The
  matching producer PCs are `$42ad` and `$42b4`.
- This proves a live RAM-to-TIA byte flow in the reference System Card run.
  It does not bind those writes to Track 02: the capture has zero raw-sector
  spans, zero CD IRQ callbacks, and zero authenticated CD-to-RAM receipts.
  Gameplay transition and native parity remain open.

- Follow-up review found that the v1 zero-page observer recorded direct
  `HU_Page1` offsets `$02/$03` as logical `$0002/$0003` and derived MPR slot 0.
  `HU_Page1` is the MPR1 window, so those rows do not prove the physical
  addresses previously reported. The historical row counts and instruction
  PCs remain observations, but discard the v1 zero-page MPR/physical mappings
  and the old zero-page verifier pass. The independent `tia_byte` mappings
  above are unaffected. The v2 observer now records `$2002/$2003`, an explicit
  zero-page offset, and derives the physical address from MPR1; a new authentic
  capture is still required to restore that evidence.

## 2026-10-30 — trace TIA byte transfers at `$468C`

- Added a bounded, reference-only PCE Fast observer for the authentic `TIA` instruction at `$468C`. It records decoded source, destination, encoded length, each read value and write address, MPR mappings, and shared event sequence numbers.
- A cold Japanese Rev. 1 capture recorded 32 complete transfers of the same 64-byte source range (`$47E0-$481F`) to alternating logical destinations `$0002/$0003`: 2,048 byte rows in total. The trace verifier passed with no truncation or incomplete transfer.
- This remains emulator evidence. The run still lacks a game IRQ, authenticated CD-to-RAM receipt, game-owned controller poll, and gameplay transition. Continue binding the transfer to its source provenance and eventual game consumers; do not treat the trace as native parity.

## 2026-10-30 — observe TIA zero-page transfers

- The original v1 observer recorded 2,240 reads and writes of offsets $02/$03 while the authentic JP System Card routine executed in $4600-$46FF. Later review found its physical mapping was wrong: direct `HU_Page1` accesses use MPR1, but v1 labeled the offsets as logical $0002/$0003 and selected MPR slot 0. Do not use those v1 rows or their verifier pass as physical-address evidence.
- The v2 trace encodes the corresponding bus addresses as $2002/$2003, retains the zero-page offsets explicitly, and resolves physical addresses through MPR1. The authentic v5 cold capture below validates this mapping.
- This remains reference trace evidence, not game-level semantics or native parity. The run has no game IRQ, authenticated CD-to-RAM receipt, game-owned input poll, or transition into gameplay. Continue tracing authentic source bytes through game-owned consumers.

## 2026-10-30 — follow the JP cold Run path into the next 256 instructions

- Added a bounded PCE Fast code-follow sidecar that starts at the authentic
  System Card fetch of `$6148`, records 256 instruction fetches with bytes and
  MPR mappings, and emits completion only when the next instruction-fetch
  hook confirms execution advanced beyond the 256th row.
- On authentic Japanese Rev. 1 media and System Card 3.0, the cold
  `run@480:300` capture produced 256 rows and the terminal completion record.
  `verify_theron_jp_code_follow_trace.py` reported `status=complete rows=256`.
  The same run produced 32 complete TIA transfers (2,048 byte rows) and 128
  v2 zero-page rows; the TIA verifier passed and confirmed the corrected MPR1
  mapping. In particular, direct `$02/$03` accesses carry the explicit
  `space=zp` tag, are represented in the `$2002/$2003` window, and use
  `mpr_slot=1` with MPR1=`$F8`.
- Scripted input was consumed only by the System Card poll (`$E4C5`); there
  were no non-System-Card polls. The transition receipt reports zero CD IRQ
  callbacks, zero authenticated CD-to-RAM receipts, and no gameplay transition.
  This is reference-only cold-loader evidence and does not establish a native
  handoff or gameplay parity.
- Raw capture files remain private on TRV2 under
  `/home/trv2/firestaff-theron-evidence/capture/theron-jp-code-follow-v6-cold-20261030/`.
  Trace SHA-256: code-follow `32dc1fde9e78dd6f3c21a61a5a7f5117d80fb2393f1bab26ec2643773048e1d4`,
  TIA `98d75b1f9ef1771a42d8fb661be0f19b8ecb24b9a77b5349df887cbae1bcefe7`,
  and transition receipt `5e53a6a9243d2556fba3414b7dc48cd6286a70b5446a849b87c750a493e94bc0`.

## 2026-10-30 — correlate the `$410D,X` dispatch bytes with disc reads

- Added `verify_theron_jp_indirect_jump_source_chain.py`. It reuses the strict
  JP Track 02 source and writer-PC validators, validates the complete code-follow
  window and indexed dispatch, then requires the latest traced System Card
  writer-PC store before each consumer to match an immediately preceding
  source-bound CD read.
- Against the authentic Japanese Rev. 1 capture, three consecutive runs matched
  the two `$4131/$4132` table bytes and the three bytes fetched at `$4319` to
  Track 02 offsets `2,876,817–2,876,818` and `2,877,305–2,877,307`. Each
  source-valid `$EA99` read is followed by a byte-equal `$EA9C` write in the
  shared event sequence, and each write is the latest `$EA9C` record before the
  dispatch/fetch. Other guest-PC writes are not included in that trace, so this
  is not exclusive memory provenance. The aggregate authenticated CD-to-RAM
  receipt remains zero and game-owned execution is still unproven. See the
  source-lock note for exact event IDs and hashes.

## 2026-10-30 — correlate the `$410D,X` dispatch with the next fetch

- Added an optional reference-only opcode `$7C` observer for PCE Fast at the
  two-byte table reads executed by the `$40E1` System Card code. It records the
  values returned by the existing `RdMem` calls, logical and physical table
  addresses, MPR state, resolved target, and shared event sequence; it makes no
  additional guest-memory reads. Its verifier joins dispatch events to the
  preceding `$40E1` fetch and the next recorded instruction fetch inside the
  bounded `$6148` code-follow window. Rows outside that window remain explicit
  but are not promoted to code-follow matches.
- A cold `run@480:300` capture on authentic JP Rev. 1 media and System Card 3.0
  recorded 40 indexed dispatch rows. One row fell within the 256-instruction
  follow window and matched on three consecutive verifier runs: event 102677
  at `$40E1`, base `$410D`, X=`$24`, effective table address `$4131` mapped by
  MPR2=`$80` to `$100131/$100132`, bytes `$19/$43`, resolved target `$4319`,
  followed by fetch event 102678 at `$4319` / physical `$100319`. The other 39
  dispatch rows occurred outside the bounded code-follow window and were not
  correlated to a later fetch by this verifier.
- The separate offline source-chain verifier authenticates the corresponding
  five Track 02 source bytes and matches them to the captured dispatch/fetch
  values and adjacent `$EA99` read / `$EA9C` write events 31301/31302 and
  31303/31304 to table writes at physical `$100131/$100132`, then events
  32277/32278, 32279/32280, and 32281/32282 to the three bytes fetched from
  `$100319-$10031B`. Every source read matches the authentic raw Track 02 byte;
  each write is the latest traced `$EA9C` store at that physical address before
  its dispatch or fetch. This writer trace does not cover writes from all guest
  PCs, so the chain does not establish exclusive memory provenance. The evidence
  does not complete the loader or establish a game-owned input poll or gameplay
  parity. The aggregate receipt
  still reports zero CD IRQ callbacks and zero authenticated CD-to-RAM receipts;
  scripted input remains at the System Card poll. See the source-lock note for
  trace hashes and capture limits.

- A targeted v3 PCE Fast capture now records cold code fetches at `$40D9` and
  `$6138/$613A/$613D/$613F/$6142/$6144` without a code-fetch overflow marker.
  Authentic Track 02 reads and guest writes back the 37 observed `$40D9` rows
  at `$1000D9-$1000DB`; this remains loader/firmware provenance, not a game
  handoff. A v4 replay with larger input and source/writer limits later
  established all scheduled events at the System Card poll and authenticated
  the observed Track 02 reads; its separate main-RAM consumer trace still hit
  its sample limit. Bind the fetched bytes to a game-owned consumer before
  enabling further Theron gameplay behavior. See
  `docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md`.

## 2026-10-30 — v4 input and consumer evidence remains firmware bounded

- The v4 cold replay removed the input-trace cap as a blocker: all 11 scheduled
  events were followed by a System Card poll at `$E4C5`. It still observed no
  non-System-Card poll, CD IRQ, authenticated CD-to-RAM receipt, `$E009`
  dispatch, or gameplay transition. Treat `$42AB/$42B2` and the extended
  writer fetches as candidate boundaries until their caller and ownership are
  source-bound.
- The source-read verifier matched 483,328 authentic Track 02 reads with no
  unbound reads; the `$EA9C` writer rows had ordered value candidates. The
  main-RAM consumer trace hit its independent sample limit, and three bytes
  differ from the latest observed writer in the broader fetch comparison.
- Next, identify the caller and ownership of the candidate PCs and follow a
  source-authenticated transfer to a game-owned poll and visible gameplay
  state. Do not infer gameplay parity from the BIOS poll or loader bytes. See
  `docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md`.

## Continue from the v17 instruction-start snapshot check

The v17 source-join verifier now requires explicit shared-sequence provenance
and immediate source-read/CD-RAM-write adjacency; its 11 authentic JP bytes
match the latest observed values at two instruction-start snapshots. This does
not show the actual opcode bus fetch, complete loader provenance, or a game
handoff. Continue tracing from the System Card/bootstrap boundary until an
authenticated transfer reaches game-owned code and input polling, then compare
that route against native Firestaff. Preserve this evidence boundary in
`docs/source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md`.
