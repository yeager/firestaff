# Autonomous 1:1 Execution Plan — Firestaff

Created: 2026-04-24
Owner instruction: pursue DM1/V1 1:1 parity without interrupting Daniel; report only when complete or genuinely blocked. After DM1/V1 parity is locked, continue with startup/start menu defects.

## Operating rules

- Work sequentially unless tasks are proven independent; avoid conflicting edits in the same tree.
- Use original runtime, ReDMCSB source, Greatstone/dmweb data, local original binaries/assets, and existing Firestaff probes as evidence.
- Never claim parity from Firestaff behavior alone.
- Every pass ends with: focused change or documented blocker, verification output, `git diff --check`, secret scan before commit, and updated evidence docs.
- Preserve unrelated dirty/untracked files unless a pass explicitly owns them.
- Do not mention AI/internal tooling in commits/docs.
- Report to Daniel only when the full mission is complete or a hard blocker needs human action.

## Current state

Recent landed passes:

- Pass 56: decoded DM PC 3.4 `TITLE` script.
- Pass 57: rendered TITLE frames; Greatstone PNG comparison matched.
- Pass 58: wired TITLE renderer into V1 frontend.
- Pass 59: finite TITLE handoff policy.
- Pass 60: stabilized launcher smoke audio by using dummy SDL audio for headless smoke.
- Pass 61: deterministic TITLE → menu handoff path.
- Pass 62: documented the initial DOSBox TITLE capture blocker: `DM VGA` reaches the original text selector, but DOS redirection and direct VGA overlay attempts do not cleanly enter the graphical runtime.
- Pass 63: added a reproducible local DOSBox Staging + `cliclick` selector-input wrapper that reaches the graphical original TITLE screen.
- Pass 64: replaced whole-screen capture with targeted DOSBox PID input and CGWindowID capture, producing clean unobstructed original TITLE window evidence.  Raw/cropped framebuffer timing and pixel comparison remain pending.

## Phase A — Finish DM1/V1 1:1 lock

### A1. TITLE runtime evidence and frontend pixel/timing lock

Goal: make TITLE animation claims truthful and evidence-backed.

Tasks:
1. Use `scripts/dosbox_dm1_title_clean_capture_pass64.sh` as the current clean original TITLE capture path.
2. Convert the clean window capture into deterministic game-content pixels, or get DOSBox framebuffer/video capture working for raw frames.
3. Extract timing/cadence evidence: frame progression, palette-display duration, transition to title menu.
4. Pixel-compare wired V1 frontend frames against original runtime capture where possible.
5. Adjust frontend timing/handoff policy only if evidence requires it.
6. Keep explicit `KNOWN_DIFF` / `UNPROVEN` labels where capture cannot prove parity.

Done when:
- TITLE has source/runtime-backed cadence status.
- Frontend presentation has either MATCHED evidence or precise remaining blocker.

### A2. Audio runtime parity evidence

Goal: close or honestly bound SFX/music cadence and overlap.

Tasks:
1. Capture original runtime title music start/stop/loop behavior.
2. Capture representative SFX overlap/priority cases.
3. Compare against `audio_sdl_m11`, SND3 event mapping, and SONG.DAT queueing.
4. Convert remaining direct marker TODO buckets if source-backed.
5. Document bug-faithful quirks if original runtime requires them.

Done when:
- SONG.DAT title loop policy and SND3 overlap/priority have original evidence or a hard reference blocker.

### A3. Visual/layout pixel lock

Goal: lock viewport/side-panel/main screen layout with original evidence.

Tasks:
1. Reuse `scripts/dosbox_dm1_capture.sh`, existing verification-screens, ReDMCSB source (`PANEL.C`, `COORD.C`, `DEFS.H`) and Greatstone data.
2. Build or update exact coordinate overlays for main screen, side panel, spell panel, inventory, champion region.
3. Remove or demote remaining invented UI chrome/text in V1.
4. Run overlay diffs and keep evidence artifacts small and named.

Done when:
- Major V1 screens have coordinate/pixel evidence and no unexplained invented UI remains.

### A4. Behavior original-binding

Goal: bind the strongest M9/M10/M11 probes to original/ReDMCSB evidence, not just internal consistency.

Tasks:
1. Pick high-value behaviors: movement, doors/sensors, champion bars, inventory interactions, action panel.
2. For each, identify ReDMCSB I34E source path and/or original runtime capture.
3. Add expected-log or invariant probes where possible.
4. Preserve M10 gates.

Done when:
- At least the critical gameplay paths have original/source anchored evidence, and remaining gaps are explicitly lower priority or blocked.

### A5. Text/UI cleanup

Goal: remove last non-original V1 presentation artifacts.

Tasks:
1. Drop tick-prefix/message-log artifacts if still present.
2. Verify message strings and surfaces against source/reference.
3. Ensure all debug/helper text is hidden from V1 surfaces.

Done when:
- No known Firestaff-only text remains in V1 unless marked debug/non-player-facing.

### A6. Final V1 lock gate

Run a full lock gate before declaring done:

- all pass-specific probes from recent V1 work
- M9/M10/M11 verify gates
- launcher smoke
- title/menu/title-hold probes
- audio probes
- overlay/capture evidence scripts that are part of the lock
- `git status`, `git diff --check`, secret scan
- update `V1_BLOCKERS.md`, `PARITY_MATRIX_DM1_V1.md`, `STATUS.md`/relevant docs

Done when:
- `V1_BLOCKERS.md` has no high/medium unknowns that prevent the selected DM1 PC 3.4 V1 target from being honestly called locked, or every remaining difference is a consciously accepted `KNOWN_DIFF` with evidence.

## Phase B — Startup/start menu defects

Start only after Phase A is done or explicitly blocked.

Known user-reported defects:

- many startup/start menu keypresses crash
- Launch button does not work
- likely bad focus/input routing and/or invalid state transitions

Plan:
1. Inventory startup/menu code paths and current tests.
2. Build a keypress matrix probe for every accepted key on every startup/menu state.
3. Reproduce crashes with exact commands and crash output.
4. Fix state/input validation so invalid keys never crash.
5. Fix Launch button path end-to-end into the intended runtime mode.
6. Add regression probes for crash keys and launch action.
7. Run launcher smoke plus menu probes before every commit.

Done when:
- keypress matrix passes without crashes
- Launch button works from startup menu
- regressions are covered by automated probes

## Reporting policy

Do not send progress pings. Report only:

1. Mission complete: DM1/V1 lock done and start menu defects fixed.
2. Hard blocker: missing asset/tool/human decision prevents further autonomous progress.
3. Safety/resource issue: disk/memory/tool failure risks damaging work.
