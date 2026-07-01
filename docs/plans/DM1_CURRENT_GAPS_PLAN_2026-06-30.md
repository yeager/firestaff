# DM1 current gaps plan - 2026-06-30

## Scope

This plan reconciles the current DM1 gap/bug surface across:

- `TODO.md`
- `DONE.md`
- `docs/FINAL_GAPS.md`
- `docs/FIRESTAFF_GAP_LIST.md`
- `docs/DM1_V1_BUG_AUDIT.md`
- `docs/dm1-v1-functional-divergence-report.md`
- `docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md`
- `docs/parity/PARITY_MATRIX_DM1_V1.md`
- `docs/parity/V1_BLOCKERS.md`
- `docs/reports/pass435_dm1_v1_original_route_capture_blocker_20260513_1534.md`

For the combined DM1/CSB objective, see
`docs/plans/REDMCSB_DM1_CSB_FULL_SUPPORT_PLAN_2026-06-30.md`.

The target remains DM1 PC DOS English 3.4, V1 original-compatible mode. Source
locks and Firestaff runtime probes are not enough for pixel/content `MATCHED`
claims unless paired original capture evidence exists.

## Compatibility doctrine

Firestaff is a clean-room rewrite that uses ReDMCSB as the functional template
for DM1 and CSB behavior. For DM1 V1 work, the priority order is:

1. ReDMCSB source-lock first for gameplay rules, UI state machines, save/load
   fields, timers, input routing, bug compatibility, and per-platform behavior
   choices.
2. Firestaff runtime probes second, to prove the rewritten implementation
   follows the source-locked contract without importing ReDMCSB code.
3. Original DOSBox captures third, to promote visual/pixel/content parity rows
   only when the original runtime and Firestaff are paired in the same state.

ReDMCSB source evidence can close functional bugs and justify intentional
compatibility quirks. It cannot, by itself, promote a screenshot/pixel row to
`MATCHED`; those rows still require same-state original capture evidence.

## Current truth

DM1 V1 implementation is mostly source-locked and runtime-gated on the
Firestaff side. The open DM1 work is concentrated in evidence promotion and a
small number of live runtime integrations:

- Original DOS capture/pixel parity is still the top blocker. Pass435 is
  source-locked but blocked because current six-shot original route captures
  still do not reach the required semantic states with unique frame hashes, and
  the old pass376 artifacts remain duplicate or non-semantic.
- `docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md` says viewport, wall, collision,
  creature-chain, and champion-panel rows cannot become global `MATCHED` until
  same-state original-to-Firestaff pairings exist.
- Resurrection rename UI has a data-free F0281 source-lock gate, but the live
  M11 prompt path, C027 panel rendering/input handoff, duplicate-name feedback,
  and paired evidence are still open.
- Original-save interop is only at classifier/readiness level. The missing step
  is real original bytes -> Firestaff runtime -> write-back round-trip.
- DM1 V2.2 finished-art/material gates are hardened, but final promotion still
  needs an operator-reviewed real pack and real screenshot/material receipt.
- Several older docs are stale. `docs/FINAL_GAPS.md` supersedes much of
  `docs/DM1_V1_BUG_AUDIT.md` and the functional divergence report: F0308 luck,
  MOV-05/F0284, and most major bug-audit items are fixed or documented. Treat
  older "open" claims as candidates for verification, not as live blockers.

## Priority order

### P0 - Keep the DM1 ledger honest

Before code work, add or update a small machine-readable/current-status note
for DM1 parity rows. The goal is to prevent stale docs from promoting old
claims back into active blockers.

Deliverables:

- Current DM1 status index that marks each stale source as superseded, active,
  or evidence-only.
- Explicit ReDMCSB-as-functional-template rule for DM1/CSB, separated from
  original DOSBox/pixel promotion evidence.
- Explicit rule: no pixel/content row moves to `MATCHED` without paired original
  capture plus Firestaff same-state output.
- TODO/DONE reconciliation for DM1-only rows.

Exit gate:

- A reviewer can answer "what blocks DM1 today?" without reading every old pass
  report.

### P1 - Unblock original route capture

