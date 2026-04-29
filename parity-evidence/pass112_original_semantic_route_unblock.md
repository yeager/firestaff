# Pass 112 — original semantic-route unblock gate

Date: 2026-04-28

## Goal

Move the original overlay/capture blocker forward by adding a hard semantic gate between raw DOSBox screenshots and any original-vs-Firestaff pixel overlay claim.

This pass does **not** claim pixel parity. It proves that the current labeled N2 route still fails before overlay comparison, with exact frame classes and hashes.

## New verifier

- Tool: `tools/pass112_original_semantic_route_audit.py`
- Default audit output: `parity-evidence/pass112_original_semantic_route_audit.json`
- N2 route-probe audit output: `parity-evidence/pass112_n2_route_probe_semantic_audit.json`

The verifier joins:

1. `original_viewport_shot_labels.tsv` from `scripts/dosbox_dm1_original_viewport_reference_capture.sh`, and
2. `pass80_original_frame_classifier.json` from `tools/pass80_original_frame_classifier.py`.

Required six-shot overlay fixture:

| # | expected route label | expected class |
|---|----------------------|----------------|
| 1 | `party_hud` | `dungeon_gameplay` |
| 2 | `` | `dungeon_gameplay` |
| 3 | `` | `dungeon_gameplay` |
| 4 | `spell_panel` | `spell_panel` |
| 5 | `` | `dungeon_gameplay` |
| 6 | `inventory_panel` | `inventory` |

The gate also rejects duplicate raw-frame hashes and anything other than exactly six `320x200` raw frames.

## N2 original-data source checked

Host: `N2` (`firestaff-worker`)

Commands:

```sh
ls -la ~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE
(cd ~/.openclaw/data/firestaff-original-games/DM && sha256sum -c SHA256SUMS)
```

Observed:

- `DM.EXE` exists at `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE`
- archive manifest verified OK, including `Game,Dungeon_Master,DOS,Software.7z: OK`

## N2 route probe

Command:

```sh
OUT_DIR=$PWD/verification-screens/pass112-n2-route-probe \
DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:party_hud f1 wait:500 shot f2 wait:500 shot one wait:500 shot:spell_panel four wait:100 four wait:100 enter wait:1000 shot i wait:800 shot:inventory_panel' \
DOSBOX=/usr/bin/dosbox \
xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Result:

- six raw screenshots were produced and normalized;
- all raw screenshots were `320x200`;
- labels were written as intended: `party_hud`, blank, blank, `spell_panel`, blank, `inventory_panel`.

Classifier command:

```sh
python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass112-n2-route-probe \
  --expected pass77 \
  --fail-on-duplicates
```

Observed classes:

```text
graphics_320x200_unclassified
title_or_menu
title_or_menu
title_or_menu
entrance_menu
entrance_menu
```

Key failure details:

- shots 2–4 are identical (`sha256=307323fbc1f7772cb259160f2f988a482981e33a339e4414b0bfc8c85f8d4bd0`), so the route did not advance through unique visual states;
- the supposed spell-panel checkpoint is still `title_or_menu`;
- the supposed inventory checkpoint is still `entrance_menu`.

Semantic audit command:

```sh
python3 tools/pass112_original_semantic_route_audit.py \
  verification-screens/pass112-n2-route-probe \
  --out-json parity-evidence/pass112_n2_route_probe_semantic_audit.json
```

Observed:

```text
semantic_route_ready_for_overlay=false
```

## Exact next unblocker

The capture stack is now capable of producing raw `320x200` DOSBox frames on N2, but the route is still semantically wrong. The next pass should focus on the original runtime input route itself, specifically the transition from title/menu/entrance controls into a stable in-game HUD state before sending F1/F2/spell/inventory keys.

Do not promote any current `pass112-n2-route-probe` frame to original overlay reference evidence.
