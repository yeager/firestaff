# Pass495 — DM1 V1 F0365/F0366 runtime/static boundary

Status: `PASS495_F0365_F0366_RUNTIME_STOPS_PROVEN_ORIGINAL_STATIC_CAPTURE_STILL_BLOCKED`

## Decision
Do not spend more Firestaff-route work on F0365/F0366; that seam is closed. The active blocker is original capture promotion: pass487/pass494 still classify the fresh click capture as entrance-first plus repeated static/no-state-delta post-entry frames, not source-visible movement state evidence.

## ReDMCSB source audit
- `GAMELOOP.C:150-219` / `F0002_MAIN_GameLoop_CPSDF` — ok=True; the game loop drains keyboard input, runs F0380, and exits the wait loop only after stop/tick state permits the next draw
- `COMMAND.C:636-685` / `G0459_as_Graphic561_SecondaryKeyboardInput_Movement` — ok=True; PC34/I34E movement keys resolve to C001/C002 turn and C003..C006 movement command ids
- `COMMAND.C:1709-1813` / `F0361_COMMAND_ProcessKeyPress` — ok=True; keyboard input is table-resolved into G0432_as_CommandQueue before F0380 consumes it
- `COMMAND.C:2045-2156` / `F0380_COMMAND_ProcessQueue_CPSC` — ok=True; F0380 gates movement cooldown, pop-loads one queued command, then dispatches turns to F0365 and steps to F0366
- `CLIKMENU.C:142-174` / `F0365_COMMAND_ProcessTypes1To2_TurnParty` — ok=True; accepted turns set stop-wait and mutate party direction through the source turn handler
- `CLIKMENU.C:180-347` / `F0366_COMMAND_ProcessTypes3To6_MoveParty` — ok=True; accepted steps compute a relative destination, commit through F0267, and set movement cooldown; blocked steps discard input before the commit path

## Runtime stop evidence
- pass475 Firestaff route closure: `True` (`PASS475_FIRESTAFF_DM1_V1_INPUT_TO_F0365_F0366_ROUTE_CLOSED`)
- pass391 post-gameplay queue-to-dispatch stop chain: `True` (`PASS391_KEYBOARD_QUEUE_TO_F0380_DISPATCH_PROVEN`)
- pass391 required predicates: `postGameplayRuntimeRan, f0361HitAfterArm, g2153IncrementObserved, f0380PopLoadAfterQueueWriteObserved, g2153DecrementPopLoadObserved, f0365OrF0366DispatchObserved, sourceAuditOk`
- diagnostic F0380 entry breakpoint observed after queue write: `False`; pop/load/decrement/dispatch predicates are the promoted runtime evidence.

## Static duplicate evidence from pass494/pass487
- pass487 classifier blocker retained: `True` (`PASS487_ORIGINAL_CLICK_ROUTE_REACHES_GAMEPLAY_STILL_LABEL_BLOCKED`)
- first frame still entrance/menu: `True`
- post-entry gameplay hash repeated: `True`
- true-stop classification: `static_no_state_delta_after_entrance_not_movement_processor_stop`

## Next unblocker
Capture or trace a fresh original run where the labeled post-entry frames are non-duplicate and source-bound to F0380 pop/load plus F0365/F0366 dispatch/stop evidence, then rerun pass80/pass487-family promotion gates.

## Non-claims
- no original-vs-Firestaff pixel parity promotion
- no claim that pass487 static post-entry frames prove movement processor stops
- no new DOSBox/FIRES runtime capture was performed by this pass
