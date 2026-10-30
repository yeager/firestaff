#!/usr/bin/env python3
"""Verify the authentic BKR Continue tuple reaches Nexus startup state."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


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


def verify_consumer_reads(root: Path) -> None:
    dumps = root / "dumps"
    with (dumps / "sh2-memory-read-trace.csv").open(
            newline="", encoding="utf-8") as stream:
        reads = list(csv.DictReader(stream))
    with (dumps / "sh2-memory-read-register-snapshots.csv").open(
            newline="", encoding="utf-8") as stream:
        snapshots = list(csv.DictReader(stream))
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
    require(len(reads) == len(snapshots), "read/snapshot row counts differ")
    for index, (read, regs) in enumerate(zip(reads, snapshots)):
        require(int(regs["read_row"]) == index,
                "register snapshot row IDs no longer match capture order")
        pc = int(read["instruction_pc"], 0)
        if pc not in expected:
            continue
        opcode, address = expected[pc]
        require(read["sh2"] == "master" and read["origin"] == "cpu" and
                read["caller_target"] == "0x6020fee" and
                read["caller_return_address"] == "0x6011196" and
                read["width"] == "2" and
                regs["registers_available"] == "1" and
                regs["pc"] == read["instruction_pc"] and
                int(read["address"], 0) == address and
                int(regs["r14"], 0) == 0x06064544,
                f"consumer read/register join changed at {pc:#x}")
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
    rows = list(csv.DictReader(
        (dumps / "sh2-memory-write-trace.csv").open(newline="", encoding="utf-8")))
    bundle = read_bundle(dumps / "sh2-memory-write-trace-bundle.txt")
    require(bundle.get("bundle_complete") == "1", "write trace bundle is incomplete")
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
    print("PASS: consumer snapshots/opcodes join R14 base to each tuple field address")
    print("PASS: FUN_0601f584, called from FUN_06020fee, registers actor/body pointers")
    print("PASS: authentic BKR X/Y body cell and heading are present in final runtime state")
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
