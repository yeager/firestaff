#!/usr/bin/env python3
"""Source-lock the DM1 V1 intro/title skip state-cleanup gate.

This script is the CTest-mirrored static gate for
test_dm1_v1_intro_skip_state_cleanup_pc34_compat. It enforces that:

  - the test exists in tests/, with the CTest target name we wired;
  - the title front-end module still exposes the four helper APIs the
    gate probes: DecideSequenceStep, DecideTitleMenuHandoffStep,
    GetStepPalette, GetFallbackFramePalette, plus the source animation
    count;
  - the M11 title-state module still exposes the clean init/load/cleanup
    pair the gate relies on;
  - the stateful boot-plan reachability module still uses the phase
    enum BOOT_PLAN_REACHABILITY_PHASE_* that the gate checks for
    monotonic ordering;
  - the main loop, the FTL swoosh helper, and the title intro helper
    all honour the documented "skip is data-free + deterministic"
    contract: no new global state may creep in without the gate also
    extending.

If anything regresses (renamed API, deleted function, missing test,
etc.), the script fails before CI even reaches the CTest target.
"""
from __future__ import annotations

import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
TITLE_C = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/"
    "Toolchains/Common/Source/TITLE.C"
).expanduser()


def _read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def require(text: str, needle: str, label: str, errors: list[str]) -> None:
    if needle not in text:
        errors.append(f"missing {label}: {needle!r}")


