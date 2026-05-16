#!/usr/bin/env python3
"""Pass482: source-lock DM1 V1 input->movement->viewport wall replay.

This gate ties together the two parity seams that must not drift independently:
PC-34 input/command queue dispatch must mutate the party first, and the next
viewport redraw must replay ReDMCSB wall squares in source order before present.
It is evidence-only and makes no pixel parity claim.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUT_DIR = ROOT / "parity-evidence" / "verification" / "pass482_dm1_v1_movement_viewport_walls_gate"
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / "pass482_dm1_v1_movement_viewport_walls_gate.md"


def read(path: Path, enc: str = "latin-1") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=enc, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing ordered needle {needle!r}")
        pos = hit


def require(text: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(text):
        raise AssertionError(f"missing {label}: {needle!r}")


def line_of(text: str, needle: str) -> int:
    i = text.find(needle)
    if i < 0:
        raise AssertionError(f"missing line needle {needle!r}")
    return text.count("\n", 0, i) + 1


def digest_slice(path: Path, start: int, end: int) -> str:
    text = "\n".join(read(path).splitlines()[start - 1:end])
    return hashlib.sha256(text.encode("utf-8", "replace")).hexdigest()


def source_audit() -> dict:
    command = read(RED / "COMMAND.C")
    io2 = read(RED / "IO2.C")
    clik = read(RED / "CLIKMENU.C")
    dungeon = read(RED / "DUNGEON.C")
    gameloop = read(RED / "GAMELOOP.C")
    dview = read(RED / "DUNVIEW.C")
    drawview = read(RED / "DRAWVIEW.C")

    require_order(io2, [
        "switch (L2944_ui_ - 0x1248)",
        "L2944_ui_ = 'L';",
        "L2944_ui_ = 'P';",
        "L2944_ui_ = 'K';",
        "L2944_ui_ = 'M';",
        "return L2944_ui_;",
    ], "PC-34 raw arrow normalization")
    require_order(command, [
        "{ C001_COMMAND_TURN_LEFT,     0x004B }",
        "{ C003_COMMAND_MOVE_FORWARD,  0x004C }",
        "{ C002_COMMAND_TURN_RIGHT,    0x004D }",
        "{ C006_COMMAND_MOVE_LEFT,     0x004F }",
        "{ C005_COMMAND_MOVE_BACKWARD, 0x0050 }",
        "{ C004_COMMAND_MOVE_RIGHT,    0x0051 }",
        "void F0361_COMMAND_ProcessKeyPress",
        "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
        "void F0380_COMMAND_ProcessQueue_CPSC",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "G2153_i_QueuedCommandsCount--;",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "keyboard command enqueue through F0380 dispatch")
    require_order(clik, [
        "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        "F0284_CHAMPION_SetPartyDirection",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "F0325_CHAMPION_DecrementStamina",
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
        "F0357_COMMAND_DiscardAllInput();",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
    ], "F0365/F0366 movement side effects")
    require_order(dungeon, [
        "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "*P0256_pi_MapX +=",
        "P0253_i_Direction += 1",
        "*P0256_pi_MapX +=",
    ], "relative movement projection")
    require_order(gameloop, [
        "for (;;) { /*_Infinite loop_*/",
        "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
        "if (G0310_i_DisabledMovementTicks)",
        "F0380_COMMAND_ProcessQueue_CPSC();",
    ], "main loop redraw/command cadence")
    require_order(dview, [
        "void F0128_DUNGEONVIEW_Draw_CPSF(",
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        "F0121_DUNGEONVIEW_DrawSquareD2C",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "F0124_DUNGEONVIEW_DrawSquareD1C",
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        "F0127_DUNGEONVIEW_DrawSquareD0C",
        "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
    ], "F0128 far-to-near wall draw/order")
    require_order(dview, [
        "case C00_ELEMENT_WALL:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
        "L0200_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
        "return;",
        "case C17_ELEMENT_DOOR_FRONT:",
        "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
        "F0111_DUNGEONVIEW_DrawDoor",
        "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
    ], "wall occlusion and door two-pass handoff")
    require_order(drawview, [
        "void F0097_DUNGEONVIEW_DrawViewport(",
        "G0324_B_DrawViewportRequested = C1_TRUE;",
        "M526_WaitVerticalBlank();",
        "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
    ], "viewport present path")

    return {
        "IO2.C:arrow_normalization": [line_of(io2, "switch (L2944_ui_ - 0x1248)"), line_of(io2, "return L2944_ui_;")],
        "COMMAND.C:key_table_queue_dispatch": [677, 2156],
        "CLIKMENU.C:F0365_F0366_movement": [142, 346],
        "DUNGEON.C:F0150_relative_projection": [1371, 1391],
        "GAMELOOP.C:redraw_and_command_cadence": [55, 185],
        "DUNVIEW.C:F0128_wall_replay": [8318, 8618],
        "DUNVIEW.C:wall_door_handoff": [6400, 6816],
        "DRAWVIEW.C:F0097_present": [709, 858],
    }


def firestaff_audit() -> dict:
    files = {
        "queue": read(ROOT / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c", "utf-8"),
        "core": read(ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c", "utf-8"),
        "pipeline": read(ROOT / "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c", "utf-8"),
        "viewport": read(ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "utf-8"),
        "m11": read(ROOT / "src/engine/m11_game_view.c", "utf-8"),
    }
    checks = {
        "queue": [
            "case 0xAB34: case 0x007F: case 0x9BFF: case 0x004B:",
            "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
            "result.movementDisabledGate = 1;",
            "result.dequeued = 1;",
        ],
        "core": [
            "dm1_v1_is_turn_command(outResult->queue.command)",
            "m11_v1_turning_apply_party_original_presentation_pc34_compat",
            "F0702_MOVEMENT_TryMove_Compat",
            "F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat",
            "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat",
            "outResult->viewportRedrawRequested = 1;",
        ],
        "pipeline": [
            "DM1_V1_MovementCommandCore_ProcessOnePc34Compat",
            "outResult->viewportDirty = outResult->core.viewportRedrawRequested;",
            "DUNVIEW.C:8446-8542 F0128 back-to-front viewport wall/object draw order",
            "originalRuntimeObserved = 0",
            "noPixelParityClaim = 1",
        ],
        "viewport": [
            "s_wall_draw_specs",
            "front alcove branches to F0115",
            "s_post_command_redraw",
            "COMMAND.C:2045-2156",
            "GAMELOOP.C:55-90",
        ],
        "m11": [
            "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat",
            "DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat",
            "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat",
        ],
    }
    for name, needles in checks.items():
        for needle in needles:
            require(files[name], needle, f"Firestaff {name} seam {needle}")
    return {name: len(needles) for name, needles in checks.items()}


def main() -> int:
    audited = source_audit()
    firestaff = firestaff_audit()
    digests = {
        "COMMAND.C:2045-2156": digest_slice(RED / "COMMAND.C", 2045, 2156),
        "CLIKMENU.C:142-346": digest_slice(RED / "CLIKMENU.C", 142, 346),
        "DUNVIEW.C:8318-8618": digest_slice(RED / "DUNVIEW.C", 8318, 8618),
        "DRAWVIEW.C:709-858": digest_slice(RED / "DRAWVIEW.C", 709, 858),
    }
    payload = {
        "status": "PASS482_DM1_V1_MOVEMENT_VIEWPORT_WALLS_SOURCE_LOCKED",
        "redmcsbRoot": str(RED),
        "audited": audited,
        "sourceSliceSha256": digests,
        "firestaffNeedleCounts": firestaff,
        "claims": [
            "PC-34 arrows/key commands enqueue and dispatch through F0380 before F0365/F0366 movement side effects.",
            "Accepted movement/turn state feeds the next F0128 viewport redraw from party direction/map coordinates.",
            "F0128 wall squares replay far-to-near and wall/door branches preserve ReDMCSB occlusion/two-pass ordering.",
            "This gate is source/metadata evidence only; it makes no original-runtime or pixel parity claim.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass482 — DM1 V1 movement → viewport walls gate",
        "",
        "Status: `PASS482_DM1_V1_MOVEMENT_VIEWPORT_WALLS_SOURCE_LOCKED`",
        "",
        "## Audited ReDMCSB slices",
    ]
    for key, value in audited.items():
        lines.append(f"- `{key}` — `{value}`")
    lines += ["", "## Source slice SHA-256"]
    for key, value in digests.items():
        lines.append(f"- `{key}` — `{value}`")
    lines += ["", "## Firestaff seams", json.dumps(firestaff, indent=2, sort_keys=True), "", "No original-runtime/pixel parity claim is made by this gate."]
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"pass482_dm1_v1_movement_viewport_walls_gate=pass manifest={OUT_JSON.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
