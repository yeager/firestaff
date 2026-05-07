# Pass210 fresh N2 movement-only capture attempt — blocker

Status: `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`

Attempt dir: `verification-screens/pass210-n2-original-movement-route-fresh/`

Scope: N2-only fresh movement-focused route. Spell/inventory were deliberately excluded from the route semantics; the legacy crop filenames from `scripts/dosbox_dm1_original_viewport_reference_capture.sh` are just fixed output labels.

## Route run

```sh
OUT_DIR=$PWD/verification-screens/pass210-n2-original-movement-route-fresh \
DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 \
DOSBOX=/usr/bin/dosbox \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=3000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:2000 shot:start kp4 wait:1200 shot:turn_left kp6 wait:1200 shot:turn_right kp8 wait:1200 shot:forward kp4 wait:1200 shot:turn_left_2 kp6 wait:1200 shot:post_redraw" \
xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

## Materialized artifacts

- DOSBox raw 320x200 PNGs are present: `image0001-raw.png` through `image0006-raw.png`.
- Normalized 224x136 viewport crops are present under `viewport_224x136/` as `.ppm` and `.png`.
- Manifests are present: `raw_manifest.tsv`, `original_viewport_224x136_manifest.tsv`, `original_viewport_shot_labels.tsv`.
- Classifier/audit JSON is present: `pass80_original_frame_classifier.json`.

## Classifier gate

Command:

```sh
python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass210-n2-original-movement-route-fresh \
  --expected dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay \
  --fail-on-duplicates
```

Exit: `1`

Problems:

- `image0001-raw.png`: classified `entrance_menu`, expected `dungeon_gameplay`.
- `image0002-raw.png`: classified `wall_closeup`, expected `dungeon_gameplay`.
- `image0005-raw.png`: classified `wall_closeup`, expected `dungeon_gameplay`.
- Duplicate raw frames detected:
  - `fbeb1b82cd096c15c2346f254d9b2b2e8c1a8d0b8d100ba1751c4230c51e3dde`: shots 2 and 5.
  - `48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397`: shots 3, 4, and 6.

Class counts: `{"dungeon_gameplay": 3, "entrance_menu": 1, "wall_closeup": 2}`.

## Shot/log binding

Route shot labels from `original_viewport_shot_labels.tsv`:

| shot | raw frame | route label | classifier | sha256 prefix |
|---|---|---|---|---|
| 1 | `image0001-raw.png` | `start` | `entrance_menu` | `17bd7e878157` |
| 2 | `image0002-raw.png` | `turn_left` | `wall_closeup` | `fbeb1b82cd09` |
| 3 | `image0003-raw.png` | `turn_right` | `dungeon_gameplay` | `48ed3743ab6a` |
| 4 | `image0004-raw.png` | `forward` | `dungeon_gameplay` | `48ed3743ab6a` |
| 5 | `image0005-raw.png` | `turn_left_2` | `wall_closeup` | `fbeb1b82cd09` |
| 6 | `image0006-raw.png` | `post_redraw` | `dungeon_gameplay` | `48ed3743ab6a` |

`original-viewpoint-route-keys.log` confirms serialized tokens in this order: startup waits/enter/one/click/one, then `shot:start`, `kp4`, `shot:turn_left`, `kp6`, `shot:turn_right`, `kp8`, `shot:forward`, `kp4`, `shot:turn_left_2`, `kp6`, `shot:post_redraw`.

`dosbox-original-viewports.log` confirms six DOSBox screenshots captured to `fires_000.png` through `fires_005.png`, which normalization materialized as `image0001-raw.png` through `image0006-raw.png`.

## Source seam binding boundary

The route is intended to exercise the previously audited ReDMCSB movement/viewport seam:

1. command queue/input dispatch: `COMMAND.C` `F0361_COMMAND_ProcessKeyPress` / `F0380_COMMAND_ProcessQueue_CPSC`;
2. movement handlers: `CLIKMENU.C` `F0365_COMMAND_ProcessTypes1To2_TurnParty` and `F0366_COMMAND_ProcessTypes3To6_MoveParty`;
3. move-result mutation path: `MOVESENS.C` `F0267_MOVE_GetMoveResult_CPSCE`;
4. redraw cadence: `GAMELOOP.C` `F0002_MAIN_GameLoop_CPSDF`;
5. viewport draw/present: `DUNVIEW.C` `F0128_DUNGEONVIEW_Draw_CPSF` and `DRAWVIEW.C` `F0097_DUNGEONVIEW_DrawViewport`.

The materialized frames do **not** promote this seam because fixed-sleep shots still include an entrance-menu frame and duplicate post-command views. The `forward` shot did not produce a distinct post-redraw viewport; it repeats the same raw SHA as shots 3 and 6.

## Exact blocker

The current N2 original-runner route can materialize raw DOSBox images and 224x136 crops, but it still cannot produce a non-duplicate, semantically aligned movement-only sequence. The blocker is route synchronization/state readiness: screenshots are still taken before the route is provably in party-control gameplay, and subsequent movement commands collapse to repeated two-state viewport hashes.

Non-claims: no <private-host> use, no push, no OpenAI API key, no Sonnet/q3.6 use, no original-vs-Firestaff pixel parity claim.
