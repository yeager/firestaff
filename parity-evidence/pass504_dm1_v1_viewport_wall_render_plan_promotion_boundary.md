# Pass504 - DM1 V1 viewport wall render-plan promotion boundary

Status: PASS504_DM1_V1_VIEWPORT_WALL_RENDER_PLAN_PROMOTION_BOUNDARY

## What is locked

- pass502 still defines the exact promotion blocker: same-viewport original and Firestaff runtime capture.
- pass503 source locks are present and green for F0128 draw order, F0115 layering, wall/door blits, wall returns, front-door two-pass occlusion, and F0097 present.
- The render-plan artifact has no unsupported wall rows and every snapshot's render events follow the ReDMCSB F0128 order.
- D4 rows remain F0115 object-stack events, not invented wall bitmap rows.

## Snapshot coverage

- start_south: events=12 d4=['D4R'] walls=['D3L2', 'D3R2', 'D3L', 'D3R', 'D2L2', 'D2R2', 'D2L', 'D1L', 'D1R', 'D0L', 'D0R']
- turn_right_west: events=16 d4=['D4L', 'D4R', 'D4C'] walls=['D3L2', 'D3R2', 'D3L', 'D3R', 'D3C', 'D2L2', 'D2R2', 'D2L', 'D2R', 'D2C', 'D1L', 'D1R', 'D1C']
- forward_west_blocked: events=16 d4=['D4L', 'D4R', 'D4C'] walls=['D3L2', 'D3R2', 'D3L', 'D3R', 'D3C', 'D2L2', 'D2R2', 'D2L', 'D2R', 'D2C', 'D1L', 'D1R', 'D1C']
- turn_left_east: events=11 d4=['D4R', 'D4C'] walls=['D3L2', 'D2L2', 'D2R2', 'D2L', 'D2R', 'D2C', 'D1L', 'D1R', 'D1C']
- forward_south_corridor: events=12 d4=['D4L', 'D4R', 'D4C'] walls=['D3R2', 'D3R', 'D2L2', 'D2R2', 'D2L', 'D2R', 'D1L', 'D0L', 'D0R']

## Promotion blocker

Do not claim pixel parity until one exact original DM1 V1 viewport frame and matching Firestaff frame are captured for the same map/x/y/direction/wall-door state and tied to the F0128-to-F0097 present boundary.

## Non-claims

- no original-vs-Firestaff pixel parity promotion
- no new runtime capture
- no movement-core edits
- no pass435 capture-route edits
