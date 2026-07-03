#include "csb_v1_input_command_bridge_pc34_compat.h"

#include <string.h>

/* Source-lock references for this file:
 *   ReDMCSB COMMAND.C:677-684  -- I34E/PC-34 movement keyboard scancode rows
 *                                  (0xAB34..0xAB36 forward/turn left/turn
 *                                  right, 0xAB31..0xAB33 strafe left/back/
 *                                  strafe right, 0xAB32 backward).
 *   ReDMCSB COMMAND.C:729-812  -- F0361_COMMAND_ProcessInput_Keyboard writes
 *                                  the keyboard rows into G2153_i_QueuedCommands.
 *   ReDMCSB COMMAND.C:1709-1813 -- F0361 keyboard dequeue path that dispatches
 *                                  the source-locked movement keys into the
 *                                  shared V1 queue.
 *   ReDMCSB COMMAND.C:2045-2156 -- F0380_COMMAND_ProcessQueue_CPSC dispatcher
 *                                  that the CSB runtime now mirrors.
 *   ReDMCSB CLIKMENU.C:142-174  -- F0365_CLIKMENU_ProcessTurn routes
 *                                  C001/C002 into (party_dir + 3/+1) & 3.
 *   ReDMCSB CHAMPION.C F0284 lines 117-130 -- CHAMPION_SetPartyDirection rotates
 *                                  every champion Cell/Direction and writes
 *                                  G0308_i_PartyDirection.
 *   ReDMCSB DEFS.H:223-226 + 3507-3509 -- C001..C006 command ids and C5/C7
 *                                  queue caps.
 */

const char* CSB_V1_InputCommandBridge_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB COMMAND.C:677-684,729-812,1709-1813,2045-2156 F0380; "
           "CLIKMENU.C:142-174 F0365; "
           "CHAMPION.C F0284 lines 117-130; "
           "DEFS.H:223-226 C001..C006, 3507-3509 C5/C7";
}

int CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
    M12_MenuInput menuInput,
    int x,
    int y,
    struct Dm1V1InputEventPc34Compat* outEvent)
{
    if (!outEvent) {
        return 0;
    }
    memset(outEvent, 0, sizeof(*outEvent));
    outEvent->kind = DM1_V1_INPUT_KIND_KEY;
    outEvent->x = x;
    outEvent->y = y;
    outEvent->buttonMask = 0;

    /* Source-lock: ReDMCSB COMMAND.C:677-684 defines the I34E/PC-34
     * movement-key scancode rows used by F0361.  The Firestaff M12
     * menu input layer has separate TURN_LEFT/TURN_RIGHT and STRAFE_*
     * tokens that already separate the two routes; the bridge maps
     * them to the source-locked scancodes that the shared V1 queue
     * accepts on the keyboard event path. */
    switch (menuInput) {
    case M12_MENU_INPUT_TURN_LEFT:
        outEvent->keyCode = CSB_V1_BRIDGE_PC34_KEY_TURN_L;
        return 1;
    case M12_MENU_INPUT_TURN_RIGHT:
        outEvent->keyCode = CSB_V1_BRIDGE_PC34_KEY_TURN_R;
        return 1;
    case M12_MENU_INPUT_UP:
        outEvent->keyCode = CSB_V1_BRIDGE_PC34_KEY_FORWARD;
        return 1;
    case M12_MENU_INPUT_DOWN:
        outEvent->keyCode = CSB_V1_BRIDGE_PC34_KEY_BACKWARD;
        return 1;
    case M12_MENU_INPUT_STRAFE_LEFT:
        outEvent->keyCode = CSB_V1_BRIDGE_PC34_KEY_LEFT;
        return 1;
    case M12_MENU_INPUT_STRAFE_RIGHT:
        outEvent->keyCode = CSB_V1_BRIDGE_PC34_KEY_RIGHT;
        return 1;
    default:
        return 0;
    }
}

int CSB_V1_InputCommandBridge_EnqueueMenuInputPc34Compat(
    CSB_V1_RuntimeProfile* profile,
    M12_MenuInput menuInput,
    int x,
    int y)
{
    struct Dm1V1InputEventPc34Compat event;
    if (!profile) {
        return 0;
    }
    if (!CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
            menuInput, x, y, &event)) {
        return 0;
    }
    /* Source-lock: ReDMCSB COMMAND.C:729-812 F0361 writes the keyboard
     * row into the shared G2153_i_QueuedCommands ring, capping regular
     * C001..C006 bursts at the source C5 (5) cap of
     * DEFS.H:3507-3509.  The shared V1 input command queue helper
     * mirrors that cap exactly. */
    return DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
        &profile->input_command_queue, event);
}

int CSB_V1_InputCommandBridge_ProcessMenuInputPc34Compat(
    CSB_V1_RuntimeProfile* profile,
    M12_MenuInput menuInput,
    int x,
    int y,
    int disabledMovementTicks,
    int projectileDisabledMovementTicks,
    int lastProjectileDisabledMovementDirection,
    CSB_V1_InputCommandBridgeResult* outResult)
{
    CSB_V1_InputCommandBridgeResult local;
    struct Dm1V1InputEventPc34Compat event;
    int enqueued;
    int dispatched;
    CSB_V1_InputCommandRuntimeResult runtime_result;

    if (!profile || !outResult) {
        return -1;
    }
    memset(&local, 0, sizeof(local));

    if (!CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
            menuInput, x, y, &event)) {
        if (outResult) *outResult = local;
        return 0;
    }
    local.mapped = 1;
    local.event = event;

    /* Source-lock: COMMAND.C:729-812 F0361 caps the regular C001..C006
     * burst at C5 (5).  If the queue is already at the cap, the enqueue
     * is rejected by the shared V1 input command queue helper and the
     * bridge surfaces that as `mapped=1, dispatched=0` so the caller
     * can decide whether to drain. */
    enqueued = DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
        &profile->input_command_queue, event);
    if (!enqueued) {
        if (outResult) *outResult = local;
        return 0;
    }

    /* Source-lock: ReDMCSB COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC
     * locks the queue, checks the empty/movement-disabled gate, dequeues
     * one command, and dispatches turns to F0365 / moves to F0366.  The
     * CSB V1 runtime adapter owns the current bounded live state routes:
     * turns reach CHAMPION.C F0284 and C003..C006 movement reaches the
     * source-locked one-step runtime helper. */
    memset(&runtime_result, 0, sizeof(runtime_result));
    dispatched = csb_v1_runtime_process_input_queue(
        profile,
        &profile->input_command_queue,
        disabledMovementTicks,
        projectileDisabledMovementTicks,
        lastProjectileDisabledMovementDirection,
        &runtime_result);
    if (dispatched < 0) {
        if (outResult) *outResult = local;
        return -1;
    }

    local.runtime_result = runtime_result;
    local.queue_result = runtime_result.queue_result;
    if (local.queue_result.dequeued) {
        int command = local.queue_result.command;
        local.is_turn = (command == DM1_V1_COMMAND_TURN_LEFT ||
                         command == DM1_V1_COMMAND_TURN_RIGHT) ? 1 : 0;
        local.is_forward_move =
            (command == DM1_V1_COMMAND_MOVE_FORWARD) ? 1 : 0;
        local.runtime_state_changed = runtime_result.runtime_state_changed;
    }

    if (outResult) *outResult = local;
    return dispatched;
}
