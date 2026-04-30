#!/usr/bin/env python3
"""Source-lock DM1/V1 inventory mouth/eye click routing to ReDMCSB."""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import sys
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REDMCSB_ROOT = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
REDMCSB_ROOT = Path(os.environ.get("REDMCSB_SOURCE_ROOT", DEFAULT_REDMCSB_ROOT))
FIRESTAFF_SRC = ROOT / "m11_game_view.c"
FIRESTAFF_PROBE = ROOT / "probes/m11/firestaff_m11_game_view_probe.c"
EVIDENCE_JSON = ROOT / "parity-evidence/verification/v1_inventory_mouth_eye_routing_redmcsb_gate.json"

SOURCE_RANGES = [
    {"file": "COMMAND.C", "start": 128, "end": 138},
    {"file": "COMMAND.C", "start": 420, "end": 428},
    {"file": "COMMAND.C", "start": 2314, "end": 2320},
    {"file": "PANEL.C", "start": 1743, "end": 1825},
    {"file": "PANEL.C", "start": 1928, "end": 1950},
    {"file": "PANEL.C", "start": 2111, "end": 2180},
    {"file": "m11_game_view.c", "start": 15569, "end": 15580},
    {"file": "m11_game_view.c", "start": 15646, "end": 15658},
    {"file": "probes/m11/firestaff_m11_game_view_probe.c", "start": 738, "end": 755},
]


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def read_text(path: Path) -> str:
    if not path.is_file():
        raise AssertionError(f"missing source file: {path}")
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def source_path(rel: str) -> Path:
    name = Path(rel).name
    if name == name.upper() and name.upper().endswith((".C", ".H")):
        return REDMCSB_ROOT / rel
    return ROOT / rel


def find_source_window(text: str, name: str, next_name: str | None = None) -> tuple[int, int, str]:
    prefix = r"(?m)^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|THING|unsigned\s+int16_t|static\s+(?:void|int))\s+"
    pattern = re.compile(prefix + re.escape(name) + r"\s*\(")
    match = pattern.search(text)
    if not match:
        raise AssertionError(f"missing function declaration for {name}")
    if next_name:
        next_pattern = re.compile(prefix + re.escape(next_name) + r"\s*\(")
        next_match = next_pattern.search(text, match.end())
        if not next_match:
            raise AssertionError(f"missing following function declaration for {next_name}")
        return match.start(), next_match.start(), text[match.start():next_match.start()]
    return match.start(), min(len(text), match.start() + 12000), text[match.start():min(len(text), match.start() + 12000)]


def require_in_order(body: str, markers: Iterable[tuple[str, str]], label: str) -> list[tuple[str, int]]:
    found: list[tuple[str, int]] = []
    cursor = -1
    for name, needle in markers:
        pos = body.find(needle, cursor + 1)
        if pos < 0:
            raise AssertionError(f"{label}: missing {name} marker {needle!r}")
        found.append((name, pos))
        cursor = pos
    return found


def require_excerpt(rel: str, start: int, end: int, needles: list[str]) -> None:
    path = source_path(rel)
    lines = read_text(path).splitlines()
    if len(lines) < end:
        raise AssertionError(f"{path} has only {len(lines)} lines, need {end}")
    excerpt = "\n".join(lines[start - 1:end])
    missing = [needle for needle in needles if needle not in excerpt]
    if missing:
        raise AssertionError(f"{rel}:{start}-{end} missing {missing}")
    print(f"sourceRange={path}:{start}-{end} status=ok")


