#!/usr/bin/env python3
"""Cross-check pass305 DM1 GRAPHICS.DAT wall records against Greatstone index labels.

Greatstone is used here as a secondary index-label catalogue, not as engine
truth.  ReDMCSB/source-locked pass300/pass306 still define rendering use.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

PASS = "pass308_greatstone_wall_labels_crosscheck"
PASS305_JSON = Path("parity-evidence/verification/pass305_dm1_wall_graphics_93_107_manifest.json")
PASS306_JSON = Path("parity-evidence/verification/pass306_dm1_wall_pixel_region_graphics_bridge.json")
OUT_JSON = Path("parity-evidence/verification/pass308_greatstone_wall_labels_crosscheck.json")
OUT_MD = Path("parity-evidence/pass308_greatstone_wall_labels_crosscheck.md")
SOURCE_URL = "http://greatstone.free.fr/dm/db_data/dm_pc_34/graphics.dat/graphics.dat.html"

GREATSTONE_WALL_LABELS: dict[int, str] = {
    93: "Dungeon Graphics - Wall (Right Side 0)",
    94: "Dungeon Graphics - Wall (Left Side 0)",
    95: "Dungeon Graphics - Wall (Right Side 1)",
    96: "Dungeon Graphics - Wall (Left Side 1)",
    97: "Dungeon Graphics - Wall (Front 1)",
    98: "Dungeon Graphics - Wall (Far Right Side 2)",
    99: "Dungeon Graphics - Wall (Far Left Side 2)",
    100: "Dungeon Graphics - Wall (Right Side 2)",
    101: "Dungeon Graphics - Wall (Left Side 2)",
    102: "Dungeon Graphics - Wall (Front 2)",
    103: "Dungeon Graphics - Wall (Far Right Side 3)",
    104: "Dungeon Graphics - Wall (Far Left Side 3)",
    105: "Dungeon Graphics - Wall (Right Side 3)",
    106: "Dungeon Graphics - Wall (Left Side 3)",
    107: "Dungeon Graphics - Wall (Front 3)",
}


def load_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def build() -> dict[str, Any]:
    p305 = load_json(PASS305_JSON)
    p306 = load_json(PASS306_JSON)
    records = {int(r["graphicIndex"]): r for r in p305.get("records", [])}
    bridge_indices = p306.get("coverage", {}).get("bridgedIndicesFromActualRenderEvents", [])
    rows: list[dict[str, Any]] = []
    missing_from_pass305: list[int] = []
    for idx, label in GREATSTONE_WALL_LABELS.items():
        rec = records.get(idx)
        if rec is None:
            missing_from_pass305.append(idx)
            continue
        rows.append({
            "graphicIndex": idx,
            "greatstoneLabel": label,
            "pass305Role": rec.get("role"),
            "pass305Symbol": rec.get("symbol"),
            "pass305ViewportZone": rec.get("viewportZone"),
            "width": rec.get("width"),
            "height": rec.get("height"),
            "fileOffset": rec.get("fileOffset"),
            "compressedRecordSha256": rec.get("compressedRecordSha256"),
            "unpackedPixelSha256": rec.get("decode", {}).get("unpackedPixelSha256"),
            "usedByPass306RenderEvents": idx in bridge_indices,
        })
    missing_from_greatstone_range = sorted(set(records) - set(GREATSTONE_WALL_LABELS))
    status = "passed" if not missing_from_pass305 and not missing_from_greatstone_range else "blocked"
    return {
        "pass": PASS,
        "status": status,
        "scope": "Secondary catalogue label cross-check for pass305 GRAPHICS.DAT wall records 93..107, tied to pass306 actual render-event usage.",
        "sourceInputs": {
            "greatstoneGraphicsDatIndexPage": SOURCE_URL,
            "pass305WallGraphicsManifest": str(PASS305_JSON),
            "pass306WallPixelRegionBridge": str(PASS306_JSON),
        },
        "sourceCaveat": "Greatstone labels are secondary catalogue metadata. Rendering truth remains ReDMCSB/source-locked pass300/pass306 evidence and canonical GRAPHICS.DAT checksums.",
        "coverage": {
            "greatstoneWallLabelCount": len(GREATSTONE_WALL_LABELS),
            "pass305RecordCount": len(records),
            "crossCheckedRows": len(rows),
            "missingFromPass305": missing_from_pass305,
            "extraPass305RecordsOutsideGreatstoneLabelSet": missing_from_greatstone_range,
            "pass306UsedIndicesPresent": sorted(i for i in bridge_indices if i in GREATSTONE_WALL_LABELS),
        },
        "rows": rows,
        "notClaimed": [
            "pixel parity",
            "Greatstone as primary engine source",
            "route-proven original PC34 screenshots",
        ],
    }


def write_md(data: dict[str, Any]) -> None:
    lines = [
        "# pass308 Greatstone wall-label cross-check",
        "",
        f"- status: `{data['status']}`",
        f"- source: {SOURCE_URL}",
        f"- rows: `{data['coverage']['crossCheckedRows']}`",
        "- caveat: Greatstone is secondary catalogue metadata; ReDMCSB/pass300/pass306 remain rendering evidence.",
        "",
        "| index | Greatstone label | pass305 role | zone | dimensions | used by pass306 |",
        "|---:|---|---|---|---|---|",
    ]
    for r in data["rows"]:
        lines.append(
            f"| {r['graphicIndex']} | {r['greatstoneLabel']} | {r['pass305Role']} | {r['pass305ViewportZone']} | {r['width']}x{r['height']} | {str(r['usedByPass306RenderEvents']).lower()} |"
        )
    lines.extend(["", "Not claimed:"])
    for item in data["notClaimed"]:
        lines.append(f"- {item}")
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--write", action="store_true")
    args = ap.parse_args()
    data = build()
    if args.write:
        OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
        OUT_JSON.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        write_md(data)
    if data["status"] != "passed":
        print(f"{PASS}: {data['status']}")
        print(json.dumps(data["coverage"], indent=2, sort_keys=True))
        return 1
    print(f"{PASS}: ok rows={data['coverage']['crossCheckedRows']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
