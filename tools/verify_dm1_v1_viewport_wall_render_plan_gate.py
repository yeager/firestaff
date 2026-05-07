#!/usr/bin/env python3
"""Build a DM1 V1 viewport wall render-plan comparator artifact.

This is the narrow seam between the existing PC34 wall/occlusion metadata and
actual render/comparator evidence.  It consumes the source-locked pass127
movement viewport snapshots, projects visible wall squares through the local
PC34 wall draw-spec table, and writes a deterministic render event list that a
pixel comparator can consume later.

It deliberately does not claim original pixel parity: original runtime capture
is still blocked on verified FIRES.MAP/debugger bindings.  The value here is
that wall metadata now has a tested render-plan output instead of stopping at
static table assertions.
"""
from __future__ import annotations

import json
from pathlib import Path
import sys
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS127 = ROOT / "parity-evidence/verification/pass127_turn_viewport_orientation_probe.json"
LOCAL = ROOT / "dm1_v1_viewport_3d_pc34_compat.c"
REDMCSB_DUNVIEW = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source/DUNVIEW.C")
OUT = ROOT / "parity-evidence/verification/dm1_v1_viewport_wall_render_plan_gate.json"
REPORT = ROOT / "parity-evidence/dm1_v1_viewport_wall_render_plan_gate.md"

DRAW_ORDER = [
    "D4L", "D4R", "D4C",
    "D3L2", "D3R2", "D3L", "D3R", "D3C",
    "D2L2", "D2R2", "D2L", "D2R", "D2C",
    "D1L", "D1R", "D1C", "D0L", "D0R", "D0C",
]

# Local mirror of DM1_ViewportWallDrawSpec.  The gate below verifies these
# identifiers are present in the C table so drift fails loudly.
#
# The graphic indices and viewport rectangles are the next comparator seam:
# ReDMCSB DUNVIEW.C selects G2107_WallSet[Cxx_WALL_*] and layout zones
# C702..C717; COORD.C F0635_ resolves those zones into viewport-relative
# destination rectangles.  These descriptors are still expected-region
# metadata, not original pixel parity.
WALL_REGIONS: dict[str, dict[str, int]] = {
    "D3L2": {"graphic": 104, "x": 0,   "y": 25, "width": 44,  "height": 49,  "srcX": 0, "srcY": 0},
    "D3R2": {"graphic": 103, "x": 180, "y": 25, "width": 44,  "height": 49,  "srcX": 0, "srcY": 0},
    "D3C":  {"graphic": 107, "x": 77,  "y": 25, "width": 70,  "height": 49,  "srcX": 0, "srcY": 0},
    "D3L":  {"graphic": 106, "x": 7,   "y": 25, "width": 83,  "height": 49,  "srcX": 0, "srcY": 0},
    "D3R":  {"graphic": 105, "x": 134, "y": 25, "width": 83,  "height": 49,  "srcX": 0, "srcY": 0},
    "D2L2": {"graphic": 99,  "x": 0,   "y": 24, "width": 8,   "height": 52,  "srcX": 0, "srcY": 0},
    "D2R2": {"graphic": 98,  "x": 216, "y": 24, "width": 8,   "height": 52,  "srcX": 0, "srcY": 0},
    "D2C":  {"graphic": 102, "x": 59,  "y": 19, "width": 106, "height": 74,  "srcX": 0, "srcY": 0},
    "D2L":  {"graphic": 101, "x": 0,   "y": 19, "width": 78,  "height": 74,  "srcX": 0, "srcY": 0},
    "D2R":  {"graphic": 100, "x": 146, "y": 19, "width": 78,  "height": 74,  "srcX": 0, "srcY": 0},
    "D1C":  {"graphic": 97,  "x": 32,  "y": 9,  "width": 160, "height": 111, "srcX": 0, "srcY": 0},
    "D1L":  {"graphic": 96,  "x": 0,   "y": 9,  "width": 60,  "height": 111, "srcX": 0, "srcY": 0},
    "D1R":  {"graphic": 95,  "x": 164, "y": 9,  "width": 60,  "height": 111, "srcX": 0, "srcY": 0},
    "D0L":  {"graphic": 94,  "x": 0,   "y": 0,  "width": 33,  "height": 136, "srcX": 0, "srcY": 0},
    "D0R":  {"graphic": 93,  "x": 191, "y": 0,  "width": 33,  "height": 136, "srcX": 0, "srcY": 0},
}

