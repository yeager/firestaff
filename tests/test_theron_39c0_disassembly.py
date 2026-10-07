#!/usr/bin/env python3
"""Verify the checked-in $39c0 listing against recorded MAME output."""

from __future__ import annotations

import pathlib
import re


LISTING = pathlib.Path(__file__).resolve().parents[1] / (
    "docs/source-lock/theron-disassembly/theron-us-captured-39c0-linear-huc6280.asm"
)
REFERENCE = pathlib.Path(__file__).resolve().parents[1] / (
    "docs/source-lock/theron-disassembly/theron-us-captured-39c0-unidasm.lst"
)
ROW = re.compile(r"^([0-9a-f]{4}):\s+((?:[0-9a-f]{2}\s+)+)(.*)$", re.I)


def rows(lines: list[str]) -> list[tuple[str, str, str]]:
    result = []
    for line in lines:
        match = ROW.match(line)
        if match:
            result.append((
                match.group(1).lower(),
                " ".join(match.group(2).split()).lower(),
                match.group(3).strip(),
            ))
    return result


def main() -> int:
    checked_in = rows(LISTING.read_text(encoding="utf-8").splitlines())
    observed = rows(REFERENCE.read_text(encoding="utf-8").splitlines())
    if not checked_in or not observed:
        raise ValueError("listing and recorded disassembler output must contain rows")
    if checked_in != observed:
        for index, (actual, expected) in enumerate(zip(checked_in, observed)):
            if actual != expected:
                raise ValueError(f"row {index + 1} differs: {actual!r} != {expected!r}")
        raise ValueError("disassembly listings differ")
    print("PASS: $39c0 checked-in HuC6280 listing matches recorded MAME unidasm output")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
