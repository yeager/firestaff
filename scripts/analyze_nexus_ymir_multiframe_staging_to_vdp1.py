#!/usr/bin/env python3
"""Join authentic Ymir SH-2 staging writes to frame-local VDP1 commands.

This analysis consumes real Ymir capture dumps. It does not synthesize game
data, renderer state, or expected command geometry.
"""
from __future__ import annotations

import argparse
import csv
import json
from collections import Counter, defaultdict
from pathlib import Path


WRITE_PCS = {"0x608f366": 0, "0x608f36a": 1, "0x608f36e": 2, "0x608f372": 3}


def read_receipt(path: Path, expected_format: str) -> dict[str, str]:
    fields = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            fields[key] = value
    if fields.get("format") != expected_format:
        raise ValueError(f"{path}: unexpected receipt format {fields.get('format')!r}")
    if fields.get("trace_file_complete") != "1":
        raise ValueError(f"{path}: trace is incomplete")
    return fields


def corners_from_write_slots(slots: dict[int, int]) -> tuple[tuple[int, int], ...]:
    return tuple(((slots[index] >> 16) & 0xFFFF, slots[index] & 0xFFFF)
                 for index in range(4))


def command_corners(row: dict[str, str]) -> tuple[tuple[int, int], ...]:
    words = tuple(int(row[f"word{index}"], 16) for index in range(6, 14))
    return tuple((words[index], words[index + 1]) for index in (0, 2, 4, 6))


def geometry_class(staged: tuple[tuple[int, int], ...], command: tuple[tuple[int, int], ...]) -> str | None:
    if staged == command:
        return "ordered"
    if any(staged == command[index:] + command[:index] for index in range(4)):
        return "cyclic"
    if Counter(staged) == Counter(command):
        return "unordered"
    return None


def analyze(dumps: Path, run: str) -> tuple[dict, list[dict[str, object]]]:
    write_receipt = read_receipt(dumps / "sh2-memory-write-trace-bundle.txt",
                                 "ymir-sh2-memory-write-trace-bundle-v1")
    command_receipt = read_receipt(dumps / "vdp1-capture-bundle.txt",
                                   "ymir-vdp1-capture-bundle-v1")
    if write_receipt.get("bundle_complete") != "1":
        raise ValueError(f"{run}: SH-2 write bundle is incomplete")
    if int(write_receipt.get("master_dropped", "-1")) or int(write_receipt.get("slave_dropped", "-1")):
        raise ValueError(f"{run}: SH-2 write rows were dropped")
    if int(command_receipt.get("trace_dropped_total", "-1")):
        raise ValueError(f"{run}: VDP1 command rows were dropped")

    with (dumps / "sh2-memory-write-trace.csv").open(newline="", encoding="utf-8") as stream:
        write_rows = list(csv.DictReader(stream))
    if len(write_rows) != int(write_receipt["trace_rows"]):
        raise ValueError(f"{run}: SH-2 trace row count does not match its receipt")
    with (dumps / "vdp1-command-trace.csv").open(newline="", encoding="utf-8") as stream:
        command_rows = list(csv.DictReader(stream))
    if len(command_rows) != int(command_receipt["trace_rows"]):
        raise ValueError(f"{run}: VDP1 trace row count does not match its receipt")

    frame_begin = int(write_receipt["sh2_ram_write_frame_begin"])
    frame_end = int(write_receipt["sh2_ram_write_frame_end_exclusive"])
    frame_min = int(write_receipt["sh2_ram_write_frame_min"])
    frame_max = int(write_receipt["sh2_ram_write_frame_max"])
    if frame_min < frame_begin or frame_max >= frame_end:
        raise ValueError(f"{run}: retained SH-2 writes exceed the requested frame interval")

    staged: dict[tuple[int, int], dict[int, int]] = defaultdict(dict)
    for row in write_rows:
        pc = row["pc"].lower()
        if pc not in WRITE_PCS:
            continue
        frame = int(row["vdp1_frame"])
        address = int(row["address"], 16)
        slot = WRITE_PCS[pc]
        base = address & ~0xF
        if address != base + slot * 4:
            raise ValueError(f"{run}: unexpected staging write address: {row}")
        key = (frame, base)
        if slot in staged[key]:
            raise ValueError(f"{run}: duplicate staging slot at frame {frame}, {base:#x}")
        staged[key][slot] = int(row["value"], 16)

    commands: dict[int, list[tuple[int, tuple[tuple[int, int], ...]]]] = defaultdict(list)
    for row in command_rows:
        frame = int(row["frame"])
        commands[frame].append((int(row["index"]), command_corners(row)))

    per_frame = []
    details: list[dict[str, object]] = []
    for frame in range(frame_min, frame_max + 1):
        frame_staging = [(base, slots) for (write_frame, base), slots in staged.items()
                         if write_frame == frame]
        frame_commands = commands.get(frame, [])
        if not frame_commands:
            raise ValueError(f"{run}: no VDP1 command rows for frame {frame}")
        matched_commands: dict[int, list[int]] = defaultdict(list)
        classes = Counter()
        complete = 0
        partial = 0
        for base, slots in sorted(frame_staging):
            if set(slots) != {0, 1, 2, 3}:
                partial += 1
                continue
            complete += 1
            staged_corners = corners_from_write_slots(slots)
            matches = [(command_index, geometry_class(staged_corners, corners))
                       for command_index, corners in frame_commands]
            matches = [(command_index, kind) for command_index, kind in matches if kind]
            if len(matches) != 1:
                raise ValueError(f"{run}: frame {frame} staging record {base:#x} has {len(matches)} geometry matches")
            command_index, kind = matches[0]
            matched_commands[command_index].append(base)
            classes[kind] += 1
            details.append({"run": run, "frame": frame, "staging_address": f"0x{base:08x}",
                            "command_index": command_index, "geometry_match": kind,
                            "staged_xy": " ".join(f"{x:04x} {y:04x}" for x, y in staged_corners)})
        duplicate_command_owners = {index: bases for index, bases in matched_commands.items()
                                    if len(bases) != 1}
        if duplicate_command_owners:
            raise ValueError(f"{run}: frame {frame} has command geometries owned by multiple staging records")
        if complete == 0:
            raise ValueError(f"{run}: no complete staging records for frame {frame}")
        per_frame.append({"frame": frame, "staging_records": len(frame_staging),
                          "complete_staging_records": complete, "partial_staging_records": partial,
                          "commands": len(frame_commands), "unique_geometry_joins": sum(classes.values()),
                          "ordered": classes["ordered"], "cyclic": classes["cyclic"],
                          "unordered_only": classes["unordered"]})

    result = {"run": run, "write_frame_range": [frame_begin, frame_end],
              "retained_frames": [frame_min, frame_max], "write_trace_rows": len(write_rows),
              "command_trace_rows": len(command_rows), "per_frame": per_frame,
              "limits": ["This verifies SH-2 coordinate staging to VDP1 command geometry only.",
                         "It does not establish retail face ownership, material/pixel ownership, complete frame parity, or renderer admission.",
                         "Cyclic and unordered-only matches do not prove corner correspondence or winding."]}
    return result, details


