# Pass210 — DM1 V1 original movement/viewport capture unblock

Status: `BLOCKED_ON_PROMOTABLE_ORIGINAL_ROUTE_ARTIFACT`

Scope: N2-only follow-up after the M15 movement source file map. This report does not change renderer/runtime code and does not claim original-vs-Firestaff pixel parity. It identifies the next concrete blocker for original-runner movement/viewport evidence.

## Source boundary checked

M15 resolved the requested missing split movement filenames to the actual ReDMCSB source map:

- `COMMAND.C` / `F0380_COMMAND_ProcessQueue_CPSC` gates movement commands and dispatches turn/step handlers.
- `CLIKMENU.C` / `F0365_COMMAND_ProcessTypes1To2_TurnParty` mutates party direction and releases input wait.
- `CLIKMENU.C` / `F0366_COMMAND_ProcessTypes3To6_MoveParty` resolves relative step, static collision, group collision, and calls move-result state transition.
- `MOVESENS.C` / `F0267_MOVE_GetMoveResult_CPSCE` owns party coordinate mutation, teleporter/pit chain resolution, move-result globals, departure/arrival sensors, and object/group/projectile movement side effects.
- `GAMELOOP.C` / `F0002_MAIN_GameLoop_CPSDF` redraws dungeon view from current party state before the next input wait.
- `DUNVIEW.C` / `F0128_DUNGEONVIEW_Draw_CPSF` and `DRAWVIEW.C` / `F0097_DUNGEONVIEW_DrawViewport` define the comparable 224x136 viewport draw/present seam.

So the next blocker is no longer source traceability. It is runtime capture evidence.

## Existing artifacts checked

Verification commands run in this worktree:

```sh
python3 tools/pass206_dm1_v1_original_runner_minimal_gate.py
python3 tools/pass207_dm1_v1_original_movement_viewport_blocker_gate.py
DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 \
DOSBOX=/usr/bin/dosbox \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 shot:party_hud kp5 wait:700 shot kp5 wait:700 shot f1 wait:700 shot:spell_panel one wait:700 shot i wait:700 shot:inventory_panel' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run
```

Observed:

- pass206: `BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE`; missing tools: `[]`; canonical `DM.EXE`, `DATA/DUNGEON.DAT`, and `DATA/GRAPHICS.DAT` all match expected hashes.
- pass207: `BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE`; 14/14 ReDMCSB seam checks pass.
- Dry-run route shape is valid: 25 tokens, 6 shots, labels for `party_hud`, `spell_panel`, `inventory_panel`.

## Exact blocker

No runner tool or original data file is missing. The missing item is a **promotable original-runner movement/viewport capture artifact set**:

- `verification-screens/pass112-n2-stable-hud-route/` has tracked manifests/classifier output, but the ignored raw screenshots and viewport crops are not materialized in this worktree.
- Its classifier result is not promotable anyway: 6 raw frames recorded, 4 semantic mismatches, and 4 duplicate `wall_closeup` frames with SHA `ee7741746ea9b30739238e9f0780f57982bd0abe07bf60cea24e9cf92018e89c`.
- pass207 confirms the manifest boundary only preserves filenames/dimensions/labels/hashes for ignored PNG/PPM assets; it does not prove the route reached the post-command redraw seam.
- `verification-screens/pass209-delayed-click-zone-route/` is also not promotable: manifests exist, but materialized ignored PNG/PPM files are absent in this worktree, the classifier reports duplicate raw frames, and shots 4/6 remain `dungeon_gameplay` instead of `spell_panel`/`inventory`.

The precise missing evidence is therefore:

1. materialized DOSBox raw frames (`imageNNNN-raw.png`) for a new/clean N2 attempt;
2. materialized normalized `viewport_224x136/*.ppm`/`.png` crops for the same attempt;
3. a classifier/audit JSON where movement shots are semantically aligned and not duplicate/stale frames;
4. enough route metadata/logging to tie each accepted shot to the ReDMCSB path: command queue -> turn/step handler -> `GAMELOOP.C` redraw -> `DRAWVIEW.C` viewport present.

## Next landable action

Land the next pass as an evidence-only capture attempt, not a renderer change:

1. Create a fresh attempt directory, e.g. `verification-screens/pass210-n2-original-movement-route/`.
2. Run `scripts/dosbox_dm1_original_viewport_reference_capture.sh --run` on N2 under `xvfb-run` with `OUT_DIR` set to that directory and with a route focused on movement only: initial gameplay, turn, forward/step, and one post-redraw confirmation shot. Do not require spell/inventory panels in the movement gate; they are separate semantic routes.
3. Run `scripts/dosbox_dm1_original_viewport_reference_capture.sh --normalize-only` for the same `OUT_DIR` if needed, then run:

```sh
python3 tools/pass80_original_frame_classifier.py \
  verification-screens/pass210-n2-original-movement-route \
  --expected pass210-movement \
  --fail-on-duplicates
```

`--expected pass210-movement` is a gameplay-only four-shot preset, and `--fail-on-duplicates` is mandatory for promotion. Together they make the pass210 path strict: stale/repeated raw screenshots, spell panels, inventory panels, entrance-menu frames, and wall-closeups all fail the movement gate instead of being mixed into the movement evidence.

4. Only if classifier passes with non-duplicate movement frames, add a small follow-up gate/report that points pass206/pass207 at the new attempt directory and records the raw/crop manifests. If classifier fails, preserve the attempt as a blocker with the exact mismatched shot/log tail.

Recommended route adjustment: split movement/viewport proof from HUD panel proof. The current six-shot mixed route can be valid as a broad UI probe, but it is a bad merge blocker for movement because spell/inventory failures obscure the thing M15 actually unblocked: source-locked post-command viewport redraw evidence.
