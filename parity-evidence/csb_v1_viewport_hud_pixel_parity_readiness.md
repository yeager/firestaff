# CSB V1 viewport/HUD pixel parity readiness

Status: `FAIL`
Broad parity: `READY_TO_PROMOTE`

This gate keeps the CSB viewport/HUD fixture shape ready for a later
paired original-vs-Firestaff hash pass. It deliberately does not run a
capture or promote parity while all future pairing fields are empty.

## Fixture Surface

| Fixture | State | Region | Geometry | Surface |
|---|---|---|---|---|

## Promotion Rule

Promote CSB viewport/HUD pixel parity only after every fixture has paired original and Firestaff artifacts, SHA256 values for both, a diff artifact/hash, and explicit changed_pixels/mae/max_delta metrics.

## Non-claims

- no original CSB frame is captured by this gate
- no Firestaff runtime frame is captured by this gate
- no original-vs-Firestaff pixel parity is promoted
- broader CSB viewport/HUD parity remains OPEN_UNPAIRED until paired evidence lands
- no user-supplied game data is committed

## Problems

- missing source manifest: parity-evidence/verification/passH2248_csb_v1_viewport_pixel_gate.json
- expected at least 6 CSB viewport/HUD fixtures, got 0
- missing required region: viewport_full
- missing required region: viewport_center
- missing required region: status_bar
- missing required region: chrome_bottom
- missing required region: panel_right
- missing required state label: csb_prison_entrance
- missing required state label: csb_prison_forward
- no HUD/chrome regions covered
- no viewport regions covered
- broad parity must remain OPEN_UNPAIRED in this readiness-only gate
