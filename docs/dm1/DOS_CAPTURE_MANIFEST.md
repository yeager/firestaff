# DM1 PC 3.4 (DOS) capture manifest

## Purpose

The DM1 V1 M11 target is source-locked and fail-closed (see
`docs/parity/DM1_V1_SYNTHETIC_PATH_AUDIT.md`). The remaining
work on the DOS platform is not code — it is **capture and
corpus collection**: packaged Mac captures of the source-owned
runtime and operator-provided original DM1 PC 3.4 `DMSAVE.DAT`
files that exercise real C13 event records.

This document specifies exactly what the operator must provide
and where to place it so that Firestaff's tests can verify the
capture without any synthetic data.

## Required corpus items

### 1. Packaged Mac application capture

Place recorded video / screenshot artifacts under:

```
firestaff-captures/dm1-v1-pc34/
    macos-app-YYYYMMDD/
        entrance.png
        movement.png
        viewport.png
        hud.png
        inventory.png
        spells.png
        capture_receipt.json    # see schema below
```

The `.png` files must be from a real firestaff M11 packaged .app
running on macOS with `~/.firestaff/data/dm1/` populated with
the hash-verified PC 3.4 `GRAPHICS.DAT`, `DUNGEON.DAT`, `SNDDRV3.EXE`
and any other required siblings. No emulator screenshots. No
placeholder art overlays.

### 2. Original C13-bearing save

Place operator-owned `DMSAVE.DAT` files exercising C13 events
under:

```
firestaff-corpus/dm1-v1-pc34/saves/
    save_C13_altar_YYYYMMDD.SAV
    save_C13_fluxcage_YYYYMMDD.SAV
    ...
```

Each save must be a real PC 3.4 in-game save (not a bootstrap
fixture) with a matching `save_receipt.json` recording origin
(operator name, capture date, hash) so provenance is auditable.

### 3. Original-vs-Firestaff paired capture

For each of the six required screens above, provide the paired
original DOS capture (DOSBox / PCem / actual DOS hardware) at the
same coordinates as the Firestaff capture. Place under:

```
firestaff-captures/dm1-v1-pc34/paired-YYYYMMDD/
    firestaff_entrance.png
    dos_entrance.png
    firestaff_movement.png
    dos_movement.png
    ...
```

## capture_receipt.json schema

```json
{
    "version": 1,
    "capture_date": "2026-08-07",
    "firestaff_version": "3.0.294",
    "platform": "macos-14-arm64",
    "data_root": "~/.firestaff/data/dm1",
    "graphics_dat_sha256": "<64 hex>",
    "dungeon_dat_sha256":  "<64 hex>",
    "captures": [
        {
            "screen": "entrance",
            "file": "entrance.png",
            "png_sha256": "<64 hex>",
            "expected_frame_hash": "<optional 8 hex>"
        }
    ]
}
```

Every `png_sha256` records the capture-file identity so the
manifest can be regenerated and compared byte-exact across
sessions.

## save_receipt.json schema

```json
{
    "version": 1,
    "capture_date": "2026-08-07",
    "operator": "user@example",
    "source_platform": "dosbox-0.74.3",
    "save_file": "save_C13_altar_20260807.SAV",
    "save_sha256": "<64 hex>",
    "exercised_events": ["C13_altar"],
    "notes": "..."
}
```

## Verification test

The bundled test `test_dm1_pc34_dos_capture_manifest.c` (this
commit) reads `capture_receipt.json` / `save_receipt.json` files
from a directory named by the `FIRESTAFF_DM1_PC34_CAPTURE_DIR`
env var, validates the JSON schema shape, verifies every
referenced file's SHA-256 matches its recorded hash, and reports
per-item PASS/FAIL. Skipped cleanly when the env var is unset.

Test does not synthesize any images; if a listed file is missing
or its hash drifts, the test fails and names the exact record.

## Where this belongs

Captures and saves are corpus data; they must NOT be committed
to the git repository (they would violate the game-data hook and
inflate the repo). Store them locally under the paths above and
reference them via env vars in CI or manual review.
