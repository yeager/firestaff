# Pass 67 — native DOSBox raw TITLE screenshot route

Date: 2026-04-25

## Scope

Pass 67 investigates DOSBox Staging's native screenshot path as the next route after pass 66's sparse host-window sequence.  The goal was to determine whether the original DM PC 3.4 runtime can produce raw 320x200 framebuffer screenshots that match the existing source-backed Firestaff/Greatstone TITLE frame dump.

This pass does **not** claim final original cadence parity.  It proves native raw still capture works, then documents why the built-in screenshot hotkey is still too blocking/sparse for a robust monotonic timing trace.

## New harnesses

```sh
scripts/dosbox_dm1_native_title_capture_pass67.sh
scripts/dosbox_dm1_native_title_match_pass67.py
```

The capture wrapper:

1. launches DOSBox Staging 0.82.2 with `capture_dir` set to `verification-screens/pass67-native-title-raw-sequence/`;
2. sets `default_image_capture_formats=raw`, so the default screenshot action writes `imageNNNN-raw.png` raw framebuffer PNGs;
3. posts the original selector choices (`1`/Return repeated 3 times) directly to the DOSBox process;
4. waits a tunable offset after selector handoff;
5. triggers DOSBox Staging's macOS screenshot mapper action (`Cmd+F5`) by posting an actual Command-key down event plus F5 to the DOSBox PID;
6. writes `manifest.tsv` with file paths, UTC mtimes, SHA-256, byte sizes, and dimensions.

Important mapper finding: setting a Command flag on F5 alone was not reliable.  The working path sends a standalone Command-key down event, then F5 down/up with `.maskCommand`, then Command up.

The match bridge:

1. validates manifest SHA-256 and PNG dimensions;
2. compares each native raw 320x200 screenshot directly to the 53 source-backed Firestaff/Greatstone TITLE frames from `verification/pass58-title-render-dump/`;
3. marks rows within the pass-65 tolerance envelope (`MAE <= 2.0`, `max delta <= 12`) as `TITLE-like`;
4. records adjacent-frame deltas and a precise blocker for cadence.

## Verification commands and results

```sh
python3 -m py_compile scripts/dosbox_dm1_native_title_match_pass67.py
scripts/dosbox_dm1_native_title_capture_pass67.sh --dry-run
SHOT_COUNT=18 WAIT_AFTER_INPUT_MS=3600 WAIT_BETWEEN_SHOTS_MS=20 scripts/dosbox_dm1_native_title_capture_pass67.sh --run
scripts/dosbox_dm1_native_title_match_pass67.py
```

Result report:

```text
verification-screens/pass67-native-title-raw-sequence-match/pass67_native_raw_sequence_match_report.md
```

Key capture facts from that report:

```text
Native raw screenshots: 18 total, 5 unique SHA-256 values
320x200 raw framebuffer screenshots: 18/18
TITLE-like matches at pass-65 tolerance: 7 frames, 1 unique images, 1 unique best references
TITLE-like best reference: frame_0012.ppm
```

The first seven rows are native raw 320x200 TITLE-like matches:

```text
00  2026-04-25T04:29:53.196277Z  frame_0012.ppm  MAE 1.355042  max delta 12
01  2026-04-25T04:29:53.566074Z  frame_0012.ppm  MAE 1.355042  max delta 12
02  2026-04-25T04:29:54.009328Z  frame_0012.ppm  MAE 1.355042  max delta 12
03  2026-04-25T04:29:54.399667Z  frame_0012.ppm  MAE 1.355042  max delta 12
04  2026-04-25T04:29:54.835618Z  frame_0012.ppm  MAE 1.355042  max delta 12
05  2026-04-25T04:29:55.374568Z  frame_0012.ppm  MAE 1.355042  max delta 12
06  2026-04-25T04:29:55.929727Z  frame_0012.ppm  MAE 1.355042  max delta 12
```

Rows 07–09 are high-residual transition/black rows, and rows 10–17 are the entrance/menu frame.  They are not source-backed TITLE cadence samples.

A second tuning run before the final retained evidence used `WAIT_AFTER_INPUT_MS=4800`; it hit `frame_0030.ppm` repeatedly with the same pass-65 tolerance profile.  That confirms the wait offset can target different source-backed TITLE stills, but it also confirms that Cmd+F5 is not dense enough to capture monotonic motion within one run.

## Conservative interpretation

Native DOSBox Staging raw screenshots are now proven useful for still-frame identity evidence:

- every retained screenshot is a native 320x200 raw framebuffer PNG, not a host-window crop;
- low-residual native frames match the source-backed TITLE dump more tightly than the pass-65/pass-66 host-window path (`MAE 1.355` vs about `1.69`, still with max delta `12`);
- the route cleanly avoids host window scaling/cropping uncertainty.

Cadence remains unclaimed:

- despite a requested 20 ms gap, the observed written-image deltas are hundreds of milliseconds (`~327–672 ms` in the retained run);
- the TITLE-like portion is one repeated source frame (`frame_0012.ppm`) followed by transition/black/menu rows;
- tuned runs can hit different source frames, but not as a single robust monotonic sequence.

## Precise remaining blocker

DOSBox Staging's mapper-triggered screenshot action is blocking/sparse on this host.  It is good enough to capture raw stills, but it cannot prove original TITLE animation cadence because it misses the short source-frame movement and transition windows.

## Next bounded route

Bypass the interactive screenshot action:

1. instrument DOSBox Staging/SDL locally to dump the raw 320x200 framebuffer once per presented VGA frame;
2. or use an emulator/debugger with frame-dump support, such as DOSBox-X or a custom Staging build;
3. in parallel, source-prove the PC conditional timing from original/ReDMCSB `TITLE.C`/PC video timing code, using native screenshots only as still-frame identity evidence.

Relevant ReDMCSB timing reconnaissance:

```text
Toolchains/Common/Source/TITLE.C includes the TITLE zoom/palette sequence and explicit vertical-blank waits around the 18 zoom steps plus the final post-title handoff wait.
```

That file still needs PC-target conditional isolation before it can close timing on its own.

## Local evidence artifacts

These are intentionally left untracked:

- `verification-screens/pass67-native-title-raw-sequence/` — 248K total
  - `manifest.tsv` — 4.5K
  - `dosbox-native-title.log` — 9.1K
  - `image0001-raw.png` … `image0018-raw.png` — 434B–15K each
- `verification-screens/pass67-native-title-raw-sequence-match/` — 12K total
  - `pass67_native_raw_sequence_match_report.md` — 8.6K

## Remaining gap

V1 TITLE animation timing, palette-display timing, and title-menu handoff remain `KNOWN_DIFF`/unclaimed until a per-presented-frame emulator dump or source-proven PC timing path produces a robust monotonic timing sample against the 53 source-backed TITLE frames.
