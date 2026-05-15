# Pass562 - DM1 V1 keyboard next boundary after pass560

- Status: PASS562_DM1_V1_NEXT_BOUNDARY_C254_IO_DRIVER_SLOT_DECODE_LOCKED
- Selected next boundary: C254/IO_DRIVER slot decode
- Pass560 status: BLOCKED_PASS560_RUNTIME_C254_IO_DRIVER_VECTOR_NOT_CAPTURED
- Manifest: parity-evidence/verification/pass562_dm1_v1_pass560_keyboard_next_boundary/manifest.json

## Canonical DM1 anchors
- DUNGEON.DAT: sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 (PASS)
- GRAPHICS.DAT: sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e (PASS)

## ReDMCSB source locks
- PASS C254/IO_DRIVER slot decode: IBMIO.C 2378-2381 - C254 publishes an IO_DRIVER table whose slots 0 and 1 are the keyboard read/present API.
- PASS C254/IO_DRIVER slot decode: IBMIO.C 2550-2552 - The IO driver installs C254 before the child game runs.
- PASS C254/IO_DRIVER slot decode: CEDT026.C 235-238 - The game binds G2162_IODriver from interrupt C254.
- PASS IO2/F0540 dispatch: IO2.C 27-61 - F0540 is downstream of slot 0 and normalizes PC34 arrow scancodes to movement-table codes.
- PASS IO2/F0540 dispatch: IO2.C 179-184 - F0539 keyboard-present is downstream of IO_DRIVER slot 1.
- PASS F0361 enqueue: GAMELOOP.C 164-168,215-219 - The game loop calls F0361 only after M527/M528 have consumed IO_DRIVER input.
- PASS F0361 enqueue: COMMAND.C 1753-1768 - F0361 is the enqueue boundary after F0540 has returned a movement-table code.

## Decision

Choose C254/IO_DRIVER slot decode next. IO2/F0540 dispatch and F0361 enqueue are both source-locked, but they are downstream of the C254 table and slot 0/1 binding. A runtime probe that jumps straight to F0540 or F0361 would merge boundaries and would not explain whether pass560 failed at the vector/table binding or later input dispatch.

## Next probe contract
- in one bounded N2 runtime, after IO startup, read IVT C254 at 0000:03F8
- decode C254 as non-null G8101_apc_IOInterruptVector
- decode table slot 0 as F8090_ and slot 1 as F8091_ before any IO2/F0540 claim
- only after that, continue to F0539/F0540 hit/value and then F0361 enqueue/G2153 increment
