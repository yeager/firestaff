#!/usr/bin/env python3
"""Verify pass306 bridge from pass300 wall pixel regions to pass305 GRAPHICS.DAT wall records.

This is intentionally a metadata bridge only: it proves every comparator-ready
wall pixelRegion emitted by pass300 names a GRAPHICS.DAT wall-set index that is
present in the pass305 decoded/checksummed wall manifest. It does not publish
bitmap bytes and does not claim original pixel parity.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

PASS = "pass306_dm1_wall_pixel_region_graphics_bridge"
PASS300_JSON = Path("parity-evidence/verification/dm1_v1_viewport_wall_render_plan_gate.json")
PASS304_JSON = Path("parity-evidence/verification/pass304_dm1_v1_original_viewport_capture_blocker_manifest.json")
PASS305_JSON = Path("parity-evidence/verification/pass305_dm1_wall_graphics_93_107_manifest.json")
OUT_JSON = Path("parity-evidence/verification/pass306_dm1_wall_pixel_region_graphics_bridge.json")

SOURCE_ANCHORS = [
    {
        "file": "DUNVIEW.C",
        "lines": "6699-6702",
        "claim": "ReDMCSB wall draw calls select G0698_puc_Bitmap_WallSet_Wall_D3LCR and Graphic558 wall frame descriptors before drawing into the viewport bitmap.",
    },
    {
        "file": "COORD.C",
        "function": "F0635_ layout-696 zone resolution",
        "claim": "pass300 pixelRegion rect/sourceClip descriptors are layout-zone based viewport regions, not raw screenshot crops.",
    },
    {
        "file": "GRAPHICS.DAT",
        "indices": "93..107",
        "claim": "pass305 decodes/checksums the wall-set records required by pass304 without dumping bitmap bytes.",
    },
]


def load_json(path: Path) -> Any:
    if not path.exists():
        raise FileNotFoundError(path)
    return json.loads(path.read_text(encoding="utf-8"))


def wall_events(pass300: dict[str, Any]) -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for snap in pass300.get("comparedSnapshots", []):
        snap_name = snap.get("name")
        party = snap.get("party", {})
        for ev in snap.get("renderEvents", []):
            pr = ev.get("pixelRegion")
            if not isinstance(pr, dict):
                continue
            idx = pr.get("wallSetGraphicIndex")
            if idx is None:
                continue
            out.append(
                {
                    "snapshot": snap_name,
                    "party": party,
                    "row": ev.get("row"),
                    "event": ev.get("event"),
                    "valid": ev.get("valid"),
                    "source": ev.get("source"),
                    "selectedWall": pr.get("selectedWall") or ev.get("selectedWall"),
                    "wallSetGraphicIndex": idx,
                    "rect": pr.get("rect"),
                    "sourceClip": pr.get("sourceClip"),
                    "zone": pr.get("zone"),
                    "layout": pr.get("layout"),
                    "expectedPixelsFrom": pr.get("expectedPixelsFrom"),
                }
            )
    return out


def build_bridge() -> dict[str, Any]:
    p300 = load_json(PASS300_JSON)
    p304 = load_json(PASS304_JSON)
    p305 = load_json(PASS305_JSON)
    records = {int(r["graphicIndex"]): r for r in p305.get("records", [])}
    events = wall_events(p300)
    bridged = []
    missing = []
    for ev in events:
        idx = int(ev["wallSetGraphicIndex"])
        rec = records.get(idx)
        if rec is None:
            missing.append(ev)
            continue
        bridged.append(
            {
                **ev,
                "graphicsDatRecord": {
                    "graphicIndex": idx,
                    "fileOffset": rec["fileOffset"],
                    "compressedBytes": rec["compressedBytes"],
                    "width": rec["width"],
                    "height": rec["height"],
                    "compressedRecordSha256": rec["compressedRecordSha256"],
                    "packedSha256": rec["decode"]["packedSha256"],
                    "unpackedPixelSha256": rec["decode"]["unpackedPixelSha256"],
                    "localPaletteNibbles": rec["decode"].get("localPaletteNibbles"),
                },
                "regionWithinDecodedBitmap": (
                    bool(ev.get("sourceClip"))
                    and ev["sourceClip"]["x"] >= 0
                    and ev["sourceClip"]["y"] >= 0
                    and ev["sourceClip"]["x"] + ev["sourceClip"]["width"] <= rec["width"]
                    and ev["sourceClip"]["y"] + ev["sourceClip"]["height"] <= rec["height"]
                ),
                "viewportRectWithin224x136": (
                    bool(ev.get("rect"))
                    and ev["rect"]["x"] >= 0
                    and ev["rect"]["y"] >= 0
                    and ev["rect"]["x"] + ev["rect"]["width"] <= 224
                    and ev["rect"]["y"] + ev["rect"]["height"] <= 136
                ),
            }
        )
    required = p304.get("requiredDecodedGraphicsDatRecords", {}).get("wallSetGraphicIndicesFromCurrentRenderPlan", [])
    present_required = sorted(i for i in required if i in records)
    missing_required = sorted(i for i in required if i not in records)
    bridged_indices = sorted({int(ev["wallSetGraphicIndex"]) for ev in bridged})
    status = "passed" if events and not missing and not missing_required and all(e["regionWithinDecodedBitmap"] and e["viewportRectWithin224x136"] for e in bridged) else "blocked"
    return {
        "pass": PASS,
        "status": status,
        "scope": "Bridge pass300 DM1 V1 wall pixelRegion descriptors to pass305 GRAPHICS.DAT wall-record checksums for indices 93..107.",
        "sourceInputs": {
            "pass300WallRenderPlan": str(PASS300_JSON),
            "pass304OriginalCaptureBlocker": str(PASS304_JSON),
            "pass305WallGraphicsManifest": str(PASS305_JSON),
        },
        "sourceAnchors": SOURCE_ANCHORS,
        "coverage": {
            "pass300WallPixelRegionEvents": len(events),
            "bridgedEvents": len(bridged),
            "missingEventRecords": len(missing),
            "requiredPass304WallIndices": required,
            "presentRequiredPass304WallIndices": present_required,
            "missingRequiredPass304WallIndices": missing_required,
            "bridgedIndicesFromActualRenderEvents": bridged_indices,
        },
        "bridgedWallPixelRegions": bridged,
        "missingWallPixelRegionRecords": missing,
        "remainingComparatorBlockers": [
            "original PC34 viewport capture remains route-unproven for pass304 required states",
            "metadata bridge does not yet perform per-pixel blit/crop comparison against original captures",
            "D4 rows remain object-stack/content events only and are intentionally excluded from wall bitmap comparison",
        ],
        "notClaimed": [
            "original pixel parity",
            "published bitmap byte dumps",
            "route-proven original PC34 screenshots for pass304 states",
        ],
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--write", action="store_true")
    args = ap.parse_args()
    manifest = build_bridge()
    text = json.dumps(manifest, indent=2, sort_keys=True) + "\n"
    if args.write:
        OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
        OUT_JSON.write_text(text, encoding="utf-8")
    if manifest["status"] != "passed":
        print(f"{PASS}: {manifest['status']}")
        print(json.dumps(manifest["coverage"], indent=2, sort_keys=True))
        return 1
    print(f"{PASS}: ok bridged={manifest['coverage']['bridgedEvents']} indices={manifest['coverage']['bridgedIndicesFromActualRenderEvents']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
