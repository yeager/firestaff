# Ymir frame 1605 face-to-material cross-join

This analysis joins the authentic Japanese retail Ymir frame-1605 face/staging
trace to the captured VDP1 command trace, VDP1 VRAM, and hash-verified retail
`LEVxx.DGN` Structure2 and Structure3 data. The inputs remain outside Git.

## Verified result

- All 138 SH-2 face/staging observations map one-to-one to captured VDP1 command
  geometry and a bounded command texture span.
- All 138 command texture spans match retail DGN image bytes exactly: 131
  indexed images and seven direct-colour images.
- Each of the 131 indexed commands also matches captured CLUT bytes to retail
  DGN palette bytes.
- For 108 indexed observations, the resolved source face is among the
  Structure3 faces whose selector matches the image ID of at least one
  byte-matched candidate DGN image record.
- Four other indexed observations have a resolved source face and exact image
  match, but the byte-matched DGN image record set has no Structure3 face owner
  in the scanned retail corpus.
- Three direct-colour observations have resolved source faces, but direct-colour
  image bytes do not carry the indexed Structure3 selector used for ownership;
  the face-to-material relation remains unproven.
- The remaining 23 runtime face pointers are unresolved in the current
  authentic SH-2 to DGN join.

## Reproduce

Run from a Firestaff checkout with the frame-1605 evidence and hash-verified
retail files available outside the repository:

```sh
python3 scripts/analyze_nexus_ymir_face_material_cross_join.py \
  --evidence /path/to/ymir-sh2-face-selection-20261102 \
  --retail-data /path/to/retail-source \
  --firestaff-repo /path/to/firestaff-checkout \
  --frame 1605 \
  --json-out /path/to/face-material-cross-join.json
```

The analyzer verifies retail DGN hashes through Firestaff's canonical hash
fixture, checks the authentic SH-2 staging-to-command join, and compares
captured image and palette byte spans with DGN records. It emits per-observation
JSON so unresolved and nonmatching relationships remain inspectable.

## Limits

This establishes byte identity and source-face selector ownership for the
listed observations. It does not prove that a command contributes visible
pixels, recover UV selection, assign a face to direct-colour material bytes,
resolve the 23 remaining runtime face pointers, prove whole-frame VDP1+VDP2
composition, or establish original-Saturn or production-renderer parity.