This is the main DM1 parity unlock. Fix the original DOSBox route executor
handoff first, not renderer code.

Deliverables:

- A capture-driver diagnostic that proves each route token after the initial
  wait is consumed.
- Fresh six-shot original PC34 route capture with non-duplicate 320x200 raw
  frames and 224x136 viewport crops.
- Re-run:
  - `tools/pass86_original_viewport_crop_manifest.py`
  - `tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py`
- Quarantine or retire pass376 artifacts until they pass the semantic route
  classifier.

Exit gate:

- Pass435 reports semantic original route ready.
- Captures include start/turn/forward states with route labels and unique frame
  hashes.

2026-06-30 update:

- The refreshed HoC route and entrance-click diagnostic captured cleanly, but
  still did not reach candidate or party-control state. Treat those artifacts
  as negative diagnostics, not promotion evidence.
- Pass162 now has a source-audited C080 queue trace plan covering
  F0359 -> F0380 -> F0377 -> F0372 -> F0275 -> F0280.
- The FIRES.MAP public-symbol bridge now emits concrete stock-original
  candidate addresses for the HoC route: F0359 `22F4:030D`, F0380 `22F4:0699`,
  F0377 `1E44:02FE`, F0275 `1859:1405`, and F0280 `1782:0031`. F0372 is static
  in this map, so F0275 is the addressable proxy immediately after F0372's
  front-wall sensor calculation.
- The remaining blocker is no longer the absence of an address map. It is the
  missing live DOSBox-debug transcript proving that the stock original runtime
  actually follows those addresses in order for the portrait click.
- A pass247 syntax probe against the DOSBox-X-backed `dosbox-debug` symlink did
  not confirm command parsing through the tmux path; the commands were echoed
  rather than listed as debugger breakpoints. A follow-up owned-PTY
  DOSBox-X `-debug -break-start` probe does prove the pass162 BP/BPM command
  packet is accepted.
- Live stock-original `-debug -break-start` probes now prove the accepted
  BP/BPM packet can stop inside the original runtime and that macOS-native
  route input reaches the DOSBox window-control layer. The standard HoC route,
  a simpler movement-area click, the HID double-click/autolock/Ctrl-F10
  controls, an external `cliclick` movement-click control, and a macOS
  System Events movement-click control, and direct CGEvent `postToPid`
  movement-click control all observed `FIRES` start and `DATA\DUNGEON.DAT`
  load, recorded 34 memory-breakpoint stops, and still did not hit
  F0359/F0380/F0377/F0275/F0280. Treat this as a first-missing-F0359/C080
  mouse-ingestion blocker, not an address-map, load-order, window-control, or
  HoC-coordinate-only failure.

### P2 - Promote the minimum paired evidence set

Once P1 is green, generate the paired Firestaff output from the same route and
promote only the rows with actual same-state pairings.

Minimum capture set:

- Viewport: start, after one legal step, after one turn.
- Wall: front-wall, side-wall, alcove or mixed wall state.
- Collision: wall, closed door, fakewall or open-door sequence transcript.
- Creature-chain: D2C creature and D1C creature.
- Champion panel: four-champion HUD and single champion status panel.
- Spell/inventory: spell panel and inventory panel.

Deliverables:

- JSON manifests with source assets, route commands, semantic labels, raw hashes,
  crop hashes, and Firestaff state hashes.
- Pixel-diff reports with residuals either fixed or classified as `KNOWN_DIFF`
  with source evidence.

Exit gate:

- `docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md` can move each area from
  scout/partial/reference-blocked to a specific evidence-backed status.

### P3 - Finish live F0281 resurrection rename UI

The source-lock exists; the open work is live integration and evidence.

Deliverables:

- M11 prompt wiring after C161 reincarnate.
- Real C027 rename panel rendering from GRAPHICS.DAT into the G0032 panel box.
- Input handoff for name/title entry, Return, Backspace, trimming, caps, and
  duplicate-name feedback.
- Focused CTest for the live M11 path, separate from the data-free contract.
- Paired Firestaff/original evidence when the capture route can reach the panel.

Exit gate:

- The data-free F0281 gate and the live M11 path both pass, and TODO can drop
  the "No live UI or parity claim yet" caveat.

