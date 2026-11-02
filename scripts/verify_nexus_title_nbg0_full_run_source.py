#!/usr/bin/env python3
"""Verify a Japanese retail TITLE.BIN span through a complete Ymir VDP2 run."""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path

from analyze_nexus_vdp2_bitmap_source import read_assets, ymir_bundle_frame


CUE_SHA256 = "878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699"
TITLE_BIN_SHA256 = "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
TITLE_OFFSET = 0xB8A2
SPAN_BYTES = 921
VRAM_OFFSET = 0xD404
WRAM_HI_OFFSET = 0xB5622
COPY_FRAME = 7716
SNAPSHOT_FRAME = 8037
EVENT_TRACE_SHA256 = "2829b136bff432ea4ec5be0abefd4952c39667145c69940a25b436dce02acee7"


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="complete Ymir VDP2 memory-event bundle")
    parser.add_argument("--cue", required=True, type=Path, help="authentic Japanese retail CUE")
    args = parser.parse_args()
    dumps = args.bundle / "dumps" if (args.bundle / "dumps").is_dir() else args.bundle

    try:
        require(sha256(args.cue.read_bytes()) == CUE_SHA256,
                "Japanese retail CUE hash mismatch")
        title_bin = read_assets(None, args.cue, {"TITLE.BIN"})["TITLE.BIN"]
        require(sha256(title_bin) == TITLE_BIN_SHA256,
                "Japanese TITLE.BIN hash mismatch")
        source = title_bin[TITLE_OFFSET:TITLE_OFFSET + SPAN_BYTES]
        require(len(source) == SPAN_BYTES, "retail TITLE.BIN source span is truncated")
        event_trace_path = dumps / "vdp2-render-event-trace.csv"
        require(sha256_file(event_trace_path) == EVENT_TRACE_SHA256,
                "Ymir event trace hash differs from the retained authentic capture")

        # This validates receipt completeness, event order, write bounds, and
        # replay equality against the authentic final VRAM and CRAM snapshots.
        regions, snapshot_frame, byte_order = ymir_bundle_frame(dumps)
        require(snapshot_frame == SNAPSHOT_FRAME,
                f"expected the authentic endpoint at frame {SNAPSHOT_FRAME}")
        registers = regions["vdp2-regs"]
        require(byte_order == "big", "VDP2 register snapshot is not big-endian")
        observed_registers = {
            offset: int.from_bytes(registers[offset:offset + 2], "big")
            for offset in (0x00, 0x20, 0x28, 0x2A, 0x2C, 0x3C, 0xE4, 0xE6)
        }
        expected_registers = {
            0x00: 0x8000, 0x20: 0x0001, 0x28: 0x0013, 0x2A: 0x0002,
            0x2C: 0x0000, 0x3C: 0x0000, 0xE4: 0x0000, 0xE6: 0x0000,
        }
        require(observed_registers == expected_registers,
                "frame-8037 VDP2 registers do not select the captured NBG0 bitmap lane")
        wram_hi = (dumps / "wram-hi.bin").read_bytes()
        require(len(wram_hi) == 0x100000, "WorkRAMH snapshot has an unexpected size")
        require(wram_hi[WRAM_HI_OFFSET:WRAM_HI_OFFSET + SPAN_BYTES] == source,
                "final WorkRAMH does not contain the retail source span")

        current = bytearray((dumps / "vdp2-vram-start.bin").read_bytes()[
            VRAM_OFFSET:VRAM_OFFSET + SPAN_BYTES])
        require(len(current) == SPAN_BYTES, "initial VRAM does not cover the source span")
        require(current != source,
                "initial VRAM already equals the retail span, so first-write provenance is ambiguous")
        first_exact: tuple[int, str] | None = None
        frame_copy_values: dict[int, int] = {}
        frame_copy_addresses: set[int] = set()
        unknown_target_writes = 0
        with event_trace_path.open(encoding="ascii", newline="") as stream:
            reader = csv.DictReader(stream)
            expected_fields = [
                "sequence", "event", "vdp1_frame", "row", "memory",
                "address", "width", "value",
            ]
            require(reader.fieldnames == expected_fields,
                    "Ymir render-event trace header is unsupported")
            for row in reader:
                if row["event"] != "write" or row["memory"] != "VRAM":
                    continue
                address = int(row["address"], 16)
                width = int(row["width"])
                write = int(row["value"], 16).to_bytes(width, "big")
                start = max(address, VRAM_OFFSET)
                end = min(address + width, VRAM_OFFSET + SPAN_BYTES)
                if start >= end:
                    continue
                frame = row["vdp1_frame"]
                if frame == "unknown":
                    unknown_target_writes += 1
                    continue
                require(frame.isdigit(), "target-span write has an invalid frame label")
                current[start - VRAM_OFFSET:end - VRAM_OFFSET] = \
                    write[start - address:end - address]
                if int(frame) == COPY_FRAME:
                    for absolute in range(start, end):
                        offset = absolute - VRAM_OFFSET
                        require(absolute not in frame_copy_addresses,
                                "copy frame writes a target byte more than once")
                        frame_copy_addresses.add(absolute)
                        value = write[absolute - address]
                        frame_copy_values[offset] = value
                        require(value == source[offset],
                                "copy-frame byte differs from the retail source")
                if first_exact is None and current == source:
                    first_exact = (int(row["sequence"]), frame)

        require(unknown_target_writes == 0,
                "an unframed write touches the retail source span")
        require(first_exact == (4075865, str(COPY_FRAME)),
                "the first exact retail source state differs from the captured copy event")
        require(len(frame_copy_addresses) == 608,
                "copy-frame write count differs from the authentic capture")
        nonzero_offsets = {offset for offset, value in enumerate(source) if value}
        require(nonzero_offsets <= set(frame_copy_values),
                "copy frame omits a nonzero byte from the retail source span")
        final_vram = regions["vdp2-vram"]
        require(final_vram[VRAM_OFFSET:VRAM_OFFSET + SPAN_BYTES] == source,
                "final VDP2 snapshot does not contain the complete retail source span")
    except (KeyError, OSError, TypeError, UnicodeError, ValueError) as error:
        print(f"NEXUS_TITLE_NBG0_FULL_RUN_SOURCE_INVALID: {error}")
        return 1

    print("title_nbg0_full_run_source=verified")
    print(f"japanese_cue_sha256={CUE_SHA256}")
    print(f"japanese_title_bin_sha256={TITLE_BIN_SHA256}")
    print(f"title_source_offset=0x{TITLE_OFFSET:x}")
    print(f"title_source_bytes={SPAN_BYTES}")
    print(f"title_source_sha256={sha256(source)}")
    print(f"vdp2_event_trace_sha256={EVENT_TRACE_SHA256}")
    print(f"title_source_nonzero_bytes={len(nonzero_offsets)}")
    print(f"wram_hi_source_offset=0x{WRAM_HI_OFFSET:x}")
    print(f"vram_source_offset=0x{VRAM_OFFSET:x}")
    print(f"copy_frame={COPY_FRAME}")
    print(f"copy_frame_byte_writes={len(frame_copy_addresses)}")
    print("copy_frame_nonzero_source_bytes_written=all")
    print(f"first_exact_source_event_sequence={first_exact[0]}")
    print(f"snapshot_frame={snapshot_frame}")
    print(f"snapshot_register_byte_order={byte_order}")
    print("TVMD=0x8000 BGON=0x0001 CHCTLA=0x0013 PLSZ=0x0002")
    print("BMPNA=0x0000 MPOFN=0x0000 CRAOFA=0x0000 CRAOFB=0x0000")
    print("vdp2_event_replay_matches_final_snapshots=1")
    print("full_bitmap_source_ownership=unverified")
    print("visible_layer_output=unverified")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
