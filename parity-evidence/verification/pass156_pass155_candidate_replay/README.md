# Pass 156 — pass155 candidate replay/crop verification

## Result

- Evidence root: `parity-evidence/verification/pass156_pass155_candidate_replay`
- Pass155 exact source base: `<N2_RUNS>/20260428-205557-pass155-champion-route-seed-finder`
- Replay run base: `<N2_RUNS>/20260428-222128-pass156-pass155-candidate-replay`
- Candidates classified: 26
- Replay errors: 0
- Exact hash replay matches: 5/26 (title/logo animation timing drifts; exact pass155 frames were copied beside replay frames for classification).
- Buckets: blocker/unknown=7, dungeon gameplay=4, menu/title variant=15

## Classification table

| # | bucket | pass155 hash | replay hash | program/scenario/action | evidence | note |
|---:|---|---|---|---|---|---|
| 1 | menu/title variant | `06ed65cfd731` | `19d5ec7c9989` DRIFT | `DM -vv -pm` / `pre_dungeon_click_grid` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__pm__pre_dungeon_click_grid__wait_5__06ed65cfd731` | Dungeon Master logo/title frame |
| 2 | menu/title variant | `1db7c4f32832` | `850609db442a` DRIFT | `DM -vv -sn -pm` / `champion_name_keys` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm__champion_name_keys__wait_5__1db7c4f32832` | Dungeon Master logo/title frame |
| 3 | blocker/unknown | `3ad66a894ed8` | `307323fbc1f7` DRIFT | `DM -vv -sn -pm` / `champion_name_keys` / `key C` `key_C_6` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm__champion_name_keys__key_C_6__3ad66a894ed8` | black frame |
| 4 | blocker/unknown | `2946940fa8ac` | `307323fbc1f7` DRIFT | `DM -vv -pm` / `pm_enter_f1_then_function_keys` / `key F2` `key_F2_6` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__pm__pm_enter_f1_then_function_keys__key_F2_6__2946940fa8ac` | black frame |
| 5 | menu/title variant | `3776a62aee87` | `83e49b0317b5` DRIFT | `DM -vv -pm` / `pm_enter_f1_then_function_keys` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__pm__pm_enter_f1_then_function_keys__wait_5__3776a62aee87` | Dungeon Master logo/title frame |
| 6 | blocker/unknown | `2a5497129ae0` | `96d587fa7318` DRIFT | `DM -vv -pm` / `champion_name_keys` / `key C` `key_C_6` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__pm__champion_name_keys__key_C_6__2a5497129ae0` | black frame |
| 7 | menu/title variant | `6bd5bd9ddfa1` | `c717a33c73ce` DRIFT | `DM -vv -pm` / `champion_name_keys` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__pm__champion_name_keys__wait_5__6bd5bd9ddfa1` | Dungeon Master logo/title frame |
| 8 | blocker/unknown | `3ad66a894ed8` | `3ad66a894ed8` MATCH | `DM -vv -sn -pm` / `menu_numeric_roster` / `key 2` `key_2_6` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm__menu_numeric_roster__key_2_6__3ad66a894ed8` | black frame |
| 9 | menu/title variant | `b1c934a63cb7` | `693527d66d55` DRIFT | `DM -vv -sn -pm` / `menu_numeric_roster` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm__menu_numeric_roster__wait_5__b1c934a63cb7` | Dungeon Master logo/title frame |
| 10 | blocker/unknown | `3ad66a894ed8` | `307323fbc1f7` DRIFT | `DM -vv -sn` / `pm_enter_f1_then_function_keys` / `key F2` `key_F2_6` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm_enter_f1_then_function_keys__key_F2_6__3ad66a894ed8` | black frame |
| 11 | menu/title variant | `90ab0553d83c` | `f86af05e2716` DRIFT | `DM -vv -sn` / `pm_enter_f1_then_function_keys` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm_enter_f1_then_function_keys__wait_5__90ab0553d83c` | Dungeon Master logo/title frame |
| 12 | blocker/unknown | `3ad66a894ed8` | `307323fbc1f7` DRIFT | `DM -vv -sn` / `champion_name_keys` / `key C` `key_C_6` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__champion_name_keys__key_C_6__3ad66a894ed8` | black frame |
| 13 | menu/title variant | `67dce89e1de2` | `99ec0762b84b` DRIFT | `DM -vv -sn` / `champion_name_keys` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__champion_name_keys__wait_5__67dce89e1de2` | Dungeon Master logo/title frame |
| 14 | blocker/unknown | `3ad66a894ed8` | `3ad66a894ed8` MATCH | `DM -vv -pm` / `menu_numeric_roster` / `key 2` `key_2_6` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__pm__menu_numeric_roster__key_2_6__3ad66a894ed8` | black frame |
| 15 | menu/title variant | `ee217343dcff` | `19d5ec7c9989` DRIFT | `DM -vv -pm` / `menu_numeric_roster` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__pm__menu_numeric_roster__wait_5__ee217343dcff` | Dungeon Master logo/title frame |
| 16 | menu/title variant | `489a3eeda949` | `31a9f7f8518f` DRIFT | `DM -vv -sn -pm` / `pre_dungeon_click_grid` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm__pre_dungeon_click_grid__wait_5__489a3eeda949` | dim/fade title-logo variant |
| 17 | menu/title variant | `6f7d7f18c50b` | `9449be421880` DRIFT | `DM -vv -sn -pm` / `pm_enter_f2_function_keys` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm__pm_enter_f2_function_keys__wait_5__6f7d7f18c50b` | dim/fade title-logo variant |
| 18 | menu/title variant | `7d0d3d21cc3a` | `25f899bd0955` DRIFT | `DM -vv -sn` / `menu_numeric_roster` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__menu_numeric_roster__wait_5__7d0d3d21cc3a` | dim/fade title-logo variant |
| 19 | menu/title variant | `850609db442a` | `1caa0a2c979a` DRIFT | `DM -vv -sn` / `pm_enter_f2_function_keys` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm_enter_f2_function_keys__wait_5__850609db442a` | dim/fade title-logo variant |
| 20 | menu/title variant | `9b9627eee38d` | `9cef49ededfc` DRIFT | `DM -vv -pm` / `pm_enter_f2_function_keys` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__pm__pm_enter_f2_function_keys__wait_5__9b9627eee38d` | dim/fade title-logo variant |
| 21 | menu/title variant | `d90583ca0e04` | `a6a9507ce1a6` DRIFT | `DM -vv -sn -pm` / `pm_enter_f1_then_function_keys` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pm__pm_enter_f1_then_function_keys__wait_5__d90583ca0e04` | dim/fade title-logo variant |
| 22 | menu/title variant | `ea2ea6bace0d` | `1a4a59b2f07a` DRIFT | `DM -vv -sn` / `pre_dungeon_click_grid` / `wait` `wait_5` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pre_dungeon_click_grid__wait_5__ea2ea6bace0d` | dim/fade title-logo variant |
| 23 | dungeon gameplay | `082b4d249740` | `18d392342edb` DRIFT | `DM -vv -sn` / `pre_dungeon_click_grid` / `wait` `wait_13` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pre_dungeon_click_grid__wait_13__082b4d249740` | clear in-dungeon corridor/control view; target pass155 seed 082b4d249740 |
| 24 | dungeon gameplay | `17bd7e878157` | `17bd7e878157` MATCH | `DM -vv -sn` / `pre_dungeon_click_grid` / `click (220,35)` `click_220_35_10` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pre_dungeon_click_grid__click_220_35_10__17bd7e878157` | entrance/dungeon view with right-side entry menu overlay |
| 25 | dungeon gameplay | `17bd7e878157` | `17bd7e878157` MATCH | `DM -vv -sn` / `pre_dungeon_click_grid` / `wait` `wait_11` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pre_dungeon_click_grid__wait_11__17bd7e878157` | same entrance/dungeon overlay as #24 |
| 26 | dungeon gameplay | `17bd7e878157` | `17bd7e878157` MATCH | `DM -vv -sn` / `pre_dungeon_click_grid` / `click (280,35)` `click_280_35_12` | `parity-evidence/verification/pass156_pass155_candidate_replay/dm__vv__sn__pre_dungeon_click_grid__click_280_35_12__17bd7e878157` | same entrance/dungeon overlay as #24 |

## Evidence files per candidate

Each candidate directory contains:
- `pass155_frame.png` — exact pass155 frame for the listed pass155 hash
- `frame.png` — fresh replay capture for the same scenario/action
- `pass155_annotated_bbox.png` and `annotated_bbox.png` — viewport/right-panel/lower-panel/top-strip boxes
- `pass155_viewport.png`, `pass155_right_panel.png`, `pass155_lower_panel.png`, `pass155_top_strip.png` plus replay crop equivalents
- `source.txt` — replay source path

## Interpretation

- No replayed/copied candidate shows champion/control progress: labels that looked promising (`champion_name_keys`, roster/function keys) were title-logo variants or black blockers in the exact pass155 frame.
- The focused `DM -vv -sn` / `pre_dungeon_click_grid` lane does reach dungeon visuals: pass155 hash `082b4d249740` is clear dungeon gameplay, and `17bd7e878157` is the entrance/dungeon overlay with right-side entry menu.
- The score-40 `graphics_320x200_unclassified` rows are mostly title-logo animation/fade frames; some are black blockers. They should not be treated as champion/control progress seeds.

## Gate

```
xvfb-run -a python3 tools/pass156_verify_pass155_candidates.py
wrote parity-evidence/verification/pass156_pass155_candidate_replay/README.md
run_base=<N2_RUNS>/20260428-222128-pass156-pass155-candidate-replay
captured=26 errors=0
```

## Next unblock recommendation

Promote only the `DM -vv -sn` / `pre_dungeon_click_grid` route around `wait_13` (`082b4d249740`) as the current dungeon-entry seed. Ignore the top unclassified title/black hashes for champion/control readiness; next pass should continue from the `082b` dungeon state and probe deterministic movement/control actions, not broaden menu/title exploration.
