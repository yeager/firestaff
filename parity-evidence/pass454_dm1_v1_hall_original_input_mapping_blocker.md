# pass454_dm1_v1_hall_original_input_mapping_blocker

- status: `BLOCKED_CAPTURE_AUTOMATION_ABSOLUTE_COORDINATES_USED_WITH_WINDOW_RELATIVE_CLICK`
- artifact manifest: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-true-stop-20260509/manifest.json`
- artifact manifest sha256: `33b54c69685392e80b3450ffdecf618b6ad0d176edde68a0a68ec1332c33fc12`
- parity claim: **not made**; all Hall candidate labels remain blocked.

## Diagnosis

Fresh-entry attempts did not prove the original state machine because logged clicks do not land at the computed client-relative PC coordinate; at least one logged click is outside the DOSBox client window. That is an automation coordinate-space bug, not evidence against mouse release/down duration, UI-ready tick, empty hand, modal state, or PC34 data identity.

## Evidence summary

- fresh attempts with static image sequences: `probe-pk-fresh-click, probe-pm-fresh-click, probe-known-entry`
- mismapped click count: `13`
- PC34 data provenance remained hash-locked; no filename-only comparison was used.

## Next executable action

- `rerun_fresh_initial_south_click_with_correct_window_relative_mapping`
- Do not reuse the stale hall-true-stop click logs as candidate evidence.
- For a DOSBox window, map PC 320x200 coordinates to client-relative coordinates and use xdotool mousemove --window with only those client-relative coordinates.
- Alternatively use absolute/root coordinates without --window, but never add the window origin and also pass --window.
- Log both requested PC coordinate and computed client-relative/absolute coordinate before every click.
- Capture candidate_select only after the click primitive is proven by a movement/turn control click or debugger-visible C080 dispatch.

## Required client-relative checks for the observed 1067x832 DOSBox window
- `window`: `1067x832`
- `pc_111_82`: `[372, 358]`
- `pc_130_115`: `[435, 468]`
- `pc_159_147`: `[532, 574]`

## Blocked labels

`candidate_select`, `panel_visible`, `cancel`, `resurrect_confirm`, `reincarnate_confirm`, and `hud_status_after` remain unpromoted for original PC34 Hall candidate parity.
