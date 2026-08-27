#!/usr/bin/env python3
"""Bind observed Nexus SMPC reads to the hash-verified retail SH-2 range.

The receipt proves that a captured SMPC return byte was observed while the
master SH-2 PC snapshot lay at a specific word of retail DM.BIN. A snapshot
does not identify the instruction being retired; it may point at a literal
pool or be sampled asynchronously. The receipt therefore does not infer an
instruction, input event meaning, menu action, or rendering consumer.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from collections import Counter
from pathlib import Path

from analyze_nexus_title_vdp2_source import cue_track1, iso_members_in_memory


HEADER = "FIRESTAFF_NEXUS_SMPC_READ_TRACE_V1"
LINE = re.compile(
    r"^frame=(?P<frame>[0-9]+) smpc=0x(?P<smpc>[0-9a-fA-F]{2}) "
    r"value=0x(?P<value>[0-9a-fA-F]{2}) pc0=0x(?P<pc0>[0-9a-fA-F]{8}) "
    r"pc1=0x(?P<pc1>[0-9a-fA-F]{8})$")
DM_SHA256 = "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"
DM_BASE = 0x06010040


def dm_word(dm: bytes, pc: int) -> int:
    offset = pc - DM_BASE
    if offset < 0 or offset + 2 > len(dm):
        raise ValueError(f"master SH-2 PC snapshot is outside retail DM.BIN: 0x{pc:08x}")
    return int.from_bytes(dm[offset:offset + 2], "big")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--cue", required=True, type=Path,
                        help="retail Nexus CUE; DM.BIN remains in memory")
    parser.add_argument("--frame-min", type=int)
    parser.add_argument("--frame-max", type=int)
    args = parser.parse_args()
    if ((args.frame_min is not None and args.frame_min < 0) or
            (args.frame_max is not None and args.frame_max < 0) or
            (args.frame_min is not None and args.frame_max is not None and
             args.frame_min > args.frame_max)):
        print("NEXUS_SMPC_READ_TRACE_INVALID: invalid frame bounds")
        return 1
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
        if not lines or lines[0] != HEADER:
            raise ValueError("bad header")
        dm = iso_members_in_memory(cue_track1(args.cue), {"DM.BIN"})["DM.BIN"]
        if hashlib.sha256(dm).hexdigest() != DM_SHA256:
            raise ValueError("DM.BIN hash mismatch")
        rows = []
        for number, line in enumerate(lines[1:], 2):
            match = LINE.fullmatch(line)
            if not match:
                raise ValueError(f"malformed row {number}")
            frame = int(match["frame"], 10)
            if ((args.frame_min is not None and frame < args.frame_min) or
                    (args.frame_max is not None and frame > args.frame_max)):
                continue
            rows.append((frame, int(match["smpc"], 16), int(match["value"], 16),
                         int(match["pc0"], 16), int(match["pc1"], 16)))
        if not rows:
            raise ValueError("no rows remain inside requested frame bounds")
        pc_words = {pc: dm_word(dm, pc) for _frame, _smpc, _value, pc, _pc1 in rows}
    except (KeyError, OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_SMPC_READ_TRACE_INVALID: {error}")
        return 1

    addresses = Counter(smpc for _frame, smpc, _value, _pc0, _pc1 in rows)
    values = Counter(value for _frame, _smpc, value, _pc0, _pc1 in rows)
    print(f"dm_bin_sha256={DM_SHA256}")
    print(f"rows={len(rows)}")
    print(f"frames={min(row[0] for row in rows)}-{max(row[0] for row in rows)}")
    print("smpc_addresses=" + ",".join(
        f"0x{address:02x}:{addresses[address]}" for address in sorted(addresses)))
    print("return_values=" + ",".join(
        f"0x{value:02x}:{values[value]}" for value in sorted(values)))
    for pc in sorted(pc_words):
        print(f"master_pc=0x{pc:08x} dm_offset=0x{pc - DM_BASE:06x} "
              f"dm_word=0x{pc_words[pc]:04x}")
    print("master_sh2_pc_range=verified")
    print("master_sh2_instruction_identity=unproven")
    print("input_consumer_semantics=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
