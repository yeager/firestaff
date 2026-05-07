#!/usr/bin/env python3
"""Pass351 DM1 V1 live route -> movement -> viewport redraw source-lock sweep.

This verifier is intentionally evidence-only. It verifies that Firestaff's
landed live route consumes route/key input into the DM1 V1 movement pipeline,
that the compat core mutates party state and raises viewport redraw only after
accepted turn/step effects, and that every stage remains anchored to ReDMCSB
source lines from Toolchains/Common/Source.
"""
from __future__ import annotations

import json
import os
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
REDMCSB = Path(os.environ.get(
    "REDMCSB_COMMON_SOURCE",
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source",
))

CHECKS = [
    {
        "id": "redmcsb_mouse_movement_zones",
        "path": REDMCSB / "COMMAND.C",
        "anchor": "COMMAND.C:106-114",
        "needles": [
            "MOUSE_INPUT G0448_as_Graphic561_SecondaryMouseInput_Movement[9]",
            "{ C003_COMMAND_MOVE_FORWARD,          263, 289, 125, 145",
            "{ C004_COMMAND_MOVE_RIGHT,            291, 318, 147, 167",
        ],
    },
    {
        "id": "redmcsb_i34e_keyboard_movement_table",
        "path": REDMCSB / "COMMAND.C",
        "anchor": "COMMAND.C:677-684",
        "needles": [
            "#ifdef MEDIA707_I34E_I34M",
            "{ C001_COMMAND_TURN_LEFT,     0x004B }",
            "{ C003_COMMAND_MOVE_FORWARD,  0x004C }",
            "{ C004_COMMAND_MOVE_RIGHT,    0x0051 }",
        ],
    },
    {
        "id": "redmcsb_f0380_gate_dequeue_dispatch",
        "path": REDMCSB / "COMMAND.C",
        "anchor": "COMMAND.C:2075-2156",
        "needles": [
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "G0310_i_DisabledMovementTicks || (G0311_i_ProjectileDisabledMovementTicks",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
    },
    {
        "id": "redmcsb_turn_and_step_side_effects",
        "path": REDMCSB / "CLIKMENU.C",
        "anchor": "CLIKMENU.C:156-173,224-346",
        "needles": [
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0284_CHAMPION_SetPartyDirection",
            "G0465_ai_Graphic561_MovementArrowToStepForwardCount",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0357_COMMAND_DiscardAllInput();",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
    },
    {
        "id": "redmcsb_main_loop_redraw_present",
        "path": REDMCSB / "GAMELOOP.C",
        "anchor": "GAMELOOP.C:90,150-155",
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "G0310_i_DisabledMovementTicks--;",
            "G0311_i_ProjectileDisabledMovementTicks--;",
        ],
    },
    {
        "id": "redmcsb_f0128_to_f0097",
        "path": REDMCSB / "DUNVIEW.C",
        "anchor": "DUNVIEW.C:8608-8611",
        "needles": [
            "#ifdef MEDIA556_F20E_F20J_I34E_I34M",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
    },
    {
        "id": "redmcsb_f0097_viewport_request_and_blit",
        "path": REDMCSB / "DRAWVIEW.C",
        "anchor": "DRAWVIEW.C:709-724,1056-1068",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
        ],
    },
    {
        "id": "firestaff_script_and_key_routes_to_m12",
        "path": REPO / "main_loop_m11.c",
        "anchor": "main_loop_m11.c:788-814,1105-1133,1603-1617",
        "needles": [
            "return M12_MENU_INPUT_UP;",
            "case SDLK_KP_5:",
            "case SDLK_KP_4:",
            "case SDLK_KP_6:",
            "M11_GameView_HandleInput(&gameView, input);",
            "M11_GameView_Draw(&gameView,",
        ],
    },
    {
        "id": "firestaff_live_m12_to_dm1_commands_and_pipeline",
        "path": REPO / "m11_game_view.c",
        "anchor": "m11_game_view.c:4864-4958",
        "needles": [
            "case M12_MENU_INPUT_UP:",
            "return DM1_V1_COMMAND_MOVE_FORWARD;",
            "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat",
            "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat",
            "return state->lastDm1V1MovementPipelineResult.viewportDirty ||",
        ],
    },
    {
        "id": "firestaff_core_sets_redraw_after_accepted_effects",
        "path": REPO / "dm1_v1_movement_command_core_pc34_compat.c",
        "anchor": "dm1_v1_movement_command_core_pc34_compat.c:74-187",
        "needles": [
            "if (dm1_v1_is_turn_command(outResult->queue.command))",
            "outResult->turnApplied = 1;",
            "outResult->viewportRedrawRequested = 1;",
            "if (!F0702_MOVEMENT_TryMove_Compat",
            "outResult->movementBlocked = 1;",
            "outResult->stepApplied = 1;",
        ],
    },
    {
        "id": "firestaff_pipeline_publishes_viewport_dirty",
        "path": REPO / "dm1_v1_movement_pipeline_pc34_compat.c",
        "anchor": "dm1_v1_movement_pipeline_pc34_compat.c:91-203",
        "needles": [
            "DM1_V1_MovementCommandCore_ProcessOnePc34Compat",
            "outResult->anyMovementOccurred =",
            "outResult->anyTurnOccurred = outResult->core.turnApplied;",
            "outResult->viewportDirty = outResult->core.viewportRedrawRequested;",
            "outResult->provenance.viewportPresent =",
        ],
    },
]


def main() -> int:
    failures: list[dict[str, str]] = []
    results: list[dict[str, str]] = []
    for check in CHECKS:
        path = Path(check["path"])
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError as exc:
            failures.append({"id": check["id"], "anchor": check["anchor"], "error": str(exc)})
            continue
        missing = [needle for needle in check["needles"] if needle not in text]
        if missing:
            failures.append({
                "id": check["id"],
                "anchor": check["anchor"],
                "error": "missing required snippet(s)",
                "missing": "; ".join(missing),
            })
        else:
            results.append({"id": check["id"], "anchor": check["anchor"], "status": "ok"})

    payload = {
        "verifier": "pass351_dm1_v1_live_viewport_redraw_parity_sweep",
        "repo": str(REPO),
        "redmcsb_common_source": str(REDMCSB),
        "checked": len(CHECKS),
        "passed": len(results),
        "failed": len(failures),
        "results": results,
        "failures": failures,
        "claim": "source-locked live route -> compat movement -> viewport redraw evidence; no original pixel parity claim",
    }
    print(json.dumps(payload, indent=2, sort_keys=True))
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
