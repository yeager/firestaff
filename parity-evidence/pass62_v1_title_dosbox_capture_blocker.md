# Pass 62 — DOSBox TITLE cadence capture blocker

Date: 2026-04-24

## Scope

Focused original-runtime DOSBox pass for the remaining V1 TITLE questions:

- wall-clock TITLE cadence,
- palette-display timing,
- original TITLE-to-menu handoff timing.

This pass gathered raw DOSBox evidence only.  It does **not** change the V1 TITLE timing policy and does **not** claim original timing parity.

## Harness

New helper:

```sh
scripts/dosbox_dm1_title_cadence_pass62.sh --prepare
```

It writes small DOSBox Staging configs under `verification-screens/pass62-dosbox-title-cadence/` using the already staged DM1 PC 3.4 tree from `verification-screens/dm1-dosbox-capture/DungeonMasterPC34`.

## Raw evidence gathered

Artifacts were kept outside this committed evidence note to avoid adding screenshots/captures to the repository.  Local paths and sizes after cleanup:

- `verification-screens/pass62-dosbox-title-cadence/live-capture/` — 888K; 9-frame Peekaboo live region capture of the DOSBox selector state.
- `verification-screens/pass62-dosbox-title-cadence/vga-arg-screen-5s.png` — 896K; `DM VGA` reaches the text selector, not TITLE.
- `verification-screens/pass62-dosbox-title-cadence/redirect-long-screen-8s.png` — 804K; long redirected input remains in selector/config prompts, not TITLE.
- `verification-screens/pass62-dosbox-title-cadence/run-vga-screen-8s.png` — 892K; direct `VGA` command is illegal because the overlay lacks a DOS executable extension.
- `verification-screens/pass62-dosbox-title-cadence/run-vgaexe-screen-8s.png` — 748K; copied `VGA.EXE` returns to prompt rather than reaching TITLE.
- `verification-screens/pass62-dosbox-title-cadence/dosbox-pipe-run.log` — 6.7K; `echo 1|DM VGA` aborts in DOSBox Staging when redirected input reaches EOF.
- `verification-screens/pass62-dosbox-title-cadence/dosbox-redirect-long.log` — 6.0K; long redirected input is accepted as redirected input but does not produce a clean graphical handoff.

## Commands and results

Prepare configs:

```sh
scripts/dosbox_dm1_title_cadence_pass62.sh --prepare
# [pass-62] wrote configs under /Users/bosse/.openclaw/workspace-main/tmp/firestaff/verification-screens/pass62-dosbox-title-cadence
```

Direct selector argument attempt:

```sh
'/Applications/DOSBox Staging.app/Contents/MacOS/dosbox' \
  -conf verification-screens/pass62-dosbox-title-cadence/dosbox-title-vga-arg.conf
```

Result: after 5s, screenshot showed:

```text
C:\>DM VGA
Please select from '*'ed options
1. [*] VGA Graphics
2. [*] EGA Graphics
3. [ ] Tandy Graphics
Q. [*] Quit
?
```

So `DM VGA` marks VGA as available but still requires an interactive selector key before the original TITLE runtime can begin.

Redirected pipe attempt:

```sh
echo 1|DM VGA
```

Result in `dosbox-pipe-run.log`:

```text
ABORT: DOS:0x0a:Redirected input reached EOF
```

Long redirected input attempt:

```sh
DM VGA < KEYS.TXT
```

Result: DOSBox logged `SHELL: Redirect input from KEYS.TXT`, but the 8s screenshot stayed in repeated/garbled selector prompts rather than reaching the graphical TITLE sequence.  This is not a clean capture path.

Direct overlay attempts:

```sh
VGA
# Illegal command: VGA
```

and:

```sh
cp VGA VGA.EXE
VGA.EXE
```

Result: `VGA` is not a DOS command without an executable extension; copied `VGA.EXE` did not reach TITLE and returned to the prompt.  The overlay cannot be used as a standalone bypass for the selector.

Peekaboo capture attempt:

```sh
peekaboo capture live --mode region --region 426,112,1067,828 \
  --duration 35 --active-fps 12 --idle-fps 2 --threshold 0.02 \
  --max-frames 220 --max-mb 30 --resolution-cap 640 \
  --path verification-screens/pass62-dosbox-title-cadence/live-capture --json
```

Result summary from `live-capture.json`:

```text
framesKept=9 durationMs=35778 fpsEffective=0.25155123260103973 maxMbHit=false maxFramesHit=false
```

The kept frames document the selector/config state, not TITLE cadence.

## Blocker

Current DOSBox automation cannot cleanly reach the original graphical TITLE runtime without an interactive selector key.  The simple non-interactive bypasses tried here either remain at the selector, abort on redirected-input EOF, or fail to launch the VGA overlay standalone.

A future timing pass needs one of:

1. a permitted and stable macOS/DOSBox keystroke injection path for the selector (`1` for VGA, then capture immediately),
2. a DOSBox-X/staging feature or mapper file that can inject the selector key before capture without host UI prompts,
3. source-level proof of the selector-to-TITLE call path and timing, avoiding GUI capture entirely.

Until then, passes 59 and 61 remain deterministic implementation policies only: 53 source-backed TITLE frames, handoff-ready at source `DO`, and optional menu surface on first post-boundary step.  Original wall-clock cadence, palette-display timing, and emulator handoff timing remain unproven.
