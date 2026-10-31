#!/usr/bin/env python3
"""Verify the authentic BKR Continue tuple reaches Nexus startup state."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

READ_REQUIRED_FIELDS = {
    "sh2", "origin", "instruction_pc", "caller_return_address",
    "caller_target", "address", "width",
}
SNAPSHOT_REQUIRED_FIELDS = {
    "sh2", "read_row", "origin", "instruction_pc", "address", "width",
    "registers_available", "pc", "r14",
}
WRITE_REQUIRED_FIELDS = {
    "sh2", "vdp1_frame", "origin", "instruction_pc",
    "caller_return_address", "caller_target", "address", "width",
    "value", "caller_stack_targets",
}


def read_bundle(path: Path) -> dict[str, str]:
    result: dict[str, str] = {}
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
        require(reader.fieldnames is not None and
                required_fields <= set(reader.fieldnames),
                f"{label} is missing required columns")
        rows = list(reader)
    for index, row in enumerate(rows, start=1):
        require(all(row.get(field) for field in required_fields),
                f"{label} row {index} is missing required data")
    return rows


def verify_consumer_reads(root: Path) -> None:
    dumps = root / "dumps"
    reads = read_trace(dumps / "sh2-memory-read-trace.csv",
                       READ_REQUIRED_FIELDS, "consumer read trace")
    with (dumps / "sh2-memory-read-register-snapshots.csv").open(
            newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        require(reader.fieldnames is not None and
                SNAPSHOT_REQUIRED_FIELDS <= set(reader.fieldnames),
                "register snapshot file is missing required columns")
        snapshots = list(reader)
    for index, snapshot in enumerate(snapshots, start=1):
        require(all(snapshot.get(field) for field in SNAPSHOT_REQUIRED_FIELDS),
                f"register snapshot row {index} is incomplete")
    snapshot_ids = [int(row["read_row"]) for row in snapshots]
    require(sorted(snapshot_ids) == list(range(len(snapshots))),
            "register snapshot row IDs are incomplete or duplicated")
    bundle = read_bundle(dumps / "sh2-memory-read-trace-bundle.txt")
    require(bundle.get("bundle_complete") == "1" and
            bundle.get("trace_file_complete") == "1" and
            bundle.get("trace_rows") == str(len(reads)) == "477" and
            bundle.get("register_snapshot_file_complete") == "1" and
            bundle.get("register_snapshot_rows") == str(len(snapshots)) == "477" and
            bundle.get("master_dropped") == "0" and
            bundle.get("slave_dropped") == "0",
            "consumer read trace/snapshot completeness receipt changed")

    consumer_wram = (dumps / "wram-hi.bin").read_bytes()
    expected = {
        0x0602103C: (0x64E1, 0x06064544),
        0x06021068: (0x64E1, 0x06064544),
        0x06021078: (0x85E2, 0x06064548),
        0x0602107E: (0x85E3, 0x0606454A),
        0x06021092: (0x85E1, 0x06064546),
    }
    require(len(reads) == len(snapshots),
            "read trace and independent register-snapshot counts differ")
    for pc, (opcode, address) in expected.items():
        matching_reads = [row for row in reads
                          if int(row["instruction_pc"], 0) == pc and
                          int(row["address"], 0) == address]
        require(matching_reads and all(
                    row["sh2"] == "master" and row["origin"] == "cpu" and
                    row["caller_target"] == "0x6020fee" and
                    row["caller_return_address"] == "0x6011196" and
                    row["width"] == "2"
                    for row in matching_reads),
                f"consumer read trace changed at {pc:#x}")
        matching_snapshots = [row for row in snapshots
                              if int(row["pc"], 0) == pc]
        require(any(
                    row["sh2"] == "master" and
                    row["registers_available"] == "1" and
                    int(row["r14"], 0) == 0x06064544
                    for row in matching_snapshots),
                f"independent R14 snapshots changed at {pc:#x}")
        offset = pc - 0x06000000
        require(int.from_bytes(consumer_wram[offset:offset + 2], "big") == opcode,
                f"consumer SH-2 opcode changed at {pc:#x}")
    require({int(row["instruction_pc"], 0) for row in reads
             if int(row["instruction_pc"], 0) in expected} == set(expected),
            "one or more expected tuple field reads are missing")
    prologue_offset = 0x06020FF2 - 0x06000000
    require(int.from_bytes(consumer_wram[prologue_offset:prologue_offset + 2], "big")
            == 0x6E43, "FUN_06020fee no longer sets R14 from R4")


def verify(root: Path, consumer_root: Path) -> None:
    verify_consumer_reads(consumer_root)
    dumps = root / "dumps"
    rows = read_trace(dumps / "sh2-memory-write-trace.csv",
                      WRITE_REQUIRED_FIELDS, "initializer write trace")
    bundle = read_bundle(dumps / "sh2-memory-write-trace-bundle.txt")
    require(bundle.get("bundle_complete") == "1" and
            bundle.get("trace_file_complete") == "1",
            "write trace bundle/file is incomplete")
    require(bundle.get("trace_rows") == str(len(rows)) == "5244",
            "trace row count changed")
    require(bundle.get("address_begin") == "0x606455c" and
            bundle.get("address_end_exclusive") == "0x6064580",
            "trace address range changed")
    require(bundle.get("master_dropped") == "0" and
            bundle.get("slave_dropped") == "0", "write rows were dropped")
    require(bundle.get("sh2_ram_write_frame_range_enabled") == "1" and
            bundle.get("sh2_ram_write_frame_min") == "12000" and
            bundle.get("sh2_ram_write_frame_max") == "14636",
            "frame coverage receipt changed")

    expected = {
        (0x0601F5A0, 0x06064568, 4, 0x060746C0),
        (0x0601F5A6, 0x0606455C, 4, 0x060792B0),
        (0x0601F5A8, 0x06064560, 4, 0x060792B0),
        (0x0601F5B4, 0x06064564, 4, 0x060746E8),
        (0x0601F5D6, 0x0606456C, 4, 0xFFFFFFFF),
    }
    observed = {
        (int(row["instruction_pc"], 0), int(row["address"], 0),
         int(row["width"]), int(row["value"], 0))
        for row in rows
        if row["vdp1_frame"] == "12931" and row["caller_target"] == "0x601f584"
        and "0x6020fee" in row["caller_stack_targets"].split("|")
    }
    require(expected <= observed,
            "Continue initializer pointer/allocation writes at frame 12931 changed")

    wram = (dumps / "wram-hi.bin").read_bytes()

    def read(address: int, size: int) -> bytes:
        offset = address - 0x06000000
        require(0 <= offset <= len(wram) - size, "address outside WRAM-HI dump")
        return wram[offset:offset + size]

    require(read(0x06064544, 8) == bytes.fromhex("00 01 40 00 00 14 00 1a"),
            "final loader tuple differs from the authentic BKR payload")
    require(read(0x06064564, 4) == (0x060746E8).to_bytes(4, "big") and
            read(0x06064568, 4) == (0x060746C0).to_bytes(4, "big"),
            "final initializer pointers do not match the captured stores")
    require(read(0x060746E8, 4) == bytes.fromhex("00 14 00 1a"),
            "runtime body cell does not contain the BKR X/Y coordinates")
    require(read(0x0606455C, 4) == (0x060792B0).to_bytes(4, "big"),
            "initializer actor record base pointer changed")
    require(read(0x060792BE, 2) == bytes.fromhex("40 00"),
            "runtime actor heading differs from the BKR heading")

    print("PASS: complete 5,244-row SH-2 write trace; zero dropped rows")
    print("PASS: independent snapshots show R14 at the tuple base for each consumer instruction PC")
    print("PASS: FUN_0601f584, called from FUN_06020fee, registers actor/body pointers")
    print("PASS: authentic BKR X/Y body cell and heading are present in final runtime state")
    print("LIMIT: snapshots have no shared per-read ID and are not joined to exact read events")
    print("LIMIT: this validates authentic Continue initialization; keyboard/mouse parity is separate")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("evidence_dir", type=Path)
    parser.add_argument("consumer_evidence_dir", type=Path)
    args = parser.parse_args()
    try:
        verify(args.evidence_dir, args.consumer_evidence_dir)
    except (OSError, ValueError, KeyError) as exc:
        parser.error(str(exc))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
