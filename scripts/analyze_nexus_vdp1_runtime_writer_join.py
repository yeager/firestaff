#!/usr/bin/env python3
"""Join an authenticated VDP1 writer code window to retail Saturn binaries.

The SH-2 PC is a runtime observation.  This tool tests an explicitly supplied
load base and also searches the authenticated DM.BIN/TM.BIN byte streams for
the captured instruction words.  Partial matches are review leads only:
relocation, decompression, and source ownership are not inferred from them.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path


HEADER = "FIRESTAFF_NEXUS_VDP1_WRITER_CODE_TRACE_V1"
LINE = re.compile(
    r"pc=0x(?P<pc>[0-9a-fA-F]+) vram_addr=0x(?P<vram>[0-9a-fA-F]+) "
    r"code_start=0x(?P<start>[0-9a-fA-F]+) words=(?P<words>[0-9]+) "
    r"code=(?P<code>[0-9a-fA-F,]+)$"
)
EXPECTED = {
    "DM.BIN": ("3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124", 555144),
    "TM.BIN": ("d87485fe6eba1f6e9fbbf487f5fcdd994911136905e6172e5bb5bc0122407eb6", 160044),
}


def longest_run(words: list[int], blob: bytes) -> tuple[int, int]:
    best_length = 0
    best_offset = -1
    for offset in range(0, len(blob) - 1, 2):
        length = 0
        while (length < len(words) and
               offset + (length + 1) * 2 <= len(blob) and
               int.from_bytes(blob[offset + length * 2:offset + length * 2 + 2], "big") == words[length]):
            length += 1
        if length > best_length:
            best_length, best_offset = length, offset
    return best_length, best_offset


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--runtime-base", type=lambda value: int(value, 0))
    parser.add_argument("--require-pc", type=lambda value: int(value, 0))
    parser.add_argument("--require-vram", type=lambda value: int(value, 0))
    args = parser.parse_args()

    try:
        lines = args.trace.read_text(encoding="ascii").splitlines()
        if len(lines) != 2 or lines[0] != HEADER:
            raise ValueError("expected one VDP1 writer code window")
        match = LINE.fullmatch(lines[1])
        if not match:
            raise ValueError("malformed writer code window")
        pc = int(match["pc"], 16)
        vram = int(match["vram"], 16)
        code_start = int(match["start"], 16)
        words = [int(value, 16) for value in match["code"].split(",")]
        if int(match["words"], 10) != len(words) or not words:
            raise ValueError("writer code word count is invalid")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP1_RUNTIME_WRITER_JOIN_INVALID: {error}")
        return 1
    if args.require_pc is not None and pc != args.require_pc:
        print(f"required_pc=0x{args.require_pc:08x} observed=0x{pc:08x}")
        return 1
    if args.require_vram is not None and vram != args.require_vram:
        print(f"required_vram=0x{args.require_vram:05x} observed=0x{vram:05x}")
        return 1

    print(f"runtime_pc=0x{pc:08x} vram_addr=0x{vram:05x} code_start=0x{code_start:08x}")
    print(f"runtime_code_words={len(words)}")
    if args.runtime_base is None:
        print("direct_runtime_mapping=not_requested")
    else:
        direct_offset = code_start - args.runtime_base
        print(f"runtime_base=0x{args.runtime_base:08x} direct_file_offset=0x{direct_offset:06x}")

    verified = 0
    exact = 0
    for name, (expected_sha, expected_size) in EXPECTED.items():
        path = args.data_dir / name
        try:
            blob = path.read_bytes()
        except OSError as error:
            print(f"{name}: unavailable ({error})")
            continue
        digest = hashlib.sha256(blob).hexdigest()
        if len(blob) != expected_size or digest != expected_sha:
            print(f"{name}: rejected size={len(blob)} sha256={digest}")
            continue
        verified += 1
        native = b"".join(word.to_bytes(2, "big") for word in words)
        found = blob.find(native)
        run, run_offset = longest_run(words, blob)
        direct = None
        if args.runtime_base is not None:
            direct_offset = code_start - args.runtime_base
            if 0 <= direct_offset <= len(blob) - len(native):
                direct = blob[direct_offset:direct_offset + len(native)] == native
        if found >= 0:
            exact += 1
        direct_text = "unavailable" if direct is None else ("match" if direct else "mismatch")
        offset_text = "none" if found < 0 else f"0x{found:06x}"
        run_offset_text = "none" if run_offset < 0 else f"0x{run_offset:06x}"
        print(f"{name}: direct={direct_text} exact_native={offset_text} "
              f"longest_word_run={run}/{len(words)} at={run_offset_text}")

    print(f"authenticated_files={verified}/2 exact_native_file_owners={exact}")
    print("runtime_code_source_identity=unbound")
    print("relocation_or_decompression_owner=unproven")
    print("semantic_admission=blocked")
    return 0 if verified == 2 else 1


if __name__ == "__main__":
    raise SystemExit(main())
