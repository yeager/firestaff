#!/usr/bin/env python3
"""Keep local-buffer CSB viewport side-wall traces out of Firestaff M10."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
CMAKE = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
SOURCES = (
    "csb_v1_viewport_d2l_d2r_wall_pc34_compat.c",
    "csb_v1_viewport_d3l2_d3r2_wall_pc34_compat.c",
)
TARGETS = (
    "test_csb_v1_viewport_d2l_d2r_wall_pc34_compat",
    "test_csb_v1_viewport_d3l2_d3r2_wall_pc34_compat",
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
    first_source = CMAKE.find(SOURCES[0])
    exclusion_start = CMAKE.rfind("list(REMOVE_ITEM M10_SOURCES", 0, first_source)
    exclusion_end = CMAKE.find("list(FILTER M10_SOURCES", exclusion_start)
    exclusion = CMAKE[exclusion_start:exclusion_end]
    if exclusion_start < 0 or exclusion_end < 0:
        return fail("M10 exclusion block is missing")
    for source, target in zip(SOURCES, TARGETS):
        if source not in exclusion:
            return fail(f"{source} is not excluded from M10")
        if not target_names_source(target, source):
            return fail(f"{target} must compile {source} explicitly")
    print("PASS: CSB side-wall contract traces are test-only")
    return 0


if __name__ == "__main__":
    sys.exit(main())
