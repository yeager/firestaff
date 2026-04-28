# Pass 141 — PM F1 dungeon control readiness

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-192537-pass141-pm-f1-dungeon-control-readiness`
- route base: DM `-pm`, Return → F1, then a pass139-proven dungeon exit, then movement keys
- scenarios: 5
- completed: 5
- errors: 0

## Movement response summary

- `f1_key4_movement`: candidate=True movement_unique_hashes=2 movement_hashes=17bd7e878157,17bd7e878157,48ed3743ab6a,48ed3743ab6a
- `f1_return_movement`: candidate=True movement_unique_hashes=2 movement_hashes=17bd7e878157,17bd7e878157,48ed3743ab6a,48ed3743ab6a
- `f1_space_movement`: candidate=True movement_unique_hashes=2 movement_hashes=17bd7e878157,17bd7e878157,48ed3743ab6a
- `f1_panel_clicks_then_move`: candidate=True movement_unique_hashes=2 movement_hashes=1ee706538fb3,48ed3743ab6a,48ed3743ab6a
- `f1_top_clicks_then_move`: candidate=True movement_unique_hashes=2 movement_hashes=17bd7e878157,48ed3743ab6a,48ed3743ab6a

## Rows

### f1_key4_movement
- initial `initial` -> `ceb0c2eec633` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0001-initial.png`
- key `Return` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0002-key_Return_2.png`
- wait `wait_3` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0003-wait_3.png`
- key `F1` -> `38f411f2ea7e` `inventory` — dense low-color inventory extent over viewport — `image0004-key_F1_4.png`
- wait `wait_5` -> `67893775f5c1` `title_or_menu` — sparse viewport plus colorful/right-column title-menu art — `image0005-wait_5.png`
- key `4` -> `672193a5deba` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0006-key_4_6.png`
- wait `wait_7` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0007-wait_7.png`
- key `Up` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0008-key_Up_8.png`
- key `Left` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0009-key_Left_9.png`
- key `Right` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-key_Right_10.png`
- key `Down` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0011-key_Down_11.png`
### f1_return_movement
- initial `initial` -> `ceb0c2eec633` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0001-initial.png`
- key `Return` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0002-key_Return_2.png`
- wait `wait_3` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0003-wait_3.png`
- key `F1` -> `d15db869b9f6` `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class — `image0004-key_F1_4.png`
- wait `wait_5` -> `7addedda4327` `title_or_menu` — sparse viewport plus colorful/right-column title-menu art — `image0005-wait_5.png`
- key `Return` -> `1eb3c7fe6494` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0006-key_Return_6.png`
- wait `wait_7` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0007-wait_7.png`
- key `Up` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0008-key_Up_8.png`
- key `Up` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0009-key_Up_9.png`
- key `Right` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-key_Right_10.png`
- key `Left` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0011-key_Left_11.png`
### f1_space_movement
- initial `initial` -> `ceb0c2eec633` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0001-initial.png`
- key `Return` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0002-key_Return_2.png`
- wait `wait_3` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0003-wait_3.png`
- key `F1` -> `f2b6ba10f514` `inventory` — dense low-color inventory extent over viewport — `image0004-key_F1_4.png`
- wait `wait_5` -> `7addedda4327` `title_or_menu` — sparse viewport plus colorful/right-column title-menu art — `image0005-wait_5.png`
- key `Space` -> `5257b4c8ee2d` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0006-key_Space_6.png`
- wait `wait_7` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0007-wait_7.png`
- key `Up` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0008-key_Up_8.png`
- key `Right` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0009-key_Right_9.png`
- key `Down` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-key_Down_10.png`
### f1_panel_clicks_then_move
- initial `initial` -> `ceb0c2eec633` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0001-initial.png`
- key `Return` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0002-key_Return_2.png`
- wait `wait_3` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0003-wait_3.png`
- key `F1` -> `f2b6ba10f514` `inventory` — dense low-color inventory extent over viewport — `image0004-key_F1_4.png`
- wait `wait_5` -> `7addedda4327` `title_or_menu` — sparse viewport plus colorful/right-column title-menu art — `image0005-wait_5.png`
- click `click_235_52_6` -> `8bc78f18f48a` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0006-click_235_52_6.png`
- click `click_276_158_7` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0007-click_276_158_7.png`
- wait `wait_8` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0008-wait_8.png`
- key `Up` -> `1ee706538fb3` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0009-key_Up_9.png`
- key `Right` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-key_Right_10.png`
- key `Left` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0011-key_Left_11.png`
### f1_top_clicks_then_move
- initial `initial` -> `ceb0c2eec633` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0001-initial.png`
- key `Return` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0002-key_Return_2.png`
- wait `wait_3` -> `014ed52c71a0` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0003-wait_3.png`
- key `F1` -> `183f110f5d3b` `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class — `image0004-key_F1_4.png`
- wait `wait_5` -> `f8d841d4357c` `title_or_menu` — sparse viewport plus colorful/right-column title-menu art — `image0005-wait_5.png`
- click `click_190_8_6` -> `1eb3c7fe6494` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0006-click_190_8_6.png`
- click `click_244_9_7` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0007-click_244_9_7.png`
- wait `wait_8` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0008-wait_8.png`
- key `Up` -> `17bd7e878157` `entrance_menu` — dungeon entrance/menu controls still occupy the right column — `image0009-key_Up_9.png`
- key `Right` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-key_Right_10.png`
- key `Left` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0011-key_Left_11.png`
