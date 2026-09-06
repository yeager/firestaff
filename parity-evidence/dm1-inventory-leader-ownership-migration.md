# DM1 inventory and leader ownership migration

## Current implementation status

The separate owner is now enabled by normal DM1 panel input. The five
default floor-drop load tests include mouse open/switch/close/reopen
isolation; the old opt-in flag has been removed. Core icon, slot, chest,
scroll, eye, mouth, inventory render and top-row inputs are migrated.
The following original diagnosis and migration requirements are historical
context; broader regression and remaining-consumer auditing are still open.

## Reproduced defect

With champion 0 leading and holding an original weapon, opening champion
1's inventory at screen (114,10) changes `activeChampionIndex` to 1.
All five Atari/Amiga original-media load diagnostics reproduce this.
This was the original failing probe; it now passes in the default load tests.

ReDMCSB PANEL.C:2363 assigns G0423 (inventory champion ordinal).
CLIKCHAM.C F0368 separately assigns G0411 (leader), transfers held weight,
and aligns the new leader's direction. Opening inventory is not F0368.

## Inspected consumers requiring coordinated migration

All entries below refer to `src/engine/m11_game_view.c`.

| Consumer | Required owner |
|---|---|
| `m11_toggle_champion_inventory`, same-open test | Inventory champion, not leader |
| `m11_process_v1_inventory_slot_box_click` | Displayed inventory champion |
| `DM1_V1_M11Runtime_GetInventorySlotIconIndexPc34Compat` | Displayed inventory champion |
| Action-hand chest open and scroll selection | Displayed inventory champion's action hand |
| Mouth consumption and empty-hand eye statistics | Displayed inventory champion |
| Food/water panel, statistics panel, complete inventory renderer | Displayed inventory champion |
| Top-row redraw `inventoryChampionIndex` and ownership `inventoryChampionOrdinal` | Separate inventory selection |
| `m11_dm1_refresh_carried_loads` held-object term | Party leader, unchanged |
| `m11_cycle_active_champion`, `m11_set_active_champion` | Party leader, unchanged |
| Death/revival leader fallback and throw context | Party leader; audit separately |

Do not globally replace `activeChampionIndex`: spell casting already has
an independent caster index, and DM2/CSB have separate runtime mirrors.
Do not temporarily switch the leader to draw or mutate inventory: hash
publication can then transfer held weight to that temporary owner.

## Implementation and verification requirements

Introduce an explicit inventory owner with a closed/unset state and a
single validated accessor. Preserve direct-fixture compatibility explicitly,
not through accidental zero initialization. Migrate DM1 open/close/switch,
input, panel ownership, and rendering as one coherent change. Keep CSB and
DM2 behavior outside this change until their authoritative owners are bound.

Extend the diagnostic beyond merely observing the leader index: exchange
an original Thing in champion 1's inventory while champion 0 holds another,
verify both identities and separate loads on press/release, render champion
1's original icons, then close/reopen and switch inventory owners. Cover
action-hand scroll/chest, mouth, empty eye, and leader change while another
inventory remains open. Repeat in Original and Modern and rerun the full
original-media corpus. Save serialization remains deferred, not verified.

This document is an inspected migration map, not a completed runtime fix.
