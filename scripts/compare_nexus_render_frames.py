#!/usr/bin/env python3
"""Compare external Nexus PPM renderer receipts without assigning semantics."""

import argparse
from pathlib import Path


def fail(message: str) -> None:
    raise SystemExit(f"NEXUS_RENDER_COMPARE_INVALID: {message}")


def ppm_payload(path: Path) -> bytes:
    try:
        data = path.read_bytes()
    except OSError as exc:
        fail(f"cannot read {path}: {exc}")
    if not data.startswith(b"P6\n"):
        fail(f"{path}: not a binary PPM")
    cursor = 0
    header = []
    while len(header) < 3:
        newline = data.find(b"\n", cursor)
        if newline < 0:
            fail(f"{path}: truncated PPM header")
        line = data[cursor:newline]
        cursor = newline + 1
        if line.startswith(b"#"):
            continue
        header.append(line)
    if header[0] != b"P6":
        fail(f"{path}: invalid PPM magic")
    try:
        width, height = (int(part) for part in header[1].split())
        maximum = int(header[2])
    except ValueError:
        fail(f"{path}: invalid PPM dimensions")
    if width <= 0 or height <= 0 or maximum != 255:
        fail(f"{path}: unsupported PPM geometry")
    payload = data[cursor:]
    if len(payload) != width * height * 3:
        fail(f"{path}: unexpected PPM payload length")
    return data[:cursor] + payload


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--control-dir", required=True, type=Path)
    parser.add_argument("--input-dir", required=True, type=Path)
    parser.add_argument("--frames", required=True,
                        help="comma-separated decimal frame numbers")
    args = parser.parse_args()
    try:
        frames = [int(part) for part in args.frames.split(",")]
    except ValueError:
        fail("--frames must contain decimal frame numbers")
    if not frames or any(frame < 0 for frame in frames) or len(set(frames)) != len(frames):
        fail("--frames must be a non-empty unique non-negative list")

    for frame in frames:
        name = f"frame-{frame:06d}.ppm"
        control = ppm_payload(args.control_dir / name)
        injected = ppm_payload(args.input_dir / name)
        if control != injected:
            changed = sum(a != b for a, b in zip(control, injected))
            fail(f"frame {frame}: renderer output differs ({changed} bytes)")
    print(f"frames={','.join(str(frame) for frame in frames)}")
    print("renderer_output=byte_identical")
    print("input_presentation_semantics=unbound")
    print("semantic_admission=blocked")


if __name__ == "__main__":
    main()
