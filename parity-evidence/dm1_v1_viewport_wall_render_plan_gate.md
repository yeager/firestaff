# DM1 V1 viewport wall render-plan comparator gate

Status: `PASS_RENDER_PLAN_COMPARATOR_SEAM`

This fixes the immediate metadata-only seam: source-locked wall/occlusion metadata now produces deterministic render events and comparator-ready expected wall pixel-region descriptors from the pass127 viewport snapshots. It does not claim original pixel parity.

## Evidence

- JSON artifact: `parity-evidence/verification/dm1_v1_viewport_wall_render_plan_gate.json`
- Snapshots compared: ['start_south', 'turn_right_west', 'forward_west_blocked', 'turn_left_east', 'forward_south_corridor']
- Unsupported visible wall rows still blocking full pixel promotion: `[]`
- Region descriptors: viewport rect, source clip, layout 696 zone, selected wall bitmap, and wall-set graphic index for every D3/D2/D1/D0 wall event

## Boundary

D4 rows are emitted as F0115 object-stack events, not missing wall specs: the audited DUNVIEW.C:8466-8477 window has only F0150 coordinate updates and F0115 calls for D4L/D4R/D4C. No unsupported visible wall rows remain in the pass127 comparator snapshots. PC34 D3/D2/D1/D0 side/front wall rows now emit selected native/parity wall bitmap, flip flag, zone, occlusion metadata, and comparator-ready expected pixel-region descriptors (viewport rect/source clip/layout 696/wall-set graphic index) in F0128 order. Remaining promotion blockers are exact original viewport pixel capture and decoded GRAPHICS.DAT bitmap/palette bytes; this gate still does not claim pixel parity.
