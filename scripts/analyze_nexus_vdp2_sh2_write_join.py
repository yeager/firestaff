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


def require_complete_trace(
    path: Path,
    receipt: dict[str, str],
    name: str,
    dropped_fields: tuple[str, ...],
) -> None:
    if receipt.get("bundle_complete") != "1" or receipt.get("trace_file_complete") != "1":
        raise ValueError(f"{name} receipt does not report a complete bundle and trace file")
    try:
        expected_rows = int(receipt["trace_rows"])
        dropped_rows = [int(receipt[field]) for field in dropped_fields]
    except (KeyError, ValueError) as error:
        raise ValueError(f"{name} receipt lacks valid trace row or drop counts") from error
    if expected_rows < 1 or any(count < 0 for count in dropped_rows) or sum(dropped_rows) != 0:
        raise ValueError(f"{name} receipt reports an empty or dropped trace")

    with path.open(newline="", encoding="utf-8") as stream:
        actual_rows = sum(1 for _row in csv.DictReader(stream))
    if actual_rows != expected_rows:
        raise ValueError(
            f"{name} receipt declares {expected_rows} trace rows, but CSV has {actual_rows}"
        )


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
    rows: list[tuple[int, int, int, int, int, int, str]],
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
    """Validate and report the selected VDP2-to-SH-2 trace join."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="Ymir raw capture bundle directory")
    parser.add_argument(
        "--frame",
        type=int,
        help="VDP1 frame to join (defaults to the last VDP2 trace frame)",
    )
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
    try:
        require_complete_trace(
            dumps / "vdp2-memory-write-trace.csv",
            vdp2_receipt,
            "VDP2",
            ("trace_dropped_total",),
        )
        require_complete_trace(
            dumps / "sh2-memory-write-trace.csv",
            sh2_receipt,
            "SH-2",
            ("master_dropped", "slave_dropped"),
        )
    except ValueError as error:
        parser.error(str(error))

    base = int(sh2_receipt["address_begin"], 0)
    end = int(sh2_receipt["address_end_exclusive"], 0)
    vdp2_rows: list[tuple[int, int, int, int]] = []
    vdp2_trace_frames: list[int] = []
    with (dumps / "vdp2-memory-write-trace.csv").open(newline="", encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            vdp2_trace_frames.append(int(row["vdp1_frame"]))
            address = int(row["address"], 0)
            if row["memory"] == "VRAM" and address < args.vram_end_exclusive:
                vdp2_rows.append((int(row["vdp1_frame"]), address, int(row["width"]), int(row["value"], 0)))
    if not vdp2_rows:
        parser.error("no VDP2 VRAM writes found in the selected address window")

    latest_vdp2_frame = max(vdp2_trace_frames)
    frame = args.frame if args.frame is not None else latest_vdp2_frame
    if frame != latest_vdp2_frame:
        parser.error(
            f"selected frame {frame} is not the last frame in the complete VDP2 trace "
            f"({latest_vdp2_frame})"
        )
    try:
        snapshot_frame = int(vdp2_receipt["current_vdp1_frame_index"])
        trace_frame_min = int(vdp2_receipt["first_vdp1_frame_index"])
        trace_frame_max = int(vdp2_receipt["last_vdp1_frame_index"])
        trace_frame_begin = int(vdp2_receipt["trace_vdp1_frame_begin"])
        trace_frame_end = int(vdp2_receipt["trace_vdp1_frame_end_exclusive"])
        register_snapshot = read_receipt(dumps / "vdp-register-snapshot.txt")
        register_snapshot_frame = int(register_snapshot["vdp1_frame_index"])
    except (KeyError, ValueError) as error:
        parser.error(f"VDP2 receipt has no valid frame bounds or final snapshot frame: {error}")
    if (vdp2_receipt.get("vdp2_vram_file_complete") != "1" or
            vdp2_receipt.get("vdp2_cram_file_complete") != "1" or
            vdp2_receipt.get("vdp2_register_file_complete") != "1"):
        parser.error("VDP2 receipt does not report complete VRAM, CRAM, and register snapshots")
    if snapshot_frame < frame:
        parser.error(
            f"final VDP2 snapshot frame {snapshot_frame} precedes selected trace frame {frame}"
        )
    if register_snapshot_frame != snapshot_frame:
        parser.error(
            f"VDP register snapshot frame {register_snapshot_frame} does not match "
            f"VDP2 memory snapshot frame {snapshot_frame}"
        )
    if (min(vdp2_trace_frames) != trace_frame_min or
            max(vdp2_trace_frames) != trace_frame_max):
        parser.error("VDP2 receipt frame bounds do not match the complete trace CSV")
    snapshot_frame_delta = snapshot_frame - frame
    if snapshot_frame_delta:
        if (vdp2_receipt.get("trace_vdp1_frame_range_enabled") != "1" or
                trace_frame_begin > frame or trace_frame_end <= snapshot_frame or
                trace_frame_min > frame or trace_frame_max != frame):
            parser.error(
                "a later VDP2 snapshot is admissible only when a complete frame-windowed "
                "trace covers the selected frame through the snapshot and contains no later writes"
            )
    try:
        sh2_frame_min = int(sh2_receipt["sh2_ram_write_frame_min"], 0)
        sh2_frame_max = int(sh2_receipt["sh2_ram_write_frame_max"], 0)
    except (KeyError, ValueError) as error:
        parser.error(f"SH-2 receipt has no valid frame bounds: {error}")
    if sh2_frame_min != frame or sh2_frame_max != frame:
        parser.error(
            f"SH-2 trace bounds {sh2_frame_min}..{sh2_frame_max} must equal "
            f"the selected VDP2 frame {frame}; SH-2 rows have no per-row frame tag"
        )

    pattern_rows = [row[1:] for row in vdp2_rows if row[0] == frame]
    if not pattern_rows:
        parser.error(f"no VDP2 VRAM writes found for frame {frame}")
    if any(width != 1 for _address, width, _value in pattern_rows):
        parser.error("this analyzer accepts only byte-width VDP2 writes")

    sh2_rows: list[tuple[int, int, int, int, int, int, str]] = []
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
                        row.get("caller_stack_targets", ""),
                    )
                )

    matches = find_contiguous(pattern_rows, sh2_rows)
    vram = (dumps / "vdp2-vram.bin").read_bytes()
    final_values = {address: value for address, _width, value in pattern_rows}
    mismatches = []
    for address, value in final_values.items():
        if address >= len(vram):
            mismatches.append((address, value, None))
        elif vram[address] != value:
            mismatches.append((address, value, vram[address]))

    print(f"frame={frame}")
    print(f"vdp2_snapshot_frame={snapshot_frame}")
    print(f"vdp2_snapshot_frame_delta={snapshot_frame_delta}")
    print(f"vdp2_snapshot_stable_after_trace_frame={int(snapshot_frame_delta == 0 or trace_frame_max == frame)}")
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
        call_chains = sorted(set(row[6] for row in segment))
        print(f"distinct_call_chains={len(call_chains)}")
        print(f"first_call_chain={segment[0][6] or 'unavailable'}")
        print(f"first_sh2_row={start}")
        print(f"last_sh2_row={start + len(segment) - 1}")

    return 0 if len(matches) == 1 and not mismatches else 1


if __name__ == "__main__":
    raise SystemExit(main())
