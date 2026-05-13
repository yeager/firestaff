# Pass510 - DM1 V1 original capture route label filename fixture

Status: `PASS510_ORIGINAL_CAPTURE_ROUTE_LABEL_FILENAME_FIXTURE`

The capture normalizer now uses each `shot:<label>` token as the normalized crop filename stem. This removes the hard-coded legacy filename drift that made pass487 report `turn_left_click` beside `02_ingame_turn_right...`.

## Fixture
- labels: `start_south, turn_right_west, forward_west_blocked, turn_left_east, forward_south_corridor, post_redraw_after_vblank`
- filenames match labels: `True`
- crop manifest matches labels: `True`
- legacy drift rows: `0`

## Source references audited
- `COMMAND.C:2045-2156` ok=True - queued route input must dispatch through the original command processor before movement/turn labels are semantic
- `CLIKMENU.C:142-174,180-347` ok=True - turn/move route labels name the state mutations implemented here
- `DUNVIEW.C:8318-8618` ok=True - captured viewport crops are only meaningful after this tuple-specific draw path
- `DRAWVIEW.C:709-858` ok=True - the capture seam must be the PC34 viewport present path, not menu/setup echoes

## Original assets checked
- `DM.EXE` exists=True sha256=4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4
- `DUNGEON.DAT` exists=True sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- `GRAPHICS.DAT` exists=True sha256=2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- `TITLE` exists=True sha256=adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745

## GreatStone cross-check
- manifest: `/home/trv2/.openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json`
- result: `PASS`; pc34 graphics items: `713`; dungeon maps: `14`; mismatches: `0`

## Non-claims
This fixture does not launch DOSBox, does not promote screenshots, and does not claim pixel parity.