def verify_redmcsb() -> list[str]:
    cmd_path = REDMCSB_ROOT / "COMMAND.C"
    cmd_text = read_text(cmd_path)
    dispatch_start, _dispatch_end, dispatch_body = find_source_window(
        cmd_text, "F0380_COMMAND_ProcessQueue_CPSC", "F1055_Post_F0380_COMMAND_ProcessQueue_CPSC"
    )
    dispatch_markers = require_in_order(
        dispatch_body,
        [
            ("mouth dispatch", "if (L1160_i_Command == C070_COMMAND_CLICK_ON_MOUTH)"),
            ("mouth handler", "F0349_INVENTORY_ProcessCommand70_ClickOnMouth()"),
            ("eye dispatch", "if (L1160_i_Command == C071_COMMAND_CLICK_ON_EYE)"),
            ("eye handler", "F0352_INVENTORY_ProcessCommand71_ClickOnEye()"),
        ],
        "ReDMCSB command dispatch mouth/eye",
    )

    panel_path = REDMCSB_ROOT / "PANEL.C"
    panel_text = read_text(panel_path)
    mouth_start, _mouth_end, mouth_body = find_source_window(
        panel_text, "F0349_INVENTORY_ProcessCommand70_ClickOnMouth", "F0350_INVENTORY_DrawStopPressingMouth"
    )
    mouth_markers = require_in_order(
        mouth_body,
        [
            ("leader empty-hand gate", "if (G0415_ui_LeaderEmptyHanded)"),
            ("pressing mouth", "G0333_B_PressingMouth = C1_TRUE"),
            ("leader hand thing", "G4055_s_LeaderHandObject.Thing"),
            ("allowed mouth slot", "MASK0x0001_MOUTH"),
            ("remove from leader hand", "F0298_CHAMPION_GetObjectRemovedFromLeaderHand()"),
            ("mouth animation zone", "C545_ZONE_MOUTH"),
            ("status/panel refresh", "F0292_CHAMPION_DrawState"),
        ],
        "ReDMCSB F0349 mouth semantics",
    )
    eye_start, _eye_end, eye_body = find_source_window(
        panel_text, "F0352_INVENTORY_ProcessCommand71_ClickOnEye", "F0353_INVENTORY_DrawStopPressingEye"
    )
    eye_markers = require_in_order(
        eye_body,
        [
            ("pressing eye", "G0331_B_PressingEye = C1_TRUE"),
            ("looking icon", "C203_ICON_EYE_LOOKING"),
            ("eye zone", "C546_ZONE_EYE"),
            ("empty hand stats", "F0351_INVENTORY_DrawChampionSkillsAndStatistics"),
            ("leader hand object description", "F0342_INVENTORY_DrawPanel_Object(G4055_s_LeaderHandObject.Thing, C1_TRUE)"),
        ],
        "ReDMCSB F0352 eye semantics",
    )

    require_excerpt("COMMAND.C", 128, 138, ["C070_COMMAND_CLICK_ON_MOUTH", "56,  71,  46,  61", "C071_COMMAND_CLICK_ON_EYE", "12,  27,  46,  61"])
    require_excerpt("COMMAND.C", 420, 428, ["C508_ZONE_SLOT_BOX_09_INVENTORY_ACTION_HAND", "C545_ZONE_MOUTH", "C546_ZONE_EYE"])
    require_excerpt("COMMAND.C", 2314, 2320, ["F0349_INVENTORY_ProcessCommand70_ClickOnMouth", "F0352_INVENTORY_ProcessCommand71_ClickOnEye"])
    require_excerpt("PANEL.C", 1743, 1825, ["F0349_INVENTORY_ProcessCommand70_ClickOnMouth", "G0333_B_PressingMouth", "MASK0x0001_MOUTH"])
    require_excerpt("PANEL.C", 1928, 1950, ["C545_ZONE_MOUTH", "F0292_CHAMPION_DrawState"])
    require_excerpt("PANEL.C", 2111, 2180, ["F0352_INVENTORY_ProcessCommand71_ClickOnEye", "C546_ZONE_EYE", "F0342_INVENTORY_DrawPanel_Object"])

    return [
        f"ReDMCSB COMMAND.C dispatch window starts at {cmd_path}:{line_no(cmd_text, dispatch_start)}",
        *(f"ReDMCSB dispatch {name}: line {line_no(cmd_text, dispatch_start + pos)}" for name, pos in dispatch_markers),
        f"ReDMCSB PANEL.C F0349 starts at {panel_path}:{line_no(panel_text, mouth_start)}",
        *(f"ReDMCSB F0349 {name}: line {line_no(panel_text, mouth_start + pos)}" for name, pos in mouth_markers),
        f"ReDMCSB PANEL.C F0352 starts at {panel_path}:{line_no(panel_text, eye_start)}",
        *(f"ReDMCSB F0352 {name}: line {line_no(panel_text, eye_start + pos)}" for name, pos in eye_markers),
    ]


