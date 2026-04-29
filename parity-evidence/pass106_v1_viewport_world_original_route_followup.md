# Pass 106 — V1 viewport/world original-route follow-up

Date: 2026-04-28
Lane: Viewport/world visuals (DM1 V1 walls/items/ornaments/creatures/draw-order/evidence)
Host: N2 (`Firestaff-Worker-VM`)
Branch: `sync/n2-dm1-v1-20260428`
Run-dir: `<N2_RUNS>/20260428-1015-viewport-world-followup/`

## Scope

Followed up the pass105 original entrance-route unblock for the viewport/world lane only. This pass checks whether the proven N2 original entrance sequence can immediately produce safer narrow viewport/world semantic parity probes. It does not change HUD, inventory, V2, CSB, or original overlay promotion.

Proven entrance prefix reused from pass105:

```text
wait:7000 enter wait:1200 click:250,53 wait:1200 click:247,135 wait:1200 kp6 wait:800
```

## Probe A — keyboard turn/forward mix after entry

Command:

```sh
RUN=<N2_RUNS>/20260428-1015-viewport-world-followup
ROUTE='wait:7000 enter wait:1200 click:250,53 wait:1200 click:247,135 wait:1200 kp6 wait:800 shot:start kp4 wait:800 shot:kp4a kp4 wait:800 shot:kp4b kp8 wait:800 shot:kp8a kp6 wait:800 shot:kp6a kp8 wait:800 shot:kp8b'
OUT_DIR="$RUN/probe-turn-left-forward-mix" \
DM1_ORIGINAL_STAGE_DIR="$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34" \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="$ROUTE" \
DOSBOX=/usr/bin/dosbox \
xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run \
  > "$RUN/probe-turn-left-forward-mix.run.log" 2>&1
python3 tools/pass80_original_frame_classifier.py \
  "$RUN/probe-turn-left-forward-mix" --fail-on-duplicates
```

Route labels:

```text
01 start
02 kp4a
03 kp4b
04 kp8a
05 kp6a
06 kp8b
```

Classifier result:

```text
pass=False
class_counts={'dungeon_gameplay': 3, 'wall_closeup': 3}
duplicate_sha256_counts={fbeb1b82cd09: 3, 9fc8530431a3: 2}
01 dungeon_gameplay 355a191cd07b
02 wall_closeup     fbeb1b82cd09
03 dungeon_gameplay 9fc8530431a3
04 dungeon_gameplay 9fc8530431a3
05 wall_closeup     fbeb1b82cd09
06 wall_closeup     fbeb1b82cd09
```

Interpretation: the entry prefix is mechanically usable, but this simple keypad route still loops through duplicate flat wall states. It is not safe as a six-shot viewport/world parity reference route.

## Probe B — mouse control pad after unlocked gameplay

Command:

```sh
RUN=<N2_RUNS>/20260428-1015-viewport-world-followup
ROUTE='wait:7000 enter wait:1200 click:250,53 wait:1200 click:247,135 wait:1200 kp6 wait:800 shot:start click:246,140 wait:800 shot:c246_140 click:306,140 wait:800 shot:c306_140 click:276,140 wait:800 shot:c276_140 click:276,170 wait:800 shot:c276_170 click:246,170 wait:800 shot:c246_170'
OUT_DIR="$RUN/probe-unlocked-mouse-controls" \
DM1_ORIGINAL_STAGE_DIR="$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34" \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="$ROUTE" \
DOSBOX=/usr/bin/dosbox \
xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run \
  > "$RUN/probe-unlocked-mouse-controls.run.log" 2>&1
python3 tools/pass80_original_frame_classifier.py \
  "$RUN/probe-unlocked-mouse-controls" --fail-on-duplicates
```

Route labels:

```text
01 start
02 c246_140
03 c306_140
04 c276_140
05 c276_170
06 c246_170
```

Classifier result:

```text
pass=False
class_counts={'dungeon_gameplay': 6}
duplicate_sha256_counts={48ed3743ab6a: 5}
01 dungeon_gameplay 355a191cd07b
02 dungeon_gameplay 48ed3743ab6a
03 dungeon_gameplay 48ed3743ab6a
04 dungeon_gameplay 48ed3743ab6a
05 dungeon_gameplay 48ed3743ab6a
06 dungeon_gameplay 48ed3743ab6a
```

Interpretation: after the pass105 entry prefix, the clicked right-column control coordinates no longer leave the route in menu frames, but they collapse to one repeated gameplay frame. This is useful negative evidence: mouse injection is mapped correctly enough to hit the original frame, but these coordinates/actions are not yet a validated semantic movement sequence for viewport parity.

## Current V1 viewport/world gates

Command:

```sh
python3 tools/verify_v1_viewport_draw_order_gate.py
cmake --build build --target firestaff_m11_game_view_probe -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
```

Result:

```text
V1 viewport draw-order source-shape verification passed
[100%] Built target firestaff_m11_game_view_probe
# summary: 578/578 invariants passed
```

Full local log: `<N2_RUNS>/20260428-1015-viewport-world-followup/v1_viewport_gates.log`.

## Conclusion

The pass105 route unblock materially improves the situation: it can reliably reach original `dungeon_gameplay` frames from N2 under Xvfb. It does not yet unblock a six-shot viewport/world parity route.

Next viewport-specific blocker: identify the original PC 3.4 post-entry movement/control sequence that yields six semantically distinct, non-wall, non-duplicate gameplay frames matching Firestaff's deterministic viewport route. Until then, keep V1 viewport/world acceptance at the current source/data/probe gate level and do not promote these original captures as parity references.
