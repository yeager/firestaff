/*
 * test_csb_v1_runtime_tick_accumulator.c
 *
 * Narrow CSB V1 post-handoff runtime tick regression.  This does not claim
 * broad CSB playability; it proves the runtime clock boundary can advance
 * source-locked 55ms game-time quanta from accumulated frame deltas.
 *
 * Source-lock:
 *   ReDMCSB TIMELINE.C F0235 lines 702-708 compares event time with
 *   G0313_ul_GameTime.
 *   ReDMCSB COMMAND.C F0380 lines 2383-2429 toggles
 *   G0301_B_GameTimeTicking for freeze/unfreeze commands.
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

static void test_subquantum_frame_slices_fire_one_tick(void)
{
    CSB_V1_RuntimeProfile profile;

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;

    csb_v1_runtime_tick(&profile, 16U);
    csb_v1_runtime_tick(&profile, 16U);
    CHECK(profile.total_play_ms == 32U,
          "two subquantum frame slices accumulate wall time");
    CHECK(profile.tick_count == 0U,
          "no V1 tick fires before the 55ms quantum");
    CHECK(csb_v1_runtime_tick_due(&profile, 0U) == 0,
          "tick_due reports false below the 55ms boundary");

    csb_v1_runtime_tick(&profile, 23U);
    CHECK(profile.total_play_ms == 55U,
          "16+16+23ms reaches one exact V1 quantum");
    CHECK(profile.tick_count == 1U,
          "one V1 tick fires from accumulated subquantum slices");
    CHECK(profile.game_time == 1U,
          "game_time advances once with the fired V1 tick");
    CHECK(profile.game_ticks == CSB_V1_TICK_MS_NOMINAL,
          "game_ticks records one nominal 55ms tick");
    CHECK(profile.chaos_magic.spell_grid_version == 1U,
          "chaos spell grid advances with the fired tick");
    CHECK(csb_v1_runtime_tick_due(&profile, 0U) == 0,
          "tick_due is false immediately after the due tick is consumed");
}

static void test_multi_quantum_tick_and_due_probe(void)
{
    CSB_V1_RuntimeProfile profile;

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;

    csb_v1_runtime_tick(&profile, CSB_V1_TICK_MS_NOMINAL * 3U + 10U);
    CHECK(profile.total_play_ms == 175U,
          "multi-quantum dt preserves the residual wall time");
    CHECK(profile.tick_count == 3U,
          "multi-quantum dt fires three V1 ticks");
    CHECK(profile.game_time == 3U,
          "game_time matches the three fired ticks");
    CHECK(profile.game_ticks == CSB_V1_TICK_MS_NOMINAL * 3U,
          "game_ticks records three nominal quanta");
    CHECK(profile.chaos_magic.spell_grid_version == 3U,
          "chaos spell grid advances once per fired tick");
    CHECK(csb_v1_runtime_tick_due(&profile, 219U) == 0,
          "tick_due(now_ms) is false until the next 55ms boundary");
    CHECK(csb_v1_runtime_tick_due(&profile, 220U) == 1,
          "tick_due(now_ms) detects the next unconsumed boundary");
}

static void test_tick_v1_steps_exactly_once_and_honors_stop_states(void)
{
    CSB_V1_RuntimeProfile profile;

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "tick_v1 fires one deterministic V1 tick from a fresh runtime");
    CHECK(profile.total_play_ms == CSB_V1_TICK_MS_NOMINAL,
          "tick_v1 advances wall time by exactly one nominal quantum");
    CHECK(profile.tick_count == 1U,
          "tick_v1 increments tick_count exactly once");
    CHECK(profile.game_time == 1U,
          "tick_v1 increments game_time exactly once");

    profile.paused = 1;
    CHECK(csb_v1_runtime_tick_v1(&profile) == 0,
          "tick_v1 is blocked while the runtime is paused");
    CHECK(profile.tick_count == 1U,
          "paused tick_v1 does not advance tick_count");
    CHECK(profile.total_play_ms == CSB_V1_TICK_MS_NOMINAL,
          "paused tick_v1 does not accumulate wall time");

    profile.paused = 0;
    profile.game_over = 1;
    csb_v1_runtime_tick(&profile, CSB_V1_TICK_MS_NOMINAL);
    CHECK(profile.tick_count == 1U,
          "game_over runtime_tick does not advance tick_count");
    CHECK(profile.total_play_ms == CSB_V1_TICK_MS_NOMINAL,
          "game_over runtime_tick does not accumulate wall time");
}

static void test_timeline_events_fire_before_game_time_increment(void)
{
    CSB_V1_RuntimeProfile profile;
    struct DM1_Event_V1 ev;
    struct DM1_TickDispatchResult_V1 dispatch;

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_PLAY_SOUND;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 1);
    ev.b_mapX = 7;
    ev.b_mapY = 8;
    CHECK(csb_v1_runtime_add_timeline_event(&profile, &ev) >= 0,
          "CSB runtime accepts a queued V1 timeline event");

    memset(&ev, 0, sizeof(ev));
    ev.type = DM1_EVENT_DOOR_ANIMATION;
    ev.map_time = DM1_MAP_TIME_MAKE(0, 2);
    ev.b_mapX = 9;
    ev.b_mapY = 10;
    CHECK(csb_v1_runtime_add_timeline_event(&profile, &ev) >= 0,
          "CSB runtime accepts a second boundary event");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "first tick fires while game_time is still zero");
    CHECK(profile.game_time == 1U,
          "first tick increments game_time after timeline processing");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 0,
          "event scheduled for time 1 does not fire at pre-increment time 0");
    CHECK(profile.timeline_queue.eventCount == 2,
          "both boundary events remain queued after tick zero processing");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "second tick processes pre-increment game_time 1");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 1,
          "event scheduled for time 1 fires before game_time advances to 2");
    CHECK(dispatch.records[0].dispatchKind == DM1_DISPATCH_SOUND,
          "time 1 event dispatches through the sound event boundary");
    CHECK(dispatch.records[0].mapX == 7 && dispatch.records[0].mapY == 8,
          "time 1 dispatch preserves event coordinates");
    CHECK(profile.game_time == 2U,
          "second tick increments game_time after event dispatch");
    CHECK(profile.timeline_queue.eventCount == 1,
          "future time 2 event remains queued after the time 1 boundary");

    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "third tick processes pre-increment game_time 2");
    CHECK(csb_v1_runtime_get_last_timeline_dispatch(&profile, &dispatch) == 1,
          "event scheduled for time 2 fires on the next boundary");
    CHECK(dispatch.records[0].dispatchKind == DM1_DISPATCH_DOOR_ANIMATION,
          "time 2 event dispatches through the door-animation boundary");
    CHECK(profile.timeline_dispatch_count == 2U,
          "CSB runtime records the cumulative timeline dispatch count");
    CHECK(profile.timeline_queue.eventCount == 0,
          "timeline queue is empty after both boundary events fire");
}

static void seed_two_champion_party(CSB_V1_PartyState *party)
{
    int i;

    csb_v1_character_init_default(party);
    party->ChampionCount = 2;
    party->ImportedFromDM1 = 1;
    party->PartyDirection = CSB_V1_DIR_NORTH;
    party->LeaderIndex = 0;
    for (i = 0; i < party->ChampionCount; i++) {
        CSB_V1_Champion *champion = &party->Champions[i];
        champion->CurrentHealth = (int16_t)(80 + i);
        champion->MaximumHealth = (int16_t)(100 + i);
        champion->Cell = (uint8_t)i;
        champion->Direction = (uint8_t)((i + 2) & 3);
    }
}

static void test_input_command_queue_turn_reaches_runtime_party_state(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    CSB_V1_PartyState after;
    struct Dm1V1InputQueueProcessResultPc34Compat dispatch;

    /* This is a focused CSB V1 command-boundary gate, not a full
     * movement/playability claim.  ReDMCSB COMMAND.C F0380 lines
     * 2075-2127 dequeues one source command and lines 2150-2156
     * dispatch C001/C002 turns to CLIKMENU.C F0365; F0365 lines
     * 156-173 maps TURN_RIGHT to party_dir+1 and calls CHAMPION.C F0284
     * lines 117-130 to rotate champion Cell/Direction. */
    csb_v1_runtime_init(&profile, NULL);
    seed_two_champion_party(&party);
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "runtime accepts a seeded imported party for input binding");

    CHECK(csb_v1_runtime_enqueue_input_command(
              &profile, DM1_V1_COMMAND_TURN_RIGHT, 291, 125) == 1,
          "CSB runtime queues one source TURN_RIGHT command");
    CHECK(profile.input_command_queue.count == 1U,
          "input command queue contains one queued command before dispatch");
    CHECK(csb_v1_runtime_process_one_input_command(&profile, 0, 0, 0) == 1,
          "CSB runtime processes one queued input command");
    CHECK(csb_v1_runtime_get_last_input_dispatch(&profile, &dispatch) == 1,
          "last input dispatch reports one dequeued command");
    CHECK(dispatch.command == DM1_V1_COMMAND_TURN_RIGHT,
          "last input dispatch preserves the TURN_RIGHT source command id");
    CHECK(dispatch.dispatchedTurn == 1 && dispatch.dispatchedMove == 0,
          "queue boundary classifies TURN_RIGHT as a turn dispatch");
    CHECK(profile.input_dispatch_count == 1U,
          "CSB runtime records the command dispatch count");
    CHECK(profile.input_command_queue.count == 0U,
          "input command queue is empty after the turn dispatch");
    CHECK(profile.party_dir == CSB_V1_DIR_EAST,
          "queued TURN_RIGHT reaches CSB runtime party_dir NORTH->EAST");
    CHECK(csb_v1_runtime_get_party_state(&profile, &after) == 2,
          "runtime party snapshot remains visible after queued turn");
    CHECK(after.PartyDirection == CSB_V1_DIR_EAST,
          "party snapshot direction follows the queued turn");
    CHECK(after.Champions[0].Cell == 1 &&
              after.Champions[1].Cell == 2,
          "queued turn rotates champion cells by +1 mod 4");
    CHECK(after.Champions[0].Direction == 3 &&
              after.Champions[1].Direction == 0,
          "queued turn rotates champion directions by +1 mod 4");
}

