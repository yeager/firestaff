# Theron's Quest Technical Reference

## Door movement boundary

Movement does not implicitly open a closed or locked door. The explicit
door-use route performs the state transition and key validation; this keeps
source-bound Track 02 object state transactional and fail-closed. A stair
query likewise returns `STAIRS` only when the destination level is resident.

## Scripted replay transport boundary

The latest authenticated external-disk replay uses the US Track 02/System
Card identity and the scripted PCE sequence `run@1:1,run@480:30,i@900:30`.
It proves CD transport and initialization: 240 raw-sector spans, 25 CD IRQ
callbacks, 256 authenticated CD-RAM receipts, 32 game-owned `$E009`
dispatches, and a parser-approved transition receipt.

The Main-RAM consumer sidecar is still init-only. It has 65,536 reads and 512
reads in `$2600-$27FF`, all zero-valued reads from `$CB22`; it has no runtime
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

> **Status reviewed 2026-08-13.** JP/US Track 02 identity and several loader
> receipts are real-data verified. Game-owned dungeon handoff, object/level
> semantics and bitmap/palette binding remain open. The verified startup
> boundary does not establish the Firestaff `$2600` consumer or gameplay
> semantics.

## Scope

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
records to every loaded level of a dungeon. Two object kinds were added to
carry decoded fields: `THERON_OBJTYPE_SOUND` (ambient sound ID) and
`THERON_OBJTYPE_PIT` (pit records own their grid position).
`theron_v1_transition_execute()` implements stairs (validated target level),
progression advance, `theron_v1_world_reset_for_dungeon()`, and quest-complete
handling at the end of `move_party_internal()`.

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
Track 02 Internals](Therons-Quest-Track02-Internals).
