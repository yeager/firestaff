# DM1 V1 Command Dispatch — Source Audit

## ReDMCSB Source Lock
ReDMCSB WIP20210206, Toolchains/Common/Source/COMMAND.C, CLIKMENU.C, MOVESENS.C, DEFS.H

## 1. Command Dispatch Pipeline

### COMMAND.C:1340 — Queue Initialization
G2153_i_QueuedCommandsCount = 0 at game start / after flush.

### COMMAND.C:1709-1813 — F0361_COMMAND_ProcessKeyPress (Keyboard Path)
F0361_ProcessKeyPress(rawKey):
  if G2153_i_QueuedCommandsCount < C5 (5):
    normalized = normalize_key(rawKey)
    command = lookup_keyboard_row(normalized)
    enqueue(command)
    G2153_i_QueuedCommandsCount++
  F0360_COMMAND_ProcessPendingClick()

Keyboard commands bypass the mouse input hit-test tables entirely;
they go directly into the command queue via COMMAND.C:579-610 primary
and COMMAND.C:636-685 movement keyboard rows.

### COMMAND.C:1452-1661 — F0359_COMMAND_ProcessClick_CPSC (Mouse Path)
F0359_ProcessClick_CPSC(x, y, buttons):
  if queue.locked:
    save pending click (G0436_B_PendingClickPresent, G0437-G0439)
    return
  if G2153_i_QueuedCommandsCount >= maximum:
    save pending click
    return
  command = F0358(x, y, buttons)   // hit-test primary mouse table
  if command == NONE:
    command = F0358(x, y, buttons)  // hit-test secondary mouse table
  enqueue(command, x, y)
  G2153_i_QueuedCommandsCount++

### COMMAND.C:1379-1449 — F0358_COMMAND_GetCommandFromMouseInput_CPSC
Hit matcher: walks mouse rows (G0447 primary, G0448 secondary),
checks button mask, returns matched command ordinal.

### COMMAND.C:2045-2156 — F0380 Dequeue + Dispatch
F0380_DequeueAndDispatch():
  if queue empty: clear pending + return
  command = dequeue()[0]
  if movement blocked:
    re-enqueue pending click + return movementDisabledGate=1
  shift queue left
  G2153_i_QueuedCommandsCount--
  F0360_ProcessPendingClick()    // replay any saved click

  if command is turn (C001/C002):
    F0365_COMMAND_ProcessTypes1To2_TurnParty(command)
  else if command is move (C003-C006):
    F0366_COMMAND_ProcessTypes3To6_MoveParty(command)

## 2. Command Ordinals (DEFS.H)
C001 COMMAND_TURN_LEFT    → F0365
C002 COMMAND_TURN_RIGHT   → F0365
C003 COMMAND_MOVE_FORWARD → F0366
C004 COMMAND_MOVE_RIGHT   → F0366
C005 COMMAND_MOVE_BACKWARD → F0366
C006 COMMAND_MOVE_LEFT    → F0366
C080 COMMAND_CLICK_IN_DUNGEON_VIEW → viewport handler
C083 COMMAND_TOGGLE_INVENTORY_LEADER → panel toggle
C129 COMMAND_RELEASE_CHAMPION_ICON → icon release (survives flush)
C254 COMMAND_STOP_PRESSING_EYE_MOUTH_WALL → stop action

## 3. Movement Gate
COMMAND.C:2045-2056 — F0380 gate check:
  movementDisabledGate =
    disabledMovementTicks > 0 ||
    (projectileDisabledMovementTicks &&
     lastProjectileDisabledDirection == normalize(partyDir + command - C003))
When set, the command is NOT dispatched; queue left intact.

## 4. Firestaff Implementation

dm1_v1_input_command_queue_pc34_compat.c:
- DM1_V1_InputCommandQueue_ProcessOnePc34Compat mirrors F0380 exactly:
  gate check, dequeue, shift, dispatch flags (dispatchedTurn/dispatchedMove),
  movementDisabledGate, pendingReplayCount
- command_for_key: two-pass lookup — primary rows (COMMAND.C:579-610),
  then movement rows (COMMAND.C:636-685)
- command_for_mouse: primary table → C129/C254 special ordinals → secondary
- Reserved release commands survive DiscardAllInput (COMMAND.C:1304-1377):
  only C129 and C254 preserved during flush

## 5. Gaps
None identified. Dispatch pipeline (F0361→queue→F0380→F0365/F0366)
fully implemented with correct queue limits, pending click mechanism,
movement gate, and reserved command preservation.

## 6. Verdict
SOURCE-LOCKED. ProcessOne correctly mirrors F0380: gate check, turn vs move
dispatch split, pending click replay, queue shift, and count decrement
are all traceable to COMMAND.C lines.