### P4 - Original save round-trip

Current gates classify original headers and protect Firestaff-native manifests.
The missing interop step is real original-save execution.

Deliverables:

- Skip-safe real-save probe that scans user-staged `DMSAVE`/`DMGAME`.
- Runtime importer path for recognized original DM1 PC34 saves.
- Write-back/export path that preserves the original-compatible fields or emits
  an explicit Firestaff-native conversion receipt.
- Round-trip report: original bytes -> Firestaff runtime -> save -> reload.

Exit gate:

- A real staged original save can be imported, loaded into runtime state, saved,
  and reloaded with documented compatibility boundaries.

### P5 - DM1 V2.2 finished-art/material promotion

The classification gates are strong enough; the missing work is real operator
material and visual evidence.

Deliverables:

- Install reviewed real material pack under the expected `~/.firestaff` path.
- Promote material gate and reviewer receipt to `FINISHED_REAL`.
- Capture runtime screenshot receipt from the real pack.
- Replace any remaining placeholder-only public language with gated language.

Exit gate:

- Finished-art material gate, reviewer receipt gate, and screenshot receipt gate
  are all final, with no synthetic or placeholder promotion path.

### P6 - Follow-up DM1 runtime coverage

These are secondary after P1/P2 unless a regression forces them forward:

- Repair or retire the two stale ordinal-2 Hall of Champions sibling probes
  called out in TODO.
- Add original-backed audio/timing cadence cases for movement, doors, title
  music, SFX overlap, and message expiry.
- Expand original-backed behavior cases for combat, creature AI, item
  pickup/drop/use, spells, pits, teleporters, stairs, sensors, projectiles, and
  tick cadence.
- Continue platform-specific DM1 media receipts, but do not let those replace
  the PC34 V1 capture lane.

## Suggested immediate next pass

Name: `dm1_v1_pass162_c080_live_transcript_gate`

Goal: prove where the HoC portrait click is lost in the original runtime before
trying more six-shot overlay routes.

Steps:

1. Use `parity-evidence/verification/pass162_c080_queue_trace/manifest.json`,
   `c080_address_gate_manifest.json`, and
   `pass162_c080_dosbox_debug_commands.txt` as the current gate receipts.
2. Start DOSBox-X/dosbox-debug with the pass162 runtime config through the
   owned-PTY `-debug -break-start` control path, apply the accepted BP/BPM
   commands, continue into the stock original runtime, wait for `FIRES` and
   `DATA\DUNGEON.DAT`, and run the source-locked portrait click route through
   the macOS-native Swift/CGEvent route driver. Do not use the failed
   tmux/pass247 echo path as promotion evidence.
3. Classify the first missing boundary in order:
   F0359 mouse translation/queue write, F0380 dequeue, F0377 dispatch and
   viewport normalization, F0275 front-wall sensor proxy after static F0372,
   F0280 candidate entry.
4. Only after F0280 is proven or disproven in the stock original binary, rerun
   pass435 six-shot captures with the corrected route/state.
5. Re-run pass86 and pass435 semantic gates.
6. Commit only the executor fix, manifests, and status doc updates. Do not
   promote parity rows in the same pass unless the same-state Firestaff pairings
   are also produced.

## 2026-06-30 ledger progress

Added `docs/parity/DM1_V1_CURRENT_STATUS_INDEX.json` as the current DM1 V1
status index. It records:

- ReDMCSB WIP20210206 as the DM1/CSB functional template for clean-room
  implementation work.
- The distinction between functional source-lock promotion and
  original-runtime pixel/content promotion.
- Current active blockers: semantic original route capture, same-state
  Firestaff pairings, live F0281 rename UI, original-save roundtrip, and V2.2
  finished-art/material receipts.
- Per-area row status for viewport, wall, collision, creature-chain,
  champion-panel, spell/inventory panels, and audio/timing.
- Disposition for stale or partially superseded DM1 documents, so old audit
  rows are treated as verification candidates rather than automatically active
  blockers.

## 2026-06-30 executor repair progress

Implemented in `scripts/dosbox_dm1_original_viewport_reference_capture.sh`:

