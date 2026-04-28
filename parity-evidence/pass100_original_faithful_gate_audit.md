# Pass 100 — DM1 V1 original-faithful gate audit

Date: 2026-04-28
Lane: DM1 V1 original-faithful parity/evidence
Run-dir: `parity-evidence/runs/pass100_original_faithful_gate_audit_20260428T065249Z`

## Scope

Evidence-only audit of source-data provenance, original-overlay readiness, and viewport/world source-shape gates. This pass does not claim pixel-perfect original-runtime parity and deliberately avoids changing active renderer/runtime files.

## Commands

```sh
python3 tools/validate_dm1_pc34_provenance.py
python3 tools/pass84_original_overlay_readiness_probe.py --out-json "parity-evidence/runs/pass100_original_faithful_gate_audit_20260428T065249Z/pass84_readiness.json" --out-md "parity-evidence/runs/pass100_original_faithful_gate_audit_20260428T065249Z/pass84_readiness.md"
python3 tools/verify_v1_viewport_draw_order_gate.py
python3 tools/verify_v1_viewport_occlusion_gate.py
cmake --build build --target firestaff_m11_game_view_probe -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
```

Full local log: `parity-evidence/runs/pass100_original_faithful_gate_audit_20260428T065249Z/output.log` (ignored local run artifact).

## Results

- DM1 PC 3.4 provenance: PASS for both local archive and extracted `DUNGEON.DAT`, `GRAPHICS.DAT`, and `SONG.DAT` hashes.
- Original overlay readiness: FAIL / not ready, as expected. The blocker is still semantic original-route evidence, not Firestaff source-data provenance.
- Viewport draw-order source-shape gate: PASS.
- Viewport occlusion source-shape gate: PASS.
- `firestaff_m11_game_view_probe`: PASS, `578/578 invariants passed`.

## Overlay readiness blockers recorded by pass84

- Default pass74 compare inputs are missing Firestaff PPM full-frame captures.
- Default pass74 compare inputs are missing original raw 320×200 screenshots.
- Recorded pass74 stats reference artifacts are absent from the fresh worktree.
- Pass78 route attempts remained text-mode/prompt captures, not gameplay.
- Original shot-label manifest is absent, so semantic route checkpoints are not auditable.

## Interpretation

The original-faithful gate stack is internally consistent on N2: source-data hashes match the locked DM1 PC 3.4 truth, source-shape viewport/world gates pass, and the broad game-view probe remains green. The audit keeps the honesty boundary explicit: original overlay comparison remains blocked until a semantically locked original gameplay route produces auditable 320×200 raw screenshots and a shot-label manifest.
