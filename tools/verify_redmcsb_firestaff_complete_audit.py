#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / "docs/parity/REDMCSB_FIRESTAFF_COMPLETE_AUDIT.md"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

REQUIRED_DOC_TOKENS = [
    "movement_input",
    "viewport_hud_render",
    "gameplay_systems",
    "frontend_route_capture",
    "MATCHED",
    "FIXED",
    "KNOWN_DIFF",
    "MISSING",
    "BLOCKED",
    "memory_creature_ai_pc34_compat.c",
    "original_overlay_regression",
]

REQUIRED_REDMCSB_FILES = [
    "COMMAND.C",
    "CLIKMENU.C",
    "MOVESENS.C",
    "GAMELOOP.C",
    "DUNVIEW.C",
    "DRAWVIEW.C",
    "PANEL.C",
    "CHAMDRAW.C",
    "GROUP.C",
    "PROJEXPL.C",
    "TITLE.C",
    "ENTRANCE.C",
    "ENDGAME.C",
]

REQUIRED_FIRESTAFF_FILES = [
    "memory_creature_ai_pc34_compat.c",
    "dm1_v1_viewport_3d_pc34_compat.c",
    "dm1_v1_movement_pipeline_pc34_compat.c",
    "dm1_v1_input_command_queue_pc34_compat.c",
    "docs/parity/COMPLETION_MATRIX.md",
]


def require(condition: bool, message: str, failures: list[str]) -> None:
    if not condition:
        failures.append(message)


def main() -> int:
    failures: list[str] = []
    require(DOC.exists(), f"missing audit document: {DOC}", failures)
    text = DOC.read_text(encoding="utf-8") if DOC.exists() else ""

    for token in REQUIRED_DOC_TOKENS:
        require(token in text, f"audit document missing token: {token}", failures)

    require(RED.is_dir(), f"missing ReDMCSB source directory: {RED}", failures)
    for name in REQUIRED_REDMCSB_FILES:
        require((RED / name).is_file(), f"missing ReDMCSB source file: {name}", failures)

    for rel in REQUIRED_FIRESTAFF_FILES:
        require((ROOT / rel).is_file(), f"missing Firestaff file: {rel}", failures)

    if failures:
        for failure in failures:
            print(f"FAIL {failure}")
        return 1

    print("PASS_REDMCSB_FIRESTAFF_COMPLETE_AUDIT_CONTRACT")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
