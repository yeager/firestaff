#include "csb_v1_queue_overflow_pc34_compat.h"

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

static const int k_burst_commands[CSB_V1_QUEUE_OVERFLOW_BURST_COUNT_PC34] = {
    DM1_V1_COMMAND_TURN_LEFT,
    DM1_V1_COMMAND_TURN_RIGHT,
    DM1_V1_COMMAND_MOVE_FORWARD,
    DM1_V1_COMMAND_MOVE_RIGHT,
    DM1_V1_COMMAND_MOVE_BACKWARD,
    DM1_V1_COMMAND_MOVE_LEFT,
    DM1_V1_COMMAND_TURN_LEFT,
    DM1_V1_COMMAND_TURN_RIGHT,
    DM1_V1_COMMAND_MOVE_FORWARD,
    DM1_V1_COMMAND_MOVE_RIGHT,
    DM1_V1_COMMAND_MOVE_BACKWARD,
    DM1_V1_COMMAND_MOVE_LEFT
};

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
    for (i = 0; i < party->ChampionCount; ++i) {
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
    CHECK_EQ(csb_v1_runtime_set_party_state(profile, &party), 0,
             "fixture party enters the CSB runtime profile");
}

static void test_source_evidence(void)
{
    const char *evidence = csb_v1_queue_overflow_source_evidence_pc34_compat();

    CHECK(evidence != NULL, "source evidence string is present");
    CHECK(strstr(evidence, "DEFS.H:3261-3264") != NULL,
          "source evidence cites queue storage size");
    CHECK(strstr(evidence, "DEFS.H:3507-3509") != NULL,
          "source evidence cites C5/C7 bounds");
    CHECK(strstr(evidence, "COMMAND.C F0359:1506-1514") != NULL,
          "source evidence cites mouse enqueue cap");
    CHECK(strstr(evidence, "COMMAND.C F0361:1744-1766") != NULL,
          "source evidence cites keyboard enqueue cap");
    CHECK(strstr(evidence, "COMMAND.C F0380:2075-2126") != NULL,
          "source evidence cites dequeue wrap boundary");
}

