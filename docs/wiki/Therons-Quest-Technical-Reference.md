# Theron's Quest Technical Reference

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

SRM discovery validates gzip framing, optional header fields, CRC32, and ISIZE.
Opaque original bodies remain unavailable until their layout is correlated with
evidence. Firestaff-native export is no-replace atomic, protecting original
Save Disk artifacts. Continue itself is transactional and carries the consumed
media route mask, checksum, and selected level bank.

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
