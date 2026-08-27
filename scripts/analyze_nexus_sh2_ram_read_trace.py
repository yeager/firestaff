#!/usr/bin/env python3
"""Fail-closed validator for an external Nexus SH-2 WorkRAMH read receipt.

This tool validates an observed controller-buffer transport/consumer chain.
It deliberately does not infer a menu choice, new-game action, or party pose.
"""

import argparse
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_SH2_RAM_READ_TRACE_V1"
FIELDS = (
    "frame", "addr", "size", "value", "pc0", "pc1", "r0", "r1", "r2",
    "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
    "r13", "r14", "r15", "pr",
)
ROW = re.compile(r"(?:^|\s)([a-z0-9]+)=(0x[0-9a-fA-F]+|[0-9]+)")


def fail(message: str) -> None:
    raise SystemExit(f"NEXUS_SH2_RAM_READ_TRACE_INVALID: {message}")


def parse(path: Path):
    try:
        lines = path.read_text(encoding="ascii").splitlines()
    except OSError as exc:
        fail(f"cannot read {path}: {exc}")
    if not lines or lines[0] != HEADER:
        fail("missing or unsupported header")
    if len(lines) == 1:
        fail("no read rows")

    rows = []
    for line_no, line in enumerate(lines[1:], 2):
        values = {key: int(value, 0) for key, value in ROW.findall(line)}
        missing = [field for field in FIELDS if field not in values]
        if missing:
            fail(f"line {line_no}: missing " + ",".join(missing))
        if values["size"] not in (1, 2, 4):
            fail(f"line {line_no}: invalid read width")
        if not 0x06000000 <= values["addr"] < 0x06100000:
            fail(f"line {line_no}: address is outside WorkRAMH")
        if values["pc1"] != 0:
            fail(f"line {line_no}: unexpected slave-SH-2 reader")
        rows.append(values)
    return rows


def require(rows, description, predicate) -> None:
    if not any(predicate(row) for row in rows):
        fail(f"missing {description}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    args = parser.parse_args()
    rows = parse(args.trace)
    frames = {row["frame"] for row in rows}
    if len(frames) != 1:
        fail("receipt spans multiple emulation frames")

    # Linked-list transfer at the first consumer; addresses and registers are
    # taken only from a same-session observed retail JP trace.
    require(rows, "first buffer consumer", lambda r:
            r["pc0"] == 0x06014388 and r["addr"] == 0x0602C90C and
            r["r4"] == 0x0602C908 and r["r3"] == 0x0602C8F8)
    require(rows, "linked buffer consumer", lambda r:
            r["pc0"] == 0x0601439A and r["addr"] == 0x0602C91C and
            r["r4"] == 0x0602C91C and r["r1"] == 0x0602C900)
    require(rows, "SMPC byte-copy reader", lambda r:
            r["pc0"] == 0x06014510 and r["addr"] == 0x0602C90C and
            r["r0"] == 0x20100021 and r["r6"] == 0x0602C908)
    require(rows, "nibble-normalization reader", lambda r:
            r["pc0"] == 0x0601457E and r["addr"] == 0x0602C91C and
            r["r0"] == 0x0602C918)
    require(rows, "post-normalization table reader", lambda r:
            r["pc0"] == 0x0601462C and r["addr"] == 0x0602C940 and
            r["r4"] == 0x10)

    print(f"rows={len(rows)}")
    print(f"frame={next(iter(frames))}")
    print("workram_input_consumer_chain=verified")
    print("input_consumer_semantics=unbound")
    print("semantic_admission=blocked")


if __name__ == "__main__":
    main()
