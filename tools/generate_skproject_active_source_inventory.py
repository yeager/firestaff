#!/usr/bin/env python3
"""Inventory active and legacy DOS callable definitions in skproject source.

Uses the system `ctags -x` output as a conservative discovery list. Rows are
source locations, not behavior or Firestaff parity claims.
"""
from __future__ import annotations

import argparse
import csv
import subprocess
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LEGACY_AUDIT = ROOT / "docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv"
DEFAULT_OUTPUT = ROOT / "docs/reference/audits/SKPROJECT_DM2_ACTIVE_SOURCE_INVENTORY.tsv"
SOURCE_SPECS = (
    # The unified engine variants.
    ("SKWINSPX", "SKWINSPX/src", True),
    ("SKWINDOS", "SKWINDOS/src", True),
    # The legacy DOS implementation lives in top-level SKWINDOS/*.cpp files,
    # outside the newer SKWINDOS/src tree. Keep it as a separate source row
    # set so its platform-specific runtime definitions are not omitted.
    ("SKWINDOS", "SKWINDOS", False),
)
LOCKED_SOURCE_COMMIT = "2c59a2c5a7978923ff5c822c15c3479e4d448493"

# BSD ctags omits this qualified v4 definition even though it discovers
# adjacent SkWinCore methods in the same file. Keep the explicit source row so
# the active inventory covers all three timer/actuator implementations.
SUPPLEMENTAL_SOURCE_ROWS = (
    (
        "SKWINSPX",
        "SKWINSPX/src/v4/skcore.cpp",
        "11765",
        "INVOKE_ACTUATOR",
        "void SkWinCore::INVOKE_ACTUATOR(Actuator *ref, U16 actionType, i16 delayPlus)",
    ),
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, required=True,
                        help="skproject checkout at the locked source revision")
    parser.add_argument("--ctags", default="ctags", help="ctags executable")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    source_root = args.source_root.resolve()
    revision = subprocess.run(
        ["git", "-C", str(source_root), "rev-parse", "HEAD"],
        check=True, capture_output=True, text=True).stdout.strip()
    if revision != LOCKED_SOURCE_COMMIT:
        raise SystemExit(
            f"expected skproject {LOCKED_SOURCE_COMMIT}, got {revision}")
    legacy_rows = list(csv.DictReader(
        LEGACY_AUDIT.open(newline="", encoding="utf-8"), delimiter="\t"))
    legacy_counts = Counter(row["symbol"] for row in legacy_rows)
    previous_rows = []
    if args.output.exists():
        with args.output.open(newline="", encoding="utf-8") as handle:
            previous_rows = list(csv.DictReader(handle, delimiter="\t"))
    previous_status = {
        (row["source_tree"], row["source_file"], row["line"], row["symbol"]):
            row.get("review_status", "UNREVIEWED")
        for row in previous_rows
    }
    discovered: list[dict[str, str]] = []
    for source_tree, source_path, recursive in SOURCE_SPECS:
        tree_root = source_root / source_path
        candidates = tree_root.rglob("*") if recursive else tree_root.glob("*")
        files = sorted(p for p in candidates
                       if p.is_file() and p.suffix.lower() in {".c", ".cpp"})
        if not files:
            raise SystemExit(f"no C/C++ files found under {tree_root}")
        for source_file in files:
            # Run per file. Exuberant ctags suppresses same-name tags when
            # multiple source files are passed together, which would erase
            # separate v4/v5 definitions from this source inventory.
            result = subprocess.run(
                [args.ctags, "-x", str(source_file)],
                check=True, capture_output=True, text=True)
            for line in result.stdout.splitlines():
                columns = line.split(None, 3)
                if len(columns) != 4 or not columns[1].isdigit():
                    continue
                symbol, line_number, _reported_path, signature = columns
                relative_path = source_file.relative_to(source_root).as_posix()
                key = (source_tree, relative_path, line_number, symbol)
                discovered.append({
                    "symbol": symbol,
                    "source_tree": source_tree.split("/", 1)[0],
                    "source_file": relative_path,
                    "line": line_number,
                    "signature": signature,
                    "legacy_audit_exact_symbol_rows": str(legacy_counts.get(symbol, 0)),
                    "review_status": previous_status.get(key, "UNREVIEWED"),
                })

    for source_tree, relative_path, line_number, symbol, signature \
            in SUPPLEMENTAL_SOURCE_ROWS:
        key = (source_tree, relative_path, line_number, symbol)
        discovered.append({
            "symbol": symbol,
            "source_tree": source_tree,
            "source_file": relative_path,
            "line": line_number,
            "signature": signature,
            "legacy_audit_exact_symbol_rows": str(legacy_counts.get(symbol, 0)),
            "review_status": previous_status.get(key, "UNREVIEWED"),
        })

    discovered_keys = {
        (row["source_tree"], row["source_file"], row["line"], row["symbol"])
        for row in discovered
    }
    for row in previous_rows:
        key = (row["source_tree"], row["source_file"], row["line"], row["symbol"])
        if key not in discovered_keys:
            # Preserve manual/legacy source entries that ctags or the current
            # source-tree scan does not rediscover. Some are covered by active
            # review rows and would break audit key coverage if discarded.
            discovered.append(row)
            discovered_keys.add(key)

    discovered.sort(key=lambda row: (row["source_file"], int(row["line"]),
                                     row["symbol"], row["signature"]))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(discovered[0]),
                                delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(discovered)
    print(f"Wrote {len(discovered)} source-definition rows to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
