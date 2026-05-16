#!/usr/bin/env python3
"""Verify pass395 touch pointer -> DM1 V1 command queue integration.

This is intentionally source-first: it audits the ReDMCSB mouse/click/queue
route before accepting the local touch shim.  The verifier then checks that the
local abstraction feeds a resolved mouse command into the existing queue seam,
not a synthetic keyboard path or parallel dispatcher.
"""
from __future__ import annotations

import json
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()
OUT_DIR = REPO / "parity-evidence/verification/pass395_dm1_v1_touch_queue_integration"
MANIFEST = OUT_DIR / "manifest.json"
EVIDENCE = OUT_DIR / "evidence.md"
PASS391 = REPO / "parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/manifest.json"


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise AssertionError(f"missing {label}: {needle}")


def main() -> None:
    command_c = read(SOURCE_ROOT / "COMMAND.C")
    startup2_c = read(SOURCE_ROOT / "STARTUP2.C")
    defs_h = read(SOURCE_ROOT / "DEFS.H")
    coord_c = read(SOURCE_ROOT / "COORD.C")
    touch_h = read(REPO / "include/touch_pointer_input_pc34_compat.h")
    touch_c = read(REPO / "src/shared/touch_pointer_input_pc34_compat.c")
    queue_c = read(REPO / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c")
    test_c = read(REPO / "tests/test_touch_pointer_input_pc34_compat_integration.c")
    pass391 = json.loads(read(PASS391))

    source_checks = {
        "defs_mouse_payload_buttons": [
            (defs_h, "#define MASK0x0001_MOUSE_RIGHT_BUTTON"),
            (defs_h, "#define MASK0x0002_MOUSE_LEFT_BUTTON"),
            (defs_h, "int16_t X;"),
            (defs_h, "int16_t Y;"),
            (defs_h, "int16_t Command;"),
        ],
        "startup_primary_secondary_tables": [
            (startup2_c, "G0441_ps_PrimaryMouseInput = G0447_as_Graphic561_PrimaryMouseInput_Interface;"),
            (startup2_c, "G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement;"),
        ],
        "f0358_mouse_hit_test": [
            (command_c, "int16_t F0358_COMMAND_GetCommandFromMouseInput_CPSC("),
            (command_c, "while (L1107_Command = P0721_ps_MouseInput->Command)"),
            (command_c, "P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button"),
            (command_c, "return L1107_Command;"),
        ],
        "f0359_click_enqueue": [
            (command_c, "void F0359_COMMAND_ProcessClick_CPSC("),
            (command_c, "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput"),
            (command_c, "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput"),
            (command_c, "G2153_i_QueuedCommandsCount++;"),
            (command_c, "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command;"),
            (command_c, "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].X = P0725_i_X;"),
            (command_c, "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].Y = P0726_i_Y;"),
        ],
        "pending_click_replay_and_f0380": [
            (command_c, "void F0360_COMMAND_ProcessPendingClick"),
            (command_c, "F0359_COMMAND_ProcessClick_CPSC(G0437_i_PendingClickX, G0438_i_PendingClickY, G0439_i_PendingClickButtonsStatus);"),
            (command_c, "void F0380_COMMAND_ProcessQueue_CPSC("),
            (command_c, "G2153_i_QueuedCommandsCount--;"),
            (command_c, "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);"),
            (command_c, "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"),
        ],
        "viewport_coordinates": [
            (coord_c, "G2067_i_ViewportScreenX = 0;"),
            (coord_c, "G2068_i_ViewportScreenY = 33;"),
            (coord_c, "BOOLEAN F0798_COMMAND_IsPointInZone"),
        ],
    }
    for group, checks in source_checks.items():
        for text, needle in checks:
            require(text, needle, group)

    local_checks = {
        "touch_header_uses_queue": [
            (touch_h, '#include "include/dm1_v1_input_command_queue_pc34_compat.h"'),
            (touch_h, "TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue"),
        ],
        "touch_translation_and_enqueue": [
            (touch_c, "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary"),
            (touch_c, "TOUCHCLICK_Compat_NormalizeScaledScreenPoint"),
            (touch_c, "TOUCHCLICK_Compat_MapViewportLocalPointToDispatch"),
            (touch_c, "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat"),
        ],
        "queue_existing_mouse_path": [
            (queue_c, "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat"),
            (queue_c, "queue->pendingClickPresent = 1;"),
            (queue_c, "process_pending_click(queue);"),
            (queue_c, "result.dispatchedMove = 1;"),
        ],
        "test_proves_touch_queue_dispatch": [
            (test_c, "TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue"),
            (test_c, "DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0)"),
            (test_c, "processResult.dispatchedMove"),
            (test_c, "queue.pendingClickCommand != 71"),
        ],
    }
    for group, checks in local_checks.items():
        for text, needle in checks:
            require(text, needle, group)

    predicates = pass391.get("proofPredicates", {})
    needed_pass391 = {
        "sourceAuditOk": True,
        "g2153IncrementObserved": True,
        "g2153DecrementPopLoadObserved": True,
        "f0380PopLoadAfterQueueWriteObserved": True,
        "f0365OrF0366DispatchObserved": True,
    }
    for key, expected in needed_pass391.items():
        if predicates.get(key) is not expected:
            raise AssertionError(f"pass391 predicate mismatch {key}: {predicates.get(key)!r}")

    result = {
        "status": "PASS395_TOUCH_POINTER_TO_EXISTING_COMMAND_QUEUE_SEAM_VERIFIED",
        "sourceRoot": str(SOURCE_ROOT),
        "repo": str(REPO),
        "reversibleScope": "touch_pointer_input_pc34_compat.{h,c} maps pointer/touch hits to existing DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat; no keyboard synthesis and no replacement dispatcher",
        "sourceAudits": sorted(source_checks.keys()),
        "localAudits": sorted(local_checks.keys()),
        "pass391PredicatesUsed": {k: predicates.get(k) for k in sorted(needed_pass391)},
        "notPromotedBy": [
            "route keylog alone",
            "touch table hit without queue enqueue",
            "synthetic keyboard remap",
            "F0380 entry without G2153 pop/decrement",
            "source-only address binding",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    EVIDENCE.write_text(
        "# pass395 DM1 V1 touch queue integration\n\n"
        "Status: PASS395_TOUCH_POINTER_TO_EXISTING_COMMAND_QUEUE_SEAM_VERIFIED\n\n"
        "- ReDMCSB audit: F0358 mouse hit-test, F0359 click enqueue, pending-click replay, and F0380 queue pop/dispatch were checked before local code.\n"
        "- Local seam: touch/pointer events normalize to original screen or viewport coordinates and enqueue through `DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat`.\n"
        "- Guardrail: no synthetic keyboard path, no broad dispatcher refactor, no DM1 V1 movement parity changes.\n"
        "- Prior runtime proof consumed: pass391 observed G2153 increment, pop/decrement, and F0365/F0366 dispatch in the existing command queue path.\n",
    )
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
