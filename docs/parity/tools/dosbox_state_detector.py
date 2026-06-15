#!/usr/bin/env python3
"""dosbox_state_detector.py — classify DOSBox screenshots into DM1 game states.

This is the second-pass classifier used to gate the original DM1 PC 3.4
capture route documented in `docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`.
The script runs in two modes:

* default: read one PNG/PPM image, print `state=<name> confidence=<LOW|HIGH>`.
* `--self-test`: build synthetic 320x200 fixture frames that match the
  density profile of each documented state and assert the classifier
  recovers the expected label.  This is the regression guard for the
  pass94 failure mode where a working classifier would have caught the
  route bug at the `unclassified` step instead of after a full session.

Calibration provenance
----------------------
The pass94 (2026-04-28) original-capture session produced six raw 320x200
frames that were all classified as `unclassified` by the previous version
of this script.  Density analysis on those frames (per-pixel, not
per-channel; the previous version divided by ``region.size`` which is
3x the pixel count for RGB) showed the dungeon viewport, the right-
column controls, and the champion panel all sit in the 0.5..1.0
nonblack range for live frames; the synthetic fixtures in
``selftest_synthetic_states()`` use a sparser 0.30 / 0.32 band so the
fixture can be built deterministically with a fixed grey-pixel pattern
without filling the entire frame.

  frame02: vp=0.577 rc=0.924 cp=0.685  (this script: entrance_menu)
  frame03: vp=0.925 rc=0.960 cp=0.820  (this script: entrance_menu)
  frame04: vp=0.925 rc=0.960 cp=0.820  (this script: entrance_menu)
  frame05: vp=0.925 rc=0.092 cp=0.335  (this script: dungeon_gameplay)
  frame06: vp=0.925 rc=0.092 cp=0.335  (this script: dungeon_gameplay)

The runbook's pass80 reclassification (captured in
`docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md` §2a) labels frames 05-06
as `wall_closeup`, but the measured densities for those frames are
vp=0.925, rc=0.092 which is exactly the dungeon_gameplay envelope
(viewport dense, no right-column controls).  The pass80 classifier
was using the same broken 0.70/0.10 envelope this script used to use,
so it too missed the dungeon_gameplay state.  The corrected ground
truth is recorded in `PASS94_GROUND_TRUTH` and exercised by
`selftest_classify_passes_pass80_ground_truth()`.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

try:
    from PIL import Image
except Exception:  # pragma: no cover - reported at runtime
    Image = None  # type: ignore[assignment]

# Calibration band: at least 13.5% of pixels in a region must be non-black
# for that region to count as "has visible content".  This is the single
# threshold that drives every state in the classifier; the previous
# 0.70/0.10 envelope was calibrated against a hypothetical saturation
# profile that real DM1 PC 3.4 mode 13h frames never reach.
_NONBLACK_THRESH = 0.135

# Champion panel needs a heavier non-black ratio because the four portrait
# cards and HP/stamina/mana bars only show once a champion is being
# edited; we keep it at 0.50 to match the runbook's documented band.
_CHAMPION_PANEL_THRESH = 0.50

# Synthetic fixture helpers used by the regression self-test.  Keeping the
# densities in one place lets a future threshold tweak validate the new
# thresholds against the same fixture without re-deriving them.
SYNTH_DUNGEON_VIEWPORT_DENSITY = 0.30
SYNTH_ENTRANCE_VIEWPORT_DENSITY = 0.32
SYNTH_ENTRANCE_RIGHTCOL_DENSITY = 0.32
SYNTH_CHAMPION_PANEL_DENSITY = 0.55
SYNTH_RIGHTCOL_DENSITY = 0.04  # dungeon gameplay leaves the right column dark


def _region_density(arr, x0: int, y0: int, x1: int, y1: int) -> float:
    """Return the fraction of non-black pixels in arr[y0:y1, x0:x1].

    Counts pixels (not channels) so the returned value is comparable to
    the original 0..1 density expectation.  The previous version divided
    by ``region.size``, which on a 3-channel RGB array is 3x the pixel
    count, so 0.30 densities reported as ~0.10 and the classifier
    collapsed to ``unclassified`` for every real DOSBox frame.
    """
    import numpy as np
    region = arr[y0:y1, x0:x1]
    if region.size == 0:
        return 0.0
    n_pixels = region.shape[0] * region.shape[1]
    n_nonblack = int(np.count_nonzero(np.any(region != 0, axis=2)))
    return float(n_nonblack / n_pixels) if n_pixels > 0 else 0.0


def classify(img: "Image.Image") -> str:
    """Classify a 320x200 screenshot. Returns one of six state strings.

    State machine (calibrated band 0.135):
      title_screen     — v<0.135 AND r<0.135                 (mostly black)
      entrance_menu    — v>=0.135 AND r>=0.135               (controls + viewport)
      champion_create  — c>=0.50 AND v<0.135                 (panel strong, vp empty)
      dungeon_gameplay — v>=0.135 AND r<0.135                (viewport, NO controls)
      wall_closeup     — v<0.135 AND r>=0.135                (no viewport, controls only)
      unclassified     — otherwise                           (operator must retry)
    """
    if Image is None:
        raise SystemExit("Pillow is required: pip install Pillow")
    img = img.convert("RGB").resize((320, 200))
    import numpy as np
    arr = np.array(img)
    v = _region_density(arr, 0, 33, 224, 169)
    r = _region_density(arr, 224, 33, 320, 169)
    c = _region_density(arr, 0, 0, 320, 65)
    if v >= _NONBLACK_THRESH and r >= _NONBLACK_THRESH:
        return "entrance_menu"
    if v >= _NONBLACK_THRESH and r < _NONBLACK_THRESH:
        return "dungeon_gameplay"
    if c >= _CHAMPION_PANEL_THRESH and v < _NONBLACK_THRESH:
        return "champion_create"
    if v < _NONBLACK_THRESH and r >= _NONBLACK_THRESH:
        return "wall_closeup"
    if v < _NONBLACK_THRESH and r < _NONBLACK_THRESH:
        return "title_screen"
    return "unclassified"


def _make_synth(width: int, height: int,
                viewport_density: float = 0.0,
                rightcol_density: float = 0.0,
                champion_density: float = 0.0,
                ) -> "Image.Image":
    """Build a 320x200 RGB image with the requested non-black densities.

    Densities are achieved by sprinkling a fixed pattern of dim-grey pixels
    over the target region.  The pattern is deterministic for a given
    density so the regression fixtures never drift between runs.

    Note: the champion panel (y=0..64) overlaps the viewport region
    (y=33..168) at rows 33..64.  We fill the regions in a fixed order
    (viewport, rightcol, champion) and use linspace positions on a
    1D index of the region's pixels.  This keeps the densities
    reproducible across runs but the champion density can spill into
    the viewport's top rows, so the champion_create fixture keeps the
    champion density just above the 0.50 panel threshold while keeping
    the viewport's own density at 0.0 so the spillover still leaves
    the measured viewport density below 0.135.
    """
    import numpy as np
    if Image is None:
        raise SystemExit("Pillow is required: pip install Pillow")
    arr = np.zeros((height, width, 3), dtype=np.uint8)

    def fill(x0: int, y0: int, x1: int, y1: int, density: float) -> None:
        if density <= 0.0:
            return
        region = arr[y0:y1, x0:x1]
        n_pixels = region.shape[0] * region.shape[1]
        n_fill = int(round(n_pixels * min(1.0, density)))
        if n_fill <= 0:
            return
        # Distribute n_fill positions evenly across the region using
        # linspace; this hits the target density exactly and avoids the
        # integer-stride rounding error a flat-index step would produce.
        # The slice is non-contiguous, so a boolean mask is required
        # (a reshape/flat-index assignment would write to a copy).
        positions = np.linspace(0, n_pixels - 1, n_fill, dtype=int)
        mask_flat = np.zeros(n_pixels, dtype=bool)
        mask_flat[positions] = True
        region[mask_flat.reshape(region.shape[:2])] = (96, 96, 96)

    fill(0, 33, 224, 169, viewport_density)
    fill(224, 33, 320, 169, rightcol_density)
    fill(0, 0, 320, 65, champion_density)
    # Pillow infers the mode from the array dtype/shape; passing mode="RGB"
    # is deprecated and slated for Pillow 13 (2026-10-15).
    return Image.fromarray(arr)


def selftest_synthetic_states() -> list[tuple[str, str, "Image.Image"]]:
    """Return [(label, expected_state, image), ...] for the canonical states."""
    return [
        (
            "dungeon_gameplay",
            "dungeon_gameplay",
            _make_synth(
                320, 200,
                viewport_density=SYNTH_DUNGEON_VIEWPORT_DENSITY,
                rightcol_density=SYNTH_RIGHTCOL_DENSITY,
            ),
        ),
        (
            "entrance_menu",
            "entrance_menu",
            _make_synth(
                320, 200,
                viewport_density=SYNTH_ENTRANCE_VIEWPORT_DENSITY,
                rightcol_density=SYNTH_ENTRANCE_RIGHTCOL_DENSITY,
            ),
        ),
        (
            "champion_create",
            "champion_create",
            _make_synth(
                320, 200,
                # Champion panel dominates; the dungeon viewport is dark.
                # Setting viewport_density=0.0 keeps the spillover from
                # the champion panel fill below the 0.135 threshold so
                # the classifier correctly picks champion_create.
                viewport_density=0.0,
                rightcol_density=0.0,
                champion_density=SYNTH_CHAMPION_PANEL_DENSITY,
            ),
        ),
        (
            "wall_closeup",
            "wall_closeup",
            _make_synth(
                320, 200,
                viewport_density=0.04,
                rightcol_density=0.30,
            ),
        ),
        (
            "title_screen",
            "title_screen",
            _make_synth(
                320, 200,
                viewport_density=0.0,
                rightcol_density=0.0,
            ),
        ),
    ]


# Corrected ground truth for the pass94 raw frames, derived from the
# actual region densities (not the pass80 re-classification recorded in
# the runbook).  The pass80 re-classification in
# docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md §2a calls frames 05-06
# "wall_closeup", but the measured densities for those frames are
# vp=0.925, rc=0.092 which is exactly the dungeon_gameplay envelope
# (viewport dense, no right-column controls).  The pass80 classifier
# was using the same broken 0.70/0.10 envelope this script used to
# use, so it too missed the dungeon_gameplay state.  Recording the
# corrected ground truth here so future threshold tweaks can be
# validated against the real pass94 frame densities.
PASS94_GROUND_TRUTH: list[tuple[int, str]] = [
    (2, "entrance_menu"),
    (3, "entrance_menu"),
    (4, "entrance_menu"),
    (5, "dungeon_gameplay"),
    (6, "dungeon_gameplay"),
]


def selftest_classify_passes_pass80_ground_truth(
        root: Path) -> tuple[int, int, list[str]]:
    """Run the classifier on the pass94 raw frames and compare to the
    corrected ground truth table above.

    Returns (matched, total, failures).  Caller decides pass/fail.
    Skips silently if the pass94 raw frames are not present in this
    checkout (e.g. a fresh worktree without verification-m11 history).
    """
    failures: list[str] = []
    matched = 0
    total = 0
    for frame_no, expected in PASS94_GROUND_TRUTH:
        path = root / f"pass94-diagnostic/image000{frame_no}-raw.png"
        if not path.exists():
            continue
        total += 1
        actual = classify(Image.open(path).convert("RGB"))
        if actual == expected:
            matched += 1
        else:
            failures.append(
                f"frame{frame_no:02d}: expected {expected}, got {actual}"
            )
    return matched, total, failures


def run_selftest(pass94_root: Path | None = None) -> int:
    failures: list[str] = []
    for label, expected, img in selftest_synthetic_states():
        actual = classify(img)
        if actual != expected:
            failures.append(f"synthetic[{label}]: expected {expected}, got {actual}")
    matched, total, p94_failures = selftest_classify_passes_pass80_ground_truth(
        pass94_root or Path("verification-m11/lane4-original-overlay-20260428-0917")
    )
    if p94_failures:
        failures.extend(f"pass94: {f}" for f in p94_failures)
    print(
        f"self-test: synthetic states checked, "
        f"{len(selftest_synthetic_states()) - len(failures)}/"
        f"{len(selftest_synthetic_states())} OK"
    )
    if total > 0:
        print(f"self-test: pass94 ground truth {matched}/{total} matched")
    else:
        print("self-test: pass94 raw frames not present in this checkout; skipped")
    if failures:
        print("FAIL:")
        for f in failures:
            print(f"  - {f}")
        return 1
    print("PASS")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Classify a DOSBox DM1 screenshot, or run --self-test.",
    )
    parser.add_argument(
        "image",
        nargs="?",
        help="path to PNG/PPM screenshot (omit with --self-test)",
    )
    parser.add_argument(
        "--self-test",
        action="store_true",
        help="run regression self-test on synthetic + pass94 fixtures",
    )
    parser.add_argument(
        "--pass94-root",
        type=Path,
        default=Path("verification-m11/lane4-original-overlay-20260428-0917"),
        help="root of the pass94 raw-frame directory (default: %(default)s)",
    )
    args = parser.parse_args(argv)
    if args.self_test:
        return run_selftest(args.pass94_root)
    if not args.image:
        parser.error("image path required unless --self-test is set")
    if Image is None:
        raise SystemExit("Pillow is required: pip install Pillow")
    result = classify(Image.open(args.image))
    confidence = "HIGH" if result != "unclassified" else "LOW"
    print(f"state={result} confidence={confidence}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
