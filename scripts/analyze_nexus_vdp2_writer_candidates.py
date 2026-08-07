#!/usr/bin/env python3
"""Report bounded retail-byte candidates for an authentic VDP2 code window.

The runtime window may be relocated or decompressed before execution.  A
partial byte match is therefore only a review lead; this tool never promotes
an asset as the VDP2 tilemap, CLUT, menu, HUD, or viewport owner.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path


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


def longest_run(needle: bytes, haystack: bytes) -> tuple[int, int, int]:
    """Return (word_count, runtime_word, asset_byte_offset)."""
    needle_words = [needle[index:index + 2] for index in range(0, len(needle), 2)]
    hay_words = [haystack[index:index + 2] for index in range(0, len(haystack) - 1, 2)]
    positions: dict[bytes, list[int]] = {}
    for index, word in enumerate(hay_words):
        positions.setdefault(word, []).append(index)
    best = (0, 0, 0)
    for runtime_index, word in enumerate(needle_words):
        for asset_index in positions.get(word, []):
            run = 0
            while (
                runtime_index + run < len(needle_words)
                and asset_index + run < len(hay_words)
                and needle_words[runtime_index + run] == hay_words[asset_index + run]
            ):
                run += 1
            candidate = (run, runtime_index, asset_index * 2)
            if candidate[0] > best[0]:
                best = candidate
    return best


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--require-pc", action="append", type=lambda value: int(value, 0), default=[])
    args = parser.parse_args()
    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
        if not lines or lines[0] != HEADER:
            raise ValueError("bad header")
        assets = {
            "TM.BIN": read_asset(args.data_dir / "TM.BIN", TM_SHA256),
            "DM.BIN": read_asset(args.data_dir / "DM.BIN", DM_SHA256),
        }
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP2_WRITER_CANDIDATE_INVALID: {error}")
        return 1

    observed: set[int] = set()
    windows = 0
    for line_number, line in enumerate(lines[1:], 2):
        match = LINE.fullmatch(line)
        if not match:
            print(f"NEXUS_VDP2_WRITER_CANDIDATE_INVALID: malformed line {line_number}")
            return 1
        code_words = match["code"].split(",")
        if int(match["words"], 10) != len(code_words) or not code_words:
            print(f"NEXUS_VDP2_WRITER_CANDIDATE_INVALID: word count at line {line_number}")
            return 1
        blob = b"".join(int(word, 16).to_bytes(2, "big") for word in code_words)
        pc = int(match["pc"], 16)
        observed.add(pc)
        windows += 1
        print(f"pc=0x{pc:08x}")
        for name, data in assets.items():
            run, runtime_word, asset_offset = longest_run(blob, data)
            print(
                f"  {name}: longest_words={run} runtime_word={runtime_word} "
                f"asset_byte=0x{asset_offset:x}"
            )

    missing = sorted(set(args.require_pc) - observed)
    print(f"windows={windows}")
    print(f"unique_pcs={len(observed)}")
    print("partial_matches=review_only")
    print("source_file_identity=unbound")
    print("semantic_admission=blocked")
    if missing:
        print("required_pcs_missing=" + ",".join(f"0x{pc:08x}" for pc in missing))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
