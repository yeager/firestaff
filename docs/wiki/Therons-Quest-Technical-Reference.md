# Theron's Quest Technical Reference

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
