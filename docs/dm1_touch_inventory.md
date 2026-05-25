# DM1 V1 — Touchscreen Inventory Input

## Source Lock
ReDMCSB WIP20210206: CLIKCHAM.C F0360/F0361; INVENTORY.C F0428/F0429; PANEL.C F0351; COMMAND.C:419-438; DATA.C:986-996; layout-696 C105/C507-C536.

## Entering/Exiting Inventory Mode
- Right-click any champion panel → C011 → toggle inventory mode (COMMAND.C:404/412)
- Left-click save icon (C140/C562), rest icon (C145/C564), or close icon (C011/C566) → exit inventory
- ESC key → F0363_COMMAND_HighlightBoxDisable → exit inventory mode

## Inventory Screen Layout
When inventory is open the viewport renders the selected champions equipment grid:

| Region | Position | Content |
|--------|----------|---------|
| Equipment slots | viewport-relative | 8 body slots (ready hand, action hand, head, torso, legs, feet, pouch2, quiver) |
| Backpack grid | viewport-relative | 30 slots in 6-column grid (slots 13-42) |
| Chest area | viewport-relative | Up to 8 chest slots (when chest open) |
| Save icon | 179,2 11×11 | C140 save/quit |
| Rest icon | 190,2 19×11 | C145 rest |
| Close icon | 209,2 11×11 | C011 close inventory |
| Music toggle | 168,3 9×9 | C141 toggle music |

Equipment slot positions (viewport-relative, 16×16 each):

```
Ready Hand:      x=6,   y=53  (C028/C507)
Action Hand:     x=62,  y=53  (C029/C508)
Head:            x=34,  y=26  (C030/C509)
Torso:           x=34,  y=46  (C031/C510)
Legs:            x=34,  y=66  (C032/C511)
Feet:            x=34,  y=86  (C033/C512)
Pouch 2:         x=6,   y=90  (C034/C513)
Quiver Line2-1:  x=79,  y=73  (C035/C514)
Quiver Line1-2:   x=62,  y=90  (C036/C515)
Quiver Line2-2:   x=79,  y=90  (C037/C516)
Neck:            x=6,   y=33  (C038/C517)
Pouch 1:         x=6,   y=73  (C039/C518)
... backpack slots 13-42 use computed grid: x=8+(col%6)*32, y=8+(row/6)*28
```

## Tap — Pick Up Item (F0360)
When cursor is empty and player taps an occupied slot:
```
CLIKCHAM.C F0360:
  if slot occupied and cursor empty:
    → copy item from slot to cursor
    → clear slot
    → recalculate champion load
```

## Tap — Place Item (F0361)
When cursor holds an item and player taps an empty or compatible slot:
```
CLIKCHAM.C F0361:
  if cursor holds item:
    if target slot empty:
      → copy cursor item to slot
      → clear cursor
    else if target slot same item type:
      → stack items (if stackable)
    recalculate champion load
```

## Tap — Drop Item (F0361 / COMMAND.C:2296)
If cursor holds an item and player taps the floor (viewport cell 0-3):
- F0374: throw/place object on target dungeon square

## Tap — Swap Hands (INVENTORY.C F0428)
Tapping the action hand slot when it is empty and the ready hand is occupied:
- Moves ready hand item to action hand slot

## Tap — Equipment Restriction
Not all items can go in all slots. PC34 slot mask constants (dm1_v1_inventory_pc34_compat.c):

| Body Part | Allowed Mask |
|-----------|--------------|
| Head | DM1_PC34_ALLOWED_HEAD |
| Torso | DM1_PC34_ALLOWED_TORSO |
| Legs | DM1_PC34_ALLOWED_LEGS |
| Feet | DM1_PC34_ALLOWED_FEET |
| Pouch | DM1_PC34_ALLOWED_POUCH |
| Quiver | DM1_PC34_ALLOWED_QUIVER_LINE1/LINE2 |
| Backpack | DM1_PC34_ALLOWED_ANY_SLOT |
| Container | DM1_PC34_ALLOWED_CONTAINER |

Attempting to place an item in a forbidden slot does not move the item.

## Backpack Grid
30 backpack slots (slots 13-42), laid out 6 columns × 5 rows:
```
slot 13: x=8,   y=8   (BP L1/1)
slot 14: x=40,  y=8   (BP L2/2)
...
slot 42: x=200, y=116 (BP L2/9)
```
Tap any backpack slot to pick up or place items using the same cursor-based flow.

## Chest Interaction
When a chest is open (G0293_ui_OpenChestThing != 0), chest slots appear in inventory:
```
Chest slots: viewport-relative, computed positions below backpack
Pickup/place works identically to regular inventory slots
CHAMDRAW.C F0292: chest slot count = DM1_PC34_CHEST_SLOT_COUNT (8)
```

## Load Calculation
Every inventory change triggers m11_inventory_recalc_load():
- Sum all slot weights (ready hand through quiver)
- Add chest slot weights if chest open
- Result stored in champion inventory load field

## Champion Switch in Inventory
Right-click on a champion name/portrait in inventory → switch active champion (same F0367 leader-toggle logic, but in inventory context the behavior is champion selection for inventory view).

## Source Evidence
- CLIKCHAM.C F0360: pickup item to cursor (slot occupied, cursor empty)
- CLIKCHAM.C F0361: place item from cursor (cursor item, target slot)
- INVENTORY.C F0428: swap hands
- PANEL.C F0351: statistic value layout and coloring
- COMMAND.C:419-438: inventory slot routing for all equipment slots
- DATA.C:986-996: slot box positions from graphic 562 layout
- CHAMDRAW.C F0292: chest slot handling and load recalculation
