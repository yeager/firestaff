# Pass 145 — follow frame48ed next route

- source atlas: `/home/trv2/.openclaw/data/firestaff-worktrees/pass145b-follow-frame48ed-next-route-1777405537/parity-evidence/pass143_frame48ed_control_delta_atlas.json`
- focus rows: 17
- 48ed static/near-static rows: 4
- 48ed non-static rows: 5

## Candidate follow-ups

Non-static 48ed rows exist; next dynamic route target should replay these exact scenarios with longer waits and movement/action sequences, capturing before/after bounding boxes.
- ratio=0.461969 `f1_key4_movement` `key Right` file `f1_key4_movement/image0010-key_Right_10.png` bbox `[0, 0, 320, 200]`
- ratio=0.461969 `f1_return_movement` `key Right` file `f1_return_movement/image0010-key_Right_10.png` bbox `[0, 0, 320, 200]`
- ratio=0.461969 `f1_space_movement` `key Down` file `f1_space_movement/image0010-key_Down_10.png` bbox `[0, 0, 320, 200]`
- ratio=0.461969 `f1_top_clicks_then_move` `key Right` file `f1_top_clicks_then_move/image0010-key_Right_10.png` bbox `[0, 0, 320, 200]`
- ratio=0.018766 `f1_panel_clicks_then_move` `key Right` file `f1_panel_clicks_then_move/image0010-key_Right_10.png` bbox `[233, 53, 320, 169]`

## Specific next pass recommendation

- If non-static rows exist: pass146 should replay the top non-static scenarios with longer waits and more movement/action inputs.
- If all static: pass146 should pivot to source/asset-backed champion-selection/Hall-of-Champions route instead of direct dungeon movement.
