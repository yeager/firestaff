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
SOURCE_TREES = ("SKWINSPX/src", "SKWINDOS/src")
LOCKED_SOURCE_COMMIT = "2c59a2c5a7978923ff5c822c15c3479e4d448493"


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
    discovered: list[dict[str, str]] = []
    for source_tree in SOURCE_TREES:
        tree_root = source_root / source_tree
        files = sorted(p for p in tree_root.rglob("*")
                       if p.is_file() and p.suffix.lower() in {".c", ".cpp"})
        if not files:
            raise SystemExit(f"no C/C++ files found under {tree_root}")
        result = subprocess.run(
            [args.ctags, "-x", *(str(path) for path in files)],
            check=True, capture_output=True, text=True)
        for line in result.stdout.splitlines():
            columns = line.split(None, 3)
            if len(columns) != 4 or not columns[1].isdigit():
                continue
            symbol, line_number, _reported_path, signature = columns
            # ctags reports the supplied path. Retain the repository-relative
            # path so the TSV is portable across local checkout locations.
            source_file = next(
                (path for path in files
                 if path.name == Path(_reported_path).name and
                 str(path).endswith(Path(_reported_path).as_posix())),
                None)
            if source_file is None:
                # The normal invocation emits the full argument path. This
                # fallback handles ctags implementations that emit basename.
                source_file = next((path for path in files
                                    if path.name == Path(_reported_path).name), None)
            if source_file is None:
                raise SystemExit(f"cannot resolve ctags path: {_reported_path}")
            discovered.append({
                "symbol": symbol,
                "source_tree": source_tree.split("/", 1)[0],
                "source_file": source_file.relative_to(source_root).as_posix(),
                "line": line_number,
                "signature": signature,
                "legacy_audit_exact_symbol_rows": str(legacy_counts.get(symbol, 0)),
                "review_status": "UNREVIEWED",
            })

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