WALL_SPECS: dict[str, dict[str, Any]] = {
    "D3L2": {"native": "DM1_WALL_D3L2", "parity": "DM1_WALL_D3R2", "zone": 702, "flip": True, "center": False, "returns": True,  "alcove": False, "source": "DUNVIEW.C:6254-6260"},
    "D3R2": {"native": "DM1_WALL_D3R2", "parity": "DM1_WALL_D3L2", "zone": 703, "flip": True, "center": False, "returns": True,  "alcove": False, "source": "DUNVIEW.C:6321-6327"},
    "D3L":  {"native": "DM1_WALL_D3L",  "parity": "DM1_WALL_D3R",  "zone": 705, "flip": True, "center": False, "returns": True,  "alcove": True,  "source": "DUNVIEW.C:6421-6427"},
    "D3R":  {"native": "DM1_WALL_D3R",  "parity": "DM1_WALL_D3L",  "zone": 706, "flip": True, "center": False, "returns": True,  "alcove": True,  "source": "DUNVIEW.C:6554-6564"},
    "D3C":  {"native": "DM1_WALL_D3C",  "parity": "DM1_WALL_D3C",  "zone": 704, "flip": False,"center": True,  "returns": True,  "alcove": True,  "source": "DUNVIEW.C:6707-6714"},
    "D2L2": {"native": "DM1_WALL_D2L2", "parity": "DM1_WALL_D2R2", "zone": 707, "flip": True, "center": False, "returns": True,  "alcove": False, "source": "DUNVIEW.C:6849-6858"},
    "D2R2": {"native": "DM1_WALL_D2R2", "parity": "DM1_WALL_D2L2", "zone": 708, "flip": True, "center": False, "returns": True,  "alcove": False, "source": "DUNVIEW.C:6880-6889"},
    "D2L":  {"native": "DM1_WALL_D2L",  "parity": "DM1_WALL_D2R",  "zone": 710, "flip": True, "center": False, "returns": True,  "alcove": True,  "source": "DUNVIEW.C:6954-6964"},
    "D2R":  {"native": "DM1_WALL_D2R",  "parity": "DM1_WALL_D2L",  "zone": 711, "flip": True, "center": False, "returns": True,  "alcove": True,  "source": "DUNVIEW.C:7105-7115"},
    "D2C":  {"native": "DM1_WALL_D2C",  "parity": "DM1_WALL_D2C",  "zone": 709, "flip": False,"center": True,  "returns": True,  "alcove": True,  "source": "DUNVIEW.C:7299-7306"},
    "D1L":  {"native": "DM1_WALL_D1L",  "parity": "DM1_WALL_D1R",  "zone": 713, "flip": True, "center": False, "returns": True,  "alcove": False, "source": "DUNVIEW.C:7445-7455"},
    "D1R":  {"native": "DM1_WALL_D1R",  "parity": "DM1_WALL_D1L",  "zone": 714, "flip": True, "center": False, "returns": True,  "alcove": False, "source": "DUNVIEW.C:7613-7623"},
    "D1C":  {"native": "DM1_WALL_D1C",  "parity": "DM1_WALL_D1C",  "zone": 712, "flip": False,"center": True,  "returns": False, "alcove": True,  "source": "DUNVIEW.C:7833-7840"},
    "D0L":  {"native": "DM1_WALL_D0L",  "parity": "DM1_WALL_D0R",  "zone": 716, "flip": True, "center": False, "returns": True,  "alcove": False, "source": "DUNVIEW.C:8016-8033"},
    "D0R":  {"native": "DM1_WALL_D0R",  "parity": "DM1_WALL_D0L",  "zone": 717, "flip": True, "center": False, "returns": True,  "alcove": False, "source": "DUNVIEW.C:8126-8139"},
}

