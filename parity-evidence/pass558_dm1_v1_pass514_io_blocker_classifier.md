# Pass558 - DM1 V1 pass514/pass556 IO blocker classifier

Status: BLOCKED_PASS558_DOSBOX_EVENT_TO_GUEST_IO2_DISPATCH_BOUNDARY_CLASSIFIED

## Decision

Classified current DM1 V1 original capture unblocker as DOSBox window/event-queue to guest keyboard interrupt/IO2 dispatch boundary: ReDMCSB requires GAMELOOP M527/M528 to call IO2 presence/read before F0361 can enqueue, pass514 reaches F0380 with no enqueue/count write, pass556 resolves the IO2 driver slots but observes no IO2 read/presence after route input, and pass557 rules out host route-key generation as the first failing boundary.

## ReDMCSB source anchors

- GAMELOOP.C:166-215 game_loop_presence_read_before_f0380
- DEFS.H:3143-4335 defs_i34e_keyboard_macros_to_io2
- IO2.C:5-184 io2_presence_and_read_driver_slots
- IBMIO.C:358-2550 ibmio_keyboard_irq_buffer_and_presence
- COMMAND.C:1357-1765 f0361_enqueue_target
- COMMAND.C:2045-2155 f0380_empty_or_pop_dispatch

## Evidence chain

- pass514 status: BLOCKED_PASS514_KEYBOARD_INPUT_DELIVERED_BUT_NO_F0361_ENQUEUE_BEFORE_EMPTY_F0380
- pass550 status: BLOCKED_PASS514_KEYBOARD_INPUT_DELIVERED_BUT_NO_F0361_ENQUEUE_BEFORE_EMPTY_F0380
- pass556 status: BLOCKED_PASS556_MOVEMENT_KEY_NEVER_REACHES_IO2
- pass557 status: BLOCKED_PASS557_HOST_ROUTE_GENERATION_PROVEN_NEXT_BOUNDARY_UNRESOLVED

## Evidence

- Manifest: parity-evidence/verification/pass558_dm1_v1_pass514_io_blocker_classifier/manifest.json
