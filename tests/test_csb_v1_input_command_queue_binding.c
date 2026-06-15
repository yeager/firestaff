/*
 * test_csb_v1_input_command_queue_binding.c
 *
 * Focused CSB V1 input-command runtime binding gate.  This proves one
 * command boundary reaches CSB runtime state: a queued C002 turn-right
 * command is dequeued through the shared V1 queue and applied to the CSB
 * party snapshot via the source-locked F0284 direction rotation.  It does
 * not claim movement, inventory, action-area, or broader playability.
 *
 * Source-lock:
 *   ReDMCSB COMMAND.C F0380 lines 2045-2156 dispatches C001/C002 to
 *   F0365_COMMAND_ProcessTypes1To2_TurnParty.
 *   ReDMCSB CHAMPION.C F0284 lines 117-130 applies the turn delta to every
 *   champion Cell and Direction before writing G0308_i_PartyDirection.
 */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
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

static void test_turn_right_command_reaches_csb_runtime_state(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    csb_v1_runtime_init(&profile, NULL);
    make_party(&party);
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "fixture party snapshot enters the CSB runtime profile");

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
              &queue,
              (struct Dm1V1InputEventPc34Compat){
                  DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }) == 1,
          "PC-34 right-turn key queues one V1 command");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "CSB runtime consumes one queued input command");
    CHECK(result.queue_result.command == DM1_V1_COMMAND_TURN_RIGHT,
          "dequeued command is C002 turn-right");
    CHECK(result.queue_result.dispatchedTurn == 1,
          "shared V1 queue marks the command as a turn dispatch");
    CHECK(result.unsupported_runtime_command == 0,
          "turn-right is a supported CSB runtime binding");
    CHECK(result.runtime_state_changed == 1,
          "CSB runtime reports a party state mutation");
    CHECK(result.old_party_dir == CSB_V1_DIR_NORTH &&
          result.new_party_dir == CSB_V1_DIR_EAST,
          "runtime direction changes north to east");
    CHECK(profile.party_dir == CSB_V1_DIR_EAST,
          "CSB runtime party_dir reaches the intended state");
    CHECK(profile.party_state.PartyDirection == CSB_V1_DIR_EAST,
          "CSB party snapshot direction reaches the intended state");
    CHECK(profile.party_state.Champions[0].Cell == 1 &&
          profile.party_state.Champions[1].Cell == 2,
          "F0284 cell delta applies to every loaded champion");
    CHECK(profile.party_state.Champions[0].Direction == CSB_V1_DIR_EAST &&
          profile.party_state.Champions[1].Direction == CSB_V1_DIR_EAST,
          "F0284 direction delta applies to every loaded champion");
    CHECK(profile.party_x == CSB_V1_START_PARTY_X &&
          profile.party_y == CSB_V1_START_PARTY_Y,
          "turn binding does not claim movement or change map coordinates");
    CHECK(queue.count == 0u,
          "input queue is empty after the consumed command");
}

static void test_movement_gate_keeps_command_queued(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;

    csb_v1_runtime_init(&profile, NULL);
    make_party(&party);
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "movement-gate fixture party enters runtime profile");

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
              &queue,
              (struct Dm1V1InputEventPc34Compat){
                  DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }) == 1,
          "PC-34 forward key queues one V1 movement command");

    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 1, 0, 0, &result) == 0,
          "CSB runtime leaves movement queued while movement is disabled");
    CHECK(result.queue_result.movementDisabledGate == 1,
          "shared V1 queue reports the movement-disabled gate");
    CHECK(result.queue_result.dequeued == 0,
          "movement command is not dequeued behind the gate");
    CHECK(result.runtime_state_changed == 0,
          "movement gate does not mutate CSB runtime state");
    CHECK(profile.party_dir == CSB_V1_DIR_NORTH,
          "movement gate leaves party direction unchanged");
    CHECK(queue.count == 1u,
          "movement command remains queued for a later runtime boundary");
}

int main(void)
{
    printf("=== CSB V1 Input Command Queue Binding Gate ===\n\n");
    test_turn_right_command_reaches_csb_runtime_state();
    test_movement_gate_keeps_command_queued();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: one queued CSB V1 turn command reaches CSB runtime party state without claiming movement or full playability");
        puts("sourceEvidence=ReDMCSB COMMAND.C F0380 lines 2045-2156; CHAMPION.C F0284 lines 117-130");
    }
    return failed == 0 ? 0 : 1;
}
