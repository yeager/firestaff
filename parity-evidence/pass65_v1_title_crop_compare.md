# Pass 65 — crop clean DOSBox TITLE capture to 320×200

Date: 2026-04-25

## Scope

Pass 65 builds the smallest reliable bridge from the clean pass-64 DOSBox window captures to usable original TITLE evidence.

This pass does **not** claim raw DOSBox framebuffer capture or original cadence parity.  It deterministically crops the mode-13h game surface from the clean window captures, rescales it to 320×200, and compares the first clean original capture against Firestaff's source-backed TITLE renderer output.

## New harness

```sh
scripts/dosbox_dm1_title_crop_compare_pass65.py
```

Inputs:

- `verification-screens/pass64-clean-title-capture/title-clean-window-00.png`
- `verification-screens/pass64-clean-title-capture/title-clean-window-01.png`
- `verification-screens/pass64-clean-title-capture/dosbox-title-clean.log`
- `verification/pass58-title-render-dump/frame_*.ppm`

The script reads the DOSBox log's graphics-mode line:

```text
DISPLAY: VGA 320x200 256-colour graphics mode 13h ... scaled to 1067x667 pixels
```

Then it:

1. Uses the logged scaled mode-13h surface size (`1067×667`).
2. Centers that width inside the captured PNG (`x = 34`).
3. Scans legal vertical offsets and chooses the single deterministic offset with the lowest MAE against the already source-backed 53-frame Firestaff/Greatstone TITLE render dump.
4. Downscales the resulting game surface to 320×200 with nearest-neighbour sampling.
5. Normalizes only DOSBox near-black background pixels (`<16`) to black before metrics.
6. Writes cropped PPM/PNG evidence and a text report.

## Verification command and result

```sh
python3 -m py_compile scripts/dosbox_dm1_title_crop_compare_pass65.py
scripts/dosbox_dm1_title_crop_compare_pass65.py
```

Result report:

```text
verification-screens/pass65-title-crop-compare/pass65_crop_compare_report.md
```

Key results from that report:

```text
DOSBox log scaled surface: 1067x667
Rendered reference frames: 53 from verification/pass58-title-render-dump

title-clean-window-00.png -> crop 34,120,1067,667 -> best reference frame_0037.ppm
  MAE 1.693047, differing pixels 16585, max delta 12
  SHA-256 cropped PPM 965301f0786d829116ec53c81ad53efc8aa05cc9ec260113fd435b2692c63f6e

title-clean-window-01.png -> crop 34,120,1067,667 -> best reference frame_0037.ppm
  MAE 1.693047, differing pixels 16585, max delta 12
  SHA-256 cropped PPM 965301f0786d829116ec53c81ad53efc8aa05cc9ec260113fd435b2692c63f6e

Cropped frame 00 vs 01: MAE 0.000000, differing pixels 0, max delta 0.
Cropped frames are byte-identical after deterministic crop/downscale/near-black normalization.
```

Interpretation:

- The first clean original TITLE capture is now reducible to a reproducible 320×200 evidence frame.
- The best reference is `frame_0037.ppm` from the source-backed Firestaff/Greatstone TITLE render dump.
- The residual comparison delta is bounded to display/capture/downscale color differences (`max delta 12`), not a gross composition mismatch.
- Both pass-64 captures reduce to the same cropped frame, so the current local evidence is a clean still, not a cadence sample.

## Local evidence artifacts

These are intentionally left untracked:

- `verification-screens/pass65-title-crop-compare/pass65_crop_compare_report.md` — 1.4K
- `verification-screens/pass65-title-crop-compare/title-clean-window-00-cropped-320x200.ppm` — 188K
- `verification-screens/pass65-title-crop-compare/title-clean-window-00-cropped-320x200.png` — 9.9K
- `verification-screens/pass65-title-crop-compare/title-clean-window-01-cropped-320x200.ppm` — 188K
- `verification-screens/pass65-title-crop-compare/title-clean-window-01-cropped-320x200.png` — 9.9K

## Remaining gap

Cadence measurement is still blocked by the available capture set: pass 64 produced two byte-identical window captures and did not record per-frame timestamps.  Pass 65 therefore correctly avoids a cadence claim.

The next bounded step is either:

1. add timestamped targeted-window sequence capture around the same crop bridge; or
2. get native/raw DOSBox screenshots emitting files in this detached run.

Only after that should V1 TITLE frame cadence, palette-display timing, and title-menu handoff be measured against Firestaff's V1 frontend policy.