static void test_input_command_queue_move_boundary_does_not_claim_movement(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputQueueProcessResultPc34Compat dispatch;

    csb_v1_runtime_init(&profile, NULL);
    seed_two_champion_party(&party);
    CHECK(csb_v1_runtime_set_party_state(&profile, &party) == 0,
          "runtime accepts a seeded imported party for move-boundary guard");
    CHECK(csb_v1_runtime_enqueue_input_command(
              &profile, DM1_V1_COMMAND_MOVE_FORWARD, 263, 125) == 1,
          "CSB runtime queues one source MOVE_FORWARD command");
    CHECK(csb_v1_runtime_process_one_input_command(&profile, 0, 0, 0) == 1,
          "CSB runtime dequeues one MOVE_FORWARD command at the command boundary");
    CHECK(csb_v1_runtime_get_last_input_dispatch(&profile, &dispatch) == 1,
          "last input dispatch reports MOVE_FORWARD was dequeued");
    CHECK(dispatch.command == DM1_V1_COMMAND_MOVE_FORWARD &&
              dispatch.dispatchedMove == 1,
          "queue boundary classifies MOVE_FORWARD as a move dispatch");
    CHECK(profile.party_dir == CSB_V1_DIR_NORTH,
          "MOVE_FORWARD boundary does not mutate party_dir before CSB movement is bound");
    CHECK(profile.party_x == CSB_V1_START_PARTY_X &&
              profile.party_y == CSB_V1_START_PARTY_Y,
          "MOVE_FORWARD boundary does not claim CSB party position movement");
}

