#!/usr/bin/env python3
"""Reject release notes that do not name their version's concrete deltas."""

import argparse
import re
import sys
from pathlib import Path


HEADER_RE = re.compile(r"^# Firestaff v([^\s]+)\s*$")
GAME_NAMES = ("DM1", "DM2", "CSB", "Nexus", "Theron")
SECTION_NAMES = ("Added", "Changed", "Removed")
GENERIC_TERMS = (
    "various",
    "miscellaneous",
    "improvements",
    "updates",
    "and more",
    "etc.",
    "etc",
)
ACTION_WORDS = {
    "Added": ("add", "introduc", "register", "admit", "enable", "create", "promot"),
    "Changed": (
        "change",
        "fix",
        "replace",
        "prevent",
        "load",
        "read",
        "write",
        "use",
        "move",
        "remove",
        "reject",
        "block",
        "bind",
        "restore",
        "update",
        "promot",
        "verif",
        "count",
    ),
    "Removed": ("remove", "delete", "drop", "retire", "disable", "unregister"),
}


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


def verify_category(lines: list[str], game: str, name: str) -> None:
    heading = f"### {name}"
    try:
        start = lines.index(heading) + 1
    except ValueError:
        fail(f"missing required {heading!r} category under {game}")

    end = len(lines)
    for i in range(start, len(lines)):
        if lines[i].startswith("### ") or lines[i].startswith("## "):
            end = i
            break

    entries = []
    for line in lines[start:end]:
        if not line.strip():
            continue
        if line.startswith("- "):
            entries.append(line)
        elif entries:
            entries[-1] += " " + line.strip()
        else:
            fail(f"{heading!r} may contain only bullet entries")
    if not entries:
        fail(f"{heading!r} must explicitly state its functional delta or 'None.'")

    for entry in entries:
        body = entry[2:]
        lower = body.lower()
        if any(term in lower for term in GENERIC_TERMS):
            fail(f"{heading!r} contains generic wording: {entry!r}")
        if body == "None.":
            continue
        if body.startswith("None."):
            fail(f"{heading!r} must use exactly 'None.' when there is no delta")
        if not re.match(r"`[^`]+`:\s+\S", body):
            fail(
                f"{heading!r} must name the changed function or feature in backticks: "
                f"{entry!r}"
            )
        action_text = body.split(":", 1)[1].lower()
        if not any(word in action_text for word in ACTION_WORDS[name]):
            fail(
                f"{heading!r} must state what changed, not only its result: "
                f"{entry!r}"
            )


def verify_game(lines: list[str], game: str) -> None:
    heading = f"## {game}"
    try:
        start = lines.index(heading) + 1
    except ValueError:
        fail(f"missing required game section {heading!r}")

    end = len(lines)
    for i in range(start, len(lines)):
        if lines[i].startswith("## "):
            end = i
            break
    game_lines = lines[start:end]
    for name in SECTION_NAMES:
        verify_category(game_lines, game, name)


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
    for game in GAME_NAMES:
        verify_game(section, game)

    print(f"release notes: {args.notes} v{args.version} has concrete functional deltas")


if __name__ == "__main__":
    main()
