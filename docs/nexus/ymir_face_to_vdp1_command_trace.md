# Nexus SH-2 face-to-VDP1 command correlation

## Verified result

This report joins authentic Japanese retail Structure3 face rows to Ymir SH-2 projected-coordinate staging records and captured VDP1 command geometry for VDP1 frame 1605.

At SH-2 PC `0x0608f364`, a complete register snapshot captures `r9` pointing to the active 12-byte face row in WRAM-H and `r2` pointing to its 16-byte projected-coordinate record. Four subsequent writes at PCs `0x0608f366`, `0x0608f36a`, `0x0608f36e`, and `0x0608f372` fill that record.

The authentic trace contains 138 face/staging observations (79 master SH-2 and 59 slave SH-2):

- Each record has all four coordinate writes at the observed `r2` address.
- Each four-corner coordinate multiset maps to exactly one captured VDP1 command geometry; the reverse mapping is also one-to-one across this captured set.
- 123 records match in exact corner order; 133 match by cyclic rotation. Five remaining records match only as unordered corner sets, so winding and corner correspondence are not claimed for those five.
- The retail-data scan covers 1,144 Structure3 entries and 18,478 faces after verifying the SHA-256 hashes of all 16 `LEVxx.DGN` files against Firestaff's retail hash fixture.
- 64 observations map to a unique face row directly. Unique contiguous face-row sequences resolve another 23 distinct runtime pointers. Overall, 87 of 120 distinct runtime face-row pointers resolve to retail rows; those pointers account for 101 of 138 capture records. The remaining 33 pointers (37 records) are unresolved. This does not show that those faces are absent from retail data.

## Reproduce

Keep the authentic disc files and Ymir captures outside Git. The evidence directory must contain `f364-correlation/dumps/` and the retail directory must contain `LEV00.DGN` through `LEV15.DGN`.

Run from a Firestaff checkout that contains `scripts/fixtures/nexus_v1_disc_file_hashes.py`:

```sh
python3 scripts/analyze_nexus_ymir_face_to_command.py \
  --evidence /path/to/ymir-sh2-face-selection-20261102 \
  --retail-data /path/to/retail-source \
  --firestaff-repo /path/to/firestaff-checkout \
  --csv-out /tmp/face-to-command.csv \
  --json-out /tmp/face-to-command.json
```

The external evidence bundle includes `SHA256SUMS.txt`, the analyzer outputs, the Ymir command and run logs, and the exact successful capture invocation. It also contains the hash-verified retail inputs. No game data or emulator memory dumps are committed.

## Limits

This verifies a source-face → projected-coordinate staging → VDP1 command geometry join for a subset of one authentic Ymir frame. It does not resolve every face row, prove winding for unordered-only matches, establish material or pixel ownership, verify complete VDP1+VDP2 frame composition, establish original-Saturn parity, or enable production renderer admission. It does not establish arbitrary-frame Nexus support.
