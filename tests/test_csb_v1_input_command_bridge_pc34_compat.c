/*
 * test_csb_v1_input_command_bridge_pc34_compat.c
 *
 * Startup-adjacent CSB V1 input-command bridge gate.
 *
 * Proves that one M12 menu input reaches the CSB V1 runtime state
 * transition path documented in include/csb_v1_runtime_pc34_compat.h.
 * The bridge translates a Firestaff M12 menu input (e.g.
 * M12_MENU_INPUT_TURN_RIGHT) into the PC-34 / I34E source-locked
 * input-command event that the shared DM1/CSB V1 input command
 * queue + F0380_COMMAND_ProcessQueue_CPSC dispatcher would have
 * written during real PC-34 gameplay, and the CSB V1 runtime
 * applies the source-locked F0284 direction rotation to the
 * party snapshot.
 *
 * This test does not claim full CSB playability: it covers exactly
 * the startup-adjacent "menu input -> input command queue -> CSB
 * runtime state transition" boundary, plus the M12 mapping contract,
 * the queue saturation/overflow behaviour, the M12 menu input
 * rejection (e.g. ACCEPT, FREEZE_TOGGLE), and the source-evidence
 * citation.
 *
 * Source-lock:
 *   ReDMCSB COMMAND.C:677-684 I34E/PC-34 movement keyboard scancode rows
 *     (0xAB34..0xAB36 forward/turn left/turn right, 0xAB31..0xAB33
 *     strafe left/back/strafe right, 0xAB32 backward).
 *   ReDMCSB COMMAND.C:729-812 F0361_COMMAND_ProcessInput_Keyboard writes
 *     the keyboard row into the shared G2153_i_QueuedCommands ring
 *     capped at the source C5 (5) cap of DEFS.H:3507-3509.
 *   ReDMCSB COMMAND.C:1709-1813 F0361 keyboard dequeue path.
 *   ReDMCSB COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC
 *     dispatcher (lock, empty/movement-disabled gate, dequeue, dispatch).
 *   ReDMCSB CLIKMENU.C:142-174 F0365_CLIKMENU_ProcessTurn routes
 *     C001/C002 into (party_dir + 3/+1) & 3.
 *   ReDMCSB CHAMPION.C F0284 lines 117-130 CHAMPION_SetPartyDirection
 *     rotates every champion Cell/Direction and writes
 *     G0308_i_PartyDirection.
 *   ReDMCSB DEFS.H:223-226 C001..C006, DEFS.H:3507-3509 C5/C7.
 */

#include "csb_v1_input_command_bridge_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, msg) do { \
    int got_value = (int)(got); \
    int want_value = (int)(want); \
    if (got_value == want_value) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s got=%d want=%d\n", msg, got_value, want_value); } \
} while (0)

#define CHECK_NULL(call, msg) do { \
    int _rc = (call); \
    if (_rc == 0) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s got=%d want=0\n", msg, _rc); } \
} while (0)

static void make_party(CSB_V1_PartyState *party)
{
    int i;
    csb_v1_character_init_default(party);
    party->ChampionCount = 2;
    party->PartyDirection = CSB_V1_DIR_NORTH;
    party->LeaderIndex = 0;
    party->MagicCasterIndex = -1;
    party->PartyMapX = CSB_V1_START_PARTY_X;
    party->PartyMapY = CSB_V1_START_PARTY_Y;
    for (i = 0; i < party->ChampionCount; i++) {
        party->Champions[i].CurrentHealth = 100;
        party->Champions[i].MaximumHealth = 100;
        party->Champions[i].Cell = (uint8_t)i;
        party->Champions[i].Direction = CSB_V1_DIR_NORTH;
    }
}

static void init_runtime_with_party(CSB_V1_RuntimeProfile *profile)
{
    CSB_V1_PartyState party;
    csb_v1_runtime_init(profile, NULL);
    make_party(&party);
    (void)csb_v1_runtime_set_party_state(profile, &party);
}

static void test_source_evidence(void)
{
    const char *evidence = CSB_V1_InputCommandBridge_SourceEvidencePc34Compat();
    CHECK(evidence != NULL, "bridge source evidence string is present");
    CHECK(strstr(evidence, "COMMAND.C:677-684") != NULL,
          "source evidence cites I34E/PC-34 movement scancode rows");
    CHECK(strstr(evidence, "F0380") != NULL,
          "source evidence cites F0380 dispatcher");
    CHECK(strstr(evidence, "CHAMPION.C F0284") != NULL,
          "source evidence cites F0284 direction rotation");
}

