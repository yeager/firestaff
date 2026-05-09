#!/usr/bin/env python3
"""Real DM1 V1 wall-asset distinctness gate.

Locks the viewport wall renderer to real GRAPHICS.DAT data, not just source
strings: each visible wall facing/location in the DM1 PC34 wall set must bind
to a distinct decoded GRAPHICS.DAT record/hash, and parity flip must swap L/R
asset IDs without collapsing them.
"""
from __future__ import annotations

import json
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
MANIFEST = REPO / "parity-evidence/verification/pass305_dm1_wall_graphics_93_107_manifest.json"
SRC = REPO / "m11_game_view.c"

EXPECTED_SHA = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"
EXPECTED = {
    "D0R": (93, 33, 136), "D0L": (94, 33, 136),
    "D1R": (95, 60, 111), "D1L": (96, 60, 111), "D1C": (97, 160, 111),
    "D2R2": (98, 8, 52), "D2L2": (99, 8, 52),
    "D2R": (100, 78, 74), "D2L": (101, 78, 74), "D2C": (102, 106, 74),
    "D3R2": (103, 44, 49), "D3L2": (104, 44, 49),
    "D3R": (105, 83, 49), "D3L": (106, 83, 49), "D3C": (107, 70, 49),
}

# G2107_WallSet ordinal order from ReDMCSB DUNVIEW.C:183-200 / DEFS.H:2359-2373.
PARITY_PAIRS = [
    ("D0L", "D0R"), ("D1L", "D1R"), ("D2L", "D2R"), ("D2L2", "D2R2"),
    ("D3L", "D3R"), ("D3L2", "D3R2"),
]
CENTER = ["D1C", "D2C", "D3C"]


def main() -> int:
    data = json.loads(MANIFEST.read_text())
    if data["canonicalGraphicsDat"]["sha256"] != EXPECTED_SHA:
        raise AssertionError("canonical GRAPHICS.DAT SHA drifted")
    records = {r["graphicIndex"]: r for r in data["records"]}
    seen_hashes: dict[str, str] = {}
    for name, (idx, w, h) in EXPECTED.items():
        rec = records.get(idx)
        if not rec:
            raise AssertionError(f"missing real GRAPHICS.DAT wall record {idx} for {name}")
        if (rec["width"], rec["height"]) != (w, h):
            raise AssertionError(f"{name}/{idx} dimensions drifted: {(rec['width'], rec['height'])} != {(w, h)}")
        pix = rec["decode"]["unpackedPixelSha256"]
        if pix in seen_hashes:
            raise AssertionError(f"{name}/{idx} hash duplicates {seen_hashes[pix]}; expected distinct real wall asset")
        seen_hashes[pix] = f"{name}/{idx}"

    src = SRC.read_text()
    for name, (idx, _w, _h) in EXPECTED.items():
        token = f"M11_GFX_WALLSET0_{name}"
        if token not in src or f"= {idx}," not in src:
            raise AssertionError(f"Firestaff constant for {name} does not bind GRAPHICS.DAT {idx}")

    for left, right in PARITY_PAIRS:
        li, ri = EXPECTED[left][0], EXPECTED[right][0]
        if li == ri:
            raise AssertionError(f"bad parity pair {left}/{right}: same index")
        if records[li]["decode"]["unpackedPixelSha256"] == records[ri]["decode"]["unpackedPixelSha256"]:
            raise AssertionError(f"bad parity pair {left}/{right}: same decoded pixels")
    for name in CENTER:
        idx = EXPECTED[name][0]
        if not records[idx]["decode"]["unpackedPixelSha256"]:
            raise AssertionError(f"center wall {name}/{idx} lacks decoded hash")

    required_runtime = [
        "return (state->world.party.mapX +",
        "state->world.party.mapY +",
        "state->world.party.direction) & 1",
        "partner = i ^ 1",
        "swapped.graphicIndex = kSideBlits[partner].graphicIndex",
    ]
    missing = [needle for needle in required_runtime if needle not in src]
    if missing:
        raise AssertionError(f"runtime parity selection missing tokens: {missing}")

    print("PASS real DM1 V1 wall assets are distinct and parity swaps L/R asset IDs")
    print(f"- canonical GRAPHICS.DAT sha256: {EXPECTED_SHA}")
    print(f"- checked {len(EXPECTED)} GRAPHICS.DAT wall records: 93..107")
    print("- parity pairs: " + ", ".join(f"{l}<->{r}" for l, r in PARITY_PAIRS))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
