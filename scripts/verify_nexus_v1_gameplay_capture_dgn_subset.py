#!/usr/bin/env python3
"""Verify the bounded DGN material subset in an authenticated Saturn capture.

This is evidence only.  It deliberately keeps semantic admission blocked:
the unmatched VDP1 commands are reported, not classified as HUD or menu.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--capture-frames", type=int, required=True)
    parser.add_argument("--expect-textured", type=int, required=True)
    parser.add_argument("--expect-source-matches", type=int, required=True)
    parser.add_argument("--expect-face-owners", type=int, required=True)
    parser.add_argument("--expect-unmatched", required=True)
    args = parser.parse_args()
    command = [
        sys.executable,
        str(Path(__file__).with_name("analyze_nexus_vdp1_dgn_command_sequence_join.py")),
        str(args.capture),
        "--data-dir", str(args.data_dir),
        "--frame", str(args.frame),
        "--capture-frames", str(args.capture_frames),
        "--require-complete",
    ]
    result = subprocess.run(command, capture_output=True, text=True, check=False)
    output = result.stdout + result.stderr
    print(output, end="")
    if result.returncode == 0:
        print("unexpected complete semantic join", file=sys.stderr)
        return 1
    summary = re.search(
        r"textured_draws=(\d+) source_matches=(\d+) .*face_owner_matches=(\d+)",
        output,
    )
    unmatched = re.search(r"unmatched_offsets=([^\n]+)", output)
    if not summary or not unmatched:
        print("missing DGN subset receipt", file=sys.stderr)
        return 1
    actual = tuple(int(value) for value in summary.groups())
    expected = (args.expect_textured, args.expect_source_matches, args.expect_face_owners)
    if actual != expected or unmatched.group(1).strip() != args.expect_unmatched:
        print(f"receipt mismatch: actual={actual} expected={expected}", file=sys.stderr)
        print(f"unmatched actual={unmatched.group(1).strip()!r} expected={args.expect_unmatched!r}", file=sys.stderr)
        return 1
    for required in ("sequence_dgn_material_join=unbound", "semantic_admission=blocked"):
        if required not in output:
            print(f"missing gate: {required}", file=sys.stderr)
            return 1
    print("NEXUS_V1_GAMEPLAY_CAPTURE_DGN_SUBSET: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
