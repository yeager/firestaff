#!/usr/bin/env python3
"""Keep source-order-only CSB viewport contract traces out of Firestaff M10."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
CMAKE = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
ENTRIES = (
    ("csb_v1_viewport_d1c_f0115_thing_pass_pc34_compat.c",
     "firestaff_csb_v1_pc_real_asset_ornament_blit_probe"),
    ("csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.c",
     "test_csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat"),
    ("csb_v1_viewport_d2l2_d2r2_f0111_partly_open_pc34_compat.c",
     "test_csb_v1_viewport_d2l2_d2r2_f0111_partly_open_pc34_compat"),
    ("csb_v1_viewport_d2l2_d2r2_wall_pc34_compat.c",
     "test_csb_v1_viewport_d2l2_d2r2_wall_pc34_compat"),
    ("csb_v1_viewport_d3c_f0107_f0108_first_backdrop_pc34_compat.c",
     "test_csb_v1_viewport_d3c_f0107_f0108_first_backdrop_pc34_compat"),
)


def fail(message: str) -> int:
    print(f"FAIL: {message}")
    return 1


def target_names_source(target: str, source: str) -> bool:
    start = CMAKE.find(f"add_executable({target}")
    if start < 0:
        return False
    end = CMAKE.find("add_executable(", start + 1)
    return source in CMAKE[start:end if end >= 0 else None]


def main() -> int:
    first_source = CMAKE.find(ENTRIES[0][0])
    exclusion_start = CMAKE.rfind("list(REMOVE_ITEM M10_SOURCES", 0, first_source)
    exclusion_end = CMAKE.find("list(FILTER M10_SOURCES", exclusion_start)
    exclusion = CMAKE[exclusion_start:exclusion_end]
    if exclusion_start < 0 or exclusion_end < 0:
        return fail("M10 exclusion block is missing")
    for source, target in ENTRIES:
        if source not in exclusion:
            return fail(f"{source} is not excluded from M10")
        if not target_names_source(target, source):
            return fail(f"{target} must compile {source} explicitly")
    print("PASS: CSB viewport source-order contract bundle is test/probe-only")
    return 0


if __name__ == "__main__":
    sys.exit(main())
