# Pass 76 capture-route state probe

This probe mirrors `verification-screens/capture_firestaff_ingame_series.c` as state, not pixels. It is a Firestaff fixture-route contract only; it does not claim the original DOS route is identical.

| capture | action | result | tick | map | x | y | dir | spellOpen | runes | inventoryOpen | champions | activeChampion | rightHandThing |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 01_ingame_start_latest | start | 1 | 0 | 0 | 1 | 3 | 2 | 0 | 0 | 0 | 1 | 0 | 0x1400 |
| 02_ingame_turn_right_latest | right | 1 | 1 | 0 | 1 | 3 | 3 | 0 | 0 | 0 | 1 | 0 | 0x1400 |
| 03_ingame_move_forward_latest | up | 1 | 2 | 0 | 0 | 3 | 3 | 0 | 0 | 0 | 1 | 0 | 0x1400 |
| 04_ingame_spell_panel_latest | spell_rune_1 | 1 | 2 | 0 | 0 | 3 | 3 | 1 | 1 | 0 | 1 | 0 | 0x1400 |
| 05_ingame_after_cast_latest | spell_cast | 0 | 2 | 0 | 0 | 3 | 3 | 1 | 1 | 0 | 1 | 0 | 0x1400 |
| 06_ingame_inventory_panel_latest | spell_clear+inventory | 1 | 2 | 0 | 0 | 3 | 3 | 0 | 0 | 1 | 1 | 0 | 0x1400 |
