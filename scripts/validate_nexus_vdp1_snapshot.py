#!/usr/bin/env python3
"""Validate a standalone VDP1 snapshot from an authenticated Saturn run."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


SNAPSHOT_MAGIC = b"FIRESTAFF_NEXUS_VDP1_SNAPSHOT_V1\n"
VDP1_MAGIC = b"FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V2\n"
VDP1_PAYLOAD_BYTES = 0x40000 * 2 + 0x20000 * 2 + 0x20000 * 2 + 1
STATE_RE = re.compile(
    rb"^state=tvmr:[0-9a-f]+,fbcr:[0-9a-f]+,ptmr:[0-9a-f]+,"
    rb"edsr:[0-9a-f]+,lopr:[0-9a-f]+,copr:[0-9a-f]+,"
    rb"ret:[0-9a-f]+,fb:[01]$"
)


def validate(blob: bytes) -> None:
    if not blob.startswith(SNAPSHOT_MAGIC):
        raise ValueError("missing VDP1 snapshot magic")
    offset = len(SNAPSHOT_MAGIC)
    if not blob.startswith(VDP1_MAGIC, offset):
        raise ValueError("missing VDP1 raw marker")
    offset += len(VDP1_MAGIC)
    state_end = blob.find(b"\n", offset)
    if state_end < 0 or not STATE_RE.fullmatch(blob[offset:state_end]):
        raise ValueError("malformed VDP1 state line")
    offset = state_end + 1
    if len(blob) - offset != VDP1_PAYLOAD_BYTES:
        raise ValueError(
            f"expected {VDP1_PAYLOAD_BYTES} VDP1 payload bytes, "
            f"found {len(blob) - offset}"
        )
    if not any(blob[offset:]):
        raise ValueError("VDP1 payload is empty")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("snapshot", type=Path)
    args = parser.parse_args()
    try:
        blob = args.snapshot.read_bytes()
        validate(blob)
    except (OSError, ValueError) as error:
        print(f"NEXUS_VDP1_SNAPSHOT_INVALID: {error}")
        return 1
    print(
        "NEXUS_VDP1_SNAPSHOT_VALID: "
        f"bytes={len(blob)} payload={len(blob) - len(SNAPSHOT_MAGIC) - len(VDP1_MAGIC)}"
    )
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
