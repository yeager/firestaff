#!/usr/bin/env python3
"""Verify the Firestaff completion matrix and rendered status output."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "parity-evidence/verification/firestaff_completion_matrix.json"
DOC = ROOT / "docs/parity/COMPLETION_MATRIX.md"
EXPECTED = {
    "DM1 V1": 57,
    "DM1 V2": 27,
    "CSB V1": 18,
    "CSB V2": 0,
    "DM2 V1": 7,
    "DM2 V2": 0,
    "DM Nexus V1": 0,
    "DM Nexus V2": 0,
}


def main() -> int:
    data = json.loads(MATRIX.read_text(encoding="utf-8"))
    assert data["schema"] == "firestaff.completion_matrix.v1"
    assert sum(c["weight"] for c in data["criteria"]) == 100
    rows = {r["target"]: r for r in data["rows"]}
    assert set(rows) == set(EXPECTED)
    for target, percent in EXPECTED.items():
        row = rows[target]
        total = sum(score for score, _ in row["scores"].values())
        assert total == row["points"] == percent, (target, total, row["points"], percent)
        assert row["completionPercent"] == percent, target
        assert row["maxPoints"] == 100, target
        assert row["primaryBlockers"], target
    doc = DOC.read_text(encoding="utf-8")
    for target, percent in EXPECTED.items():
        assert f"| {target} | {percent}% | {percent}/100 |" in doc, target
    out = subprocess.check_output([sys.executable, str(ROOT / "tools/firestaff_completion_status.py")], text=True)
    for target, percent in EXPECTED.items():
        assert f"{target} | completionPercent={percent}%" in out, target
    print("firestaff completion matrix verifier OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
