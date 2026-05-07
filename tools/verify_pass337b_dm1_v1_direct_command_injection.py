#!/usr/bin/env python3
"""Verify pass337b DM1 V1 direct command injection evidence."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EVIDENCE = ROOT / "parity-evidence" / "pass337b_dm1_v1_direct_command_injection.md"

REQUIRED_FILES = [
    ROOT / "dm1_v1_input_command_queue_pc34_compat.c",
    ROOT / "dm1_v1_input_command_queue_pc34_compat.h",
    ROOT / "dm1_v1_movement_command_core_pc34_compat.c",
    ROOT / "dm1_v1_movement_command_core_pc34_compat.h",
    ROOT / "dm1_v1_movement_pipeline_pc34_compat.c",
    ROOT / "dm1_v1_movement_pipeline_pc34_compat.h",
    ROOT / "firestaff_memory_graphics_dat_original_input_command_queue_probe.c",
    ROOT / "firestaff_memory_graphics_dat_original_main_loop_command_loop_probe.c",
    ROOT / "firestaff_memory_graphics_dat_original_main_loop_command_probe.c",
    ROOT / "firestaff_memory_graphics_dat_original_main_loop_command_queue_probe.c",
    ROOT / "firestaff_memory_graphics_dat_original_typed_command_queue_probe.c",
    ROOT / "parity-evidence" / "pass333_dm1_v1_keypad_mode_command_queue_probe.md",
    ROOT / "parity-evidence" / "pass335_dm1_v1_keyboard_table_route_readiness.md",
    RED / "INPUT.C",
    RED / "COMMAND.C",
]

EVIDENCE_NEEDLES = [
    "Status: **YES — source-locked at the compat command-queue seam; no OS keypad delivery is required.**",
    "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat",
    "DM1_V1_COMMAND_MOVE_FORWARD = 3",
    "F0543_INPUT_DeviceInterruptHandler`: `INPUT.C:298-751`",
    "F1097_StoreKeyInBuffer`: `INPUT.C:822-834`",
    "F0359_COMMAND_ProcessClick_CPSC`: `COMMAND.C:1452-1690`",
    "F0361_COMMAND_ProcessKeyPress`: `COMMAND.C:1709-1813`",
    "F0380_COMMAND_ProcessQueue_CPSC`: `COMMAND.C:2045-2156`",
    "Direct queue/core injection:** **NOT BLOCKED.**",
    "OS keypad/NumLock route:** **BYPASSED.**",
    "First-class pipeline command API:** **MINOR API GAP, not a parity blocker.**",
]

FIRESTAFF_NEEDLES = {
    "dm1_v1_input_command_queue_pc34_compat.h": [
        "DM1_V1_COMMAND_TURN_LEFT = 1",
        "DM1_V1_COMMAND_MOVE_FORWARD = 3",
        "DM1_V1_COMMAND_MOVE_LEFT = 6",
        "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat",
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
    ],
    "dm1_v1_input_command_queue_pc34_compat.c": [
        "static int enqueue_command",
        "return enqueue_command(queue, command, x, y);",
        "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat",
        "result.dispatchedMove = 1;",
        "result.dispatchedTurn = 1;",
    ],
    "dm1_v1_movement_command_core_pc34_compat.c": [
        "DM1_V1_MovementCommandCore_ProcessOnePc34Compat",
        "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
        "dm1_v1_is_turn_command(outResult->queue.command)",
        "dm1_v1_is_step_command(outResult->queue.command)",
        "F0702_MOVEMENT_TryMove_Compat",
    ],
    "dm1_v1_movement_pipeline_pc34_compat.h": [
        "struct Dm1V1InputCommandQueuePc34Compat commandQueue;",
        "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat",
    ],
    "dm1_v1_movement_pipeline_pc34_compat.c": [
        "DM1_V1_InputCommandQueue_InitPc34Compat(&pipeline->commandQueue);",
        "DM1_V1_InputCommandQueue_EnqueueEventPc34Compat",
        "DM1_V1_MovementCommandCore_ProcessOnePc34Compat",
    ],
}

RED_NEEDLES = {
    "INPUT.C": [
        "struct InputEvent* F0543_INPUT_DeviceInterruptHandler",
        "RawKeyConvert(L1737_ps_InputEvent1, G1049_auc_ANSIBytes",
        "F1097_StoreKeyInBuffer((int16_t)L2623_l_NormalizedKeyCode);",
        "void F1097_StoreKeyInBuffer",
        "int16_t F1098_GetFirstKeyFromBuffer",
    ],
    "COMMAND.C": [
        "COMMAND G0432_as_CommandQueue",
        "KEYBOARD_INPUT G0459_as_Graphic561_SecondaryKeyboardInput_Movement[7]",
        "C001_COMMAND_TURN_LEFT,     0x004B",
        "void F0357_COMMAND_DiscardAllInput",
        "int16_t F0358_COMMAND_GetCommandFromMouseInput_CPSC",
        "void F0359_COMMAND_ProcessClick_CPSC",
        "void F0360_COMMAND_ProcessPendingClick",
        "void F0361_COMMAND_ProcessKeyPress",
        "void F0380_COMMAND_ProcessQueue_CPSC",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ],
}

LINE_RANGE_MARKERS = [
    (RED / "INPUT.C", "struct InputEvent* F0543_INPUT_DeviceInterruptHandler", 298, 751),
    (RED / "INPUT.C", "void F1097_StoreKeyInBuffer", 822, 834),
    (RED / "INPUT.C", "int16_t F1098_GetFirstKeyFromBuffer", 836, 847),
    (RED / "COMMAND.C", "void F0357_COMMAND_DiscardAllInput", 1304, 1377),
    (RED / "COMMAND.C", "int16_t F0358_COMMAND_GetCommandFromMouseInput_CPSC", 1379, 1450),
    (RED / "COMMAND.C", "void F0359_COMMAND_ProcessClick_CPSC", 1452, 1690),
    (RED / "COMMAND.C", "void F0360_COMMAND_ProcessPendingClick", 1692, 1707),
    (RED / "COMMAND.C", "void F0361_COMMAND_ProcessKeyPress", 1709, 1813),
    (RED / "COMMAND.C", "void F0380_COMMAND_ProcessQueue_CPSC", 2045, 2156),
]


def read(path: Path) -> str:

    if path.suffix == ".md" or "pass337b" in path.name:
        return path.read_text(encoding="utf-8")
    return path.read_text(encoding="latin-1")


def line_of(text: str, needle: str) -> int:
    idx = text.find(needle)
    if idx < 0:
        raise AssertionError(f"missing marker {needle!r}")
    return text.count("\n", 0, idx) + 1


def main() -> int:
    missing = [str(p) for p in REQUIRED_FILES if not p.exists()]
    if missing:
        raise AssertionError("missing required files: " + ", ".join(missing))

    ev = read(EVIDENCE)
    for needle in EVIDENCE_NEEDLES:
        if needle not in ev:
            raise AssertionError(f"evidence missing claim: {needle}")

    for rel, needles in FIRESTAFF_NEEDLES.items():
        text = read(ROOT / rel)
        for needle in needles:
            if needle not in text:
                raise AssertionError(f"{rel} missing {needle!r}")

    for rel, needles in RED_NEEDLES.items():
        text = read(RED / rel)
        for needle in needles:
            if needle not in text:
                raise AssertionError(f"ReDMCSB {rel} missing {needle!r}")

    for path, marker, expected_start, expected_end in LINE_RANGE_MARKERS:
        text = read(path)
        total = text.count("\n") + 1
        if expected_end > total:
            raise AssertionError(f"{path.name}:{marker} cited end beyond file: {expected_end}>{total}")
        # Verify the marker occurs in or adjacent to the audited cited range. Some cited ranges
        # intentionally include the preceding ReDMCSB comment/preprocessor guard line.
        window = "\n".join(text.splitlines()[max(0, expected_start - 3):expected_end + 2])
        if marker not in window:
            raise AssertionError(f"{path.name}:{marker} not found in cited range {expected_start}-{expected_end}")

    q_c = read(ROOT / "dm1_v1_input_command_queue_pc34_compat.c")
    if not re.search(r"int\s+DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat[\s\S]*return\s+enqueue_command\(queue, command, x, y\);", q_c):
        raise AssertionError("direct command enqueue wrapper no longer forwards explicit command to enqueue_command")

    print("pass337b_dm1_v1_direct_command_injection=pass evidence=parity-evidence/pass337b_dm1_v1_direct_command_injection.md direct_queue_core=NOT_BLOCKED pipeline_wrapper=MINOR_API_GAP os_keypad=BYPASSED")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"pass337b_dm1_v1_direct_command_injection=fail {exc}", file=sys.stderr)
        raise SystemExit(1)
