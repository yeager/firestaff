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
