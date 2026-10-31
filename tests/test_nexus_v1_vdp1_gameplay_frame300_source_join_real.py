#!/usr/bin/env python3
"""Verify VDP1 command and retail DGN joins for authentic gameplay frame 300."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_CAPTURE_HASHES = {
    "vdp1-capture-bundle.txt":
        "7825022e760eecaec329b1a376106635815f84817afde1212f691a4c227c7965",
    "vdp1-command-trace.csv":
        "5b898067f27b39710e4773ab027de202e9fc14e855e8eebca845f779c8f7875d",
    "vdp1-vram.bin":
        "223ae7def1993ae4b62d0e180044a831d194fa772429b102ff4ce47ecdb55be0",
}
EXPECTED_DGN_HASH = (
    "b359a89ffa344439b6d0d15de223f3211caa7725b13dfdb4461b579c48f06723"
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run(command: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(command, check=False, capture_output=True, text=True)


def main() -> int:
    capture_root = os.environ.get(
        "FIRESTAFF_NEXUS_YMIR_GAMEPLAY_FRAME300_CAPTURE_DIR"
    )
    data_root = os.environ.get(
        "FIRESTAFF_NEXUS_DATA_DIR",
        str(Path.home() / ".firestaff" / "data" / "nexus"),
    )
    if not capture_root or not Path(capture_root).is_dir():
        print("SKIP: set FIRESTAFF_NEXUS_YMIR_GAMEPLAY_FRAME300_CAPTURE_DIR to authentic Ymir dumps")
        return 77
    if not Path(data_root).is_dir():
        print("SKIP: authentic Nexus retail data directory is unavailable")
        return 77

    capture = Path(capture_root)
    retail = Path(data_root)
    for name, expected in EXPECTED_CAPTURE_HASHES.items():
        path = capture / name
        if not path.is_file() or sha256(path) != expected:
            print(f"FAIL: authentic frame-300 capture hash mismatch: {name}")
            return 1
    lev01 = retail / "LEV01.DGN"
    if not lev01.is_file() or sha256(lev01) != EXPECTED_DGN_HASH:
        print("SKIP: source join requires the captured Japanese retail LEV01.DGN revision")
        return 77

    audit = run([
        sys.executable,
        str(ROOT / "scripts" / "audit_nexus_vdp1_ymir_trace.py"),
        str(capture / "vdp1-command-trace.csv"),
        "--frame",
        "300",
    ])
    if audit.returncode != 0:
        sys.stderr.write(audit.stdout)
        sys.stderr.write(audit.stderr)
        return audit.returncode
    for receipt in (
        '"all_dispatch_roots_zero": true',
        '"all_frame_roots_zero": true',
        '"count": 301',
        '"frame": 300',
        '"records": 83',
        '"root": "0x00000"',
    ):
        if receipt not in audit.stdout:
            print(f"FAIL: frame-300 VDP1 trace did not match receipt {receipt}")
            return 1

    sequence = run([
        sys.executable,
        str(ROOT / "scripts" / "analyze_nexus_vdp1_dgn_command_sequence_join.py"),
        str(capture),
        "--data-dir",
        data_root,
        "--frame",
        "300",
        "--ymir-bundle",
        "--require-complete",
    ])
    if sequence.returncode not in (0, 1):
        sys.stderr.write(sequence.stdout)
        sys.stderr.write(sequence.stderr)
        return sequence.returncode
    required = (
        "frame=300 chain_records=83",
        "textured_draws=68 source_matches=66 palette_matches=51 direct_color_matches=15 face_owner_matches=61",
        "informative_textured_draws=68 transparent_zero_filled_draws=0",
        "bounded_failures=2 unmatched_offsets=0x0cbe0,0x0cc20",
        "sequence_dgn_material_join=unbound",
        "transform_culling=unproven",
        "vdp2_composition=unproven",
        "semantic_admission=blocked",
    )
    missing = [receipt for receipt in required if receipt not in sequence.stdout]
    if missing:
        print("FAIL: frame-300 DGN join did not match the authentic receipt")
        print("missing=" + "|".join(missing))
        sys.stderr.write(sequence.stdout)
        return 1
    if sequence.returncode != 1:
        print("FAIL: frame-300 partial material join no longer has its expected blocked status")
        sys.stderr.write(sequence.stdout)
        return 1

    print("PASS: frame 300 contains an 83-record authenticated VDP1 chain with zero dropped rows")
    print("PASS: 66/68 textured commands match hash-verified retail DGN sources")
    print("PASS: 51 palettes and 15 direct-color spans match; 61 draws have candidate face owners")
    print("PASS: two zero-size draws remain unbound; semantic admission stays blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
