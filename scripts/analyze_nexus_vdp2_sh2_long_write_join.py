#!/usr/bin/env python3
"""Join SH-2 longword stores to the ordered VDP2 halfword write trace.

This read-only analyzer verifies the bus-width conversion for a selected
authentic Ymir capture. A match establishes writer attribution and byte order;
it does not assign tile, layer, or UI semantics to the written values.
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def read_fields(path: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            result[key] = value
    return result


def csv_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def require_rows(rows: list[dict[str, str]], fields: dict[str, str], label: str) -> None:
    if fields.get("bundle_complete") != "1" or fields.get("trace_file_complete") != "1":
        raise ValueError(f"{label} trace receipt is not complete")
    if len(rows) != int(fields["trace_rows"]):
        raise ValueError(f"{label} trace row count does not match its receipt")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="Ymir trace-bundle directory")
    parser.add_argument("--frame", type=int, required=True, help="VDP1 frame to compare")
    parser.add_argument(
        "--pc", type=lambda value: int(value, 0),
        help="restrict SH-2 rows to one writer PC (for example 0x06023640)",
    )
    parser.add_argument(
        "--cpu", choices=("master", "slave", "both"), default="master",
        help="select a SH-2 trace source (default: master)",
    )
    parser.add_argument(
        "--bus-vram-end", type=lambda value: int(value, 0), default=0x05E80000,
        help="exclusive SH-2 bus end for VDP2 VRAM (default: 0x05e80000)",
    )
    args = parser.parse_args()

    try:
        vdp2_receipt = read_fields(args.bundle / "vdp2-memory-write-bundle.txt")
        sh2_receipt = read_fields(args.bundle / "sh2-vdp2-write-trace-bundle.txt")
        vdp2_rows = csv_rows(args.bundle / "vdp2-memory-write-trace.csv")
        sh2_rows = csv_rows(args.bundle / "sh2-vdp2-write-trace.csv")
        require_rows(vdp2_rows, vdp2_receipt, "VDP2")
        require_rows(sh2_rows, sh2_receipt, "SH-2")
        if int(vdp2_receipt.get("trace_dropped_total", "-1")) != 0:
            raise ValueError("VDP2 trace reports dropped rows")
        if int(sh2_receipt.get("master_dropped", "-1")) != 0 or int(
            sh2_receipt.get("slave_dropped", "-1")
        ) != 0:
            raise ValueError("SH-2 trace reports dropped rows")
        sh2_base = int(sh2_receipt["address_begin"], 0)
        vdp2_range_end = min(
            int(sh2_receipt["address_end_exclusive"], 0), args.bus_vram_end
        )
        target_vdp2 = [
            (int(row["address"], 0), int(row["width"]), int(row["value"], 0))
            for row in vdp2_rows
            if int(row["vdp1_frame"]) == args.frame and row["memory"] == "VRAM"
        ]
        expanded: list[tuple[int, int, int, int]] = []
        source_rows = 0
        for row in sh2_rows:
            if int(row["vdp1_frame"]) != args.frame:
                continue
            if args.cpu != "both" and row.get("sh2") != args.cpu:
                continue
            pc = int(row["pc"], 0)
            if args.pc is not None and pc != args.pc:
                continue
            address = int(row["address"], 0)
            width = int(row["width"])
            value = int(row["value"], 0)
            if not sh2_base <= address < vdp2_range_end:
                continue
            if width == 2:
                expanded.append((address - sh2_base, width, value & 0xFFFF, pc))
            elif width == 4:
                expanded.append((address - sh2_base, 2, (value >> 16) & 0xFFFF, pc))
                expanded.append((address - sh2_base + 2, 2, value & 0xFFFF, pc))
            else:
                raise ValueError(f"unsupported SH-2 write width {width} at {address:#x}")
            source_rows += 1
    except (OSError, KeyError, ValueError) as error:
        parser.error(str(error))

    if not target_vdp2 or not expanded:
        parser.error("selected frame or writer has no VDP2 VRAM writes")
    if any(width != 2 for _address, width, _value in target_vdp2):
        parser.error("selected VDP2 trace contains a non-halfword VRAM event")

    expected = [(address, value) for address, _width, value, _pc in expanded]
    observed = [(address, value) for address, _width, value in target_vdp2]
    exact_ordered_match = expected == observed
    matched = 0
    cursor = 0
    for event in expected:
        while cursor < len(observed) and observed[cursor] != event:
            cursor += 1
        if cursor == len(observed):
            break
        matched += 1
        cursor += 1
    ordered_subsequence = matched == len(expected)

    print(f"frame={args.frame}")
    print(f"source_cpu={args.cpu}")
    print(f"source_pc={f'{args.pc:#010x}' if args.pc is not None else 'all'}")
    print(f"sh2_source_rows={source_rows}")
    print(f"sh2_expanded_halfword_events={len(expected)}")
    print(f"vdp2_vram_halfword_events={len(observed)}")
    print(f"ordered_subsequence_matches={matched}/{len(expected)}")
    print(f"exact_ordered_trace_match={int(exact_ordered_match)}")
    print("interpretation=writer_and_bus_value_join_only")
    return 0 if ordered_subsequence else 1


if __name__ == "__main__":
    raise SystemExit(main())
