#!/usr/bin/env python3
"""Compare an authentic Ymir gameplay priority stack with its screenshot."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from PIL import Image


WIDTH = 320
HEIGHT = 224
EXPECTED_LAYERS = {0: 60_375, 1: 3_954, 2: 148, 3: 7_203}
CAPTURE_EXPECTATIONS = {
    239: {
        "screenshot_sha256": "ffd622503bafd437e4f20513e13b7ea3983f1240c930b755ae4b24986e5c90bf",
        "stack_sha256": "1fcf190a75bc137f312160e92dfb4d5d0d0b2e1fa84144a035bac15ba974199b",
    },
    397: {
        "screenshot_sha256": "b03843d943bfade7b68d19c8f195896b59f8aa254c7a8bc549abe3b702d510cc",
        "stack_sha256": "402ca6d5d295214f68d574097753ee04c6194f42d82bb832bd902052bbbe97e3",
    },
    2173: {
        "screenshot_sha256": "742ea48a955f07c8490169c6b444b53fb947385ed97a3f1812b684b65b2a2872",
        "stack_sha256": "116b9ca3a19a6ce2642224d42efbf45a22f0d2481022ae7ba19b6bc100336fdf",
    },
}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("stack", type=Path, help="Raw layer,R,G,B stack capture")
    parser.add_argument("screenshot", type=Path, help="640x448 Ymir screenshot")
    parser.add_argument("--capture-frame", type=int, choices=CAPTURE_EXPECTATIONS,
                        required=True)
    args = parser.parse_args()
    screenshot_bytes = args.screenshot.read_bytes()
    digest = hashlib.sha256(screenshot_bytes).hexdigest()
    expected = CAPTURE_EXPECTATIONS[args.capture_frame]
    if digest != expected["screenshot_sha256"]:
        raise SystemExit(f"screenshot SHA-256 mismatch: {digest}")
    image = Image.open(args.screenshot).convert("RGB")
    if image.size != (WIDTH * 2, HEIGHT * 2):
        raise SystemExit(f"unexpected screenshot size: {image.size}")
    stack = args.stack.read_bytes()
    if len(stack) != WIDTH * HEIGHT * 4:
        raise SystemExit(f"unexpected stack byte count: {len(stack)}")
    stack_digest = hashlib.sha256(stack).hexdigest()
    if stack_digest != expected["stack_sha256"]:
        raise SystemExit(f"priority stack SHA-256 mismatch: {stack_digest}")

    counts = {layer: 0 for layer in EXPECTED_LAYERS}
    max_error = 0
    differing_samples = 0
    for y in range(HEIGHT):
        for x in range(WIDTH):
            offset = (y * WIDTH + x) * 4
            layer, red, green, blue = stack[offset : offset + 4]
            if layer not in counts:
                raise SystemExit(f"unexpected layer id {layer} at ({x}, {y})")
            counts[layer] += 1
            for dy in range(2):
                for dx in range(2):
                    expected = image.getpixel((x * 2 + dx, y * 2 + dy))
                    actual = (red, green, blue)
                    error = max(abs(a - b) for a, b in zip(actual, expected))
                    max_error = max(max_error, error)
                    differing_samples += error != 0
    if counts != EXPECTED_LAYERS:
        raise SystemExit(f"layer counts mismatch: {counts}")
    print(f"frame={args.capture_frame}; pixels={WIDTH * HEIGHT}; doubled_samples={WIDTH * HEIGHT * 4}")
    print(f"layer_counts={counts}; max_channel_error={max_error}; differing_samples={differing_samples}")
    return 0 if max_error == 0 and differing_samples == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
