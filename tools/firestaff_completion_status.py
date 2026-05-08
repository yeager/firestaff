#!/usr/bin/env python3
"""Print the verified Firestaff completion matrix.

The matrix is deliberately conservative: completionPercent comes from
parity-evidence/verification/firestaff_completion_matrix.json and counts only
repo-evidenced points in the documented 100-point parity model.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "parity-evidence/verification/firestaff_completion_matrix.json"
EXPECTED_TARGETS = (
    "DM1 V1",
    "DM1 V2",
    "CSB V1",
    "CSB V2",
    "DM2 V1",
    "DM2 V2",
    "DM Nexus V1",
    "DM Nexus V2",
)


def load_matrix() -> dict[str, Any]:
    data = json.loads(MATRIX.read_text(encoding="utf-8"))
    if data.get("schema") != "firestaff.completion_matrix.v1":
        raise SystemExit(f"unsupported completion matrix schema in {MATRIX}")
    return data


def validate_matrix(data: dict[str, Any]) -> list[dict[str, Any]]:
    criteria = data.get("criteria")
    rows = data.get("rows")
    if not isinstance(criteria, list) or not criteria:
        raise SystemExit("completion matrix has no criteria")
    if not isinstance(rows, list):
        raise SystemExit("completion matrix has no rows")
    weights = {c.get("id"): c.get("weight") for c in criteria if isinstance(c, dict)}
    if sum(v for v in weights.values() if isinstance(v, int)) != 100:
        raise SystemExit("completion criteria weights must total 100")
    by_target = {r.get("target"): r for r in rows if isinstance(r, dict)}
    missing = [target for target in EXPECTED_TARGETS if target not in by_target]
    if missing:
        raise SystemExit(f"completion matrix missing targets: {', '.join(missing)}")
    ordered: list[dict[str, Any]] = []
    for target in EXPECTED_TARGETS:
        row = by_target[target]
        scores = row.get("scores")
        if not isinstance(scores, dict):
            raise SystemExit(f"{target}: scores must be an object")
        total = 0
        for criterion, weight in weights.items():
            value = scores.get(criterion)
            if not (isinstance(value, list) and len(value) == 2):
                raise SystemExit(f"{target}: missing score for {criterion}")
            score, note = value
            if not isinstance(score, (int, float)) or score < 0 or score > weight:
                raise SystemExit(f"{target}: invalid score {criterion}={score}/{weight}")
            if not isinstance(note, str) or not note.strip():
                raise SystemExit(f"{target}: missing evidence note for {criterion}")
            total += score
        if row.get("points") != total:
            raise SystemExit(f"{target}: points mismatch {row.get('points')} != {total}")
        if row.get("completionPercent") != round(total, 1):
            raise SystemExit(f"{target}: completionPercent mismatch")
        ordered.append(row)
    return ordered


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=ROOT, help="accepted for compatibility; must be this checkout")
    parser.add_argument("--json", action="store_true", help="print raw matrix JSON")
    args = parser.parse_args()
    if args.repo.resolve() != ROOT.resolve():
        parser.error(f"--repo must be this checkout: {ROOT}")

    data = load_matrix()
    rows = validate_matrix(data)
    if args.json:
        print(json.dumps(data, indent=2, ensure_ascii=False))
        return 0
    for row in rows:
        blockers = row.get("primaryBlockers") or []
        blocker = blockers[0] if blockers else "none"
        print(
            f"{row['target']} | completionPercent={row['completionPercent']}% "
            f"| points={row['points']}/100 | status={row['status']} | primaryBlocker={blocker}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
