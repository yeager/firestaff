#!/usr/bin/env python3
"""Validate provenance of exported Nexus SH-2 decompilation results.

Ghidra output is evidence only when every requested function and disassembly
window was produced from one captured Saturn work-RAM image.  This tool reads
the comments emitted by the project's headless Ghidra export scripts and
produces a small JSON receipt.  It deliberately does not interpret a function
as a rendered menu or as a playable start path.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

SHA_RE = re.compile(r"^/\* input_sha256=([0-9a-f]{64}) \*/$", re.MULTILINE)
LANGUAGE_RE = re.compile(r"^/\* language=([^*]+?) \*/$", re.MULTILINE)
FUNCTION_RE = re.compile(
    r"^/\* requested=([0-9a-f]{8}) entry=([0-9a-f]{8}) "
    r"name=([^ ]+) bytes=(\d+) complete=(true|false) \*/$", re.MULTILINE
)
RANGE_RE = re.compile(r"^/\* range=([0-9a-f]{8})\.\.([0-9a-f]{8}) \*/$", re.MULTILINE)


def read_export(path: Path) -> tuple[str, str, list[dict[str, object]], list[dict[str, str]], dict[str, str]]:
    try:
        text = path.read_text(encoding="utf-8")
    except OSError as exc:
        raise ValueError(f"cannot read {path}: {exc}") from exc
    hashes = SHA_RE.findall(text)
    languages = [match.strip() for match in LANGUAGE_RE.findall(text)]
    if not hashes or not languages:
        raise ValueError(f"{path}: missing Ghidra input_sha256 or language header")
    if len(set(hashes)) != 1:
        raise ValueError(f"{path}: conflicting Ghidra input_sha256 headers")
    if len(set(languages)) != 1:
        raise ValueError(f"{path}: conflicting Ghidra language headers")
    functions = [
        {
            "requested": match.group(1),
            "entry": match.group(2),
            "name": match.group(3),
            "bytes": int(match.group(4)),
            "complete": match.group(5) == "true",
        }
        for match in FUNCTION_RE.finditer(text)
    ]
    ranges = [
        {"start": match.group(1), "end": match.group(2)}
        for match in RANGE_RE.finditer(text)
    ]
    if not functions and not ranges:
        raise ValueError(f"{path}: no exported functions or disassembly range")
    return (hashes[0], languages[0], functions, ranges,
            {"path": str(path)})


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exports", nargs="+", type=Path,
                        help="Ghidra function or disassembly export text files")
    parser.add_argument("--require", action="append", default=[], metavar="ADDRESS",
                        help="require a complete function requested at ADDRESS")
    args = parser.parse_args()

    rows: list[dict[str, object]] = []
    input_hashes: set[str] = set()
    languages: set[str] = set()
    functions: list[dict[str, object]] = []
    ranges: list[dict[str, object]] = []
    try:
        for path in args.exports:
            sha, language, exported, exported_ranges, row = read_export(path)
            input_hashes.add(sha)
            languages.add(language)
            row["sha256"] = sha
            row["functions"] = exported
            rows.append(row)
            functions.extend(exported)
            ranges.extend({"path": str(path), **entry}
                          for entry in exported_ranges)
    except ValueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    if len(input_hashes) != 1 or len(languages) != 1:
        print("error: exports do not share one captured input and language", file=sys.stderr)
        return 2
    incomplete = [row for row in functions if not row["complete"]]
    if incomplete:
        print("error: incomplete Ghidra function export", file=sys.stderr)
        return 2
    requested = {str(row["requested"]) for row in functions}
    missing = [address.lower().removeprefix("0x") for address in args.require
               if address.lower().removeprefix("0x") not in requested]
    if missing:
        print("error: required function export missing: " + ", ".join(missing),
              file=sys.stderr)
        return 2
    receipt = {
        "kind": "nexus-sh2-decompilation-provenance-v1",
        "inputSha256": next(iter(input_hashes)),
        "language": next(iter(languages)),
        "exports": rows,
        "functionCount": len(functions),
        "disassemblyRanges": ranges,
        "interpretation": "provenance-only; no renderer or start-route admission",
    }
    print(json.dumps(receipt, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