def compare_runs(first: list[dict[str, object]], second: list[dict[str, object]]) -> dict[str, int]:
    first_xy = {(int(row["frame"]), str(row["staging_address"])): str(row["staged_xy"])
                for row in first}
    second_xy = {(int(row["frame"]), str(row["staging_address"])): str(row["staged_xy"])
                 for row in second}
    shared = first_xy.keys() & second_xy.keys()
    same_xy = sum(first_xy[key] == second_xy[key] for key in shared)
    return {"shared_staging_records": len(shared), "same_xy_records": same_xy,
            "different_xy_records": len(shared) - same_xy,
            "only_first_records": len(first_xy.keys() - second_xy.keys()),
            "only_second_records": len(second_xy.keys() - first_xy.keys())}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dumps", type=Path, required=True,
                        help="one authentic Ymir run's dumps directory")
    parser.add_argument("--run", required=True, help="label for this capture, such as control or up")
    parser.add_argument("--paired-dumps", type=Path,
                        help="optional second authentic run for staging-coordinate comparison")
    parser.add_argument("--paired-run", help="label for --paired-dumps")
    parser.add_argument("--json-out", type=Path, required=True)
    parser.add_argument("--csv-out", type=Path, required=True)
    args = parser.parse_args()
    if bool(args.paired_dumps) != bool(args.paired_run):
        parser.error("--paired-dumps and --paired-run must be provided together")
    summary, rows = analyze(args.dumps, args.run)
    summaries = [summary]
    if args.paired_dumps:
        first_rows = rows
        paired_summary, paired_rows = analyze(args.paired_dumps, args.paired_run)
        summaries.append(paired_summary)
        comparison = compare_runs(first_rows, paired_rows)
        rows = first_rows + paired_rows
    else:
        comparison = None
    result = {"runs": summaries}
    if comparison is not None:
        result["paired_comparison"] = comparison
    args.json_out.parent.mkdir(parents=True, exist_ok=True)
    args.csv_out.parent.mkdir(parents=True, exist_ok=True)
    args.json_out.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    with args.csv_out.open("w", newline="", encoding="utf-8") as stream:
        fields = ["run", "frame", "staging_address", "command_index", "geometry_match", "staged_xy"]
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    print(json.dumps({"runs": [{"run": item["run"], "frames": len(item["per_frame"]),
                               "total_staging_records": sum(row["complete_staging_records"]
                                                             for row in item["per_frame"]),
                               "total_unique_geometry_joins": sum(row["unique_geometry_joins"]
                                                                   for row in item["per_frame"]),
                               "ordered": sum(row["ordered"] for row in item["per_frame"]),
                               "cyclic": sum(row["cyclic"] for row in item["per_frame"]),
                               "unordered_only": sum(row["unordered_only"]
                                                     for row in item["per_frame"])}
                       for item in summaries], "paired_comparison": comparison}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
