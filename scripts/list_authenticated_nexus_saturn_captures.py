#!/usr/bin/env python3
"""List hash-bound Nexus Saturn captures for the current retail CUE.

This is capture discovery only.  Consumers must still inspect the captured
registers and memory before assigning a hardware-state or screen identity.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import re
from pathlib import Path


def fields(path: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    try:
        for line in path.read_text(encoding="utf-8").splitlines():
            if "=" in line:
                key, value = line.split("=", 1)
                result[key] = value
    except (OSError, UnicodeError):
        return {}
    return result


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def manifests(raw: Path) -> list[Path]:
    directory = raw.parent
    candidates = [directory / "capture.manifest", directory / "manifest.txt"]
    candidates.extend(sorted(directory.glob("*.manifest")))
    return [candidate for candidate in candidates if candidate.is_file()]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cue", type=Path, required=True)
    parser.add_argument("roots", nargs="*", type=Path)
    args = parser.parse_args()
    if not args.cue.is_file():
        return 1
    cue_hash = digest(args.cue)
    roots = list(args.roots)
    if not roots:
        home = Path.home() / ".firestaff"
        roots = [
            home / "data/nexus/captures",
            home / "devtools/nexus-captures",
            home / "external/nexus-capture",
        ]
    seen: set[Path] = set()
    for root in roots:
        if not root.is_dir():
            continue
        for raw in sorted(root.rglob("*.raw")):
            resolved = raw.resolve()
            if resolved in seen:
                continue
            seen.add(resolved)
            try:
                size = raw.stat().st_size
            except OSError:
                continue
            for manifest in manifests(raw):
                receipt = fields(manifest)
                expected_hash = receipt.get("raw_sha256", "").lower()
                expected_size = receipt.get("raw_bytes", "")
                if (
                    receipt.get("capture_magic")
                    != "FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1"
                    or receipt.get("disc_sha256", "").lower() != cue_hash
                    or not re.fullmatch(r"[0-9a-f]{64}", expected_hash)
                    or not expected_size.isdigit()
                    or int(expected_size) != size
                    or digest(raw) != expected_hash
                    or not receipt.get("skip_frames", "").isdigit()
                    or not receipt.get("frame_limit", "").isdigit()
                    or int(receipt["frame_limit"]) < 1
                    or not receipt.get("press_start_frame", "").isdigit()
                    or not receipt.get("press_start_length", "").isdigit()
                ):
                    continue
                print(raw)
                break
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