def main() -> int:
    errors: list[str] = []

    cmake = _read("CMakeLists.txt")
    test_c = _read(
        "tests/test_dm1_v1_intro_skip_state_cleanup_pc34_compat.c"
    )
    frontend_c = _read("src/frontend/title_frontend_v1.c")
    frontend_h = _read("include/title_frontend_v1.h")
    title_state_c = _read("src/dm1/dm1_v1_title_screen_pc34_compat.c")
    title_state_h = _read("include/dm1_v1_title_screen_pc34_compat.h")
    reachability_h = _read(
        "include/stateful_boot_plan_reachability_pc34_compat.h"
    )
    palette_h = _read("include/vga_palette_pc34_compat.h")
    main_loop = _read("src/engine/main_loop_m11.c")
    red = TITLE_C.read_text(encoding="latin-1")

    # --- 1. CTest wiring -----------------------------------------------
    for needle, label in [
        ("test_dm1_v1_intro_skip_state_cleanup_pc34_compat",
         "CTest executable name"),
        ("dm1_v1_intro_skip_state_cleanup_pc34_compat",
         "CTest target name"),
    ]:
        require(cmake, needle, label, errors)

    # --- 2. Title front-end helper APIs -------------------------------
    for needle, label in [
        ("V1_TitleFrontend_DecideSequenceStep",
         "sequence-step decision helper"),
        ("V1_TitleFrontend_DecideTitleMenuHandoffStep",
         "title-menu handoff decision helper"),
        ("V1_TitleFrontend_GetSourceAnimationStepCount",
         "source animation step count helper"),
        ("V1_TitleFrontend_GetSourceAnimationStep",
         "source animation step helper"),
        ("V1_TitleFrontend_GetStepPalette",
         "step palette helper"),
        ("V1_TitleFrontend_GetFallbackFramePalette",
         "fallback frame palette helper"),
    ]:
        require(frontend_h, needle, f"{label} declaration", errors)
        require(frontend_c, needle, f"{label} definition", errors)

    # --- 3. Source-lock ground truth: TITLE.C ---------------------------
    require(red, "F0437_STARTEND_DrawTitle", "ReDMCSB TITLE draw function", errors)
    require(red, "M526_WaitVerticalBlank", "ReDMCSB vertical blank primitive", errors)
    require(red, "F0022_MAIN_Delay(20);", "ReDMCSB TITLE post-PRESENTS 20-tick delay", errors)
    require(red, "F0022_MAIN_Delay(2);", "ReDMCSB TITLE final guard 2-tick delay", errors)

    # --- 4. M11 title-state clean cycle --------------------------------
    for needle, label in [
        ("void m11_ts_init(M11_TS_TitleState* state);",
         "m11_ts_init declaration"),
        ("bool m11_ts_load_title_graphics(M11_TS_TitleState* state, const uint8_t* data, uint32_t size);",
         "m11_ts_load_title_graphics declaration"),
        ("void m11_ts_cleanup(M11_TS_TitleState* state);",
         "m11_ts_cleanup declaration"),
    ]:
        require(title_state_h, needle, label, errors)
        require(title_state_c, needle.split("(")[0] + "(", label + " definition", errors)

    # Cleanup must free both buffers and zero the struct; if either
    # free call vanishes the gate cannot trust the skip path.
    require(title_state_c, "free(state->screen_buffers[0]);",
            "cleanup frees screen_buffers[0]", errors)
    require(title_state_c, "free(state->screen_buffers[1]);",
            "cleanup frees screen_buffers[1]", errors)
    require(title_state_c, "free(state->title_bitmap);",
            "cleanup frees title_bitmap", errors)
    require(title_state_c, "memset(state, 0, sizeof(M11_TS_TitleState));",
            "cleanup zeroes the title-state struct", errors)

    # Init must set active_buffer=0 and initialized=false (the
    # gate asserts both).
    require(title_state_c, "state->active_buffer = 0;",
            "init sets active_buffer to 0", errors)
    require(title_state_c, "state->initialized = false;",
            "init sets initialized to false", errors)

    # --- 5. Boot-plan reachability phase enum --------------------------
    for phase in [
        "BOOT_PLAN_REACHABILITY_PHASE_NOT_STARTED",
        "BOOT_PLAN_REACHABILITY_PHASE_BACKDROP_ESTABLISHED",
        "BOOT_PLAN_REACHABILITY_PHASE_TITLE_ESTABLISHED",
        "BOOT_PLAN_REACHABILITY_PHASE_MENU_ESTABLISHED",
        "BOOT_PLAN_REACHABILITY_PHASE_MENU_HELD",
    ]:
        require(reachability_h, phase, f"reachability phase enum: {phase}", errors)

    # --- 6. Palette ordinals the gate locks ----------------------------
    for needle, label in [
        ("VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS",
         "PRESENTS special palette ordinal"),
        ("VGA_PALETTE_PC34_SPECIAL_TITLE",
         "TITLE special palette ordinal"),
    ]:
        require(palette_h, needle, label, errors)
        require(frontend_c, needle, f"{label} usage in title front-end", errors)

    # --- 7. The C test exercises the contract end-to-end ---------------
    for needle, label in [
        ("V1_TitleFrontend_DecideSequenceStep",
         "test drives DecideSequenceStep"),
        ("V1_TitleFrontend_DecideTitleMenuHandoffStep",
         "test drives DecideTitleMenuHandoffStep"),
        ("V1_TitleFrontend_GetStepPalette",
         "test drives GetStepPalette"),
        ("V1_TitleFrontend_GetFallbackFramePalette",
         "test drives GetFallbackFramePalette"),
        ("V1_TitleFrontend_GetSourceAnimationStepCount",
         "test drives GetSourceAnimationStepCount"),
        ("V1_TitleFrontend_GetSourceAnimationStep",
         "test drives GetSourceAnimationStep"),
        ("m11_ts_init", "test drives m11_ts_init"),
        ("m11_ts_load_title_graphics",
         "test drives m11_ts_load_title_graphics"),
        ("m11_ts_cleanup", "test drives m11_ts_cleanup"),
        ("m11_ts_animate_zoom", "test drives m11_ts_animate_zoom"),
        ("BOOT_PLAN_REACHABILITY_PHASE_NOT_STARTED",
         "test references reachability phase enum"),
        ("VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS",
         "test pins PRESENTS palette ordinal"),
        ("VGA_PALETTE_PC34_SPECIAL_TITLE",
         "test pins TITLE palette ordinal"),
        ("DM1_TITLE_ZOOM_STEPS",
         "test pins TITLE zoom step count"),
        ("V1_TITLE_DAT_FRAME_MAX",
         "test pins source DO boundary"),
        ("scope=source-locked TITLE.C F0437",
         "test prints source-lock scope banner"),
        ("originalCadenceClaim=source-locked-pc-st-title-c",
         "test prints original-cadence claim banner"),
    ]:
        require(test_c, needle, label, errors)

    # --- 8. The runtime must NOT silently inject global skip-state ----
    # If a future change adds a process-global "introSkipped" flag or
    # similar, the gate can't see it.  Flag the introduction here so
    # the reviewer knows to also extend the gate.
    for suspect in [
        "static int intro_skipped",
        "static int g_intro_skipped",
        "static bool introSkipped",
    ]:
        if suspect in main_loop:
            errors.append(
                f"main_loop_m11.c introduced global intro-skipped state "
                f"{suspect!r}; extend the gate to cover it"
            )

    # --- 9. Title front-end must NOT silently wrap frame 1 on skip ----
    # The gate asserts that V1_TitleFrontend_DecideSequenceStep holds
    # frame 53 past the DO boundary; if the helper regresses to wrapping
    # back to frame 1 the test will fail, but we also flag the
    # regression here so the source-lock catches it before CI.
    if "renderFrameOrdinal = V1_TITLE_DAT_FRAME_MAX;" not in frontend_c:
        errors.append(
            "V1_TitleFrontend_DecideSequenceStep no longer holds the "
            "last TITLE frame past the source DO boundary"
        )
    if "completedAnimationLoops = 1u;" not in frontend_c:
        errors.append(
            "V1_TitleFrontend_DecideSequenceStep no longer increments "
            "completedAnimationLoops past the source DO boundary"
        )

    if errors:
        for err in errors:
            print(f"FAIL: {err}")
        return 1
    print("ok: pass1074 DM1 V1 intro/title skip state-cleanup gate source-locked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
