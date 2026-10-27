#!/usr/bin/env python3
"""Regression tests for the rolling 12-hour Firestaff release cooldown."""

from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timedelta, timezone
from pathlib import Path


SCRIPT = Path(__file__).with_name("verify_release_cooldown.py")


def run_policy(
    tag: str,
    releases: list[dict[str, object]],
) -> subprocess.CompletedProcess[str]:
    payload = "".join(json.dumps(release) + "\n" for release in releases)
    return subprocess.run(
        [sys.executable, str(SCRIPT), "--tag", tag],
        input=payload,
        text=True,
        capture_output=True,
        check=False,
    )


def published(
    tag: str,
    timestamp: datetime,
    *,
    draft: bool = False,
) -> dict[str, object]:
    return {
        "tag_name": tag,
        "published_at": timestamp.isoformat(),
        "draft": draft,
    }


def main() -> int:
    now = datetime.now(timezone.utc)
    cases = [
        (
            "no previous release allows publication",
            "v3.0.418",
            [],
            0,
        ),
        (
            "a different release within 12 hours blocks publication",
            "v3.0.418",
            [published("v3.0.417", now - timedelta(hours=11))],
            1,
        ),
        (
            "a draft does not consume the cooldown",
            "v3.0.418",
            [published("v3.0.417-rc.1", now - timedelta(minutes=2), draft=True)],
            0,
        ),
        (
            "a release older than 12 hours allows publication",
            "v3.0.418",
            [published("v3.0.417", now - timedelta(hours=12, seconds=30))],
            0,
        ),
        (
            "retrying the same recent tag remains idempotent",
            "v3.0.417",
            [published("v3.0.417", now - timedelta(hours=1))],
            0,
        ),
        (
            "same-tag retry is blocked by a different recent release",
            "v3.0.417",
            [
                published("v3.0.417", now - timedelta(hours=1)),
                published("v3.0.416", now - timedelta(hours=2)),
            ],
            1,
        ),
        (
            "an invalid publication timestamp is rejected",
            "v3.0.418",
            [{"tag_name": "v3.0.417", "published_at": "invalid", "draft": False}],
            2,
        ),
        (
            "a publication timestamp without timezone is rejected",
            "v3.0.418",
            [{"tag_name": "v3.0.417", "published_at": "2026-10-27T08:00:00", "draft": False}],
            2,
        ),
    ]

    for description, tag, releases, expected in cases:
        result = run_policy(tag, releases)
        if result.returncode != expected:
            print(f"FAIL: {description}: expected exit {expected}, got {result.returncode}")
            if result.stdout:
                print(result.stdout, end="")
            if result.stderr:
                print(result.stderr, end="", file=sys.stderr)
            return 1
        print(f"PASS: {description}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
