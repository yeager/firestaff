#!/usr/bin/env python3
"""Regression tests for the one-published-release-per-day policy."""

from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timedelta
from pathlib import Path
from zoneinfo import ZoneInfo


SCRIPT = Path(__file__).with_name("verify_daily_release_limit.py")
LOCAL_ZONE = ZoneInfo("Europe/Stockholm")


def run_policy(tag: str, releases: list[dict[str, object]]) -> subprocess.CompletedProcess[str]:
    payload = "".join(json.dumps(release) + "\n" for release in releases)
    return subprocess.run(
        [sys.executable, str(SCRIPT), "--tag", tag],
        input=payload,
        text=True,
        capture_output=True,
        check=False,
    )


def published(tag: str, timestamp: datetime, *, draft: bool = False) -> dict[str, object]:
    return {
        "tag_name": tag,
        "published_at": timestamp.isoformat(),
        "draft": draft,
    }


def main() -> int:
    now = datetime.now(LOCAL_ZONE)
    yesterday = now - timedelta(days=1)

    cases = [
        (
            "no previous release allows publication",
            "v2.5.4",
            [],
            0,
        ),
        (
            "another release published today blocks publication",
            "v2.5.4",
            [published("v2.5.3", now)],
            1,
        ),
        (
            "a draft published today does not consume the quota",
            "v2.5.4",
            [published("v2.5.3-rc.1", now, draft=True)],
            0,
        ),
        (
            "a release from yesterday does not consume today's quota",
            "v2.5.4",
            [published("v2.5.3", yesterday)],
            0,
        ),
        (
            "retrying the same published tag updates the existing release",
            "v2.5.4",
            [published("v2.5.4", now)],
            0,
        ),
        (
            "same-tag retry is blocked after a different release today",
            "v2.5.4",
            [published("v2.5.4", now), published("v2.5.3", now)],
            1,
        ),
        (
            "retrying the same published tag on the next day is allowed",
            "v2.5.4",
            [published("v2.5.4", yesterday)],
            0,
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

    invalid = run_policy("v2.5.4", [{"tag_name": "v2.5.3", "published_at": "invalid", "draft": False}])
    if invalid.returncode != 2:
        print(f"FAIL: invalid publication timestamp: expected exit 2, got {invalid.returncode}")
        return 1
    print("PASS: invalid publication timestamp is rejected")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
