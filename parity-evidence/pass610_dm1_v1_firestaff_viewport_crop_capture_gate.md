# Pass610 - DM1 V1 Firestaff viewport crop capture gate

Status: PASS610_DM1_V1_FIRESTAFF_VIEWPORT_CROP_CAPTURE_LOCKED

This gate locks the Firestaff-side 224x136 viewport crop artifacts needed before any later same-viewport original/Firestaff comparison can be promoted.

Source evidence:
- COORD.C:1693-1722 ok=True - PC34 dungeon viewport crops use x=0, y=33, width=224, height=136.
- DUNVIEW.C:2962-3003,3048-3078,8318-8610 ok=True - Walls compose into G0296 before F0128 presents the dungeon view.
- DRAWVIEW.C:842-857 ok=True - F0097 presents G0296 through C007_ZONE_VIEWPORT, so the crop is the compare boundary.

Runtime crops:
- 01_start_south_1_3 map=0 x=1 y=3 dir=2 crop=01_start_south_1_3_viewport_224x136.ppm sha256=c4a2fd67d88aa5b7872da33af2e969670fd2082d1b8c3547d6295f583daae2c2
- 02_turn_right_west_1_3 map=0 x=1 y=3 dir=3 crop=02_turn_right_west_1_3_viewport_224x136.ppm sha256=48ebb87dbc1ce98b6ea4e6156ed5aa2417a1457f801851e9c73676d02897cea5
- 03_blocked_west_wall_1_3 map=0 x=1 y=3 dir=3 crop=03_blocked_west_wall_1_3_viewport_224x136.ppm sha256=48ebb87dbc1ce98b6ea4e6156ed5aa2417a1457f801851e9c73676d02897cea5
- 04_forward_south_1_4 map=0 x=1 y=4 dir=2 crop=04_forward_south_1_4_viewport_224x136.ppm sha256=99f4f2903fcdb28c5375c3fbc2aaf4352aede055f194ecbde95192d367d5dd8e

Non-claims:
- no original PC34 frame was captured
- no original-vs-Firestaff pixel parity is promoted
- the crop hashes are Firestaff capture-readiness evidence only
- no TODO.md update
