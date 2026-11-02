# Nexus SH-2 face-to-VDP1 command correlation

## Verified result

This report joins authentic Japanese retail Structure3 face rows to Ymir SH-2 projected-coordinate staging records and captured VDP1 command geometry for VDP1 frame 1605.

At SH-2 PC `0x0608f364`, a complete register snapshot captures `r9` pointing to the active 12-byte face row in WRAM-H and `r2` pointing to its 16-byte projected-coordinate record. Four subsequent writes at PCs `0x0608f366`, `0x0608f36a`, `0x0608f36e`, and `0x0608f372` fill that record.

The authentic trace contains 138 face/staging observations (79 master SH-2 and 59 slave SH-2):

- Each record has all four coordinate writes at the observed `r2` address.
- Each four-corner coordinate multiset maps to exactly one captured VDP1 command geometry; the reverse mapping is also one-to-one across this captured set.
- 123 records match in exact corner order; 133 match by cyclic rotation. Five remaining records match only as unordered corner sets, so winding and corner correspondence are not claimed for those five.
- The retail-data scan covers 1,144 Structure3 entries and 18,478 faces after verifying the SHA-256 hashes of all 16 `LEVxx.DGN` files against Firestaff's retail hash fixture.
- 64 observations map to a unique face row directly. Unique contiguous face-row sequences resolve another 23 distinct runtime pointers.
- A further six runtime pointers resolve by table offset: each candidate must agree with at least two already-resolved face rows from the same CPU and DGN entry within 96 bytes, with the observed 12-byte pointer stride matching the DGN face-ordinal stride. The analyzer verifies this invariant across all 425 nearby same-entry anchor pairs and rejects any conflict.
- Overall, 93 of 120 distinct runtime face-row pointers resolve to retail rows; those pointers account for 111 of 138 capture records. The remaining 27 pointers (27 records) are unresolved. This does not show that those faces are absent from retail data.

## Reproduce

Keep the authentic disc files and Ymir captures outside Git. The evidence directory must contain `f364-correlation/dumps/` and the retail directory must contain `LEV00.DGN` through `LEV15.DGN`.

Run from a Firestaff checkout that contains `scripts/fixtures/nexus_v1_disc_file_hashes.py`:

```sh
python3 scripts/analyze_nexus_ymir_face_to_command.py \
  --evidence /path/to/ymir-sh2-face-selection-20261102 \
  --retail-data /path/to/retail-source \
  --firestaff-repo /path/to/firestaff-checkout \
  --csv-out /path/to/output/face-to-command.csv \
  --json-out /path/to/output/face-to-command.json
```

The external evidence bundle includes `SHA256SUMS.txt`, the analyzer outputs, the Ymir command and run logs, and the exact successful capture invocation. It also contains the hash-verified retail inputs. No game data or emulator memory dumps are committed.

## Limits

This verifies a source-face → projected-coordinate staging → VDP1 command geometry join for a subset of one authentic Ymir frame. It does not resolve every face row, prove winding for unordered-only matches, establish material or pixel ownership, verify complete VDP1+VDP2 frame composition, establish original-Saturn parity, or enable production renderer admission. It does not establish arbitrary-frame Nexus support.

## Consecutive-frame staging-to-command verification

The paired authentic movement capture at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_movement_staging_writer_capture_20261030/`
also contains SH-2 coordinate writes and VDP1 command traces for eight consecutive
frames. Both runs load the same Japanese retail New Game state; the `up` run adds
30 frames of Control Pad 1 Up before this frame window. Each trace receipt reports
complete data with zero dropped rows. The SH-2 trace covers frames 1598–1605, and
the command trace covers frames 0–1605.

Analyze the authentic control capture and compare it with the paired Up capture:

```sh
python3 scripts/analyze_nexus_ymir_multiframe_staging_to_vdp1.py \
  --dumps /path/to/nexus_movement_staging_writer_capture_20261030/control/dumps \
  --run control \
  --paired-dumps /path/to/nexus_movement_staging_writer_capture_20261030/up/dumps \
  --paired-run up \
  --register-dumps /path/to/ymir-sh2-face-selection-20261102/f364-correlation/dumps \
  --json-out /path/to/output/control-summary.json \
  --csv-out /path/to/output/control-joins.csv
```

The analyzer checks both Ymir receipts, rejects incomplete or dropped traces,
groups the four observed coordinate writes into each staging record, then
requires a unique frame-local VDP1 geometry match and one-to-one reverse
ownership. Its paired comparison counts shared staging addresses whose XY words
changed between runs and records present in only one run.

The optional `--register-dumps` input cross-links the control capture to a
separate SH-2 register-read run. It requires identical full VDP1 command-trace
and VDP1 VRAM hashes, then checks each frame's PC `0x608f364` `r2` staging
pointer against the captured writes. The read run contains 138 such pointers
per frame; all 1,104 pointers match the control writes. The two final WRAM-H
snapshots also contain identical 12-byte face rows at all 1,104 observed
`r9` events (120 distinct CPU/pointer pairs). The same set and event
multiplicities for those pointers occur in every frame from 1598 through 1605.
These are end-of-capture memory snapshots, so this check does not prove the
face-row bytes at intermediate frames.

| Run | Frames | Staging records | Unique geometry joins | Ordered | Cyclic | Unordered only |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Control | 1598–1605 | 1,104 (138/frame) | 1,104 | 984 | 80 | 40 |
| Up | 1598–1605 | 1,056 (132/frame) | 1,056 | 888 | 104 | 64 |

All captured records join uniquely in each of the eight frames, with no partial
staging records. Across the pair, all 1,056 shared frame/address records have
different XY words after the Up input; 48 additional control records are absent
from the Up trace. This confirms input-sensitive SH-2 projection coordinates
reaching authentic VDP1 commands across this captured window. It does not bind
those rows to retail DGN faces, prove winding for cyclic or unordered matches,
establish material or pixel ownership, verify full-frame VDP1+VDP2 composition,
or establish original Saturn parity. It is evidence for this state and frame
window, not arbitrary gameplay coverage or production renderer admission.
