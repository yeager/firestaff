#!/usr/bin/env python3
"""Regression checks for authentic Ymir VDP2 bitmap capture receipts."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import tempfile


SCRIPT = Path(__file__).with_name("analyze_nexus_vdp2_bitmap_source.py")
SPEC = importlib.util.spec_from_file_location("nexus_vdp2_bitmap_source", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
ANALYZER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(ANALYZER)


def write_bundle(root: Path, trace_rows: int = 0, complete: bool = True) -> None:
    trace_range = (
        "first_vdp1_frame_index=1604\nlast_vdp1_frame_index=1604\n"
        if trace_rows else ""
    )
    receipt = (
        "format=ymir-vdp2-memory-write-bundle-v1\n"
        "current_vdp1_frame_index=1605\n"
        "trace_rows=" + str(trace_rows) + "\n" + trace_range
        + "trace_dropped_total=0\n"
        + "bundle_complete=" + ("1" if complete else "0") + "\n"
        + "trace_file_complete=1\n"
        + "vdp2_vram_file_complete=1\n"
        + "vdp2_cram_file_complete=1\n"
    )
    (root / "vdp2-memory-write-bundle.txt").write_text(receipt, encoding="ascii")
    (root / "vdp2-memory-write-trace.csv").write_text(
        "vdp1_frame,memory,address,width,value\n"
        + ("1604,VRAM,0x000000,1,0x12\n" if trace_rows else ""),
        encoding="ascii",
    )
    (root / "vdp2-registers.bin").write_bytes(bytes(512))
    (root / "vdp2-vram.bin").write_bytes(bytes(512 * 1024))
    (root / "vdp2-cram.bin").write_bytes(bytes(4096))


def write_event_bundle(root: Path, corrupt_final_snapshot: bool = False) -> None:
    (root / "vdp2-memory-write-bundle.txt").write_text(
        "format=ymir-vdp2-memory-write-bundle-v1\n"
        "current_vdp1_frame_index=8037\n"
        "address_begin=0x0\naddress_end_exclusive=0x80000\n"
        "trace_vram=1\ntrace_cram=1\ntrace_file_complete=1\n"
        "render_event_trace_file_complete=1\ninitial_vram_file_complete=1\n"
        "initial_cram_file_complete=1\nrow_state_reconstruction_complete=1\n"
        "vdp2_vram_file_complete=1\nvdp2_cram_file_complete=1\n"
        "vdp2_register_file_complete=1\ntrace_rows=1\nrender_event_rows=2\n"
        "trace_dropped_total=0\ntrace_vdp1_frame_range_enabled=0\nbundle_complete=1\n",
        encoding="ascii",
    )
    (root / "vdp2-vram-start.bin").write_bytes(bytes(512 * 1024))
    (root / "vdp2-cram-start.bin").write_bytes(bytes(4096))
    final_vram = bytearray(512 * 1024)
    final_vram[0:2] = b"\x12\x34"
    if corrupt_final_snapshot:
        final_vram[0] = 0
    (root / "vdp2-vram.bin").write_bytes(final_vram)
    (root / "vdp2-cram.bin").write_bytes(bytes(4096))
    (root / "vdp2-registers.bin").write_bytes(bytes(512))
    (root / "vdp2-render-event-trace.csv").write_text(
        "sequence,event,vdp1_frame,row,memory,address,width,value\n"
        "1,write,unknown,,VRAM,0x000000,2,0x1234\n"
        "2,scanline,8037,0,,,,\n",
        encoding="ascii",
    )


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="nexus-vdp2-empty-trace-") as temp:
        root = Path(temp)
        write_bundle(root)
        regions, frame, byte_order = ANALYZER.ymir_bundle_frame(root)
        assert frame == 1605
        assert byte_order == "big"
        assert len(regions["vdp2-regs"]) == 512
        assert len(regions["vdp2-vram"]) == 512 * 1024
        assert len(regions["vdp2-cram"]) == 4096

        write_bundle(root, trace_rows=1)
        regions, frame, byte_order = ANALYZER.ymir_bundle_frame(root)
        assert frame == 1605 and byte_order == "big"
        assert len(regions["vdp2-vram"]) == 512 * 1024

        write_bundle(root, complete=False)
        try:
            ANALYZER.ymir_bundle_frame(root)
        except ValueError:
            pass
        else:
            raise AssertionError("incomplete empty-trace receipt was accepted")

        write_bundle(root, trace_rows=2)
        try:
            ANALYZER.ymir_bundle_frame(root)
        except ValueError:
            pass
        else:
            raise AssertionError("trace row count mismatch was accepted")

        write_event_bundle(root)
        regions, frame, byte_order = ANALYZER.ymir_bundle_frame(root)
        assert frame == 8037 and byte_order == "big"
        assert regions["vdp2-vram"][:2] == b"\x12\x34"

        write_event_bundle(root, corrupt_final_snapshot=True)
        try:
            ANALYZER.ymir_bundle_frame(root)
        except ValueError:
            pass
        else:
            raise AssertionError("event replay with a mismatched final snapshot was accepted")

    print("NEXUS_VDP2_BITMAP_RECEIPT_REPLAY_TEST_OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
