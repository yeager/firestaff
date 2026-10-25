#!/usr/bin/env python3
"""Compare two raw Ymir high-WRAM dumps around a Nexus input action.

Ymir's Debug > Dump all memory action writes a raw ``wram-hi.bin`` file. This
tool reads those authentic 1 MiB snapshots without modifying them. It reports
the retail event queue and item-pair deltas in the observed Nexus context
records. When authentic DM.BIN is supplied, event IDs are resolved through its
name-pointer table. Empty queue slots are marked stale, not presented as live
events. The tool does not identify a context as a champion or infer that an
input caused a state change.

Example::

    python3 scripts/analyze_nexus_inventory_wram_pair.py before/wram-hi.bin after/wram-hi.bin
"""

from __future__ import annotations

import argparse
import hashlib
import os
import struct
import sys
from pathlib import Path


WRAM_SIZE = 0x100000
WRAM_BASE = 0x06000000
DM_BASE = 0x06010000
DM_BIN_SHA256 = "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"
EVENT_NAME_POINTER_TABLE_OFFSET = 0x3ADC4
EVENT_NAME_COUNT = 61
EVENT_QUEUE_OFFSET = 0x797B0
EVENT_QUEUE_COUNT_OFFSET = 0x79814
EVENT_QUEUE_STATE_OFFSET = 0x79810
EVENT_QUEUE_CAPACITY = 8
EVENT_QUEUE_RECORD_SIZE = 12
CONTEXT_BASE_OFFSET = 0x7FBA0
CONTEXT_STRIDE = 0x13C
CONTEXT_ITEM_PAIRS_OFFSET = 0x110
CONTEXT_ITEM_SLOT_COUNT = 8


class CaptureError(ValueError):
    """Raised when a capture cannot be safely interpreted as high WRAM."""


def read_capture(path: Path) -> bytes:
    try:
        with path.open("rb") as capture:
            data = capture.read(WRAM_SIZE + 1)
    except OSError as error:
        raise CaptureError(f"cannot read {path}: {error}") from error
    if len(data) != WRAM_SIZE:
        size = f"at least {len(data)}" if len(data) > WRAM_SIZE else str(len(data))
        raise CaptureError(
            f"{path}: expected {WRAM_SIZE} bytes for Ymir high WRAM, got {size}"
        )
    return data


def resolve_dm_bin(explicit: Path | None) -> Path | None:
    if explicit is not None:
        return explicit
    data_dir = os.environ.get("FIRESTAFF_NEXUS_DATA_DIR")
    candidate = (Path(data_dir) / "DM.BIN") if data_dir else (
        Path.home() / ".firestaff" / "data" / "nexus" / "DM.BIN"
    )
    return candidate if candidate.is_file() else None


def read_event_names(path: Path) -> dict[int, str]:
    try:
        data = path.read_bytes()
    except OSError as error:
        raise CaptureError(f"cannot read authentic DM.BIN {path}: {error}") from error
    digest = hashlib.sha256(data).hexdigest()
    if digest != DM_BIN_SHA256:
        raise CaptureError(
            f"{path}: expected canonical Japanese retail DM.BIN SHA-256 "
            f"{DM_BIN_SHA256}, got {digest}"
        )
    names: dict[int, str] = {}
    for event_id in range(EVENT_NAME_COUNT):
        pointer_offset = EVENT_NAME_POINTER_TABLE_OFFSET + event_id * 4
        if pointer_offset + 4 > len(data):
            raise CaptureError("DM.BIN event-name pointer table is truncated")
        address = int.from_bytes(data[pointer_offset:pointer_offset + 4], "big")
        string_offset = address - DM_BASE
        if not 0 <= string_offset < len(data):
            raise CaptureError(
                f"DM.BIN event-name pointer {event_id} is out of range: 0x{address:08x}"
            )
        end = data.find(b"\0", string_offset, min(len(data), string_offset + 128))
        if end < 0:
            raise CaptureError(f"DM.BIN event name {event_id} is unterminated")
        try:
            names[event_id] = data[string_offset:end].decode("ascii")
        except UnicodeDecodeError as error:
            raise CaptureError(f"DM.BIN event name {event_id} is not ASCII") from error
    return names


