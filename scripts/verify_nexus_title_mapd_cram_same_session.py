#!/usr/bin/env python3
"""Verify MAPD palette RAM -> VDP2 CRAM in one authenticated Saturn frame."""
from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

READ_HEADER = "FIRESTAFF_NEXUS_SH2_RAM_READ_TRACE_V2"
WRITE_HEADER = "FIRESTAFF_NEXUS_VDP2_WRITE_TRACE_V1"
REG_HEADER = "FIRESTAFF_NEXUS_VDP2_WRITER_REGISTER_TRACE_V1"
FRAME = 12592
RAM = 0x060C0C4C
CRAM = 0x100400
PALETTE_BYTES = 32
TRACK_SECTOR = 2352
TITLE_BIN_LBA = 6035
PALETTE_OFFSET = 0xE278 + 0x8C54


def manifest(path: Path) -> dict[str, str]:
    return dict(line.split("=", 1) for line in path.read_text().splitlines()
                if "=" in line)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("capture", type=Path)
    ap.add_argument("track1", type=Path)
    args = ap.parse_args()
    try:
        cap = args.capture
        paths = {name: cap / name for name in
                 ("manifest.txt", "reads.trace", "vdp2-writes.trace",
                  "vdp2-writer-registers.trace", "runtime-vdp12.raw")}
        meta = manifest(paths["manifest.txt"])
        if meta.get("capture_exit_status") != "0" or \
                meta.get("skip_frames") != str(FRAME) or \
                meta.get("frame_limit") != "1":
            raise ValueError("capture timing/status is not the authenticated frame")
        hash_keys = {
            "reads.trace": "FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS_sha256",
            "vdp2-writes.trace": "FIRESTAFF_NEXUS_TRACE_VDP2_WRITES_sha256",
            "vdp2-writer-registers.trace":
                "FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS_sha256",
            "runtime-vdp12.raw": "raw_sha256",
        }
        for name, key in hash_keys.items():
            if meta.get(key) != sha256(paths[name]):
                raise ValueError(f"hash mismatch: {name}")

        track = args.track1.read_bytes()
        absolute = (TITLE_BIN_LBA + PALETTE_OFFSET // 2048) * TRACK_SECTOR + \
            16 + PALETTE_OFFSET % 2048
        palette = track[absolute:absolute + PALETTE_BYTES]
        if len(palette) != PALETTE_BYTES:
            raise ValueError("truncated retail MAPD palette")
        words = [int.from_bytes(palette[i:i + 2], "big")
                 for i in range(0, PALETTE_BYTES, 2)]

        reads = paths["reads.trace"].read_text().splitlines()
        if not reads or reads[0] != READ_HEADER or not any(
                f"frame={FRAME} addr=0x{RAM:08x}" in row and
                "pc0=0x060856ec" in row and "r4=0x25f00400" in row and
                f"r5=0x{RAM:08x}" in row for row in reads[1:]):
            raise ValueError("retail MAPD palette source/destination receipt missing")

        writes = paths["vdp2-writes.trace"].read_text().splitlines()
        if not writes or writes[0] != WRITE_HEADER:
            raise ValueError("bad VDP2 write header")
        cram_rows = []
        pattern = re.compile(r"^area=cram addr=0x([0-9a-f]+) size=2 "
                             r"value=0x([0-9a-f]+) pc0=0x060856f0 ")
        for row in writes[1:]:
            match = pattern.match(row)
            if match and CRAM <= int(match[1], 16) < CRAM + PALETTE_BYTES:
                cram_rows.append((int(match[1], 16), int(match[2], 16)))
        if cram_rows != [(CRAM + i * 2, word) for i, word in enumerate(words)]:
            raise ValueError("MAPD palette does not exactly match CRAM writes")

        regs = paths["vdp2-writer-registers.trace"].read_text().splitlines()
        if not regs or regs[0] != REG_HEADER:
            raise ValueError("bad writer-register header")
        for i, word in enumerate(words):
            address, source = CRAM + i * 2, RAM + i * 2
            if not any(f"frame={FRAME} addr=0x{address:06x} " in row and
                       f"r1=0x{word | (0xffff0000 if word & 0x8000 else 0):08x}" in row and
                       f"r4=0x25f{address - 0x100000:05x}" in row and
                       f"r5=0x{source:08x}" in row for row in regs[1:]):
                raise ValueError(f"missing RAM-to-CRAM register row {i}")
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"NEXUS_TITLE_MAPD_CRAM_INVALID: {error}")
        return 1
    print("title_mapd_palette_ram=0x060c0c4c-0x060c0c6b")
    print("title_mapd_palette_cram=0x100400-0x10041f")
    print("title_mapd_palette_frame=12592")
    print("title_mapd_palette_ram_to_cram_same_session=verified")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
