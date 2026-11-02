#!/usr/bin/env python3
"""Verify authentic Nexus row-80 SCU DMA, VDP1, and MENU.BPK byte joins."""

from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    evidence = os.environ.get("FIRESTAFF_NEXUS_ROW80_DMA_EVIDENCE_DIR")
    frame300 = os.environ.get("FIRESTAFF_NEXUS_FRAME300_EVIDENCE_DIR")
    data = Path(os.environ.get(
        "FIRESTAFF_NEXUS_DATA_DIR",
        str(Path.home() / ".firestaff" / "data" / "nexus"),
    ))
    if not evidence or not (Path(evidence) / "dumps").is_dir():
        print("SKIP: set FIRESTAFF_NEXUS_ROW80_DMA_EVIDENCE_DIR to the authentic DMA evidence root")
        return 77
    if not frame300:
        print("SKIP: set FIRESTAFF_NEXUS_FRAME300_EVIDENCE_DIR to the authentic frame-300 evidence root")
        return 77
    if not data.is_dir():
        print("SKIP: authentic Nexus retail data directory is unavailable")
        return 77
    menu_bpk = data / "MENU.BPK"
    if not menu_bpk.is_file():
        print("SKIP: authentic Japanese retail MENU.BPK is unavailable")
        return 77

    command = [
        sys.executable,
        str(ROOT / "scripts" / "analyze_nexus_vdp1_row80_scu_dma_join.py"),
        "--dma-dumps", str(Path(evidence) / "dumps"),
        "--frame300-dumps", str(Path(frame300) / "capture-301" / "dumps"),
        "--menu-bpk", str(menu_bpk),
    ]
    result = subprocess.run(command, check=False, capture_output=True, text=True)
    if result.stdout:
        print(result.stdout, end="")
    if result.stderr:
        print(result.stderr, end="", file=sys.stderr)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
