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
- Those same 108 source faces' own DGN Structure2 image bytes match their
  captured command spans. In 48 observations, the command CLUT also matches the
  palette attached to that source face's Structure2 image; in the other 60,
  the command CLUT still matches retail palette bytes, but not that descriptor's
  palette bytes.
- Four other indexed observations have a resolved source face and exact image
  byte match at the command level, but their source face's own DGN material
  record does not match the captured command image bytes. The byte-matched
  command image record set has no Structure3 owner in the scanned corpus for
  these faces.
- Three direct-colour observations have resolved source faces, but direct-colour
  command images do not match the indexed source material records selected by
  those faces; their face-to-material relation remains unproven.
- The remaining 23 runtime face pointers are unresolved in the current
  authentic SH-2 to DGN join.

The seven resolved faces whose source material bytes differ from the observed
command spans are:

| VDP1 command | Mode | Resolved source face | Source DGN selector/material | Byte-matched command image |
| --- | --- | --- | --- |
| `0x001c0` | direct colour | `LEV07.DGN`, entry 58, face 19 | selector 59, indexed `LEV07.DGN` Structure2 image 59 | repeated direct-colour Structure2 image 0 bytes |
| `0x00260` | indexed | `LEV09.DGN`, entry 53, face 43 | selector 56, indexed `LEV09.DGN` Structure2 image 56 | indexed `LEV01.DGN` Structure2 image 1 |
| `0x00280` | direct colour | `LEV14.DGN`, entry 30, face 34 | selector 18, indexed `LEV14.DGN` Structure2 image 18 | repeated direct-colour Structure2 image 0 bytes |
| `0x002e0` | indexed | `LEV09.DGN`, entry 17, face 79 | selector 65, direct-colour `LEV09.DGN` Structure2 image 65 | indexed `LEV01.DGN` Structure2 image 3 |
| `0x00340` | direct colour | `LEV15.DGN`, entry 48, face 74 | selector 49, indexed `LEV15.DGN` Structure2 image 49 | repeated direct-colour Structure2 image 0 bytes |
| `0x00aa0` | indexed | `LEV08.DGN`, entry 78, face 24 | selector 116, indexed `LEV08.DGN` Structure2 image 116 | indexed `LEV01.DGN` Structure2 image 7 |
| `0x00ac0` | indexed | `LEV13.DGN`, entry 18, face 65 | selector 53, indexed `LEV13.DGN` Structure2 image 53 | indexed `LEV01.DGN` Structure2 image 7 |

The face identities are grounded in the captured SH-2 face rows and exact or
unique winding-preserving DGN index-row matches. The different source/command
material records are a real unresolved join; this analyzer does not infer a
level remap or semantic explanation for them.

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
listed observations. A command may use a retail palette different from the
palette paired with the face's DGN Structure2 image descriptor. It does not
prove that a command contributes visible pixels, recover UV selection, assign
a face to direct-colour material bytes, resolve the 23 remaining runtime face
pointers, prove whole-frame VDP1+VDP2 composition, or establish original-Saturn
or production-renderer parity.
