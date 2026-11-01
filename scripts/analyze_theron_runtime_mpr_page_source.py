#!/usr/bin/env python3
"""Search authenticated Theron media for a captured HuC6280 MPR page."""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from dataclasses import dataclass
from pathlib import Path


TRACE_HEADER_V1 = "source=mednafen-pce-fast-theron-c852-runtime-window-v1"
TRACE_HEADER_V2 = (
    "source=mednafen-pce-fast-theron-c852-runtime-window-v2 "
    "event_sequence_source=TheronTraceCDSharedEventSequence"
)
ROW_PREFIX = "theron_c852_instruction "
END_PREFIX = "theron_c852_window_end "
FIELD_RE = re.compile(r"([a-z0-9_]+)=([^ ]+)")
PAGE_BYTES = 0x2000
EXPECTED_DISPATCHES = 129


@dataclass(frozen=True)
class Media:
    label: str
    path: Path
    expected_md5: str


def fail(message: str) -> int:
    print(f"FAIL: {message}", file=sys.stderr)
    return 2


def fields_from(line: str) -> dict[str, str]:
    return dict(FIELD_RE.findall(line))


def read_dispatches(path: Path) -> tuple[list[dict[str, int]], dict[int, set[bytes]]]:
    text = path.read_text(encoding="ascii")
    lines = text.splitlines()
    if not lines or lines[0] not in (TRACE_HEADER_V1, TRACE_HEADER_V2):
        raise ValueError("unrecognized PCE Fast runtime-window trace header")
    if sum(line in (TRACE_HEADER_V1, TRACE_HEADER_V2) for line in lines) != 1:
        raise ValueError("trace must contain exactly one provenance header")
    shared_sequence_trace = lines[0] == TRACE_HEADER_V2

    dispatches: list[dict[str, int]] = []
    page_samples: dict[int, set[bytes]] = {}
    end_records: list[dict[str, str]] = []
    previous_event_sequence = -1
    for line_number, line in enumerate(lines[1:], 2):
        if line.startswith(END_PREFIX):
            end_records.append(fields_from(line[len(END_PREFIX):]))
            continue
        if not line.startswith(ROW_PREFIX):
            continue
        values = fields_from(line[len(ROW_PREFIX):])
        try:
            row = {
                "window": int(values["window"], 10),
                "step": int(values["step"], 10),
                "logical_pc": int(values["logical_pc"], 16),
                "physical_pc": int(values["physical_pc"], 16),
                "opcode": int(values["opcode"], 16),
                "operand1": int(values["operand1"], 16),
                "operand2": int(values["operand2"], 16),
                "mpr0": int(values["mpr0"], 16),
                "mpr1": int(values["mpr1"], 16),
                "mpr2": int(values["mpr2"], 16),
                "mpr3": int(values["mpr3"], 16),
                "mpr4": int(values["mpr4"], 16),
                "mpr5": int(values["mpr5"], 16),
                "mpr6": int(values["mpr6"], 16),
                "mpr7": int(values["mpr7"], 16),
            }
        except (KeyError, ValueError) as exc:
            raise ValueError(f"malformed instruction row on line {line_number}: {exc}") from exc

        if shared_sequence_trace:
            try:
                row["event_sequence"] = int(values["event_sequence"], 10)
            except (KeyError, ValueError) as exc:
                raise ValueError(
                    f"malformed shared event sequence on line {line_number}: {exc}"
                ) from exc
            if row["event_sequence"] <= previous_event_sequence:
                raise ValueError(
                    f"non-increasing shared event sequence on line {line_number}"
                )
            previous_event_sequence = row["event_sequence"]

        pc = row["logical_pc"]
        mpr = row[f"mpr{pc >> 13}"]
        expected_physical_pc = (mpr << 13) | (pc & 0x1FFF)
        if row["window"] != 0:
            raise ValueError(f"unexpected window id on line {line_number}")
        if row["step"] != len(dispatches):
            raise ValueError(f"non-contiguous dispatch order on line {line_number}")
        if any(row[f"mpr{i}"] > 0xFF for i in range(8)):
            raise ValueError(f"MPR outside byte range on line {line_number}")
        if row["physical_pc"] != expected_physical_pc:
            raise ValueError(f"logical/physical PC mismatch on line {line_number}")
        dispatches.append(row)

        if 0x4000 <= pc < 0x6000 and row["mpr2"] == 0x68:
            code = bytes((row["opcode"], row["operand1"], row["operand2"]))
            page_samples.setdefault(pc & 0x1FFF, set()).add(code)

    if len(end_records) != 1:
        raise ValueError("trace must contain exactly one completion record")
    try:
        complete_rows = int(end_records[0]["rows"], 10)
        complete_status = end_records[0]["status"]
    except (KeyError, ValueError) as exc:
        raise ValueError("malformed completion record") from exc
    if complete_status != "complete" or complete_rows != len(dispatches):
        raise ValueError("runtime instruction window is incomplete")
    if len(dispatches) != EXPECTED_DISPATCHES:
        raise ValueError(
            f"expected {EXPECTED_DISPATCHES} dispatches, got {len(dispatches)}"
        )
    if not page_samples:
        raise ValueError("no instructions map through MPR2=$68")

    call_seen = False
    target_seen = False
    for index, row in enumerate(dispatches[:-1]):
        if (row["logical_pc"] == 0xC85E and row["opcode"] == 0x20 and
                row["operand1"] == 0x39 and row["operand2"] == 0x50):
            next_row = dispatches[index + 1]
            if next_row["logical_pc"] == 0x5039 and row["mpr2"] == next_row["mpr2"] == 0x68:
                call_seen = True
                target_seen = True
                break
    if not call_seen or not target_seen:
        raise ValueError("ordered $C85E JSR $5039 target dispatch is absent")
    return dispatches, page_samples


