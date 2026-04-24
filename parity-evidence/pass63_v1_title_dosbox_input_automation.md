# Pass 63 — DOSBox TITLE selector input automation

Date: 2026-04-24

## Scope

Pass 62 established that the original DM PC 3.4 runtime could be launched under DOSBox Staging, but non-interactive DOS input redirection was not a clean path through the text setup selector.  Pass 63 investigated permitted host-side input automation on this Mac and adds a reproducible wrapper that reaches the graphical original TITLE screen.

This pass does **not** claim TITLE cadence parity yet.  It only unblocks the selector handoff needed for later bounded timing/pixel capture.

## New harness

```sh
scripts/dosbox_dm1_title_input_pass63.sh --dry-run
scripts/dosbox_dm1_title_input_pass63.sh --run
```

The wrapper writes a deterministic DOSBox Staging config under:

```text
verification-screens/pass63-dosbox-title-automation/dosbox-title-input.conf
```

Then `--run` launches DOSBox Staging and uses the already-installed macOS `cliclick` utility to type the setup sequence:

```text
1 <Return>  # VGA Graphics
1 <Return>  # No Sound
1 <Return>  # Mouse
```

The sequence is intentionally explicit rather than hidden in a DOS redirection file, because the original selector advances through graphics, sound, and input menus before entering the graphical runtime.

## Methods investigated

| Method | Result | Notes |
|--------|--------|-------|
| AppleScript `System Events` keystroke | blocked | `osascript` activation can find `DOSBox Staging`, but sending `System Events` keystrokes failed with macOS error `-1743` (`Not authorized to send Apple events to System Events`). |
| `cliclick t:1` only | partial | Types `1` into the graphics prompt, but the selector needs `<Return>` before advancing. |
| `cliclick t:1 kp:return` | partial | Advances from graphics selection to sound selection. |
| `cliclick t:1 kp:return t:1 kp:return` | partial | Advances to input/controller selection. |
| `cliclick t:1 kp:return t:1 kp:return t:1 kp:return` | success | Reaches the graphical original `Dungeon Master` TITLE screen under DOSBox Staging. |

This keeps Pass 62's DOS redirection findings intact: redirection remains a bad selector automation path; host keystroke injection is the stable path available on this Mac.

## Verification commands and results

Dry run:

```sh
scripts/dosbox_dm1_title_input_pass63.sh --dry-run
```

Result:

```text
[pass-63] wrote .../verification-screens/pass63-dosbox-title-automation/dosbox-title-input.conf
[pass-63] cliclick sequence: t:1 kp:return t:1 kp:return t:1 kp:return
```

Actual run:

```sh
scripts/dosbox_dm1_title_input_pass63.sh --run
```

Result:

```text
[pass-63] wrote .../verification-screens/pass63-dosbox-title-automation/dosbox-title-input.conf
-rw-------@ 1 bosse  staff   966K Apr 24 21:56 .../verification-screens/pass63-dosbox-title-automation/title-after-selector.png
```

Visual inspection of `title-after-selector.png` shows the original graphical `Dungeon Master` title screen in DOSBox Staging after the three selector choices.

## Local evidence artifacts

These are intentionally left untracked unless a later pass curates smaller evidence:

- `verification-screens/pass63-dosbox-title-automation/title-after-selector.png` — 966K; host screenshot showing original graphical TITLE reached.
- `verification-screens/pass63-dosbox-title-automation/cliclick-title-input.log` — typed selector sequence.
- `verification-screens/pass63-dosbox-title-automation/dosbox-title-input.log` — DOSBox Staging run log.
- Earlier exploratory screenshots/logs in the same directory document partial one-/two-selection attempts and AppleScript denial.

A macOS Keychain prompt was visible over the host screenshot during this run.  That does not invalidate the selector automation result, but it means this screenshot is not suitable for pixel overlay.  The next capture pass should either close/avoid unrelated host dialogs first or use DOSBox's own framebuffer screenshot/video capture after the `cliclick` selector handoff.

## Remaining gap

The hard selector blocker is downgraded: a stable local path now reaches graphical TITLE.  Remaining TITLE evidence work is still open:

1. Capture clean original runtime frames without host-window obstruction.
2. Measure cadence/palette-display timing and source/runtime handoff timing.
3. Pixel-compare Firestaff's wired TITLE frontend against clean original emulator frames before promoting timing/presentation rows beyond `KNOWN_DIFF` / `UNPROVEN`.