- `--preflight-route` now writes `original_viewport_route_plan.json` with token
  count, expected shot count, shot labels, total planned wait time, selected
  injector, and screenshot hotkey.
- Swift and xdotool route helpers now log `route-token-start`,
  `route-token-done`, and `route-complete` with timestamps, so a killed or hung
  route can be localized to the exact token.
- `--run` now reports route-injector failure explicitly and tails the injector
  log instead of falling through to a later missing-artifact failure.
- The post-route screenshot wait now respects `DM1_ORIGINAL_EXPECTED_SHOTS`
  instead of hard-coding six captures, preserving single transcript-row
  workflows.
- macOS Swift injection now supports `DM1_DOSBOX_SCREENSHOT_HOTKEY=cmd-f5` or
  `ctrl-f5`, and records that choice in the route plan.

## 2026-06-30 HoC route contract correction

The pass435 next-unblock command now uses the source-locked Hall of Champions
transition before collecting the six promotable route shots:

- `enter` leaves the entrance menu.
- `click:111,82` targets the ReDMCSB C127/F0280 portrait path in the viewport.
- `click:130,115` targets the C160 resurrect choice.
- `enter` confirms before the six route labels are captured.
- The six labels remain `party_hud`, `turn_left_after_vblank`,
  `turn_right_after_vblank`, `spell_panel`, `post_spell_redraw`, and
  `inventory_panel`.

`scripts/dosbox_dm1_original_viewport_reference_capture.sh` now has
`--print-pass435-hoc-route` so this command can be printed directly from the
capture tool. A preflight run against `/tmp/firestaff-dm1-pass435-hoc-preflight`
validated the route shape: 30 tokens, six labeled shots, Swift injector, and
`original_viewport_route_plan.json` with `pass=true`.

`tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py` now
requires the HoC prelude tokens in its next-unblock contract. The gate still
reports `BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY` because the old
pass376 artifacts remain duplicate/non-semantic and quarantined. If the HoC
route still collapses to the known `48ed3743ab6a` no-party frame, the next fix
is to instrument the pass162 C080/F0377/F0280 gate rather than inventing another
six-shot overlay route.

## 2026-06-30 HoC runtime diagnostics

Two local DOSBox Staging runs were executed in scratch directories. The capture
script successfully injected all tokens and produced healthy raw frames/crops,
but neither route reached party-control-ready state:

- `/tmp/firestaff-dm1-hoc-route-attempt-20260630`: the HoC prelude route
  produced classes `dungeon_gameplay`, `wall_closeup`, `dungeon_gameplay`,
  `dungeon_gameplay`, `wall_closeup`, `wall_closeup`; pass113 reported
  `party_control_ready=false`, no control classes, six blank-right-column
  frames, and repeated raw hashes.
- `/tmp/firestaff-dm1-hoc-entrance-click-diagnostic-20260630`: the source
  entrance click changed/opened the entrance-door/menu state, but the portrait
  click still occurred before a proven C127 gameplay pose. The run ended in
  no-party dungeon gameplay, not candidate or party control.

The metadata-only receipt is
`parity-evidence/verification/pass435_dm1_v1_hoc_route_attempt_20260630.json`.
`verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py` now includes
that receipt and reports the additional blocker
`blocked/entrance-and-c127-runtime-boundary-not-proven`.

The follow-up live pass162 HoC probe is recorded in
`parity-evidence/verification/pass162_c080_queue_trace/live_hoc_break_start_probe/manifest.json`
and summarized in `parity-evidence/pass162_c080_hoc_live_break_start_probe.md`.
It accepted the debugger packet, stopped 34 times in stock-original runtime
memory watchpoints, saw `FIRES` plus `DATA\DUNGEON.DAT`, and delivered the HoC
route through mapped native macOS HID clicks. No C080-chain code breakpoint
fired.

The companion movement-click control is recorded in
`parity-evidence/verification/pass162_c080_queue_trace/live_movement_click_control_break_start_probe/manifest.json`
and summarized in
`parity-evidence/pass162_c080_movement_click_control_live_break_start_probe.md`.
It uses the same stock-original runtime, debugger packet, HID mouse posting,
FIRES/DUNGEON readiness, and window-control proof, but clicks the movement
arrow area instead of the HoC portrait. It also stops before F0359 with 34
memory-breakpoint stops and no C080-chain code breakpoint.

