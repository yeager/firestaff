#!/usr/bin/env python3
"""Verify the captured Nexus B -> inventory -> A event trace against retail data."""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path


DM_BIN_SHA256 = "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"
DM_BASE = 0x06010000
EVENT_NAME_TABLE = 0x3ADC4
EVENT_DISPATCH_TABLE = 0x2B4
EVENT_DISPATCH_BASE = 0x29E
WRAM_SIZE = 0x100000
EVENT_QUEUE_OFFSET = 0x797B0
CONTEXT_BASE_OFFSET = 0x7FBA0
CONTEXT_STRIDE = 0x13C
CONTEXT_INDEX = 10
CONTEXT_ITEM_PAIR_OFFSET = 0x122

CAPTURE_SHA256 = {
    "before": "9a838e897324452a2a757ae1de58c602afdae9fe05c594f10dc2b626338e5e91",
    "after_b": "3c80558714d5b410e32c23b354a8364c9e3764a2e3e283bb3493b198bdc871b0",
    "after_b_a": "1b3c4e80cac9bc4d67c1bb522a2a7f4e2e712ff6e63dda6c183140e1a11da32d",
}


class TraceError(ValueError):
    pass


def read_pinned(path: Path, expected_sha256: str, size: int) -> bytes:
    try:
        data = path.read_bytes()
    except OSError as error:
        raise TraceError(f"cannot read {path}: {error}") from error
    actual = hashlib.sha256(data).hexdigest()
    if actual != expected_sha256:
        raise TraceError(f"{path}: SHA-256 mismatch: {actual}")
    if len(data) != size:
        raise TraceError(f"{path}: expected {size} bytes, got {len(data)}")
    return data


def event_name(dm_bin: bytes, event_id: int) -> tuple[str, int]:
    pointer_offset = EVENT_NAME_TABLE + event_id * 4
    if pointer_offset + 4 > len(dm_bin):
        raise TraceError(f"event-name pointer for ID {event_id} is out of bounds")
    pointer = struct.unpack_from(">I", dm_bin, pointer_offset)[0]
    string_offset = pointer - DM_BASE
    if not 0 <= string_offset < len(dm_bin):
        raise TraceError(f"event-name pointer for ID {event_id} is invalid")
    end = dm_bin.find(b"\0", string_offset)
    if end < 0:
        raise TraceError(f"event-name string for ID {event_id} is unterminated")
    try:
        name = dm_bin[string_offset:end].decode("ascii")
    except UnicodeDecodeError as error:
        raise TraceError(f"event-name string for ID {event_id} is not ASCII") from error
    return name, string_offset


def dispatch_target(dm_bin: bytes, event_id: int) -> int:
    table_offset = EVENT_DISPATCH_TABLE + (event_id - 2) * 2
    if event_id < 2 or table_offset + 2 > len(dm_bin):
        raise TraceError(f"dispatch-table entry for ID {event_id} is out of bounds")
    relative = struct.unpack_from(">h", dm_bin, table_offset)[0]
    return EVENT_DISPATCH_BASE + relative


def queued_event(wram: bytes) -> tuple[int, int, int]:
    return struct.unpack_from(">III", wram, EVENT_QUEUE_OFFSET)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dm_bin", type=Path, help="hash-pinned Japanese retail DM.BIN")
    parser.add_argument("evidence_dir", type=Path,
                        help="nexus-v1-inventory-button-debug-20261025 directory")
    args = parser.parse_args()

    try:
        dm_bin = args.dm_bin.read_bytes()
        if not dm_bin:
            raise TraceError(f"DM.BIN is empty: {args.dm_bin}")
        dm_hash = hashlib.sha256(dm_bin).hexdigest()
        if dm_hash != DM_BIN_SHA256:
            raise TraceError(f"DM.BIN SHA-256 mismatch: {dm_hash}")

        captures = {
            label: read_pinned(args.evidence_dir / filename, expected_hash, WRAM_SIZE)
            for label, filename, expected_hash in (
                ("before", "wram-hi-before.bin", CAPTURE_SHA256["before"]),
                ("after_b", "wram-hi-after-b.bin", CAPTURE_SHA256["after_b"]),
                ("after_b_a", "wram-hi-after-b-a.bin", CAPTURE_SHA256["after_b_a"]),
            )
        }

        expected = {
            "after_b": (12, 0, 0),
            "after_b_a": (39, 8, 0),
        }
        for label, (event_id, argument, extra) in expected.items():
            actual = queued_event(captures[label])
            if actual != (event_id, argument, extra):
                raise TraceError(f"{label}: expected queue head "
                                 f"{(event_id, argument, extra)}, got {actual}")

        inventory_name, inventory_string = event_name(dm_bin, 12)
        ring_name, ring_string = event_name(dm_bin, 39)
        if (inventory_name, ring_name) != ("EV_INVENTORY", "EV_RING_SELECT"):
            raise TraceError(f"unexpected event names: {inventory_name}, {ring_name}")

        inventory_case = dispatch_target(dm_bin, 12)
        ring_case = dispatch_target(dm_bin, 39)
        if (inventory_case, ring_case) != (0x682, 0x7EE):
            raise TraceError(f"unexpected event cases: {inventory_case:#x}, {ring_case:#x}")

        before = captures["before"]
        after = captures["after_b_a"]
        context_offset = (CONTEXT_BASE_OFFSET + CONTEXT_INDEX * CONTEXT_STRIDE +
                          CONTEXT_ITEM_PAIR_OFFSET)
        pair_before = before[context_offset:context_offset + 4]
        pair_after = after[context_offset:context_offset + 4]
        if (pair_before, pair_after) != (bytes.fromhex("ffff0000"),
                                        bytes.fromhex("141a4000")):
            raise TraceError(f"context-10 pair mismatch: {pair_before.hex()} -> "
                             f"{pair_after.hex()}")

    except (OSError, TraceError) as error:
        print(f"NEXUS_INVENTORY_BUTTON_TRACE_INVALID: {error}", file=sys.stderr)
        return 1

    print(f"dm_bin_sha256={dm_hash}")
    print(f"button_b_event=12:{inventory_name} dispatch=0x{inventory_case:03x}")
    print(f"button_a_event=39:{ring_name} argument=8 dispatch=0x{ring_case:03x}")
    print(f"context10_pair=0x{context_offset:05x} {pair_before.hex()}->{pair_after.hex()}")
    print("retail_event_trace=verified champion_item_join=unproven "
          "persistent_item_transfer=unproven")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
