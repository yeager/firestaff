#!/usr/bin/env python3
"""Verify an authentic Ymir Nexus title-state VDP2 checkpoint.

The retail disc, BIOS, save state, and Ymir dump are supplied at invocation
time.  No game bytes are stored in this repository.  This deliberately checks
only the static VDP2 title checkpoint; it does not claim VDP1 menu parity.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path


EXPECTED = {
    "cue": "878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699",
    "bios": "dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac",
    "state": "11d88ef517c408b3bca5c078c0ccb8081dbf3342f308239dd73403c73f0c3774",
    "vdp2-vram.bin": "de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c",
    "vdp2-cram.bin": "db282593790dec4a2bf8f76293c0458a1a140adc3351650bd56a417f0359e0de",
    "vdp2-registers.bin": "79efa616c2db727749aa9018864d1ad32c1583935652fafb16534ba69f8e1f2d",
}

EXPECTED_SIZES = {
    "vdp1-registers.bin": 32,
    "vdp2-registers.bin": 512,
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require_digest(label: str, path: Path) -> None:
    actual = digest(path)
    expected = EXPECTED[label]
    if actual != expected:
        raise ValueError(f"{label} SHA-256 mismatch: {actual} != {expected}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cue", required=True, type=Path,
                        help="hash-pinned Japanese retail Nexus CUE")
    parser.add_argument("--bios", required=True, type=Path,
                        help="hash-pinned Japanese Saturn IPL ROM")
    parser.add_argument("--state", required=True, type=Path,
                        help="hash-pinned Ymir title-prompt save state")
    parser.add_argument("--dump-root", required=True, type=Path,
                        help="Ymir DumpMemory output after loading CLI slot 2")
    args = parser.parse_args()

    for label, path in (("cue", args.cue), ("bios", args.bios), ("state", args.state)):
        require_digest(label, path)
    for name in ("vdp2-vram.bin", "vdp2-cram.bin", "vdp2-registers.bin"):
        require_digest(name, args.dump_root / name)
    for name, size in EXPECTED_SIZES.items():
        actual = (args.dump_root / name).stat().st_size
        if actual != size:
            raise ValueError(f"{name} size mismatch: {actual} != {size}")

    snapshot = (args.dump_root / "vdp-register-snapshot.txt").read_text()
    if "format=ymir-vdp-register-snapshot-v1\n" not in snapshot:
        raise ValueError("missing Ymir VDP register snapshot receipt")
    print("Nexus Ymir title-state VDP2 checkpoint: PASS")


if __name__ == "__main__":
    main()
