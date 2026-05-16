#!/usr/bin/env python3
"""Verify pass342 DM1 V1 ReDMCSB F0361/F0380 binding map evidence."""
from pathlib import Path
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EVIDENCE = ROOT / "parity-evidence" / "pass342_dm1_v1_redmcsb_f0361_f0380_binding_map.md"
MANIFEST = ROOT / "parity-evidence" / "verification" / "pass342_dm1_v1_redmcsb_f0361_f0380_binding_map" / "manifest.json"

REQUIRED_FILES = [
    EVIDENCE,
    MANIFEST,
    ROOT / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c",
    ROOT / "include/dm1_v1_input_command_queue_pc34_compat.h",
    ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c",
    ROOT / "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c",
    ROOT / "include/dm1_v1_movement_pipeline_pc34_compat.h",
    ROOT / "src/dm1/dm1_v1_movement_timing_pc34_compat.c",
    ROOT / "src/memory/memory_movement_pc34_compat.c",
    ROOT / "src/memory/memory_sensor_execution_pc34_compat.c",
    ROOT / "src/engine/main_loop_m11.c",
    ROOT / "parity-evidence" / "pass336_dm1_v1_keypad_code_binding.md",
    ROOT / "parity-evidence" / "pass337b_dm1_v1_direct_command_injection.md",
    ROOT / "parity-evidence" / "pass338b_dm1_v1_route_token_key_symbol_audit.md",
    RED / "COMMAND.C",
    RED / "CLIKMENU.C",
    RED / "MOVESENS.C",
    RED / "GAMELOOP.C",
]

EVIDENCE_NEEDLES = [
    "Status: **SOURCE-LOCKED BINDING MAP COMPLETE.**",
    "F0361_COMMAND_ProcessKeyPress`: `COMMAND.C:1709-1813`",
    "F0380_COMMAND_ProcessQueue_CPSC`: `COMMAND.C:2045-2156`",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty`: `CLIKMENU.C:142-174`",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty`: `CLIKMENU.C:180-346`",
    "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
    "DM1_V1_MovementCommandCore_ProcessOnePc34Compat",
    "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat",
    "product/runtime integration from Firestaff scripted/SDL route input into the DM1 V1 compat movement pipeline and the live viewport redraw",
    "OS keypad/NumLock is explicitly not required",
]

FIRESTAFF_NEEDLES = {
    "include/dm1_v1_input_command_queue_pc34_compat.h": [
        "DM1_V1_COMMAND_TURN_LEFT = 1",
        "DM1_V1_COMMAND_MOVE_FORWARD = 3",
        "DM1_V1_COMMAND_MOVE_LEFT = 6",
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
    ],
    "src/dm1/dm1_v1_input_command_queue_pc34_compat.c": [
        "static int command_for_key",
        "static int enqueue_command",
        "process_pending_click",
        "DM1_V1_InputCommandQueue_EnqueueEventPc34Compat",
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        "result.movementDisabledGate = 1;",
        "result.dispatchedTurn = 1;",
        "result.dispatchedMove = 1;",
    ],
    "src/dm1/dm1_v1_movement_command_core_pc34_compat.c": [
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        "dm1_v1_is_turn_command(outResult->queue.command)",
        "dm1_v1_is_step_command(outResult->queue.command)",
        "F0702_MOVEMENT_TryMove_Compat",
        "F0718_SENSOR_ProcessPartyEnterLeave_Compat",
        "outResult->stopWaitingForPlayerInput = 1;",
        "outResult->viewportRedrawRequested = 1;",
    ],
    "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c": [
        "DM1_V1_MovementPipeline_EnqueueInputPc34Compat",
        "DM1_V1_MovementCommandCore_ProcessOnePc34Compat",
        "outResult->viewportDirty = outResult->core.viewportRedrawRequested;",
        "DM1_V1_MovementTiming_DecrementCooldownsPc34Compat",
    ],
    "src/memory/memory_movement_pc34_compat.c": [
        "F0700_MOVEMENT_TurnDirection_Compat",
        "F0701_MOVEMENT_GetStepDelta_Compat",
        "F0702_MOVEMENT_TryMove_Compat",
        "F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat",
        "F0705_MOVEMENT_ResolveStairsTransition_Compat",
        "F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat",
    ],
    "src/memory/memory_sensor_execution_pc34_compat.c": [
        "F0718_SENSOR_ProcessPartyEnterLeave_Compat",
    ],
    "src/engine/main_loop_m11.c": [
        "M12_MENU_INPUT_UP",
        "M12_MENU_INPUT_STRAFE_LEFT",
        "M12_MENU_INPUT_RIGHT",
        "SDLK_UP",
        "SDLK_LEFT",
        "SDLK_RIGHT",
    ],
}

