# pass760_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_wall_ornament_source_lock

Status: `PASS`

## Scope

- DM1 V1 D0L2/D0R2 combined wall + floor + ceiling + ornament source-lock metadata.
- Non-duplicative with pass729 D0L/D0R F0108, pass733 D0C F0108, and pass717 D1L2/D1R2 wall composition gates.
- Contract-only metadata gate; no original-vs-Firestaff pixel parity and no game-data load.

## Required ReDMCSB Anchors

- DUNVIEW.C F0108_DUNGEONVIEW_DrawFloorOrnament:3940-4011
- DUNVIEW.C F0107:3502-3938
- DUNVIEW.C F0098:2962-3002
- DUNVIEW.C F0115:4547-4581,5180-5188,5211-5214,5668-5671
- DUNVIEW.C:6432-6600 M575_VIEW_WALL_D3L_RIGHT
- DEFS.H:2088 C10_COLOR_FLESH
- DEFS.H:2596-2611 I34E/P31J view-square ordinals
- DEFS.H:2668-2677 and 2698-2702 cell orders and M575..M579 view-square ordinals
- DEFS.H:4045-4046 C705/C706 wall zones
- DRAWVIEW.C F0097:1-50 wall-side dispatch
- DUNVIEW.C F0104:3113-3156 native C10 blit
- DUNVIEW.C F0105:3185-3247 flipped C10 blit

## Verification

- C test: `/Users/bosse/.openclaw/workspace-main-20260612142622-d0l2-d0r2-f0108-wall-floor-ornament/build/test_dm1_v1_viewport_3d_pc34_compat`
- pass760 assertions: 100 pass / 0 fail
- Local checks: 14/14
- Source checks: 15/16

## Drift TODOs

- Required anchor DRAWVIEW.C F0097:1-50 appears drifted in local ReDMCSB; F0097_DUNGEONVIEW_DrawViewport is at line 709. Anchor text was not changed.
