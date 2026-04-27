# Pass 86 — original viewport crop/manifest builder

This pass adds `tools/pass86_original_viewport_crop_manifest.py`, an evidence-only helper for preparing original DM1 runtime screenshots for DM1/V1 parity comparison.

Purpose:

- read a six-frame DOSBox Staging raw screenshot route (`imageNNNN-raw.png`)
- reuse the pass-80 semantic classifier before trusting frames as route evidence
- require the canonical six-scene sequence by default:
  1. dungeon gameplay
  2. dungeon gameplay after right turn
  3. dungeon gameplay after move forward
  4. spell panel
  5. dungeon gameplay after cast
  6. inventory panel
- crop the DM1 viewport rectangle `0,33,224,136`
- emit pass-70-compatible original viewport PNG/PPM files and `original_viewport_224x136_manifest.tsv`

Honesty: this is original-reference input preparation only. It does not capture gameplay by itself and does not claim pixel parity.

Example dry run against an existing route attempt:

```sh
python3 tools/pass86_original_viewport_crop_manifest.py \
  /path/to/original-route-attempt \
  --dry-run
```

Example artifact build once the route labels pass:

```sh
python3 tools/pass86_original_viewport_crop_manifest.py \
  /path/to/original-route-attempt \
  --out-dir verification-screens/pass70-original-dm1-viewports

python3 tools/pass70_viewport_pair_compare.py \
  --original-dir verification-screens/pass70-original-dm1-viewports/viewport_224x136 \
  --original-manifest verification-screens/pass70-original-dm1-viewports/original_viewport_224x136_manifest.tsv \
  --plan-json parity-evidence/overlays/pass70/pass70_viewport_pairing_plan.json
```

If the classifier reports `wall_closeup`, `title_or_menu`, or `graphics_320x200_unclassified`, do not promote the crops as parity evidence. Fix the original route capture first.
