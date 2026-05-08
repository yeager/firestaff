#!/usr/bin/env python3
"""Pass381 source-lock: DM1 V1 movement/turn commands feed viewport wall redraw order.

Evidence-only gate.  It checks ReDMCSB anchors proving the chain:
command queue -> turn/move state mutation -> F0128 viewport replay -> wall/door/content
occlusion order -> deferred viewport screen copy.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))

CHECKS: list[dict[str, Any]] = [
    {
        "id": "command-queue-dispatches-turn-and-move",
        "file": "COMMAND.C",
        "range": "2045-2156",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "markers": [
            "void F0380_COMMAND_ProcessQueue_CPSC(",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT)) {",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)) {",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "why": "The live command queue routes turns/movement into the same CLIKMENU handlers that mutate party view state.",
    },
    {
        "id": "turn-party-mutates-direction-and-sensors",
        "file": "CLIKMENU.C",
        "range": "142-174",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "markers": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty(",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, C1_TRUE, C0_FALSE);",
            "F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + ((P0734_i_Command == C002_COMMAND_TURN_RIGHT) ? 1 : 3)));",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, C1_TRUE, C1_TRUE);",
        ],
        "why": "A turn changes G0308_i_PartyDirection through the champion direction helper, not a renderer-only rotation.",
    },
    {
        "id": "move-party-computes-target-blockers-and-applies-move",
        "file": "CLIKMENU.C",
        "range": "180-347",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "markers": [
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty(",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection, G0465_ai_Graphic561_MovementArrowToStepForwardCount[AL1118_ui_MovementArrowIndex], G0466_ai_Graphic561_MovementArrowToStepRightCount[AL1118_ui_MovementArrowIndex], &L1121_i_MapX, &L1122_i_MapY);",
            "if (L1116_i_SquareType == C00_ELEMENT_WALL) {",
            "if (L1116_i_SquareType == C04_ELEMENT_DOOR) {",
            "if (L1117_B_MovementBlocked) {",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
        "why": "Movement source first resolves relative coordinates, blocks walls/closed doors/fake walls/groups, then calls the move core.",
    },
    {
        "id": "champion-direction-helper-updates-party-and-champion-cells",
        "file": "CHAMPION.C",
        "range": "93-131",
        "function": "F0284_CHAMPION_SetPartyDirection",
        "markers": [
            "void F0284_CHAMPION_SetPartyDirection(",
            "if (P0600_i_Direction == G0308_i_PartyDirection)",
            "L0834_i_Delta = P0600_i_Direction - G0308_i_PartyDirection",
            "L0835_ps_Champion->Cell = M021_NORMALIZE(L0835_ps_Champion->Cell + L0834_i_Delta);",
            "L0835_ps_Champion->Direction = M021_NORMALIZE(L0835_ps_Champion->Direction + L0834_i_Delta);",
            "G0308_i_PartyDirection = P0600_i_Direction;",
        ],
        "why": "Turning updates both the party direction and champion cells/directions that later affect viewport content placement.",
    },
    {
        "id": "move-core-updates-party-map-and-can-redraw-during-fall",
        "file": "MOVESENS.C",
        "range": "315-560",
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "markers": [
            "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE(",
            "if (P0557_T_Thing == C0xFFFF_THING_PARTY) {",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
            "L0716_ui_Direction = G0308_i_PartyDirection;",
            "F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + L0712_ps_Teleporter->Rotation));",
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY);",
        ],
        "why": "The move core owns party coordinate updates and invokes F0128 for visible pit-fall traversal using current party direction.",
    },
    {
        "id": "viewport-main-replays-visible-squares-far-to-near",
        "file": "DUNVIEW.C",
        "range": "8318-8542",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "markers": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1",
            "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
        "why": "F0128 redraws the viewport from current direction/map coordinates and layers visible cells from far to near.",
    },
    {
        "id": "wall-square-and-door-branches-enforce-source-occlusion",
        "file": "DUNVIEW.C",
        "range": "6361-6835",
        "function": "F0116/F0117/F0118 D3 square drawers",
        "markers": [
            "STATICFUNCTION void F0116_DUNGEONVIEW_DrawSquareD3L(",
            "case C00_ELEMENT_WALL:",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
            "return;",
            "case C17_ELEMENT_DOOR_FRONT:",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "F0111_DUNGEONVIEW_DrawDoor(L0201_ai_SquareAspect[M557_DOOR_THING_INDEX]",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C);",
        ],
        "why": "Wall squares draw wall zones and normally return; door-front squares explicitly split rear contents, door/frame, then front contents.",
    },
    {
        "id": "object-content-pass-is-encoded-cell-order-replay",
        "file": "DUNVIEW.C",
        "range": "4547-4910",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "markers": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "P0146_ui_OrderedViewCellOrdinals",
            "P0146_ui_OrderedViewCellOrdinals >>= 4;",
            "L0139_i_Cell = M021_NORMALIZE(AL0126_i_ViewCell + P0142_i_Direction);",
            "P0141_T_Thing = L0146_T_FirstThingToDraw;",
        ],
        "why": "Contents replay source-encoded cell-order nibbles inside each square, so wall/content parity depends on ordered replay.",
    },
    {
        "id": "drawviewport-requests-vblank-copy",
        "file": "DRAWVIEW.C",
        "range": "709-722",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "markers": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
        ],
        "why": "After F0128 composes G0296, DrawViewport requests a screen copy and waits for vertical blank.",
    },
    {
        "id": "vblank-copies-viewport-buffer-to-screen",
        "file": "BASE.C",
        "range": "961-987",
        "function": "E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF",
        "markers": [
            "tst.w   G0324_B_DrawViewportRequested(A4)",
            "clr.w   G0324_B_DrawViewportRequested(A4)",
            "movea.l G0296_puc_Bitmap_Viewport(A4),A0",
            "movea.l G0348_Bitmap_Screen(A4),A1",
            "adda.w  #5280,A1",
            "dbf     D0,T0017015",
        ],
        "why": "The vblank handler copies the composed 224x136 viewport bitmap to the screen memory region.",
    },
]


def compact(text: str) -> str:
    return " ".join(text.split())


def source_slice(path: Path, line_range: str) -> str:
    start_s, end_s = line_range.split("-", 1)
    start, end = int(start_s), int(end_s)
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    return "\n".join(lines[start - 1:end])


def run(cmd: list[str]) -> dict[str, Any]:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"cmd": cmd, "returncode": p.returncode, "outputTail": p.stdout[-4000:]}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--skip-existing-wall-gate", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    results: list[dict[str, Any]] = []
    ok = True
    for check in CHECKS:
        path = args.source / check["file"]
        text = source_slice(path, check["range"]) if path.exists() else ""
        haystack = compact(text)
        missing = [marker for marker in check["markers"] if compact(marker) not in haystack]
        passed = path.exists() and not missing
        ok = ok and passed
        results.append({
            "id": check["id"],
            "passed": passed,
            "source": "{}:{}".format(check["file"], check["range"]),
            "function": check["function"],
            "why": check["why"],
            "missing": missing,
        })

    chained: dict[str, Any] | None = None
    if not args.skip_existing_wall_gate:
        chained = run([sys.executable, "scripts/verify_dm1_v1_viewport_wall_draw_order_source_lock.py", "--source", str(args.source)])
        ok = ok and chained["returncode"] == 0

    payload = {
        "gate": "pass381_dm1_v1_movement_viewport_walls_source_lock",
        "sourceRoot": str(args.source),
        "passed": ok,
        "checks": results,
        "chainedExistingWallGate": chained,
    }
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for result in results:
            status = "PASS" if result["passed"] else "FAIL"
            print("{} {} {} {}".format(status, result["id"], result["source"], result["function"]))
            print("  {}".format(result["why"]))
            for marker in result["missing"]:
                print(f"  missing: {marker}")
        if chained is not None:
            print("{} chained existing wall gate".format("PASS" if chained["returncode"] == 0 else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
