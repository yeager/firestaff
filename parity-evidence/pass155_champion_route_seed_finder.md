# Pass 155 — champion/control route seed finder

- purpose: pivot away from static 48ed and identify non-48ed route seeds for champion/party-control capture.
- run base: `<N2_RUNS>/20260428-205557-pass155-champion-route-seed-finder`
- completed scenarios: 20
- errors: 0

## Top non-48ed seeds

- score=40 `DM -vv -pm` `pre_dungeon_click_grid` wait : 06ed65cfd731 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn -pm` `champion_name_keys` wait : 1db7c4f32832 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -pm` `pm_enter_f1_then_function_keys` key F2: 2946940fa8ac `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -pm` `champion_name_keys` key C: 2a5497129ae0 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -pm` `pm_enter_f1_then_function_keys` wait : 3776a62aee87 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn -pm` `menu_numeric_roster` key 2: 3ad66a894ed8 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn -pm` `champion_name_keys` key C: 3ad66a894ed8 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn` `pm_enter_f1_then_function_keys` key F2: 3ad66a894ed8 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn` `champion_name_keys` key C: 3ad66a894ed8 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -pm` `menu_numeric_roster` key 2: 3ad66a894ed8 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn -pm` `pre_dungeon_click_grid` wait : 489a3eeda949 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn` `champion_name_keys` wait : 67dce89e1de2 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -pm` `champion_name_keys` wait : 6bd5bd9ddfa1 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn -pm` `pm_enter_f2_function_keys` wait : 6f7d7f18c50b `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn` `menu_numeric_roster` wait : 7d0d3d21cc3a `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn` `pm_enter_f2_function_keys` wait : 850609db442a `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn` `pm_enter_f1_then_function_keys` wait : 90ab0553d83c `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -pm` `pm_enter_f2_function_keys` wait : 9b9627eee38d `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn -pm` `menu_numeric_roster` wait : b1c934a63cb7 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn -pm` `pm_enter_f1_then_function_keys` wait : d90583ca0e04 `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -sn` `pre_dungeon_click_grid` wait : ea2ea6bace0d `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=40 `DM -vv -pm` `menu_numeric_roster` wait : ee217343dcff `graphics_320x200_unclassified` — 320x200 graphics frame, but layout heuristics did not match a known class
- score=20 `DM -vv -sn` `pre_dungeon_click_grid` wait : 082b4d249740 `dungeon_gameplay` — viewport content with mostly dark in-game right column
- score=15 `DM -vv -sn -pm` `pre_dungeon_click_grid` key Return: 014ed52c71a0 `entrance_menu` — dungeon entrance/menu controls still occupy the right column
- score=15 `DM -vv -sn -pm` `pre_dungeon_click_grid` wait : 014ed52c71a0 `entrance_menu` — dungeon entrance/menu controls still occupy the right column
- score=15 `DM -vv -sn -pm` `pre_dungeon_click_grid` click (45, 35): 014ed52c71a0 `entrance_menu` — dungeon entrance/menu controls still occupy the right column
- score=15 `DM -vv -pm` `champion_name_keys` wait : 063b7cc5200a `entrance_menu` — dungeon entrance/menu controls still occupy the right column
- score=15 `DM -vv -sn -pm` `pre_dungeon_click_grid` click (220, 35): 17bd7e878157 `entrance_menu` — dungeon entrance/menu controls still occupy the right column
- score=15 `DM -vv -sn -pm` `pre_dungeon_click_grid` wait : 17bd7e878157 `entrance_menu` — dungeon entrance/menu controls still occupy the right column
- score=15 `DM -vv -sn -pm` `pre_dungeon_click_grid` click (280, 35): 17bd7e878157 `entrance_menu` — dungeon entrance/menu controls still occupy the right column

## Scenario summary

- `DM -vv -sn -pm` `pm_enter_f1_then_function_keys`: unique_hashes=4 non48ed=15 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -sn -pm` `pm_enter_f2_function_keys`: unique_hashes=4 non48ed=11 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -sn -pm` `menu_numeric_roster`: unique_hashes=5 non48ed=15 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -sn -pm` `pre_dungeon_click_grid`: unique_hashes=8 non48ed=12 classes=dungeon_gameplay,entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -sn -pm` `champion_name_keys`: unique_hashes=7 non48ed=15 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -sn` `pm_enter_f1_then_function_keys`: unique_hashes=5 non48ed=15 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -sn` `pm_enter_f2_function_keys`: unique_hashes=4 non48ed=11 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -sn` `menu_numeric_roster`: unique_hashes=4 non48ed=15 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -sn` `pre_dungeon_click_grid`: unique_hashes=9 non48ed=13 classes=dungeon_gameplay,entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -sn` `champion_name_keys`: unique_hashes=7 non48ed=15 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -pm` `pm_enter_f1_then_function_keys`: unique_hashes=5 non48ed=15 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -pm` `pm_enter_f2_function_keys`: unique_hashes=4 non48ed=11 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -pm` `menu_numeric_roster`: unique_hashes=5 non48ed=15 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -pm` `pre_dungeon_click_grid`: unique_hashes=9 non48ed=13 classes=dungeon_gameplay,entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM -vv -pm` `champion_name_keys`: unique_hashes=7 non48ed=15 classes=entrance_menu,graphics_320x200_unclassified,title_or_menu
- `DM` `pm_enter_f1_then_function_keys`: unique_hashes=5 non48ed=15 classes=non_graphics_blocker
- `DM` `pm_enter_f2_function_keys`: unique_hashes=5 non48ed=11 classes=non_graphics_blocker
- `DM` `menu_numeric_roster`: unique_hashes=5 non48ed=15 classes=non_graphics_blocker
- `DM` `pre_dungeon_click_grid`: unique_hashes=3 non48ed=17 classes=non_graphics_blocker
- `DM` `champion_name_keys`: unique_hashes=6 non48ed=15 classes=non_graphics_blocker
