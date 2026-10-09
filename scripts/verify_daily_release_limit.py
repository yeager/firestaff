#!/usr/bin/env python3
"""Enforce one newly published Firestaff release per Stockholm calendar day."""

from __future__ import annotations

import argparse
import json
import sys
from datetime import datetime
from zoneinfo import ZoneInfo


LOCAL_ZONE = ZoneInfo("Europe/Stockholm")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tag", required=True, help="Release tag this workflow will publish")
    args = parser.parse_args()

    today = datetime.now(LOCAL_ZONE).date()
    releases = []
    for line_number, line in enumerate(sys.stdin, start=1):
        line = line.strip()
        if not line:
            continue
        try:
            release = json.loads(line)
        except json.JSONDecodeError as error:
            print(f"Invalid GitHub release JSON on line {line_number}: {error}", file=sys.stderr)
            return 2
        if not isinstance(release, dict):
            print(f"Invalid GitHub release record on line {line_number}", file=sys.stderr)
            return 2
        releases.append(release)

    published_today = []
    for release in releases:
        published_at = release.get("published_at")
        if release.get("draft") or not published_at:
            continue
        try:
            timestamp = datetime.fromisoformat(published_at.replace("Z", "+00:00"))
        except (AttributeError, ValueError):
            print(
                f"Invalid published_at value for release {release.get('tag_name', '(unknown)')}",
                file=sys.stderr,
            )
            return 2
        if timestamp.astimezone(LOCAL_ZONE).date() == today:
            published_today.append(str(release.get("tag_name", "(unknown)")))

    if published_today:
        tags = ", ".join(published_today)
        print(
            f"::error::A Firestaff release was already published in Europe/Stockholm today "
            f"({today}): {tags}. The limit is one release per calendar day.",
            file=sys.stderr,
        )
        return 1

    print(f"Daily release limit available for {today} (Europe/Stockholm).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
