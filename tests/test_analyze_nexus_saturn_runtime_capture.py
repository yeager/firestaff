#!/usr/bin/env python3
"""Regression-check VDP1 draw-buffer extraction from capture schemas.

Fixtures model only the public raw-capture envelope; they contain no game
data.  V2 state carries the current selector while early V2 files may carry a
trailing byte instead.
"""

from __future__ import annotations

import os
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import analyze_nexus_saturn_runtime_capture as capture  # noqa: E402
import verify_nexus_title_nbg0_producer as producer  # noqa: E402
from validate_nexus_saturn_runtime_capture import (  # noqa: E402
    RUNTIME_MAGIC,
    VDP1_MAGIC_V2,
    VDP1_PAYLOAD_BYTES,
    VDP2_MAGIC,
    VDP2_PAYLOAD_BYTES,
)


def frame(index: int, selector: int, trailing: bytes = b"") -> bytes:
    return (f"frame={index}\n".encode("ascii") + VDP1_MAGIC_V2 +
            b"state=tvmr:00,fbcr:03,ptmr:02,edsr:03,lopr:0000,copr:000000,ret:ffffffff,fb:"
            + str(selector).encode("ascii") + b",sysclipx:013f,sysclipy:00df\n" +
            bytes(VDP1_PAYLOAD_BYTES) + trailing + VDP2_MAGIC +
            bytes(VDP2_PAYLOAD_BYTES))


def witness(selector: int, trailing: bytes = b"") -> bytes:
    return RUNTIME_MAGIC + frame(0, selector, trailing)


def main() -> int:
    stage_root = os.environ.get("FIRESTAFF_TEST_STAGE_ROOT")
    if not stage_root:
        print("SKIP: FIRESTAFF_TEST_STAGE_ROOT is not set")
        return 77
    stage = Path(stage_root) / "nexus-capture-analysis"
    stage.mkdir(parents=True, exist_ok=True)
    current = witness(0)
    frames, states = capture.frame_regions(current, 1)
    assert states == [
        "state=tvmr:00,fbcr:03,ptmr:02,edsr:03,lopr:0000,copr:000000,ret:ffffffff,fb:0,sysclipx:013f,sysclipy:00df"
    ]
    assert frames[0]["vdp1-fb-draw-which"] == b"\x00"
    path = stage / "current.raw"
    path.write_bytes(current)
    streamed = list(capture.iter_frame_regions_file(path, 1))
    assert streamed[0][1]["vdp1-fb-draw-which"] == b"\x00"

    window = stage / "window.raw"
    window.write_bytes(RUNTIME_MAGIC + frame(0, 0) + frame(1, 1))
    assert producer.capture_frame(window, 2, 1)["vdp1-fb-draw-which"] == b"\x01"
    try:
        producer.capture_frame(window, 2, 2)
    except ValueError:
        pass
    else:
        raise AssertionError("out-of-window title frame must be rejected")

    # The delta helper receives an already observed VDP2 bus baseline.  Check
    # its byte-lane semantics directly; this does not model game data.
    delta, writes = producer.replay_vram_records(
        bytes(0x80000),
        [("vram", 0x10, 1, 0xab00, 0, 0),
         ("vram", 0x11, 1, 0x00cd, 0, 0)],
    )
    assert writes == 2 and delta[0x10:0x12] == b"\xab\xcd"

    early = witness(0, b"\x01")
    early_frames, _ = capture.frame_regions(early, 1)
    assert early_frames[0]["vdp1-fb-draw-which"] == b"\x01"
    print("Nexus Saturn capture draw-buffer analysis: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
