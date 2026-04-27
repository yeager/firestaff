# Pass 84 — original overlay comparison readiness probe

Date: 2026-04-26

## Goal

Audit whether a fresh `origin/main` worktree can run the V1 original-vs-Firestaff fullscreen overlay comparison for the six-step route:

1. in-game start,
2. turn right,
3. move forward,
4. spell panel,
5. after cast,
6. champion inventory panel.

This pass is evidence-only. It does not generate reference screenshots, does not promote screenshots, and does not claim pixel parity.

## Probe

- Tool: `tools/pass84_original_overlay_readiness_probe.py`
- Machine-readable output: `parity-evidence/pass84_original_overlay_readiness_probe.json`

Command:

```sh
python3 tools/pass84_original_overlay_readiness_probe.py > parity-evidence/pass84_original_overlay_readiness_probe.json
```

Result summary:

```text
ready_for_overlay_comparison=false
blockers=5
```

## Findings

- The six review PNGs exist in the fresh worktree, so Firestaff-side visual review frames are present.
- The default pass-74 comparison is not reproducible from tracked inputs in this fresh worktree:
  - six `verification-screens/*_latest.ppm` Firestaff full-frame inputs are absent;
  - six `verification-screens/pass70-original-dm1-viewports/image000*-raw.png` original raw screenshots are absent;
  - the existing pass-74 stats JSON records six pairs, but references artifacts that are not present in the fresh worktree.
- Existing pass-78 original-route attempts remain negative evidence: all four audited attempts report `all_gameplay_320x200=false` and only `720x400` text-mode/prompt captures.
- The pass-84 readiness probe now audits `verification-screens/pass70-original-dm1-viewports/original_viewport_shot_labels.tsv` and blocks readiness if the semantic route labels are absent or do not match `party_hud`, `spell_panel`, and `inventory_panel` at the expected six-shot checkpoints.
- The original route script accepts explicit `f1`-`f4` champion keys and validates six-shot route shape. It now also accepts labeled capture tokens such as `shot:party_hud`, `shot:spell_panel`, and `shot:inventory_panel`, and writes those labels to `original_viewport_shot_labels.tsv` during normalization. This is metadata support only; it does not prove that the original runtime reaches the champion party/inventory/spell-panel states.

## Champion party / inventory / spell-panel blocker

The next overlay comparison must not be called pixel parity until the original runtime produces six verified `320x200` gameplay raw frames for the same semantic route as Firestaff, especially:

- champion party selection via explicit `f1`-`f4` or validated equivalent original input;
- spell-panel opening and rune/cast state in the right-column C013/C011 areas;
- inventory panel opening for the selected champion, not a close-wall or text-mode false positive.

At present, the route is **not semantically locked**. The tracked evidence is ready to describe the blocker, not to claim parity.

## Verification run in this worktree

```sh
python3 -m py_compile tools/pass80_original_frame_classifier.py tools/pass78_original_route_attempt_audit.py tools/pass74_fullscreen_panel_pair_compare.py tools/pass84_original_overlay_readiness_probe.py
python3 tools/pass80_original_frame_classifier.py --self-test
DM1_ORIGINAL_ROUTE_EVENTS="wait:5000 shot:party_hud f1 wait:500 shot f2 wait:500 shot one wait:500 shot:spell_panel four wait:100 four wait:100 enter wait:1000 shot i wait:800 shot:inventory_panel" scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run
```

Observed:

- classifier self-test: `pass=true`, `cases=3`;
- pass-84 readiness regeneration: `ready_for_overlay_comparison=false`, `blockers=5`, including absent `original_viewport_shot_labels.tsv`;
- route dry-run: staged DM1 tree is missing when absent, but helper generation and six-shot shape validation with three labeled shots succeed;
- direct `python3 tools/pass74_fullscreen_panel_pair_compare.py` is currently blocked by missing fresh-worktree PPM/raw inputs, so no new overlay measurements were claimed.


## Route-label metadata follow-up

A follow-up tooling pass made the route labels explicit without changing M10 semantics or claiming parity:

- `shot:<label>` is accepted wherever `shot` was accepted;
- label syntax is restricted to lowercase `a-z`, digits, `_`, and `-`;
- the required count remains exactly six captures;
- normalization keeps the legacy six crop filenames stable and writes a separate `original_viewport_shot_labels.tsv` mapping each crop to its route label/token.

The intended overlay-readiness labels are `shot:party_hud`, `shot:spell_panel`, and `shot:inventory_panel`. These labels are assertions about the requested route checkpoints only. A separate raw-frame/classifier audit is still required before treating any capture as semantically matched original evidence.