The movement-click control now posts an explicit `mouseMoved` event to the
mapped SDL-window point before mouse down/up (`pre-move=true` in the route log).
That still leaves F0359 as the first missing boundary, so the current blocker
is not explained by a missing host mouse move before the click.

The same movement-click control now also warps the macOS system cursor to the
mapped SDL-window point before posting the `mouseMoved` and down/up events
(`cursor-warp=true` in the route log). That still records the same 34
stock-original memory stops and first-missing F0359 boundary, so the current
blocker is not explained by CGEvent carrying the right coordinates while the
real host cursor remains elsewhere.

A second control,
`parity-evidence/verification/pass162_c080_queue_trace/live_movement_double_click_control_break_start_probe/manifest.json`,
sends the same movement-area click twice after `DATA\DUNGEON.DAT` readiness.
Both click route-log entries have `pre-move=true`; the run still records 34
memory-breakpoint stops and no F0359/F0380/F0377/F0275/F0280 code stop. This
rules out the simple explanation that the first click is only consumed by
DOSBox focus or mouse capture while the second would reach the game.

The autolock control,
`parity-evidence/verification/pass162_c080_queue_trace/live_movement_autolock_control_break_start_probe/manifest.json`,
uses DOSBox-X `autolock=true`, `mouse_emulation=always`, and
`clip_mouse_button=none` while keeping HID posting, cursor warp, and two
movement-area clicks. It still records 34 stock-original memory stops and
first-missing F0359. That means the default DOSBox-X `autolock=false` /
`mouse_emulation=locked` configuration was not the missing boundary.

The same autolock control now writes a DOSBox-X runtime log at
`parity-evidence/verification/pass162_c080_queue_trace/live_movement_autolock_control_break_start_probe/dosbox_runtime.log`
with `mouse=debug`. The log proves DOSBox-X initialized mouse emulation and
INT33 (`MOUSE:INT 33H emulation enabled`, `MOUSE:INT 33h reset`), but the
injected click window contains no logged mouse motion/button event for the two
route clicks. This moves the next boundary before original INT33/F0359 and into
SDL/Cocoa event ingestion or the debugger build's event pump.

The keyboard-capture control,
`parity-evidence/verification/pass162_c080_queue_trace/live_movement_keyboard_capture_control_break_start_probe/manifest.json`,
sends `ctrl-f10` before the same two movement-area clicks. The route log proves
the helper emitted `key ctrl-f10`, but the `mouse=debug` DOSBox-X log still has
no motion/button rows for the subsequent clicks and the runtime still stops
before F0359. That means keyboard-driven capture toggling is not enough to make
the injected macOS HID events enter DOSBox-X's logged mouse path.

The external-input control,
`parity-evidence/verification/pass162_c080_queue_trace/live_movement_cliclick_control_break_start_probe/manifest.json`,
uses `/opt/homebrew/bin/cliclick` instead of the Swift helper's CGEvent mouse
posting while keeping cursor warp, DOSBox-X autolock, `mouse_emulation=always`,
`clip_mouse_button=none`, and `mouse=debug`. It still records 34
memory-breakpoint stops, first-missing F0359, and no logged motion/button rows.
That means the blocker is broader than the Swift/CGEvent helper path.

The System Events control,
`parity-evidence/verification/pass162_c080_queue_trace/live_movement_systemevents_control_break_start_probe/manifest.json`,
uses `osascript` / macOS Accessibility `System Events` clicking instead of
direct CGEvent or `cliclick` posting. The route log proves both clicks targeted
the DOSBox-X `FIRES` window and returned success, but the run still records 34
memory-breakpoint stops, first-missing F0359, and no logged motion/button rows.
That means Accessibility/System Events delivery is not enough either.

