# DM1 V1 original-faithful parity/evidence retry — 2026-04-28 09:31 CEST

Lane: DM1 V1 original-faithful parity/evidence retry
Host: N2 (`Firestaff-Worker-VM`)
Repo: `/home/trv2/work/firestaff`
Run-dir: `verification-m11/lane1-original-faithful-parity-20260428-0931/`

## Result

Evidence-only rerun completed. Source-data provenance and Firestaff source-shape/runtime gates are green. Original-runtime overlay comparison is still correctly **not ready**; no pixel-perfect/original-overlay parity is claimed.

## Commands

| Command | Exit | Result |
| --- | ---: | --- |
| `python3 tools/validate_dm1_pc34_provenance.py --check-extracted` | 0 | PASS: local archive and extracted PC34 `DUNGEON.DAT`, `GRAPHICS.DAT`, `SONG.DAT` match locked hashes. |
| `python3 tools/pass84_original_overlay_readiness_probe.py` | 0 | Probe ran; `ready_for_overlay_comparison=false` remains correct. |
| `cmake --build build --target firestaff_m11_game_view_probe` | 0 | PASS: game-view probe target builds. |
| `build/firestaff_m11_game_view_probe` | 0 | PASS: `578/578 invariants passed`. |
| `python3 tools/verify_v1_viewport_draw_order_gate.py` | 0 | PASS: viewport draw-order source-shape gate. |
| `python3 tools/verify_v1_viewport_occlusion_gate.py` | 0 | PASS: viewport occlusion/source-sampling gate. |
| `env DM1_ORIGINAL_STAGE_DIR=/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 DM1_ORIGINAL_ROUTE_EVENTS='wait:5000 shot:party_hud f1 wait:500 shot f2 wait:500 shot one wait:500 shot:spell_panel four wait:100 four wait:100 enter wait:1000 shot i wait:800 shot:inventory_panel' scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run` | 0 | PASS: route shape validates as 21 tokens, 6 shots, 3 labeled (`party_hud`, `spell_panel`, `inventory_panel`). |
| `./run_firestaff_m11_ingame_capture_smoke.sh` | 0 | PASS: six Firestaff screenshots emitted. |

## pass84 blockers still present

- Default pass74 compare inputs are missing Firestaff PPM full-frame captures.
- Default pass74 compare inputs are missing original raw `320x200` screenshots.
- Recorded pass74 stats reference artifacts are absent from the fresh worktree.
- Tracked pass78 route attempts remain text-mode/prompt captures, not gameplay.
- `verification-screens/pass70-original-dm1-viewports/original_viewport_shot_labels.tsv` is absent, so semantic route checkpoints are not auditable.

## Interpretation / next step

The consolidated gate state is internally consistent after the viewport/HUD/inventory/original-overlay follow-ups: source provenance is green, Firestaff V1 source-shape gates are green, the current route shape is serializable, and the broad game-view probe is green. The remaining blocker is still the semantic original PC 3.4 runtime route: capture six accepted raw gameplay frames and the shot-label manifest before rerunning pass84/pass74 overlay comparison or promoting pixel parity.
