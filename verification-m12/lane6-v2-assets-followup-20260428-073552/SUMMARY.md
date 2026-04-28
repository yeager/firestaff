# Lane 6 — V2 scaffolding/assets follow-up (2026-04-28 07:35 UTC)

Scope: V2-only manifest/scaffold/assets verification on N2. No DM1 V1 HUD/viewport/inventory/capture-lane files were edited.

## Results
- `build_v2_initial_4k_capture`: exit `2` — `cmake --build build --target firestaff_m11_v2_initial_4k_capture -j2`
- `build_v2_initial_4k_capture_after_fix`: exit `0` — `cmake --build build --target firestaff_m11_v2_initial_4k_capture -j2`
- `firestaff_m11_v2_initial_4k_capture`: exit `127` — ``
- `firestaff_m11_v2_initial_4k_capture_after_fix`: exit `2` — `build/firestaff_m11_v2_initial_4k_capture`
- `firestaff_m11_v2_initial_4k_capture_with_args`: exit `0` — `build/firestaff_m11_v2_initial_4k_capture verification-m12/lane6-v2-assets-followup-20260428-073552/v2_initial_4k_capture /home/trv2/.openclaw/data/firestaff-original-games/DM`
- `tools_validate_v2_manifests_check_files`: exit `0` — `python3 tools/validate_v2_manifests.py --check-files --json`
- `tools_validate_v2_manifests_check_files_text`: exit `0` — `python3 tools/validate_v2_manifests.py --check-files`
- `tools_validate_v2_manifests_json`: exit `0` — `python3 tools/validate_v2_manifests.py --json`
- `tools_validate_v2_manifests_selftest`: exit `0` — `python3 tools/validate_v2_manifests.py --self-test`
- `tools_validate_v2_manifests_strict_files`: exit `1` — `python3 tools/validate_v2_manifests.py --check-files --strict-files --json`
- `validate_v2_asset_manifests`: exit `0` — `python3 scripts/validate_v2_asset_manifests.py`
- `validate_v2_asset_manifests_strict`: exit `0` — `python3 scripts/validate_v2_asset_manifests.py --strict-paths`

## Key findings
- PASS: repo-local V2 manifest validation: 14 manifests / 98 assets.
- PASS: strict scaffold path validation: all manifest `masterDir`, `derivedDir`, and `spec` paths exist.
- PASS: validator self-test rejects non-50% downscale and duplicate `packId` cases.
- PASS: duplicate-pack/downscale JSON validation: 98/98 assets satisfy exact 4K -> 1080p 50% policy, 0 errors.
- PASS after local fix: V2 initial 4K capture target builds and writes `base_scene.ppm`, `creature_scene.ppm`, and `scene_meta.txt`.
- PASS after comment-only rebuild: V2 capture target still builds after documenting the static-link order.
- BLOCKER for strict file-art completion: `--check-files --strict-files` fails with 12 missing expected-size PNG deliverables. Non-strict `--check-files` reports them as warnings, not schema/scaffold errors.

## Missing-art warnings
- WARN: assets-v2/manifests/firestaff-v2-all-assets-foundation.manifest.json: fs.v2.foundation.ui-hud-shells.wave1 has no PNG in assets-v2/ui/wave1 with expected size 3200x2000
- WARN: assets-v2/manifests/firestaff-v2-all-assets-foundation.manifest.json: fs.v2.foundation.ui-hud-shells.wave1 has no PNG in assets-v2/ui/wave1 with expected size 1600x1000
- WARN: assets-v2/manifests/firestaff-v2-all-assets-foundation.manifest.json: fs.v2.foundation.creatures.wave1 has no PNG in assets-v2/creatures/wave1 with expected size 1200x1200
- WARN: assets-v2/manifests/firestaff-v2-all-assets-foundation.manifest.json: fs.v2.foundation.creatures.wave1 has no PNG in assets-v2/creatures/wave1 with expected size 600x600
- WARN: assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json: fs.v2.ui.action-area.base has no PNG in assets-v2/ui/wave1/action-area/masters/4k with expected size 870x450
- WARN: assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json: fs.v2.ui.action-area.base has no PNG in assets-v2/ui/wave1/action-area/exports/1080p with expected size 435x225
- WARN: assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json: fs.v2.ui.status-box.left-frame has no PNG in assets-v2/ui/wave1/status-boxes/masters/4k with expected size 670x290
- WARN: assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json: fs.v2.ui.status-box.left-frame has no PNG in assets-v2/ui/wave1/status-boxes/exports/1080p with expected size 335x145
- WARN: assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json: fs.v2.ui.status-box.right-frame has no PNG in assets-v2/ui/wave1/status-boxes/masters/4k with expected size 670x290
- WARN: assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json: fs.v2.ui.status-box.right-frame has no PNG in assets-v2/ui/wave1/status-boxes/exports/1080p with expected size 335x145
- WARN: assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json: fs.v2.ui.party-hud-slot.standard has no PNG in assets-v2/ui/wave1/party-hud-cells/masters/4k with expected size 216x216
- WARN: assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json: fs.v2.ui.party-hud-slot.standard has no PNG in assets-v2/ui/wave1/party-hud-cells/exports/1080p with expected size 108x108

## Local fix
`CMakeLists.txt` now links `firestaff_m11_v2_initial_4k_capture` with `firestaff_m12` on both sides of `firestaff_m11` to resolve the static-library cycle where M11 references M12 startup/asset helpers. This is bounded to the V2 capture target.

## Generated capture
- `v2_initial_4k_capture/base_scene.ppm` (192015 bytes)
- `v2_initial_4k_capture/creature_scene.ppm` (192015 bytes)
- `v2_initial_4k_capture/scene_meta.txt` (207 bytes)
