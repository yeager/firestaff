#!/usr/bin/env python3
"""Verify bounded CD data-port traces against authentic JP Track 02."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import re
import sys
from pathlib import Path


@dataclass(frozen=True)
class Track02Layout:
    name: str
    md5: str
    size: int
    start_lba: int
    end_lba: int
    index1_file_offset: int
    sector_bytes: int
    file_user_data_offset: int
    source_user_data_offset: int = 16
    user_data_bytes: int = 2_048


JP_TRACK02_BIN = Track02Layout(
    "MODE1/2352",
    "b7afb338ad31be1025b53f9aff12d73a",
    8_102_640,
    3_590,
    6_961,
    526_848,
    2_352,
    16,
)
JP_TRACK02_ISO = Track02Layout(
    "MODE1/2048",
    "62a39bbf43415c9739c41c2481080a49",
    6_596_608,
    3_590,
    6_961,
    0,
    2_048,
    0,
)
TRACK02_LAYOUTS = (JP_TRACK02_BIN, JP_TRACK02_ISO)
# Preserve the raw-BIN constants imported by existing source-chain tools.
EXPECTED_TRACK02_MD5 = JP_TRACK02_BIN.md5
EXPECTED_TRACK02_BYTES = JP_TRACK02_BIN.size
TRACK02_START_LBA = JP_TRACK02_BIN.start_lba
TRACK02_END_LBA = JP_TRACK02_BIN.end_lba
TRACK02_INDEX1_FILE_OFFSET = JP_TRACK02_BIN.index1_file_offset
SECTOR_BYTES = JP_TRACK02_BIN.sector_bytes
USER_DATA_OFFSET = JP_TRACK02_BIN.source_user_data_offset
USER_DATA_BYTES = 2_048
MAX_OBSERVED_BYTES = 1_048_576
TRACE_HEADER = "source=mednafen-pce-fast-cd-data-port-read"
TRACE_HEADER_WITH_READER_PC = "source=mednafen-pce-fast-cd-data-port-read-with-reader-pc"
TRACE_HEADER_WITH_READER_MPR_CONTEXT = "source=mednafen-pce-fast-cd-data-port-read-with-reader-mpr-context"
TRACE_HEADER_WITH_READER_MPR_SHARED_SEQUENCE = (
    TRACE_HEADER_WITH_READER_MPR_CONTEXT
    + " event_sequence_source=TheronTraceCDSharedEventSequence"
)


def identify_track02(raw: bytes) -> Track02Layout:
    actual_md5 = hashlib.md5(raw).hexdigest()
    for layout in TRACK02_LAYOUTS:
        if len(raw) == layout.size and actual_md5 == layout.md5:
            return layout
    raise ValueError("Track 02 is not an authenticated JP Rev. 1 BIN or MODE1/2048 ISO")


def identify_trace_layout(row: dict[str, int]) -> Track02Layout | None:
    for layout in TRACK02_LAYOUTS:
        if (
            row["track02_start_lba"] == layout.start_lba
            and row["track02_end_lba"] == layout.end_lba
            and row["track02_index1_file_offset"] == layout.index1_file_offset
            and row["track02_sector_bytes"] == layout.sector_bytes
        ):
            return layout
    return None


def parse_row(line: str, line_number: int) -> dict[str, int]:
    match = re.fullmatch(
        r"cd_data_port_read sequence=(\d+) (?:cpu_pc=([0-9a-f]{4}) )?address=([0-9a-f]{4}) "
        r"value=([0-9a-f]{2}) "
        r"(?:reader_pc=([0-9a-f]{4}) reader_physical_pc=([0-9a-f]{6}) )?"
        r"(?:reader_mpr_slot=(\d+) reader_mpr0=([0-9a-f]{2}) reader_mpr1=([0-9a-f]{2}) "
        r"reader_mpr2=([0-9a-f]{2}) reader_mpr3=([0-9a-f]{2}) reader_mpr4=([0-9a-f]{2}) "
        r"reader_mpr5=([0-9a-f]{2}) reader_mpr6=([0-9a-f]{2}) reader_mpr7=([0-9a-f]{2}) )?"
        r"source_valid=([01]) source_lba=(\d+) "
        r"source_user_offset=(\d+) source_raw_offset=(\d+) "
        r"track02_start_lba=(\d+) track02_end_lba=(\d+) "
        r"track02_index1_file_offset=(\d+) track02_sector_bytes=(\d+) "
        r"track02_raw_file_offset=(\d+)"
        r"(?: event_sequence=(\d+))?",
        line,
    )
    if not match:
        raise ValueError(f"malformed data-port row at line {line_number}")
    groups = match.groups()
    source_index = 15
    row = {
        "sequence": int(groups[0]),
        "address": int(groups[2], 16),
        "value": int(groups[3], 16),
        "source_valid": int(groups[source_index]),
        "source_lba": int(groups[source_index + 1]),
        "source_user_offset": int(groups[source_index + 2]),
        "source_raw_offset": int(groups[source_index + 3]),
        "track02_start_lba": int(groups[source_index + 4]),
        "track02_end_lba": int(groups[source_index + 5]),
        "track02_index1_file_offset": int(groups[source_index + 6]),
        "track02_sector_bytes": int(groups[source_index + 7]),
        "track02_raw_file_offset": int(groups[source_index + 8]),
    }
    if groups[-1] is not None:
        row["event_sequence"] = int(groups[-1])
    if groups[1] is not None:
        row["cpu_pc"] = int(groups[1], 16)
    if groups[4] is not None:
        row["reader_pc"] = int(groups[4], 16)
        row["reader_physical_pc"] = int(groups[5], 16)
    if groups[6] is not None:
        row["reader_mpr_slot"] = int(groups[6])
        for index in range(8):
            row[f"reader_mpr{index}"] = int(groups[7 + index], 16)
    return row


def verify(
    trace_path: Path,
    track02_path: Path,
    allow_other_sources: bool = False,
    required_lba: int | None = None,
    required_user_range: tuple[int, int] | None = None,
) -> tuple[bytes, list[dict[str, int]]]:
    raw = track02_path.read_bytes()
    layout = identify_track02(raw)

    lines = trace_path.read_text(encoding="ascii").splitlines()
    reader_pc_trace = bool(lines and lines[0] == TRACE_HEADER_WITH_READER_PC)
    reader_mpr_trace = bool(lines and lines[0] in (
        TRACE_HEADER_WITH_READER_MPR_CONTEXT,
        TRACE_HEADER_WITH_READER_MPR_SHARED_SEQUENCE,
    ))
    shared_sequence_trace = bool(lines and lines[0] == TRACE_HEADER_WITH_READER_MPR_SHARED_SEQUENCE)
    if not lines or lines[0] not in (
        TRACE_HEADER,
        TRACE_HEADER_WITH_READER_PC,
        TRACE_HEADER_WITH_READER_MPR_CONTEXT,
        TRACE_HEADER_WITH_READER_MPR_SHARED_SEQUENCE,
    ):
        raise ValueError("unexpected CD data-port trace header")
    rows = [parse_row(line, index + 2) for index, line in enumerate(lines[1:])]
    if not rows or len(rows) > MAX_OBSERVED_BYTES:
        raise ValueError(
            f"expected 1-{MAX_OBSERVED_BYTES} data-port rows, found {len(rows)}"
        )
    if reader_mpr_trace != all("reader_mpr_slot" in row for row in rows):
        raise ValueError("reader MPR fields do not match the trace header")

    # Pass 1 checks the trace's ordered sequence and source metadata.
    event_sequences = [row["event_sequence"] for row in rows if "event_sequence" in row]
    if shared_sequence_trace and len(event_sequences) != len(rows):
        raise ValueError("shared-sequence header requires an event ID on every CD read")
    if event_sequences and (len(event_sequences) != len(rows) or any(
        current <= previous for previous, current in zip(event_sequences, event_sequences[1:])
    )):
        raise ValueError("shared event sequence is missing or out of order")
    for sequence, row in enumerate(rows):
        if row["sequence"] != sequence or row["address"] != 0x1808:
            raise ValueError(f"unexpected sequence or data-port address at row {sequence}")
        if row["source_valid"] != 1:
            raise ValueError(f"unbound source at row {sequence}")

    # The optional pass checks the traced CPU that performed each data-port read.
    if reader_pc_trace:
        for sequence, row in enumerate(rows):
            if row.get("reader_pc") != 0xEA99 or row.get("reader_physical_pc") != 0x0A99:
                raise ValueError(f"unexpected CD data-port reader PC at row {sequence}")
    elif reader_mpr_trace:
        for sequence, row in enumerate(rows):
            expected_slot = (row["reader_pc"] >> 13) & 7
            expected_physical_pc = (
                (row[f"reader_mpr{expected_slot}"] << 13) | (row["reader_pc"] & 0x1fff)
            )
            if (
                row["reader_pc"] != row.get("cpu_pc")
                or row["reader_mpr_slot"] != expected_slot
                or row["reader_physical_pc"] != expected_physical_pc
            ):
                raise ValueError(f"invalid reader PC to physical PC mapping at row {sequence}")
    else:
        for sequence, row in enumerate(rows):
            if "reader_pc" in row or "reader_mpr_slot" in row:
                raise ValueError(
                    f"reader-PC fields require the reader-PC trace header at row {sequence}"
                )

    # Pass 2 independently recomputes each JP Track 02 row's sector and
    # file offset from the trace's declared CUE layout. The authenticated input
    # may be either the raw BIN or its MODE1/2048 projection.
    # Long instrumented captures can read non-adjacent sectors; validate each
    # observed byte against its own CD source receipt instead of assuming the
    # entire trace is one contiguous 4096-byte bootstrap read.
    trace_layouts: set[str] = set()
    for sequence, row in enumerate(rows):
        trace_layout = identify_trace_layout(row)
        if trace_layout is None:
            if allow_other_sources:
                continue
            raise ValueError(f"unexpected source span at row {sequence}")
        trace_layouts.add(trace_layout.name)
        sector_index = row["source_lba"] - trace_layout.start_lba
        user_offset = row["source_user_offset"]
        if not 0 <= sector_index < trace_layout.end_lba - trace_layout.start_lba:
            if allow_other_sources:
                continue
            raise ValueError(f"source LBA is outside JP Track 02 at row {sequence}")
        if not 0 <= user_offset < trace_layout.user_data_bytes:
            raise ValueError(f"source user offset is outside its sector at row {sequence}")
        expected_trace_offset = (
            trace_layout.index1_file_offset
            + sector_index * trace_layout.sector_bytes
            + trace_layout.file_user_data_offset
            + user_offset
        )
        if (
            row["source_raw_offset"] != trace_layout.source_user_data_offset + user_offset
            or row["track02_raw_file_offset"] != expected_trace_offset
        ):
            raise ValueError(f"unexpected source span at row {sequence}")

    # Pass 3 maps LBA/user offsets into the authenticated representation passed
    # by the caller, independently of the representation used by the trace.
    observed = bytearray()
    for sequence, row in enumerate(rows):
        trace_layout = identify_trace_layout(row)
        if trace_layout is None:
            continue
        if not trace_layout.start_lba <= row["source_lba"] < trace_layout.end_lba:
            continue
        media_offset = (
            layout.index1_file_offset
            + (row["source_lba"] - layout.start_lba) * layout.sector_bytes
            + layout.file_user_data_offset
            + row["source_user_offset"]
        )
        if media_offset >= len(raw) or raw[media_offset] != row["value"]:
            raise ValueError(f"data-port byte differs from authentic media at row {sequence}")
        observed.append(row["value"])

    if required_user_range is not None and required_lba is None:
        raise ValueError("a required user-data range requires --require-lba")
    if required_lba is not None:
        if not layout.start_lba <= required_lba < layout.end_lba:
            raise ValueError("required LBA is outside the authenticated Track 02 span")
        target_rows = [
            row for row in rows
            if row["source_valid"] == 1 and row["source_lba"] == required_lba
        ]
        if not target_rows:
            raise ValueError(f"required source LBA {required_lba} was not observed")
        if required_user_range is not None:
            first, end = required_user_range
            if not 0 <= first < end <= layout.user_data_bytes:
                raise ValueError("required user-data range is outside one sector")
            observed_offsets = {
                row["source_user_offset"] for row in target_rows
                if identify_trace_layout(row) is not None
            }
            missing = [offset for offset in range(first, end) if offset not in observed_offsets]
            if missing:
                raise ValueError(
                    f"LBA {required_lba} is missing {len(missing)} required user-data byte(s)"
                )
            authenticated_offsets = {
                row["source_user_offset"] for row in target_rows
                if identify_trace_layout(row) is not None
                and first <= row["source_user_offset"] < end
            }
            expected_count = end - first
            if len(authenticated_offsets) != expected_count:
                raise ValueError("required source range is not fully authenticated")
    return bytes(observed), rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("track02", type=Path)
    parser.add_argument("--require-lba", type=int)
    parser.add_argument(
        "--require-user-range",
        type=lambda value: tuple(int(part, 0) for part in value.split(":")),
        help="require complete coverage of the half-open user-data range FIRST:END",
    )
    args = parser.parse_args()
    try:
        if args.require_user_range is not None and len(args.require_user_range) != 2:
            raise ValueError("--require-user-range must be FIRST:END")
        observed, rows = verify(
            args.trace,
            args.track02,
            required_lba=args.require_lba,
            required_user_range=args.require_user_range,
        )
    except (OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    loop_count = 5 if "reader_mpr_slot" in rows[0] else 4 if "reader_pc" in rows[0] else 3
    source_spans = 1
    for previous, current in zip(rows, rows[1:]):
        if current["track02_raw_file_offset"] != previous["track02_raw_file_offset"] + 1:
            source_spans += 1
    source_lbas = sorted({row["source_lba"] for row in rows})
    layout = identify_track02(args.track02.read_bytes())
    layout_names = sorted({
        trace_layout.name
        for row in rows
        if (trace_layout := identify_trace_layout(row)) is not None
    })
    print(f"authenticated_media_layout={layout.name} md5={layout.md5}")
    print(f"trace_media_layouts={','.join(layout_names) if layout_names else 'none'}")
    print(
        f"PASS: {loop_count} verification loops; {len(observed)} bytes match "
        "authentic JP Track 02"
    )
    print(
        f"source_lbas={source_lbas[0]}-{source_lbas[-1]} "
        f"unique_lbas={len(source_lbas)} source_spans={source_spans}"
    )
    print(f"observed_sha256={hashlib.sha256(observed).hexdigest()}")
    if "reader_pc" in rows[0]:
        print(f"reader_pc=0x{rows[0]['reader_pc']:04x} physical_pc=0x{rows[0]['reader_physical_pc']:06x}")
    if "reader_mpr_slot" in rows[0]:
        active_mpr = rows[0][f"reader_mpr{rows[0]['reader_mpr_slot']}"]
        print(f"reader_mpr_slot={rows[0]['reader_mpr_slot']} active_mpr=0x{active_mpr:02x}")
    if args.require_lba is not None:
        target_offsets = {
            row["source_user_offset"] for row in rows
            if row["source_valid"] == 1 and row["source_lba"] == args.require_lba
        }
        print(f"required_lba={args.require_lba} observed_user_bytes={len(target_offsets)}")
        if args.require_user_range is not None:
            first, end = args.require_user_range
            print(f"required_user_range={first}:{end} complete=1")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
