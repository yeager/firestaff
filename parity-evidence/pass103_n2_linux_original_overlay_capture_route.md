# Pass 103 — N2 Linux original overlay/capture route follow-up

Date: 2026-04-28
Lane: 4 — original overlay/capture unblock for HUD/viewport
Host: N2 (`Firestaff-Worker-VM`)
Branch: `sync/n2-dm1-v1-20260428`
Run-dir: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-0903-lane4-overlay-capture-route-followup/`

## Scope

Focused follow-up on the original DOSBox capture route gate. This pass keeps away from HUD/inventory implementation files and only touches the original reference-capture route helper so the route can execute on N2/Linux instead of being macOS/Swift-only.

## Route tooling update

`scripts/dosbox_dm1_original_viewport_reference_capture.sh` now has a Linux/N2 path:

- writes a generated `original_viewport_route_keys_xdotool.sh` helper when preparing a run;
- supports `xvfb-run -a` + `/usr/bin/dosbox` + `xdotool` for route key/click injection when Swift is unavailable;
- writes `captures=${OUT_DIR}` for DOSBox 0.74, whose capture directory is configured under `[dosbox]`;
- normalizes DOSBox 0.74 screenshot names (`anim_000.png`, `selector_*.png`, etc.) into stable `image000N-raw.png` files when exactly six raw screenshots are present, preserving downstream pass70/pass84 expectations.

This is capture-route plumbing only. It does not claim original-runtime semantic parity.

## Commands and results

| Command | rc | Result |
| --- | ---: | --- |
| `bash -n scripts/dosbox_dm1_original_viewport_reference_capture.sh` | 0 | shell syntax OK |
| `python3 -m py_compile tools/pass80_original_frame_classifier.py tools/pass84_original_overlay_readiness_probe.py` | 0 | probe/classifier syntax OK |
| canonical labeled route `--dry-run` with `shot:party_hud`, `shot:spell_panel`, `shot:inventory_panel` | 0 | route shape OK: 21 tokens, 6 shots, 3 labeled |
| canonical labeled route under `DOSBOX=/usr/bin/dosbox xvfb-run -a ... --run` | 0 | six 320x200 raw PNGs captured and normalized to six 224x136 crops |
| `python3 tools/pass80_original_frame_classifier.py $RUN_DIR/pass70-xvfb-run3 --fail-on-duplicates` | 1 | negative semantic evidence: 4 `entrance_menu`, 2 `graphics_320x200_unclassified`, duplicate raw frames |
| pass94 entrance-click diagnostic under `DOSBOX=/usr/bin/dosbox xvfb-run -a ... --run` | 0 | six 320x200 raw PNGs captured and normalized |
| `python3 tools/pass80_original_frame_classifier.py $RUN_DIR/pass94-xvfb-diagnostic --expected pass94-diagnostic --fail-on-duplicates` | 1 | negative semantic evidence: 5 `entrance_menu`, 1 `title_or_menu`; click route did not reach dungeon gameplay |
| `python3 tools/pass84_original_overlay_readiness_probe.py` | 0 | `ready_for_overlay_comparison=false`, 5 blockers remain |
| `./run_firestaff_m11_ingame_capture_smoke.sh` | 0 | `In-game capture smoke PASS: 6 screenshots` |

Full stdout/stderr logs are in the run-dir.

## Findings

The N2/Linux route execution blocker is reduced: the original capture script can now run under `xvfb-run` on N2, inject keyboard/mouse input with `xdotool`, capture six DOSBox 0.74 raw 320x200 screenshots, and normalize them into the existing pass70 artifact shape.

The semantic blocker remains. Both the canonical labeled route and the pass94 entrance-click diagnostic produce raw screenshots, but the classifier says they remain menu/selector/title states rather than verified dungeon gameplay. The pass94 diagnostic specifically shows the `click:260,50` entrance action is not sufficient under this N2/DOSBox path.

Pass84 therefore correctly remains false. HUD/viewport parity work can use this as negative route evidence, not as original pixel-parity evidence.

## Minimum next fix

Find the actual original PC 3.4 entrance/menu action sequence on N2 that transitions from the entrance menu into dungeon gameplay, then rerun the same `xvfb-run` route. Only promote/copy artifacts into the default pass70 location after `pass80_original_frame_classifier.py` reports the expected gameplay/route classes without duplicate-frame failures.
