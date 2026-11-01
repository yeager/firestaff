# Theron's Quest Track 02 Internals

> **Current source-map note (2026-11-01):** The authenticated JP Akutuba
> descriptor and four tile arrays are located in raw Track 02 sector 308
> (relative LBA 84, disc LBA 3674). The identity-checked
> [`--dump-grids` planner](../../tools/plan_theron_jp_akutuba_map_trace.py)
> prints descriptor values and opaque bytes from all four arrays. This
> establishes static source layout only: unknown descriptor fields, tile
> semantics, runtime map selection, and the pose-to-render path remain open.
> An offline scan of an authentic JP `.mc0` state found this complete source
> span at inflated offset `0x6633b` (state SHA-256
> `b4f2a06788e4723110b19892f93e8ac4aef1b5c18c6358a9fe36f1baf747b068`). All
> four maps are present, so the active map remains unknown. The source-locked
> [Mednafen 1.32.1 state inspector](../../tools/inspect_theron_mednafen_pce_fast_state.py)
> maps the match to HuC CD-RAM page `$74` (physical `$0E891D`); the saved MPRs
> do not select that page. The state embeds a preview and named
> VCE/VDC0/VRAM/SAT snapshot variables, but no pose-bound tile consumer is
> established.
> See the [source report](../source-lock/theron-disassembly/theron-jp-akutuba-map-source-sector-20261101.md)
> and [capture readiness](../theron/THERON_CAPTURE_READINESS.md).

> Current runtime status and open evidence joins are in
> [Theron capture readiness](../theron/THERON_CAPTURE_READINESS.md) and
> [Theron TODO](../../TODO-theron.md). Track 02 byte layouts and historical
> capture notes below are not claims of semantic dungeon or gameplay parity.

An authentic US standard-PCE save-state comparison maps all four Akutuba map
arrays to the `HuC.SysCardRAM` bytes selected by saved MPR3. The report records
the source offsets, dimensions and hashes; active-map selection and a
pose-bound tile consumer remain open. See the [US state-map source lock](../source-lock/theron-us-authentic-standard-pce-state-map-binding-20261101.md).

## Authenticated startup handoff (2026-08-08)

**Native status (2026-11-01):** Firestaff's bounded JP fresh-start and US
Continue routes consume authentic Track 02 data in the native C/SDL runtime.
Continue reaches dungeon 2 with its authentic Ak-Tu-Ba-complete BRAM; the JP
fresh start reaches a source-verified loaded dungeon. These routes do not
prove the original game-owned cold handoff or complete gameplay parity. The
current evidence boundary and next joins are summarized in
[`THERON_CAPTURE_READINESS.md`](../theron/THERON_CAPTURE_READINESS.md) and
[`TODO-theron.md`](../../TODO-theron.md).

**Reference trace boundary (2026-10-31):** an isolated authentic JP save-state
run verifies that the corrected PCE Fast instrumentation gate admits 4,096
bounded VRAM commit records after the scripted guest read at `$44C1` in shared
event order. The wrapper reports `transition=missing`; the state-only capture
does not join a cold CD handoff, decode VRAM semantics, prove a visible input
effect, or establish native parity. See the
[source-locked trace report](../source-lock/theron-disassembly/theron-jp-pce-fast-input-gated-vram-trace-20261031.md).

**JP candidate map-index disassembly (2026-11-01):** the authentic state
contains a unique code window at `$50D2`; a bounded trace observes reads from
a candidate four-list structure at the end of the unclassified `thing_list`
region. Its role, active-map relation and tile consumer are not established;
no rendered result is established.
See the [disassembly report](../source-lock/theron-disassembly/theron-jp-akutuba-map-index-reader-20261101.md).

**JP map-array reads (2026-11-01):** three authentic state-only captures
recorded 1,677, 1,513 and 1,556 reads from the parser-established arrays.
They touched 17 source-matched byte addresses in Map 0 and Map 3 without
reaching the 4096-row cap. Static raw-BIN windows decode the reader PCs as
three `LDA ($14),Y` accesses and one `ADC $6DF6,X`; the earlier captures lack
same-event registers and fetched opcode bytes. The compiled v4 hook records
opcode/register context, page-1 operands and MPRs, but has not yet produced a
gameplay trace. This does not establish active-map selection or rendering. See
the [capture report](../source-lock/theron-disassembly/theron-jp-state-only-map-array-read-capture-20261101.md)
and [tool note](../source-lock/theron-disassembly/theron-jp-map-array-read-trace-tool-20261101.md).

