# Pass 70 — viewport pair compare helper

Date: 2026-04-26

## Scope

Prepared the next Firestaff-vs-original viewport pixel/content pass without claiming pixel parity. Original DM1 viewport crops are still absent in this worktree, so this pass adds a robust pairing/inventory helper and a machine-readable plan that can run the moment original crops arrive.

## Inventory result

Present after a local Firestaff capture run:

- `verification-screens/capture_manifest_sha256.tsv`
- six Firestaff `224x136` viewport crops:
  - `01_ingame_start_latest_viewport_224x136.ppm/png`
  - `02_ingame_turn_right_latest_viewport_224x136.ppm/png`
  - `03_ingame_move_forward_latest_viewport_224x136.ppm/png`
  - `04_ingame_spell_panel_latest_viewport_224x136.ppm/png`
  - `05_ingame_after_cast_latest_viewport_224x136.ppm/png`
  - `06_ingame_inventory_panel_latest_viewport_224x136.ppm/png`

Still missing:

- `verification-screens/pass70-original-dm1-viewports/original_viewport_224x136_manifest.tsv`
- six original DM1 PC 3.4 crops under `verification-screens/pass70-original-dm1-viewports/viewport_224x136/`

The helper wrote the current pairing inventory to:

```text
parity-evidence/overlays/pass70/pass70_viewport_pairing_plan.json
```

## New helper

Added:

```sh
tools/pass70_viewport_pair_compare.py
```

It defines the exact crop pairings, validates manifest schema/geometry, writes a JSON plan, and can run six whole-crop `224x136` diffs only when both sides exist.

Manifest schema for both sides:

```text
kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256
```

Accepted Firestaff kind:

```text
viewport_224x136
```

Accepted original kind:

```text
original_viewport_224x136
```

Geometry is hard-gated to `224x136`.

## Exact pairings

| # | Scene | Firestaff crop | Original crop |
|---|---|---|---|
| 01 | `ingame_start` | `verification-screens/01_ingame_start_latest_viewport_224x136.png` | `verification-screens/pass70-original-dm1-viewports/viewport_224x136/01_ingame_start_original_viewport_224x136.png` |
| 02 | `ingame_turn_right` | `verification-screens/02_ingame_turn_right_latest_viewport_224x136.png` | `verification-screens/pass70-original-dm1-viewports/viewport_224x136/02_ingame_turn_right_original_viewport_224x136.png` |
| 03 | `ingame_move_forward` | `verification-screens/03_ingame_move_forward_latest_viewport_224x136.png` | `verification-screens/pass70-original-dm1-viewports/viewport_224x136/03_ingame_move_forward_original_viewport_224x136.png` |
| 04 | `ingame_spell_panel` | `verification-screens/04_ingame_spell_panel_latest_viewport_224x136.png` | `verification-screens/pass70-original-dm1-viewports/viewport_224x136/04_ingame_spell_panel_original_viewport_224x136.png` |
| 05 | `ingame_after_cast` | `verification-screens/05_ingame_after_cast_latest_viewport_224x136.png` | `verification-screens/pass70-original-dm1-viewports/viewport_224x136/05_ingame_after_cast_original_viewport_224x136.png` |
| 06 | `ingame_inventory_panel` | `verification-screens/06_ingame_inventory_panel_latest_viewport_224x136.png` | `verification-screens/pass70-original-dm1-viewports/viewport_224x136/06_ingame_inventory_panel_original_viewport_224x136.png` |

## Commands

Inventory/pairing plan:

```sh
tools/pass70_viewport_pair_compare.py \
  --plan-json parity-evidence/overlays/pass70/pass70_viewport_pairing_plan.json
```

Run all six diffs after original crops exist:

```sh
tools/pass70_viewport_pair_compare.py \
  --plan-json parity-evidence/overlays/pass70/pass70_viewport_pairing_plan.json \
  --run-diff
```

Equivalent one-off overlay command pattern (first row):

```sh
python3 tools/redmcsb_overlay_diff.py \
  --firestaff verification-screens/01_ingame_start_latest_viewport_224x136.png \
  --reference verification-screens/pass70-original-dm1-viewports/viewport_224x136/01_ingame_start_original_viewport_224x136.png \
  --region-xywh 0,0,224,136 \
  --out parity-evidence/overlays/pass70/01_ingame_start_viewport_original_vs_firestaff
```

## Verification performed

```sh
python3 -m py_compile tools/pass70_viewport_pair_compare.py
./run_capture_screenshots.sh "$HOME/.firestaff/data"
tools/pass70_viewport_pair_compare.py --plan-json parity-evidence/overlays/pass70/pass70_viewport_pairing_plan.json
tools/pass70_viewport_pair_compare.py --run-diff
```

Observed blocker from `--run-diff` is expected and honest:

```text
[blocked] missing original manifest: .../verification-screens/pass70-original-dm1-viewports/original_viewport_224x136_manifest.tsv
[blocked] not all six Firestaff/original viewport crop pairs are present
```

## Claim boundary

No original crops are present, no pixel diff was run, and no parity is claimed. This pass only prepares the exact compare path and locks Firestaff-side crop evidence for the next source-backed diff pass.
