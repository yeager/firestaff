#!/usr/bin/env python3
"""Keep the source-backed CSB hidden-item decision available to M11."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
CMAKE = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
SOURCE = "csb_v1_graphics_hidden_item_skip_pc34_compat.c"
CONTRACT_DEFINE = "CSB_V1_HIDDEN_ITEM_SKIP_CONTRACT_ONLY=1"


def fail(message: str) -> int:
    print(f"FAIL: {message}")
    return 1


def target_names_source(target: str) -> bool:
    start = CMAKE.find(f"add_executable({target}")
    if start < 0:
        return False
    end = CMAKE.find("add_executable(", start + 1)
    return SOURCE in CMAKE[start:end if end >= 0 else None]


def main() -> int:
    source_pos = CMAKE.find(SOURCE)
    if source_pos < 0:
        return fail("hidden-item decision source is missing from CMake")
    exclusion_start = CMAKE.rfind("list(REMOVE_ITEM M10_SOURCES", 0, source_pos)
    exclusion_end = CMAKE.find("list(FILTER M10_SOURCES", exclusion_start)
    exclusion = CMAKE[exclusion_start:exclusion_end]
    if exclusion_start >= 0 and exclusion_end >= 0 and SOURCE in exclusion:
        return fail("M11's authenticated hidden-item decision is excluded from M10")
    if "csb_v1_graphics_hidden_should_skip_item" not in (ROOT / "src/engine/m11_game_view.c").read_text(encoding="utf-8"):
        return fail("M11 must consume the source-backed hidden-item decision")
    if CMAKE.count(CONTRACT_DEFINE) != 2:
        return fail("hidden-item contract mode must be restricted to its test and real-media probe")
    if not target_names_source("test_csb_v1_graphics_hidden_item_skip"):
        return fail("the focused hidden-item regression must compile its helper explicitly")
    if not target_names_source("firestaff_csb_v1_real_asset_launch_probe"):
        return fail("the real-media launch probe must compile its helper explicitly")
    print("PASS: M11 consumes the source-backed CSB hidden-item decision")
    return 0


if __name__ == "__main__":
    sys.exit(main())
