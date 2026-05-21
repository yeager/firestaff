# DM1 V1 champion statistic drawn-panel pixel gate

Status: `PASS_SOURCE_LOCKED_PIXEL_PARITY_BLOCKED_ON_ORIGINAL_REFERENCE`

This gate locks the ReDMCSB draw contract for the inventory eye-panel champion statistic rows. It deliberately stops short of original-runtime pixel parity.

## Source Evidence

- PASS `PANEL.C:F0351 draws the empty panel then statistic text runs` - PANEL.C:2013-2108
- PASS `DEFS.H statistic and zone ids` - DEFS.H:743-754,3919-3927
- PASS `COORD.C text metrics and one-pixel zone margin` - COORD.C:1753-1758,2434-2448
- PASS `Greatstone PC 3.4 data index` - /home/trv2/.openclaw/data/firestaff-greatstone-atlas/raw/greatstone.free.fr__dm__g_dm.html.html
- PASS `Original PC34 canonical GRAPHICS.DAT` - _canonical/dm1/GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- PASS `Original PC34 canonical DUNGEON.DAT` - _canonical/dm1/DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85

## Firestaff Join

- PASS `local header exposes source statistic panel constants` - include/dm1_v1_champion_panel_hud_pc34_compat.h:131-138,235
- PASS `local implementation preserves split current/max text runs` - src/dm1/dm1_v1_champion_panel_hud_pc34_compat.c:288-313
- PASS `local tests cover the drawn statistic text-run contract` - tests/test_dm1_v1_champion_panel_hud_pc34_compat.c; tests/test_m11_inventory_full_panel_runtime_pc34_compat.c
- PASS `CMake registers pass621 gate` - CMakeLists.txt

## Remaining Blockers

- A verified original PC 3.4 320x200 runtime frame with inventory open, empty leader hand, eye-panel champion statistics visible, and a documented input transcript/state snapshot.
- A matching Firestaff indexed framebuffer generated from the same champion, inventory, panel-content, and palette state.
- A crop/overlay comparator for the panel statistic text rows that separates palette-index deltas, glyph/text-run deltas, and panel background deltas.

## Non-Claims

- No original-vs-Firestaff pixel parity claim.
- No renderer behavior change.
- No original DOS runtime capture was launched by this gate.
- No push, package, tag, release, or external action.
