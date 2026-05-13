# Pass504 - DM1 V1 keyboard-buffer state-delta blocker

Status: PASS504_KEYBOARD_BUFFER_STATE_DELTA_BLOCKER_LOCKED

## Decision

Keyboard-buffer transcripts are still non-promotable until they prove M528 key extraction, F0361 table match plus G0432/G2153 queue write, and the following F0380 pop/decrement/dispatch before the post-command F0128/F0097 state-delta frame.

## ReDMCSB source audit

- IO2.C:27-61 / F0540_INPUT_Crawcin - PC-34 keyboard evidence starts with an actual Crawcin value after IO2 normalization, not a route-label transcript. ok=True
- COMMAND.C:677-684 / G0459_as_Graphic561_SecondaryKeyboardInput_Movement - F0361 can only queue movement when the drained keyboard code matches the PC-34 secondary keyboard table. ok=True
- COMMAND.C:1709-1813 / F0361_COMMAND_ProcessKeyPress - A keyboard transcript must prove a matching F0361 queue write and G2153 increment; F0361 entry alone is not enough. ok=True
- COMMAND.C:2045-2156 / F0380_COMMAND_ProcessQueue_CPSC - The state delta starts only after F0380 proves non-empty pop/decrement and dispatches to F0365/F0366. ok=True
- GAMELOOP.C:164-219 / F0002_MAIN_GameLoop_CPSDF - The runtime chain is keyboard-buffer drain, F0361 queueing, F0380 processing, then wait-loop exit. ok=True

## Promotion predicate

- capture/log the concrete M528_GetCharacterInKeyboardBuffer value after M527 reports non-empty
- prove that value matches COMMAND.C G0459 PC-34 table row before F0361 exits
- prove F0361 writes G0432 and increments G2153_i_QueuedCommandsCount
- prove the next F0380 sees non-zero G2153, loads the same command, decrements G2153, then dispatches F0365/F0366
- only then bind the following F0128/F0097 frame to the changed direction/X/Y state

## Reject as non-promotable

- route keylog labels without M528/F0361 key-code value
- F0361 entry stop with no G0432 write and no G2153 increment
- F0380 stop where G2153_i_QueuedCommandsCount is zero or movement-disabled gate bypasses dispatch
- F0128/F0097 frame evidence without the preceding F0380 pop/decrement/dispatch chain

## Inputs

- pass386 predicates: keyboard F0361 hit=True, queue count changed=False, dispatch reached=False
- pass498 status: PASS498_ORIGINAL_CAPTURE_BLOCKER_NARROWED_TO_POST_COMMAND_STATE_DELTA

## Gate

- python3 tools/verify_pass504_dm1_v1_keyboard_buffer_state_delta_blocker.py

Manifest: parity-evidence/verification/pass504_dm1_v1_keyboard_buffer_state_delta_blocker/manifest.json
