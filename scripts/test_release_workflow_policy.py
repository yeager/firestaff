#!/usr/bin/env python3
"""Guard the GitHub Actions release serialization and draft publication path."""

from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
WORKFLOW = ROOT / ".github" / "workflows" / "release.yml"


def main() -> int:
    text = WORKFLOW.read_text(encoding="utf-8")
    checks = {
        "all release runs share a non-cancelling concurrency group":
            "group: firestaff-release-12h-cooldown" in text
            and "cancel-in-progress: false" in text,
        "12-hour cooldown is checked before build and immediately before publication":
            text.count("python3 scripts/verify_release_cooldown.py --tag") == 2,
        "workflow has no daily-limit bypass": "override_daily_limit" not in text,
        "draft retries pass an explicit true or false value to gh release edit":
            'release_flags=(--draft="$FIRESTAFF_DRAFT")' in text,
    }
    for description, passed in checks.items():
        if not passed:
            print(f"FAIL: {description}")
            return 1
        print(f"PASS: {description}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
