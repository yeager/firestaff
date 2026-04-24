#!/usr/bin/env python3
"""Pass 57 TITLE renderer vs Greatstone PNG comparison.

The C renderer decodes the local original TITLE binary into temporary PPM frames.
This probe only uses the Greatstone source PNGs as an external facit for pixel
comparison; it does not make V2/upscaled assets part of V1 runtime data.
"""

from __future__ import annotations

import hashlib
import sys
from pathlib import Path
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REF = ROOT / "v2-assets" / "title-animation" / "source-greatstone"


def digest_image(path: Path) -> bytes:
    return hashlib.sha256(Image.open(path).convert("RGB").tobytes()).digest()


def main() -> int:
    if len(sys.argv) < 2:
        print("FAIL P57_TITLE_PNG_00 missing rendered PPM directory argument")
        return 1
    render_dir = Path(sys.argv[1])
    ref_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_REF
    if not ref_dir.is_dir():
        print(f"SKIP P57_TITLE_PNG_00 Greatstone source PNG dir not present: {ref_dir}")
        return 0

    refs = [p for p in sorted(ref_dir.glob("*.png")) if p.name[5:7] in {"EN", "DL"}]
    ppms = sorted(render_dir.glob("frame_*.ppm"))
    passed = total = 0

    def record(name: str, ok: bool, msg: str) -> None:
        nonlocal passed, total
        total += 1
        if ok:
            passed += 1
            print(f"PASS {name} {msg}")
        else:
            print(f"FAIL {name} {msg}")

    record("P57_TITLE_PNG_01", len(refs) == 53 and len(ppms) == 53,
           "Greatstone EN/DL references and rendered TITLE outputs both contain 53 frames")

    order_ok = True
    dimensions_ok = True
    pixels_ok = True
    mismatch = ""
    digest = hashlib.sha256()
    for ordinal, (ref, ppm) in enumerate(zip(refs, ppms), 1):
        order_ok = order_ok and ppm.name == f"frame_{ordinal:04d}.ppm"
        ref_img = Image.open(ref).convert("RGB")
        ppm_img = Image.open(ppm).convert("RGB")
        dimensions_ok = dimensions_ok and ref_img.size == (320, 200) and ppm_img.size == (320, 200)
        diff = ImageChops.difference(ref_img, ppm_img)
        bbox = diff.getbbox()
        if bbox is not None and pixels_ok:
            pixels_ok = False
            mismatch = f"first mismatch frame {ordinal} vs {ref.name}, bbox={bbox}"
        digest.update(f"{ordinal:04d}:{ref.name}:".encode("ascii"))
        digest.update(digest_image(ppm))

    record("P57_TITLE_PNG_02", order_ok and dimensions_ok,
           "rendered frame ordinal order and 320x200 dimensions match Greatstone EN/DL item order")
    record("P57_TITLE_PNG_03", pixels_ok,
           "rendered TITLE RGB pixels are identical to Greatstone source PNGs" if pixels_ok else mismatch)
    sha = digest.hexdigest()
    record("P57_TITLE_PNG_04", sha == "248136ab58a8a3c8b57571d5cbe72be0ac09fc60a3f22761a0a9bfd126773cb6",
           "pass-57 rendered frame tree fingerprint matches expected Greatstone-order RGB digest")
    print(f"# pass57 rendered title rgb tree sha256: {sha}")
    print(f"# summary: {passed}/{total} invariants passed")
    return 0 if passed == total else 1


if __name__ == "__main__":
    raise SystemExit(main())
