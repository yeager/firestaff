#!/usr/bin/env python3
"""Regression checks for release-package release-note selection."""

import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from verify_release_notes import selected_section  # noqa: E402


def main() -> None:
    notes = (
        "# Unreleased\n\n"
        "## Fixed\n\n"
        "- Deferred work.\n\n"
        "# Firestaff v9.8.7\n\n"
        "## Fixed\n\n"
        "- `packager`: Keeps only this version.\n\n"
        "# Firestaff v9.8.6\n\n"
        "## Fixed\n\n"
        "- Historical work.\n"
    )
    actual = "\n".join(selected_section(notes, "9.8.7"))
    expected = "# Firestaff v9.8.7\n\n## Fixed\n\n- `packager`: Keeps only this version.\n"
    if actual != expected:
        raise AssertionError(f"unexpected selected release notes: {actual!r}")
    print("release-notes extraction regression checks: PASS")


if __name__ == "__main__":
    main()
