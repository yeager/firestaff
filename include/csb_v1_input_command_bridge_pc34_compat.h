#ifndef CSB_V1_INPUT_COMMAND_BRIDGE_PC34_COMPAT_H
#define CSB_V1_INPUT_COMMAND_BRIDGE_PC34_COMPAT_H

/*
 * csb_v1_input_command_bridge_pc34_compat.h
 *
 * Source-locked CSB V1 startup/input-command bridge.
 *
 * Translates a Firestaff M12 menu input (e.g. M12_MENU_INPUT_TURN_RIGHT)
 * into the PC-34 / I34E source-locked input-command event that the
 * shared DM1/CSB V1 input command queue + F0380_COMMAND_ProcessQueue_CPSC
 * dispatcher would have written during real PC-34 gameplay.  The bridge
 * feeds `csb_v1_runtime_t` so a single menu input reaches the CSB V1
 * runtime state transition path documented in
 * include/csb_v1_runtime_pc34_compat.h.
 *
 * Source-lock:
 *   ReDMCSB COMMAND.C:677-684 / 1709-1813 — I34E/PC-34 movement-key
 *     scancode rows: 0xAB34 / 0xAB35 / 0xAB36 / 0xAB31 / 0xAB32 / 0xAB33.
 *   ReDMCSB COMMAND.C:2045-2156 F0380 — locks the queue, dequeues one
 *     command, dispatches turns to F0365_CLIKMENU_ProcessTurn.
 *   ReDMCSB CLIKMENU.C F0366 lines 224-351 — applies one movement
 *     coordinate step for C003..C006 commands.
 *   ReDMCSB CHAMPION.C F0284 lines 117-130 — applies the (target_dir -
 *     party_dir) delta to every champion Cell/Direction then writes
 *     G0308_i_PartyDirection.
 *
 * This is intentionally a startup-adjacent, data-free bridge.  It does
 * not claim full CSB playability, full mouse support, or full movement
 * semantics; it proves the menu input reaches the runtime command queue,
 * TURN_LEFT/TURN_RIGHT reaches the CSB runtime party state, and C003..C006
 * movement can apply one bounded dungeon-aware step.  Broader playability
 * remains an active TODO and is intentionally out of scope here.
 */

#include "csb_v1_runtime_pc34_compat.h"
#include "menu_startup_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PC-34 / I34E movement keyboard scancodes.  Mirrors the canonical
 * rows in include/dm1_v1_input_command_queue_pc34_compat.c.  Re-exposing
 * the values here so the bridge header stands on its own when callers
 * do not want to depend on the shared V1 queue header. */
enum {
    CSB_V1_BRIDGE_PC34_KEY_FORWARD  = 0xAB35,
    CSB_V1_BRIDGE_PC34_KEY_BACKWARD = 0xAB32,
    CSB_V1_BRIDGE_PC34_KEY_LEFT     = 0xAB31,
    CSB_V1_BRIDGE_PC34_KEY_RIGHT    = 0xAB33,
    CSB_V1_BRIDGE_PC34_KEY_TURN_L   = 0xAB34,
    CSB_V1_BRIDGE_PC34_KEY_TURN_R   = 0xAB36
};

/* Result of a single menu-input bridge call.  `mapped` reports the
 * raw source-locked event that the bridge fed into the queue, so
 * callers can inspect what was actually enqueued without reaching
 * into csb_v1_runtime_t internals.  `dispatch` is the runtime
 * dispatch result (forwarded from
 * csb_v1_runtime_process_input_queue via the input command queue helper).
 * `runtime_state_changed` mirrors the runtime result's flag and is
 * convenient for callers that just want a boolean. */
typedef struct {
    int mapped;                       /* 1 if the menu input was a recognised
                                       * CSB movement/turn/forward direction. */
    int is_turn;                      /* 1 if the dispatch was a turn command. */
    int is_forward_move;              /* 1 if the dispatch was a move-forward
                                       * command. */
    struct Dm1V1InputEventPc34Compat event;
    struct Dm1V1InputQueueProcessResultPc34Compat queue_result;
    CSB_V1_InputCommandRuntimeResult runtime_result;
    int runtime_state_changed;
} CSB_V1_InputCommandBridgeResult;

/* Translate a Firestaff M12 menu input into a PC-34 source-locked
 * input event that the shared DM1/CSB V1 input command queue will
 * accept.  Returns 1 if a CSB-relevant event was produced (turn,
 * forward, strafe, backward, side-step), 0 if the menu input does
 * not map to a CSB movement/turn command and the caller should
 * fall back to the regular M11 dispatch path.  The translation is
 * intentionally narrow: it does not try to be a full M12->V1 map
 * for spell runes, action areas, or any non-movement CSB input. */
int CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
    M12_MenuInput menuInput,
    int x,
    int y,
    struct Dm1V1InputEventPc34Compat* outEvent);

/* Enqueue a single menu input into the CSB V1 runtime's input command
 * queue.  Returns 1 on a successful enqueue, 0 if the input is not
 * a CSB movement/turn command or the queue is full (the shared V1
 * queue caps regular C001..C006 bursts at the source C5 cap of 5).
 * The runtime profile's `input_command_queue` is updated in place. */
int CSB_V1_InputCommandBridge_EnqueueMenuInputPc34Compat(
    CSB_V1_RuntimeProfile* profile,
    M12_MenuInput menuInput,
    int x,
    int y);

/* Enqueue a menu input and then drain at most one queued command,
 * applying it to the CSB V1 runtime.  This is the startup-adjacent
 * "input command reaches runtime state transition" gate: the bridge
 * receives a M12 menu input, writes the PC-34 source-locked event
 * into the input command queue, and after dequeue the runtime
 * profile reflects the source-locked F0284 rotation for turns or the
 * bounded one-step F0366 movement route for movement commands.  Returns 1 if
 * a command was
 * consumed, 0 if the queue stayed empty (no recognisable menu
 * input or queue was already empty after enqueue), -1 on a hard
 * error (NULL profile, NULL outResult, unsupported menu input, or
 * a runtime dispatch failure).  The function does not claim full movement:
 * sensors, stairs, teleporters, doors, and other movement consequences remain
 * outside this bridge. */
int CSB_V1_InputCommandBridge_ProcessMenuInputPc34Compat(
    CSB_V1_RuntimeProfile* profile,
    M12_MenuInput menuInput,
    int x,
    int y,
    int disabledMovementTicks,
    int projectileDisabledMovementTicks,
    int lastProjectileDisabledMovementDirection,
    CSB_V1_InputCommandBridgeResult* outResult);

/* Source-evidence citation string for the bridge.  Matches the
 * citation style used by sibling CSB V1 source-locked modules so
 * audit scripts can grep for it. */
const char* CSB_V1_InputCommandBridge_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
