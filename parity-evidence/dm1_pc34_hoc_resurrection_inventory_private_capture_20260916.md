# DM1 PC 3.4 HoC resurrection and inventory private-capture receipt

## Scope

This receipt records a fresh, private original-runtime transaction from the
English DOS PC 3.4 edition: Entrance, Hall of Champions, C127 candidate
selection, Chani resurrection, and the source-owned secondary-button inventory
open route.  No game frame, recording, game data, or emulator dump is stored
in this repository.

## Source and capture boundary

- Original program `DM.EXE` SHA-256:
  `4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4`.
- Original `DATA/DUNGEON.DAT` SHA-256:
  `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`.
- Emulator: DOSBox-X, used only as offline capture tooling.
- Input: original 320×200 mouse coordinates plus paced keypad commands.  Each
  movement command was separated by 500 ms so the original command queue
  consumed every source action.

## Observed private frames

| State | SHA-256 |
| --- | --- |
| Chani resurrected | `1717d2c59aaa33cc5b1b9655935e6e554d4e082ba05098dd9fc3703f68096fc5` |
| Chani inventory opened by secondary button | `87891c4bf94015832710d8091c8cfbf9c9f302b5fcda6545c750dbfed33c6e5c` |
| Selected inventory-slot attempt | `c6f9ac40dcc1bef88a16a9c4f4341c7e2c5a72e2247eb0828e183628462bc19c` |
| Follow-up slot attempt | `196708ea35939f8319a62cb2cde4ada9349fc34afb4e6b662faa6f3863c5f6a2` |

The raw-frame health gate accepted all four 320×200 frames.  The inventory
slot selected in this particular authentic run was empty, so the last two rows
do **not** establish an item-to-hand transfer.

## What this proves and what remains open

The result proves the real DOS PC 3.4 C127 → resurrection → secondary-click
inventory route and fixes the prior uncertainty about whether the Hall route
had reached a usable champion state.  It does not prove floor pickup,
scroll-in-hand/Eye text, or full pixel parity.  Those require a route to a
real floor object and an authentic scroll-bearing state, captured privately
from the same edition.
