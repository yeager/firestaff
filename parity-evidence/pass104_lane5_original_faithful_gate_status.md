# Pass 104 — Lane 5 DM1 V1 original-faithful gate status

Date: 2026-04-28
Lane: 5 — DM1 V1 original-faithful parity/evidence
Host: N2 (`Firestaff-Worker-VM`)
Branch: `sync/n2-dm1-v1-20260428`
Run-dir: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-0920-lane5-original-faithful-evidence-followup/`

## Scope

Evidence-only consolidation after pass103. This pass reruns the current source/data and V1 gate stack and records the current honesty boundary: which gates are original-faithful/source-backed now, and which remain blocked by the missing semantic original DM1 gameplay route. No renderer, runtime, capture-route implementation, or lane-owned HUD/inventory/viewport files were changed.

## Commands and results

| Command | rc | Result |
| --- | ---: | --- |
| `python3 tools/validate_dm1_pc34_provenance.py` | 0 | PASS: local archive and extracted `DUNGEON.DAT`, `GRAPHICS.DAT`, `SONG.DAT` match locked DM1 PC 3.4 hashes |
| `python3 tools/pass84_original_overlay_readiness_probe.py --out-json "$RUN_DIR/pass84_readiness.json" --out-md "$RUN_DIR/pass84_readiness.md"` | 0 | Probe ran; `ready_for_overlay_comparison=false` remains correct |
| canonical original route `--dry-run` with `shot:party_hud`, `shot:spell_panel`, `shot:inventory_panel` | 0 | route shape OK: 21 tokens, 6 shots, 3 labeled |
| `python3 tools/verify_v1_viewport_draw_order_gate.py` | 0 | PASS: source-shape draw order verified |
| `python3 tools/verify_v1_viewport_occlusion_gate.py` | 0 | PASS: source-shape occlusion/sample gates verified |
| `./run_firestaff_m11_ingame_capture_smoke.sh` | 0 | PASS: six Firestaff screenshots emitted |
| `./run_firestaff_m11_game_view_probe.sh` | 0 | PASS: `578/578 invariants passed` |

Full logs and `summary.tsv` are in the run-dir.

## Current original-faithful / source-backed gates

These are safe to treat as original-faithful source/probe coverage, not as pixel-perfect original runtime overlay parity:

- DM1 PC 3.4 data provenance is green for archive and extracted `DUNGEON.DAT`, `GRAPHICS.DAT`, and `SONG.DAT`.
- V1 viewport draw-order source shape is green: wall/door ornaments before open-cell contents; open-cell order remains floor ornaments → floor items → creatures → projectiles/effects.
- V1 viewport occlusion/source sampling shape is green across max-visible-forward, pits, floor ornaments, stairs, teleporter fields, side walls, side doors, side-door ornaments, destroyed-door masks, and viewport call-site wiring.
- Firestaff deterministic in-game capture smoke is green with six current screenshots.
- Broad M11 game-view invariant gate is green at `578/578`.
- The original route helper can still serialize the canonical six-shot route shape with three semantic labels.

## Still blocked by semantic original route

These must not be promoted to pixel-perfect/original-runtime parity yet:

- `pass84` still reports `ready_for_overlay_comparison=false`.
- Default pass74 comparison inputs are still missing Firestaff PPM full-frame captures.
- Default pass74 comparison inputs are still missing original raw `320x200` gameplay screenshots.
- Recorded pass74 stats reference artifacts are absent from the fresh worktree.
- Tracked pass78 route attempts remain text-mode/prompt captures, not gameplay.
- `verification-screens/pass70-original-dm1-viewports/original_viewport_shot_labels.tsv` is still absent, so semantic checkpoints are not auditable.

Pass103 reduced the mechanical N2 capture-route blocker: `xvfb-run`/DOSBox/`xdotool` can capture and normalize six raw frames on N2. The unresolved blocker is semantic: the N2 route still has not proven a transition into the expected dungeon gameplay states. Until that route produces classified gameplay shots for `party_hud`, spell panel, and inventory panel without duplicate/menu/title failures, pass84 should remain false and overlay/pixel parity claims should stay blocked.

## Minimum next landing target

Find and verify the original PC 3.4 N2 entrance/menu action sequence that reaches dungeon gameplay, then rerun the existing pass103 route under `xvfb-run`. Promote artifacts only after `tools/pass80_original_frame_classifier.py` accepts the six screenshots and a shot-label manifest exists for the canonical semantic checkpoints.
