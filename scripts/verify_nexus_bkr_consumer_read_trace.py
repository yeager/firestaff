#!/usr/bin/env python3
"""Check completeness and expected call-context rows in an authentic BKR trace."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


READ_REQUIRED_FIELDS = {
    "sh2", "origin", "instruction_pc", "caller_return_address",
    "caller_target", "address", "width",
}
WRITE_REQUIRED_FIELDS = {
    "sh2", "vdp1_frame", "origin", "instruction_pc", "address",
    "width", "value",
}


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


def read_trace(path: Path, required_fields: set[str], label: str) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        require(reader.fieldnames is not None and required_fields <= set(reader.fieldnames),
                f"{label} is missing required columns")
        rows = list(reader)
    for index, row in enumerate(rows, start=1):
        require(all(row.get(field) for field in required_fields),
                f"{label} row {index} is missing required data")
    return rows


def verify(root: Path) -> None:
    dumps = root / "dumps"
    reads = read_trace(dumps / "sh2-memory-read-trace.csv",
                       READ_REQUIRED_FIELDS, "read trace")
    writes = read_trace(dumps / "sh2-memory-write-trace.csv",
                        WRITE_REQUIRED_FIELDS, "write trace")
    with (dumps / "sh2-memory-read-register-snapshots.csv").open(
            newline="", encoding="utf-8") as stream:
        snapshots = list(csv.DictReader(stream))
    read_receipt = read_bundle(dumps / "sh2-memory-read-trace-bundle.txt")
    write_receipt = read_bundle(dumps / "sh2-memory-write-trace-bundle.txt")

    require(read_receipt.get("bundle_complete") == "1", "read trace is incomplete")
    require(read_receipt.get("trace_file_complete") == "1",
            "read trace file is incomplete")
    require(read_receipt.get("trace_rows") == str(len(reads)) == "477",
            "read row count changed")
    require(read_receipt.get("master_dropped") == "0" and
            read_receipt.get("slave_dropped") == "0", "read rows were dropped")
    require(read_receipt.get("register_snapshot_file_complete") == "1" and
            read_receipt.get("register_snapshot_rows") == str(len(snapshots)) == "477",
            "register snapshot count changed")
    require(write_receipt.get("bundle_complete") == "1" and
            write_receipt.get("trace_file_complete") == "1" and
            write_receipt.get("trace_rows") == str(len(writes)) == "42" and
            write_receipt.get("master_dropped") == "0" and
            write_receipt.get("slave_dropped") == "0", "write trace receipt changed")

    targets = {"0x602103c", "0x6021068", "0x6021078", "0x602107e", "0x6021092"}
    selected = [row for row in reads if row["instruction_pc"] in targets]
    require({row["instruction_pc"] for row in selected} == targets,
            "expected loader-initializer call-context PCs are missing")
    require(all(row["caller_target"] == "0x6020fee" and
                row["caller_return_address"] == "0x6011196"
                for row in selected), "call-context metadata changed")
    tuple_fields = {"0x6064544", "0x6064546", "0x6064548", "0x606454a"}
    require(tuple_fields <= {row["address"] for row in selected},
            "expected tuple field addresses are absent")

    print("PASS: 477 complete SH-2 read rows; 477 register snapshots recorded separately; zero dropped rows")
    print("PASS: initializer call-context includes observed reads at tuple offsets +0, +2, +4, +6")
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
