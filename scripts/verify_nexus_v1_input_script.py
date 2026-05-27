#!/usr/bin/env python3
"""
verify_nexus_v1_input_script.py
================================
Validate a Nexus V1 deterministic input script (JSON).

Schema:
  {
    "name": string,
    "level": integer (0-15),
    "provenance_seed": string,
    "steps": [
      { "action": "MOVE_FORWARD"|"TURN_LEFT"|"TURN_RIGHT"|"STATE_HASH", "ticks": integer, "expected": string (optional) }
    ]
  }

Exit codes:
  0  — valid script
  1  — invalid schema

Usage:
  python3 scripts/verify_nexus_v1_input_script.py <script.json>
"""

import sys
import json
from pathlib import Path

VALID_ACTIONS = {"MOVE_FORWARD", "TURN_LEFT", "TURN_RIGHT", "STATE_HASH"}


def validate_script(path: Path) -> tuple[bool, list[str]]:
    errors = []
    try:
        with open(path) as f:
            script = json.load(f)
    except json.JSONDecodeError as e:
        return False, [f"JSON parse error: {e}"]
    except Exception as e:
        return False, [f"Cannot read file: {e}"]

    # name
    if not isinstance(script.get("name"), str) or not script["name"]:
        errors.append("'name' must be a non-empty string")

    # level
    lvl = script.get("level")
    if not isinstance(lvl, int) or not (0 <= lvl <= 15):
        errors.append(f"'level' must be integer 0-15, got: {lvl!r}")

    # provenance_seed
    if not isinstance(script.get("provenance_seed"), str):
        errors.append("'provenance_seed' must be a string")

    # steps
    steps = script.get("steps")
    if not isinstance(steps, list) or len(steps) == 0:
        errors.append("'steps' must be a non-empty array")
    else:
        for i, step in enumerate(steps):
            if not isinstance(step, dict):
                errors.append(f"step[{i}]: must be an object")
                continue
            action = step.get("action")
            if action not in VALID_ACTIONS:
                errors.append(f"step[{i}]: invalid action {action!r} (must be one of {VALID_ACTIONS})")
            ticks = step.get("ticks")
            if not isinstance(ticks, int) or ticks < 1:
                errors.append(f"step[{i}]: 'ticks' must be integer >= 1, got: {ticks!r}")
            if action == "STATE_HASH":
                expected = step.get("expected")
                if not isinstance(expected, str) or not expected.startswith("0x"):
                    errors.append(f"step[{i}]: STATE_HASH requires 'expected' as hex string (0x...)")

    return len(errors) == 0, errors


def main():
    if len(sys.argv) < 2:
        print("Usage: verify_nexus_v1_input_script.py <script.json>", file=sys.stderr)
        sys.exit(1)

    path = Path(sys.argv[1])
    ok, errors = validate_script(path)

    if not ok:
        print(f"FAIL: {path}")
        for e in errors:
            print(f"  - {e}")
        sys.exit(1)

    print(f"PASS: {path}")
    sys.exit(0)


if __name__ == "__main__":
    main()