def matching_page_bases(source: bytes, samples: dict[int, set[bytes]]) -> list[int]:
    anchors = [
        (code, offset)
        for offset, variants in samples.items()
        for code in variants
    ]
    if not anchors:
        return []
    seed_code, seed_offset = min(
        anchors,
        key=lambda item: source.count(item[0]),
    )
    candidates: list[int] = []
    position = source.find(seed_code)
    while position >= 0:
        base = position - seed_offset
        if base >= 0 and base + PAGE_BYTES <= len(source):
            candidates.append(base)
        position = source.find(seed_code, position + 1)

    matches: list[int] = []
    for base in candidates:
        if all(
            source[base + offset:base + offset + 3] in variants
            for offset, variants in samples.items()
        ):
            matches.append(base)
    return sorted(set(matches))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("runtime_window", type=Path)
    parser.add_argument(
        "--us-track02", type=Path,
        help="authentic US Track 02 raw MODE1/2352 BIN",
    )
    parser.add_argument(
        "--us-track19", type=Path,
        help="authentic US Track 19 raw MODE1/2352 BIN",
    )
    args = parser.parse_args()
    if not args.us_track02 and not args.us_track19:
        return fail("provide at least one identity-checked US source BIN")

    try:
        dispatches, samples = read_dispatches(args.runtime_window)
    except (OSError, UnicodeError, ValueError) as exc:
        return fail(str(exc))

    print(
        f"trace=complete dispatches={len(dispatches)} "
        f"mpr2=$68_unique_logical_pcs={len(samples)} "
        f"shared_event_sequence={'verified' if 'event_sequence' in dispatches[0] else 'absent'}"
    )
    if 0x1039 in samples:
        for value in sorted(samples[0x1039]):
            print(f"logical_pc=$5039 live_bytes={value.hex()}")

    media_inputs = (
        Media("US Track 02 raw MODE1/2352", args.us_track02,
              "f23601102138f87c33025877767ebf76"),
        Media("US Track 19 raw MODE1/2352", args.us_track19,
              "fd45d13690a214b17b48a6b7c05b93b4"),
    )
    for media in media_inputs:
        if media.path is None:
            continue
        try:
            source = media.path.read_bytes()
        except OSError as exc:
            return fail(f"cannot read {media.label}: {exc}")
        actual_md5 = hashlib.md5(source).hexdigest()
        if actual_md5 != media.expected_md5:
            return fail(
                f"{media.label} identity mismatch: expected "
                f"{media.expected_md5}, got {actual_md5}"
            )
        matches = matching_page_bases(source, samples)
        rendered = ",".join(f"0x{offset:07x}" for offset in matches) or "none"
        print(
            f"source={media.label} md5={actual_md5} "
            f"contiguous_8k_page_bases={rendered}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
