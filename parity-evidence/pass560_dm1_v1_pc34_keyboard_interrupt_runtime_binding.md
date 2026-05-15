# Pass560 - DM1 V1 PC34 keyboard interrupt/runtime binding

- Status: BLOCKED_PASS560_RUNTIME_C254_IO_DRIVER_VECTOR_NOT_CAPTURED
- Manifest: parity-evidence/verification/pass560_dm1_v1_pc34_keyboard_interrupt_runtime_binding/manifest.json

## ReDMCSB anchors
- PASS IBMIO.C F8088_ / S8A_Vector lines 381-390,399-414 - PC34 IO startup saves INT 09 and installs S8A_Vector as the raw keyboard interrupt handler.
- PASS IBMIO.C S8A_Vector lines 527-529,581-611,623-623 - The raw INT 09 handler queues accepted make scancodes, acknowledges the keyboard/PIC path, or chains to the saved vector.
- PASS IBMIO.C F8089_ lines 629-643 - PC34 IO shutdown restores the saved INT 09 vector.
- PASS IBMIO.C F8090_ / F8091_ / G8101_apc_IOInterruptVector lines 646-680,2378-2381 - The game-facing keyboard API is IO_DRIVER slot 0/1, not the INT 09 vector itself.
- PASS IBMIO.C main IO driver loop lines 2550-2552,2582-2586 - The IO driver publishes the G8101 table through C254 before child game execution and removes hooks after return.
- PASS CEDT026.C F0539_INPUT_Cconis / startup lines 217-220,238-238 - The game-side keyboard-present path is driven from G2162_IODriver loaded from C254.
- PASS IO2.C F0540_INPUT_Crawcin / F0539_INPUT_Cconis lines 27-61,179-184 - I34E movement input enters via IO_DRIVER slot 0/1 and only then becomes the K/L/M/P movement-table codes.
- PASS GAMELOOP.C F0002_MAIN_GameLoop_CPSDF lines 164-219 - The game loop consumes the bound IO_DRIVER keyboard API before command-queue dispatch.
- PASS COMMAND.C F0361_COMMAND_ProcessKeyPress / F0380_COMMAND_ProcessQueue_CPSC lines 1734-1768,2075-2127,2150-2156 - Runtime movement promotion requires F0361 enqueue/count increment and F0380 pop/count decrement/dispatch.

## Decision

Pass559 null INT09-handler decode is not the game-facing movement blocker by itself: ReDMCSB routes raw IRQ09 through S8A_Vector, then exposes keyboard input to DM.EXE through C254 -> G8101_apc_IOInterruptVector slot0/slot1. Next capture must decode C254 and slots 0/1 before checking M528/F0361/F0380.
