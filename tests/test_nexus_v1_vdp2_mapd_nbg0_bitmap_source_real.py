#!/usr/bin/env python3
"""Run Firestaff's raw MAPD NBG0 decoder against authentic Ymir snapshots."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import subprocess
import sys


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: test_nexus_v1_vdp2_mapd_nbg0_bitmap_source_real.py "
              "<runtime tilemap test> <capture output>", file=sys.stderr)
        return 2
    evidence_text = os.environ.get(
        "FIRESTAFF_NEXUS_YMIR_MAPD_NBG0_EVIDENCE_DIR"
    )
    if not evidence_text:
        print("SKIP: set FIRESTAFF_NEXUS_YMIR_MAPD_NBG0_EVIDENCE_DIR to the "
              "external authentic MAPD evidence directory")
        return 77
    evidence = Path(evidence_text)
    snapshots = evidence / "vdp-snapshots"
    layer_trace = evidence / "layer-trace-frame891-start60-20261101"
    output = Path(sys.argv[2])
    env = os.environ.copy()
    env.update({
        "FIRESTAFF_NEXUS_YMIR_MAPD_NBG0_VRAM": str(
            snapshots / "vdp1-891-vdp2-1280-vdp2-vram.bin"
        ),
        "FIRESTAFF_NEXUS_YMIR_MAPD_NBG0_CRAM": str(
            snapshots / "vdp1-891-vdp2-1280-vdp2-cram.bin"
        ),
        "FIRESTAFF_NEXUS_YMIR_MAPD_NBG0_REGS": str(
            snapshots / "vdp1-891-vdp2-1280-vdp2-registers.bin"
        ),
        "FIRESTAFF_NEXUS_YMIR_MAPD_NBG0_RENDER_STATE": str(
            layer_trace / "render-state.csv"
        ),
        "FIRESTAFF_NEXUS_YMIR_MAPD_NBG0_LAYERS": str(
            layer_trace / "layers.csv"
        ),
        "FIRESTAFF_NEXUS_YMIR_MAPD_NBG0_OUT": str(output),
    })
    result = subprocess.run([sys.argv[1]], env=env, check=False)
    if result.returncode != 0:
        print("FAIL: Firestaff MAPD NBG0 source decoder did not match the "
              "authentic Ymir layer trace", file=sys.stderr)
        return result.returncode or 1
    data = output.read_bytes()
    if len(data) != 320 * 224 * 4:
        print("FAIL: Firestaff NBG0 plane has an unexpected size", file=sys.stderr)
        return 1
    print("PASS: Firestaff raw NBG0 bitmap decode matches all authentic "
          f"Ymir layer pixels (plane SHA-256 {hashlib.sha256(data).hexdigest()})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
