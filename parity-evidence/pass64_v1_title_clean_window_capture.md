# Pass 64 — clean DOSBox TITLE window capture

Date: 2026-04-24

## Scope

Pass 64 turns the pass-63 selector handoff into a clean, targeted original TITLE capture path.  The goal was not to claim final TITLE timing parity; it was to remove the unrelated host-window obstruction and leave a reusable path for the next cadence/pixel pass.

## New harness

```sh
scripts/dosbox_dm1_title_clean_capture_pass64.sh --dry-run
scripts/dosbox_dm1_title_clean_capture_pass64.sh --run
```

The wrapper writes a deterministic DOSBox Staging config and two local Swift helpers under:

```text
verification-screens/pass64-clean-title-capture/
```

It keeps the same original selector choices as pass 63:

```text
1 <Return>  # VGA Graphics
1 <Return>  # No Sound
1 <Return>  # Mouse
```

The important change is that input and capture are targeted rather than whole-desktop dependent:

1. Launch DOSBox Staging with the local DM PC 3.4 tree.
2. Post the selector key events directly to the DOSBox process using `CGEvent.postToPid`.
3. Find the `DOSBox Staging` window with `CGWindowListCopyWindowInfo`.
4. Capture that window by CGWindowID using `screencapture -x -l <id>`.

This avoids both previously observed host problems: unrelated macOS dialogs/notifications over the desktop and the app not becoming the frontmost application in this detached run.

## Non-invasive overlay findings

- The existing pass-63 whole-screen capture path was re-run and still produced obstructed desktop captures: first the previous Problem Reporter/Keychain state, then a background-app/authentication notification over a black desktop.
- Dismissing the stale macOS Problem Reporter window was non-destructive, but whole-screen capture remained fragile because DOSBox could open in the background.
- DOSBox native screenshot hotkey probing did not create usable capture artifacts in this detached context.
- Targeted CGWindowID capture succeeded without requiring keychain edits, accessibility changes, destructive cleanup, or foreground control.

## Verification commands and results

Dry run:

```sh
scripts/dosbox_dm1_title_clean_capture_pass64.sh --dry-run
```

Result:

```text
[pass-64] wrote .../verification-screens/pass64-clean-title-capture/dosbox-title-clean.conf
[pass-64] wrote .../verification-screens/pass64-clean-title-capture/post_selector_keys.swift
[pass-64] wrote .../verification-screens/pass64-clean-title-capture/find_dosbox_window.swift
[pass-64] targeted PID sequence: keycode 18 ('1'), keycode 36 (Return), repeated 3 times
```

Actual run:

```sh
rm -rf verification-screens/pass64-clean-title-capture
scripts/dosbox_dm1_title_clean_capture_pass64.sh --run
```

Result:

```text
-rw-------  1 bosse  staff   5.8K ... dosbox-title-clean.log
-rw-------  1 bosse  staff   190B ... dosbox-window.txt
-rw-------  1 bosse  staff     0B ... post-selector-keys.log
-rw-------@ 1 bosse  staff   137K ... title-clean-window-00.png
-rw-------@ 1 bosse  staff   137K ... title-clean-window-01.png
```

The DOSBox log confirms the original runtime entered graphics mode:

```text
DISPLAY: VGA 320x200 256-colour graphics mode 13h at 70.086 Hz throttled VFR, scaled to 1067x667 pixels with 1:1 (1:1) pixel aspect ratio
```

Window selected for capture:

```text
id=2290
owner=DOSBox Staging
name=SELECTOR - 3000 cycles/ms - to capture the mouse press Cmd+F10 or click any button
bounds={ Height = 828; Width = 1067; X = 426; Y = 112; }
```

SHA-256 for both captured frames:

```text
8cfa3f074c78e2f694be33d6e5e4fcab503f27241cb30cdc3bd1032f8cbfb56a  title-clean-window-00.png
8cfa3f074c78e2f694be33d6e5e4fcab503f27241cb30cdc3bd1032f8cbfb56a  title-clean-window-01.png
```

## Local evidence artifacts

These are intentionally left untracked:

- `verification-screens/pass64-clean-title-capture/title-clean-window-00.png` — 137K; clean targeted DOSBox window capture of the original `Dungeon Master` TITLE screen.
- `verification-screens/pass64-clean-title-capture/title-clean-window-01.png` — 137K; second capture 1.2s later; byte-identical to frame 00 in this short sample.
- `verification-screens/pass64-clean-title-capture/dosbox-title-clean.log` — 5.8K; DOSBox Staging run log, including mode 13h entry.
- `verification-screens/pass64-clean-title-capture/dosbox-window.txt` — 190B; selected CGWindowID and bounds.
- `verification-screens/pass64-clean-title-capture/post-selector-keys.log` — 0B; targeted selector helper emitted no errors.
- `verification-screens/pass64-clean-title-capture/dosbox-title-clean.conf` plus generated Swift helpers — reproducibility files emitted by the wrapper.

## Remaining gap

The clean-capture blocker is downgraded: a reusable path now produces unobstructed original TITLE evidence without host desktop overlays.

This is still a window capture, not a cropped raw 320×200 framebuffer.  Before final frontend pixel/cadence parity claims, the next pass should either crop the game content deterministically from these window captures or get DOSBox/native raw screenshots working, then measure animation cadence, palette-display timing, and title-menu handoff against Firestaff's V1 frontend output.
