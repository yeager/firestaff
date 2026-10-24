#!/usr/bin/env python3
"""Lint the shared game-gap review and game/platform status formats.

This checks the common gap classifications, review sections and priority order,
then validates that each platform row has a recognized, defined status plus an
explicit scope/open boundary. It does not validate prose truth or prove parity.
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / "docs/PLATFORM_STATUS.md"
GAP_DOC = ROOT / "docs/MISSING_FUNCTIONS_BY_GAME.md"
GAMES = (
    "Dungeon Master",
    "Chaos Strikes Back",
    "Dungeon Master II: The Legend of Skullkeep",
    "DM Nexus",
    "Theron's Quest",
)
STATUSES = {
    "Playable",
    "Playable from ZIP or extracted files",
    "Verified route",
    "Verified bounded route",
    "Verified bounded routes",
    "Verified bounded title route",
    "Data path",
    "Preservation",
    "Unsupported",
}
BASE_STATUS = {
    "Playable from ZIP or extracted files": "Playable",
    "Verified bounded route": "Verified route",
    "Verified bounded routes": "Verified route",
    "Verified bounded title route": "Verified route",
}
TABLE_HEADER = ("Platform", "Status", "Current scope", "Open boundary")
GAP_GAMES = ("DM1", "DM2", "CSB", "Nexus", "Theron")
GAP_CATEGORIES = (
    "Missing code",
    "Missing original data",
    "Missing verification",
    "Intentionally closed",
)


def _cells(line: str) -> list[str]:
    return [cell.strip() for cell in line.strip().strip("|").split("|")]


def inspect_document(text: str) -> list[str]:
    failures: list[str] = []
    lines = text.splitlines()
    try:
        status_heading = lines.index("## Status terms")
    except ValueError:
        failures.append("missing status-term definitions")
        defined_statuses: set[str] = set()
    else:
        status_lines = []
        for line in lines[status_heading + 1 :]:
            if line.startswith("## "):
                break
            status_lines.append(line)
        defined_statuses = {
            match.group(1)
            for line in status_lines
            if (cells := _cells(line)) and len(cells) == 2
            and (match := re.fullmatch(r"\*\*(.+)\*\*", cells[0]))
        }
        for required in {BASE_STATUS.get(status, status) for status in STATUSES}:
            if required not in defined_statuses:
                failures.append(f"status-term table does not define {required!r}")
    seen: set[str] = set()
    active_game: str | None = None
    table_seen = False
    row_counts: dict[str, int] = {}
    platforms: dict[str, set[str]] = {}

    for line_number, line in enumerate(lines, 1):
        if line.startswith("## "):
            heading = line[3:].strip()
            if heading in GAMES:
                active_game = heading
                seen.add(heading)
                table_seen = False
                row_counts.setdefault(heading, 0)
                platforms.setdefault(heading, set())
            else:
                active_game = None
            continue
        if active_game is None or not line.startswith("|"):
            continue
        cells = _cells(line)
        if cells == list(TABLE_HEADER):
            if table_seen:
                failures.append(f"line {line_number}: duplicate table for {active_game}")
            table_seen = True
            continue
        if not table_seen or all(set(cell) <= {"-", ":", " "} for cell in cells):
            continue
        if len(cells) != 4:
            failures.append(f"line {line_number}: {active_game} row must have four cells")
            continue
        platform, status_cell, scope, boundary = cells
        match = re.fullmatch(r"\*\*(.+)\*\*", status_cell)
        status = match.group(1) if match else ""
        if not platform:
            failures.append(f"line {line_number}: {active_game} platform name is empty")
        elif platform in platforms[active_game]:
            failures.append(f"line {line_number}: duplicate platform row {active_game}/{platform}")
        else:
            platforms[active_game].add(platform)
        if status not in STATUSES:
            failures.append(f"line {line_number}: {active_game}/{platform} has unknown status {status!r}")
        elif BASE_STATUS.get(status, status) not in defined_statuses:
            failures.append(f"line {line_number}: {active_game}/{platform} status has no definition")
        if not scope:
            failures.append(f"line {line_number}: {active_game}/{platform} has no current scope")
        if not boundary:
            failures.append(f"line {line_number}: {active_game}/{platform} has no open boundary")
        row_counts[active_game] += 1

    for game in GAMES:
        if game not in seen:
            failures.append(f"missing game section: {game}")
        elif row_counts.get(game, 0) == 0:
            failures.append(f"{game} has no platform rows")
    return failures


def inspect_gap_review(text: str) -> list[str]:
    failures: list[str] = []
    lines = text.splitlines()
    headings = {line[3:].strip() for line in lines if line.startswith("## ")}
    for game in GAP_GAMES:
        if game not in headings:
            failures.append(f"missing gap-review section: {game}")
    for category in GAP_CATEGORIES:
        if f"**{category}**" not in text:
            failures.append(f"missing shared gap classification: {category}")
    if "Shared Priority Order" not in headings:
        failures.append("missing shared priority order")
    return failures


def main() -> int:
    try:
        text = DOC.read_text(encoding="utf-8")
    except OSError as exc:
        print(f"platform status matrix: cannot read {DOC}: {exc}")
        return 1
    try:
        gap_text = GAP_DOC.read_text(encoding="utf-8")
    except OSError as exc:
        print(f"gap review inventory: cannot read {GAP_DOC}: {exc}")
        return 1
    failures = inspect_gap_review(gap_text) + inspect_document(text)
    if failures:
        for failure in failures:
            print(f"FAIL: {failure}")
        return 1
    print("shared parity review and platform-status structures OK (claims remain evidence-review responsibilities)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
