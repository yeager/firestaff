#!/usr/bin/env python3
"""Verify a same-session SH-2 source-byte to VDP2 write sequence.

This is a producer-side provenance check. It proves that the selected SH-2
register source bytes equal the observed VDP2 write values and can bind the
source window to an exact retail file/offset. It does not authorize a host
tilemap, text, HUD, or final-screen compositor.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


REGISTER = re.compile(
    r"^(?:frame=[0-9]+ )?addr=0x(?P<addr>[0-9a-fA-F]+) pc=0x(?P<pc>[0-9a-fA-F]+)"
    r"(?P<registers>.*?) src_r(?P<src_reg>[0-9]+)=0x(?P<src>[0-9a-fA-F]+)"
    # The diagnostic hook names the r4 sample `src_words`, while r5 and
    # future numbered samples use `src5_words`; accept both spellings.
    r" src[0-9]*_words=(?P<words>[0-9a-fA-F]{4}(?:,[0-9a-fA-F]{4}){7})"
    r"(?: src_r[0-9]+=0x[0-9a-fA-F]+ src[0-9]*_words=[0-9a-fA-F,]+)?$")
WRITE = re.compile(
    r"^area=(?P<area>[a-z]+) addr=0x(?P<addr>[0-9a-fA-F]+)"
    r" size=(?P<size>[0-9]+) value=0x(?P<value>[0-9a-fA-F]+)"
    r" pc0=0x(?P<pc>[0-9a-fA-F]+) pc1=0x[0-9a-fA-F]+$")
REG_VALUE = re.compile(r" r([0-9]+)=0x([0-9a-fA-F]+)")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("writes", type=Path)
    parser.add_argument("registers", type=Path)
    parser.add_argument("--writer-pc", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--source-register", type=int, required=True)
    parser.add_argument("--source-load-base", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--destination-min", type=lambda value: int(value, 0), default=0)
    parser.add_argument("--require-area", choices=("vram", "cram", "regs"))
    parser.add_argument("--minimum-writes", type=int, default=64)
    parser.add_argument("--data-dir", type=Path, required=True)
    args = parser.parse_args()

    try:
        witnesses = []
        for line in args.registers.read_text(encoding="ascii").splitlines():
            match = REGISTER.fullmatch(line)
            if not match or int(match["pc"], 16) != args.writer_pc:
                continue
            if int(match["src_reg"]) != args.source_register:
                continue
            registers = {
                int(index): int(value, 16)
                for index, value in REG_VALUE.findall(match["registers"])
            }
            words = bytes().join(
                int(word, 16).to_bytes(2, "big")
                for word in match["words"].split(",")
            )
            witnesses.append((int(match["addr"], 16),
                              registers.get(args.source_register), words))

        writes = []
        for line in args.writes.read_text(encoding="ascii").splitlines():
            match = WRITE.fullmatch(line)
            if not match or int(match["pc"], 16) != args.writer_pc:
                continue
            writes.append((int(match["addr"], 16), int(match["size"]),
                           int(match["value"], 16), match["area"]))
        # The register hook is capped independently from the write hook and
        # also observes CRAM/register writes interleaved with VRAM writes.
        # Join by destination address and occurrence, rather than assuming
        # that the two bounded streams have identical length/order.
        witnesses_by_address = {}
        for witness in witnesses:
            witnesses_by_address.setdefault(witness[0], []).append(witness)
        rows = []
        for write_addr, write_size, value, area in writes:
            candidates = witnesses_by_address.get(write_addr)
            if not candidates:
                continue
            _, source, source_bytes = candidates.pop(0)
            if source is None:
                continue
            if write_size != 2 or source_bytes[:2] != value.to_bytes(2, "big"):
                raise ValueError(f"source/value mismatch at 0x{write_addr:x}")
            if (write_addr >= args.destination_min and
                    (args.require_area is None or area == args.require_area)):
                rows.append((write_addr, source, source_bytes, area))
        if len(rows) < args.minimum_writes:
            raise ValueError(f"only {len(rows)} same-session source/value writes")
        rows.sort()
        rows = rows[: args.minimum_writes]

        asset_hits = []
        for path in args.data_dir.rglob("*"):
            if not path.is_file():
                continue
            data = path.read_bytes()
            for _, source, source_bytes, _ in rows:
                # A zero-filled witness is an initialization write, not an
                # asset identity. Do not turn it into a false positive by
                # matching the same padding in every candidate file.
                if len(set(source_bytes)) <= 1:
                    continue
                offset = data.find(source_bytes)
                if offset >= 0:
                    asset_hits.append((path.name, offset))
                    break
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_VDP2_SOURCE_WRITE_JOIN_INVALID: {error}")
        return 1

    source_offsets = [source - args.source_load_base for _, source, _, _ in rows
                      if source >= args.source_load_base]
    areas = sorted({area for _, _, _, area in rows})
    print(f"writer_pc=0x{args.writer_pc:08x}")
    print(f"source_register=r{args.source_register}")
    print(f"verified_writes={len(rows)}")
    print(f"source_load_base=0x{args.source_load_base:08x}")
    if len(source_offsets) == len(rows):
        print(f"source_file_offset_start=0x{min(source_offsets):x}")
        print(f"source_file_offset_end=0x{max(source_offsets) + 16:x}")
    else:
        print("source_file_offset_range=mixed-runtime-address-domains")
    print("destination_areas=" + ",".join(areas))
    print("source_value_join=verified")
    if asset_hits:
        print("asset_identity=verified")
        print("asset_matches=" + "|".join(f"{name}:0x{offset:x}" for name, offset in asset_hits))
    else:
        print("asset_identity=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
