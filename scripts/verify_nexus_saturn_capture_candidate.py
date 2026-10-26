#!/usr/bin/env python3
"""Reject a raw Saturn capture whose rendered receipts are known non-menu frames.

The raw Mednafen VDP witness deliberately says nothing about what a frame means.
This small operator-side guard binds a requested rendered frame in the capture
manifest to its PPM receipt, then rejects it if its SHA-256 is one of the
operator supplied, authenticated non-menu fingerprints.  It does not identify
menus, render game data, or promote any Firestaff runtime gate.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path


RENDER_RECEIPT = re.compile(
    r"^render_frame_(\d{6})_ppm_sha256=([0-9a-f]{64})$", re.MULTILINE
)


def parse_fingerprints(values: list[str]) -> set[str]:
    fingerprints: set[str] = set()
    for value in values:
        fingerprint = value.lower()
        if not re.fullmatch(r"[0-9a-f]{64}", fingerprint):
            raise ValueError(f"invalid SHA-256 fingerprint: {value}")
        fingerprints.add(fingerprint)
    return fingerprints


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--ppm", type=Path, required=True)
    parser.add_argument(
        "--reject-sha256",
        action="append",
        default=[],
        metavar="HEX",
        help="authenticated non-menu PPM SHA-256; may be supplied repeatedly",
    )
    args = parser.parse_args()

    if args.frame < 0 or args.frame > 999999:
        parser.error("--frame must fit the six-digit Mednafen receipt format")
    try:
        rejected = parse_fingerprints(args.reject_sha256)
    except ValueError as error:
        parser.error(str(error))
    if not rejected:
        parser.error("at least one --reject-sha256 fingerprint is required")

    manifest = args.manifest.read_text(encoding="utf-8")
    receipts = {int(frame): digest for frame, digest in RENDER_RECEIPT.findall(manifest)}
    expected = receipts.get(args.frame)
    if expected is None:
        raise SystemExit(f"FAIL: manifest has no rendered receipt for frame {args.frame}")

    actual = hashlib.sha256(args.ppm.read_bytes()).hexdigest()
    if actual != expected:
        raise SystemExit(
            f"FAIL: frame {args.frame} PPM hash does not match the capture manifest"
        )
    if actual in rejected:
        raise SystemExit(
            f"REJECT: frame {args.frame} matches an authenticated non-menu fingerprint"
        )

    print(
        "NEXUS_SATURN_CAPTURE_CANDIDATE: PASS "
        f"frame={args.frame} ppm_sha256={actual} classification=unclassified"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
