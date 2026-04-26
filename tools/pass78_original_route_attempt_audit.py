#!/usr/bin/env python3
"""Audit original-DOS route capture attempts for pass 78.

The goal is honesty: before treating original screenshots as viewport/gameplay
references, verify that the raw captures are 320x200 graphics frames.  Text-mode
720x400 selector/prompt frames are a blocker, not reference evidence.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def png_dims(path: Path) -> tuple[int, int]:
    data = path.read_bytes()[:32]
    if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        raise SystemExit(f"not PNG/IHDR: {path}")
    return struct.unpack(">II", data[16:24])


def display(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("attempt_dir", type=Path)
    ap.add_argument("--out-json", type=Path, default=None)
    args = ap.parse_args(argv)
    paths = sorted(args.attempt_dir.glob("image*.png"))
    rows = []
    dims_seen = {}
    for path in paths:
        w, h = png_dims(path)
        dims_seen[f"{w}x{h}"] = dims_seen.get(f"{w}x{h}", 0) + 1
        rows.append({
            "file": display(path),
            "width": w,
            "height": h,
            "bytes": path.stat().st_size,
            "sha256": sha256(path),
            "classification": "gameplay_graphics_candidate" if (w, h) == (320, 200) else "text_or_non_gameplay_blocker",
        })
    result = {
        "schema": "pass78_original_route_attempt_audit.v1",
        "attempt_dir": display(args.attempt_dir),
        "capture_count": len(rows),
        "dimensions_seen": dims_seen,
        "all_gameplay_320x200": bool(rows) and all(r["width"] == 320 and r["height"] == 200 for r in rows),
        "honesty": "Only 320x200 raw DOSBox graphics frames may be normalized as original DM1 viewport references. 720x400 text-mode selector/prompt captures are blockers.",
        "captures": rows,
    }
    out = args.out_json or (args.attempt_dir / "pass78_original_route_attempt_audit.json")
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({"captures": len(rows), "dimensions_seen": dims_seen, "all_gameplay_320x200": result["all_gameplay_320x200"], "out": display(out)}, indent=2))
    return 0 if result["all_gameplay_320x200"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
