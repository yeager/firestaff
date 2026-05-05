#!/usr/bin/env python3
"""Pass223 DM1 V1 post-command redraw instrumentation source lock.

JSON-only blocker/instruction gate. It pins exact ReDMCSB seams that a future
original-runner probe must observe to promote a movement frame from "key was
sent" to: command accepted -> movement applied -> viewport present.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DEFAULT_OUT = ROOT / "parity-evidence/verification/pass223_dm1_v1_post_redraw_instrumentation_lock.json"
DEFAULT_REPORT = ROOT / "parity-evidence/pass223_dm1_v1_post_redraw_instrumentation_lock.md"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "command_accepted_queue_dispatch",
        "file": "COMMAND.C",
        "line_range": [2045, 2156],
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT))",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "observable": "Record accepted command id and queue index immediately after L1160_i_Command is loaded and before the turn/step handler call returns.",
    },
    {
        "id": "turn_handler_applies_direction_and_releases_wait",
        "file": "CLIKMENU.C",
        "line_range": [142, 173],
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0284_CHAMPION_SetPartyDirection",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        ],
        "observable": "After the turn handler returns, record G0308_i_PartyDirection and G0321_B_StopWaitingForPlayerInput; this is command accepted -> turn state applied.",
    },
    {
        "id": "step_handler_resolves_destination_and_releases_wait",
        "file": "CLIKMENU.C",
        "line_range": [180, 328],
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "needles": [
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        ],
        "observable": "After the move-result call returns, record accepted command plus G0306_i_PartyMapX/G0307_i_PartyMapY/G0310_i_DisabledMovementTicks; this is command accepted -> step state applied.",
    },
    {
        "id": "move_result_mutates_party_coordinates",
        "file": "MOVESENS.C",
        "line_range": [442, 443],
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "needles": [
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
        ],
        "observable": "Record destination map X/Y at the coordinate assignment seam for successful party moves.",
    },
    {
        "id": "game_loop_draws_mutated_party_state",
        "file": "GAMELOOP.C",
        "line_range": [88, 91],
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
        ],
        "observable": "Record draw-call arguments (direction, mapX, mapY) at the game loop call site after command processing has released the wait loop.",
    },
    {
        "id": "dungeon_view_consumes_state_before_viewport_request",
        "file": "DUNVIEW.C",
        "line_range": [8318, 8610],
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "observable": "Record the consumed P0183/P0184/P0185 tuple just before F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW).",
    },
    {
        "id": "viewport_requested_then_vertical_blank_returned",
        "file": "DRAWVIEW.C",
        "line_range": [709, 722],
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
        ],
        "observable": "Record viewport request timestamp/counter before M526_WaitVerticalBlank and a returned-from-vblank counter after it returns.",
    },
    {
        "id": "viewport_bitmap_blitted_to_screen",
        "file": "DRAWVIEW.C",
        "line_range": [840, 842],
        "function": "E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF",
        "needles": [
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT, CM1_COLOR_NO_TRANSPARENCY);",
        ],
        "observable": "Record the vblank blit counter for C007_ZONE_VIEWPORT. This is the viewport-present half of the proof.",
    },
]

CHAIN = [
    "command_accepted_queue_dispatch",
    "turn_handler_applies_direction_and_releases_wait OR step_handler_resolves_destination_and_releases_wait",
    "move_result_mutates_party_coordinates for step commands",
    "game_loop_draws_mutated_party_state",
    "dungeon_view_consumes_state_before_viewport_request",
    "viewport_requested_then_vertical_blank_returned",
    "viewport_bitmap_blitted_to_screen",
]


def compact(text: str) -> str:
    return " ".join(text.split())


def source_slice(file: str, start: int, end: int) -> str:
    path = REDMCSB / file
    if not path.is_file():
        raise AssertionError(f"missing ReDMCSB source file: {path}")
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    return "\n".join(lines[start - 1 : end])


def verify_lock(lock: dict[str, Any]) -> dict[str, Any]:
    start, end = lock["line_range"]
    text = compact(source_slice(lock["file"], start, end))
    missing = [needle for needle in lock["needles"] if compact(needle) not in text]
    return {
        "id": lock["id"],
        "file": lock["file"],
        "function": lock["function"],
        "citation": f"{lock['file']}:{start}-{end}",
        "observable": lock["observable"],
        "verified": not missing,
        "missing": missing,
    }


def write_report(manifest: dict[str, Any], report: Path) -> None:
    lines = [
        "# Pass223 — DM1 V1 post-command redraw instrumentation lock",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "This is a JSON-only source-locked blocker/instruction gate. It commits no PNG/PPM capture artifacts.",
        "",
        "## Required observable chain",
        "",
    ]
    for step in manifest["required_chain"]:
        lines.append(f"- `{step}`")
    lines += ["", "## Instrumentation points", ""]
    for item in manifest["source_locks"]:
        mark = "PASS" if item["verified"] else "FAIL"
        lines.append(f"- {mark} `{item['id']}` — `{item['citation']}` / `{item['function']}`")
        lines.append(f"  - observe: {item['observable']}")
        if item["missing"]:
            lines.append(f"  - missing: `{item['missing']}`")
    lines += [
        "",
        "## Promotion rule for the next original-runner probe",
        "",
        "A movement/turn capture is promotable only if its JSON trace links one accepted command to a strictly later state mutation/draw tuple and then to a strictly later viewport vblank blit. Key delivery, menu churn, or repeated raw frame SHA is not enough.",
        "",
        "Non-claims: no source patching, no DOSBox screenshots, no PNG/PPM output, no pixel parity claim.",
        "",
    ]
    report.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    args = parser.parse_args()

    locks = [verify_lock(lock) for lock in SOURCE_LOCKS]
    status = "PASS_SOURCE_LOCKED_POST_REDRAW_INSTRUMENTATION_POINTS" if all(item["verified"] for item in locks) else "FAIL_SOURCE_LOCK_DRIFT"
    manifest = {
        "schema": "pass223_dm1_v1_post_redraw_instrumentation_lock.v1",
        "status": status,
        "repo": str(ROOT),
        "redmcsb_source_root": str(REDMCSB),
        "artifact_policy": {"json_only": True, "forbidden_extensions": [".png", ".ppm"]},
        "required_chain": CHAIN,
        "source_locks": locks,
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({"status": status, "out": str(args.out), "report": str(args.report), "source_locks": len(locks)}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
