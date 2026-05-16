#!/usr/bin/env python3
"""Verify pass340 DM1 V1 direct enqueue wrapper evidence and implementation."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EVIDENCE = ROOT / "parity-evidence" / "pass340_dm1_v1_direct_enqueue_wrapper.md"
LOGDIR = ROOT / "parity-evidence" / "verification" / "pass340_dm1_v1_direct_enqueue_wrapper"

REQUIRED = [
    EVIDENCE,
    LOGDIR,
    ROOT / "include/dm1_v1_input_command_queue_pc34_compat.h",
    ROOT / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c",
    ROOT / "include/dm1_v1_movement_pipeline_pc34_compat.h",
    ROOT / "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c",
    ROOT / "tests/test_dm1_v1_movement_pipeline_pc34_compat.c",
    RED / "COMMAND.C",
    RED / "INPUT.C",
    RED / "GAMELOOP.C",
]

EVIDENCE_NEEDLES = [
    "Status: **CLOSED with minimal runtime API.**",
    "COMMAND.C:677-684",
    "COMMAND.C:1452-1690",
    "COMMAND.C:1709-1813",
    "COMMAND.C:2045-2156",
    "DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat",
    "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat",
    "First-class pipeline command API:** CLOSED.",
    "OS keypad/NumLock route:** BYPASSED by design.",
]

FIRESTAFF_NEEDLES = {
    "include/dm1_v1_input_command_queue_pc34_compat.h": [
        "DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat",
        "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat",
    ],
    "src/dm1/dm1_v1_input_command_queue_pc34_compat.c": [
        "int DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat",
        "return enqueue_command(queue, command, x, y);",
        "return DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(queue, command, x, y);",
    ],
    "include/dm1_v1_movement_pipeline_pc34_compat.h": [
        "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat",
        "It bypasses",
    ],
    "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c": [
        "int DM1_V1_MovementPipeline_EnqueueCommandPc34Compat",
        "DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(",
    ],
    "tests/test_dm1_v1_movement_pipeline_pc34_compat.c": [
        "test_direct_command_wrapper_forward_step",
        "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(",
        "DM1_V1_COMMAND_MOVE_FORWARD",
        "direct_wrapper_dequeued",
    ],
}

RED_MARKERS = [
    (RED / "COMMAND.C", "COMMAND G0432_as_CommandQueue", 6, 12),
    (RED / "COMMAND.C", "C001_COMMAND_TURN_LEFT,     0x004B", 677, 684),
    (RED / "COMMAND.C", "void F0359_COMMAND_ProcessClick_CPSC", 1452, 1690),
    (RED / "COMMAND.C", "void F0361_COMMAND_ProcessKeyPress", 1709, 1813),
    (RED / "COMMAND.C", "void F0380_COMMAND_ProcessQueue_CPSC", 2045, 2156),
    (RED / "COMMAND.C", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", 2151, 2151),
    (RED / "COMMAND.C", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);", 2155, 2155),
    (RED / "INPUT.C", "struct InputEvent* F0543_INPUT_DeviceInterruptHandler", 298, 751),
    (RED / "INPUT.C", "F1097_StoreKeyInBuffer((int16_t)L2623_l_NormalizedKeyCode);", 527, 569),
    (RED / "INPUT.C", "void F1097_StoreKeyInBuffer", 822, 834),
    (RED / "INPUT.C", "int16_t F1098_GetFirstKeyFromBuffer", 836, 847),
    (RED / "GAMELOOP.C", "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());", 167, 167),
    (RED / "GAMELOOP.C", "F0380_COMMAND_ProcessQueue_CPSC();", 215, 215),
]


def read(path: Path) -> str:
    enc = "utf-8" if path.suffix in {".md", ".py"} else "latin-1"
    return path.read_text(encoding=enc)


def range_contains(path: Path, marker: str, start: int, end: int) -> bool:
    lines = read(path).splitlines()
    if end > len(lines):
        raise AssertionError(f"{path.name}:{end} beyond file length {len(lines)}")
    window = "\n".join(lines[max(0, start - 3):min(len(lines), end + 2)])
    return marker in window


def main() -> int:
    missing = [str(p) for p in REQUIRED if not p.exists()]
    if missing:
        raise AssertionError("missing required paths: " + ", ".join(missing))

    ev = read(EVIDENCE)
    for needle in EVIDENCE_NEEDLES:
        if needle not in ev:
            raise AssertionError(f"evidence missing {needle!r}")

    for rel, needles in FIRESTAFF_NEEDLES.items():
        text = read(ROOT / rel)
        for needle in needles:
            if needle not in text:
                raise AssertionError(f"{rel} missing {needle!r}")

    q_c = read(ROOT / "src/dm1/dm1_v1_input_command_queue_pc34_compat.c")
    if not re.search(r"int\s+DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat[\s\S]*?return\s+enqueue_command\(queue, command, x, y\);", q_c):
        raise AssertionError("queue wrapper does not directly forward explicit command")

    pipe_c = read(ROOT / "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c")
    if not re.search(r"int\s+DM1_V1_MovementPipeline_EnqueueCommandPc34Compat[\s\S]*?&pipeline->commandQueue, command, x, y", pipe_c):
        raise AssertionError("pipeline wrapper does not forward to pipeline command queue")

    for path, marker, start, end in RED_MARKERS:
        if not range_contains(path, marker, start, end):
            raise AssertionError(f"ReDMCSB marker not in cited range: {path.name}:{start}-{end} {marker!r}")

    print("pass340_dm1_v1_direct_enqueue_wrapper=pass direct_queue_wrapper=CLOSED pipeline_wrapper=CLOSED os_keypad=BYPASSED evidence=parity-evidence/pass340_dm1_v1_direct_enqueue_wrapper.md")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"pass340_dm1_v1_direct_enqueue_wrapper=fail {exc}", file=sys.stderr)
        raise SystemExit(1)
