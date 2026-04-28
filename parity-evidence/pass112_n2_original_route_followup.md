# Pass 112 N2 original-route follow-up

This follow-up ran on N2 (`Firestaff-Worker-VM`, `~/work/firestaff`) against the curated original DM PC 3.4 tree under `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34`.

## What changed operationally

The previous N2 probe launched through the wrong original runtime path and captured title/menu/entrance frames. This pass used direct DM flags instead:

- keyboard route probes: `DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk"`
- mouse route probe: `DM1_ORIGINAL_PROGRAM="DM -vv -sn -pm"`
- `DM1_ROUTE_SKIP_STARTUP_SELECTOR=1`

For keyboard mode, the stable pre-shot entry preamble that reaches in-game HUD is:

```text
wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500
```

This fixes the original blocker where shots were still title/menu/entrance. The route now reaches `dungeon_gameplay` before the requested spell/inventory actions.

## Fresh semantic evidence

Best stable-HUD keyboard route attempt:

```bash
OUT_DIR=$PWD/verification-screens/pass112-n2-stable-hud-route \
DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DOSBOX=/usr/bin/dosbox \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 shot:party_hud kp5 wait:700 shot kp5 wait:700 shot f1 wait:700 shot:spell_panel one wait:700 shot i wait:700 shot:inventory_panel" \
xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Classifier gate:

```bash
python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass112-n2-stable-hud-route \
  --expected pass77 \
  --fail-on-duplicates
```

Pass112 join gate:

```bash
python3 tools/pass112_original_semantic_route_audit.py \
  verification-screens/pass112-n2-stable-hud-route \
  --out-json parity-evidence/pass112_n2_stable_hud_route_audit.json \
  --out-md parity-evidence/pass112_n2_stable_hud_route_audit.md
```

Measured semantic classes:

```text
01 dungeon_gameplay
02 dungeon_gameplay
03 wall_closeup
04 wall_closeup
05 wall_closeup
06 wall_closeup
```

Additional mouse-mode route (`verification-screens/pass112-n2-mousemode-route-v1`) proved the original can leave the entrance menu under `-pm`, but still did not open spell/inventory:

```text
entrance_menu, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay
```

## Current blocker

Overlay readiness is still **not** promoted. The route now reaches stable in-game HUD, but the original runtime has no loaded/selected champion state in this source tree, and the tested F1/F2/letter/mouse actions do not produce `spell_panel` or a real `inventory` panel. `pass112-n2-route-probe` remains invalid and must not be used as overlay reference evidence.

The next unblock is to provide or create a deterministic original save/party route (or complete a Hall-of-Champions selection route) before expecting spell/inventory screenshots to classify as `spell_panel`/`inventory`.