EXPECTED_BY_SNAPSHOT = {
    "start_south": ["D4L", "D4R", "D3R2", "D3L", "D3C", "D2R2", "D2L", "D2R", "D1C", "D0L"],
    "turn_right_west": ["D4L", "D4R", "D4C", "D3L2", "D3R2", "D3L", "D3R", "D3C", "D2L2", "D2R2", "D2L", "D2R", "D2C", "D1R", "D0L", "D0R"],
    "move_forward_west": ["D4L", "D4R", "D4C", "D3L2", "D3R2", "D3L", "D3R", "D3C", "D2L2", "D2R2", "D2L", "D2R", "D2C", "D1L", "D1R", "D1C", "D0R"],
    "turn_left_east": ["D4R", "D4C", "D3L2", "D3R2", "D2L", "D2R", "D2C", "D1C", "D0L", "D0R"],
    "blocked_forward_south_wall": ["D4L", "D4R", "D3R2", "D3L", "D3C", "D2R2", "D2L", "D2R", "D1C", "D0L"],
}


THING_LAYERS: list[dict[str, Any]] = [
    {"layer": "objects", "source": "DUNVIEW.C:4567-4571,4853-4860"},
    {"layer": "creatures", "source": "DUNVIEW.C:4573,5195-5202", "d4Suppressed": True},
    {"layer": "projectiles", "source": "DUNVIEW.C:4575-4577,5679-5683"},
    {"layer": "explosions", "source": "DUNVIEW.C:4579-4581,5916-5933"},
]

D4_OBJECT_SPECS: dict[str, dict[str, Any]] = {
    "D4L": {"viewSquare": "M598_VIEW_SQUARE_D4L", "cellOrder": "C0x0001_CELL_ORDER_BACKLEFT", "source": "DUNVIEW.C:8468-8469", "relDepth": 4, "relLateral": -1},
    "D4R": {"viewSquare": "M599_VIEW_SQUARE_D4R", "cellOrder": "C0x0001_CELL_ORDER_BACKLEFT", "source": "DUNVIEW.C:8472-8473", "relDepth": 4, "relLateral": 1},
    "D4C": {"viewSquare": "M597_VIEW_SQUARE_D4C", "cellOrder": "C0x0001_CELL_ORDER_BACKLEFT", "source": "DUNVIEW.C:8476-8477", "relDepth": 4, "relLateral": 0},
}

D4_FORBIDDEN_WALL_NEEDLES = (
    "F0100_DUNGEONVIEW_DrawWallSetBitmap",
    "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap",
    "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally",
    "F0792_VIDEO_BlitShrinkWithPaletteChanges",
    "C702_ZONE_WALL_D3L2",
    "C703_ZONE_WALL_D3R2",
)


def require_local_table_matches() -> None:
    text = LOCAL.read_text(encoding="utf-8")
    failures: list[str] = []
    for row, spec in WALL_SPECS.items():
        square = f"DM1_VIEW_SQUARE_{row}"
        if square not in text:
            failures.append(f"missing local square {square}")
        for wall in (spec["native"], spec["parity"]):
            if wall not in text:
                failures.append(f"missing local wall id {wall}")
        if f"DM1_PC34_ZONE_WALL_{row}" not in text:
            failures.append(f"missing local zone id for {row}")
        if spec["source"] not in text:
            failures.append(f"missing local source citation {spec['source']}")
        region = WALL_REGIONS.get(row)
        if region is None:
            failures.append(f"missing comparator pixel region for {row}")
        else:
            if not (0 <= region["x"] < 224 and 0 < region["width"] <= 224 - region["x"]):
                failures.append(f"bad viewport x/width for {row}: {region}")
            if not (0 <= region["y"] < 136 and 0 < region["height"] <= 136 - region["y"]):
                failures.append(f"bad viewport y/height for {row}: {region}")
    if failures:
        raise SystemExit("local wall table drift:\n- " + "\n- ".join(failures))


