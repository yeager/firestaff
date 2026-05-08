# DM1 V1 original evidence sweep — N2 2026-05-01

Scope: N2 only (`firestaff-worker`, repo `<firestaff-repo>`, source at `dc53ed5`, after required `4287c47`). This sweep audits ReDMCSB first, then runs the original DOSBox/Xvfb evidence gates. It does **not** claim original-vs-Firestaff pixel parity.

## ReDMCSB source locks used before runtime probing

Primary source: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

- Movement mouse matrix: `COMMAND.C:396-405` maps the visible movement panel and viewport click routes: turn left/right, move forward/back/left/right, `C080_COMMAND_CLICK_IN_DUNGEON_VIEW`, and right-click inventory toggle.
- Movement keyboard matrix: `COMMAND.C:272-305` maps keypad/arrows/shift variants onto `C001`..`C006`; `INPUT.C:548-568` normalizes numeric keypad 7/9/8/5/2/4/6 to DEL/Help/arrows before buffering.
- Viewport screen anchor/size: `COORD.C:1693-1722` defines PC viewport screen origin `x=0`, `y=33`, bitmap byte count `15232`, width `224`, height `136`.
- Viewport composition buffer: `DUNVIEW.C:2969-3000` clears/copies ceiling/floor/black areas and sets `G0296_puc_Bitmap_Viewport` pixel width/height to 224x136.
- Champion portrait route: `DUNVIEW.C:520-525` defines portrait-on-wall box `{96,127,35,63}`; with the `COORD.C` viewport origin, center is screen `111,82`.
- Viewport click handling: `CLIKVIEW.C:348-349` subtracts viewport origin; `CLIKVIEW.C:407-431` scans view cells and calls `F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor()` for C05.
- Candidate champion sensor: `MOVESENS.C:1392-1502` explicitly permits `C127_SENSOR_WALL_CHAMPION_PORTRAIT` when no leader exists and calls `F0280_CHAMPION_AddCandidateChampionToParty()`.
- Resurrect/reincarnate panel: `COMMAND.C:231-238` gives source button boxes; `COMMAND.C:508-511` binds C160/C161/C162 to viewport-relative panel zones.

## Runtime gates run on N2

### pass166 — source portrait click route probe

Command:

```sh
cd <firestaff-repo>
python3 -m py_compile tools/pass166_source_portrait_click_route_probe.py tools/pass118_state_aware_original_route_driver.py tools/pass80_original_frame_classifier.py
xvfb-run -a python3 tools/pass166_source_portrait_click_route_probe.py
```

Result file: `parity-evidence/verification/pass166_source_portrait_click_route_probe/README.md`.
Run base: `<firestaff-data>/firestaff-n2-runs/20260501-152806-pass166-source-portrait-click-route-probe`.

Outcome: **blocked/static-no-party** for both source-locked routes:

- resurrect route: static no-party hash `48ed3743ab6a` recurs after confirmation/movement attempts.
- reincarnate route: static no-party hash `48ed3743ab6a` recurs after confirmation/movement attempts.

The probe shows a visible transition around the source portrait/C160/C161 sequence and reaches `dungeon_gameplay`, but the tail is the known no-party/static frame and has no inventory/spell/control marker. This blocks any party/HUD/inventory overlay parity claim.

### pass153 — Xvfb input delivery matrix

Command:

```sh
cd <firestaff-repo>
base="$HOME/.openclaw/data/firestaff-n2-runs/20260501-153102-pass153-xvfb-input-delivery"
xvfb-run -a python3 tools/pass153_xvfb_input_delivery_matrix.py "$base"
```

Result file: `parity-evidence/pass153_xvfb_input_delivery_matrix.md`.

Outcome: input delivery is not the blocker. Helper keys, xdotool window keys, xdotool typed digits, and helper panel clicks all produce dungeon frames, but every dungeon frame remains `48ed3743ab6a` in this rerun.

### overlay source/tool lock

Command:

```sh
cd <firestaff-repo>
python3 tools/verify_original_overlay_capture_source_lock.py \
  --attempt-dir parity-evidence/verification/pass166_source_portrait_click_route_probe/enter_portrait11182_then_resurrect
```

Outcome: ReDMCSB source/tool lock is OK, but `semanticReadyForOverlay=0` because the pass166 attempt directory is not a pass80/pass112 semantic route artifact and lacks `original_viewport_shot_labels.tsv` and `pass80_original_frame_classifier.json`.

## Current blocker classification

- **Not missing binaries/tools:** `/usr/bin/xvfb-run`, `/usr/bin/dosbox`, `/usr/bin/xdotool`, `/usr/bin/scrot`, and `/usr/bin/python3` are present on N2.
- **Not input delivery:** pass153 proves multiple key/click delivery methods reach dungeon frames under Xvfb.
- **Current blocker:** original runtime route still lands in the stock no-party/static dungeon state (`48ed3743ab6a`) after the source-locked portrait route. The source route is credible, but this automated DOSBox route has not proven recruited party/control state.
- **Safe next gate:** a debugger/address-map or deterministic saved-state route must prove the C127/F0280 candidate and C160/C161 acceptance effects in the stock original binary before using captured frames as party/HUD/overlay parity evidence.
