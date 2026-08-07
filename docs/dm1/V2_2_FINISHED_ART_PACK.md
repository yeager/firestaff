# DM1 V2.2 finished-art pack — review schema

## Purpose

DM1 V2.2 selection can only activate when an operator-reviewed
finished-art pack is installed and its `finish_receipt.json`
proves every required slot has been human-reviewed. This document
specifies the required manifest, the receipt schema, and the
selection gate that Firestaff uses to admit or reject a pack.

Without an admitted pack, V2.2 selection falls back to the V2.1
EPX presentation route over the original V1 pixels (see
`docs/plans/DM1_V2_PLAN.md`).

## Pack layout

```
firestaff-dm1-v22-artpack/
    manifest.json           # required slots + checksums
    finish_receipt.json     # operator-reviewed slot approvals
    art/
        wall_d0.png
        wall_d1.png
        wall_d2.png
        wall_d3.png
        floor.png
        ceiling.png
        teleporter_field.png
        ...
```

Each PNG covers a specific F0107..F0115 material slot the M11
V2.2 consumer reads. The full slot list matches
`scripts/build_dm1_v22_complete_artpack.py`.

## manifest.json schema

```json
{
    "version": 2,
    "pack_name": "firestaff-dm1-v22-pc34-source",
    "created": "2026-07-16",
    "slots": [
        {
            "id": "wall_d0",
            "file": "art/wall_d0.png",
            "sha256": "<64 hex>",
            "width": 224,
            "height": 136,
            "source": "hash-verified PC3.4 GRAPHICS.DAT"
        },
        {
            "id": "teleporter_field",
            "file": "art/teleporter_field.png",
            "sha256": "<64 hex>",
            "width": 224,
            "height": 136,
            "source": "operator-drawn"
        }
    ]
}
```

Required slots (all seven runtime-gated V2.2 materials):

  1. `wall_d0`   — D0 wall crop
  2. `wall_d1`   — D1 wall crop
  3. `wall_d2`   — D2 wall crop
  4. `wall_d3`   — D3 wall crop
  5. `floor`     — floor tile
  6. `ceiling`   — ceiling tile
  7. `teleporter_field` — teleporter field overlay

Missing slots block V2.2 admission. Partial packs cause fallback
to V2.1.

## finish_receipt.json schema

```json
{
    "version": 1,
    "pack_manifest_sha256": "<sha256 of manifest.json>",
    "reviewed_by": "operator@example",
    "review_date": "2026-08-07",
    "slots_approved": [
        {
            "id": "wall_d0",
            "expected_sha256": "<must match manifest.json entry>",
            "approved": true,
            "notes": "Matches PC3.4 crop within 1px tolerance"
        }
    ]
}
```

Selection gate rules:

  * `pack_manifest_sha256` must match the actual manifest.json
    file's hash. Any drift rejects the pack.
  * Every slot in manifest.json must appear in `slots_approved`
    with `approved: true`. Otherwise falls back to V2.1.
  * Each `expected_sha256` must match the manifest's `sha256`
    for that slot. A hash drift rejects the pack even if the
    receipt claims approval.

## Firestaff-side gate

`src/dm1v2/dm1_v2_finish_receipt_gate.c` (existing) reads
`finish_receipt.json` from the installed pack root, verifies the
above rules, and publishes a `DM1_V2_FinishReceiptState` that the
M11 V2.2 boot consumer reads via
`dm1_v2_finish_receipt_state_valid_pc34()`.

If the state is invalid or missing, `m11_v22_boot_admit()` rejects
V2.2 and admits V2.1 instead. There is no synthetic fallback —
V2.2 either has a byte-verified reviewed pack or it does not draw.

## Building a pack

`scripts/build_dm1_v22_complete_artpack.py` composes a source-
derived pack from the hash-verified PC 3.4 `GRAPHICS.DAT`, writes
manifest.json + all seven slot PNGs, and writes a matching
`finish_receipt.json` naming the operator that ran the script.
The output pack is installed under
`~/.firestaff/dm1_v22_pack/` and picked up by boot.

## Why the review is required

The V2.2 art is a modernised presentation, not the original PC 3.4
pixels. Any modernised art risks drift from the source geometry
that the M11 runtime enforces. The `finish_receipt.json` review
step is the operator's contractual sign-off that each slot was
compared to the original PC 3.4 material and is authorised for
V2.2 selection.

No script can substitute for the review. The receipt exists so
Firestaff can prove that a human validated the pack before any
V2.2 pixel reaches the framebuffer.