static void test_menu_input_to_event_mapping(void)
{
    struct Dm1V1InputEventPc34Compat event;
    /* Source-lock: ReDMCSB COMMAND.C:677-684 maps the I34E/PC-34
     * movement-key scancodes; the Firestaff M12 menu input layer has
     * separate TURN_LEFT/TURN_RIGHT and STRAFE_* tokens. */
    memset(&event, 0xff, sizeof(event));
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_TURN_LEFT, 0, 0, &event),
             1, "TURN_LEFT maps to a CSB-relevant event");
    CHECK_EQ(event.kind, DM1_V1_INPUT_KIND_KEY,
             "TURN_LEFT event is a keyboard event");
    CHECK_EQ(event.keyCode, CSB_V1_BRIDGE_PC34_KEY_TURN_L,
             "TURN_LEFT event uses the source-locked 0xAB34 scancode");

    memset(&event, 0, sizeof(event));
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_TURN_RIGHT, 1, 2, &event),
             1, "TURN_RIGHT maps to a CSB-relevant event");
    CHECK_EQ(event.keyCode, CSB_V1_BRIDGE_PC34_KEY_TURN_R,
             "TURN_RIGHT event uses the source-locked 0xAB36 scancode");
    CHECK_EQ(event.x, 1, "TURN_RIGHT event preserves x");
    CHECK_EQ(event.y, 2, "TURN_RIGHT event preserves y");

    memset(&event, 0, sizeof(event));
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_UP, 0, 0, &event),
             1, "UP maps to a CSB-relevant forward event");
    CHECK_EQ(event.keyCode, CSB_V1_BRIDGE_PC34_KEY_FORWARD,
             "UP event uses the source-locked 0xAB35 forward scancode");

    memset(&event, 0, sizeof(event));
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_DOWN, 0, 0, &event),
             1, "DOWN maps to a CSB-relevant backward event");
    CHECK_EQ(event.keyCode, CSB_V1_BRIDGE_PC34_KEY_BACKWARD,
             "DOWN event uses the source-locked 0xAB32 backward scancode");

    memset(&event, 0, sizeof(event));
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_STRAFE_LEFT, 0, 0, &event),
             1, "STRAFE_LEFT maps to a CSB-relevant event");
    CHECK_EQ(event.keyCode, CSB_V1_BRIDGE_PC34_KEY_LEFT,
             "STRAFE_LEFT event uses the source-locked 0xAB31 scancode");

    memset(&event, 0, sizeof(event));
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_STRAFE_RIGHT, 0, 0, &event),
             1, "STRAFE_RIGHT maps to a CSB-relevant event");
    CHECK_EQ(event.keyCode, CSB_V1_BRIDGE_PC34_KEY_RIGHT,
             "STRAFE_RIGHT event uses the source-locked 0xAB33 scancode");
}

static void test_unmapped_menu_inputs(void)
{
    struct Dm1V1InputEventPc34Compat event;
    /* The bridge is intentionally narrow: it only owns the CSB
     * movement/turn/forward direction set.  Other M12 menu inputs
     * must fall through to the regular M11 dispatch path. */
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_ACCEPT, 0, 0, &event),
             0, "ACCEPT is not a CSB bridge command");
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_BACK, 0, 0, &event),
             0, "BACK is not a CSB bridge command");
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_FREEZE_TOGGLE, 0, 0, &event),
             0, "FREEZE_TOGGLE is not a CSB bridge command");
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_NONE, 0, 0, &event),
             0, "NONE is not a CSB bridge command");
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_CYCLE_CHAMPION, 0, 0, &event),
             0, "CYCLE_CHAMPION is not a CSB bridge command");
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_DISK_MENU, 0, 0, &event),
             0, "DISK_MENU is not a CSB bridge command");
    CHECK_EQ(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                 M12_MENU_INPUT_CHAMPION_1_INVENTORY, 0, 0, &event),
             0, "CHAMPION_1_INVENTORY is not a CSB bridge command");

    CHECK_NULL(CSB_V1_InputCommandBridge_MenuInputToEventPc34Compat(
                   M12_MENU_INPUT_NONE, 0, 0, NULL),
               "NULL outEvent is rejected without dereference");
}