static void test_regular_burst_caps_and_drops(void)
{
    CSB_V1_RuntimeProfile profile;
    struct Dm1V1InputQueueProcessResultPc34Compat dispatch;
    int enqueue_ok[CSB_V1_QUEUE_OVERFLOW_BURST_COUNT_PC34];
    int i;

    init_runtime_with_party(&profile);

    /* ReDMCSB source-lock: COMMAND.C F0361 lines 1744-1766 only enqueues
     * keyboard commands while G2153_i_QueuedCommandsCount < C5; DEFS.H
     * lines 3507-3509 defines C5/C7, and COMMAND.C F0359 lines 1506-1514
     * reserves the C7 path for release/stop mouse events rather than regular
     * C001..C006 bursts. */
    for (i = 0; i < CSB_V1_QUEUE_OVERFLOW_BURST_COUNT_PC34; ++i) {
        enqueue_ok[i] = csb_v1_runtime_enqueue_input_command(
            &profile, k_burst_commands[i], 100 + i, 140 + i);
    }

    CHECK_EQ(CSB_V1_QUEUE_OVERFLOW_STORAGE_SIZE_PC34, 8,
             "I34/CSB queue storage size remains eight slots");
    CHECK_EQ(CSB_V1_QUEUE_OVERFLOW_REGULAR_CAP_PC34, 5,
             "regular commands stop at the source C5 cap");
    CHECK_EQ(CSB_V1_QUEUE_OVERFLOW_RESERVED_CAP_PC34, 7,
             "reserved release-command cap remains C7");

    for (i = 0; i < CSB_V1_QUEUE_OVERFLOW_REGULAR_CAP_PC34; ++i) {
        CHECK_EQ(enqueue_ok[i], 1, "regular burst command before C5 is accepted");
        CHECK_EQ(profile.input_command_queue.commands[i].command,
                 k_burst_commands[i],
                 "accepted command preserves FIFO order in the CSB runtime queue");
        CHECK_EQ(profile.input_command_queue.commands[i].x,
                 100 + i,
                 "accepted command preserves source X coordinate");
        CHECK_EQ(profile.input_command_queue.commands[i].y,
                 140 + i,
                 "accepted command preserves source Y coordinate");
    }

    for (i = CSB_V1_QUEUE_OVERFLOW_REGULAR_CAP_PC34;
         i < CSB_V1_QUEUE_OVERFLOW_BURST_COUNT_PC34;
         ++i) {
        CHECK_EQ(enqueue_ok[i], 0, "regular burst command after C5 is dropped");
    }

    CHECK_EQ(profile.input_command_queue.count,
             CSB_V1_QUEUE_OVERFLOW_REGULAR_CAP_PC34,
             "queue count is capped at C5 after a long regular burst");
    CHECK_EQ(profile.input_command_queue.droppedFullCount,
             CSB_V1_QUEUE_OVERFLOW_BURST_COUNT_PC34 -
             CSB_V1_QUEUE_OVERFLOW_REGULAR_CAP_PC34,
             "overflow attempts are counted as dropped");

    /* ReDMCSB source-lock: COMMAND.C F0380 lines 2075-2126 reads the first
     * queued command, decrements G2153_i_QueuedCommandsCount, advances the
     * first index, and wraps across M529_COMMAND_QUEUE_SIZE. */
    for (i = 0; i < CSB_V1_QUEUE_OVERFLOW_REGULAR_CAP_PC34; ++i) {
        CHECK_EQ(csb_v1_runtime_process_one_input_command(&profile, 0, 0, 0),
                 1,
                 "runtime drains one accepted command");
        CHECK_EQ(csb_v1_runtime_get_last_input_dispatch(&profile, &dispatch),
                 1,
                 "last dispatch reports a dequeued command");
        CHECK_EQ(dispatch.command,
                 k_burst_commands[i],
                 "drained command order matches the accepted burst prefix");
        CHECK_EQ(dispatch.dequeued, 1,
                 "dispatch record marks the command dequeued");
        CHECK_EQ(profile.input_command_queue.count,
                 CSB_V1_QUEUE_OVERFLOW_REGULAR_CAP_PC34 - 1 - i,
                 "queue count decreases after each drain");
    }

    CHECK_EQ(csb_v1_runtime_process_one_input_command(&profile, 0, 0, 0),
             0,
             "no dropped overflow command is available after the C5 prefix drains");
    CHECK_EQ(csb_v1_runtime_get_last_input_dispatch(&profile, &dispatch),
             0,
             "empty runtime queue reports no dequeued command");
    CHECK_EQ(dispatch.command, DM1_V1_COMMAND_NONE,
             "empty runtime queue leaves command as C000 none");
    CHECK_EQ(profile.input_command_queue.count, 0,
             "queue is empty after draining the accepted prefix");
    CHECK_EQ(profile.input_dispatch_count,
             CSB_V1_QUEUE_OVERFLOW_REGULAR_CAP_PC34,
             "runtime dispatch count advances only for accepted commands");
}

static void test_capacity_frees_after_drain(void)
{
    CSB_V1_RuntimeProfile profile;
    struct Dm1V1InputQueueProcessResultPc34Compat dispatch;

    init_runtime_with_party(&profile);

    CHECK_EQ(csb_v1_runtime_enqueue_input_command(
                 &profile, DM1_V1_COMMAND_MOVE_LEFT, 1, 2),
             1,
             "first command queues in an empty runtime queue");
    CHECK_EQ(csb_v1_runtime_process_one_input_command(&profile, 0, 0, 0),
             1,
             "first command drains from the runtime queue");
    CHECK_EQ(profile.input_command_queue.count, 0,
             "queue returns to empty after one drain");

    CHECK_EQ(csb_v1_runtime_enqueue_input_command(
                 &profile, DM1_V1_COMMAND_TURN_LEFT, 3, 4),
             1,
             "queue accepts a new command after capacity is freed");
    CHECK_EQ(csb_v1_runtime_process_one_input_command(&profile, 0, 0, 0),
             1,
             "newly accepted command drains after reuse");
    CHECK_EQ(csb_v1_runtime_get_last_input_dispatch(&profile, &dispatch),
             1,
             "reused queue dispatch is visible");
    CHECK_EQ(dispatch.command, DM1_V1_COMMAND_TURN_LEFT,
             "reused queue preserves the new command id");
    CHECK_EQ(profile.input_command_queue.count, 0,
             "reused queue is empty after drain");
}

int main(void)
{
    printf("=== CSB V1 Queue Overflow Gate ===\n\n");
    test_source_evidence();
    test_regular_burst_caps_and_drops();
    test_capacity_frees_after_drain();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: CSB V1 regular command bursts are capped at C5, overflow is dropped, and only the accepted prefix drains");
        puts(csb_v1_queue_overflow_source_evidence_pc34_compat());
    }
    return failed == 0 ? 0 : 1;
}
