# pass455_dm1_v1_hall_corrected_click_primitive_capture

- status: `BLOCKED_PASS455_CORRECTED_COORDINATES_LOGGED_MOUSE_PRIMITIVE_NOT_PROVEN`
- artifact root: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509`
- external manifest: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509/pass455_dm1_v1_hall_corrected_click_primitive_capture.json`
- parity claim: **not made**; Hall candidate framebuffer labels remain blocked.

## Evidence summary

- corrected-coordinate runs: `probe-initial-south-corrected, probe-turn-click-primitive-stable, probe-turn-click-grid, probe-turn-click-grid-pm, probe-turn-click-grid-globalmouse, probe-turn-click-grid-cliclick`
- click primitive proven: `False`
- candidate transition promoted: `False`
- PC34 data provenance remained hash-locked; no filename-only comparison was used.

## Blocker

Corrected coordinate logging now separates client-relative and absolute/root coordinates, but the local macOS reruns did not prove a mouse click primitive by movement/turn frame transition; do not promote Hall candidate frames from this artifact.

## Blocked labels

`candidate_select`, `panel_visible`, `cancel`, `resurrect_confirm`, `reincarnate_confirm`, `hud_status_after`
