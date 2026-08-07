#!/usr/bin/env python3
"""Summarize source-addressed Nexus SCSP mailbox and 68K writes.

The trace is runtime evidence only. It does not infer a SAL codec, event
selector, or authorize host playback without the corresponding source bind.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import re
from pathlib import Path


SCSP_HEADER = "FIRESTAFF_NEXUS_SCSP_WRITE_TRACE_V1"
MAIN_HEADER = "FIRESTAFF_NEXUS_MAIN_SCSP_WRITE_TRACE_V1"
SCSP_LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc=0x(?P<pc>[0-9a-fA-F]+)$"
)
MAIN_LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc0=0x(?P<pc0>[0-9a-fA-F]+) "
    r"pc1=0x(?P<pc1>[0-9a-fA-F]+)$"
)


def read_trace(path: Path, main: bool) -> list[dict[str, int]]:
    lines = path.read_text(encoding="ascii").splitlines()
    expected = MAIN_HEADER if main else SCSP_HEADER
    pattern = MAIN_LINE if main else SCSP_LINE
    if not lines or lines[0] != expected:
        raise ValueError(f"{path}: bad trace header")
    rows = []
    for number, line in enumerate(lines[1:], 2):
        match = pattern.fullmatch(line)
        if not match:
            raise ValueError(f"{path}: malformed line {number}")
        rows.append({key: int(value, 16) for key, value in match.groupdict().items()})
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("scsp_trace", type=Path)
    parser.add_argument("--main-trace", type=Path)
    parser.add_argument("--driver", type=Path)
    parser.add_argument("--driver-load-base", type=lambda value: int(value, 0), default=0x1000)
    parser.add_argument("--mailbox", type=lambda value: int(value, 0), default=0x100400)
    parser.add_argument("--require-mailbox", action="store_true")
    args = parser.parse_args()
    try:
        rows = read_trace(args.scsp_trace, False)
        main_rows = read_trace(args.main_trace, True) if args.main_trace else []
        driver_hash = hashlib.sha256(args.driver.read_bytes()).hexdigest() if args.driver else None
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_SCSP_WRITE_TRACE_INVALID: {error}")
        return 1

    mailbox = [row for row in rows if row["addr"] == args.mailbox and row["value"]]
    main_mailbox = [row for row in main_rows if row["addr"] == args.mailbox and row["value"]]
    pcs = collections.Counter(row["pc"] for row in mailbox)
    main_pcs = collections.Counter(row["pc0"] for row in main_mailbox)
    print(f"scsp_records={len(rows)}")
    print(f"mailbox=0x{args.mailbox:06x} scsp_nonzero_records={len(mailbox)}")
    print("scsp_pc_counts=" + ",".join(f"0x{pc:08x}:{count}" for pc, count in pcs.most_common()))
    if args.driver:
        offsets = []
        driver_size = args.driver.stat().st_size
        for pc in sorted(pcs):
            offset = pc - args.driver_load_base
            offsets.append(
                f"0x{pc:08x}->0x{offset:06x}:{int(0 <= offset < driver_size)}"
            )
        print("scsp_pc_driver_offsets=" + ",".join(offsets))
    print(f"main_records={len(main_rows)} main_mailbox_nonzero_records={len(main_mailbox)}")
    print("main_pc_counts=" + ",".join(f"0x{pc:08x}:{count}" for pc, count in main_pcs.most_common()))
    if driver_hash:
        print(f"driver_sha256={driver_hash}")
    print("semantic_admission=blocked")
    if args.require_mailbox and not mailbox:
        print("required_mailbox=missing")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
