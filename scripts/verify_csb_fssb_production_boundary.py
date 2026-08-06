#!/usr/bin/env python3
"""Keep Firestaff's synthetic FSSB contract out of the CSB product archive."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
CMAKE = ROOT / "CMakeLists.txt"
FSSB = "src/csb/csb_v1_save_export_import_pc34_compat.c"
UTILITY = "src/csb/csb_v1_utility_save_transaction_pc34_compat.c"
FSSB_NAME = Path(FSSB).name
UTILITY_NAME = Path(UTILITY).name


def fail(message: str) -> int:
    print(f"FAIL: {message}")
    return 1


def main() -> int:
    text = CMAKE.read_text(encoding="utf-8")
    fssb_exclusion = text.find(FSSB)
    utility_exclusion = text.find(UTILITY)
    exclusion_start = text.rfind("list(REMOVE_ITEM M10_SOURCES", 0,
                                 min(fssb_exclusion, utility_exclusion))
    exclusion_end = text.find("list(FILTER M10_SOURCES", exclusion_start)
    exclusion = text[exclusion_start:exclusion_end]
    if (exclusion_start < 0 or exclusion_end < 0 or
            FSSB_NAME not in exclusion or UTILITY_NAME not in exclusion):
        return fail("missing M10 exclusion for the FSSB contract")

    library_start = text.find("add_library(firestaff_m10 STATIC")
    if library_start < 0:
        return fail("firestaff_m10 library declaration is missing")
    library_end = text.find("# ──", library_start + 1)
    library = text[library_start:library_end if library_end >= 0 else len(text)]
    if FSSB in library or UTILITY in library:
        return fail("a synthetic FSSB source is listed directly in firestaff_m10")

    utility_test = text.find("add_executable(test_csb_v1_utility_save_transaction_pc34_compat")
    if utility_test < 0:
        return fail("missing focused Utility transaction contract target")
    utility_end = text.find(")", utility_test)
    utility_sources = text[utility_test:utility_end]
    if FSSB not in utility_sources or UTILITY not in utility_sources:
        return fail("focused Utility contract must compile both FSSB sources explicitly")

    print("PASS: CSB FSSB contract sources are excluded from firestaff_m10")
    return 0


if __name__ == "__main__":
    sys.exit(main())
