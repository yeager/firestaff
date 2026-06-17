# Pass610 - DM1 V1 Firestaff viewport crop capture gate

Status: FAIL_PASS610_DM1_V1_FIRESTAFF_VIEWPORT_CROP_CAPTURE

This gate locks the Firestaff-side 224x136 viewport crop artifacts needed before any later same-viewport original/Firestaff comparison can be promoted.

Source evidence:
- COORD.C:1693-1722 ok=True - PC34 dungeon viewport crops use x=0, y=33, width=224, height=136.
- DUNVIEW.C:2962-3003,3048-3078,8318-8610 ok=True - Walls compose into G0296 before F0128 presents the dungeon view.
- DRAWVIEW.C:842-857 ok=True - F0097 presents G0296 through C007_ZONE_VIEWPORT, so the crop is the compare boundary.

Runtime crops:

Non-claims:
- no original PC34 frame was captured
- no original-vs-Firestaff pixel parity is promoted
- the crop hashes are Firestaff capture-readiness evidence only
- no TODO.md update

Problems:
- DUNGEON RESOLVE: dataDir=[/Users/bosse/.firestaff/data] gameId=[dm1]
  TRY: [/Users/bosse/.firestaff/data/dm1/DUNGEON.DAT] FOUND
LOADING DUNGEON: [/Users/bosse/.firestaff/data/dm1/DUNGEON.DAT]
wrote /Users/bosse/.openclaw/workspace-main/build/pass610_dm1_v1_firestaff_viewport_crop_capture_gate/dm1_v1_wall_collision_runtime_capture.json and /Users/bosse/.openclaw/workspace-main/build/pass610_dm1_v1_firestaff_viewport_crop_capture_gate/dm1_v1_wall_collision_runtime_capture.md
FAIL dm1_v1_wall_collision_runtime_capture rows=4 out=/Users/bosse/.openclaw/workspace-main/build/pass610_dm1_v1_firestaff_viewport_crop_capture_gate