def pixel_region_descriptor(row_name: str, spec: dict[str, Any], selected_wall: str) -> dict[str, Any]:
    region = WALL_REGIONS[row_name]
    return {
        "kind": "expected_wall_bitmap_region",
        "viewport": {"width": 224, "height": 136, "origin": "DUNVIEW.C viewport bitmap G0296_puc_Bitmap_Viewport"},
        "rect": {
            "x": region["x"],
            "y": region["y"],
            "width": region["width"],
            "height": region["height"],
        },
        "sourceClip": {
            "x": region["srcX"],
            "y": region["srcY"],
            "width": region["width"],
            "height": region["height"],
        },
        "zone": spec["zone"],
        "layout": 696,
        "wallSetGraphicIndex": region["graphic"],
        "selectedWall": selected_wall,
        "expectedPixelsFrom": "GRAPHICS.DAT wall-set bitmap bytes after DM1 PC34 palette decode",
        "source": "COORD.C:F0635_ layout-696 zone resolution; DUNVIEW.C wall draw call cited by event.source",
    }


def require_d4_source_has_no_wall_draws() -> dict[str, Any]:
    text = REDMCSB_DUNVIEW.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    d4_window = "\n".join(lines[8465:8477])
    failures: list[str] = []
    for row, spec in D4_OBJECT_SPECS.items():
        if spec["viewSquare"] not in d4_window:
            failures.append(f"{row} missing {spec['viewSquare']} in D4 F0128 window")
        if spec["cellOrder"] not in d4_window:
            failures.append(f"{row} missing {spec['cellOrder']} in D4 F0128 window")
    for forbidden in D4_FORBIDDEN_WALL_NEEDLES:
        if forbidden in d4_window:
            failures.append(f"D4 F0128 window unexpectedly contains wall draw needle {forbidden}")
    if failures:
        raise SystemExit("D4 source audit drift:\n- " + "\n- ".join(failures))
    return {
        "redmcsbSource": str(REDMCSB_DUNVIEW),
        "auditedRange": "DUNVIEW.C:8466-8477",
        "predicate": "D4L/D4R/D4C call F0150 then F0115 with BACKLEFT cell order only; the D4 window contains no local PC34 wall bitmap/zone draw call",
        "rows": D4_OBJECT_SPECS,
    }


def render_events(snapshot: dict[str, Any]) -> list[dict[str, Any]]:
    parity = ((int(snapshot["mapX"]) + int(snapshot["mapY"]) + int(snapshot["direction"])) & 1) != 0
    rows = {row["row"]: row for row in snapshot["viewportRows"]}
    events: list[dict[str, Any]] = []
    for row_name in DRAW_ORDER:
        cell = rows[row_name]
        if int(cell.get("elementType", -1)) != 0:
            continue
        spec = WALL_SPECS.get(row_name)
        if not spec:
            d4_spec = D4_OBJECT_SPECS.get(row_name)
            if d4_spec:
                events.append({
                    "row": row_name,
                    "event": "draw_d4_f0115_object_stack",
                    "mapX": cell["mapX"],
                    "mapY": cell["mapY"],
                    "valid": cell["valid"],
                    "viewSquare": d4_spec["viewSquare"],
                    "cellOrder": d4_spec["cellOrder"],
                    "thingLayers": [layer["layer"] for layer in THING_LAYERS],
                    "localWallSpecRequired": False,
                    "source": d4_spec["source"],
                    "sourcePredicate": "F0128 D4 path has no wall bitmap/zone draw; it directly delegates visible content to F0115",
                    "pixelRegion": None,
                    "comparatorBoundary": "content event only; compare through F0115 object/creature/projectile/explosion regions, not wall bitmap regions",
                })
                continue
            events.append({
                "row": row_name,
                "event": "wall_visible_without_local_pc34_wall_spec",
                "mapX": cell["mapX"],
                "mapY": cell["mapY"],
                "valid": cell["valid"],
                "blocker": "needs D0C current-square content render seam before pixel comparator parity",
            })
            continue
        selected = spec["parity"] if parity else spec["native"]
        events.append({
            "row": row_name,
            "event": "draw_wall_bitmap",
            "mapX": cell["mapX"],
            "mapY": cell["mapY"],
            "valid": cell["valid"],
            "pc34Zone": spec["zone"],
            "nativeWall": spec["native"],
            "parityWall": spec["parity"],
            "selectedWall": selected,
            "flipHorizontally": bool(parity and spec["flip"]),
            "centerWall": spec["center"],
            "wallCaseReturns": spec["returns"],
            "frontAlcoveMayRevealContents": spec["alcove"],
            "source": spec["source"],
            "pixelRegion": pixel_region_descriptor(row_name, spec, selected),
        })
    return events


