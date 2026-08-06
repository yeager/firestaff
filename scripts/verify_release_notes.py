#!/usr/bin/env python3
"""Reject release notes that do not account for every functional delta."""

import argparse
import re
import sys
from pathlib import Path


HEADER_RE = re.compile(r"^# Firestaff v([^\s]+)\s*$")
SECTION_NAMES = ("Added", "Changed", "Removed")
GENERIC_TERMS = ("various", "miscellaneous", "improvements", "updates")


def fail(message: str) -> None:
    print(f"release notes: {message}", file=sys.stderr)
    raise SystemExit(1)


def selected_section(notes: str, version: str) -> list[str]:
    expected = f"# Firestaff v{version}"
    lines = notes.splitlines()
    start = next((i for i, line in enumerate(lines) if line == expected), None)
    if start is None:
        fail(f"missing exact heading {expected!r}")

    end = len(lines)
    for i in range(start + 1, len(lines)):
        if HEADER_RE.match(lines[i]):
            end = i
            break
    return lines[start:end]


def verify_category(lines: list[str], name: str) -> None:
    heading = f"## {name}"
    try:
        start = lines.index(heading) + 1
    except ValueError:
        fail(f"missing required {heading!r} category")

    end = len(lines)
    for i in range(start, len(lines)):
        if lines[i].startswith("## "):
            end = i
            break

    entries = [line for line in lines[start:end] if line.strip()]
    if not entries:
        fail(f"{heading!r} must explicitly state its functional delta or 'None.'")
    if any(not line.startswith("- ") for line in entries):
        fail(f"{heading!r} may contain only bullet entries")

    for entry in entries:
        body = entry[2:]
        lower = body.lower()
        if any(term in lower for term in GENERIC_TERMS):
            fail(f"{heading!r} contains generic wording: {entry!r}")
        if body.startswith("None."):
            continue
        if not re.match(r"`[^`]+`:\s+\S", body):
            fail(
                f"{heading!r} must name the changed function or feature in backticks: "
                f"{entry!r}"
            )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--notes", required=True, type=Path)
    parser.add_argument("--version", required=True)
    args = parser.parse_args()

    try:
        notes = args.notes.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"cannot read {args.notes}: {exc}")

    section = selected_section(notes, args.version)
    for name in SECTION_NAMES:
        verify_category(section, name)

    print(f"release notes: {args.notes} v{args.version} has concrete functional deltas")


if __name__ == "__main__":
    main()