The PID-post control,
`parity-evidence/verification/pass162_c080_queue_trace/live_movement_pid_control_break_start_probe/manifest.json`,
uses CoreGraphics `postToPid` for mouse move/down/up against the DOSBox process
instead of the HID event tap. It still records 34 memory-breakpoint stops,
first-missing F0359, and no logged motion/button rows. That closes the current
macOS helper's main posting modes: HID tap, post-to-PID, `cliclick`, and System
Events all fail before DOSBox-X reports mouse motion/button input.

`parity-evidence/verification/pass162_c080_queue_trace/c080_address_gate_manifest.json`
now carries a machine-readable `dosbox_mouse_log_summary` for each live probe.
For the current surface-backed autolock, keyboard-capture, `cliclick`,
System Events, and post-to-PID controls, the summaries all report an existing
DOSBox-X mouse log, MOUSE/INT33 initialization lines, and
`route_motion_button_line_count=0`. That turns the previous manual log grep into
part of the generated pass162 evidence.

The OpenGL-output control,
`parity-evidence/verification/pass162_c080_queue_trace/live_movement_opengl_control_break_start_probe/manifest.json`,
switches DOSBox-X `[sdl] output` from `surface` to `opengl` while keeping the
same movement route. This did not produce a comparable F0359 mouse-ingestion
result: the run failed to prove the BP/BPM packet was retained, did not observe
FIRES or `DATA\DUNGEON.DAT`, and never reached a proven route-window click.
Treat this as an OpenGL backend harness/readiness blocker, not as evidence that
OpenGL fixes or fails the original C080 path.

Next DM1 pass should keep the same live transcript gate and narrow why HID
or externally posted macOS mouse events do not enter the original F0359
mouse-click queue writer. Start inside DOSBox-X SDL/Cocoa event ingestion or the
debugger build's event path before trying more HoC coordinates. Do not spend
the next pass on another six-shot overlay route until that boundary proves the
stock original binary actually reaches F0280 before C160/C161.
- macOS `--run` now performs a best-effort DOSBox focus/activation before
  route injection. This fixes the local case where the route reached
  `route-complete` but DOSBox did not consume the screenshot accelerator unless
  the app had been manually activated first.

Local verification on macOS:

- `bash -n scripts/dosbox_dm1_original_viewport_reference_capture.sh` passed.
- `python3 -m py_compile tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py tools/pass86_original_viewport_crop_manifest.py` passed.
- `tools/test_dm1_v1_capture_runbook_consistency.py` passed 22/22.
- `--preflight-route` with the pass435-style six-shot route selected Swift and
  wrote a passing route plan.
- A single-row live DOSBox run reached `route-complete` with both
  `DM1_DOSBOX_SCREENSHOT_HOTKEY=cmd-f5` and `ctrl-f5`, proving the route
  injector itself is not stopping at the first wait on this host.
- After the focus fix, a clean single-row live DOSBox run without manual
  activation produced raw screenshot health OK and generated:
  `/tmp/firestaff-dm1-single-capture-script-focus/raw_manifest.tsv`,
  `/tmp/firestaff-dm1-single-capture-script-focus/raw_frame_health.json`, and
  `/tmp/firestaff-dm1-single-capture-script-focus/viewport_224x136/01_02_turn_right_west_1_3_original_viewport_224x136.png`.
- A clean six-shot pass435-style live DOSBox run also produced six raw
  screenshots and six crops under `/tmp/firestaff-dm1-sixshot-script-focus`.
  Raw health passed, but semantic classification still failed: only two unique
  raw frame hashes repeated across the six labels, and pass80 reported
  wall/gameplay states where spell/inventory/turn labels were expected.
- Full-frame inspection and pass113 show the route is reaching a no-party
  dungeon-control state: the right column contains only movement arrows,
  `blank_right_column_frames=6`, `control_classes_seen=[]`, and
  `party_control_ready=false`. The next P1 task is to reach/recruit a champion
  or otherwise enter a party-control-ready state before spell/inventory labels,
  not to adjust screenshot capture.

Remaining blocker:

- The executor can now produce rawshot artifacts locally. The DM1 parity blocker
  moves back to original runtime route semantics: enter a party-control-ready
  state, run the full six-shot semantic original route to unique expected
  states, retire/quarantine pass376 duplicate/non-semantic artifacts, and make
  pass435 report semantic original route ready before promoting paired Firestaff
  rows.
