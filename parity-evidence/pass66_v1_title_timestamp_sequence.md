# Pass 66 — timestamped DOSBox TITLE window sequence

Date: 2026-04-25

## Scope

Pass 66 adds a bounded timestamped targeted-window capture path for the original DM PC 3.4 TITLE runtime.  It builds directly on pass 64's CGWindowID capture and pass 65's crop/compare bridge.

This pass does **not** claim final original cadence parity.  The capture path uses host `screencapture`, so frame starts are sparse (~0.65 s apart in this run) and bounded by host window-capture overhead, not the raw DOSBox framebuffer clock.

## New harnesses

```sh
scripts/dosbox_dm1_title_sequence_capture_pass66.sh
scripts/dosbox_dm1_title_sequence_match_pass66.py
```

The capture wrapper:

1. launches DOSBox Staging with the staged DM1 PC 3.4 tree;
2. posts the original selector choices (`1`/Return repeated 3 times) directly to the DOSBox process;
3. finds the DOSBox Staging window by CGWindowID;
4. captures a short PNG still sequence;
5. writes `manifest.tsv` with before/after UTC timestamps, epoch-ns timings, file paths, SHA-256, and byte sizes.

The match bridge:

1. parses the DOSBox graphics-mode log (`320x200 mode 13h`, scaled to `1067x667`);
2. crops/downscales each targeted window capture with the pass-65 deterministic bridge;
3. verifies the manifest SHA-256 for each source PNG;
4. compares every crop to the 53-frame source-backed Firestaff/Greatstone TITLE render dump;
5. marks low-residual rows within the pass-65 tolerance envelope (`MAE <= 2.0`, `max delta <= 12`) as `TITLE-like` and treats larger residuals as transition/non-TITLE evidence rather than cadence proof.

## Verification commands and results

```sh
python3 -m py_compile scripts/dosbox_dm1_title_sequence_match_pass66.py
scripts/dosbox_dm1_title_sequence_capture_pass66.sh --dry-run
CAPTURE_COUNT=16 WAIT_AFTER_INPUT_MS=5000 WAIT_BETWEEN_CAPTURES_MS=250 scripts/dosbox_dm1_title_sequence_capture_pass66.sh --run
scripts/dosbox_dm1_title_sequence_match_pass66.py
```

Result report:

```text
verification-screens/pass66-title-timestamp-sequence-match/pass66_sequence_match_report.md
```

Key capture facts from that report:

```text
DOSBox log scaled surface: 1067x667
Rendered reference frames: 53 from verification/pass58-title-render-dump
Window captures: 16 total, 6 unique SHA-256 values
Cropped normalized frames: 6 unique SHA-256 values
TITLE-like matches at pass-65 tolerance: 7 frames, 3 unique crops, 3 unique best references
TITLE-like best references: frame_0037.ppm, frame_0039.ppm, frame_0046.ppm
```

The first seven rows are low-residual TITLE-like matches:

```text
00  2026-04-25T04:08:38.349507Z  frame_0039.ppm  MAE 1.694479  max delta 12
01  2026-04-25T04:08:39.050087Z  frame_0046.ppm  MAE 1.692990  max delta 12
02  2026-04-25T04:08:39.701415Z  frame_0037.ppm  MAE 1.693047  max delta 12
03  2026-04-25T04:08:40.355621Z  frame_0037.ppm  MAE 1.693047  max delta 12
04  2026-04-25T04:08:41.011888Z  frame_0037.ppm  MAE 1.693047  max delta 12
05  2026-04-25T04:08:41.577503Z  frame_0037.ppm  MAE 1.693047  max delta 12
06  2026-04-25T04:08:42.230552Z  frame_0037.ppm  MAE 1.693047  max delta 12
```

Rows 07–15 have high residuals (`MAE` roughly 16–53, `max delta 255`) and are therefore recorded as transition/non-TITLE rows, not as source-backed TITLE cadence samples.

## Conservative interpretation

The targeted-window sequence path now captures timestamped changes.  It produced several low-residual TITLE-like samples and then high-residual transition/non-TITLE samples.  That is enough to prove the pass-64/pass-65 bridge can be driven as a timestamped sequence, but it is **not** enough to lock original cadence.

Reasons cadence remains unclaimed:

- host `screencapture -l` took about 120–133 ms per still and the observed start deltas were about 0.65 s despite the requested 250 ms gap;
- the TITLE-like best-reference sequence is sparse and not monotonic (`frame_0039`, `frame_0046`, then `frame_0037` hold), which is a sampling/correlation warning rather than a clean animation timing signal;
- later rows no longer match the source-backed TITLE dump within the pass-65 tolerance envelope, so they cannot be used as TITLE frame cadence evidence.

Next useful route: either native/raw DOSBox image capture with reliable per-frame output, or a faster launch-time targeted-window capture path that starts earlier and proves a monotonic source-frame sequence before any final frontend cadence/handoff claim.

## Local evidence artifacts

These are intentionally left untracked:

- `verification-screens/pass66-title-timestamp-sequence/` — 3.1M total
  - `manifest.tsv` — 4.8K
  - `dosbox-title-sequence.log` — 5.8K
  - `title-seq-window-00.png` … `title-seq-window-15.png` — 84K–256K each
- `verification-screens/pass66-title-timestamp-sequence-match/` — 2.9M total
  - `pass66_sequence_match_report.md` — 8.5K
  - `title-seq-window-00-cropped-320x200.ppm` … `title-seq-window-15-cropped-320x200.ppm` — 188K each

## Remaining gap

V1 TITLE animation timing, palette-display timing, and title-menu handoff remain `KNOWN_DIFF`/unclaimed until a raw or sufficiently dense original-runtime sequence produces a robust monotonic timing sample against the 53 source-backed TITLE frames.