def be_u32(data: bytes, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def queue_records(data: bytes) -> list[tuple[int, int, int]]:
    return [
        struct.unpack_from(">III", data, EVENT_QUEUE_OFFSET + i * EVENT_QUEUE_RECORD_SIZE)
        for i in range(EVENT_QUEUE_CAPACITY)
    ]


def context_pairs(data: bytes, index: int) -> tuple[tuple[int, int], ...]:
    start = CONTEXT_BASE_OFFSET + index * CONTEXT_STRIDE + CONTEXT_ITEM_PAIRS_OFFSET
    return tuple(
        struct.unpack_from(">HH", data, start + slot * 4)
        for slot in range(CONTEXT_ITEM_SLOT_COUNT)
    )


def format_pair(pair: tuple[int, int]) -> str:
    item, auxiliary = pair
    item_text = "empty" if item == 0xFFFF else f"0x{item:04x}"
    return f"{item_text}/aux=0x{auxiliary:04x}"


def emit_capture(label: str, data: bytes,
                 event_names: dict[int, str] | None) -> None:
    queue_count = be_u32(data, EVENT_QUEUE_COUNT_OFFSET)
    read_cursor = be_u32(data, EVENT_QUEUE_STATE_OFFSET)
    print(f"{label}_sha256={hashlib.sha256(data).hexdigest()}")
    print(f"{label}_queue_count=0x{queue_count:08x}")
    print(f"{label}_queue_read_cursor=0x{read_cursor:08x}")
    queue_valid = queue_count <= EVENT_QUEUE_CAPACITY and read_cursor < EVENT_QUEUE_CAPACITY
    if not queue_valid:
        print(f"{label}_queue_warning=count_or_read_cursor_out_of_range")
    live_indices = (
        {(read_cursor + index) % EVENT_QUEUE_CAPACITY for index in range(queue_count)}
        if queue_valid else set()
    )
    records = queue_records(data)
    for index, record in enumerate(records):
        event, arg0, arg1 = record
        status = "live" if index in live_indices else "stale"
        event_name = (event_names or {}).get(event)
        rendered_event = f"{event}({event_name})" if event_name else str(event)
        print(
            f"{label}_queue[{index}]={status} event:{rendered_event} "
            f"arg0:0x{arg0:08x} "
            f"arg1:0x{arg1:08x}"
        )
    if any(index in live_indices and records[index][0] == 44
           for index in range(EVENT_QUEUE_CAPACITY)):
        print(f"{label}_event44_live_in_queue=yes")
    else:
        print(f"{label}_event44_live_in_queue=no")
    if any(record[0] == 44 for record in records):
        print(f"{label}_event44_present_in_queue_storage=yes")
    else:
        print(f"{label}_event44_present_in_queue_storage=no")


def analyze(before: bytes, after: bytes, context_start: int, context_count: int,
            event_names: dict[int, str] | None = None) -> int:
    before_queue = queue_records(before)
    after_queue = queue_records(after)
    emit_capture("before", before, event_names)
    emit_capture("after", after, event_names)

    changed_slots = 0
    changed_contexts = 0
    for index in range(context_start, context_start + context_count):
        old_pairs = context_pairs(before, index)
        new_pairs = context_pairs(after, index)
        deltas = [
            (slot, old, new)
            for slot, (old, new) in enumerate(zip(old_pairs, new_pairs))
            if old != new
        ]
        if deltas:
            changed_contexts += 1
            changed_slots += len(deltas)
            for slot, old, new in deltas:
                address = (
                    WRAM_BASE
                    + CONTEXT_BASE_OFFSET
                    + index * CONTEXT_STRIDE
                    + CONTEXT_ITEM_PAIRS_OFFSET
                    + slot * 4
                )
                print(
                    f"context[{index}].slot[{slot}] @0x{address:08x}: "
                    f"{format_pair(old)} -> {format_pair(new)}"
                )

    print(f"context_range={context_start}..{context_start + context_count - 1}")
    print(f"changed_contexts={changed_contexts}")
    print(f"changed_item_slots={changed_slots}")
    print(f"queue_storage_changed={'yes' if before_queue != after_queue else 'no'}")
    if before == after:
        print("capture_warning=before and after snapshots are byte-identical")
    print(
        "interpretation=raw memory deltas only; context-to-champion ownership, "
        "click causality, and item semantics require separate evidence"
    )
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("before", type=Path, help="authentic Ymir high-WRAM baseline")
    parser.add_argument("after", type=Path, help="authentic Ymir high-WRAM post-action dump")
    parser.add_argument(
        "--context-start", type=int, default=0,
        help="first retail context index to compare (default: 0)",
    )
    parser.add_argument(
        "--context-count", type=int, required=True,
        help="number of retail context records to compare",
    )
    parser.add_argument(
        "--dm-bin", type=Path,
        help="hash-verified authentic Japanese retail DM.BIN for event-name labels "
             "(defaults to FIRESTAFF_NEXUS_DATA_DIR/DM.BIN or ~/.firestaff/data/nexus/DM.BIN)",
    )
    args = parser.parse_args(argv)
    if args.context_start < 0 or args.context_count < 1:
        parser.error("context indices must be nonnegative and context-count must be positive")
    last_context = args.context_start + args.context_count - 1
    if CONTEXT_BASE_OFFSET + last_context * CONTEXT_STRIDE + CONTEXT_ITEM_PAIRS_OFFSET + 32 > WRAM_SIZE:
        parser.error("requested context range exceeds the high-WRAM dump")

    try:
        before = read_capture(args.before)
        after = read_capture(args.after)
        dm_bin = resolve_dm_bin(args.dm_bin)
        event_names = read_event_names(dm_bin) if dm_bin is not None else None
    except CaptureError as error:
        print(f"NEXUS_WRAM_PAIR_INVALID: {error}", file=sys.stderr)
        return 1
    if event_names is None:
        print("event_names=unavailable; pass --dm-bin to resolve authentic IDs")
    else:
        print(f"event_names=verified dm_sha256={DM_BIN_SHA256}")
    return analyze(before, after, args.context_start, args.context_count, event_names)


if __name__ == "__main__":
    raise SystemExit(main())
