#!/usr/bin/env python3
"""Verify decompiled Continue field copies against authentic Ymir RAM dumps.

The two inputs must be the same run's raw 1 MiB WorkRAM-HI and WorkRAM-Low
files. This checks only fields whose copy/conversion operations are visible in
the pcrecomp/Ghidra decompilation; it does not assign save-field semantics.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path
import struct


WRAM_BYTES = 0x100000
PAYLOAD_OFFSET = 0x4000
PAYLOAD_BYTES = 0x5000
AUTHENTIC_PAYLOADS = {
    "9c969745745acc7f843148abf78f114ab56e82b39b7f01251b6407a0d9fdf4eb":
        ("two-champion", 2),
    "1e7e20ee72cdc57c50557ec6bf6325be22c299558a9b8f2731a78877d89725a3":
        ("three-champion", 3),
    "1c460c65cc6156843ec4e0c1c8ca4ae3667241cc46a04151624b578e478c47e5":
        ("four-champion", 4),
    "b77461232e7cf3485be0e179b5feab777c91efa187d0ad6bdcc7bb9867058e3e":
        ("four-champion-leader-3", 4),
}
PAYLOAD_MUTATION_OFFSET = 0x58
SOURCE_RECORD_OFFSET = 0x11C
SOURCE_STRIDE_CONSTANT = 0x2EF5A
TARGET_STRIDE_CONSTANT = 0x2EF5C
WORLD_CONTEXT_OFFSET = 0x80090
CHAMPION_ARRAY_OFFSET = 0x7FBA0


def read_dump(path: Path, label: str) -> bytes:
    try:
        data = path.read_bytes()
    except OSError as error:
        raise ValueError(f"cannot read {label}: {error}") from error
    if len(data) != WRAM_BYTES:
        raise ValueError(f"{label} must be exactly 0x{WRAM_BYTES:x} bytes")
    return data


def verify_leader3_update_trace(
    trace_path: Path, bundle_path: Path
) -> tuple[list[str], set[tuple[int, int]]]:
    try:
        bundle_lines = bundle_path.read_text(encoding="ascii").splitlines()
        with trace_path.open("r", encoding="ascii", newline="") as stream:
            reader = csv.DictReader(stream)
            rows = list(reader)
            fields = reader.fieldnames
    except (OSError, UnicodeError, csv.Error) as error:
        raise ValueError(f"cannot read runtime-update trace: {error}") from error
    if fields != [
        "sh2", "vdp1_frame", "pc", "caller_return_address", "caller_target",
        "address", "width", "value", "caller_stack_targets",
    ]:
        raise ValueError("unexpected SH-2 write-trace columns")
    bundle = dict(line.split("=", 1) for line in bundle_lines if "=" in line)
    if (bundle.get("format") != "ymir-sh2-memory-write-trace-bundle-v1" or
            bundle.get("trace_file_complete") != "1" or
            bundle.get("bundle_complete") != "1" or
            bundle.get("master_dropped") != "0" or
            bundle.get("slave_dropped") != "0" or
            int(bundle.get("trace_rows", "-1")) != len(rows)):
        raise ValueError("SH-2 write-trace bundle is incomplete or inconsistent")

    targets = (0x0607FBA8, 0x0607FCE4, 0x0607FE20, 0x0607FF5C)
    target_rows = {
        address: [row for row in rows if int(row["address"], 16) == address]
        for address in targets
    }
    expected = [
        (12931, 0x00, 0x06041FBC),
        (12931, 0x43, 0x0602EE88),
        (13055, 0x44, 0x06036158),
        (13055, 0x43, 0x06036176),
        (13951, 0x40, 0x06036158),
        (13951, 0x42, 0x06036158),
    ]
    for index, address in enumerate(targets):
        target = target_rows[address]
        observed = [
            (int(row["vdp1_frame"]), int(row["value"], 16), int(row["pc"], 16))
            for row in target
        ]
        if observed != expected:
            raise ValueError(
                f"leader-3 record {index} field write sequence differs from the retained retail trace"
            )
        if (target[0]["width"] != "1" or
                any(row["width"] != "4" for row in target[1:])):
            raise ValueError(f"leader-3 record {index} field trace has an unexpected write width")
        if any("0x60360fc" not in row["caller_stack_targets"] for row in target[2:]):
            raise ValueError(f"leader-3 record {index} post-load writes do not join to FUN_060360fc")

    # Source +0x40 ends at runtime +0x10. The retained window reaches only
    # the first three of those four fields; do not claim the fourth was traced.
    plus10_targets = (0x0607FBB0, 0x0607FCEC, 0x0607FE28)
    plus10_expected = [
        (12931, 0x00, 0x06041FBC),
        (12931, 0x12, 0x0602EE94),
        (13951, 0x13, 0x06036272),
    ]
    traced_fields = {(index, 0x3C) for index in range(4)}
    for index, address in enumerate(plus10_targets):
        target = [row for row in rows if int(row["address"], 16) == address]
        observed = [
            (int(row["vdp1_frame"]), int(row["value"], 16), int(row["pc"], 16))
            for row in target
        ]
        if observed != plus10_expected:
            raise ValueError(
                f"leader-3 record {index} +0x10 write sequence differs from the retained retail trace"
            )
        if target[0]["width"] != "1" or any(row["width"] != "4" for row in target[1:]):
            raise ValueError(f"leader-3 record {index} +0x10 trace has an unexpected write width")
        if "0x60361a8" not in target[2]["caller_stack_targets"]:
            raise ValueError(f"leader-3 record {index} +0x10 update does not join to FUN_060361a8")
        traced_fields.add((index, 0x40))

    # FUN_060361a8 also updates the unsigned source halfwords +0x44/+0x46 at
    # runtime +0x18/+0x1a. These addresses are in range for records 0-2 only.
    for source_field, target_field, load_value, update_values, update_pc in (
        (0x44, 0x18, 0x674, (0x672, 0x670, 0x66E), 0x060363EC),
        (0x46, 0x1A, 0x6CF, (0x6CE, 0x6CD, 0x6CC), 0x06036462),
    ):
        load_pc = 0x0602EE9E if source_field == 0x44 else 0x0602EEA4
        expected_sequence = [
            (12931, 0x00, 0x06041FBC),
            (12931, load_value, load_pc),
            (13055, update_values[0], update_pc),
            (13951, update_values[1], update_pc),
            (13951, update_values[2], update_pc),
        ]
        for index in range(3):
            address = 0x0607FBA0 + index * 0x13C + target_field
            target = [row for row in rows if int(row["address"], 16) == address]
            observed = [
                (int(row["vdp1_frame"]), int(row["value"], 16), int(row["pc"], 16))
                for row in target
            ]
            if observed != expected_sequence:
                raise ValueError(
                    f"leader-3 record {index} +0x{source_field:x} write sequence "
                    "differs from the retained retail trace"
                )
            if target[0]["width"] != "1" or any(row["width"] != "2" for row in target[1:]):
                raise ValueError(
                    f"leader-3 record {index} +0x{source_field:x} trace has an unexpected write width"
                )
            if any("0x60361a8" not in row["caller_stack_targets"] for row in target[2:]):
                raise ValueError(
                    f"leader-3 record {index} +0x{source_field:x} update "
                    "does not join to FUN_060361a8"
                )
            traced_fields.add((index, source_field))
    return [
        f"leader3_target_trace_rows={sum(map(len, target_rows.values())) + 39}",
        "leader3_records_traced=4",
        "leader3_source40_runtime10_records_traced=3",
        "leader3_source40_runtime10_record3_in_trace=0",
        "leader3_target_clear=frame12931:value0:pc0x06041fbc",
        "leader3_load_write=frame12931:value67:pc0x0602ee88",
        "leader3_postload_writes=frame13055:68,67;frame13951:64,66",
        "leader3_postload_writer=FUN_060360fc",
        "leader3_source40_runtime10_postload=frame13951:value19:pc0x06036272:caller_FUN_060361a8",
        "leader3_source44_runtime18_postload=frame13055:0x672;frame13951:0x670,0x66e:caller_FUN_060361a8",
        "leader3_source46_runtime1a_postload=frame13055:0x6ce;frame13951:0x6cd,0x6cc:caller_FUN_060361a8",
        "leader3_source44_runtime18_record3_in_trace=0",
        "leader3_source46_runtime1a_record3_in_trace=0",
    ], traced_fields


def verify(
    hi: bytes,
    low: bytes,
    update_trace: list[str] | None = None,
    traced_fields: set[tuple[int, int]] | None = None,
) -> list[str]:
    payload = bytearray(low[PAYLOAD_OFFSET:PAYLOAD_OFFSET + PAYLOAD_BYTES])
    if len(payload) != PAYLOAD_BYTES:
        raise ValueError("captured Continue payload is truncated")
    captured_hash = hashlib.sha256(payload).hexdigest()
    captured_fixup = bytes(payload[PAYLOAD_MUTATION_OFFSET:PAYLOAD_MUTATION_OFFSET + 4])
    if captured_fixup != bytes.fromhex("05efbf3c"):
        raise ValueError("captured payload does not contain the traced runtime fixup")
    payload[PAYLOAD_MUTATION_OFFSET:PAYLOAD_MUTATION_OFFSET + 4] = bytes.fromhex(
        "000000cc"
    )
    restored_hash = hashlib.sha256(payload).hexdigest()
    if restored_hash not in AUTHENTIC_PAYLOADS:
        raise ValueError("normalized captured payload is not one of the four authentic BKR corpus entries")
    variant, expected_count = AUTHENTIC_PAYLOADS[restored_hash]
    payload_header = struct.unpack_from(">II", payload, 0x54)
    if payload_header != (0x10, 0xCC):
        raise ValueError("normalized payload does not contain the decompiled source offsets")

    source_stride = struct.unpack_from(">H", hi, SOURCE_STRIDE_CONSTANT)[0]
    target_stride = struct.unpack_from(">H", hi, TARGET_STRIDE_CONSTANT)[0]
    if (source_stride, target_stride) != (0xE0, 0x13C):
        raise ValueError("captured retail code constants do not match the decompiled record strides")

    champion_count = struct.unpack_from(">H", hi, WORLD_CONTEXT_OFFSET)[0]
    source_count = payload[0x62]
    if champion_count != expected_count or source_count != expected_count:
        raise ValueError(
            f"{variant} count mismatch: payload={source_count}, runtime={champion_count}"
        )
    party_leader_index = struct.unpack_from(">b", payload, 0x61)[0]
    runtime_party_leader_index = struct.unpack_from(">i", hi, WORLD_CONTEXT_OFFSET + 0x0C)[0]
    if not 0 <= party_leader_index < champion_count:
        raise ValueError(f"{variant} party leader index is outside the roster")
    if runtime_party_leader_index != party_leader_index:
        raise ValueError(
            f"{variant} party leader index mismatch: payload={party_leader_index}, "
            f"runtime={runtime_party_leader_index}"
        )
    if SOURCE_RECORD_OFFSET + champion_count * source_stride > len(payload):
        raise ValueError("source champion records exceed the captured payload")
    if CHAMPION_ARRAY_OFFSET + champion_count * target_stride > len(hi):
        raise ValueError("target champion records exceed WorkRAM-HI")

    matches: list[str] = []
    differences: list[str] = []
    traced_differences: list[str] = []
    unresolved_differences: list[str] = []
    # Keep each operation explicit against FUN_0602eb4c's decompilation. The
    # offsets prove mechanical projection only; they do not assign meanings
    # to the source or runtime fields.
    byte_copies = (
        (0x00, 0x2C, 6),
        (0x06, 0x32, 1),
        (0x07, 0x1C, 1),
        (0x08, 0x1D, 1),
        (0x09, 0x35, 1),
        (0x32, 0x3E, 6),
        (0x2F, 0x4C, 1),
        (0x30, 0x3C, 2),
        (0xD0, 0x34, 1),
        (0xD1, 0x12C, 6),
    )
    byte_to_halfword_copies = (
        (0x0A, 0x36),
        (0x0B, 0x38),
    )
    signed_halfword_copies = (
        (0x38, 0x00),
        (0x3A, 0x04),
        (0x3C, 0x08),
        (0x3E, 0x0C),
        (0x40, 0x10),
        (0x42, 0x14),
        (0x48, 0x20),
        (0x4A, 0x24),
    )
    unsigned_halfword_copies = (
        (0x44, 0x18),
        (0x46, 0x1A),
        (0x4C, 0x28),
        (0x4E, 0x2A),
        (0x50, 0x44),
        (0x52, 0x48),
        (0x54, 0x4A),
        (0x56, 0x58),
        (0xD8, 0x46),
    )
    for index in range(champion_count):
        source = SOURCE_RECORD_OFFSET + index * source_stride
        target = CHAMPION_ARRAY_OFFSET + index * target_stride
        for source_field, target_field, width in byte_copies:
            expected = payload[source + source_field:source + source_field + width]
            actual = hi[target + target_field:target + target_field + width]
            if actual != expected:
                raise ValueError(
                    f"champion record {index}, byte copy +0x{source_field:x}"
                    f" -> +0x{target_field:x} does not match"
                )
            matches.append(f"record{index}:bytes+0x{source_field:x}->+0x{target_field:x}")
        for source_field, target_field in byte_to_halfword_copies:
            expected = payload[source + source_field]
            actual = struct.unpack_from(">H", hi, target + target_field)[0]
            if actual != expected:
                raise ValueError(
                    f"champion record {index}, byte-to-halfword +0x{source_field:x}"
                    f" -> +0x{target_field:x} does not match"
                )
            matches.append(
                f"record{index}:u8+0x{source_field:x}->u16+0x{target_field:x}"
            )
        for item_index in range(14):
            source_field = 0x21 + item_index
            target_field = 0x110 + item_index * 2
            source_value = payload[source + source_field]
            expected = (
                0xFFFF if item_index % 2 == 0 and source_value == 0xFF
                else source_value
            )
            actual = struct.unpack_from(">H", hi, target + target_field)[0]
            if actual != expected:
                raise ValueError(
                    f"champion record {index}, byte table +0x{source_field:x}"
                    f" -> halfword table +0x{target_field:x} does not match"
                )
            matches.append(
                f"record{index}:table_u8+0x{source_field:x}->u16+0x{target_field:x}"
            )
        for item_index in range(10):
            for source_field, target_field, width in (
                (0x58 + item_index * 4, 0x70 + item_index * 0x10, 2),
                (0x5A + item_index * 4, 0x78 + item_index * 0x10, 2),
                (0x80 + item_index * 8, 0x74 + item_index * 0x10, 4),
                (0x84 + item_index * 8, 0x7C + item_index * 0x10, 4),
            ):
                expected = payload[source + source_field:source + source_field + width]
                actual = hi[target + target_field:target + target_field + width]
                if actual != expected:
                    raise ValueError(
                        f"champion record {index}, table projection +0x{source_field:x}"
                        f" -> +0x{target_field:x} does not match"
                    )
                matches.append(
                    f"record{index}:table+0x{source_field:x}->+0x{target_field:x}"
                )
        for source_field, target_field in signed_halfword_copies:
            expected = struct.unpack_from(">h", payload, source + source_field)[0]
            actual = struct.unpack_from(">i", hi, target + target_field)[0]
            if actual != expected:
                known_leader3_difference = (
                    variant == "four-champion-leader-3" and
                    source_field == 0x3C and target_field == 0x08 and
                    expected == 67 and actual == 66
                )
                known_updated_value_difference = (
                    source_field == 0x40 and target_field == 0x10 and
                    expected == 18 and actual == 19
                )
                if not known_leader3_difference and not known_updated_value_difference:
                    raise ValueError(
                        f"champion record {index}, signed halfword +0x{source_field:x}"
                        f" -> longword +0x{target_field:x} does not match"
                    )
                difference = (
                    f"record{index}:source_s16+0x{source_field:x}={expected}:"
                    f"final_runtime_s32+0x{target_field:02x}={actual}"
                )
                differences.append(difference)
                if traced_fields and (index, source_field) in traced_fields:
                    traced_differences.append(difference)
                else:
                    unresolved_differences.append(difference)
            else:
                matches.append(
                    f"record{index}:s16+0x{source_field:x}->s32+0x{target_field:x}"
                )
        for source_field, target_field in unsigned_halfword_copies:
            expected = struct.unpack_from(">H", payload, source + source_field)[0]
            actual = struct.unpack_from(">H", hi, target + target_field)[0]
            if actual != expected:
                known_runtime_decrement = (
                    (source_field == 0x44 and target_field == 0x18 and
                     expected >= 6 and actual == expected - 6) or
                    (source_field == 0x46 and target_field == 0x1A and
                     expected >= 3 and actual == expected - 3)
                )
                if not known_runtime_decrement:
                    raise ValueError(
                        f"champion record {index}, unsigned halfword +0x{source_field:x}"
                        f" -> +0x{target_field:x} does not match"
                    )
                difference = (
                    f"record{index}:source_u16+0x{source_field:x}={expected}:"
                    f"final_runtime_u16+0x{target_field:02x}={actual}"
                )
                differences.append(difference)
                if traced_fields and (index, source_field) in traced_fields:
                    traced_differences.append(difference)
                else:
                    unresolved_differences.append(difference)
            else:
                matches.append(
                    f"record{index}:u16+0x{source_field:x}->+0x{target_field:x}"
                )

    return [
        f"captured_payload_sha256={captured_hash}",
        f"normalized_authentic_payload_sha256={restored_hash}",
        f"authentic_payload_variant={variant}",
        "payload_source_offsets=0x10,0xcc",
        f"champion_count={champion_count}",
        f"party_leader_index={party_leader_index}",
        f"party_leader_index_projection=payload+0x61->runtime+0x{WORLD_CONTEXT_OFFSET + 0x0C:05x}",
        f"source_champion_stride=0x{source_stride:x}",
        f"target_champion_stride=0x{target_stride:x}",
        f"matched_projection_checks={len(matches)}",
        f"total_projection_checks={len(matches) + len(differences)}",
        *matches,
        f"final_snapshot_field_differences={len(differences)}",
        *differences,
        f"trace_explained_differences={len(traced_differences)}",
        *traced_differences,
        f"unresolved_final_snapshot_differences={len(unresolved_differences)}",
        *unresolved_differences,
        *(update_trace or []),
        "payload_projection=" + (
            "verified_for_listed_fields"
            if not differences else
            "partial_runtime_update_observed" if not unresolved_differences else
            "partial_with_unresolved_final_snapshot_differences"
        ),
        "save_field_semantics=unassigned",
        "complete_continue_parity=unproven",
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("wram_hi", type=Path)
    parser.add_argument("wram_low", type=Path)
    parser.add_argument("--leader3-write-trace", type=Path)
    parser.add_argument("--leader3-write-trace-bundle", type=Path)
    args = parser.parse_args()
    try:
        hi = read_dump(args.wram_hi, "WorkRAM-HI")
        low = read_dump(args.wram_low, "WorkRAM-Low")
        update_trace = None
        traced_fields = None
        if args.leader3_write_trace or args.leader3_write_trace_bundle:
            if not args.leader3_write_trace or not args.leader3_write_trace_bundle:
                raise ValueError("both leader-3 trace CSV and bundle are required")
            update_trace, traced_fields = verify_leader3_update_trace(
                args.leader3_write_trace, args.leader3_write_trace_bundle
            )
        receipt = verify(hi, low, update_trace, traced_fields)
    except (OSError, ValueError, struct.error) as error:
        print(f"NEXUS_BKR_RUNTIME_PROJECTION_INVALID: {error}")
        return 1
    print("\n".join(receipt))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
