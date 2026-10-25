#!/usr/bin/env python3
"""Compare two raw Ymir high-WRAM dumps around a Nexus inventory action.

Ymir's Debug > Dump all memory action writes a raw ``wram-hi.bin`` file. This
tool reads those authentic 1 MiB snapshots without modifying them. It reports
the retail event queue and item-pair deltas in the observed Nexus context
records. It deliberately does not identify a context as a champion or infer
that a click succeeded from a pointer position alone.

Example::

    python3 scripts/analyze_nexus_inventory_wram_pair.py before/wram-hi.bin after/wram-hi.bin
"""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path


WRAM_SIZE = 0x100000
WRAM_BASE = 0x06000000
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
        data = path.read_bytes()
    except OSError as error:
        raise CaptureError(f"cannot read {path}: {error}") from error
    if len(data) != WRAM_SIZE:
        raise CaptureError(
            f"{path}: expected {WRAM_SIZE} bytes for Ymir high WRAM, got {len(data)}"
        )
    return data


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


def emit_capture(label: str, data: bytes) -> None:
    print(f"{label}_sha256={hashlib.sha256(data).hexdigest()}")
    print(f"{label}_queue_count=0x{be_u32(data, EVENT_QUEUE_COUNT_OFFSET):08x}")
    print(f"{label}_queue_state=0x{be_u32(data, EVENT_QUEUE_STATE_OFFSET):08x}")
    records = queue_records(data)
    for index, record in enumerate(records):
        event, arg0, arg1 = record
        print(
            f"{label}_queue[{index}]=event:{event} arg0:0x{arg0:08x} "
            f"arg1:0x{arg1:08x}"
        )
    if any(record[0] == 44 for record in records):
        print(f"{label}_event44_present_in_queue_storage=yes")
    else:
        print(f"{label}_event44_present_in_queue_storage=no")


def analyze(before: bytes, after: bytes, context_start: int, context_count: int) -> int:
    before_queue = queue_records(before)
    after_queue = queue_records(after)
    emit_capture("before", before)
    emit_capture("after", after)

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
    args = parser.parse_args(argv)
    if args.context_start < 0 or args.context_count < 1:
        parser.error("context indices must be nonnegative and context-count must be positive")
    last_context = args.context_start + args.context_count - 1
    if CONTEXT_BASE_OFFSET + last_context * CONTEXT_STRIDE + CONTEXT_ITEM_PAIRS_OFFSET + 32 > WRAM_SIZE:
        parser.error("requested context range exceeds the high-WRAM dump")

    try:
        before = read_capture(args.before)
        after = read_capture(args.after)
    except CaptureError as error:
        print(f"NEXUS_INVENTORY_WRAM_PAIR_INVALID: {error}", file=sys.stderr)
        return 1
    return analyze(before, after, args.context_start, args.context_count)


if __name__ == "__main__":
    raise SystemExit(main())
