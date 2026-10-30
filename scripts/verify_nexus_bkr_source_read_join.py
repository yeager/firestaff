#!/usr/bin/env python3
"""Verify the authentic Nexus BKR loader source-read to WRAM write join."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
import sys


SOURCE_READS = (
    (0x06022548, 0x00204010),
    (0x0602254C, 0x00204014),
)
DESTINATION_WRITES = (
    (0x0602254A, 0x06064544, 0x00014000),
    (0x0602254E, 0x06064548, 0x0014001A),
)
EXPECTED_SOURCE_OPCODES = (0x52E4, 0x2022, 0x52E5, 0x1021)
EXPECTED_LOADER_BYTES = bytes.fromhex("00 01 40 00 00 14 00 1a")


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def parse_hex(value: str) -> int:
    return int(value, 0)


def read_bundle(path: Path) -> dict[str, str]:
    values: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            values[key] = value
    return values


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def verify(evidence_dir: Path) -> None:
    dumps = evidence_dir / "dumps"
    reads = read_csv(dumps / "sh2-memory-read-trace.csv")
    snapshots = read_csv(dumps / "sh2-memory-read-register-snapshots.csv")
    writes = read_csv(dumps / "sh2-memory-write-trace.csv")
    read_bundle_data = read_bundle(dumps / "sh2-memory-read-trace-bundle.txt")
    write_bundle_data = read_bundle(dumps / "sh2-memory-write-trace-bundle.txt")

    require(read_bundle_data.get("bundle_complete") == "1",
            "source read trace bundle is incomplete")
    require(read_bundle_data.get("trace_rows") == "10" and
            read_bundle_data.get("master_dropped") == "0" and
            read_bundle_data.get("slave_dropped") == "0",
            "source read trace row count or dropped-row receipt changed")
    require(len(reads) == int(read_bundle_data["trace_rows"]),
            "source read CSV row count differs from its receipt")
    require(read_bundle_data.get("register_snapshot_file_complete") == "1" and
            read_bundle_data.get("register_snapshot_rows") == "10",
            "aligned register snapshots are incomplete")
    require(len(snapshots) == int(read_bundle_data["register_snapshot_rows"]),
            "register snapshot CSV row count differs from its receipt")
    require(write_bundle_data.get("bundle_complete") == "1" and
            write_bundle_data.get("master_dropped") == "0" and
            write_bundle_data.get("slave_dropped") == "0",
            "destination write trace bundle is incomplete")
    require(len(writes) == int(write_bundle_data["trace_rows"]),
            "destination write CSV row count differs from its receipt")

    source_rows = [
        (index, (parse_hex(row["instruction_pc"]),
                 parse_hex(row["address"])), row)
        for index, row in enumerate(reads)
        if (parse_hex(row["instruction_pc"]), parse_hex(row["address"]))
        in SOURCE_READS
    ]
    require(tuple(key for _, key, _ in source_rows) == SOURCE_READS,
            "the two payload reads are missing, duplicated, or out of order")
    for _, (pc, _), row in source_rows:
        require(parse_hex(row["width"]) == 4 and row["sh2"] == "master",
                f"unexpected SH-2 source read at {pc:#x}")

    snapshot_rows = {int(row["read_row"]): row for row in snapshots}
    require(len(snapshot_rows) == len(snapshots) and
            set(snapshot_rows) == set(range(len(reads))),
            "register snapshots are duplicated or not aligned with read rows")
    for index, row in enumerate(reads):
        snapshot = snapshot_rows[index]
        require(parse_hex(snapshot["instruction_pc"]) ==
                parse_hex(row["instruction_pc"]) and
                parse_hex(snapshot["address"]) == parse_hex(row["address"]),
                f"register snapshot {index} does not match its read row")
    first_index, _, _ = source_rows[0]
    second_index, _, _ = source_rows[1]
    first_snapshot = snapshot_rows[first_index]
    second_snapshot = snapshot_rows[second_index]
    require(parse_hex(first_snapshot["r14"]) == 0x00204000 and
            parse_hex(first_snapshot["r0"]) == 0x06064544,
            "first load base or destination register does not match")
    require(parse_hex(second_snapshot["r14"]) == 0x00204000 and
            parse_hex(second_snapshot["r0"]) == 0x06064544 and
            parse_hex(second_snapshot["r2"]) == 0x00014000,
            "second load does not follow the first tuple word")

    wram = (dumps / "wram-hi.bin").read_bytes()
    code = tuple(int.from_bytes(wram[offset:offset + 2], "big")
                 for offset in range(0x22548, 0x22550, 2))
    require(code == EXPECTED_SOURCE_OPCODES,
            f"runtime SH-2 instruction sequence changed: {code!r}")
    require(code[0] == 0x52E4 and code[2] == 0x52E5,
            "runtime code no longer loads longwords at payload +0x10/+0x14")
    require(code[1] == 0x2022 and code[3] == 0x1021,
            "runtime code no longer stores the loaded longwords at +0/+4")

    observed_writes = [
        (parse_hex(row["instruction_pc"]), parse_hex(row["address"]),
         parse_hex(row["value"]))
        for row in writes
        if row["vdp1_frame"] == "12931" and
        parse_hex(row["address"]) in {write[1] for write in DESTINATION_WRITES}
    ]
    require(tuple(observed_writes) == DESTINATION_WRITES,
            "the paired tuple stores are missing, duplicated, or out of order at frame 12931")
    require(wram[0x64544:0x6454C] == EXPECTED_LOADER_BYTES,
            "final WRAM-HI loader tuple differs from the authentic payload")

    print("PASS: captured SH-2 reads payload +0x10/+0x14")
    print("PASS: runtime opcodes load both longwords and store them at WRAM-HI +0/+4")
    print("PASS: dynamic stores and final WRAM-HI bytes equal 00 01 40 00 00 14 00 1a")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("evidence_dir", type=Path)
    args = parser.parse_args()
    try:
        verify(args.evidence_dir)
    except (OSError, KeyError, ValueError, IndexError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
