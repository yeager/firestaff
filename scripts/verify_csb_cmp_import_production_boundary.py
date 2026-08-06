#!/usr/bin/env python3
"""Keep portrait-only CMP fixture helpers out of Firestaff M10."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
CMAKE = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
CMP = "csb_v1_cmp_import_pc34_compat.c"
CONTRACT_DEFINE = "CSB_V1_CMP_IMPORT_CONTRACT_ONLY=1"


def fail(message: str) -> int:
    print(f"FAIL: {message}")
    return 1


def main() -> int:
    cmp_pos = CMAKE.find(CMP)
    exclusion_start = CMAKE.rfind("list(REMOVE_ITEM M10_SOURCES", 0, cmp_pos)
    exclusion_end = CMAKE.find("list(FILTER M10_SOURCES", exclusion_start)
    exclusion = CMAKE[exclusion_start:exclusion_end]
    if exclusion_start < 0 or exclusion_end < 0 or CMP not in exclusion:
        return fail("portrait-only CMP helper is not excluded from M10")
    if CMAKE.count(CONTRACT_DEFINE) != 3:
        return fail("CMP contract mode must be restricted to its two tests and real-asset probe")
    test_pos = CMAKE.find("add_executable(test_csb_v1_cmp_import")
    if test_pos < 0 or CMP not in CMAKE[test_pos:test_pos + 1600]:
        return fail("the focused CMP contract must compile its helper explicitly")
    print("PASS: portrait-only CMP helpers are test/probe-only")
    return 0


if __name__ == "__main__":
    sys.exit(main())
