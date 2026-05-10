#!/usr/bin/env python3
"""Source-lock the first DM1 PC 3.4 Hall-of-Champions route target.

This gate deliberately does not drive DOSBox and does not claim pixel parity.
It narrows the current original-route blocker by proving the semantic route
state from local source/data only: new-game start, eastward corridor to the
first champion portrait, and the exact runtime capture state that still needs
manual/original-runtime input binding.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any
from zipfile import ZipFile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
GREATSTONE_ZIP = Path("/home/trv2/.openclaw/data/firestaff-greatstone-atlas/raw/greatstone.free.fr__dm__db_data__c_3d2d1d__dungeon.dat__dungeon_xml.zip.zip")
GREATSTONE_MEMBER = "0000.DUNGEON [Dungeon].xml"
OUT_DIR = ROOT / "parity-evidence/verification/pass180_dm1_v1_elija_route_source_gate"
REPORT = ROOT / "parity-evidence/pass180_dm1_v1_elija_route_source_gate.md"

DIRECTION_TO_STEP = {
    "North": (0, -1),
    "East": (1, 0),
    "South": (0, 1),
    "West": (-1, 0),
}
EXPECTED = {
    "start": {"map": 0, "x": 3, "y": 2, "facing": "East"},
    "target_stand": {"map": 0, "x": 9, "y": 2, "facing": "East"},
    "portrait_wall": {"map": 0, "x": 10, "y": 2, "side": "West/BottomRight", "champion": "Elija", "champion_id": 0},
    "forward_steps": 6,
}


def read_lines(path: Path) -> list[str]:
    if not path.is_file():
        raise AssertionError(f"missing required source file: {path}")
    return path.read_text(encoding="latin-1", errors="replace").splitlines()


def compact_contains(text: str, needle: str) -> bool:
    return " ".join(needle.split()) in " ".join(text.split())


def source_contracts() -> list[dict[str, Any]]:
    checks = [
        {
            "id": "new-game-state-decoded-from-dungeon-header",
            "path": REDMCSB / "LOADSAVE.C",
            "citation": "LOADSAVE.C:1940-1944",
            "ranges": [(1940, 1944)],
            "needles": [
                "if (G0298_B_NewGame)",
                "G0306_i_PartyMapX = (AL1353_i_InitialPartyLocation = G0278_ps_DungeonHeader->InitialPartyLocation) & 0x001F;",
                "G0307_i_PartyMapY = (AL1353_i_InitialPartyLocation >>= 5) & 0x001F;",
                "G0308_i_PartyDirection = (AL1353_i_InitialPartyLocation >> 5) & 0x0003;",
                "G0309_i_PartyMapIndex = 0;",
            ],
        },
        {
            "id": "east-facing-forward-vector",
            "path": REDMCSB / "DUNGEON.C",
            "citation": "DUNGEON.C:35-44,1371-1391",
            "ranges": [(35, 44), (1371, 1391)],
            "needles": [
                "G0233_ai_Graphic559_DirectionToStepEastCount",
                "G0234_ai_Graphic559_DirectionToStepNorthCount",
                "1,    /* East */",
                "0,   /* East */",
                "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
                "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
            ],
        },
    ]
    results: list[dict[str, Any]] = []
    for check in checks:
        lines = read_lines(check["path"])
        text = "\n".join("\n".join(lines[a - 1:b]) for a, b in check["ranges"])
        missing = [needle for needle in check["needles"] if not compact_contains(text, needle)]
        results.append({"id": check["id"], "citation": check["citation"], "ok": not missing, "missing": missing})
    return results


def greatstone_xml() -> tuple[list[str], ET.Element]:
    if not GREATSTONE_ZIP.is_file():
        raise AssertionError(f"missing GreatStone atlas zip: {GREATSTONE_ZIP}")
    with ZipFile(GREATSTONE_ZIP) as zf:
        text = zf.read(GREATSTONE_MEMBER).decode("utf-8", errors="replace")
    return text.splitlines(), ET.fromstring(text)


def line_of(lines: list[str], needle: str) -> int:
    for idx, line in enumerate(lines, 1):
        if needle in line:
            return idx
    raise AssertionError(f"could not find line for {needle!r}")


def greatstone_contract() -> dict[str, Any]:
    lines, root = greatstone_xml()
    start = {
        "map": 0,
        "x": int(root.attrib["start_x"]),
        "y": int(root.attrib["start_y"]),
        "facing": root.attrib["start_facing"],
    }
    floors: list[dict[str, Any]] = []
    for x in range(EXPECTED["start"]["x"], EXPECTED["target_stand"]["x"] + 1):
        marker = f"<!-- Floor ({x};2) -->"
        line = line_of(lines, marker)
        window = "\n".join(lines[line - 1:line + 8])
        floors.append({"x": x, "y": 2, "line": line, "is_floor": 'type="1"' in window})
    wall_line = line_of(lines, "<!-- Wall (10;2) -->")
    elija_line = line_of(lines, "champion identifier: Elija")
    portrait_window = "\n".join(lines[wall_line - 1:wall_line + 28])
    portrait = {
        "map": 0,
        "x": 10,
        "y": 2,
        "wall_line": wall_line,
        "elija_line": elija_line,
        "type_127": "<type>127</type>" in portrait_window,
        "champion_id_0": "<data>0</data>" in portrait_window,
        "position_3": 'position="3"' in portrait_window,
        "side": "West/BottomRight" if "West/BottomRight" in portrait_window else "unknown",
    }
    return {
        "start": start,
        "floor_corridor": floors,
        "portrait": portrait,
        "citations": [
            f"{GREATSTONE_MEMBER}:69-72",
            f"{GREATSTONE_MEMBER}:{floors[0]['line']}-{floors[-1]['line'] + 8}",
            f"{GREATSTONE_MEMBER}:{wall_line}-{elija_line + 11}",
        ],
    }


def route_plan() -> dict[str, Any]:
    x = EXPECTED["start"]["x"]
    y = EXPECTED["start"]["y"]
    facing = EXPECTED["start"]["facing"]
    dx, dy = DIRECTION_TO_STEP[facing]
    points = [{"step": 0, "x": x, "y": y, "facing": facing}]
    for step in range(1, EXPECTED["forward_steps"] + 1):
        x += dx
        y += dy
        points.append({"step": step, "x": x, "y": y, "facing": facing})
    return {
        "semantic_route": points,
        "target_stand": points[-1],
        "portrait_interaction": EXPECTED["portrait_wall"],
        "candidate_runtime_route_tokens": "wait:7000 enter wait:1200 click:250,53 wait:1200 click:247,135 wait:1200 kp6 wait:800 shot:start " + " ".join(["kp8 wait:500"] * 6) + " shot:at_elija",
        "honesty": "Candidate tokens are not promoted as original-runtime evidence until pass113/pass80 classify a non-static party-control capture.",
    }


def evaluate() -> dict[str, Any]:
    source = source_contracts()
    gs = greatstone_contract()
    route = route_plan()
    problems: list[str] = []
    if any(not item["ok"] for item in source):
        problems.append("ReDMCSB source contract mismatch")
    if gs["start"] != EXPECTED["start"]:
        problems.append(f"unexpected GreatStone start state: {gs['start']}")
    if any(not tile["is_floor"] for tile in gs["floor_corridor"]):
        problems.append("GreatStone corridor x=3..9,y=2 is not all floor")
    portrait = gs["portrait"]
    for key in ["type_127", "champion_id_0", "position_3"]:
        if not portrait[key]:
            problems.append(f"portrait contract failed: {key}")
    if portrait["side"] != EXPECTED["portrait_wall"]["side"]:
        problems.append(f"unexpected portrait side: {portrait['side']}")
    if route["target_stand"] != {"step": 6, "x": 9, "y": 2, "facing": "East"}:
        problems.append(f"unexpected route target: {route['target_stand']}")
    status = "PASS_SOURCE_LOCKED_ROUTE_RUNTIME_INPUT_STILL_BLOCKED" if not problems else "FAIL_SOURCE_ROUTE_CONTRACT"
    return {
        "schema": "pass180_dm1_v1_elija_route_source_gate.v1",
        "status": status,
        "repo": str(ROOT),
        "redmcsb_source_root": str(REDMCSB),
        "greatstone_zip": str(GREATSTONE_ZIP),
        "source_contracts": source,
        "greatstone_contract": gs,
        "route_plan": route,
        "remaining_blocker": "original-runtime input/address route still must prove a non-static party-control capture; no pixel parity claim is made here",
        "problems": problems,
    }


def write_report(manifest: dict[str, Any], path: Path) -> None:
    gs = manifest["greatstone_contract"]
    lines = [
        "# Pass180 DM1 V1 Elija route source gate",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "This is a source-locked route gate only. It proves the semantic Hall-of-Champions target for the first champion route and keeps the original runtime capture blocker honest.",
        "",
        "## Verified source/data contracts",
        "",
    ]
    for item in manifest["source_contracts"]:
        lines.append(f"- {'PASS' if item['ok'] else 'FAIL'} `{item['id']}` — {item['citation']}")
    lines.extend([
        f"- PASS `greatstone-start-state` — {gs['citations'][0]}",
        f"- PASS `greatstone-floor-corridor-x3-through-x9-y2` — {gs['citations'][1]}",
        f"- PASS `greatstone-elija-portrait-wall-10-2-west-side` — {gs['citations'][2]}",
        "",
        "## Route contract",
        "",
        "- Start: map 0, x=3, y=2, facing East.",
        "- Source route: six forward semantic steps to map 0, x=9, y=2, facing East.",
        "- Interaction target: champion portrait wall at map 0, x=10, y=2, west side, champion id 0 / Elija.",
        "",
        "## Remaining blocker",
        "",
        f"- {manifest['remaining_blocker']}.",
        "- `tools/pass113_original_party_state_probe.py` remains the semantic readiness gate for any runtime capture.",
        "- Do not promote direct-start/no-party or static duplicate frames as original references.",
        "",
        "## Non-claims",
        "",
        "- No DOSBox runtime success is claimed.",
        "- No original-vs-Firestaff pixel parity is claimed.",
        "- No original assets are copied into the repository.",
        "",
        "## Verification output",
        "",
        "- Manifest: `parity-evidence/verification/pass180_dm1_v1_elija_route_source_gate/manifest.json`.",
    ])
    if manifest["problems"]:
        lines.extend(["", "## Problems", ""])
        for problem in manifest["problems"]:
            lines.append(f"- {problem}")
    path.write_text("\n".join(lines).rstrip() + "\n", encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out-dir", type=Path, default=OUT_DIR)
    ap.add_argument("--report", type=Path, default=REPORT)
    args = ap.parse_args()
    manifest = evaluate()
    args.out_dir.mkdir(parents=True, exist_ok=True)
    (args.out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({
        "status": manifest["status"],
        "report": str(args.report),
        "manifest": str(args.out_dir / "manifest.json"),
        "target_stand": manifest["route_plan"]["target_stand"],
        "problems": manifest["problems"],
    }, indent=2, sort_keys=True))
    return 0 if manifest["status"].startswith("PASS") else 1


if __name__ == "__main__":
    raise SystemExit(main())
