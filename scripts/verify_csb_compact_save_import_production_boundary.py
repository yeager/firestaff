#!/usr/bin/env python3
"""Keep the compact CSBGAME roster test reader out of Firestaff M10."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
CMAKE = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
IMPORTER = "csb_v1_save_import_path_pc34_compat.c"
CONTRACT_DEFINE = "CSB_V1_CSBWIN_SAVE_LOADER_BOUNDARY_CONTRACT_ONLY=1"


def fail(message: str) -> int:
    print(f"FAIL: {message}")
    return 1


def main() -> int:
    importer_pos = CMAKE.find(IMPORTER)
    exclusion_start = CMAKE.rfind("list(REMOVE_ITEM M10_SOURCES", 0,
                                  importer_pos)
    exclusion_end = CMAKE.find("list(FILTER M10_SOURCES", exclusion_start)
    exclusion = CMAKE[exclusion_start:exclusion_end]
    if exclusion_start < 0 or exclusion_end < 0 or IMPORTER not in exclusion:
        return fail("compact CSBGAME roster reader is not excluded from M10")
    if CMAKE.count(CONTRACT_DEFINE) != 2:
        return fail("only the focused CSBWin test and probe may enable compact roster parsing")
    test_pos = CMAKE.find("add_executable(test_csb_v1_save_import_path_pc34_compat")
    if test_pos < 0 or IMPORTER not in CMAKE[test_pos:test_pos + 2000]:
        return fail("the focused compact roster contract must compile its reader explicitly")
    print("PASS: compact CSBGAME roster reader is test/probe-only")
    return 0


if __name__ == "__main__":
    sys.exit(main())
