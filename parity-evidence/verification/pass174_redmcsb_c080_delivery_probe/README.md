# Pass174 — ReDMCSB C080/F0377 delivery probe

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260429-081157-pass174-redmcsb-c080-delivery-probe`
- evidence root: `parity-evidence/verification/pass174_redmcsb_c080_delivery_probe`
- completed: 12
- errors: 0
- buckets: {'click-no-visible-delta': 12}
- ReDMCSB source root: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source`

## ReDMCSB source audit

- `COMMAND.C:397-403,2322-2323` — C007_ZONE_VIEWPORT left-click maps to C080_COMMAND_CLICK_IN_DUNGEON_VIEW, which calls F0377_COMMAND_ProcessType80_ClickInDungeonView.
- `CLIKVIEW.C:348-349` — PC build converts screen coordinates to viewport-relative by subtracting G2067_i_ViewportScreenX/G2068_i_ViewportScreenY.
- `CLIKVIEW.C:407-431` — with empty leader hand, a hit in C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT calls F0372.
- `CLIKVIEW.C:21-25` — F0372 computes the front square and calls F0275_SENSOR_IsTriggeredByClickOnWall on the opposite side.
- `MOVESENS.C:1392,1501-1502` — C127_SENSOR_WALL_CHAMPION_PORTRAIT is allowed with no leader and calls F0280_CHAMPION_AddCandidateChampionToParty.
- `DUNVIEW.C:525,3913-3928` — the portrait box is source-locked to viewport x=96..127 y=35..63; center maps to screen x=111,y=82 for PC y-origin 33.
- `COORD.C:1693-1698` — G2067_i_ViewportScreenX=0 and F20E G2068_i_ViewportScreenY=33.

## Result

- `helper_scaled_window_relative_source_screen_portrait_center`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `helper_scaled_window_relative_source_center_if_viewport_y31`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `helper_scaled_window_relative_source_center_plus_one`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `absolute_screen_click_source_screen_portrait_center`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `absolute_screen_click_source_center_if_viewport_y31`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `absolute_screen_click_source_center_plus_one`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_down_up_source_screen_portrait_center`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_down_up_source_center_if_viewport_y31`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_down_up_source_center_plus_one`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_click_repeat3_source_screen_portrait_center`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_click_repeat3_source_center_if_viewport_y31`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
- `window_relative_click_repeat3_source_center_plus_one`: **click-no-visible-delta** delta={'bbox': None, 'changed_pixels': 0, 'changed_ratio': 0.0}
