# Pass 122 — DM1 V1 original route solver

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-153524-pass122-dm1-v1-original-route-solver`
- scope: larger consolidated original-route pass for DM1 V1 priority. It combines proven keypad route changes from pass120/121 with panel keys, inventory/champion keys, and click probes after each routed state.

## Route summaries

- `baseline` keys `[]` unique_hashes=2: route:baseline:48ed3743ab6a:dungeon_gameplay -> panel_key:F1:48ed3743ab6a:dungeon_gameplay -> panel_key:F2:48ed3743ab6a:dungeon_gameplay -> panel_key:F3:48ed3743ab6a:dungeon_gameplay -> panel_key:F4:48ed3743ab6a:dungeon_gameplay -> panel_key:1:48ed3743ab6a:dungeon_gameplay -> panel_key:2:48ed3743ab6a:dungeon_gameplay -> panel_key:3:48ed3743ab6a:dungeon_gameplay -> panel_key:4:48ed3743ab6a:dungeon_gameplay -> panel_key:5:48ed3743ab6a:dungeon_gameplay -> panel_key:6:48ed3743ab6a:dungeon_gameplay -> panel_key:i:48ed3743ab6a:dungeon_gameplay ...
- `left` keys `['KP_Left']` unique_hashes=3: route:baseline:48ed3743ab6a:dungeon_gameplay -> route:KP_Left:fbeb1b82cd09:wall_closeup -> panel_key:F1:fbeb1b82cd09:wall_closeup -> panel_key:F2:fbeb1b82cd09:wall_closeup -> panel_key:F3:fbeb1b82cd09:wall_closeup -> panel_key:F4:fbeb1b82cd09:wall_closeup -> panel_key:1:fbeb1b82cd09:wall_closeup -> panel_key:2:fbeb1b82cd09:wall_closeup -> panel_key:3:fbeb1b82cd09:wall_closeup -> panel_key:4:fbeb1b82cd09:wall_closeup -> panel_key:5:fbeb1b82cd09:wall_closeup -> panel_key:6:fbeb1b82cd09:wall_closeup ...
- `right` keys `['KP_Right']` unique_hashes=3: route:baseline:48ed3743ab6a:dungeon_gameplay -> route:KP_Right:fbeb1b82cd09:wall_closeup -> panel_key:F1:fbeb1b82cd09:wall_closeup -> panel_key:F2:fbeb1b82cd09:wall_closeup -> panel_key:F3:fbeb1b82cd09:wall_closeup -> panel_key:F4:fbeb1b82cd09:wall_closeup -> panel_key:1:fbeb1b82cd09:wall_closeup -> panel_key:2:fbeb1b82cd09:wall_closeup -> panel_key:3:fbeb1b82cd09:wall_closeup -> panel_key:4:fbeb1b82cd09:wall_closeup -> panel_key:5:fbeb1b82cd09:wall_closeup -> panel_key:6:fbeb1b82cd09:wall_closeup ...
- `left_left` keys `['KP_Left', 'KP_Left']` unique_hashes=4: route:baseline:48ed3743ab6a:dungeon_gameplay -> route:KP_Left:fbeb1b82cd09:wall_closeup -> route:KP_Left:9fc8530431a3:dungeon_gameplay -> panel_key:F1:9fc8530431a3:dungeon_gameplay -> panel_key:F2:9fc8530431a3:dungeon_gameplay -> panel_key:F3:9fc8530431a3:dungeon_gameplay -> panel_key:F4:9fc8530431a3:dungeon_gameplay -> panel_key:1:9fc8530431a3:dungeon_gameplay -> panel_key:2:9fc8530431a3:dungeon_gameplay -> panel_key:3:9fc8530431a3:dungeon_gameplay -> panel_key:4:9fc8530431a3:dungeon_gameplay -> panel_key:5:9fc8530431a3:dungeon_gameplay ...
- `right_right` keys `['KP_Right', 'KP_Right']` unique_hashes=4: route:baseline:48ed3743ab6a:dungeon_gameplay -> route:KP_Right:fbeb1b82cd09:wall_closeup -> route:KP_Right:9fc8530431a3:dungeon_gameplay -> panel_key:F1:9fc8530431a3:dungeon_gameplay -> panel_key:F2:9fc8530431a3:dungeon_gameplay -> panel_key:F3:9fc8530431a3:dungeon_gameplay -> panel_key:F4:9fc8530431a3:dungeon_gameplay -> panel_key:1:9fc8530431a3:dungeon_gameplay -> panel_key:2:9fc8530431a3:dungeon_gameplay -> panel_key:3:9fc8530431a3:dungeon_gameplay -> panel_key:4:9fc8530431a3:dungeon_gameplay -> panel_key:5:9fc8530431a3:dungeon_gameplay ...
- `left_right` keys `['KP_Left', 'KP_Right']` unique_hashes=3: route:baseline:48ed3743ab6a:dungeon_gameplay -> route:KP_Left:fbeb1b82cd09:wall_closeup -> route:KP_Right:48ed3743ab6a:dungeon_gameplay -> panel_key:F1:48ed3743ab6a:dungeon_gameplay -> panel_key:F2:48ed3743ab6a:dungeon_gameplay -> panel_key:F3:48ed3743ab6a:dungeon_gameplay -> panel_key:F4:48ed3743ab6a:dungeon_gameplay -> panel_key:1:48ed3743ab6a:dungeon_gameplay -> panel_key:2:48ed3743ab6a:dungeon_gameplay -> panel_key:3:48ed3743ab6a:dungeon_gameplay -> panel_key:4:48ed3743ab6a:dungeon_gameplay -> panel_key:5:48ed3743ab6a:dungeon_gameplay ...
- `right_left` keys `['KP_Right', 'KP_Left']` unique_hashes=3: route:baseline:48ed3743ab6a:dungeon_gameplay -> route:KP_Right:fbeb1b82cd09:wall_closeup -> route:KP_Left:48ed3743ab6a:dungeon_gameplay -> panel_key:F1:48ed3743ab6a:dungeon_gameplay -> panel_key:F2:48ed3743ab6a:dungeon_gameplay -> panel_key:F3:48ed3743ab6a:dungeon_gameplay -> panel_key:F4:48ed3743ab6a:dungeon_gameplay -> panel_key:1:48ed3743ab6a:dungeon_gameplay -> panel_key:2:48ed3743ab6a:dungeon_gameplay -> panel_key:3:48ed3743ab6a:dungeon_gameplay -> panel_key:4:48ed3743ab6a:dungeon_gameplay -> panel_key:5:48ed3743ab6a:dungeon_gameplay ...
- `left_left_right` keys `['KP_Left', 'KP_Left', 'KP_Right']` unique_hashes=4: route:baseline:48ed3743ab6a:dungeon_gameplay -> route:KP_Left:fbeb1b82cd09:wall_closeup -> route:KP_Left:9fc8530431a3:dungeon_gameplay -> route:KP_Right:fbeb1b82cd09:wall_closeup -> panel_key:F1:fbeb1b82cd09:wall_closeup -> panel_key:F2:fbeb1b82cd09:wall_closeup -> panel_key:F3:fbeb1b82cd09:wall_closeup -> panel_key:F4:fbeb1b82cd09:wall_closeup -> panel_key:1:fbeb1b82cd09:wall_closeup -> panel_key:2:fbeb1b82cd09:wall_closeup -> panel_key:3:fbeb1b82cd09:wall_closeup -> panel_key:4:fbeb1b82cd09:wall_closeup ...
- `right_right_left` keys `['KP_Right', 'KP_Right', 'KP_Left']` unique_hashes=4: route:baseline:48ed3743ab6a:dungeon_gameplay -> route:KP_Right:fbeb1b82cd09:wall_closeup -> route:KP_Right:9fc8530431a3:dungeon_gameplay -> route:KP_Left:fbeb1b82cd09:wall_closeup -> panel_key:F1:fbeb1b82cd09:wall_closeup -> panel_key:F2:fbeb1b82cd09:wall_closeup -> panel_key:F3:fbeb1b82cd09:wall_closeup -> panel_key:F4:fbeb1b82cd09:wall_closeup -> panel_key:1:fbeb1b82cd09:wall_closeup -> panel_key:2:fbeb1b82cd09:wall_closeup -> panel_key:3:fbeb1b82cd09:wall_closeup -> panel_key:4:fbeb1b82cd09:wall_closeup ...

## Interesting/non-baseline probes

- `baseline` panel_key `Escape` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `right_top` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `move_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `right_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `left_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `bottom_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `viewport_left_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `viewport_mid_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `viewport_right_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `resurrect_left` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `baseline` click `reincarnate_right` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` panel_key `Escape` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `right_top` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `move_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `right_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `left_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `bottom_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `viewport_left_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `viewport_mid_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `viewport_right_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `resurrect_left` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left` click `reincarnate_right` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` panel_key `Escape` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `right_top` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `move_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `right_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `left_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `bottom_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `viewport_left_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `viewport_mid_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `viewport_right_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `resurrect_left` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right` click `reincarnate_right` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` panel_key `Escape` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `right_top` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `move_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `right_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `left_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `bottom_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `viewport_left_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `viewport_mid_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `viewport_right_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `resurrect_left` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left` click `reincarnate_right` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` panel_key `Escape` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `right_top` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `move_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `right_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `left_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `bottom_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `viewport_left_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `viewport_mid_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `viewport_right_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `resurrect_left` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_right` click `reincarnate_right` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` panel_key `Escape` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `right_top` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `move_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `right_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `left_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `bottom_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `viewport_left_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `viewport_mid_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `viewport_right_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `resurrect_left` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_right` click `reincarnate_right` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` panel_key `Escape` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `right_top` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `move_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `right_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `left_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `bottom_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `viewport_left_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `viewport_mid_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `viewport_right_mirror` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `resurrect_left` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `right_left` click `reincarnate_right` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left_right` panel_key `Escape` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left_right` click `right_top` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`
- `left_left_right` click `move_mid` -> `6d20a6fee397` `graphics_320x200_unclassified` `320x200 graphics frame, but layout heuristics did not match a known class`

## Interpretation

This pass is evidence-only. If it does not discover a party/spell/inventory classifier transition, the next blocker is not basic input delivery but original route semantics: the automation can turn the no-party viewport, but still lacks the original champion/recruitment path needed for DM1 V1 overlay parity.

