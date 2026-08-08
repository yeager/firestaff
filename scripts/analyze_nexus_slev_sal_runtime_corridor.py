#!/usr/bin/env python3
"""Correlate an authenticated Nexus SCSP trace with opaque retail sound data.

This is a provenance receipt only.  MAP bytes and SAL windows are reported as
raw observations; the tool deliberately does not assign event meaning, decode
SAL payloads, or authorize host playback.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import re
from pathlib import Path

from fixtures.nexus_v1_disc_file_hashes import DISC_HASH


SCSP_HEADER = "FIRESTAFF_NEXUS_SCSP_WRITE_TRACE_V1"
SCSP_LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc=0x(?P<pc>[0-9a-fA-F]+)$"
)
MAIN_HEADER = "FIRESTAFF_NEXUS_MAIN_SCSP_WRITE_TRACE_V1"
MAIN_LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc0=0x(?P<pc0>[0-9a-fA-F]+) "
    r"pc1=0x(?P<pc1>[0-9a-fA-F]+)$"
)


def read_trace(path: Path, main: bool) -> list[dict[str, int]]:
    lines = path.read_text(encoding="ascii").splitlines()
    header = MAIN_HEADER if main else SCSP_HEADER
    pattern = MAIN_LINE if main else SCSP_LINE
    if not lines or lines[0] != header:
        raise ValueError(f"{path}: bad trace header")
    rows = []
    for number, line in enumerate(lines[1:], 2):
        match = pattern.fullmatch(line)
        if not match:
            raise ValueError(f"{path}: malformed line {number}")
        rows.append({key: int(value, 16) for key, value in match.groupdict().items()})
    return rows


def retail_hash(path: Path, name: str) -> bool:
    expected = DISC_HASH[name]
    return hashlib.sha256(path.read_bytes()).hexdigest() == expected


def map_rows(map_data: bytes, sal_size: int) -> tuple[list[dict[str, int]], int]:
    """Apply the DMWeb retail eight-byte MAP grammar used by the C loader."""
    rows = []
    offset = 0
    terminator = -1
    while offset + 1 < len(map_data):
        if map_data[offset:offset + 2] == b"\xff\xff":
            terminator = offset
            break
        if offset + 8 > len(map_data):
            raise ValueError(f"MAP record truncated at 0x{offset:x}")
        record = map_data[offset:offset + 8]
        sal_offset = int.from_bytes(record[1:4], "big")
        size = int.from_bytes(record[5:8], "big")
        rows.append({
            "offset": offset,
            "event_byte": record[0],
            "data_id": (record[0] >> 4) & 0x07,
            "attribute": record[0] & 0x0f,
            "sal_offset": sal_offset,
            "sal_size": size,
            "in_bounds": int(size > 0 and sal_offset <= sal_size and size <= sal_size - sal_offset),
        })
        offset += 8
    return rows, terminator


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("scsp_trace", type=Path)
    parser.add_argument("--main-trace", type=Path, required=True)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--driver", type=Path, required=True)
    parser.add_argument("--mailbox", type=lambda value: int(value, 0), default=0x100400)
    args = parser.parse_args()

    try:
        scsp = read_trace(args.scsp_trace, False)
        main_trace = read_trace(args.main_trace, True)
        required = ["SDDRVS.TSK"]
        required += [f"SLEV{level:02d}.BIN" for level in range(16)]
        required += [f"SNDLEV{level:02d}.{suffix}" for level in range(16) for suffix in ("MAP", "SAL")]
        invalid = []
        files = {}
        for name in required:
            path = args.driver if name == "SDDRVS.TSK" else args.data_dir / name
            files[name] = path
            if not path.is_file() or not retail_hash(path, name):
                invalid.append(name)
        if invalid:
            print("NEXUS_SLEV_SAL_RUNTIME_INVALID: " + ",".join(invalid))
            return 1
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"NEXUS_SLEV_SAL_RUNTIME_INVALID: {error}")
        return 1

    maps = []
    total_rows = 0
    direct_file_oob = 0
    for level in range(16):
        map_name = f"SNDLEV{level:02d}.MAP"
        sal_name = f"SNDLEV{level:02d}.SAL"
        rows, terminator = map_rows(files[map_name].read_bytes(), files[sal_name].stat().st_size)
        maps.append((level, rows, terminator))
        total_rows += len(rows)
        direct_file_oob += sum(not row["in_bounds"] for row in rows)

    mailbox = [row for row in scsp if row["addr"] == args.mailbox and row["value"]]
    main_mailbox = [row for row in main_trace if row["addr"] == args.mailbox and row["value"]]
    values = collections.Counter(row["value"] for row in mailbox)
    pcs = collections.Counter(row["pc"] for row in mailbox)
    main_values = collections.Counter(row["value"] for row in main_mailbox)
    print("source_hashes_verified=1")
    print("slev_files_verified=16")
    print("map_files_verified=16")
    print("sal_files_verified=16")
    print(f"driver_sha256={hashlib.sha256(args.driver.read_bytes()).hexdigest()}")
    event_bytes = collections.Counter(
        row["event_byte"] for _, rows, _ in maps for row in rows
    )
    data_ids = collections.Counter(row["data_id"] for _, rows, _ in maps for row in rows)
    print(f"map_rows={total_rows} map_direct_file_interval_oob={direct_file_oob}")
    print("map_offsets_are_opaque_driver_area=1")
    print("map_terminators=" + ",".join(f"{level}:{terminator}" for level, _, terminator in maps))
    print("map_event_byte_counts=" + ",".join(f"0x{value:02x}:{count}" for value, count in sorted(event_bytes.items())))
    print("map_raw_data_id_counts=" + ",".join(f"{value}:{count}" for value, count in sorted(data_ids.items())))
    print(f"scsp_records={len(scsp)} mailbox_nonzero_records={len(mailbox)}")
    print("scsp_mailbox_values=" + ",".join(f"0x{value:02x}:{count}" for value, count in values.most_common()))
    print("scsp_mailbox_pcs=" + ",".join(f"0x{pc:04x}:{count}" for pc, count in pcs.most_common()))
    print("scsp_mailbox_sequence=" + ",".join(
        f"0x{row['addr']:06x}/0x{row['value']:x}/0x{row['pc']:04x}"
        for row in scsp if row["addr"] in (args.mailbox, args.mailbox + 1,
                                            args.mailbox + 2) and row["value"]
    ))
    print(f"main_records={len(main_trace)} main_mailbox_nonzero_records={len(main_mailbox)}")
    print("main_mailbox_values=" + ",".join(f"0x{value:04x}:{count}" for value, count in main_values.most_common()))
    print("main_mailbox_sequence=" + ",".join(
        f"0x{row['addr']:08x}/0x{row['value']:x}/0x{row['pc0']:08x}/0x{row['pc1']:08x}"
        for row in main_trace if row["addr"] in (args.mailbox, args.mailbox + 1,
                                                  args.mailbox + 2) and row["value"]
    ))
    print("event_selector_semantics=unproven")
    print("sal_codec=unproven")
    print("host_playback=blocked")
    if not mailbox or not main_mailbox:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