int main(void)
{
    printf("=== CSB V1 Runtime Tick Accumulator Follow-up ===\n\n");
    test_subquantum_frame_slices_fire_one_tick();
    test_multi_quantum_tick_and_due_probe();
    test_tick_v1_steps_exactly_once_and_honors_stop_states();
    test_timeline_events_fire_before_game_time_increment();
    test_input_command_queue_turn_reaches_runtime_party_state();
    test_input_command_queue_move_boundary_does_not_claim_movement();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: CSB V1 runtime tick boundary accumulates sub-55ms frame slices, fires source-locked V1 quanta, and dispatches timeline events before game_time increments");
        puts("sourceEvidence=ReDMCSB TIMELINE.C F0235/F0240/F0261 lines 702-708,1833-1850; GAMELOOP.C F0002 lines 69-124; COMMAND.C F0380 lines 2383-2429");
        puts("ok: CSB V1 runtime input queue processes one source TURN_RIGHT into party_dir and champion Cell/Direction state without claiming full movement/playability");
        puts("sourceEvidence=ReDMCSB COMMAND.C F0380 lines 2075-2127,2150-2156; CLIKMENU.C F0365 lines 156-173; CHAMPION.C F0284 lines 117-130");
    }
    return failed == 0 ? 0 : 1;
}
