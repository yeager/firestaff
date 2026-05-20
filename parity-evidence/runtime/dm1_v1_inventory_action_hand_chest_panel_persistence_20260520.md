# DM1 V1 Inventory Action-Hand Chest Panel Persistence

## ReDMCSB Source Evidence

- `COMMAND.C:498-507` defines `G0456_as_Graphic561_MouseInput_PanelChest`: commands `C058..C065` route viewport-relative chest slot boxes `C537..C544`.
- `CHEST.C:43-46` records the open container in `G0426_T_OpenChest` and draws the open chest icon when not pressing the eye.
- `CHEST.C:53-76` copies at most the first eight linked container things into `G0425_aT_ChestSlots` for the visible panel.
- `CHEST.C:112-133` compacts non-empty `G0425_aT_ChestSlots` back into the container linked list and clears `G0426_T_OpenChest` on close.
- `CHAMPION.C:512-514` removes chest-slot items from `G0425_aT_ChestSlots`, while `CHAMPION.C:609-610` adds placed items back to those visible chest slots.
- `CHAMPION.C:557-562` closes the open chest when the inventory action-hand chest being removed matches `G0426_T_OpenChest`.
- `CHAMPION.C:636-638` marks the inventory panel dirty when a container is added to the inventory action hand.
- `PANEL.C:1651-1691` closes any previous chest, reads `C01_SLOT_ACTION_HAND`, selects `M569_PANEL_CHEST` for containers, then routes the current action-hand object through `F0342`/`F0333`.

## Firestaff Runtime Slice

The M11 V1 inventory bridge already had source-backed C537..C544 chest slot clicks and linked-list writeback. This pass tightens the action-hand panel lifetime: C508 action-hand slot clicks now clear `v1OpenChestThing` when the open chest is removed or replaced, and reopen panel state when a container is placed into the action hand.

The regression lives in `tests/test_m11_inventory_full_panel_runtime_pc34_compat.c` and proves:

- picking up the open action-hand chest clears open-panel state;
- stale C537 chest clicks are ignored after the close;
- the container linked-list head and next pointer survive the action-hand panel close;
- placing the held chest back into C508 reopens panel state for the same container.
