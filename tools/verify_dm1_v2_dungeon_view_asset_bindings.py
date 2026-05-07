#!/usr/bin/env python3
"""Verify pass271 DM1 V2 dungeon-view logical asset bindings.

This gate keeps the V2 shared dungeon-view logical IDs from drifting back to
catalog-only placeholders. It requires each wall/floor/ceiling/door/stairs ID to
bind to a V2 manifest entry with source evidence, and it checks the cited
ReDMCSB source still contains the symbols/routes that make the binding real.
"""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
VERIFY_OUT = ROOT / "parity-evidence/verification/pass271_dm1_v2_dungeon_view_asset_bindings.json"
REQUIRED_LOGICAL_IDS = [
    "fs.v2.shared.dungeon-view.wall.front",
    "fs.v2.shared.dungeon-view.wall.side",
    "fs.v2.shared.dungeon-view.floor.base",
    "fs.v2.shared.dungeon-view.ceiling.base",
    "fs.v2.shared.dungeon-view.door.wood.closed",
    "fs.v2.shared.dungeon-view.stairs.down",
]
REQUIRED_SOURCE_MARKERS = {
    "DEFS.H": [
        "M650_GRAPHIC_FLOOR_SET_0_FLOOR",
        "M651_GRAPHIC_FLOOR_SET_0_CEILING",
        "M646_GRAPHIC_FIRST_WALL_SET",
        "C097_GRAPHIC_WALLSET_0_D1C",
        "C107_GRAPHIC_WALLSET_0_D3C",
        "M633_GRAPHIC_FIRST_DOOR_SET",
        "C018_STAIRS_GRAPHIC_COUNT",
        "C08_STAIRS_BITMAP_DOWN_FRONT_D3C",
        "C16_STAIRS_BITMAP_DOWN_SIDE_D1L",
    ],
    "DUNVIEW.C": [
        "F0094_DUNGEONVIEW_LoadFloorSet",
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "F0100_DUNGEONVIEW_DrawWallSetBitmap",
        "F0102_DUNGEONVIEW_DrawDoorBitmap",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap",
        "F0111_DUNGEONVIEW_DrawDoor",
        "G0676_i_StairsNativeBitmapIndex_Up_Front_D3C",
        "G0683_i_StairsNativeBitmapIndex_Down_Front_D3C",
        "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
    ],
}


def fail(message: str) -> None:
    raise SystemExit(f"error: {message}")


def load_manifest_assets() -> dict[str, dict]:
    assets: dict[str, dict] = {}
    for path in sorted((ROOT / "assets-v2/manifests").glob("firestaff-v2-*.manifest.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        for asset in data.get("assets", []):
            if isinstance(asset, dict) and isinstance(asset.get("id"), str):
                assets.setdefault(asset["id"], asset)
    return assets


def load_catalog_bindings() -> dict[str, str]:
    data = json.loads((ROOT / "assets-v2/catalog/logical-ids/shared-v2-logical-ids.json").read_text(encoding="utf-8"))
    bindings: dict[str, str] = {}
    for category in data.get("categories", []):
        for entry in category.get("entries", []):
            logical_id = entry.get("logicalId")
            existing = entry.get("binding", {}).get("existingManifestId")
            if logical_id in REQUIRED_LOGICAL_IDS:
                if not isinstance(existing, str) or not existing:
                    fail(f"{logical_id} does not bind to binding.existingManifestId")
                bindings[logical_id] = existing
    missing = sorted(set(REQUIRED_LOGICAL_IDS) - set(bindings))
    if missing:
        fail(f"required logical IDs missing from catalog: {missing}")
    return bindings


def has_source_evidence(asset: dict) -> bool:
    source = asset.get("sourceReference")
    return isinstance(source, dict) and isinstance(source.get("sourceEvidence"), list) and bool(source["sourceEvidence"])


def run_catalog_gate() -> str:
    cmd = [sys.executable, str(ROOT / "scripts/validate_v2_logical_catalog.py")]
    for logical_id in REQUIRED_LOGICAL_IDS:
        cmd += ["--require-existing-manifest-binding", logical_id]
        cmd += ["--require-bound-manifest-source-evidence", logical_id]
    completed = subprocess.run(cmd, cwd=ROOT, check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return completed.stdout.strip()


def check_redmcsb_markers() -> dict[str, list[str]]:
    found: dict[str, list[str]] = {}
    for filename, markers in REQUIRED_SOURCE_MARKERS.items():
        text = (REDMCSB / filename).read_text(encoding="utf-8", errors="replace")
        hits = []
        for marker in markers:
            if marker not in text:
                fail(f"ReDMCSB {filename} missing marker {marker}")
            hits.append(marker)
        found[filename] = hits
    return found


def main() -> int:
    assets = load_manifest_assets()
    bindings = load_catalog_bindings()
    for logical_id, asset_id in bindings.items():
        asset = assets.get(asset_id)
        if asset is None:
            fail(f"{logical_id} binds to missing manifest asset {asset_id}")
        if not has_source_evidence(asset):
            fail(f"{logical_id} bound asset lacks sourceReference.sourceEvidence: {asset_id}")
    catalog_stdout = run_catalog_gate()
    markers = check_redmcsb_markers()
    result = {
        "status": "passed",
        "scope": "DM1 V2 dungeon-view source-evidenced asset bindings",
        "logicalBindings": bindings,
        "catalogGate": catalog_stdout,
        "redmcsbSourceRoot": str(REDMCSB),
        "redmcsbMarkers": markers,
        "completionImpact": "Closes one pass267 asset-binding gap for wall/front, wall/side, floor, ceiling, door/wood.closed and stairs/down logical IDs. Does not prove final rebuilt art, screenshot parity, or full viewport composition.",
    }
    VERIFY_OUT.parent.mkdir(parents=True, exist_ok=True)
    VERIFY_OUT.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"verified {len(bindings)} DM1 V2 dungeon-view source-evidenced bindings; evidence={VERIFY_OUT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