def compare_snapshot(snapshot: dict[str, Any]) -> dict[str, Any]:
    events = render_events(snapshot)
    wall_spec_rows = [e["row"] for e in events if e["event"] == "draw_wall_bitmap"]
    unsupported_rows = [e["row"] for e in events if e["event"] == "wall_visible_without_local_pc34_wall_spec"]
    d4_object_rows = [e["row"] for e in events if e["event"] == "draw_d4_f0115_object_stack"]
    expected = EXPECTED_BY_SNAPSHOT[snapshot["name"]]
    covered_rows = wall_spec_rows + d4_object_rows + unsupported_rows
    missing = [r for r in expected if r not in covered_rows]
    unexpected = [r for r in covered_rows if r not in expected]
    if missing or unexpected:
        snap_name = snapshot.get(chr(110)+chr(97)+chr(109)+chr(101))
        raise SystemExit(repr((snap_name, missing, unexpected, covered_rows, expected)))
    return {
        "name": snapshot["name"],
        "party": {"mapIndex": snapshot["mapIndex"], "mapX": snapshot["mapX"], "mapY": snapshot["mapY"], "direction": snapshot["direction"]},
        "parityFlip": ((int(snapshot["mapX"]) + int(snapshot["mapY"]) + int(snapshot["direction"])) & 1) != 0,
        "expectedWallRows": expected,
        "renderEvents": events,
        "d4ObjectRows": d4_object_rows,
        "unsupportedWallRows": unsupported_rows,
        "status": "PASS_RENDER_PLAN_WITH_UNSUPPORTED_D4_D0C_BLOCKERS" if unsupported_rows else "PASS_RENDER_PLAN",
    }


