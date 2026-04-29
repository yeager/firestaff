# Pass 112 — original capture input-effect probe

Scope: N2 original DOSBox route/capture unblock for HUD/viewport overlay readiness.

This pass deliberately does not promote original frames into `verification-screens/pass70-original-dm1-viewports/`.

## What changed

- Added a focused evidence JSON for three sequential N2 route probes.
- Verified the original archive manifest on N2 with `sha256sum -c SHA256SUMS` (OK).
- Captured three six-shot original DOSBox routes under `<N2_RUNS>/20260428-1335-original-overlay-capture/`.

## Result

The original capture path is healthy, but route semantics are still blocked. The proven entrance sequence reaches dungeon gameplay, but inventory/spell attempts are no-ops:

- `probe-f1-spell-clicks`: all six frames classify as `dungeon_gameplay`; frames 2-6 share one raw SHA-256; `spell_area` remains black after F1 and spell/cast clicks.
- `probe-turn-cycle`: 3 `dungeon_gameplay`, 3 `wall_closeup`; duplicate wall hash remains.
- `probe-move-turn-mix`: 4 `dungeon_gameplay`, 2 `wall_closeup`; two duplicate hashes remain.

## Exact commands

```sh
cd ~/.openclaw/data/firestaff-original-games/DM && sha256sum -c SHA256SUMS

OUT_DIR=$RUN_DIR/probe-f1-spell-clicks \
DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 NEW_FILE_TIMEOUT_MS=6000 DOSBOX=/usr/bin/dosbox \
DM1_ORIGINAL_ROUTE_EVENTS="$ENT shot:party_hud f1 wait:800 shot:inventory_panel f1 wait:500 shot:after_inventory_close click:236,51 wait:500 shot:spell_panel click:249,63 wait:500 shot:rune1 click:312,69 wait:800 shot:after_cast" \
xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run

python3 tools/pass80_original_frame_classifier.py $RUN_DIR/probe-f1-spell-clicks --fail-on-duplicates
python3 tools/pass80_original_frame_classifier.py $RUN_DIR/probe-turn-cycle --fail-on-duplicates
python3 tools/pass80_original_frame_classifier.py $RUN_DIR/probe-move-turn-mix --fail-on-duplicates
```

## Next unblocker

Do not keep trying spell/inventory keys from the current no-party dungeon state. First capture/recruit a champion or prove the PC 3.4 direct-start route already has a usable party. Then retry F1/spell controls from a semantically valid party state.

JSON: `parity-evidence/overlays/pass112_original_capture_input_effect_probe.json`
