#!/usr/bin/env python3
"""Keep the unconsumed CSB hidden-item safety loader out of Firestaff M10."""

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
    exclusion_start = CMAKE.rfind("list(REMOVE_ITEM M10_SOURCES", 0, source_pos)
    exclusion_end = CMAKE.find("list(FILTER M10_SOURCES", exclusion_start)
    exclusion = CMAKE[exclusion_start:exclusion_end]
    if exclusion_start < 0 or exclusion_end < 0 or SOURCE not in exclusion:
        return fail("hidden-item safety loader is not excluded from M10")
    if CMAKE.count(CONTRACT_DEFINE) != 2:
        return fail("hidden-item contract mode must be restricted to its test and real-media probe")
    if not target_names_source("test_csb_v1_graphics_hidden_item_skip"):
        return fail("the focused hidden-item regression must compile its helper explicitly")
    if not target_names_source("firestaff_csb_v1_real_asset_launch_probe"):
        return fail("the real-media launch probe must compile its helper explicitly")
    print("PASS: unconsumed CSB hidden-item safety loader is test/probe-only")
    return 0


if __name__ == "__main__":
    sys.exit(main())
