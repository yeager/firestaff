#!/usr/bin/env python3
"""Validate the measured NBG0 copy helper against retail SH-2 code.

This verifier reads ``DM.BIN`` directly from a user-supplied CUE into memory.
It scans the complete fixed-width SH-2 word stream for direct ``BSR`` edges,
then validates the execution receipt for the retail copy loop.  The SH-2 PR
register is a live link register, so it is reported rather than attributed to
a different static BSR: nested helpers may legitimately overwrite it before a
sampled VDP2 store.  It is an execution/provenance receipt only: it does not
identify the display consumer or permit a native title render.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

from analyze_nexus_title_vdp2_source import cue_track1, iso_members_in_memory


DM_SHA256 = "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"
DM_BASE = 0x06010040
COPY_PC = 0x0602312C
COPY_ENTRY = 0x06023112
COPY_RTS = 0x06023144
OUTER_ENTRY = 0x060230C0
CALL_SITE = 0x06022772
RETURN_ADDRESS = CALL_SITE + 4
TITLE_FRAME = 12596
COPY_BYTES = 31616
COPY_WORDS = {
    COPY_ENTRY: 0x2FE6,  # mov.l r14,@-r15
    0x06023120: 0x6154,  # mov.b @r5+,r1
    0x06023128: 0x2410,  # mov.b r1,@r4
    0x0602312C: 0x4E15,  # cmp/pl r14
    COPY_RTS: 0x000B,  # rts
}
HEADER = "FIRESTAFF_NEXUS_VDP2_WRITER_REGISTER_TRACE_V1"
FRAME = re.compile(r"^frame=([0-9]+)\b")
PC = re.compile(r"\bpc=0x([0-9a-fA-F]+)\b")
PR = re.compile(r"\bpr=0x([0-9a-fA-F]+)\b")


def word(data: bytes, pc: int) -> int:
    offset = pc - DM_BASE
    if offset < 0 or offset + 2 > len(data):
        raise ValueError(f"PC outside DM.BIN: 0x{pc:08x}")
    return int.from_bytes(data[offset:offset + 2], "big")


def direct_bsr_edges(data: bytes) -> set[tuple[int, int]]:
    """Decode every aligned word in the authenticated executable.

    SH-2 instructions are fixed at 16 bits. Only opcode ``1011dddddddddddd``
    is a direct BSR; the signed disp12 rule is applied exactly. Scanning the
    whole member avoids treating a nearby hand-picked byte window as a caller.
    """
    edges: set[tuple[int, int]] = set()
    for offset in range(0, len(data) - 1, 2):
        opcode = int.from_bytes(data[offset:offset + 2], "big")
        if opcode >> 12 != 0xB:
            continue
        displacement = opcode & 0x0FFF
        if displacement & 0x0800:
            displacement -= 0x1000
        pc = DM_BASE + offset
        edges.add((pc, pc + 4 + displacement * 2))
    return edges


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cue", required=True, type=Path,
                        help="retail Nexus CUE; DM.BIN remains in memory")
    parser.add_argument("--writer-registers", type=Path,
                        help="V14 writer-register receipt containing pr=")
    parser.add_argument("--static-only", action="store_true",
                        help="verify the full retail SH-2 scan without admitting a call receipt")
    args = parser.parse_args()
    try:
        dm = iso_members_in_memory(cue_track1(args.cue), {"DM.BIN"})["DM.BIN"]
        if hashlib.sha256(dm).hexdigest() != DM_SHA256:
            raise ValueError("DM.BIN hash mismatch")
        for pc, expected in COPY_WORDS.items():
            if word(dm, pc) != expected:
                raise ValueError(f"copy instruction mismatch at 0x{pc:08x}")
        if word(dm, CALL_SITE) >> 12 != 0xB:
            raise ValueError("expected caller is not a BSR")
        edges = direct_bsr_edges(dm)
        if (CALL_SITE, OUTER_ENTRY) not in edges:
            raise ValueError("full DM.BIN BSR scan lacks the outer call edge")
        if args.static_only:
            if args.writer_registers is not None:
                raise ValueError("--static-only cannot be combined with --writer-registers")
        elif args.writer_registers is None:
            raise ValueError("--writer-registers is required unless --static-only is used")
        if not args.static_only:
            lines = args.writer_registers.read_text(encoding="ascii").splitlines()
            if not lines or lines[0] != HEADER:
                raise ValueError("bad writer-register trace header")
            prs = []
            for line in lines[1:]:
                frame = FRAME.search(line)
                pc = PC.search(line)
                if (not frame or int(frame.group(1), 10) != TITLE_FRAME or
                        not pc or int(pc.group(1), 16) != COPY_PC):
                    continue
                pr = PR.search(line)
                if not pr:
                    raise ValueError("copy row has no PR return-address witness")
                prs.append(int(pr.group(1), 16))
            if not prs:
                raise ValueError("writer-register trace has no title-copy rows")
            if len(prs) != COPY_BYTES:
                raise ValueError(
                    f"expected {COPY_BYTES} frame-{TITLE_FRAME} copy rows, got {len(prs)}")
            if len(set(prs)) != 1:
                got = ",".join(f"0x{value:08x}" for value in sorted(set(prs)))
                raise ValueError(f"copy rows have inconsistent live PR values: {got}")
    except (KeyError, OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_COPY_CALLCHAIN_INVALID: {error}")
        return 1

    print(f"dm_bin_sha256={DM_SHA256}")
    print(f"sh2_words_scanned={len(dm) // 2}")
    print(f"title_nbg0_copy_pc=0x{COPY_PC:08x}")
    print(f"title_nbg0_copy_frame={TITLE_FRAME}")
    print(f"title_nbg0_copy_rows={COPY_BYTES}")
    print(f"title_nbg0_copy_entry=0x{COPY_ENTRY:08x}")
    print(f"title_nbg0_copy_rts=0x{COPY_RTS:08x}")
    print(f"title_nbg0_outer_bsr=0x{CALL_SITE:08x}")
    print(f"title_nbg0_outer_bsr_pr=0x{RETURN_ADDRESS:08x}")
    print("title_nbg0_copy_callchain=static-verified" if args.static_only
          else "title_nbg0_copy_callchain=verified")
    if not args.static_only:
        print(f"title_nbg0_copy_live_pr=0x{prs[0]:08x}")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
