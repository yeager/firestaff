#!/usr/bin/env python3
"""Source-lock the runtime TITLE cadence binding used at launcher handoff."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/TITLE.C").expanduser()
errors = []


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        errors.append(f"missing {label}: {needle!r}")
    return pos


def require_order(text: str, markers: list[str], label: str) -> None:
    last = -1
    for marker in markers:
        pos = text.find(marker, last + 1)
        if pos < 0:
            errors.append(f"missing {label}: {marker!r}")
            return
        last = pos


def main() -> int:
    main_loop = (ROOT / "main_loop_m11.c").read_text(encoding="utf-8")
    frontend_c = (ROOT / "title_frontend_v1.c").read_text(encoding="utf-8")
    frontend_h = (ROOT / "title_frontend_v1.h").read_text(encoding="utf-8")
    test_c = (ROOT / "test_title_frontend_runtime_cadence_pc34_compat.c").read_text(encoding="utf-8")
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    red = RED.read_text(encoding="latin-1")

    require(red, "F0437_STARTEND_DrawTitle", "ReDMCSB TITLE draw function")
    require(red, "M526_WaitVerticalBlank", "ReDMCSB vertical blank primitive")
    require_order(red, [
        "F0437_STARTEND_DrawTitle",
        "F0660_(CM58_NEGGRAPHIC_TITLE_PRESENTS",
        "F0129_VIDEO_BlitShrinkWithPaletteChanges",
        "while (--L1380_i_Counter >= 0)",
        "M526_WaitVerticalBlank();",
        "F0021_MAIN_BlitToScreen",
        "F0022_MAIN_Delay(20);",
        "F0660_(CM60_NEGGRAPHIC_TITLE_STRIKES_BACK",
        "F0022_MAIN_Delay(2);",
    ], "ReDMCSB TITLE zoom/final presentation path")

    for needle, label in [
        ("V1_TitleFrontend_GetRuntimeFrameDelayMs", "runtime frame delay helper declaration/definition"),
        ("V1_TitleFrontend_GetRuntimeFinalGuardDelayMs", "runtime final guard helper declaration/definition"),
    ]:
        require(frontend_h, needle, label)
        require(frontend_c, needle, label)

    require(main_loop, "SDL_Delay(V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing));", "main loop uses runtime frame delay helper")
    require(main_loop, "SDL_Delay(V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&timing));", "main loop uses runtime final guard helper")
    if "SDL_Delay((timing.postZoomVblankCount + timing.finalFadeGuardVblankCount) * 20U)" in main_loop:
        errors.append("main loop reverted to inline final guard arithmetic")
    if "if (timing.vblankBeforeEachZoomStep)" in main_loop:
        errors.append("main loop reverted to inline vblank delay branch")

    for needle, label in [
        ("V1_TitleFrontend_GetSourceTimingEvidence", "C runtime cadence test loads source timing"),
        ("V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing)", "C runtime cadence test observes frame helper"),
        ("V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&timing)", "C runtime cadence test observes final guard helper"),
        ("20u", "C runtime cadence test locks one-vblank frame delay"),
        ("60u", "C runtime cadence test locks three-vblank final guard"),
    ]:
        require(test_c, needle, label)

    require(cmake, "test_title_frontend_runtime_cadence_pc34_compat", "CMake runtime cadence executable")
    require(cmake, "pass457_dm1_v1_title_runtime_cadence_gate", "CMake pass457 static gate")

    if errors:
        for error in errors:
            print(f"FAIL: {error}")
        return 1
    print("ok: pass457 TITLE runtime cadence gate source-locked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
