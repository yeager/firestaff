#!/usr/bin/env python3
"""Check completeness and expected call-context rows in an authentic BKR trace."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def read_bundle(path: Path) -> dict[str, str]:
    result = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            result[key] = value
    return result


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def verify(root: Path) -> None:
    dumps = root / "dumps"
    reads = list(csv.DictReader((dumps / "sh2-memory-read-trace.csv").open(newline="")))
    snapshots = list(csv.DictReader(
        (dumps / "sh2-memory-read-register-snapshots.csv").open(newline="")))
    read_receipt = read_bundle(dumps / "sh2-memory-read-trace-bundle.txt")
    write_receipt = read_bundle(dumps / "sh2-memory-write-trace-bundle.txt")

    require(read_receipt.get("bundle_complete") == "1", "read trace is incomplete")
    require(read_receipt.get("trace_rows") == str(len(reads)) == "477",
            "read row count changed")
    require(read_receipt.get("master_dropped") == "0" and
            read_receipt.get("slave_dropped") == "0", "read rows were dropped")
    require(read_receipt.get("register_snapshot_file_complete") == "1" and
            read_receipt.get("register_snapshot_rows") == str(len(snapshots)) == "477",
            "aligned register snapshots are incomplete")
    require(write_receipt.get("bundle_complete") == "1" and
            write_receipt.get("trace_rows") == "42" and
            write_receipt.get("master_dropped") == "0" and
            write_receipt.get("slave_dropped") == "0", "write trace receipt changed")

    targets = {"0x602103c", "0x6021068", "0x6021078", "0x602107e", "0x6021092"}
    joined = [(row, snapshots[index]) for index, row in enumerate(reads)
              if row["instruction_pc"] in targets]
    require({row["instruction_pc"] for row, _ in joined} == targets,
            "expected loader-initializer call-context PCs are missing")
    require(all(row["caller_target"] == "0x6020fee" and
                row["caller_return_address"] == "0x6011196"
                for row, _ in joined), "call-context metadata changed")
    require(any(row["address"] == "0x6064544" for row, _ in joined) and
            any(row["address"] == "0x6064546" for row, _ in joined) and
            any(row["address"] == "0x6064548" for row, _ in joined),
            "expected tuple field addresses are absent")

    print("PASS: 477 complete SH-2 read rows and aligned snapshots; zero dropped rows")
    print("PASS: initializer call-context includes observed reads at tuple offsets +0, +2, +4")
    print("LIMIT: trace context alone does not prove effective base pointers or loaded values")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("evidence_dir", type=Path)
    args = parser.parse_args()
    try:
        verify(args.evidence_dir)
    except (OSError, ValueError, KeyError) as exc:
        parser.error(str(exc))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