def main() -> int:
    require_local_table_matches()
    d4_source_audit = require_d4_source_has_no_wall_draws()
    doc = json.loads(PASS127.read_text(encoding="utf-8"))
    if doc.get("schema") != "pass127_turn_viewport_orientation_probe.v4":
        raise SystemExit(f"unexpected pass127 schema {doc.get('schema')!r}")
    if doc.get("viewportRowOrder") != ",".join(DRAW_ORDER):
        raise SystemExit("pass127 viewport row order no longer matches F0128 comparator order")
    snapshots = {s["name"]: s for s in doc["snapshots"]}
    missing_snapshots = [name for name in EXPECTED_BY_SNAPSHOT if name not in snapshots]
    if missing_snapshots:
        raise SystemExit(f"missing snapshots: {missing_snapshots}")
    compared = [compare_snapshot(snapshots[name]) for name in EXPECTED_BY_SNAPSHOT]
    unsupported = sorted({row for c in compared for row in c["unsupportedWallRows"]})
    status = "PASS_RENDER_PLAN_COMPARATOR_SEAM"
    if unsupported:
        status = "PASS_RENDER_PLAN_COMPARATOR_SEAM_WITH_KNOWN_D4_D0C_BLOCKERS"
    result = {
        "schema": "dm1_v1_viewport_wall_render_plan_gate.v1",
        "status": status,
        "sourceInputs": {
            "pass127": "parity-evidence/verification/pass127_turn_viewport_orientation_probe.json",
            "localWallMetadata": "dm1_v1_viewport_3d_pc34_compat.c",
            "redmcsbOrder": "DUNVIEW.C:8318-8542 F0128 far-to-near draw order",
            "redmcsbWallSpecs": "DUNVIEW.C:6254-8144 PC34 wall cases and occlusion returns",
            "redmcsbZoneDefines": "DEFS.H:4042-4057 C702..C717 wall-zone IDs",
            "redmcsbZoneResolver": "COORD.C:F0635_ layout-696 zone-to-viewport clipping semantics",
            "regionDerivation": "tools/resolve_dm1_zone.py + parity-evidence/dm1_all_graphics_phase10_source_front_walls.md + parity-evidence/dm1_all_graphics_phase11_source_side_walls.md",
        },
        "d4SourceAudit": d4_source_audit,
        "boundary": {
            "nowProduced": "deterministic wall render events plus comparator-ready expected wall pixel-region descriptors (viewport rect, source clip, layout zone, wall-set graphic index), with D4 object-stack events sourced to F0128/F0115",
            "notClaimed": "original pixel parity, screenshot comparison, or decoded GRAPHICS.DAT byte equality",
            "d4Decision": "D4 needs no local PC34 wall draw specs: F0128 has no D4 wall bitmap/zone branch and only calls F0115 object/content rendering with BACKLEFT cell order",
            "remainingBlocker": "unsupported D0C current-square content must be modeled before a full pixel comparator can be promoted" if unsupported else None,
        },
        "comparedSnapshots": compared,
        "unsupportedWallRows": unsupported,
        "pixelComparatorPromotionBlockers": [
            "Need captured original DM1 PC34 viewport pixels for the same pass127 route snapshots, including crop sha256 and palette provenance",
            "Need decoded GRAPHICS.DAT wall-set bitmap bytes keyed by selectedWall/wallSetGraphicIndex with asset sha256 and palette table",
            "Need a comparator harness to blit expected wall regions over the floor/ceiling base and then apply F0115 content layers for D4/content events",
        ],
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text(
        "# DM1 V1 viewport wall render-plan comparator gate\n\n"
        f"Status: `{status}`\n\n"
        "This fixes the immediate metadata-only seam: source-locked wall/occlusion metadata now produces deterministic render events and comparator-ready expected wall pixel-region descriptors from the pass127 viewport snapshots. It does not claim original pixel parity.\n\n"
        "## Evidence\n\n"
        f"- JSON artifact: `{OUT.relative_to(ROOT)}`\n"
        f"- Snapshots compared: {list(EXPECTED_BY_SNAPSHOT)}\n"
        f"- Unsupported visible wall rows still blocking full pixel promotion: `{unsupported}`\n"
        "- Region descriptors: viewport rect, source clip, layout 696 zone, selected wall bitmap, and wall-set graphic index for every D3/D2/D1/D0 wall event\n\n"
        "## Boundary\n\n"
        "D4 rows are emitted as F0115 object-stack events, not missing wall specs: the audited DUNVIEW.C:8466-8477 window has only F0150 coordinate updates and F0115 calls for D4L/D4R/D4C. No unsupported visible wall rows remain in the pass127 comparator snapshots. PC34 D3/D2/D1/D0 side/front wall rows now emit selected native/parity wall bitmap, flip flag, zone, occlusion metadata, and comparator-ready expected pixel-region descriptors (viewport rect/source clip/layout 696/wall-set graphic index) in F0128 order. Remaining promotion blockers are exact original viewport pixel capture and decoded GRAPHICS.DAT bitmap/palette bytes; this gate still does not claim pixel parity.\n",
        encoding="utf-8",
    )
    print(f"PASS dm1_v1_viewport_wall_render_plan_gate wrote {OUT}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except OSError as exc:
        print(f"FAIL dm1_v1_viewport_wall_render_plan_gate: {exc}", file=sys.stderr)
        raise SystemExit(1)
