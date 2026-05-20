#!/usr/bin/env python3
"""Lock the DM1 V1 collision/doors parity row to current source/runtime evidence."""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT_DIR = ROOT / "parity-evidence/verification/dm1_v1_collision_doors_parity_matrix_source_lock"
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/dm1_v1_collision_doors_parity_matrix_source_lock.md"
STATUS = "DM1_V1_COLLISION_DOORS_PARITY_MATRIX_SOURCE_LOCKED"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "f0366_movement_blockers_before_move_result",
        "file": "CLIKMENU.C",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "needles": [
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
            "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
            "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
            "F0357_COMMAND_DiscardAllInput();",
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "F0267_MOVE_GetMoveResult_CPSCE",
        ],
        "claim": "Wall, closed-door, and closed solid-fakewall blockers are resolved before accepted movement reaches F0267.",
    },
    {
        "id": "f0377_door_button_click_adds_door_event",
        "file": "CLIKVIEW.C",
        "function": "F0377_COMMAND_ProcessType80_ClickInDungeonView",
        "needles": [
            "void F0377_COMMAND_ProcessType80_ClickInDungeonView",
            "L1155_i_MapX += G0233_ai_Graphic559_DirectionToStepEastCount",
            "L1151_ps_Junk = (JUNK*)F0157_DUNGEON_GetSquareFirstThingData",
            "((DOOR*)L1151_ps_Junk)->Button",
            "F0268_SENSOR_AddEvent(C10_EVENT_DOOR",
            "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor();",
        ],
        "claim": "Viewport door-button clicks require the source DOOR->Button path and otherwise fall through to wall/front-sensor handling.",
    },
    {
        "id": "f0267_accepted_move_commits_party_tuple",
        "file": "MOVESENS.C",
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "needles": [
            "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE",
            "if (P0557_T_Thing == C0xFFFF_THING_PARTY)",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
            "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
        ],
        "claim": "Accepted collision outcomes mutate the party map tuple in the move-result path, not in a renderer-only helper.",
    },
    {
        "id": "f0128_consumes_party_tuple_for_viewport",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "claim": "Collision/door movement state is observable through the same direction/x/y tuple used by the viewport draw.",
    },
]

REQUIRED_REPO_LOCKS = [
    (ROOT / "CMakeLists.txt", "dm1_v1_movement_command_core_pc34_compat"),
    (ROOT / "CMakeLists.txt", "dm1_v1_door_button_click_pc34_compat"),
    (ROOT / "CMakeLists.txt", "dm1_v1_wall_collision_runtime_capture"),
    (ROOT / "CMakeLists.txt", "dm1_v1_wall_collision_capture_manifest_source_lock"),
    (ROOT / "tests/test_dm1_v1_movement_command_core_pc34_compat.c", "pass547 closed door movement blocked"),
    (ROOT / "tests/test_dm1_v1_door_button_click_pc34_compat.c", "door with button accepted"),
    (ROOT / "parity-evidence/dm1_v1_wall_collision_capture_manifest_source_lock.md", "not an original DOS pixel-parity claim"),
]


def compact(text: str) -> str:
    return " ".join(text.split())


def read(path: Path, encoding: str = "utf-8") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")


def line_hits(path: Path, needles: list[str]) -> dict[str, int | None]:
    lines = read(path, "latin-1").splitlines()
    hits: dict[str, int | None] = {}
    for needle in needles:
        target = compact(needle)
        hits[needle] = next((idx for idx, line in enumerate(lines, 1) if target in compact(line)), None)
    return hits


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    if not RED.exists():
        raise AssertionError(f"missing ReDMCSB source root: {RED}")
    for lock in SOURCE_LOCKS:
        path = RED / lock["file"]
        hits = line_hits(path, lock["needles"])
        present = [line for line in hits.values() if line is not None]
        missing = [needle for needle, line in hits.items() if line is None]
        rows.append({
            **lock,
            "path": str(path),
            "ok": not missing,
            "lineRange": [min(present), max(present)] if present else None,
            "lineHits": hits,
            "missing": missing,
        })
    return rows


