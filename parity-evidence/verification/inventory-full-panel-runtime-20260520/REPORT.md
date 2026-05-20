# Inventory Full Panel Runtime Slice - 2026-05-20

Status: PASS for the targeted inventory runtime slice.

## Scope

This slice extends the normal V1 inventory panel runtime bridge so source backpack slot boxes C528..C536 no longer stop as unmapped zones. They now map to reserved champion inventory storage slots `CHAMPION_SLOT_BACKPACK_9..17`, route through the existing source mouse command bridge, and mutate storage through the existing leader-hand pickup/place path.

Out of scope: chest linked-list runtime integration beyond the already-landed helper, full visual icon parity for every new backpack cell with original assets, broad unrelated `m11_game_view` failures.

## ReDMCSB anchors

- `DEFS.H:778-817` defines source slots `C00_SLOT_READY_HAND` through `C37_SLOT_CHEST_8`, including `C13..C29` backpack and `C30..C37` chest.
- `DATA.C:1049-1087` defines `G0038_ai_Graphic562_SlotMasks[38]`: 30 champion inventory masks plus 8 container/chest masks.
- `CHAMPION.C:F0302`, `CHAMPION.C:677-699` resolves inventory slot-box clicks to source slot index, reads champion slots or `G0425_aT_ChestSlots`, no-ops empty/empty, and rejects leader-hand objects whose `AllowedSlots` do not match `G0038_ai_Graphic562_SlotMasks`.
- `CHAMPION.C:F0302`, `CHAMPION.C:700-712` performs the leader-hand/slot swap and redraw.
- `PANEL.C:F0347`, `PANEL.C:1651-1691` closes/rebuilds panel content from the inventory champion action hand and dispatches food/water, chest, or scroll panel content.

## Runtime changes

- `include/memory_champion_state_pc34_compat.h`: names reserved storage slots 21..29 as `CHAMPION_SLOT_BACKPACK_9..17`; aliases `CHAMPION_SLOT_ACTION_HAND` to the real right-hand/action-hand storage slot.
- `src/engine/m11_game_view.c`: maps source slot boxes 29..37 / zones C528..C536 to `BACKPACK_9..17`, draws them in the normal V1 inventory dynamic overlay loop, and keeps action/status hand reads on the real action-hand slot.
- `probes/m11/firestaff_m11_game_view_probe.c`: updates `INV_GV_362/362B` to assert the full C507..C536 slot bridge and C528..C536 storage mutation.
- `tests/test_m11_inventory_full_panel_runtime_pc34_compat.c`: focused deterministic source-lock gate for full backpack source mapping and click pickup/place runtime.

## Verification

- Configure/build: see `build.log`.
- Targeted CTest: see `ctest-targeted.log`; result `100% tests passed, 0 tests failed out of 5`.
- Focused runtime probe: see `full_panel_runtime_probe.log`; result `17 passed, 0 failed`.
- Broad `m11_game_view` probe: see `m11_game_view_probe.log` and `m11_game_view_inventory_excerpt.log`; inventory probes `INV_GV_362`, `INV_GV_362A`, `INV_GV_362B`, `INV_GV_362C`, `INV_GV_363`, `INV_GV_353..358` passed. The broad test remains failing on unrelated pre-existing non-inventory probes (`INV_GV_07I*`, `INV_GV_33..36`, `INV_GV_172I`); this slice did not address those.
- `git diff --check`: PASS.
- High-signal secret scan of changed diff: PASS; no matches for key/token/password/private-key patterns.
