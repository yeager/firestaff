#!/usr/bin/env python3
"""Validate SH-2 code windows captured at authentic VDP2 write PCs.

The tool checks exact byte identity against hash-verified retail binaries when
possible. A relocated/decompressed runtime window remains an execution
receipt, not permission to decode a tilemap, CLUT, menu, HUD or viewport.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

from analyze_nexus_title_vdp2_source import cue_track1, iso_members_in_memory


HEADER = "FIRESTAFF_NEXUS_VDP2_WRITER_CODE_TRACE_V1"
LINE = re.compile(
    r"pc=0x(?P<pc>[0-9a-fA-F]+) code_start=0x(?P<start>[0-9a-fA-F]+) "
    r"words=(?P<words>[0-9]+) code=(?P<code>[0-9a-fA-F,]+)$"
)
TM_SHA256 = "d87485fe6eba1f6e9fbbf487f5fcdd994911136905e6172e5bb5bc0122407eb6"
DM_SHA256 = "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"


def read_asset(path: Path, expected: str) -> bytes:
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != expected:
        raise ValueError(f"{path.name} hash mismatch")
    return data


def read_retail_assets(data_dir: Path | None, cue: Path | None) -> dict[str, bytes]:
    """Read the two retail executables without materialising CUE members.

    ``--cue`` is the preferred form for player-supplied Saturn media.  Loose
    files remain useful for a separately mounted, hash-verified research
    corpus, but neither route writes a member to disk.
    """
    if cue is not None:
        assets = iso_members_in_memory(cue_track1(cue), {"TM.BIN", "DM.BIN"})
    elif data_dir is not None:
        assets = {
            "TM.BIN": (data_dir / "TM.BIN").read_bytes(),
            "DM.BIN": (data_dir / "DM.BIN").read_bytes(),
        }
    else:
        raise ValueError("either --data-dir or --cue is required")
    for name, expected in (("TM.BIN", TM_SHA256), ("DM.BIN", DM_SHA256)):
        if hashlib.sha256(assets[name]).hexdigest() != expected:
            raise ValueError(f"{name} hash mismatch")
    return assets


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--data-dir", type=Path,
                        help="directory containing separately mounted retail members")
    source.add_argument("--cue", type=Path,
                        help="retail Saturn CUE; members are read in memory")
    parser.add_argument("--require-pc", action="append", type=lambda value: int(value, 0), default=[])
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
        if not lines or lines[0] != HEADER:
            raise ValueError("bad header")
        assets = read_retail_assets(args.data_dir, args.cue)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP2_WRITER_CODE_INVALID: {error}")
        return 1

    observed: set[int] = set()
    exact: set[str] = set()
    windows = 0
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            print(f"NEXUS_VDP2_WRITER_CODE_INVALID: malformed line {line_number}")
            return 1
        pc = int(match["pc"], 16)
        words = int(match["words"], 10)
        code_words = match["code"].split(",")
        if words != len(code_words) or words == 0:
            print(f"NEXUS_VDP2_WRITER_CODE_INVALID: word count at line {line_number}")
            return 1
        blob = b"".join(int(word, 16).to_bytes(2, "big") for word in code_words)
        observed.add(pc)
        windows += 1
        owner_offsets = {
            name: data.find(blob)
            for name, data in assets.items()
            if data.find(blob) >= 0
        }
        owners = list(owner_offsets)
        if owners:
            exact.update(owners)
        print(
            f"pc=0x{pc:08x} code_start=0x{int(match['start'], 16):08x} "
            f"words={words} exact_owner={','.join(owners) if owners else 'none'} "
            "exact_offsets=" +
            ("|".join(f"{name}:0x{offset:x}" for name, offset in owner_offsets.items())
             if owner_offsets else "none")
        )

    missing = sorted(set(args.require_pc) - observed)
    print(f"windows={windows}")
    print(f"unique_pcs={len(observed)}")
    print(f"exact_retail_owners={','.join(sorted(exact)) if exact else 'none'}")
    print("runtime_code_window=verified")
    print("source_file_identity=verified" if exact else "source_file_identity=unbound")
    print("semantic_admission=blocked")
    if missing:
        print("required_pcs_missing=" + ",".join(f"0x{pc:08x}" for pc in missing))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