def audit_repo_locks() -> list[dict[str, Any]]:
    rows = []
    for path, needle in REQUIRED_REPO_LOCKS:
        text = read(path)
        rows.append({"path": str(path.relative_to(ROOT)), "needle": needle, "ok": needle in text})
    return rows


def audit_parity_matrix() -> dict[str, Any]:
    matrix = read(ROOT / "docs/parity/PARITY_MATRIX_DM1_V1.md")
    row = next((line for line in matrix.splitlines() if line.startswith("| Collision and doors |")), "")
    required = [
        "KNOWN_DIFF",
        "dm1_v1_movement_command_core_pc34_compat",
        "dm1_v1_door_button_click_pc34_compat",
        "dm1_v1_wall_collision_capture_manifest_source_lock",
        "not an original DOS pixel-parity claim",
    ]
    return {
        "row": row,
        "ok": bool(row) and "| `UNPROVEN` | Add original-backed cases |" not in row and all(token in row for token in required),
        "missing": [token for token in required if token not in row],
        "stillUnprovenStub": "| `UNPROVEN` | Add original-backed cases |" in row,
    }


def build() -> dict[str, Any]:
    sources = audit_sources()
    repo = audit_repo_locks()
    matrix = audit_parity_matrix()
    problems: list[str] = []
    problems.extend(f"source {row['file']} missing {row['missing']}" for row in sources if not row["ok"])
    problems.extend(f"repo lock missing {row['needle']} in {row['path']}" for row in repo if not row["ok"])
    if not matrix["ok"]:
        problems.append(f"parity matrix collision row not updated; missing={matrix['missing']} unprovenStub={matrix['stillUnprovenStub']}")
    return {
        "schema": "firestaff.dm1_v1_collision_doors_parity_matrix_source_lock.v1",
        "status": STATUS if not problems else "FAIL_DM1_V1_COLLISION_DOORS_PARITY_MATRIX_SOURCE_LOCK",
        "ok": not problems,
        "redmcsbRoot": str(RED),
        "sourceAudit": sources,
        "repoLocks": repo,
        "parityMatrix": matrix,
        "nonClaims": [
            "does not promote original DOS pixel/content parity",
            "does not remove the need for original-backed overlay cases",
            "does not change Firestaff movement or viewport runtime behavior",
        ],
        "problems": problems,
    }


def write_report(data: dict[str, Any]) -> None:
    lines = [
        "# DM1 V1 collision/doors parity matrix source lock",
        "",
        f"Status: {data['status']}",
        "",
        "This gate narrows the stale collision/doors parity row from a bare UNPROVEN stub to source/runtime-backed evidence. It keeps the original DOS pixel/content parity blocker explicit.",
        "",
        "## ReDMCSB locks",
        "",
    ]
    for row in data["sourceAudit"]:
        rng = row["lineRange"] or ["?", "?"]
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']}:{rng[0]}-{rng[1]} {row['function']} - {row['claim']}")
    lines += ["", "## Repo evidence", ""]
    for row in data["repoLocks"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['path']} contains `{row['needle']}`")
    lines += [
        "",
        "## Parity matrix",
        "",
        f"- row updated: {data['parityMatrix']['ok']}",
        f"- still old UNPROVEN stub: {data['parityMatrix']['stillUnprovenStub']}",
        "",
        "## Non-claims",
        "",
    ]
    lines.extend(f"- {item}" for item in data["nonClaims"])
    if data["problems"]:
        lines += ["", "## Problems", ""]
        lines.extend(f"- {problem}" for problem in data["problems"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    data = build()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(data)
    print(data["status"])
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    for problem in data["problems"]:
        print(problem)
    return 0 if data["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
