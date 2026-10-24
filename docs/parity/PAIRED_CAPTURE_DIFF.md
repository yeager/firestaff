# Paired Runtime Capture Diff

`tools/paired_capture_diff.py` measures original/reference and Firestaff images
only after both sides are bound to matching source-state fields. The tool is
game- and platform-neutral, so DM1, CSB, DM2, Theron, and Nexus adapters share
one capture manifest and review format while supplying their own source
coordinates, mode, party, champion, or other state fields. Platform identifiers
remain adapter-defined.

## Evidence contract

The manifest contains one or more named checkpoint pairs. Each side records an
image path and SHA-256, a canonical state object, and a JSON evidence artifact
with its own SHA-256. `stateEvidence.bindings` maps every comparison key to a
JSON Pointer in that artifact. The evidence must also bind `captureId` and
`imageSha256` to JSON Pointers that resolve to the pair's capture ID and that
side's exact image SHA-256. The tool verifies both file hashes and all state
and image bindings before comparing pixels. This prevents a valid state receipt
from being reused with an unrelated image or checkpoint.

Every checkpoint pair also requires a `provenance` object. Its
`authenticMedia.receipt` is a hash-verified JSON receipt that asserts
`authentic: true` and the exact media `identity` named by the manifest.
`firestaffDebug.receipt` is a separately hash-verified JSON runtime receipt
that asserts `debug: true` and the exact `buildId` named by the manifest;
`firestaffDebug.enabled` must be `true`. This makes the debug build and
authentic-data basis reviewable for every pair. A valid receipt only verifies
what it explicitly asserts; reviewers must still inspect how the receipt was
produced. Synthetic test fixtures may exercise the validator but are never
evidence of game or platform parity.

`frameSize` declares the exact width and height for a capture pair. It supports
platform-native layouts, including 320x200 DOS/Atari ST/Amiga views, 320x224
Saturn compositions, and 256x224 PC Engine frames. Both images must already
match the declared geometry; the tool does not resize, crop, or letterbox.
Region rectangles use the same full-frame coordinates for both images. If an
adapter normalizes either image before comparison, record that operation and
its provenance alongside the manifest, and hash the normalized input image.

Example manifest:

```json
{
  "schema": "firestaff.parity.paired_capture.v2",
  "gameId": "dm1",
  "platformId": "dos-pc34",
  "referenceBuildId": "DOSBox-X-version-or-machine-image-id",
  "firestaffBuildId": "git-commit-or-build-id",
  "frameSize": [320, 200],
  "comparisonStateKeys": ["map", "position.x", "position.y", "facing"],
  "regions": [
    {"name": "viewport", "x": 0, "y": 33, "width": 224, "height": 136},
    {"name": "hud", "x": 224, "y": 0, "width": 96, "height": 200}
  ],
  "captures": [
    {
      "id": "hoc-entry",
      "provenance": {
        "authenticMedia": {
          "identity": "verified-edition-media-id",
          "receipt": {
            "path": "private-authentic-media-receipt.json",
            "sha256": "<64-hex-digest>"
          }
        },
        "firestaffDebug": {
          "enabled": true,
          "buildId": "git-commit-or-build-id",
          "receipt": {
            "path": "private-firestaff-debug-receipt.json",
            "sha256": "<64-hex-digest>"
          }
        }
      },
      "reference": {
        "path": "original-hoc-entry.png",
        "sha256": "<64-hex-digest>",
        "state": {"map": 2, "position": {"x": 5, "y": 7}, "facing": "north"},
        "stateEvidence": {
          "path": "original-hoc-entry-state.json",
          "sha256": "<64-hex-digest>",
          "bindings": {
            "captureId": "/captureId", "imageSha256": "/imageSha256",
            "map": "/map", "position.x": "/position/x",
            "position.y": "/position/y", "facing": "/facing"
          }
        }
      },
      "firestaff": {
        "path": "firestaff-hoc-entry.png",
        "sha256": "<64-hex-digest>",
        "state": {"map": 2, "position": {"x": 5, "y": 7}, "facing": "north"},
        "stateEvidence": {
          "path": "firestaff-hoc-entry-state.json",
          "sha256": "<64-hex-digest>",
          "bindings": {
            "captureId": "/captureId", "imageSha256": "/imageSha256",
            "map": "/map", "position.x": "/position/x",
            "position.y": "/position/y", "facing": "/facing"
          }
        }
      }
    }
  ]
}
```

Run it with a private output directory outside the repository or under a
Git-ignored directory:

```sh
python3 tools/paired_capture_diff.py \
  --manifest /path/to/private/hoc-entry.json \
  --out-dir /path/to/private/hoc-entry-diff
```

The output includes one red/white PNG mask per region and
`paired_capture_report.json` with source-image hashes, evidence hashes, matched
state fields, media/debug provenance receipt hashes, declared frame geometry,
and per-region differing-pixel counts.
Capture IDs and region names are restricted to filename-safe identifiers, and
generated files are resolved within the approved output directory.
For all five games, adapters should use state fields available for that exact
checkpoint and must leave unknown coordinates unknown. For example, a title
screen pair can compare startup phase and scene identity; it must not invent a
dungeon position. A nonzero difference is only a measurement. It does not
itself determine which renderer is correct or promote a parity claim. Original
game media and screenshots should remain in local/private evidence storage
and outside Git.
