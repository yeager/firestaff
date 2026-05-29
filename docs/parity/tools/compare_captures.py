#!/usr/bin/env python3
"""compare_captures.py — pixel-diff two 320x200 (or cropped viewport) captures."""
from PIL import Image
import numpy as np
import sys
from pathlib import Path

_THRESHOLD_MAE = 5.0
_THRESHOLD_MAX = 20.0

def load_normalize(path):
    """Load image, ensure 320x200 RGB, return np array."""
    img = Image.open(path).convert("RGB")
    if img.size != (320, 200):
        img = img.resize((320, 200), Image.LONEST)
    return np.array(img)

def compare(orig_path: str, fires_path: str, label: str = "compare",
           crop: bool = True, viewport_box: tuple = (0, 33, 224, 136)) -> bool:
    """
    Compare two captures.

    Args:
        orig_path:   Path to original DOSBox PNG
        fires_path:   Path to Firestaff PPM or PNG
        label:       Label for output
        crop:        If True, crop both to viewport region before comparing
        viewport_box: (x, y, w, h) viewport area
    """
    orig  = load_normalize(orig_path)
    fires = load_normalize(fires_path)

    if crop:
        x, y, w, h = viewport_box
        orig  = orig[y:y+h, x:x+w]
        fires = fires[y:y+h, x:x+w]

    assert orig.shape == fires.shape, \
        f"Shape mismatch after crop: {orig.shape} vs {fires.shape}"

    diff  = np.abs(orig.astype(int) - fires.astype(int))
    mae   = diff.mean()
    max_d = diff.max()
    # Per-channel max
    max_r, max_g, max_b = diff[..., 0].max(), diff[..., 1].max(), diff[..., 2].max()

    ok = mae < _THRESHOLD_MAE and max_d < _THRESHOLD_MAX
    status = "PASS" if ok else "FAIL"
    print(f"{label}: MAE={mae:.4f} max={max_d}  R/G/Bmax={max_r}/{max_g}/{max_b}  [{status}]")

    if not ok:
        # Give hints
        if mae >= _THRESHOLD_MAE:
            print(f"  MAE {mae:.2f} >= threshold {_THRESHOLD_MAE} — colour/shading difference")
        if max_d >= _THRESHOLD_MAX:
            print(f"  Max delta {max_d} >= threshold {_THRESHOLD_MAX} — single pixel outlier")

    return ok

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} original.png firestaff.ppm [label] [--no-crop]")
        sys.exit(1)

    orig   = sys.argv[1]
    fires  = sys.argv[2]
    label  = sys.argv[3] if len(sys.argv) > 3 else "compare"
    crop   = "--no-crop" not in sys.argv

    ok = compare(orig, fires, label, crop=crop)
    sys.exit(0 if ok else 1)