**Matched JP input/data-read update (2026-11-01):** a verified state-only UP
replay read the scripted mask at `$44C1`; its matched no-input control and UP
trace each retained 256 identical data-read rows, all matching authentic JP
Track 02 bytes. The reads are before the map arrays and no movement-command
write was observed. This does not identify the reader's data, the active map,
player movement, or a rendered result. Both traces report
`transition=missing`. See the [matched trace report](../source-lock/theron-disassembly/theron-jp-state-only-up-control-data-read-20261101.md).

Native direct launch enters the Theron title/startup path without Firestaff's
Run menu or a System Card. The Debug boot-probe receipt remained at
`levelLoaded=0`; it does not establish Track 02 dungeon loading or gameplay.
See the [direct-launch evidence note](../source-lock/theron-native-debug-direct-launch-20261031.md).

Firestaff verifies the US CUE/Track 02 and System Card boundaries, then records
the original media reads required for startup. This proves the startup/media
transport only; it does not promote the Track 02 payload to semantic level,
object, tile, palette, HUD, creature, T700 or T900 state.

**Track 02 source records (2026-11-01):** hash-verified US and JP full-ISO
projections now admit the same eight creature-pointer and five regular-spawn
zone records as their raw BINs. Native ISO dungeon startup binds these source
records. In both regions, pointer records 0–4 contain offsets `$0058`,
`$00D7`, `$0102`, `$0129`, and `$0150`, which resolve in order to the five
zone records relative to the authenticated regional table base; records 5–7
have zero offsets. This static source-table relationship does not establish
runtime pointer selection, JP category semantics, a spawn/RNG consumer,
creature generation, or combat. See the
[ISO projection source lock](../source-lock/theron-track02-spawn-record-iso-projection-20261101.md).

**Native debug receipt (2026-11-01):** three native JP and three native US
runtime loops emitted the bounded `--debug` receipt from authentic Track 02.
Each run reached a loaded level and every reported header, offset, pointer,
zone field and static link matched the independently verified raw BIN
projection. This does not prove original runtime selection or creature
spawning. The per-run verification is documented in the
[debug receipt note](../source-lock/theron-track02-spawn-debug-receipt-20261101.md).

**Capture-tooling update (2026-11-01):** the reference wrapper supports an
opt-in System Card `run-path` PC filter for `$EA4D`, `$EA50`, `$E162` and
callback `$2259`. Instrumenter, streaming verifier and wrapper regressions
passed three loops. A later authentic US cold capture used this filter, but
the wrapper rejected the package because trace caps were reached and live
`$45C3` windows failed source validation. This adds no accepted handoff or
gameplay evidence. See the [ordered call-window report](../source-lock/theron-disassembly/theron-us-c852-runtime-call-capture-20261101.md)
and [capture readiness](../theron/THERON_CAPTURE_READINESS.md).

An additional opt-in PCE Fast `drator-row` filter retains only the observed US
consumer reads at `$CBC9`/`$0DABC9` and `$45C3`/`$0D05C3` for physical row bytes
`$1F093B-$1F093D`. Its address-only receipt is parser-checked and does not
identify row fields or semantics. Three focused tooling loops passed; a later
authentic US cold capture used this filter but was rejected because trace caps
were reached and live `$45C3` windows failed source validation. The capture
adds no accepted handoff or gameplay evidence. See the
[ordered call-window report](../source-lock/theron-disassembly/theron-us-c852-runtime-call-capture-20261101.md).

The source-sector planner maps JP Akutuba's descriptor and maps to disc LBA
3674. The reference capture wrapper now accepts
`THERON_CAPTURE_PCE_FAST_EXACT_SOURCE_LBA=3674` and gates CD data-port rows and
FIFO-origin RAM receipts before their quotas. It leaves the full FIFO origin
sequence intact. This is an instrumentation filter, not a source-to-consumer
proof; no accepted authentic capture has exercised it yet. See the
[exact-LBA tool note](../source-lock/theron-jp-akutuba-exact-lba-capture-tool-20261101.md).

