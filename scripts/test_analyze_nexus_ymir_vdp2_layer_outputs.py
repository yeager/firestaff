#!/usr/bin/env python3
"""Protocol-level test for the Ymir VDP2 layer trace analyzer."""

from __future__ import annotations

import argparse
import importlib.util
from pathlib import Path
import struct
import tempfile


SCRIPT = Path(__file__).with_name("analyze_nexus_ymir_vdp2_layer_outputs.py")
SPEC = importlib.util.spec_from_file_location("nexus_ymir_layer_analyzer", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
ANALYZER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(ANALYZER)


def put_record(output: bytearray, frame: int, serial: int, y: int, layer: int,
               width: int, base_color: int) -> None:
    output.extend(struct.pack("<QQIIBBH", frame, serial, y, y, 0, layer, width))
    for x in range(width):
        output.extend(struct.pack("<I", base_color + x))
    output.extend(bytes([layer + 1] * width))
    output.extend(bytes([x % 2 for x in range(width)]))


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="nexus-ymir-layer-trace-test-") as temporary:
        root = Path(temporary)
        trace_path = root / "layer-outputs.bin"
        receipt_path = root / "layer-outputs.bin.receipt.txt"
        reference_path = root / "vdp1-1-vdp2-1280-2x2.rgbx8888le"
        index_path = root / "index.csv"
        output_path = root / "layer.rgba"
        width = height = 2
        records = bytearray(ANALYZER.MAGIC)
        reference = bytearray()
        for serial in (1279, 1280):
            for y in range(height):
                for layer in range(len(ANALYZER.LAYER_NAMES)):
                    color = 0x00100000 + serial + y * 16 + layer * 0x100
                    put_record(records, 1, serial, y, layer, width, color)
                    if serial == 1280 and layer == 3:
                        for x in range(width):
                            value = color + x
                            reference.extend(bytes((value & 0xFF, (value >> 8) & 0xFF,
                                                    (value >> 16) & 0xFF, 0)))
        trace_path.write_bytes(records)
        reference_path.write_bytes(reference)
        index_path.write_text(
            "frame_index,vdp2_frame_serial,width,height,pixel_format,file\n"
            "1,1280,2,2,rgbx8888le,vdp1-1-vdp2-1280-2x2.rgbx8888le\n",
            encoding="utf-8",
        )
        receipt_path.write_text(
            "format=ymir-vdp2-layer-output-trace-v1\n"
            "magic=YMLYR001\nrecord_header_bytes=28\n"
            "record_header=frame_index:u64le,vdp2_frame_serial:u64le,line_call_ordinal:u32le,"
            "row_vcnt:u32le,alternate_field:u8,layer_id:u8,width:u16le\n"
            "pixel_planes=color_u32le[width],priority_u8[width],special_color_calc_u8[width]\n"
            f"records_written=28\nbytes_written={len(records)}\nbyte_limit=67108864\n"
            "limit_exceeded=0\nwrite_error=0\nframe_range_half_open=1:2\n"
            "row_vcnt_range_half_open=0:2\nobserved_frame_count=1\n"
            "observed_vdp2_serial_count=2\nobserved_vdp2_serial_ids=1279,1280\n"
            "observed_pass_field_count=2\ncapture_status=complete\n",
            encoding="utf-8",
        )
        args = argparse.Namespace(
            trace=trace_path,
            receipt=receipt_path,
            reference_rgbx=reference_path,
            rendered_frames_index=index_path,
            frame=1,
            serial=1280,
            layer=3,
            width=width,
            height=height,
            output_rgba=output_path,
            output_json=None,
        )
        result = ANALYZER.analyze(args)
        assert result["exact_rgb_matches"] == 4
        assert result["exact_rgb_mismatches"] == 0
        assert result["nonzero_priority_pixels"] == 4
        assert result["special_color_calc_pixels"] == 2
        expected_rgba = bytearray()
        for offset in range(0, len(reference), 4):
            expected_rgba.extend(reference[offset : offset + 3])
            expected_rgba.append(0xFF)
        assert output_path.read_bytes() == expected_rgba

        receipt_path.write_text(receipt_path.read_text(encoding="utf-8").replace(
            "capture_status=complete", "capture_status=incomplete"), encoding="utf-8")
        try:
            ANALYZER.analyze(args)
        except ANALYZER.TraceError:
            pass
        else:
            raise AssertionError("incomplete capture receipt was accepted")

    print("NEXUS_YMIR_VDP2_LAYER_TRACE_TEST_OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
