#!/usr/bin/env python3
"""Enforce a rolling 12-hour cooldown between published Firestaff releases."""

from __future__ import annotations

import argparse
import json
import sys
from datetime import datetime, timedelta, timezone


COOLDOWN = timedelta(hours=12)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tag", required=True, help="Release tag this workflow will publish")
    args = parser.parse_args()

    now = datetime.now(timezone.utc)
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

    cooling_releases = []
    for release in releases:
        published_at = release.get("published_at")
        if release.get("draft") or not published_at:
            continue
        try:
            timestamp = datetime.fromisoformat(published_at.replace("Z", "+00:00"))
            if timestamp.tzinfo is None or timestamp.utcoffset() is None:
                raise ValueError("timestamp must include a timezone")
        except (AttributeError, ValueError):
            print(
                f"Invalid published_at value for release {release.get('tag_name', '(unknown)')}",
                file=sys.stderr,
            )
            return 2
        age = now - timestamp.astimezone(timezone.utc)
        if age < COOLDOWN:
            cooling_releases.append(str(release.get("tag_name", "(unknown)")))

    other_recent_releases = [tag for tag in cooling_releases if tag != args.tag]
    if other_recent_releases:
        tags = ", ".join(other_recent_releases)
        print(
            f"::error::A Firestaff release was published within the rolling 12-hour "
            f"cooldown: {tags}. Wait until 12 hours have elapsed.",
            file=sys.stderr,
        )
        return 1

    if args.tag in cooling_releases:
        print(
            f"Release {args.tag} is already published; an idempotent same-tag retry "
            "may update it without creating a second release."
        )
        return 0

    print("Rolling 12-hour release cooldown clear.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
