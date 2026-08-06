# Theron's Quest Track 02 Internals

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
records to every loaded level of a dungeon, and
`theron_v1_transition_execute()` implements stairs (validated target level),
progression advance, `theron_v1_world_reset_for_dungeon()`, and quest-complete
handling at the end of `move_party_internal()`. This remains real Track 02
data decode, not synthesized object placement.

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