static void test_turn_right_reaches_csb_runtime_state(void)
{
    /* The headline startup-adjacent CSB input command gate:
     * a single M12_MENU_INPUT_TURN_RIGHT reaches the CSB runtime
     * party state via the bridge, the shared V1 input command
     * queue, and the source-locked F0284 direction rotation. */
    CSB_V1_RuntimeProfile profile;
    CSB_V1_InputCommandBridgeResult result;
    int rc;

    init_runtime_with_party(&profile);
    memset(&result, 0, sizeof(result));

    /* ReDMCSB COMMAND.C:677-684 maps the TURN_RIGHT scancode to
     * C002_COMMAND_TURN_RIGHT; COMMAND.C:2045-2156 F0380 dispatches
     * C002 to F0365_CLIKMENU_ProcessTurn which delegates the rotation
     * to CHAMPION.C F0284 lines 117-130. */
    rc = CSB_V1_InputCommandBridge_ProcessMenuInputPc34Compat(
        &profile,
        M12_MENU_INPUT_TURN_RIGHT,
        0, 0,
        0, 0, 0,
        &result);
    CHECK_EQ(rc, 1, "TURN_RIGHT bridge returns one dequeued command");
    CHECK_EQ(result.mapped, 1, "TURN_RIGHT bridge marks the input as mapped");
    CHECK_EQ(result.event.keyCode, CSB_V1_BRIDGE_PC34_KEY_TURN_R,
             "TURN_RIGHT bridge feeds the source-locked 0xAB36 scancode");
    CHECK_EQ(result.is_turn, 1,
             "TURN_RIGHT bridge reports the dispatch was a turn command");
    CHECK_EQ(result.runtime_state_changed, 1,
             "TURN_RIGHT bridge reports a CSB runtime state mutation");
    CHECK_EQ(result.queue_result.command, DM1_V1_COMMAND_TURN_RIGHT,
             "dequeued command is C002 turn-right");
    CHECK_EQ(result.queue_result.dispatchedTurn, 1,
             "shared V1 queue marks the command as a turn dispatch");
    CHECK_EQ(result.queue_result.dequeued, 1,
             "shared V1 queue reports the command dequeued");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_EAST,
             "CSB runtime party_dir reaches the intended state (north->east)");
    CHECK_EQ(profile.party_state.PartyDirection, CSB_V1_DIR_EAST,
             "CSB party snapshot direction reaches the intended state");
    CHECK_EQ(profile.party_state.Champions[0].Cell, 1,
             "F0284 cell delta applies to champion 0");
    CHECK_EQ(profile.party_state.Champions[0].Direction, CSB_V1_DIR_EAST,
             "F0284 direction delta applies to champion 0");
    CHECK_EQ(profile.party_state.Champions[1].Cell, 2,
             "F0284 cell delta applies to champion 1");
    CHECK_EQ(profile.party_state.Champions[1].Direction, CSB_V1_DIR_EAST,
             "F0284 direction delta applies to champion 1");
    CHECK_EQ(profile.input_command_queue.count, 0u,
             "input command queue is empty after the consumed turn");
    CHECK_EQ(profile.input_dispatch_count, 1u,
             "runtime dispatch count advances for the turn");
}

static void test_turn_left_round_trip(void)
{
    /* After the previous test, party_dir is east (1).  TURN_LEFT
     * back to north completes the round-trip and proves the bridge
     * keeps the F0284 invariant intact across two consecutive
     * dispatches. */
    CSB_V1_RuntimeProfile profile;
    CSB_V1_InputCommandBridgeResult result;

    init_runtime_with_party(&profile);
    memset(&result, 0, sizeof(result));

    CHECK_EQ(CSB_V1_InputCommandBridge_ProcessMenuInputPc34Compat(
                 &profile, M12_MENU_INPUT_TURN_RIGHT, 0, 0, 0, 0, 0, &result),
             1, "first TURN_RIGHT bridge dispatches");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_EAST, "party_dir is east after first turn");

    memset(&result, 0, sizeof(result));
    CHECK_EQ(CSB_V1_InputCommandBridge_ProcessMenuInputPc34Compat(
                 &profile, M12_MENU_INPUT_TURN_LEFT, 0, 0, 0, 0, 0, &result),
             1, "TURN_LEFT bridge dispatches");
    CHECK_EQ(result.is_turn, 1, "TURN_LEFT reports a turn dispatch");
    CHECK_EQ(result.queue_result.command, DM1_V1_COMMAND_TURN_LEFT,
             "dequeued command is C001 turn-left");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_NORTH,
             "party_dir returns to north after the TURN_LEFT round-trip");
    CHECK_EQ(profile.party_state.Champions[0].Cell, 0,
             "F0284 cell delta returns champion 0 to its starting cell");
    CHECK_EQ(profile.party_state.Champions[0].Direction, CSB_V1_DIR_NORTH,
             "F0284 direction delta returns champion 0 to its starting direction");
    CHECK_EQ(profile.input_dispatch_count, 2u,
             "runtime dispatch count advances for the second turn");
}

