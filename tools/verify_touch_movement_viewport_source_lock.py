#!/usr/bin/env python3
"""Verify the narrow ReDMCSB source-lock for movement/viewport touch zones.

This is a provenance gate only. It does not drive input and does not claim any
new runtime behavior; it checks that Firestaff's starter touch/click matrix keeps
the eight original COMMAND.C movement/viewport routes source-locked to the I34E
layout-696 zone coordinates.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))
REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
COMMAND_C = REDMCSB_SOURCE / "COMMAND.C"
COORD_C = REDMCSB_SOURCE / "COORD.C"
ZONES_JSON = ROOT / "zones_h_reconstruction.json"
MATRIX_C = ROOT / "touch_click_zone_matrix_pc34_compat.c"
DEFAULT_OUT = ROOT / "parity-evidence/verification/touch_movement_viewport_source_lock.json"

EXPECTED = [
    {"command": 1, "zone": 68, "name": "movement.turn_left", "x": 234, "y": 125, "w": 28, "h": 21},
    {"command": 3, "zone": 70, "name": "movement.forward", "x": 263, "y": 125, "w": 27, "h": 21},
    {"command": 2, "zone": 69, "name": "movement.turn_right", "x": 291, "y": 125, "w": 28, "h": 21},
    {"command": 6, "zone": 73, "name": "movement.left", "x": 234, "y": 147, "w": 28, "h": 21},
    {"command": 5, "zone": 72, "name": "movement.backward", "x": 263, "y": 147, "w": 27, "h": 21},
    {"command": 4, "zone": 71, "name": "movement.right", "x": 291, "y": 147, "w": 28, "h": 21},
    {"command": 80, "zone": 7, "name": "viewport.dungeon", "x": 0, "y": 33, "w": 224, "h": 136},
    {"command": 83, "zone": 2, "name": "inventory.toggle_leader", "x": 0, "y": 0, "w": 320, "h": 200, "button": "RIGHT"},
]

ROUTE_SYMBOLS = {
    1: "C001_COMMAND_TURN_LEFT",
    3: "C003_COMMAND_MOVE_FORWARD",
    2: "C002_COMMAND_TURN_RIGHT",
    6: "C006_COMMAND_MOVE_LEFT",
    5: "C005_COMMAND_MOVE_BACKWARD",
    4: "C004_COMMAND_MOVE_RIGHT",
    80: "C080_COMMAND_CLICK_IN_DUNGEON_VIEW",
    83: "C083_COMMAND_TOGGLE_INVENTORY_LEADER",
}

ZONE_SYMBOLS = {
    68: "C068_ZONE_TURN_LEFT",
    70: "C070_ZONE_MOVE_FORWARD",
    69: "C069_ZONE_TURN_RIGHT",
    73: "C073_ZONE_MOVE_LEFT",
    72: "C072_ZONE_MOVE_BACKWARD",
    71: "C071_ZONE_MOVE_RIGHT",
    7: "C007_ZONE_VIEWPORT",
    2: "C002_ZONE_SCREEN",
}

LEFT_BUTTON = "MASK0x0002_MOUSE_LEFT_BUTTON"
RIGHT_BUTTON = "MASK0x0001_MOUSE_RIGHT_BUTTON"


def read_lines(path: Path) -> list[str]:
    try:
        return path.read_text(encoding="latin-1").splitlines()
    except FileNotFoundError as exc:
        raise SystemExit(f"missing required source file: {path}") from exc


def line_range(lines: list[str], start: int, end: int) -> str:
    return "\n".join(lines[start - 1 : end])


def verify_command_table(lines: list[str]) -> list[dict[str, Any]]:
    block = line_range(lines, 396, 405)
    rows: list[dict[str, Any]] = []
    for item in EXPECTED:
        cmd = ROUTE_SYMBOLS[item["command"]]
        zone = ZONE_SYMBOLS[item["zone"]]
        button = RIGHT_BUTTON if item.get("button") == "RIGHT" else LEFT_BUTTON
        pattern = rf"\{{\s*{cmd}\s*,\s*CM1_SCREEN_RELATIVE\s*,\s*{zone}\s*,\s*0\s*,\s*0\s*,\s*{button}\s*\}}"
        ok = re.search(pattern, block) is not None
        rows.append({
            "name": item["name"],
            "command": item["command"],
            "zone": item["zone"],
            "button": "right" if button == RIGHT_BUTTON else "left",
            "command_c_line_range": "COMMAND.C:396-405",
            "route_found": ok,
        })
        if not ok:
            raise SystemExit(f"COMMAND.C:396-405 missing route {cmd} -> {zone} / {button}")
    return rows


def verify_dispatch_and_coord(lines_command: list[str], lines_coord: list[str]) -> list[str]:
    required = [
        ("COMMAND.C:1403-1431", line_range(lines_command, 1403, 1431), [
            "while (L1107_Command = P0721_ps_MouseInput->Command)",
            "P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button",
            "F0798_COMMAND_IsPointInZone",
        ]),
        ("COMMAND.C:1641-1644", line_range(lines_command, 1641, 1644), [
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput",
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
        ]),
        ("COORD.C:1915-1920", line_range(lines_coord, 1915, 1920), [
            "BOOLEAN F0798_COMMAND_IsPointInZone",
            "P0750_i_X >= M704_ZONE_LEFT",
            "P0751_i_Y <= M707_ZONE_BOTTOM",
        ]),
    ]
    citations: list[str] = []
    for cite, block, needles in required:
        compact = " ".join(block.split())
        for needle in needles:
            if " ".join(needle.split()) not in compact:
                raise SystemExit(f"{cite} missing expected text: {needle}")
        citations.append(cite)
    return citations


def resolve_zone(records: dict[int, dict[str, int]], zone: int) -> dict[str, int]:
    """Resolve a layout-696 zone rectangle using the repo COORD.C parity port."""
    from tools.resolve_dm1_zone import resolve

    rec = records[zone]
    parent = records[rec["parent"]]
    if parent["type"] != 9:
        raise SystemExit(f"zone {zone} expected type-9 size parent, got {parent['type']}")
    actual = resolve(zone, parent["d1"], parent["d2"])
    if actual is None:
        raise SystemExit(f"zone {zone} did not resolve")
    return {"x": actual["x"], "y": actual["y"], "w": actual["w"], "h": actual["h"]}


def verify_zones() -> list[dict[str, Any]]:
    data = json.loads(ZONES_JSON.read_text())
    records = {int(k): v for k, v in data["records"].items()}
    rows: list[dict[str, Any]] = []
    for item in EXPECTED:
        actual = resolve_zone(records, item["zone"])
        expected = {k: item[k] for k in ("x", "y", "w", "h")}
        if actual != expected:
            raise SystemExit(f"zone {item['zone']} mismatch: expected {expected}, got {actual}")
        rows.append({
            "name": item["name"],
            "zone": item["zone"],
            "layout_696_rect": actual,
            "layout_citation": f"zones_h_reconstruction.json records[{item['zone']}] + parent size record",
        })
    return rows


def verify_firestaff_matrix() -> list[dict[str, Any]]:
    text = MATRIX_C.read_text()
    rows: list[dict[str, Any]] = []
    for item in EXPECTED:
        button = "TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT" if item.get("button") == "RIGHT" else "TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT"
        pattern = (
            rf"\{{\s*{item['command']}u\s*,\s*{item['zone']}u\s*,\s*"
            rf"TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT\s*,\s*{button}\s*,\s*"
            rf"{item['x']}\s*,\s*{item['y']}\s*,\s*{item['w']}\s*,\s*{item['h']}\s*,\s*\"{re.escape(item['name'])}\""
        )
        ok = re.search(pattern, text) is not None
        rows.append({"name": item["name"], "matrix_entry_found": ok})
        if not ok:
            raise SystemExit(f"Firestaff matrix missing exact {item['name']} entry")
    return rows


def build_report() -> dict[str, Any]:
    command_lines = read_lines(COMMAND_C)
    coord_lines = read_lines(COORD_C)
    routes = verify_command_table(command_lines)
    dispatch_citations = verify_dispatch_and_coord(command_lines, coord_lines)
    zones = verify_zones()
    matrix = verify_firestaff_matrix()
    return {
        "status": "pass",
        "scope": "narrow movement/viewport UI hit-zone source lock for future touch/click abstraction",
        "non_claims": [
            "No input behavior is changed by this verifier.",
            "This does not source-lock V2 inventory art, inventory slot gates, or HUD rendering behavior.",
            "This does not prove original runtime delivery of queued C080 side effects; it only locks route geometry/provenance.",
        ],
        "source_root": str(REDMCSB_SOURCE),
        "source_citations": [
            "COMMAND.C:396-405 G0448_as_Graphic561_SecondaryMouseInput_Movement routes left/right movement and viewport/toggle commands to zones/buttons",
            "COMMAND.C:1403-1431 F0358_COMMAND_GetCommandFromMouseInput_CPSC scans mouse-input entries and tests button/zone matches",
            "COMMAND.C:1641-1644 F0359_COMMAND_ProcessClick_CPSC checks primary then secondary mouse tables",
            "COORD.C:1915-1920 F0798_COMMAND_IsPointInZone inclusive zone hit-test",
        ],
        "dispatch_citations_verified": dispatch_citations,
        "routes": routes,
        "zones": zones,
        "firestaff_matrix": matrix,
        "verified_files": [
            str(COMMAND_C),
            str(COORD_C),
            str(ZONES_JSON),
            str(MATRIX_C),
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", type=Path, nargs="?", const=DEFAULT_OUT, help="write JSON report (default path if no value)")
    args = parser.parse_args()
    report = build_report()
    output = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.write:
        args.write.parent.mkdir(parents=True, exist_ok=True)
        args.write.write_text(output)
    print(output, end="")
    return 0


if __name__ == "__main__":
    sys.exit(main())
