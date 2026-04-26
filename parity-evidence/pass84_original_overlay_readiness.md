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
blockers=4
```

## Findings

- The six review PNGs exist in the fresh worktree, so Firestaff-side visual review frames are present.
- The default pass-74 comparison is not reproducible from tracked inputs in this fresh worktree:
  - six `verification-screens/*_latest.ppm` Firestaff full-frame inputs are absent;
  - six `verification-screens/pass70-original-dm1-viewports/image000*-raw.png` original raw screenshots are absent;
  - the existing pass-74 stats JSON records six pairs, but references artifacts that are not present in the fresh worktree.
- Existing pass-78 original-route attempts remain negative evidence: all four audited attempts report `all_gameplay_320x200=false` and only `720x400` text-mode/prompt captures.
- The original route script accepts explicit `f1`-`f4` champion keys and validates six-shot route shape, but this only proves manifest support. It does not prove that the original runtime reaches the champion party/inventory/spell-panel states.

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
DM1_ORIGINAL_ROUTE_EVENTS="wait:5000 shot f1 wait:500 shot f2 wait:500 shot one wait:500 shot four wait:100 four wait:100 enter wait:1000 shot i wait:800 shot" scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run
```

Observed:

- classifier self-test: `pass=true`, `cases=3`;
- route dry-run: staged DM1 tree is missing, but route manifest generation and six-shot shape validation succeed;
- direct `python3 tools/pass74_fullscreen_panel_pair_compare.py` is currently blocked by missing fresh-worktree PPM/raw inputs, so no new overlay measurements were claimed.
