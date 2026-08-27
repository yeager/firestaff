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


HEADER_V1 = "FIRESTAFF_NEXUS_SMPC_READ_TRACE_V1"
HEADER_V2 = "FIRESTAFF_NEXUS_SMPC_READ_TRACE_V2"
LINE_V1 = re.compile(
    r"^frame=(?P<frame>[0-9]+) smpc=0x(?P<smpc>[0-9a-fA-F]{2}) "
    r"value=0x(?P<value>[0-9a-fA-F]{2}) pc0=0x(?P<pc0>[0-9a-fA-F]{8}) "
    r"pc1=0x(?P<pc1>[0-9a-fA-F]{8})$")
LINE_V2 = re.compile(
    r"^frame=(?P<frame>[0-9]+) cpu=(?P<cpu>[01]) "
    r"smpc=0x(?P<smpc>[0-9a-fA-F]{2}) "
    r"value=0x(?P<value>[0-9a-fA-F]{2}) "
    r"pc_id=0x(?P<pc_id>[0-9a-fA-F]{8}) "
    r"pid=0x(?P<pid>[0-9a-fA-F]{4}) "
    r"pc_if=0x(?P<pc_if>[0-9a-fA-F]{8}) "
    r"pif=0x(?P<pif>[0-9a-fA-F]{4}) "
    r"rpc=0x(?P<rpc>[0-9a-fA-F]{8})$")
DM_SHA256 = "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"
DM_BASE = 0x06010040
SNAPSHOT_HEADER = b"FIRESTAFF_NEXUS_SH2_MEMORY_SNAPSHOT_V1\n"
SNAPSHOT_ROW = re.compile(
    rb"frame=(?P<frame>[0-9]+) base=0x(?P<base>[0-9a-fA-F]+) size=(?P<size>[0-9]+)\n")


def dm_word(dm: bytes, pc: int) -> int:
    offset = pc - DM_BASE
    if offset < 0 or offset + 2 > len(dm):
        raise ValueError(f"master SH-2 PC snapshot is outside retail DM.BIN: 0x{pc:08x}")
    return int.from_bytes(dm[offset:offset + 2], "big")


def snapshot_frames(path: Path) -> dict[int, tuple[int, bytes]]:
    stream = path.read_bytes()
    if not stream.startswith(SNAPSHOT_HEADER):
        raise ValueError("invalid SH-2 snapshot header")
    cursor = len(SNAPSHOT_HEADER)
    frames = {}
    while cursor < len(stream):
        line_end = stream.find(b"\n", cursor)
        if line_end < 0:
            raise ValueError("truncated SH-2 snapshot frame header")
        match = SNAPSHOT_ROW.fullmatch(stream[cursor:line_end + 1])
        if not match:
            raise ValueError("malformed SH-2 snapshot frame header")
        frame = int(match["frame"])
        base = int(match["base"], 16)
        size = int(match["size"])
        cursor = line_end + 1
        payload = stream[cursor:cursor + size]
        if (size != 0x100000 or len(payload) != size or cursor + size >= len(stream) or
                stream[cursor + size] != 10 or frame in frames):
            raise ValueError("invalid SH-2 snapshot payload")
        frames[frame] = (base, payload)
        cursor += size + 1
    if not frames:
        raise ValueError("SH-2 snapshot contains no frames")
    return frames


def snapshot_word(frames: dict[int, tuple[int, bytes]], frame: int, pc: int) -> int:
    if frame not in frames:
        raise ValueError(f"SH-2 snapshot frame is absent: {frame}")
    base, payload = frames[frame]
    offset = pc - base
    if offset < 0 or offset + 2 > len(payload):
        raise ValueError(f"pipeline PC lies outside SH-2 snapshot: 0x{pc:08x}")
    return int.from_bytes(payload[offset:offset + 2], "big")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--cue", required=True, type=Path,
                        help="retail Nexus CUE; DM.BIN remains in memory")
    parser.add_argument("--frame-min", type=int)
    parser.add_argument("--frame-max", type=int)
    parser.add_argument("--ram-snapshot", type=Path,
                        help="same-session WorkRAMH snapshot for relocated pipeline opcodes")
    args = parser.parse_args()
    if ((args.frame_min is not None and args.frame_min < 0) or
            (args.frame_max is not None and args.frame_max < 0) or
            (args.frame_min is not None and args.frame_max is not None and
             args.frame_min > args.frame_max)):
        print("NEXUS_SMPC_READ_TRACE_INVALID: invalid frame bounds")
        return 1
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
        if not lines or lines[0] not in (HEADER_V1, HEADER_V2):
            raise ValueError("bad header")
        dm = iso_members_in_memory(cue_track1(args.cue), {"DM.BIN"})["DM.BIN"]
        if hashlib.sha256(dm).hexdigest() != DM_SHA256:
            raise ValueError("DM.BIN hash mismatch")
        v2 = lines[0] == HEADER_V2
        rows = []
        for number, line in enumerate(lines[1:], 2):
            match = (LINE_V2 if v2 else LINE_V1).fullmatch(line)
            if not match:
                raise ValueError(f"malformed row {number}")
            frame = int(match["frame"], 10)
            if ((args.frame_min is not None and frame < args.frame_min) or
                    (args.frame_max is not None and frame > args.frame_max)):
                continue
            if v2:
                rows.append((frame, int(match["cpu"], 10), int(match["smpc"], 16),
                             int(match["value"], 16), int(match["rpc"], 16),
                             int(match["pc_id"], 16), int(match["pid"], 16)))
            else:
                rows.append((frame, 0, int(match["smpc"], 16), int(match["value"], 16),
                             int(match["pc0"], 16), None, None))
        if not rows:
            raise ValueError("no rows remain inside requested frame bounds")
        master_rows = [row for row in rows if row[1] == 0]
        if not master_rows:
            raise ValueError("no master-SH-2 rows remain")
        pipeline_bound = (v2 and all(pc_id != 0
                                      for _frame, _cpu, _smpc, _value, _rpc, pc_id, _pid
                                      in master_rows))
        relocated_pipeline = pipeline_bound and args.ram_snapshot is not None
        pc_words = ({} if relocated_pipeline else
                    {rpc: dm_word(dm, rpc)
                     for _frame, _cpu, _smpc, _value, rpc, _pc_id, _pid in master_rows})
        if pipeline_bound:
            if relocated_pipeline:
                ram = snapshot_frames(args.ram_snapshot)
                if any(pid != snapshot_word(ram, frame, pc_id)
                       for frame, _cpu, _smpc, _value, _rpc, pc_id, pid in master_rows):
                    raise ValueError("master SH-2 pipeline opcode does not match WorkRAMH snapshot")
            elif any(pid != dm_word(dm, pc_id)
                     for _frame, _cpu, _smpc, _value, _rpc, pc_id, pid in master_rows):
                raise ValueError("master SH-2 pipeline opcode does not match retail DM.BIN")
    except (KeyError, OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_SMPC_READ_TRACE_INVALID: {error}")
        return 1

    addresses = Counter(smpc for _frame, _cpu, smpc, _value, _rpc, _pc_id, _pid in rows)
    values = Counter(value for _frame, _cpu, _smpc, value, _rpc, _pc_id, _pid in rows)
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
    if v2 and not pipeline_bound:
        print("master_sh2_instruction_identity=pipeline_unavailable")
    elif v2 and args.ram_snapshot is not None:
        print("master_sh2_instruction_identity=runtime_snapshot_verified")
    else:
        print("master_sh2_instruction_identity=" + ("verified" if v2 else "unproven"))
    print("input_consumer_semantics=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
