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
    "DM1 V1": 78,
    "DM1 V2": 51,
    "CSB V1": 43,
    "CSB V2": 31,
    "DM2 V1": 36,
    "DM2 V2": 16,
    "DM Nexus V1": 37,
    "DM Nexus V2": 20,
    "Theron V1": 72,
}
FAMILY_TARGETS = {
    "DM1": ("DM1 V1", "DM1 V2"),
    "CSB": ("CSB V1", "CSB V2"),
    "DM2": ("DM2 V1", "DM2 V2"),
    "DM Nexus": ("DM Nexus V1", "DM Nexus V2"),
    "Theron's Quest": ("Theron V1",),
}
VERSION_PROFILES = {
    "DM1": 16,
    "CSB": 9,
    "DM2": 7,
    "DM Nexus": 5,
    "Theron's Quest": 4,
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
    active_target = None
    documented_scores: dict[str, int] = {}
    for line in doc.splitlines():
        if line.startswith("### "):
            active_target = line[4:]
        elif active_target in rows and line.startswith("| `"):
            cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
            if len(cells) >= 2:
                score = cells[1].split("/", 1)[0]
                if score.isdigit():
                    documented_scores[active_target] = documented_scores.get(active_target, 0) + int(score)
    for target, percent in EXPECTED.items():
        assert f"| {target} | {percent}% | {percent}/100 |" in doc, target
        assert documented_scores.get(target) == percent, (
            f"documented criterion scores do not sum to {percent} for {target}: "
            f"{documented_scores.get(target)}"
        )
    for family, targets in FAMILY_TARGETS.items():
        mean = round(sum(rows[target]["points"] for target in targets) / len(targets), 1)
        summary = f"| {family} | {len(targets)} | {VERSION_PROFILES[family]} | {mean:.1f}% |"
        assert summary in doc, f"stale family roll-up: expected {summary}"
    total_points = sum(row["points"] for row in rows.values())
    total_summary = (
        f"| **Total** | **{len(rows)}** | **{sum(VERSION_PROFILES.values())}** | "
        f"**{total_points / len(rows):.1f}%** | Unweighted mean of the {len(rows)} runtime targets: "
        f"{total_points}/{len(rows) * 100}. |"
    )
    assert total_summary in doc, f"stale completion total: expected {total_summary}"
    out = subprocess.check_output([sys.executable, str(ROOT / "tools/firestaff_completion_status.py")], text=True)
    for target, percent in EXPECTED.items():
        assert f"{target} | completionPercent={percent}%" in out, target
    print("firestaff completion matrix verifier OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