static void test_forward_move_queued_but_not_applied(void)
{
    /* The bridge is intentionally narrow: forward/up movement is
     * source-locked to a C003_COMMAND_MOVE_FORWARD event that the
     * shared V1 input command queue accepts, but the CSB V1 runtime
     * does not yet apply a step to runtime state in this build
     * (movement-disabled gate, dungeon-aware stepping, and
     * square/sensor materialization are tracked separately).  The
     * bridge still drains the queue so callers see a dequeued
     * command, and `is_forward_move=1` lets the caller distinguish
     * the dispatch from a turn. */
    CSB_V1_RuntimeProfile profile;
    CSB_V1_InputCommandBridgeResult result;

    init_runtime_with_party(&profile);
    memset(&result, 0, sizeof(result));
    CHECK_EQ(CSB_V1_InputCommandBridge_ProcessMenuInputPc34Compat(
                 &profile, M12_MENU_INPUT_UP, 0, 0, 0, 0, 0, &result),
             1, "UP bridge dispatches a forward command");
    CHECK_EQ(result.is_forward_move, 1,
             "UP bridge reports the dispatch was a forward move");
    CHECK_EQ(result.is_turn, 0,
             "UP bridge does not report a turn dispatch");
    CHECK_EQ(result.queue_result.command, DM1_V1_COMMAND_MOVE_FORWARD,
             "dequeued command is C003 move-forward");
    CHECK_EQ(result.runtime_state_changed, 0,
             "UP bridge does not claim movement (party_dir/x/y unchanged)");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_NORTH,
             "UP bridge leaves party_dir unchanged");
    CHECK_EQ(profile.party_x, CSB_V1_START_PARTY_X,
             "UP bridge leaves party_x unchanged");
    CHECK_EQ(profile.party_y, CSB_V1_START_PARTY_Y,
             "UP bridge leaves party_y unchanged");
    CHECK_EQ(profile.input_command_queue.count, 0u,
             "input command queue is empty after the forward dispatch");
}

static void test_unmapped_menu_input_returns_zero(void)
{
    /* An unmapped menu input (e.g. ACCEPT) is intentionally not a
     * CSB bridge command: the bridge returns 0 with mapped=0 and
     * leaves the runtime untouched. */
    CSB_V1_RuntimeProfile profile;
    CSB_V1_InputCommandBridgeResult result;
    int rc;

    init_runtime_with_party(&profile);
    memset(&result, 0xff, sizeof(result));
    rc = CSB_V1_InputCommandBridge_ProcessMenuInputPc34Compat(
        &profile, M12_MENU_INPUT_ACCEPT, 0, 0, 0, 0, 0, &result);
    CHECK_EQ(rc, 0, "ACCEPT is not a CSB bridge command (returns 0)");
    CHECK_EQ(result.mapped, 0, "ACCEPT bridge result is unmapped");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_NORTH,
             "ACCEPT bridge leaves CSB runtime party_dir unchanged");
    CHECK_EQ(profile.input_command_queue.count, 0u,
             "ACCEPT bridge does not enqueue anything");
    CHECK_EQ(profile.input_dispatch_count, 0u,
             "ACCEPT bridge does not advance the dispatch counter");
}

static void test_null_arguments_return_error(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_InputCommandBridgeResult result;
    init_runtime_with_party(&profile);

    CHECK_EQ(CSB_V1_InputCommandBridge_ProcessMenuInputPc34Compat(
                 NULL, M12_MENU_INPUT_TURN_RIGHT, 0, 0, 0, 0, 0, &result),
             -1, "NULL profile is rejected");
    CHECK_EQ(CSB_V1_InputCommandBridge_ProcessMenuInputPc34Compat(
                 &profile, M12_MENU_INPUT_TURN_RIGHT, 0, 0, 0, 0, 0, NULL),
             -1, "NULL outResult is rejected");
    CHECK_EQ(CSB_V1_InputCommandBridge_EnqueueMenuInputPc34Compat(
                 NULL, M12_MENU_INPUT_TURN_RIGHT, 0, 0),
             0, "NULL profile in enqueue helper is rejected");
}

int main(void)
{
    printf("=== CSB V1 Input Command Bridge Gate (startup-adjacent) ===\n\n");
    test_source_evidence();
    test_menu_input_to_event_mapping();
    test_unmapped_menu_inputs();
    test_turn_right_reaches_csb_runtime_state();
    test_turn_left_round_trip();
    test_forward_move_queued_but_not_applied();
    test_unmapped_menu_input_returns_zero();
    test_null_arguments_return_error();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: one M12 menu input reaches the CSB V1 runtime command queue + F0284 party state");
        puts("sourceEvidence=ReDMCSB COMMAND.C:677-684,729-812,1709-1813,2045-2156 F0380; CLIKMENU.C:142-174 F0365; CHAMPION.C F0284 lines 117-130; DEFS.H:223-226 C001..C006, 3507-3509 C5/C7");
    }
    return failed == 0 ? 0 : 1;
}
