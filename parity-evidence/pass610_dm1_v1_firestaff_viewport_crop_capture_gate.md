# Pass610 - DM1 V1 Firestaff viewport crop capture gate

Status: PASS610_DM1_V1_FIRESTAFF_VIEWPORT_CROP_CAPTURE_LOCKED

This gate locks the Firestaff-side 224x136 viewport crop artifacts needed before any later same-viewport original/Firestaff comparison can be promoted.

Source evidence:
- COORD.C:1693-1722 ok=True - PC34 dungeon viewport crops use x=0, y=33, width=224, height=136.
- DUNVIEW.C:2962-3003,3048-3078,8318-8610 ok=True - Walls compose into G0296 before F0128 presents the dungeon view.
- DRAWVIEW.C:842-857 ok=True - F0097 presents G0296 through C007_ZONE_VIEWPORT, so the crop is the compare boundary.

Runtime crops:
- 01_start_south_1_3 map=0 x=1 y=3 dir=2 crop=01_start_south_1_3_viewport_224x136.ppm sha256=dbddd83083612ec7bcaaf3796a93b0b9681b8db2b99f18e83d0aead0335d688e
- 02_turn_right_west_1_3 map=0 x=1 y=3 dir=3 crop=02_turn_right_west_1_3_viewport_224x136.ppm sha256=1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81
- 03_blocked_west_wall_1_3 map=0 x=1 y=3 dir=3 crop=03_blocked_west_wall_1_3_viewport_224x136.ppm sha256=1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81
- 04_forward_south_1_4 map=0 x=1 y=4 dir=2 crop=04_forward_south_1_4_viewport_224x136.ppm sha256=3a72a9707bc48ac407e307f3a061593f362faedf036242093d7caade8a2187e9

Non-claims:
- no original PC34 frame was captured
- no original-vs-Firestaff pixel parity is promoted
- the crop hashes are Firestaff capture-readiness evidence only
- no TODO.md update
