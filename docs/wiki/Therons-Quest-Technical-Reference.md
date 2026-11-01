# Theron's Quest Technical Reference

> Current evidence overview: [Theron capture readiness](../theron/THERON_CAPTURE_READINESS.md).
> This page is a technical reference; dated capture notes retain their
> historical scope and do not supersede the current project-status page.

> **Status reviewed 2026-11-01.** Firestaff runs Theron's Quest in native
> C/SDL using authentic PC Engine CD Track 02 data through bounded Japanese
> and US routes. The Japanese fresh-start route reaches a loaded dungeon; the
> US authentic BRAM Continue route reaches dungeon 2 and has bounded native
> movement and inventory transactions. Source-only loaders cover all seven
> dungeons in both regions, but do not establish their retail runtime
> semantics. Native M11 rejects the authentic empty JP BRAM as Continue;
> positive JP progress restoration remains unverified because no authentic
> progressed JP BRAM is available. A cold Japanese emulator route visibly
> reaches the Akutuba dungeon after a four-second System Card RUN hold. A
> separate authentic JP capture confirms that four-second Button I holds on
> NEW GAME and FILE 1 advance into the story. Its `.mc0` snapshot is an
> emulator state rather than BRAM; the capture does not join input to an
> authenticated CD-to-RAM transition or establish native parity. A separate US save-state replay
> records mouse reads and a candidate command consumer, but has no runtime CD
> provenance and proves no semantic action, movement, pose, or viewport
> change. Original presentation, later campaign progression, broader save
> behavior, and full gameplay remain evidence-gated. A 2026-10-31 authentic US
> Drator follow-up joins generated-row writes and reads for offsets 6–8 and a
> later ten-byte TII copy to a work buffer. The first five 512-byte runtime
> `$C852` windows exactly match US Track 02 raw offset `$A3AD2`. The runtime
> source is mapped, while direct generated-row consumer attribution and
> downstream row semantics remain open; the associated BIOS trace was
> explicitly truncated and fails closed. See [capture readiness](../theron/THERON_CAPTURE_READINESS.md).
> Three JP PCE Fast
> reference UP runs bind scripted `up@61:120` to the same-frame guest read at
> `$44C1` and the shared event order through `$D334` read, `$D349` write, and
> `$D334` reread of `$2912`, absent in three neutral controls. The order does
> not prove causality; the byte's meaning and game effect remain unidentified.
> A cold JP emulator capture also reached a
> first-person dungeon view, but lacks a game-owned input/loader join and a
> matched Firestaff frame. The native Firestaff snapshot is nonblack but has no
> paired retail frame. Capture tools are research instruments only and are
> never a Firestaff runtime dependency. See the
> [current platform status](../PLATFORM_STATUS.md#therons-quest),
> [capture readiness](../theron/THERON_CAPTURE_READINESS.md) and
> [input notes](../theron/theron_input.md). A cold System Card RUN capture
> binds the `$1800=$C8` / `$1801=$04` callback to `$2259=$04` and branch
> `$E166->$E176`; no confirmed game-owned handoff was observed. See the
> [dated source-lock supplement](../source-lock/theron-disassembly/theron-jp-system-card-run-path-20261030.md#2026-10-31-r5-callback-and-transfer-boundary).

> **Drator capture tooling (2026-11-01):** an opt-in PCE Fast filter now
> records only the observed US reads of generated-row offsets 6–8 at logical
> PCs `$CBC9` and `$45C3` with physical data `$1F093B-$1F093D`. The parser
> validates the versioned address receipt but assigns no row semantics. Its
> focused tooling checks passed three loops; no new authentic run used it, and
> the previous BIOS-truncated capture remains rejected. See [capture readiness](../theron/THERON_CAPTURE_READINESS.md).

> **JP Akutuba source maps (2026-11-01):** the identity-checked planner maps
> the 68-byte descriptor and four opaque tile arrays to raw MODE1 sector 308
> (Track 02 LBA 84, disc LBA 3674). Its `--dump-grids` option prints the
> authenticated descriptor values and raw bytes in coordinate order. The
> fields named `unknown_1` and `unknown_2`, tile meanings, active-map selector,
> and party-coordinate mapping remain unresolved. An offline scan found the
> complete descriptor-and-map source span in an authentic JP `.mc0` state, but
> all four arrays are resident and the inflated file offset is not mapped to
> guest RAM/MPR or a consumer. This is a source-analysis aid only; it does not
> authorize runtime rendering. See the [source-sector
> report](../source-lock/theron-disassembly/theron-jp-akutuba-map-source-sector-20261101.md)
> and [capture acceptance chain](../../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain).

> **Matched PCE Fast control-flow trace (2026-10-31):** three state-only runs
> record the same 128-instruction path into `$C1FC`, including `$C1D6`
> call/return and conditional fall-through. Since `$C1FC` is also associated
> with Track 02 teleporter destination copying, the repeated context does not
> prove player movement or identify the active-map input object. See the
> [control-flow report](../source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md).

> **Native direct launch (2026-10-31):** Firestaff enters Theron's native
> title/startup path without its Run menu or a System Card. The US Debug
> boot-probe receipt remains at `levelLoaded=0`; this does not add a dungeon
> or gameplay claim. See the [direct-launch evidence note](../source-lock/theron-native-debug-direct-launch-20261031.md).

> **JP movement capture limit (2026-10-31):** A same-state UP, neutral and
> DOWN reference check faces a wall in all three captures. The bounded
> position observer sees neither a `$C1FC` TII candidate nor writes through
> `$2040/$2041`; the separate `$2912` input/RAM event ordering is not causal
> proof. This is not successful movement or rendering evidence. See the
> [blocked-wall report](../source-lock/theron-disassembly/theron-jp-up-input-latch-blocked-wall-probe-20261031.md).

> **Matched JP state-only comparison (2026-10-31):** Three Button II+UP runs
> repeat TII transfers into candidate `$2040/$2041` bytes after guest reads;
> three Button II-only controls retain the prior values. This is a
> repeatable input-conditioned state transfer, not yet verified coordinates,
> passability, visible movement, cold CD handoff or parity. See the
> [comparison bundle](../../parity-evidence/verification/theron_v1_jp_state_movement_input_compare_20261031/README.md).

> **TII control-flow boundary (2026-10-31):** three state-only replays now
> record an identical 128-instruction context into `$C1FC`, including a
> `$C1D6` call/return and conditional fall-through. This bounds the dynamic
> instruction path but does not source-bind the tested fields, establish the
> routine's gameplay purpose, or identify the active-map object. Since the
> same TII is associated with teleporter destination copying, its transfer
> cannot yet be assigned to player movement. See the
> [control-flow capture](../source-lock/theron-disassembly/theron-pce-fast-tii-control-flow-20261031.md).

> **Input-gated VRAM trace (2026-10-31):** a corrected PCE Fast callback
> gates the bounded VRAM trace on the guest-visible controller value. An
> authentic JP state-only run records the scripted `$44C1` read at shared
> event 58758, followed by 4,096 contiguous commit rows beginning at event
> 58761. The run reports `transition=missing`; it verifies the trace gate and
> ordering only. It does not prove input causality, VRAM semantics, visible
> gameplay, cold media handoff, or parity. See the [source-locked report](../source-lock/theron-disassembly/theron-jp-pce-fast-input-gated-vram-trace-20261031.md).

> **Track 02 spawn source records (2026-11-01):** authenticated US and JP full
> ISO projections decode the same creature-pointer and regular-spawn zone
> records as their raw BINs, and native ISO dungeon startup binds those source
> records. In both regions the first five pointer offsets (`$0058`, `$00D7`,
> `$0102`, `$0129`, `$0150`) resolve in order to the five zone records; the
> remaining three offsets are zero. This static source-table join does not
> establish runtime pointer selection, category semantics, RNG, creature
> generation or combat. See the
> [ISO projection source lock](../source-lock/theron-track02-spawn-record-iso-projection-20261101.md).

> **Drator reader source-window verifier (2026-11-01):** the strict verifier
> accepts 512-byte windows at `$CBC6` and `$45C3` built from the hash-verified
> US retail BIN and reports four and three matching raw offsets, respectively.
> This exercises the real-media byte matcher with constructed rows; it is not
> a new emulator capture and does not bind the live fetched instructions to a
> unique runtime LBA/source occurrence. Keep row fields and native spawn
> behavior unassigned. See the [source-map report](../source-lock/theron-disassembly/theron-us-drator-row-reader-source-map-20261101.md)
> and [capture readiness](../theron/THERON_CAPTURE_READINESS.md).

> **System Card trace tooling (2026-11-01):** the reference capture wrapper
> can restrict BIOS I/O/RAM rows to the source-locked run-path PCs `$EA4D`,
> `$EA50`, `$E162`, and callback `$2259`. Focused instrumentation, verifier
> and wrapper checks passed three loops. No original-game capture has used
> this filter; the previous capped BIOS trace remains rejected, and the tool
> does not change runtime support. See [capture readiness](../theron/THERON_CAPTURE_READINESS.md).

## Door movement boundary

Movement does not implicitly open a closed or locked door. Firestaff accepts
a USE command for a facing door, but authentic Track 02 door effects remain
fail-closed because the original T900 button/actuator consumer is not yet
bound. Host-fixture state transitions and key validation do not establish
retail behavior. A stair query likewise returns `STAIRS` only when the
destination level is resident.

## Turn input receipt boundary

Left and right turn inputs share one receipt path. If the bounded
source-actuator event queue cannot reserve the turn's events, the runtime
returns `BLOCKED`, preserves the party pose, and requests no redraw. A focused
full-queue regression covers all four turn bindings. This proves error
reporting and transactional failure handling only; it does not establish
retail actuator timing, queue capacity, or broader turn parity. See the
[input notes](../theron/theron_input.md) and [verified work log](../../DONE-theron.md).

## r30 emulator/capture boundary

The 2026-08-14 r30 state replay retained 65,756 spawn-register rows, 256
`$B0E5` address hits, 2,213 RNG-window samples, 65,536 main-RAM consumer
reads and 4,096 target-write rows. The `$B0E5` register values remained
`A=$2C/$85`, so there were no valid `A=0..3` spawn entries; the same session
had no authenticated CD→RAM receipt and `transition=missing`. It remains
negative overlay evidence and does not open spawn, RNG, AI, combat or record
semantics.

Mednafen also logged a missing optional `palettes/pce.pal` file. The emulator
falls back to its built-in PCE palette; this warning is not evidence of a
Track 02 graphics failure. VDC/VCE captures remain screen-space diagnostics
until a source-owned consumer join is proven.

## r31 scripted replay boundary

The 2026-08-14 r31 replay used the authenticated US Track 02/System Card, the
late Mednafen state and seven scripted PCE inputs. It retained 21,786
spawn-register rows and 12 physical `$B0E5` overlay hits, all with
`A=$2C/$85`; there were no valid `A=0..3` entries, no `$4644/$4667` samples,
no authenticated CD→RAM receipt and `transition=missing`. This is negative
runtime evidence, not a source-owned spawn or level consumer. The capture
sidecars remain local on the external disk.

## JP screen-space replay check

The authenticated JP snapshot pair (`VRAM 8ae1e419`, `VCE 4e48c361`) passes
the production `theron_v1_vram_trace_real_capture` test. This admits only
the captured BAT/tile/palette screen route; JP level/object publication and
the HuC6280 consumer remain fail-closed.

## Historical scripted replay transport boundary (2026-08-13)

This dated authenticated external-disk replay used the US Track 02/System
Card identity and the scripted PCE sequence `run@1:1,run@480:30,i@900:30`.
It proves CD transport and initialization: 240 raw-sector spans, 25 CD IRQ
callbacks, 256 authenticated CD-RAM receipts, 32 game-owned `$E009`
dispatches, and a parser-approved transition receipt.

In that replay, the Main-RAM consumer sidecar was init-only. It had 65,536
reads and 512 reads in `$2600-$27FF`, all zero-valued reads from `$CB22`; it
had no runtime
reads, no `$C3A0` reader, and no dynamic level/object publication. Therefore
this replay does not open square/tile/HUD/T700/T900 semantics or production
dungeon admission. The raw sidecars and instrumented emulator remain local on
the external disk.

## r26 cold-start transport receipt

A fresh cold-start run against the same authenticated US Track 02 and System
Card produced 161 raw-sector spans, 25 CD IRQ callbacks, two authenticated
CD→RAM receipts, 32 game-owned `$E009` dispatches and an observed transition.
The 65,536-row main-RAM sidecar contains 512 reads in `$2600-$27FF`, all zero
valued and owned by the `$CB22` initialization reader. This proves the
same-session transport boundary, but not the dynamic level/object consumer;
runtime `$2600` and `$C3A0` evidence comes only from the separate state-replay
witness below. Capture sidecars and the instrumented binary remain on the
external disk.

## r26 state-replay consumer boundary

An authenticated r26 state replay now has a stronger main-RAM consumer
witness: 65,536 reads, 311 reads in `$2600-$27FF`, 128 non-zero values, 311
runtime reads, 47 `$C3A0-$C429` reads and six distinct reader PCs. The receipt
still fails the `$2c54-$2c69` code-window check and its transition sidecar has
no CD-origin receipts or game-owned `$E009` dispatches. It is therefore
runtime-address evidence only; level/object, square/tile, VRAM/VCE, HUD, T700
and T900 publication remain fail-closed. The exact capture identity and
boundary are recorded in
`docs/source-lock/theron-disassembly/theron-runtime-spawn-capture.md`.

The extended bounded state trace (1,048,576 main-RAM rows) independently
retains 427 runtime `$2600-$27FF` reads, 147 non-zero values and 84
`$C3A0-$C429` reads across six reader PCs. Its sidecar MD5 is
`cae9aa15aef10bc9c88b7de891d34d8e`. The code-window check and same-session
CD-origin join are still absent, so this remains diagnostic runtime-address
evidence and does not authorize semantic publication.

## Runtime record-table witness

An authentic Mednafen savestate execution-window capture now identifies a
mutable ten-byte runtime record table. `$C9BD` derives a table base from
`$6000,X`; `$CB89` scans records at `$611D` in ten-byte steps; and `$CBCC`
copies `$2935–$293E` into `$611D–$6126`. `$CC4C` also reaches the existing
`$4667` helper during this path.

This is runtime consumer evidence, not a level/object claim. The savestate
does not contain a same-session CD-origin receipt, source LBA, or proven role
for the table. The production level/object, VRAM, and HuC6280-RAM gates remain
closed until one authenticated session joins Track 02 bytes, post-CD RAM,
executing PC/bank state, table mutation, and a reproducible gameplay
transaction. Full provenance is recorded in
`docs/source-lock/theron-disassembly/theron-runtime-record-table-consumer-20260814.md`.

The bounded receipt replay emits 4,096 rows. One recurring raw row,
`4080007098a8c8700020`, occurs byte-exactly at seven offsets in authenticated
US `TQUS02.bin`: `0x0b0eed`, `0x0faa0d`, `0x144a03`, `0x18da0d`, `0x1d7b7d`,
`0x2206ed`, and `0x26a85c`. This is raw source-byte overlap only: the
savestate replay has no same-session CD-origin receipt, so no level/object or
gameplay semantics are admitted.

## Track 02 map-directory boundary

The world handoff validates the complete authenticated Track 02 map directory
before replacing a dungeon bank. A zero or oversized map count, or a map whose
`x_dim + 1` / `y_dim + 1` exceeds Firestaff's fixed square grid, is rejected
without changing the previous bank. This protects the source-bound state
boundary; it does not claim the unresolved post-CD level/object consumer.

## Door state boundary

The runtime treats `LOCKED=6` as a sentinel state, not as a later opening
animation frame. The bounded passability range is
`QUARTER_OPEN..DESTROYED`; movement queries, movement mutation and
`door_open()` share that predicate. This prevents a locked door from becoming
passable through numeric state ordering. Source-authenticated key/object
consumption remains capture-gated.

## Teleporter failure boundary

`theron_v1_teleporter_resolve()` is transactional: an absent endpoint, cyclic
chain, unloaded destination or invalid coordinate returns `-1` without
publishing party or transition state. Movement now propagates that failure as
`THERON_MOVE_BLOCKED` and does not apply per-move effects. Only the verified
object-ID and Track 02 coordinate routes may publish a teleport destination;
this guard does not infer new dungeon semantics.

> **Status reviewed 2026-08-13.** The missing-endpoint regression is covered by
> the mechanics-hardening probe. Source-owned level/object consumers remain
> capture-gated.

## Stairs failure boundary

Movement queries and execution require the stairs destination level to be
loaded. If transition setup or execution fails, the result is
`THERON_MOVE_BLOCKED`; no party position, transition state or per-move effect
is published. This preserves the fail-closed boundary while dynamic
source-owned level loading and the original stairs consumer remain unresolved.

## Dungeon exit failure boundary

An exit is actionable only when `dungeon_complete` is true and transition
execution succeeds. Queries and movement both report `THERON_MOVE_BLOCKED`
otherwise, without applying per-move effects. This keeps quest completion and
next-dungeon source consumers behind their existing evidence gates.

## Pit failure boundary

On a source-authenticated level, a pit does not use the host fixture's damage
or stamina rules. Query, movement and the public pit handler remain blocked
until the original T700 pit consumer is joined to the source level/object
receipt. Synthetic fixture levels retain the existing probe-only behavior.

> **Historical status note (2026-08-13).** JP/US Track 02 identity and several loader
> receipts are real-data verified. Game-owned dungeon handoff, object/level
> semantics and bitmap/palette binding remain open. The verified startup
> boundary does not establish the Firestaff `$2600` consumer or gameplay
> semantics.

## Scope

**Current capture boundary (reviewed 2026-10-31):** An isolated authentic JP
cold capture confirms that a four-second Button I hold on NEW GAME opens file
selection and a second four-second hold on FILE 1 enters the Akutuba story.
The later dungeon `.mc0` is an emulator snapshot, not BRAM. This visible result
does not join guest input to a game-owned CD loader or establish native
Firestaff timing/parity. The separate cold System Card RUN trace remains
System Card-only: it records the device callback byte and BIOS branch
`$E166->$E176`, but no confirmed game-owned handoff. See the
[source-locked title-action capture](../source-lock/theron-jp-four-second-title-action-capture-20261031.md),
[capture readiness](../theron/THERON_CAPTURE_READINESS.md), and
[input notes](../theron/theron_input.md). DMWeb provides retail edition and
manual references. Greatstone's Theron material covers a CSBWin
custom-dungeon conversion, not retail PC Engine data.

Theron's Quest is a PC Engine CD target. Firestaff accepts original Track 02
media through a strict CUE/BIN handoff and verifies known media identities. It
does not assume ISO-9660 files or create a directory from a disc image.

## Record-Based CD Layout

System Card access is record-based relative to a CD base. Official Hu7 layouts
compile record constants into the executable, so the final disc has no runtime
file directory to enumerate. Firestaff follows observed loader record handoffs
rather than filename scans.

The verified JP/US Track 02 chain is:

* `CD_EXEC` record `0x0003e7` into local RAM at `$4000`;
* a traced stage-two one-sector `CD_READ` into local RAM at `$3800`;
* record `0x0004df` for JP and `0x0004e0` for US;
* a payload beginning `BRK $ff` with a bounded `$0308` envelope.

The resulting 218-entry manifest is structural provenance only. It has no
object, level, palette, bitmap, or text meaning until loader control flow
proves that meaning.

## Startup, Levels, and Saves

M12-to-M11 receipts preserve CUE declarations, raw-sector form, known hash,
IPL, stage-two record, user-data span, header, and payload hash. Incomplete
bitmap or object-table evidence blocks title, Soul Room, Continue, and dungeon
entry rather than producing a synthetic route.

Level/object candidates are staged and committed atomically only when every
original-data receipt validates. Descriptor proximity and repeated byte shapes
are not semantic evidence.

The JP Track 02 champion cluster has a separate source receipt at raw offset
`0x0B3D98`. It validates eight newline/NUL-framed records against the JP BIN
identity and decodes the A–P nibble fields for HP, stamina, mana, attributes
and skill levels. This is data-format evidence only: the US BIN has no proven
equivalent text consumer, and portraits or live gameplay ownership are not
inferred from the JP records.

SRM discovery validates gzip framing, optional header fields, CRC32, and ISIZE.
Opaque original bodies remain unavailable until their layout is correlated with
evidence. Firestaff-native export is no-replace atomic, protecting original
Save Disk artifacts. Continue itself is transactional and carries the consumed
media route mask, checksum, and selected level bank.

## Multi-Level Object Table and Progression

Track 02 object-table decode has been extended from level-0/32x27-only to
every level of a dungeon: `theron_v1_track02_read_object_table()` bounds
records against the real per-dungeon level count
(`0..THERON_MAX_LEVELS_PER_DUNGEON-1`), and
`theron_v1_track02_decode_dungeon_level_object_table()` extracts one level's
records from a full-dungeon buffer.
`theron_v1_world_apply_track02_object_table_for_dungeon()` routes decoded
records to every loaded level of a dungeon. Two object kinds carry decoded
fields: `THERON_OBJTYPE_SOUND` (ambient sound ID) and `THERON_OBJTYPE_PIT`
(pit records own their grid position). The authenticated stream does not yet
bind stairs direction, target level or target coordinates, so source-loaded
stairs remain blocked rather than using the former host `+/-1`/same-cell
model. Only separately proven teleporter and quest-complete transitions may
publish a destination.

## Verification

```bash
cmake --build build --target firestaff_theron_v1_track02_ipl_loader_probe \
  firestaff_theron_v1_track02_level_handoff_probe \
  test_theron_v1_startup_save_resume_pc34 --parallel

FIRESTAFF_THERON_TRACK02_US_BIN="/path/to/us-track02.bin" \
FIRESTAFF_THERON_TRACK02_JP_BIN="/path/to/jp-track02.bin" \
  ./build/firestaff_theron_v1_track02_level_handoff_probe
```

Open technical work remains semantic binding for later loader records, object
tables, non-startup levels, original SRM bodies, and packaged-app capture.

## Dungeon Text Boundary

The seven US Track 02 dungeon-text spans are retained directly from the real
disc data. Firestaff verifies their source word counts and diagnostic
terminator counts in a release-build-safe regression, but publishes none as
world or UI text: every span has unresolved five-bit control glyphs. A
cross-check against DMBuilder's source confirms the same offsets and sizes,
not the PC Engine renderer or its control-code semantics. A future text
route requires an executing HuC6280 consumer plus a source payload join; host
text and guessed scroll markup are deliberately excluded.

For strict CUE/record handling and Stage 2/3 receipts, see [Theron's Quest
Track 02 Internals](Therons-Quest-Track02-Internals.md).
