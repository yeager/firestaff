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

- PASS firestaff-far-door-front-metadata (dm1_v1_viewport_3d_pc34_compat.c:949-950)
  - line 949: DM1_VIEW_SQUARE_D3L2, 0x0218, 0x0349
  - line 949: DUNVIEW.C:6270 floor ornament under far rear pass
  - line 950: DM1_VIEW_SQUARE_D3R2, 0x0128, 0x0439
  - line 950: DUNVIEW.C:6337 floor ornament under mirrored far rear pass

- PASS firestaff-far-door-front-runtime-test (test_dm1_v1_viewport_3d_pc34_compat.c:1304-1317)
  - line 1304: DM1_VIEW_SQUARE_D3L2, "6270"
  - line 1305: DM1_VIEW_SQUARE_D3R2, "6337"
  - line 1317: door_front_occlusion_spec_count(), 11

- PASS firestaff-source-evidence-string (dm1_v1_viewport_3d_pc34_compat.c:4197-4198)
  - line 4197: DUNVIEW.C:6270-6286 D3L2 far door-front occlusion
  - line 4198: DUNVIEW.C:6337-6353 D3R2 mirrored far door-front occlusion

## Verification

- /Volumes/Extern-disk/firestaff-claude/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS drift.pass576.test_wall_source_row_clip present in tests/test_dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass577.d1l_visible_square present in src/dm1/dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass577.d0c_visible_square present in src/dm1/dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass577.d1c_projectile present in src/dm1/dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass577.runtime_test present in tests/test_dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass510.party_tuple_source_citation present in src/engine/m11_game_view.c
PASS drift.pass510.party_tuple_flip_predicate present in src/engine/m11_game_view.c
PASS drift.pass510.wallset_variant_binding present in src/engine/m11_game_view.c
PASS drift.pass510.center_wall_flip_path present in src/engine/m11_game_view.c
PASS drift.pass510.side_wall_lr_swap_path present in src/engine/m11_game_view.c
PASS drift.pass643.d3l2_d3r2_f0111_runtime_consumer present in src/engine/m11_game_view.c
PASS drift.pass643.d3l2_d3r2_f0111_redmcsb_anchors present in src/engine/m11_game_view.c
PASS drift.pass643.d3l2_d3r2_material_plan_consumed present in src/engine/m11_game_view.c
PASS dm1_v1_viewport_3d_source_lock
~~~

- /Applications/Xcode.app/Contents/Developer/usr/bin/python3 /Volumes/Extern-disk/firestaff-claude/tools/verify_pass561_dm1_v1_far_door_front_source_lock.py --check-only: rc=0
~~~
PASS pass561 check-only
~~~
