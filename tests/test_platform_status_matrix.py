#!/usr/bin/env python3
"""Exercise the platform-status structure gate against the real status page."""
from __future__ import annotations

import importlib.util
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "verify_platform_status_matrix", ROOT / "tools/verify_platform_status_matrix.py"
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def main() -> int:
    actual = MODULE.DOC.read_text(encoding="utf-8")
    assert MODULE.inspect_document(actual) == []
    gap_review = MODULE.GAP_DOC.read_text(encoding="utf-8")
    assert MODULE.inspect_gap_review(gap_review) == []
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    registration = (
        "NAME parity_review_inventory_structure\n"
        "            COMMAND ${Python3_EXECUTABLE} "
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_platform_status_matrix.py"
    )
    assert registration in cmake, "platform status gate is not registered with CTest"

    malformed = actual.replace(
        "| PC DOS 3.4 | **Playable** |",
        "| PC DOS 3.4 | **Synthetic parity** |",
        1,
    )
    failures = MODULE.inspect_document(malformed)
    assert any("unknown status 'Synthetic parity'" in failure for failure in failures)

    malformed_review = gap_review.replace("**Missing code**", "**Unreviewed**", 1)
    failures = MODULE.inspect_gap_review(malformed_review)
    assert any("missing shared gap classification: Missing code" in failure for failure in failures)

    incomplete = actual.replace(
        "| X68000 | **Unsupported** | Not part of the DM1 support matrix. | None planned in the current target. |",
        "| X68000 | **Unsupported** | | None planned in the current target. |",
        1,
    )
    failures = MODULE.inspect_document(incomplete)
    assert any("Dungeon Master/X68000 has no current scope" in failure for failure in failures)

    duplicate = actual.replace(
        "| X68000 | **Unsupported** | Not part of the DM1 support matrix. | None planned in the current target. |",
        "| PC-9801 | **Preservation** | Japanese floppy provenance is documented. | No Firestaff runtime route. |",
        1,
    )
    failures = MODULE.inspect_document(duplicate)
    assert any("duplicate platform row Dungeon Master/PC-9801" in failure for failure in failures)
    print("platform status matrix tests OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
