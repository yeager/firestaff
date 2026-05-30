# Pass561 DM1 V1 far door-front source lock

Status: passed

Claim: D3L2 and mirrored D3R2 front-door branches use ReDMCSB's two-pass far door-front order: rear F0115 pass, far F0111 door, then front F0115 pass.

## Primary ReDMCSB Evidence

- PASS d3l2-far-door-front-split (DUNVIEW.C:6269-6286)
  - line 6269: case C17_ELEMENT_DOOR_FRONT:
  - line 6271: C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT
  - line 6272: C3700_ZONE_DOOR_D3L2
  - line 6273: C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT
  - line 6274: goto T0676017;
  - line 6286: C14_VIEW_SQUARE_D3L2, L2483_i_Order

- PASS d3r2-mirrored-far-door-front-split (DUNVIEW.C:6336-6353)
  - line 6336: case C17_ELEMENT_DOOR_FRONT:
  - line 6338: C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT
  - line 6339: C3710_ZONE_DOOR_D3R2
  - line 6340: C0x0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT
  - line 6341: goto T0677018;
  - line 6353: C15_VIEW_SQUARE_D3R2, L2485_i_Order

## Firestaff Evidence

- PASS firestaff-far-door-front-metadata (dm1_v1_viewport_3d_pc34_compat.c:231-240)
  - line 233: DM1_VIEW_SQUARE_D3L2, 0x0218, 0x0349
  - line 233: DUNVIEW.C:6270 floor ornament under far rear pass
  - line 234: DM1_VIEW_SQUARE_D3R2, 0x0128, 0x0439
  - line 234: DUNVIEW.C:6337 floor ornament under mirrored far rear pass

- PASS firestaff-far-door-front-runtime-test (test_dm1_v1_viewport_3d_pc34_compat.c:707-770)
  - line 722: DM1_VIEW_SQUARE_D3L2, "6270"
  - line 723: DM1_VIEW_SQUARE_D3R2, "6337"
  - line 735: door_front_occlusion_spec_count(), 11

- PASS firestaff-source-evidence-string (dm1_v1_viewport_3d_pc34_compat.c:2110-2118)
  - line 2112: DUNVIEW.C:6270-6286 D3L2 far door-front occlusion
  - line 2113: DUNVIEW.C:6337-6353 D3R2 mirrored far door-front occlusion

## Verification

- /Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS source_evidence.defs_zones == 1
PASS source_evidence.wall_source_clip_gate == 1
PASS source_evidence.wall_empty_blit_gate == 1
PASS source_evidence.occlusion == 1
PASS source_evidence.command_dispatch == 1
PASS source_evidence.next_redraw == 1
PASS source_evidence.present_wait == 1
PASS source_evidence.same_viewport_mouse == 1
PASS source_evidence.same_viewport_turn == 1
PASS source_evidence.same_viewport_move == 1
PASS source_evidence.same_viewport_draw == 1
PASS source_evidence.same_viewport_present == 1
PASS source_evidence.same_viewport_assets == 1
PASS dm1_v1_viewport_3d_source_lock
~~~

- /opt/homebrew/opt/python@3.14/bin/python3.14 /Users/bosse/.openclaw/workspace-main/tools/verify_pass561_dm1_v1_far_door_front_source_lock.py --check-only: rc=0
~~~
PASS pass561 check-only
~~~
