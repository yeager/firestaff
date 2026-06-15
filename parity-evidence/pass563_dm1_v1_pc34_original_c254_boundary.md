# Pass563 - DM1 V1 PC34 original C254 boundary

- Status: BLOCKED_PASS563_PC34_C254_CHAIN_INCOMPLETE
- Manifest: parity-evidence/verification/pass563_dm1_v1_pc34_original_c254_boundary/manifest.json

## Decision

Existing pass514/pass558/pass560/pass562 evidence chain is incomplete in this checkout.

## Canonical original payload
- DM.EXE: PASS sha256 4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4
- DATA/GRAPHICS.DAT: PASS sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- DATA/DUNGEON.DAT: PASS sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- original manifest: PASS /Users/bosse/.openclaw/data/firestaff-original-games/DM/_manifests/dm_originals_asset_inventory_20260510.json

## ReDMCSB locks
- PASS GAMELOOP.C:166-215 pc34_game_loop_consumes_io_before_queue_dispatch - DM1 PC/I34E consumes keyboard-present/read before F0361 enqueue and F0380 dispatch.
- PASS DEFS.H:3143-4335 i34e_keyboard_macros_bind_to_io2 - I34E keyboard macros are IO2 calls backed by IO_DRIVER slot 0/1 through C254.
- PASS IO2.C:5-184 io2_slot0_slot1_dispatch - F0540/F0539 are downstream of decoded IO_DRIVER slot 0/1, not raw host input.
- PASS IBMIO.C:358-528 ibmio_installs_keyboard_irq09_and_buffers_scancodes - IBMIO first receives raw IRQ09 scancodes and buffers accepted keys.
- PASS IBMIO.C:2379-2586 ibmio_publishes_game_facing_c254_io_driver_table - The game-facing boundary is C254 -> G8101 table -> F8090/F8091.
- PASS CEDT026.C:220-238 game_binds_g2162_from_c254 - The child game binds G2162_IODriver from C254 before using IO2 keyboard-present/read.
- PASS COMMAND.C:1357-2121 f0361_f0380_queue_contract - F0361 enqueue/count and F0380 pop/count remain downstream proof points after C254 slot decode.

## Pass chain

- pass514: BLOCKED_PASS514_MISSING_N2_DEBUGGER_PREREQUISITE
- pass558: BLOCKED_PASS558_PASS514_IO_CLASSIFICATION_INCOMPLETE
- pass560: BLOCKED_PASS560_RUNTIME_C254_IO_DRIVER_VECTOR_NOT_CAPTURED
- pass562: PASS562_DM1_V1_NEXT_BOUNDARY_C254_IO_DRIVER_SLOT_DECODE_LOCKED

## Next bounded runtime probe

- Read IVT C254 at 0000:03F8 after IBMIO startup in the canonical PC34 run.
- Decode C254 as G8101_apc_IOInterruptVector and table slot 0 as F8090_, slot 1 as F8091_.
- Only then continue to IO2/F0539/F0540, F0361 enqueue/G2153 increment, and F0380 pop/dispatch.
