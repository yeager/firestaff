#!/usr/bin/env python3
"""Verify Theron V1 probe registration hygiene.

The Theron startup/Track 02 lane uses probe files as executable evidence.
Every `probes/theron/*.c` file should therefore be wired into CMake, and
obsolete descriptor-entry API names must not reappear in source.
"""

from __future__ import annotations

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
THERON_PROBE_DIR = ROOT / "probes" / "theron"
SOURCE_DIRS = [
    ROOT / "include",
    ROOT / "src" / "theron",
    ROOT / "probes" / "theron",
    ROOT / "tests",
]

OBSOLETE_SYMBOLS = [
    "Theron_Track02DescriptorTableRole",
    "Theron_Track02DescriptorEntrySemantic",
    "theron_v1_track02_classify_descriptor_entry",
    "theron_v1_track02_descriptor_table_role_name",
]
OBSOLETE_PATTERNS = [
    (symbol, re.compile(rf"(?<![A-Za-z0-9_]){re.escape(symbol)}(?![A-Za-z0-9_])"))
    for symbol in OBSOLETE_SYMBOLS
]


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    sys.exit(1)


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"could not read {path}: {exc}")
        raise AssertionError("unreachable")


def verify_probe_sources_registered(cmake: str) -> None:
    missing: list[str] = []
    for probe in sorted(THERON_PROBE_DIR.glob("*.c")):
        rel = probe.relative_to(ROOT).as_posix()
        if rel not in cmake:
            missing.append(rel)

    if missing:
        for rel in missing:
            print(f"missing CMake reference: {rel}")
        fail(f"{len(missing)} Theron probe source(s) are not registered")

    print(f"OK: {len(list(THERON_PROBE_DIR.glob('*.c')))} Theron probe sources are registered")


def verify_no_obsolete_symbols() -> None:
    offenders: list[str] = []
    for base in SOURCE_DIRS:
        if not base.exists():
            continue
        for path in sorted(base.rglob("*")):
            if path.suffix not in {".c", ".h", ".py"}:
                continue
            text = read(path)
            for symbol, pattern in OBSOLETE_PATTERNS:
                if pattern.search(text):
                    offenders.append(f"{path.relative_to(ROOT).as_posix()}: {symbol}")

    if offenders:
        for offender in offenders:
            print(f"obsolete symbol: {offender}")
        fail(f"{len(offenders)} obsolete descriptor-entry symbol reference(s) found")

    print("OK: obsolete descriptor-entry API names are absent")


def main() -> int:
    cmake = read(ROOT / "CMakeLists.txt")
    verify_probe_sources_registered(cmake)
    verify_no_obsolete_symbols()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