def verify_firestaff() -> list[str]:
    text = read_text(FIRESTAFF_SRC)
    route_start, _route_end, route_body = find_source_window(text, "m11_v1_mouse_route_zone_rect", "m11_v1_mouse_route_matches")
    route_markers = require_in_order(
        route_body,
        [
            ("slotbox route before mouth/eye", "route->zoneId >= 507 && route->zoneId <= 536"),
            ("mouth/eye zone predicate", "route->zoneId == 545 || route->zoneId == 546"),
            ("mouth x", "(route->zoneId == 545) ? 56 : 12"),
            ("mouth/eye y", "if (outY) *outY = 13"),
            ("zone size", "if (outW) *outW = 16"),
        ],
        "Firestaff mouse route C545/C546 zone bridge",
    )
    command_start = text.find("static const M11_V1MouseRoute inventoryRoutes[]")
    if command_start < 0:
        raise AssertionError("missing Firestaff inventoryRoutes table")
    command_body = text[command_start:command_start + 2600]
    command_markers = require_in_order(
        command_body,
        [
            ("action hand", "{ 29, M11_DM1_MOUSE_SPACE_VIEWPORT, 508"),
            ("pouch2", "{ 34, M11_DM1_MOUSE_SPACE_VIEWPORT, 513"),
            ("mouth command", "{ 70, M11_DM1_MOUSE_SPACE_VIEWPORT, 545"),
            ("eye command", "{ 71, M11_DM1_MOUSE_SPACE_VIEWPORT, 546"),
            ("next quiver command", "{ 35, M11_DM1_MOUSE_SPACE_VIEWPORT, 514"),
        ],
        "Firestaff inventoryRoutes table mouth/eye insertion",
    )
    probe_text = read_text(FIRESTAFF_PROBE)
    probe_markers = require_in_order(
        probe_text,
        [
            ("mouth probe id", "INV_GV_437A"),
            ("mouth command assertion", ") == 70"),
            ("mouth zone assertion", "zoneId == 545"),
            ("eye probe id", "INV_GV_437B"),
            ("eye command assertion", ") == 71"),
            ("eye zone assertion", "zoneId == 546"),
        ],
        "Firestaff probe mouth/eye route assertions",
    )
    require_excerpt("m11_game_view.c", 15569, 15580, ["route->zoneId == 545 || route->zoneId == 546", "? 56 : 12", "if (outY) *outY = 13"])
    require_excerpt("m11_game_view.c", 15646, 15658, ["{ 70, M11_DM1_MOUSE_SPACE_VIEWPORT, 545", "{ 71, M11_DM1_MOUSE_SPACE_VIEWPORT, 546"])
    require_excerpt("probes/m11/firestaff_m11_game_view_probe.c", 738, 755, ["INV_GV_437A", "INV_GV_437B", "zoneId == 545", "zoneId == 546"])
    return [
        f"Firestaff m11_v1_mouse_route_zone_rect starts at {FIRESTAFF_SRC}:{line_no(text, route_start)}",
        *(f"Firestaff route bridge {name}: line {line_no(text, route_start + pos)}" for name, pos in route_markers),
        *(f"Firestaff inventoryRoutes {name}: line {line_no(text, command_start + pos)}" for name, pos in command_markers),
        *(f"Firestaff probe {name}: line {line_no(probe_text, pos)}" for name, pos in probe_markers),
    ]


def verify_evidence_json() -> None:
    data = json.loads(EVIDENCE_JSON.read_text(encoding="utf-8"))
    if data.get("gate") != "v1_inventory_mouth_eye_routing_redmcsb":
        raise AssertionError("evidence JSON gate id mismatch")
    ranges = {(entry.get("file"), entry.get("start"), entry.get("end")) for entry in data.get("sourceRanges", [])}
    expected = {(entry["file"], entry["start"], entry["end"]) for entry in SOURCE_RANGES}
    missing = expected - ranges
    if missing:
        raise AssertionError(f"evidence JSON missing source ranges: {sorted(missing)}")
    non_claims = data.get("nonClaims", [])
    if not any("runtime" in claim.lower() for claim in non_claims):
        raise AssertionError("evidence JSON must preserve runtime non-claim")
    print(f"evidence={EVIDENCE_JSON.relative_to(ROOT)} status=ok")


def main() -> int:
    print("probe=v1_inventory_mouth_eye_routing_redmcsb_gate")
    print(f"sourceRoot={REDMCSB_ROOT}")
    lines = verify_redmcsb() + verify_firestaff()
    verify_evidence_json()
    print("v1InventoryMouthEyeRoutingRedmcsbGateOk=1")
    for line in lines:
        print(f"- {line}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, json.JSONDecodeError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