RED_MARKERS = [
    ("COMMAND.C", 252, 305, "G0459_as_Graphic561_SecondaryKeyboardInput_Movement"),
    ("COMMAND.C", 1709, 1813, "void F0361_COMMAND_ProcessKeyPress"),
    ("COMMAND.C", 1734, 1812, "G0435_B_CommandQueueLocked"),
    ("COMMAND.C", 2045, 2156, "void F0380_COMMAND_ProcessQueue_CPSC"),
    ("COMMAND.C", 2150, 2156, "F0366_COMMAND_ProcessTypes3To6_MoveParty"),
    ("CLIKMENU.C", 142, 174, "void F0365_COMMAND_ProcessTypes1To2_TurnParty"),
    ("CLIKMENU.C", 180, 346, "void F0366_COMMAND_ProcessTypes3To6_MoveParty"),
    ("CLIKMENU.C", 325, 346, "F0267_MOVE_GetMoveResult_CPSCE"),
    ("MOVESENS.C", 316, 328, "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE"),
    ("MOVESENS.C", 799, 818, "F0276_SENSOR_ProcessThingAdditionOrRemoval"),
    ("GAMELOOP.C", 86, 92, "F0128_DUNGEONVIEW_Draw_CPSF"),
    ("GAMELOOP.C", 150, 155, "G0310_i_DisabledMovementTicks"),
]


def read_text(path: Path) -> str:
    if path.suffix in {".md", ".json", ".py"}:
        return path.read_text(encoding="utf-8")
    return path.read_text(encoding="latin-1")


def window(text: str, start: int, end: int) -> str:
    lines = text.splitlines()
    if start < 1 or end > len(lines) or start > end:
        raise AssertionError(f"invalid cited range {start}-{end} for file with {len(lines)} lines")
    return "\n".join(lines[start - 1:end])


def require_ordered(text: str, needles, label: str) -> None:
    pos = -1
    for needle in needles:
        nxt = text.find(needle, pos + 1)
        if nxt < 0:
            raise AssertionError(f"{label} missing ordered marker {needle!r}")
        pos = nxt


def main() -> int:
    missing = [str(path) for path in REQUIRED_FILES if not path.exists()]
    if missing:
        raise AssertionError("missing required files: " + ", ".join(missing))

    evidence = read_text(EVIDENCE)
    for needle in EVIDENCE_NEEDLES:
        if needle not in evidence:
            raise AssertionError(f"evidence missing {needle!r}")
    require_ordered(evidence, ["## ReDMCSB movement input anchors", "## Firestaff binding map", "## Exact remaining seam", "## Verification"], "evidence sections")

    for rel, needles in FIRESTAFF_NEEDLES.items():
        text = read_text(ROOT / rel)
        for needle in needles:
            if needle not in text:
                raise AssertionError(f"{rel} missing {needle!r}")

    for file_name, start, end, marker in RED_MARKERS:
        text = read_text(RED / file_name)
        cite = window(text, start, end)
        if marker not in cite:
            raise AssertionError(f"{file_name}:{start}-{end} missing marker {marker!r}")

    queue_text = read_text(ROOT / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c")
    if not re.search(r"queue->locked\s*=\s*1;[\s\S]*result\.dequeued\s*=\s*1;[\s\S]*process_pending_click\(queue\);", queue_text):
        raise AssertionError("queue process no longer models lock/dequeue/pending-click order")

    core_text = read_text(ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c")
    if "outResult->viewportRedrawRequested = 1;" not in core_text or "outResult->stopWaitingForPlayerInput = 1;" not in core_text:
        raise AssertionError("movement core no longer exposes stop-wait/redraw flags")

    manifest = json.loads(read_text(MANIFEST))
    if manifest.get("schema") != "pass342_dm1_v1_redmcsb_f0361_f0380_binding_map.v1":
        raise AssertionError("manifest schema mismatch")
    if manifest.get("status") != "SOURCE_LOCKED_BINDING_MAP_COMPLETE":
        raise AssertionError("manifest status mismatch")
    if manifest.get("exact_remaining_seam") != "product/runtime route-to-compat-pipeline/live-viewport bridge":
        raise AssertionError("manifest remaining seam mismatch")

    print("pass342_dm1_v1_redmcsb_f0361_f0380_binding_map=pass evidence=parity-evidence/pass342_dm1_v1_redmcsb_f0361_f0380_binding_map.md remaining_seam=product_runtime_route_to_compat_pipeline_live_viewport_bridge")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"pass342_dm1_v1_redmcsb_f0361_f0380_binding_map=fail {exc}", file=sys.stderr)
        raise SystemExit(1)
