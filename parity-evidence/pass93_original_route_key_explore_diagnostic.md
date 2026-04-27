# Pass 93 — original route key exploration diagnostic

Date: 2026-04-27

Scope: original overlay/capture tooling and classifier only. No V1 HUD/viewport rendering, DM1 V1 route/runtime, or V2 asset changes.

## Finding

The repeated wall-closeup problem is not a renderer parity signal. It is an original-route state problem: the automated route can reach the DM1 entrance and can also end up in a wall-closeup/movement-pad view, but it still does not reach a champion-party state where spell and inventory overlays are valid.

Pass 93 explored keyboard-only transitions after `DM -vv -sn -pk`:

```sh
OUT_DIR=$PWD/verification-screens/pass93-subagent-key-explore \
DM1_ORIGINAL_STAGE_DIR=<repo-root>/verification-screens/dm1-dosbox-capture/DungeonMasterPC34 \
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:title enter wait:800 shot:after_enter space wait:800 shot:after_space f1 wait:800 shot:after_f1 f2 wait:800 shot:after_f2 esc wait:800 shot:after_esc' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

## Classifier improvement

`tools/pass80_original_frame_classifier.py` now recognizes `entrance_menu`: a visible dungeon/entrance viewport with ENTER/RESUME/QUIT controls still occupying the right column. This avoids collapsing the pre-start entrance menu into generic unclassified graphics or gameplay.

Self-test now covers 4 cases: wall closeup, inventory, entrance menu, spell panel.

## Raw frame hashes/classes

### Before classifier refinement on the same pass93 raw frames

From `parity-evidence/overlays/pass93/pass93_key_explore_classifier.json`:

| # | sha256 | class |
|---|--------|-------|
| 1 | `c79222a6c543544ba17a8e419e56a48b7966066ef5a3a972c377b992414c7250` | `graphics_320x200_unclassified` |
| 2 | `b6398313372c956dc031d353e4f49ebfe8cec66b6251332d649b7d0f96aa8f05` | `graphics_320x200_unclassified` |
| 3 | `12d500d7b8d6200200358506152eb6c759e0869854c0e0253f132aea2ce13d42` | `graphics_320x200_unclassified` |
| 4 | `5ae7199a3df064c9ebc934c15e87711d4fdacbafea6fd699458c934137b07d9a` | `graphics_320x200_unclassified` |
| 5 | `5ae7199a3df064c9ebc934c15e87711d4fdacbafea6fd699458c934137b07d9a` | `graphics_320x200_unclassified` |
| 6 | `dcfdff6845bfc053ed38c5662a8d81c0a4901abf6b3be09c05676776fb8c4709` | `wall_closeup` |

### After classifier refinement on the same pass93 raw frames

From `parity-evidence/overlays/pass93/pass93_key_explore_classifier_after_entrance_menu.json`:

| # | sha256 | class |
|---|--------|-------|
| 1 | `c79222a6c543544ba17a8e419e56a48b7966066ef5a3a972c377b992414c7250` | `graphics_320x200_unclassified` |
| 2 | `b6398313372c956dc031d353e4f49ebfe8cec66b6251332d649b7d0f96aa8f05` | `graphics_320x200_unclassified` |
| 3 | `12d500d7b8d6200200358506152eb6c759e0869854c0e0253f132aea2ce13d42` | `entrance_menu` |
| 4 | `5ae7199a3df064c9ebc934c15e87711d4fdacbafea6fd699458c934137b07d9a` | `entrance_menu` |
| 5 | `5ae7199a3df064c9ebc934c15e87711d4fdacbafea6fd699458c934137b07d9a` | `entrance_menu` |
| 6 | `dcfdff6845bfc053ed38c5662a8d81c0a4901abf6b3be09c05676776fb8c4709` | `wall_closeup` |

Earlier blocked route evidence from pass80/pass89 still contains the repeated close-wall hash `dcfdff6845bfc053ed38c5662a8d81c0a4901abf6b3be09c05676776fb8c4709`; the refined classifier keeps that as unsafe `wall_closeup`, not inventory/spell evidence.

## Conclusion

Not evidence-valid yet. The route needs a validated champion-party original state (save/resume disk or source-backed champion acquisition path) before spell/inventory overlay comparisons are honest. This pass improves the classifier and narrows the blocker, but does not unblock original-vs-Firestaff parity evidence.
