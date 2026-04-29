# Pass 139 — PM F1 inventory/control route

- run base: `<N2_RUNS>/20260428-191522-pass139-pm-f1-inventory-control-route`
- base route: Return → F1 (pass137 inventory hit)
- scenarios: 5
- completed: 5
- errors: 0
- interesting rows: 24

## Interesting rows

- `f1_inventory_keys`: key `F1` -> `5e8d56c494d8` `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class — `image0004-key_F1_4.png`
- `f1_inventory_keys`: key `4` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0009-key_4_9.png`
- `f1_inventory_keys`: key `Return` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-key_Return_10.png`
- `f1_inventory_keys`: key `Space` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0011-key_Space_11.png`
- `f1_inventory_keys`: key `i` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0012-key_i_12.png`
- `f1_inventory_keys`: key `I` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0013-key_I_13.png`
- `f1_inventory_fkeys`: key `F1` -> `7c65aa72a52a` `inventory` — dense low-color inventory extent over viewport — `image0004-key_F1_4.png`
- `f1_inventory_fkeys`: key `F1` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0009-key_F1_9.png`
- `f1_inventory_fkeys`: key `Return` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-key_Return_10.png`
- `f1_inventory_clicks_champ_top`: key `F1` -> `96d587fa7318` `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class — `image0004-key_F1_4.png`
- `f1_inventory_clicks_champ_top`: wait `wait_5` -> `705b1c45d0de` `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class — `image0005-wait_5.png`
- `f1_inventory_clicks_champ_top`: click `click_190_8_9` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0009-click_190_8_9.png`
- `f1_inventory_clicks_champ_top`: click `click_244_9_10` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-click_244_9_10.png`
- `f1_inventory_clicks_champ_top`: click `click_285_9_11` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0011-click_285_9_11.png`
- `f1_inventory_clicks_panel`: key `F1` -> `92c127e9f422` `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class — `image0004-key_F1_4.png`
- `f1_inventory_clicks_panel`: wait `wait_5` -> `705b1c45d0de` `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class — `image0005-wait_5.png`
- `f1_inventory_clicks_panel`: click `click_246_93_9` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0009-click_246_93_9.png`
- `f1_inventory_clicks_panel`: click `click_275_122_10` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-click_275_122_10.png`
- `f1_inventory_clicks_panel`: click `click_276_158_11` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0011-click_276_158_11.png`
- `f1_inventory_clicks_panel`: click `click_300_180_12` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0012-click_300_180_12.png`
- `f1_inventory_viewport`: key `F1` -> `92c127e9f422` `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class — `image0004-key_F1_4.png`
- `f1_inventory_viewport`: wait `wait_5` -> `705b1c45d0de` `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class — `image0005-wait_5.png`
- `f1_inventory_viewport`: click `click_235_52_9` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0009-click_235_52_9.png`
- `f1_inventory_viewport`: click `click_276_158_10` -> `48ed3743ab6a` `dungeon_gameplay` — viewport content with mostly dark in-game right column — `image0010-click_276_158_10.png`

## Scenario summary

- `f1_inventory_keys`: classes=entrance_menu,entrance_menu,entrance_menu,graphics_320x200_unclassified,title_or_menu,entrance_menu,entrance_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay hashes=014ed52c71a0,17bd7e878157,45f3fb628588,48ed3743ab6a,5e8d56c494d8,a9da0dd6b19e,ceb0c2eec633
- `f1_inventory_fkeys`: classes=entrance_menu,entrance_menu,entrance_menu,inventory,title_or_menu,entrance_menu,entrance_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay hashes=014ed52c71a0,1398df5ceb85,17bd7e878157,48ed3743ab6a,7c65aa72a52a,ceb0c2eec633,f8b58f95769c
- `f1_inventory_clicks_champ_top`: classes=entrance_menu,entrance_menu,entrance_menu,graphics_320x200_unclassified,graphics_320x200_unclassified,entrance_menu,entrance_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay hashes=014ed52c71a0,17bd7e878157,48ed3743ab6a,705b1c45d0de,96d587fa7318,ceb0c2eec633,f938fdba63a1
- `f1_inventory_clicks_panel`: classes=entrance_menu,entrance_menu,entrance_menu,graphics_320x200_unclassified,graphics_320x200_unclassified,entrance_menu,entrance_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,dungeon_gameplay hashes=014ed52c71a0,17bd7e878157,48ed3743ab6a,705b1c45d0de,92c127e9f422,ceb0c2eec633,f938fdba63a1
- `f1_inventory_viewport`: classes=entrance_menu,entrance_menu,entrance_menu,graphics_320x200_unclassified,graphics_320x200_unclassified,entrance_menu,entrance_menu,entrance_menu,dungeon_gameplay,dungeon_gameplay hashes=014ed52c71a0,17bd7e878157,48ed3743ab6a,705b1c45d0de,92c127e9f422,ceb0c2eec633,f3aae170c687
