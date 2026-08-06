#!/usr/bin/env python3
"""Regression checks for the version-specific GitHub release-notes gate."""

import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
VERIFY = ROOT / "scripts" / "verify_release_notes.py"


def note(added: str, changed: str, removed: str) -> str:
    return f"""# Firestaff v9.8.7

## Added
{added}

## Changed
{changed}

## Removed
{removed}
"""


def run_case(contents: str, expected: int) -> None:
    with tempfile.TemporaryDirectory() as temp_dir:
        path = Path(temp_dir) / "notes.md"
        path.write_text(contents, encoding="utf-8")
        completed = subprocess.run(
            [sys.executable, str(VERIFY), "--notes", str(path), "--version", "9.8.7"],
            check=False,
            capture_output=True,
            text=True,
        )
    if completed.returncode != expected:
        raise AssertionError(
            f"expected exit {expected}, got {completed.returncode}: {completed.stderr}"
        )


def main() -> None:
    run_case(
        note(
            "- `dm2_v1_boot_load`: Adds FM Towns SKULL.EXP validation.",
            "- `dm2_runtime_refresh_music_map_trigger`: Uses the committed party coordinates.",
            "- None.",
        ),
        0,
    )
    run_case(
        note(
            "- `DM2`: Various improvements.",
            "- None.",
            "- None.",
        ),
        1,
    )
    run_case(
        note(
            "- `dm2_v1_boot_load`: FM Towns SKULL.EXP validation is available.",
            "- None.",
            "- None.",
        ),
        1,
    )
    run_case(
        note(
            "- None.",
            "- None.",
            "- None. No removal was made.",
        ),
        1,
    )
    print("release-notes verifier regression checks: PASS")


if __name__ == "__main__":
    main()