**Current reference boundary (2026-10-31):** a separate authentic Japanese
cold System Card RUN trace follows the `$1800=$C8`, `$1801=$04`, `$2259=$04`
callback through BIOS branch `$E166->$E176`. It does not observe a confirmed
game-owned handoff or gameplay and does not change the transport-only claim
above. See the [source-locked trace supplement](../source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md#2026-10-31-r5-callback-and-transfer-boundary).

The JP PCE Fast UP/neutral/DOWN follow-up used an authentic F5 save facing a
wall. Each reference screenshot retained the same front wall, and the bounded
observer saw no `$C1FC` TII candidate or `$2040/$2041` position write. The
separate input-to-`$2912` event ordering remains noncausal and does not show
movement. Do not infer Track 02 tile semantics from this blocked-wall result;
a reachable authentic pose and same-session position/frame join are still
needed. See the [source-locked movement boundary](../source-lock/theron-disassembly/theron-jp-up-input-latch-blocked-wall-probe-20261031.md).

Later matched state-only runs show repeatable Button II+UP-conditioned TII
transfers from `$20B4` to candidate `$2040/$2041` bytes, absent in Button II-only
controls. The same `$C1FC` TII is associated in the source-locked analysis with
copying teleporter destination coordinates. A follow-up three-run trace records
the same 128-instruction path through `$C1D6` and into `$C1FC`, including a
call/return and conditional fall-through. It does not source-bind the tested
fields, establish gameplay purpose, or identify an active-map object, so it
does not prove that the transfer is player movement. Coordinate semantics,
collision behavior, rendered change and cold media provenance also remain
unresolved. See the [comparison bundle](../../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md)
and [control-flow capture](../source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md).

## Why the Disc Is Not an ISO Filesystem

PC Engine System Card software works with record numbers relative to a CD base.
Official Hu7 production places record constants in the executable; the finished
disc has no runtime central directory for a game to enumerate. Firestaff uses
strict CUE parsing, raw-sector layout validation, known hashes, and traced
record transitions rather than filename inference.

## IPL and Stage Two

For verified raw JP/US Track 02 media, the IPL executes record `0x0003e7` into
local RAM `$4000`. The subsequent observed CD_READ loads a single sector into
`$3800`: record `0x04df` in JP and `0x04e0` in US. The receipt records variant,
record, raw sector, MODE1 user-data offset/size, content hash, destination, and
live record-register provenance.

The initial `$3800` payload begins `BRK $ff` followed by `$0308` and a bounded
218-entry envelope. `BRK $ff` is control transfer through the active HuC6280
IRQ2 protocol; the subsequent six-byte entries are not executable code merely
because they share the loaded sector.

## Stage Three Boundary

The stage-two runtime handoff accepts the exact traced record and structural
manifest receipt, then rejects a mismatched or mutated record before startup
state can advance. This is intentionally transport evidence only. It does not
name a manifest entry as a level, object, bitmap, palette, sound, or script.

The current authenticated Main-RAM replay remains init-only: `$2600-$27FF`
contains 512 zero-valued `$CB22` reads and no `$C3A0` reader. Firestaff keeps
parser admission separate from the `$2c54-$2c69` execution-window receipt, so
this transport evidence does not open level/object rendering semantics.

## Startup and Route Transactions

M12 carries strict CUE/Track02 provenance into M11. Startup title, stage,
Soul Room, forcefield, and Continue routes require all of their original bitmap
and level receipts. Route assembly is candidate-first: a level/object route is
committed only after every required data receipt validates. An incomplete route
leaves the old runtime state unchanged and cannot fall through to a synthetic
surface.

## Startup Level Envelope

The authenticated raw JP and US startup payloads agree on the complete
12-byte envelope prefix: dimensions, seed, level index, and an opaque
two-byte extension `0x0103` at offsets 10-11. Firestaff fingerprints that
extension with the CD-record boundary receipt, but assigns it no gameplay
meaning. In particular, it is not treated as a start pose, object count, flag,
or command; the following `0x380` user-data tail remains unparsed.

`theron_v1_track02_decode_initial_level_envelope()` is the real-media decode
surface for that one payload. It requires the known raw JP/US hash, IPL INDEX
01 coordinate, descriptor-relative candidate, and MODE1 user-data copy before
it promotes the header fields and the exact `0x360`-byte grid span. The grid is
still byte-faithful and unclassified: no tile meanings, objects, or visuals
are inferred. Its receipt explicitly blocks fallback visuals and keeps both
the extension and tail unpromoted.

Each of the seven later-level spans now also has a source-locked resource
frame. The retail `$23AD` routine reads the little-endian word at resource
offset `+2`, treats `word - 5` as the remaining bitstream byte count, and
starts reading after the six-byte header. Firestaff verifies this framing for
all seven US and all seven JP blocks and retains the exact header, bounded
bitstream slice, and user-data end offset in the runtime receipt. The first
two header bytes are deliberately still opaque:
the static caller has not proven them to be a destination address or level
identifier, and no map/tile/palette semantics are promoted from that shape.

The static HuC6280 lift is now byte-complete through `$252A`. The C boundary
`theron_v1_huc6280_decode_resource()` follows the retail variable-width reader,
`$0100` width markers, pointer-table backreferences, low-byte copy, and the
high-byte `TII` continuation. It requires caller-owned source/destination
windows and pointer entries, so it cannot silently invent MPR mappings. The
authentic resource bytes can therefore be decoded as bytes when a matching
runtime bank snapshot is supplied, but no decoded output is promoted to map,
tile, bitmap, palette, or object semantics until the stage-2 MPR and post-CD
consumer join is captured.

The capture build now includes the post-patch `main_ram_loader_write` trace as
well. That hook records game-owned byte writes together with logical and
MPR-derived physical destinations and the writer PC, which is required for
the pending source/destination join. Its `dispatch_sequence=unbound` marker
keeps the E009/CD-sector ownership boundary explicit. It is provenance only;
it does not turn those writes into semantic game data.

The longer authenticated replay also reaches the source-owned `$CC4C` path and
its `$4644` preconsumer / `$4667` helper windows. The execution-window parser
accepts this as provenance even when the bounded run does not visit `$C96B`;
the strict spawn-admission parser still requires both windows, a valid
`$B0E5` category, and the later source-owned writes. In the current replay all
`$4667` samples have `$B3=$FF`, so the `$B3 & 7 == 4` RAM-loaded branch is not
observed. No RNG, spawn, AI, T700 or T900 semantics are promoted from this
window.

## Stage-2 bank and destination contract

The authenticated US and JP retail projections also contain the same generic
resource-command handler at HuC6280 `$4C3F` (162 bytes, FNV-1a
`46360d97`). Its source order fills four MPR values at `$3b7e-$3b81` from a
selected four-byte table row, runs the source-owned variable-bit path, then
hands the result to the copy helper with source `$3002/$3003 = $6000`,
destination `$3004/$3005` from the command record, and produced length
`$3006/$3007`.

This is static proof of the bank-table and destination-register contract. It
is still a generic command handler: the level-specific table row, command
index, source record and executing-PC/source-LBA join have not been captured.
The seven real level blocks therefore remain framed opaque inputs; they are
not promoted to map, object, tile, bitmap or palette data. See
[`tqr_v1_stage2_resource_handler_disassembly_2026-08-06.md`](../source-lock/tqr_v1_stage2_resource_handler_disassembly_2026-08-06.md).

## Multi-Level Object Table

`theron_v1_track02_read_object_table()` now bounds decoded records against the
real per-dungeon level count (`0..THERON_MAX_LEVELS_PER_DUNGEON-1`), replacing
the earlier level-0/32x27-only assumption.
`theron_v1_track02_decode_initial_level_object_table()` accepts records for
any bounded level, and the new
`theron_v1_track02_decode_dungeon_level_object_table()` extracts a single
level's object records from a full-dungeon buffer. Two additional object
kinds carry decoded state: `THERON_OBJTYPE_SOUND` (default
`THERON_SOUND_AMBIENT_1` for the movement code) and `THERON_OBJTYPE_PIT`
(pit records own their grid position).
`theron_v1_world_apply_track02_object_table_for_dungeon()` routes decoded
records to every loaded level of a dungeon. Track 02 establishes a
stairs-class tile but does not yet bind direction, target level or target
coordinates to an original consumer; authenticated stairs are therefore
blocked rather than using the former host `+/-1`/same-cell rule.
`theron_v1_transition_execute()` retains fixture and separately proven
teleporter/quest-complete mechanics, but must not promote an unbound stairs
route. This remains real Track 02 data decode, not synthesized object
placement.

## SRM Boundary

Save Disk candidates require one valid gzip member with bounded header parsing,
CRC32, and ISIZE validation. Unknown original bodies can be catalogued but are
not restored until their structure is independently correlated. Firestaff-native
SRM export uses no-replace atomic publication so an original artifact is not
overwritten by a test save.

Continue carries the accepted media route mask, checksum, and selected level
bank in its receipt. Restore is atomic: bad media, bad gzip, or an unbound body
cannot partly mutate the live world.

## Verification

```bash
FIRESTAFF_THERON_TRACK02_US_BIN=/path/to/us.bin \
FIRESTAFF_THERON_TRACK02_JP_BIN=/path/to/jp.bin \
  ./build/firestaff_theron_v1_track02_ipl_loader_probe

./build/firestaff_theron_v1_track02_level_handoff_probe
./build/test_theron_v1_startup_save_resume_pc34
```

The next valid semantic step must come from an original loader read, dispatch,
or layout correlation for a later record. Repeated bytes, descriptor adjacency,
and guessed PC Engine data formats are not sufficient evidence.
