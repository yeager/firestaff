# Nexus retail inventory-button input trace (2026-10-25)

## Setup

Ymir 0.4.1 ran on TRV2 with `--debug` against the Japanese retail Saturn
disc. The isolated profile used a hash-verified Japanese Saturn BIOS and an
authentic retail gameplay savestate. ControlPad port 1 mapped `K` to B and
`J` to A. `before-button-b.png` is the savestate view before the input
sequence; `after-button-b.png` follows B; `after-button-a.png` follows A
while the inventory flow is active. These are emulator captures of retail
game output, not Firestaff renders or generated game data.

| Input | SHA-256 |
| --- | --- |
| Retail cue | `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699` |
| Retail `DM.BIN` | `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124` |
| Japanese Saturn BIOS | `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac` |
| Gameplay savestate | `942154f51530094c4f553d3254573098d056f90da2fbb5966448aefdefeb3372` |

## Observed button and event sequence

The Ymir `--debug` memory dumps preserve the event queue in high WRAM at
`0x060797b0` (12-byte records). After B, the first queued event is ID 12;
after A in the inventory flow, the first queued event is ID 39 with argument
8. The hash-pinned `DM.BIN` event-name pointer table at file offset `0x3adc4`
resolves these to `EV_INVENTORY` and `EV_RING_SELECT`. The event-39 jump-table
case at `DM.BIN:0x7ee` calls `0x060327c8` with the argument and current context.

After A, the visible retail view changes to an equipment page with Japanese
text `頭に装備` (“equip to head”). Disassembly of the event handlers confirms
that event 39 routes a UI page callback through the current view object. Event
12 validates and stores a pair in the global UI selection list. Neither handler
writes to the champion inventory/equipment arrays in this capture. The first
four candidate `0x13c` records keep their item slots unchanged. Context record
10, resolved by the retail context base `0x0607fba0` and stride `0x13c`, changes
bytes at offsets `+0x122` and `+0x124` from `ffff0000` to `141a4000`, but there
is no proven join from that record to a champion or to an item declaration.
The trace therefore proves button-driven menu navigation, not item transfer,
and does not authorize an inventory mutation in Firestaff.

The three complete 1 MiB high-WRAM dumps came from Ymir's Debug → Dump all
memory command. Their SHA-256 values are:

| File | SHA-256 |
| --- | --- |
| `wram-hi-before.bin` | `9a838e897324452a2a757ae1de58c602afdae9fe05c594f10dc2b626338e5e91` |
| `wram-hi-after-b.bin` | `3c80558714d5b410e32c23b354a8364c9e3764a2e3e283bb3493b198bdc871b0` |
| `wram-hi-after-b-a.bin` | `1b3c4e80cac9bc4d67c1bb522a2a7f4e2e712ff6e63dda6c183140e1a11da32d` |

## Captures

| File | SHA-256 |
| --- | --- |
| `before-button-b.png` | `b9f7b32ceeabba9ff9560d98d54176200064de3ba6f4ea80ea8a38d4b618fdd9` |
| `after-button-b.png` | `3dcf0d062b8cf644b5546610f4179cd67b8be97a473839f55aa897be2bde73b3` |
| `after-button-a.png` | `de45a6b30586452f99bd5ac6bca7cd8a7a878a40c68afd0792fb8e64097a87e9` |

The Firestaff input contract and remaining join requirements are recorded in
[`docs/nexus_input.md`](../../../docs/nexus_input.md).

## Reproduction

Run the repository analyzer against the external authentic retail `DM.BIN`:

```sh
python3 scripts/analyze_nexus_inventory_button_trace.py \
  /path/to/nexus/DM.BIN \
  parity-evidence/verification/nexus-v1-inventory-button-debug-20261025
```

It verifies the retail disc hash, all three full high-WRAM capture hashes,
event queue values, event names, dispatch destinations, and the observed
context-record byte pair. It deliberately reports the champion/item join and
persistent item transfer as unproven.
