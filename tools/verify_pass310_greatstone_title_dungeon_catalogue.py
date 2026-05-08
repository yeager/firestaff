#!/usr/bin/env python3
"""Record Greatstone TITLE and DUNGEON.DAT catalogue anchors for DM PC 3.4.

Greatstone is secondary catalogue/reference metadata only.  Canonical binary
identity is tied to local PC34 TITLE and DUNGEON.DAT SHA256 hashes; engine truth
remains ReDMCSB/source-locked evidence.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

PASS = "pass310_greatstone_title_dungeon_catalogue"
CANON_TITLE = (Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE")
CANON_DUNGEON = (Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT")
OUT_JSON = Path("parity-evidence/verification/pass310_greatstone_title_dungeon_catalogue.json")
OUT_MD = Path("parity-evidence/pass310_greatstone_title_dungeon_catalogue.md")
TITLE_URL = "http://greatstone.free.fr/dm/db_data/dm_pc_34/title/title.html"
DUNGEON_URL = "http://greatstone.free.fr/dm/db_data/dm_pc_34/dungeon.dat/dungeon.html"
MAP1_URL = "http://greatstone.free.fr/dm/db_data/dm_pc_34/dungeon.dat/map_1.html"

TITLE_ITEMS: list[dict[str, Any]] = [
    {"index": 0, "label": "ANimation"},
    {"index": 1, "label": "BReak"},
    {"index": 2, "label": "EGA Palette"},
    {"index": 3, "label": "PaLette"},
    {"index": 4, "label": "ENcoded image"},
    *[{"index": i, "label": "Delta Layer"} for i in range(5, 41)],
    {"index": 41, "label": "PaLette"},
    {"index": 42, "label": "ENcoded image"},
    *[{"index": i, "label": "Delta Layer"} for i in range(43, 58)],
    {"index": 58, "label": "DOne"},
]

DUNGEON_MAP_LINKS = [
    {"map": i, "url": f"http://greatstone.free.fr/dm/db_data/dm_pc_34/dungeon.dat/map_{i}.html"}
    for i in range(1, 15)
]

MAP1_ANCHORS = [
    {"x": 8, "y": 5, "description": "Hall of champions"},
    {"x": 5, "y": 17, "description": "Vi altar of rebirth"},
    {"x": 14, "y": 2, "description": "champion Chani"},
    {"x": 10, "y": 3, "description": "champion Iaido"},
    {"x": 10, "y": 6, "description": "champion Zed"},
    {"x": 11, "y": 16, "description": "champion Stamm"},
]

EXPECTED_SHA256 = {
    "TITLE": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
    "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def build() -> dict[str, Any]:
    title_hash = sha256(CANON_TITLE)
    dungeon_hash = sha256(CANON_DUNGEON)
    title_ok = title_hash == EXPECTED_SHA256["TITLE"]
    dungeon_ok = dungeon_hash == EXPECTED_SHA256["DUNGEON.DAT"]
    title_indices = [item["index"] for item in TITLE_ITEMS]
    title_index_ok = title_indices == list(range(59))
    dungeon_map_ok = [m["map"] for m in DUNGEON_MAP_LINKS] == list(range(1, 15))
    map1_anchor_ok = any(a["description"] == "Hall of champions" and a["x"] == 8 and a["y"] == 5 for a in MAP1_ANCHORS)
    status = "passed" if title_ok and dungeon_ok and title_index_ok and dungeon_map_ok and map1_anchor_ok else "blocked"
    return {
        "pass": PASS,
        "status": status,
        "scope": "Catalogue Greatstone TITLE and DUNGEON.DAT references Daniel supplied, with canonical local PC34 binary identity checks.",
        "sourceInputs": {
            "greatstoneTitle": TITLE_URL,
            "greatstoneDungeon": DUNGEON_URL,
            "greatstoneDungeonMap1": MAP1_URL,
            "canonicalTitle": str(CANON_TITLE),
            "canonicalDungeonDat": str(CANON_DUNGEON),
        },
        "sourceCaveat": "Greatstone is secondary catalogue metadata. ReDMCSB and canonical PC34 binaries remain the primary evidence sources for behavior/parity.",
        "canonicalBinaryIdentity": {
            "TITLE": {"sha256": title_hash, "expectedSha256": EXPECTED_SHA256["TITLE"], "ok": title_ok},
            "DUNGEON.DAT": {"sha256": dungeon_hash, "expectedSha256": EXPECTED_SHA256["DUNGEON.DAT"], "ok": dungeon_ok},
        },
        "titleCatalogue": {
            "itemCount": len(TITLE_ITEMS),
            "indicesContiguous0To58": title_index_ok,
            "items": TITLE_ITEMS,
        },
        "dungeonCatalogue": {
            "mapLinkCount": len(DUNGEON_MAP_LINKS),
            "mapsContiguous1To14": dungeon_map_ok,
            "mapLinks": DUNGEON_MAP_LINKS,
            "map1Anchors": MAP1_ANCHORS,
        },
        "coverage": {
            "titleItems": len(TITLE_ITEMS),
            "dungeonMaps": len(DUNGEON_MAP_LINKS),
            "map1Anchors": len(MAP1_ANCHORS),
        },
        "notClaimed": [
            "full dungeon tile parity",
            "title animation timing parity",
            "Greatstone as primary engine source",
        ],
    }


def write_md(data: dict[str, Any]) -> None:
    lines = [
        "# pass310 Greatstone TITLE/DUNGEON.DAT catalogue",
        "",
        f"- status: `{data['status']}`",
        f"- TITLE source: {TITLE_URL}",
        f"- DUNGEON source: {DUNGEON_URL}",
        f"- TITLE SHA256: `{data['canonicalBinaryIdentity']['TITLE']['sha256']}`",
        f"- DUNGEON.DAT SHA256: `{data['canonicalBinaryIdentity']['DUNGEON.DAT']['sha256']}`",
        f"- TITLE items: `{data['coverage']['titleItems']}` contiguous 0..58",
        f"- dungeon maps: `{data['coverage']['dungeonMaps']}` contiguous 1..14",
        "",
        "## TITLE catalogue landmarks",
        "",
        "| index | label |",
        "|---:|---|",
    ]
    for item in data["titleCatalogue"]["items"]:
        if item["index"] in {0, 1, 2, 3, 4, 41, 42, 58}:
            lines.append(f"| {item['index']} | {item['label']} |")
    lines.extend(["", "## Dungeon map 1 anchors", "", "| x | y | description |", "|---:|---:|---|"])
    for a in data["dungeonCatalogue"]["map1Anchors"]:
        lines.append(f"| {a['x']} | {a['y']} | {a['description']} |")
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
    print(f"{PASS}: ok titleItems={data['coverage']['titleItems']} dungeonMaps={data['coverage']['dungeonMaps']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
