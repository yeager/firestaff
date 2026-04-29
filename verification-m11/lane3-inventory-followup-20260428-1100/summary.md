# Lane3 Inventory follow-up — 2026-04-28 11:00 CEST

Lane: DM1 V1 inventory/UI/slots/icons on N2 (`Firestaff-Worker-VM`). Branch: `sync/n2-dm1-v1-20260428`.

## Narrow follow-up landed

Added `INV_GV_362B` to the M11 game-view probe. It covers the previously documented compact-model edge: original source backpack boxes `C528..C536` / slot-box indices `29..37` are valid DM1 route zones, but Firestaff currently models only eight backpack slots (`C520..C527`). The new gate proves those remaining source boxes:

- still resolve through the source inventory mouse table as viewport-relative `COMMAND.C` commands `C049..C057`;
- map to no Firestaff champion slot (`M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox(...) == -1`);
- do not alias/mutate populated compact slots (`ready hand`, `backpack 8`);
- do not create a transient leader-hand pickup.

This turns the known backpack-model gap into an explicit non-aliasing regression gate instead of just a comment/negative note.

## Commands and outputs

Run dir: `<repo>`

```sh
mkdir -p verification-m11/lane3-inventory-followup-20260428-1100
./run_firestaff_m11_game_view_probe.sh verification-m11/lane3-inventory-followup-20260428-1100 2>&1 | tee verification-m11/lane3-inventory-followup-20260428-1100/game_view_probe.full.log
```

Key output:

```text
PASS INV_GV_362B V1 source backpack slots C528..C536 route but do not alias compact Firestaff backpack slots
# summary: 579/579 invariants passed
M11 game-view probe: # summary: 579/579 invariants passed
```

```sh
./run_firestaff_memory_graphics_dat_slots_probe.sh 2>&1 | tee verification-m11/lane3-inventory-followup-20260428-1100/slots_probe.log
```

Output:

```text
ok
```

## Blocker status

No runtime inventory bug was found in this bounded pass. The product gap remains model expansion for original backpack source boxes beyond the compact Firestaff eight-slot backpack; this commit only hardens the current behavior so those source boxes cannot silently alias the wrong champion slot while that expansion is pending.
