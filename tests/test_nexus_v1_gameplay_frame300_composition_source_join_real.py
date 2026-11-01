#!/usr/bin/env python3
"""Verify one authentic gameplay frame's composition and retail DGN joins."""

from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: test_nexus_v1_gameplay_frame300_composition_source_join_real.py <tilemap-test>")
        return 2
    evidence = Path(os.environ.get("FIRESTAFF_NEXUS_FRAME300_EVIDENCE_DIR", ""))
    capture = evidence / "capture-301"
    dumps = capture / "dumps"
    if not dumps.is_dir():
        print("SKIP: set FIRESTAFF_NEXUS_FRAME300_EVIDENCE_DIR to the authentic frame-300 Ymir evidence")
        return 77
    reference = capture / "vdp2-rendered" / "vdp1-300-vdp2-301-320x224.rgbx8888le"
    if not reference.is_file():
        print("SKIP: authentic frame-300 Ymir composed reference is unavailable")
        return 77

    with tempfile.TemporaryDirectory(prefix="firestaff-nexus-frame300-") as temp:
        output = Path(temp) / "firestaff-priority-stack.bin"
        env = os.environ.copy()
        env.update({
            "FIRESTAFF_NEXUS_YMIR_FRAME300_VDP1_FBS": str(dumps / "vdp1-fbs.bin"),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_VRAM": str(dumps / "vdp2-vram.bin"),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_CRAM": str(dumps / "vdp2-cram.bin"),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_VDP2_REGS": str(dumps / "vdp2-registers.bin"),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_RENDERED": str(reference),
            "FIRESTAFF_NEXUS_YMIR_FRAME300_STACK_OUT": str(output),
        })
        composition = subprocess.run(
            [sys.argv[1]], check=False, env=env, capture_output=True, text=True
        )
        sys.stdout.write(composition.stdout)
        sys.stderr.write(composition.stderr)
        if composition.returncode != 0:
            return composition.returncode

        env["FIRESTAFF_NEXUS_YMIR_GAMEPLAY_FRAME300_CAPTURE_DIR"] = str(dumps)
        join = subprocess.run(
            [sys.executable, str(ROOT / "tests" /
             "test_nexus_v1_vdp1_gameplay_frame300_source_join_real.py")],
            check=False,
            env=env,
            capture_output=True,
            text=True,
        )
        sys.stdout.write(join.stdout)
        sys.stderr.write(join.stderr)
        if join.returncode != 0:
            return join.returncode
        print("PASS: the same authentic frame-300 capture passed VDP composition and retail DGN source-join checks")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
