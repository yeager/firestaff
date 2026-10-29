#!/usr/bin/env python3
"""Merge source-bound Theron CD/RAM and PCE Fast VWR traces by shared order."""

from __future__ import annotations

import argparse
import heapq
import json
import re
import sys
from pathlib import Path
from typing import Iterator


EVENT_FIELD = re.compile(r"(?:^| )event_sequence=([0-9]+)(?: |$)")
TRACE_HEADERS = {
    "cd_data_port_read": ("source=mednafen-pce-fast-cd-data-port-read",),
    "cd_ram_target_write": ("source=mednafen-pce-fast-cd-ram-target-write",),
    "vram_commit": (
        "FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V3",
        "source=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window",
        "scope=cpu-port-vwr-commits-only;dma-writes-excluded",
    ),
    "vce_write": (
        "FIRESTAFF_THERON_PCE_FAST_VCE_WRITE_TRACE_V1",
        "source=mednafen-1.32.1-pce-fast-vdc-vce-write",
        "scope=color-table-data-writes-only;address-port-writes-excluded",
    ),
}


class TraceError(ValueError):
    pass


def records(path: Path, event_type: str) -> Iterator[tuple[int, str]]:
    previous = -1
    headers = TRACE_HEADERS[event_type]
    header_index = 0
    with path.open("r", encoding="ascii") as stream:
        for line_number, raw in enumerate(stream, 1):
            line = raw.rstrip("\n")
            if not line:
                continue
            if header_index < len(headers):
                if line != headers[header_index]:
                    raise TraceError(f"{path}:{line_number}: unexpected trace header")
                header_index += 1
                continue
            if not line.startswith(event_type + " "):
                raise TraceError(f"{path}:{line_number}: unexpected record type")
            matches = list(EVENT_FIELD.finditer(line))
            if len(matches) != 1:
                raise TraceError(f"{path}:{line_number}: expected one event_sequence field")
            sequence = int(matches[0].group(1))
            if sequence <= previous:
                raise TraceError(f"{path}:{line_number}: event_sequence is not strictly increasing")
            previous = sequence
            yield sequence, line
    if header_index != len(headers):
        raise TraceError(f"{path}: missing trace header")


def merge(inputs: list[tuple[str, Path]]) -> Iterator[dict[str, object]]:
    iterators = [records(path, event_type) for event_type, path in inputs]
    heap: list[tuple[int, int, str]] = []
    for index, iterator in enumerate(iterators):
        try:
            sequence, line = next(iterator)
        except StopIteration:
            continue
        heapq.heappush(heap, (sequence, index, line))
    previous = -1
    while heap:
        sequence, index, line = heapq.heappop(heap)
        if sequence <= previous:
            raise TraceError(f"duplicate or unordered shared event_sequence={sequence}")
        previous = sequence
        yield {"event_sequence": sequence, "event": line.split(" ", 1)[0], "record": line}
        try:
            next_sequence, next_line = next(iterators[index])
        except StopIteration:
            continue
        heapq.heappush(heap, (next_sequence, index, next_line))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cd-data-port", required=True, type=Path)
    parser.add_argument("--cd-ram-target", required=True, type=Path)
    parser.add_argument("--vram-commit", required=True, type=Path)
    parser.add_argument("--vram-commit-limit", type=int)
    parser.add_argument("--vce-write", type=Path)
    parser.add_argument("--vce-write-limit", type=int)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    inputs = [
        ("cd_data_port_read", args.cd_data_port),
        ("cd_ram_target_write", args.cd_ram_target),
        ("vram_commit", args.vram_commit),
    ]
    if args.vce_write is not None:
        inputs.append(("vce_write", args.vce_write))
    if args.vram_commit_limit is not None and args.vram_commit_limit < 1:
        parser.error("--vram-commit-limit must be positive")
    if args.vce_write_limit is not None and args.vce_write_limit < 1:
        parser.error("--vce-write-limit must be positive")
    if args.vce_write_limit is not None and args.vce_write is None:
        parser.error("--vce-write-limit requires --vce-write")
    limits: dict[str, int] = {}
    if args.vram_commit_limit is not None:
        limits["vram_commit"] = args.vram_commit_limit
    if args.vce_write_limit is not None:
        limits["vce_write"] = args.vce_write_limit
    created_output = False
    try:
        stream_counts = {
            event_type: sum(1 for _ in records(path, event_type))
            for event_type, path in inputs
        }
        for event_type, limit in limits.items():
            if stream_counts[event_type] > limit:
                raise TraceError(
                    f"{event_type} trace has {stream_counts[event_type]} records, "
                    f"exceeding configured limit {limit}"
                )
        limit_reached = any(
            stream_counts[event_type] >= limit
            for event_type, limit in limits.items()
        )
        with args.output.open("x", encoding="ascii", newline="\n") as output:
            created_output = True
            output.write(json.dumps({
                "format": "firestaff-theron-source-graphics-order-v1",
                "ordering": "shared_event_sequence",
                "interpretation": "temporal_order_only_not_causal_dataflow",
                "streams": [event_type for event_type, _ in inputs],
                "completeness": (
                    "possible_truncation_at_event_limit" if limit_reached else
                    "below_configured_event_limits" if limits else "not_checked"
                ),
                "bounded_streams": {
                    event_type: {
                        "limit": limit,
                        "records": stream_counts[event_type],
                        "limit_reached": stream_counts[event_type] >= limit,
                    }
                    for event_type, limit in limits.items()
                },
            }, separators=(",", ":")) + "\n")
            for record in merge(inputs):
                output.write(json.dumps(record, separators=(",", ":")) + "\n")
    except (OSError, TraceError) as error:
        if created_output:
            try:
                args.output.unlink(missing_ok=True)
            except OSError:
                pass
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
