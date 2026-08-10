#!/usr/bin/env python3
"""Join a VDP2 write trace with its SH-2 register witness.

The register snapshot establishes the runtime destination, source pointer,
and a bounded pointed-to RAM byte window at a writer PC.  Optional retail
corpus scanning can identify an exact file/offset, but neither that match nor
the transport join authorizes a menu/HUD/viewport consumer.  That boundary is
kept explicit so a source match is never mistaken for a FONT256 or CLUT join.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path


REG_LINE = re.compile(r"^(?:frame=[0-9]+ )?addr=0x([0-9a-fA-F]+) pc=0x([0-9a-fA-F]+)(.*)$")
REG_VALUE = re.compile(r" r([0-9]+)=0x([0-9a-fA-F]+)")
SOURCE_BYTES_R4 = re.compile(
    r" src_r4=0x([0-9a-fA-F]+) src_words=([0-9a-fA-F]{4}(?:,[0-9a-fA-F]{4}){7})$")
SOURCE_BYTES_R5 = re.compile(
    r" src_r5=0x([0-9a-fA-F]+) src5_words=([0-9a-fA-F]{4}(?:,[0-9a-fA-F]{4}){7})$")
WRITE_LINE = re.compile(
    r"^area=([a-z]+) addr=0x([0-9a-fA-F]+) size=([0-9]+) value=0x([0-9a-fA-F]+) "
    r"pc0=0x([0-9a-fA-F]+) pc1=0x([0-9a-fA-F]+)$"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("writes", type=Path)
    parser.add_argument("registers", type=Path)
    parser.add_argument("--writer-pc", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--destination", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--source-register", type=int, choices=range(16), default=4)
    parser.add_argument("--data-dir", type=Path)
    parser.add_argument("--minimum-writes", type=int, default=64)
    args = parser.parse_args()
    try:
        witness = None
        for line in args.registers.read_text(encoding="ascii").splitlines():
            match = REG_LINE.fullmatch(line)
            if match and int(match.group(1), 16) == args.destination and \
                    int(match.group(2), 16) == args.writer_pc:
                witness = {
                    int(index): int(value, 16)
                    for index, value in REG_VALUE.findall(match.group(3))
                }
                source_pattern = (SOURCE_BYTES_R5 if args.source_register == 5
                                  else SOURCE_BYTES_R4)
                source_match = source_pattern.search(match.group(3))
                if source_match:
                    witness["source_address"] = int(source_match.group(1), 16)
                    witness["source_bytes"] = b"".join(
                        int(word, 16).to_bytes(2, "big")
                        for word in source_match.group(2).split(",")
                    )
                break
        if witness is None:
            raise ValueError("matching register witness is missing")

        rows = []
        for line in args.writes.read_text(encoding="ascii").splitlines():
            match = WRITE_LINE.fullmatch(line)
            if not match:
                continue
            if (int(match.group(2), 16) >= args.destination and
                    int(match.group(5), 16) == args.writer_pc):
                rows.append((int(match.group(2), 16), int(match.group(4), 16),
                             match.group(1)))
        rows.sort()
        if len(rows) < args.minimum_writes:
            raise ValueError(f"only {len(rows)} matching writes")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP2_REGISTER_WRITER_INVALID: {error}")
        return 1

    end = rows[args.minimum_writes - 1][0] + 2
    areas = sorted({row[2] for row in rows[:args.minimum_writes]})
    print(f"writer_pc=0x{args.writer_pc:08x} destination=0x{args.destination:05x}")
    print(f"verified_writes={args.minimum_writes} destination_end=0x{end:05x}")
    print("destination_areas=" + ",".join(areas))
    source_register = args.source_register
    print("source_pointer=" +
          (f"0x{witness[source_register]:08x}"
           if source_register in witness else "unobserved"))
    print(f"source_pointer_register=r{source_register}")
    source_bytes = witness.get("source_bytes")
    if isinstance(source_bytes, bytes):
        print("source_bytes_capture=verified")
        print(f"source_bytes_address=0x{witness['source_address']:08x}")
        print(f"source_bytes={len(source_bytes)}")
        print(f"source_bytes_sha256={hashlib.sha256(source_bytes).hexdigest()}")
    else:
        print("source_bytes_capture=missing")
    print("vdp2_destination_transport=verified")
    asset_matches = []
    if args.data_dir is not None and isinstance(source_bytes, bytes):
        try:
            for path in args.data_dir.rglob("*"):
                if not path.is_file():
                    continue
                data = path.read_bytes()
                if source_bytes in data:
                    asset_matches.append(f"{path.name}:0x{data.index(source_bytes):x}")
                elif source_bytes[::-1] in data:
                    asset_matches.append(f"{path.name}:word-swap-0x{data.index(source_bytes[::-1]):x}")
        except (OSError, UnicodeError) as error:
            print(f"NEXUS_VDP2_REGISTER_WRITER_INVALID: asset scan failed: {error}")
            return 1
    if asset_matches:
        print("asset_identity=verified")
        print("asset_matches=" + "|".join(asset_matches))
    else:
        print("asset_identity=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
