# Pass 70 — original DM1 viewport crops and first diff results

Date: 2026-04-26

## What changed

The original DOS game is available locally and was used through DOSBox Staging to produce six `224x136` original viewport reference crops under:

- `verification-screens/pass70-original-dm1-viewports/viewport_224x136/`

The capture helper now supports `DM1_ROUTE_SKIP_STARTUP_SELECTOR=1` for runs where the generated DOSBox config starts `DM VGA` directly but the explicit route still needs to drive the PC 3.4 graphics/sound/input selectors itself.

## Route used for this evidence pass

```sh
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:1000 one enter wait:800 one enter wait:800 one enter wait:6000 right wait:5000 shot wait:500 shot wait:500 shot wait:500 shot enter wait:1200 shot wait:500 shot' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Honesty note: this route is a confirmed original DOS capture route, but it is not a full semantic match for every Firestaff deterministic action label yet. Several early crops intentionally capture the same closed-door viewport because keyboard movement/action mapping still needs a tighter original-control route.

## Normalization gate

- Raw screenshots normalized: 6
- Crop geometry: exactly `224x136`
- Manifest: `verification-screens/pass70-original-dm1-viewports/original_viewport_224x136_manifest.tsv`

## Pixel diff results

Command:

```sh
tools/pass70_viewport_pair_compare.py --plan-json parity-evidence/overlays/pass70/pass70_viewport_pairing_plan.json --run-diff
```

Results, tolerance 0 per channel:

| Pair | Differing pixels | Total | Delta |
| --- | ---: | ---: | ---: |
| 01_ingame_start | 24701 | 30464 | 81.0826% |
| 02_ingame_turn_right | 24767 | 30464 | 81.2992% |
| 03_ingame_move_forward | 20792 | 30464 | 68.2511% |
| 04_ingame_spell_panel | 20792 | 30464 | 68.2511% |
| 05_ingame_after_cast | 24068 | 30464 | 79.0047% |
| 06_ingame_inventory_panel | 28480 | 30464 | 93.4874% |

Worst pair: `06_ingame_inventory_panel` at `93.4874%`.

## Next blocker

The remaining blocker is not missing original media anymore. It is exact original input parity: map the Firestaff deterministic actions to DOS DM controls/mouse events so that all six original crops represent the same semantic states as the Firestaff capture labels.
