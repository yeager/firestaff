# Pass 102 — original overlay/capture follow-up on N2

Date: 2026-04-28
Lane: Original overlay/capture unblock for viewport/world
Host: N2 (`Firestaff-Worker-VM`)
Branch: `sync/n2-dm1-v1-20260428`
Run-dir: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-0902-original-overlay-capture-followup/`

## Scope

Follow-up evidence run for the remaining viewport/world blocker: semantically matched original DM1 gameplay capture/route before any pixel-perfect runtime parity claim. This pass reruns the available provenance, original overlay readiness, route-shape, original GRAPHICS.DAT presentation/screen, and viewport/world gates. It does not promote original screenshots or claim original-runtime pixel parity.

## Commands and results

| Command | rc | Result |
| --- | ---: | --- |
| `python3 tools/validate_dm1_pc34_provenance.py` | 0 | DM1 PC 3.4 archive/extracted provenance PASS |
| `python3 tools/pass84_original_overlay_readiness_probe.py` | 0 | `ready_for_overlay_comparison=false` |
| `DM1_ORIGINAL_STAGE_DIR=/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 DM1_ORIGINAL_ROUTE_EVENTS='wait:5000 shot:party_hud f1 wait:500 shot f2 wait:500 shot one wait:500 shot:spell_panel four wait:100 four wait:100 enter wait:1000 shot i wait:800 shot:inventory_panel' scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run` | 0 | route shape OK: 21 tokens, 6 shots, 3 labeled |
| `./run_firestaff_m11_ingame_capture_smoke.sh` | 0 | `In-game capture smoke PASS: 6 screenshots` |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh` | 0 | wrapper/build gate passed |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh` | 0 | wrapper/build gate passed |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh` | 0 | wrapper/build gate passed |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh "$GRAPHICS_DAT" "$RUN_DIR/original_visible_dispatch_index0.pgm" 0` | 0 | ok, `224x136` |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh "$GRAPHICS_DAT" 0` | 0 | ok, `presentedBytes=15232` |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh "$GRAPHICS_DAT" 0` | 0 | ok, `screenCopiedBytes=15232` |
| same visible/present/screen probes for indexes `78` and `79` | 0 | ok, `224x97` floor and `224x39` ceiling |
| `python3 tools/verify_v1_viewport_draw_order_gate.py` | 0 | PASS |
| `python3 tools/verify_v1_viewport_occlusion_gate.py` | 0 | PASS |
| `./run_firestaff_m11_game_view_probe.sh` | 0 | `578/578 invariants passed` |

Full stdout/stderr and command files are in the run-dir. `summary.tsv` records the exact command, rc, and observed marker for each step.

## Findings

The previous default-index original GRAPHICS.DAT probe blocker is cleared in this tree: index `0` now passes visible-dispatch, present, and screen probes without the prior heap abort/mismatch. Positive viewport source controls `78` and `79` still pass.

The remaining blocker is therefore narrower and unchanged at the runtime-route level: pass84 still reports overlay comparison is not ready because the fresh worktree lacks the promoted six Firestaff PPM full-frame captures, six original raw `320x200` screenshots, referenced pass74 artifacts, and `original_viewport_shot_labels.tsv`; tracked pass78 attempts also remain prompt/text-mode negative evidence.

## Minimum next fix

Capture or import a validated original DOSBox route that produces six auditable `320x200` gameplay raw screenshots plus `original_viewport_shot_labels.tsv` with the canonical semantic checkpoints:

`party_hud`, blank, blank, `spell_panel`, blank, `inventory_panel`.

Then run `tools/pass80_original_frame_classifier.py` / `tools/pass86_original_viewport_crop_manifest.py` before rerunning pass84 and pass70/pass74 overlay comparisons. Until that exists, viewport/world may keep source/probe gates green but must not claim pixel-perfect original runtime parity.
