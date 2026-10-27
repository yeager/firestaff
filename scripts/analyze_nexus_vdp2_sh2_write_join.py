#!/usr/bin/env python3
"""Verify ordered SH-2 byte writes against a captured VDP2 VRAM trace.

This read-only analyzer proves a byte-level producer join for the selected
frame and bounded bus window. It does not identify the retail asset semantics
or admit the captured image to Firestaff's renderer.
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def read_receipt(path: Path) -> dict[str, str]:
    fields: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            fields[key] = value
    return fields


def prefix_table(pattern: list[tuple[int, int, int]]) -> list[int]:
    table = [0] * len(pattern)
    matched = 0
    for index in range(1, len(pattern)):
        while matched and pattern[index] != pattern[matched]:
            matched = table[matched - 1]
        if pattern[index] == pattern[matched]:
            matched += 1
        table[index] = matched
    return table


def find_contiguous(
    pattern: list[tuple[int, int, int]],
    rows: list[tuple[int, int, int, int, int, int]],
) -> list[int]:
    table = prefix_table(pattern)
    matches: list[int] = []
    matched = 0
    for index, row in enumerate(rows):
        item = row[:3]
        while matched and item != pattern[matched]:
            matched = table[matched - 1]
        if item == pattern[matched]:
            matched += 1
        if matched == len(pattern):
            matches.append(index - len(pattern) + 1)
            matched = table[matched - 1]
    return matches


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="Ymir raw capture bundle directory")
    parser.add_argument("--frame", type=int, help="VDP1 frame; defaults to the last traced frame")
    parser.add_argument(
        "--vram-end-exclusive",
        type=lambda value: int(value, 0),
        default=0x20000,
        help="maximum VDP2 VRAM byte address included (default: 0x20000)",
    )
    args = parser.parse_args()

    dumps = args.bundle / "dumps"
    vdp2_receipt = read_receipt(dumps / "vdp2-memory-write-bundle.txt")
    sh2_receipt = read_receipt(dumps / "sh2-memory-write-trace-bundle.txt")
    if vdp2_receipt.get("bundle_complete") != "1" or sh2_receipt.get("bundle_complete") != "1":
        parser.error("both VDP2 and SH-2 trace receipts must report bundle_complete=1")

    base = int(sh2_receipt["address_begin"], 0)
    end = int(sh2_receipt["address_end_exclusive"], 0)
    vdp2_rows: list[tuple[int, int, int, int]] = []
    with (dumps / "vdp2-memory-write-trace.csv").open(newline="", encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            address = int(row["address"], 0)
            if row["memory"] == "VRAM" and address < args.vram_end_exclusive:
                vdp2_rows.append((int(row["vdp1_frame"]), address, int(row["width"]), int(row["value"], 0)))
    if not vdp2_rows:
        parser.error("no VDP2 VRAM writes found in the selected address window")

    frame = args.frame if args.frame is not None else max(row[0] for row in vdp2_rows)
    pattern_rows = [row[1:] for row in vdp2_rows if row[0] == frame]
    if not pattern_rows:
        parser.error(f"no VDP2 VRAM writes found for frame {frame}")
    if any(width != 1 for _address, width, _value in pattern_rows):
        parser.error("this analyzer accepts only byte-width VDP2 writes")

    sh2_rows: list[tuple[int, int, int, int, int, int]] = []
    with (dumps / "sh2-memory-write-trace.csv").open(newline="", encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            if row["sh2"] != "master":
                continue
            address = int(row["address"], 0)
            if base <= address < end:
                sh2_rows.append(
                    (
                        address - base,
                        int(row["width"]),
                        int(row["value"], 0),
                        int(row["pc"], 0),
                        int(row["caller_return_address"], 0),
                        int(row["caller_target"], 0),
                    )
                )

    matches = find_contiguous(pattern_rows, sh2_rows)
    vram = (dumps / "vdp2-vram.bin").read_bytes()
    final_values = {address: value for address, _width, value in pattern_rows}
    mismatches = [
        (address, value, vram[address])
        for address, value in final_values.items()
        if address >= len(vram) or vram[address] != value
    ]

    print(f"frame={frame}")
    print(f"vdp2_byte_writes={len(pattern_rows)}")
    print(f"vdp2_unique_addresses={len(final_values)}")
    print(f"sh2_master_rows_in_receipt_range={len(sh2_rows)}")
    print(f"contiguous_ordered_matches={len(matches)}")
    print(f"match_start_indices={','.join(map(str, matches)) or 'none'}")
    print(f"snapshot_agreeing_addresses={len(final_values) - len(mismatches)}")
    print(f"snapshot_mismatches={len(mismatches)}")
    if matches:
        start = matches[0]
        segment = sh2_rows[start : start + len(pattern_rows)]
        contexts = sorted(set(row[3:] for row in segment))
        print(f"first_match_context=pc:0x{segment[0][3]:08x},caller_return:0x{segment[0][4]:08x},caller_target:0x{segment[0][5]:08x}")
        print(f"distinct_contexts={len(contexts)}")
        print(f"first_sh2_row={start}")
        print(f"last_sh2_row={start + len(segment) - 1}")

    return 0 if len(matches) == 1 and not mismatches else 1


if __name__ == "__main__":
    raise SystemExit(main())
