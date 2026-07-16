#include "dm1_v1_input_command_queue_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_int(const char* label, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s: got %d expected %d\n", label, actual, expected);
        failures++;
    }
}

static void expect_uint(const char* label, unsigned int actual, unsigned int expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s: got %u expected %u\n", label, actual, expected);
        failures++;
    }
}

static void expect_contains(const char* label, const char* haystack, const char* needle)
{
    if (!haystack || !needle || strstr(haystack, needle) == 0) {
        fprintf(stderr, "%s: missing \"%s\" in \"%s\"\n",
                label,
                needle ? needle : "(null)",
                haystack ? haystack : "(null)");
        failures++;
    }
}

static void populate_mixed_queue(struct Dm1V1InputCommandQueuePc34Compat* queue)
{
    DM1_V1_InputCommandQueue_InitPc34Compat(queue);
    expect_int("enqueue move forward",
               DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
                   queue, DM1_V1_COMMAND_MOVE_FORWARD, 10, 11),
               1);
    expect_int("enqueue release icon",
               DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
                   queue, DM1_V1_COMMAND_RELEASE_CHAMPION_ICON, 12, 13,
                   DM1_V1_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION),
               1);
    expect_int("enqueue turn right",
               DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
                   queue, DM1_V1_COMMAND_TURN_RIGHT, 14, 15),
               1);
    expect_int("enqueue stop wall press",
               DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
                   queue, DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL, 16, 17,
                   DM1_V1_BUTTON_LEFT_UP),
               1);
    expect_uint("mixed queue count before discard", queue->count, 4u);
}

static void assert_only_reserved_release_commands_remain(
    const struct Dm1V1InputCommandQueuePc34Compat* queue)
{
    expect_uint("discarded queue count", queue->count, 2u);
    expect_int("first survivor command",
               queue->commands[0].command,
               DM1_V1_COMMAND_RELEASE_CHAMPION_ICON);
    expect_int("first survivor x", queue->commands[0].x, 12);
    expect_int("first survivor y", queue->commands[0].y, 13);
    expect_int("second survivor command",
               queue->commands[1].command,
               DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL);
    expect_int("second survivor x", queue->commands[1].x, 16);
    expect_int("second survivor y", queue->commands[1].y, 17);
    expect_int("pending click cleared", queue->pendingClickPresent, 0);
    expect_int("pending command cleared",
               queue->pendingClickCommand,
               DM1_V1_COMMAND_NONE);
    expect_int("queue unlocked", queue->locked, 0);
}

int main(void)
{
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1InputCommandQueuePc34Compat compatQueue;

    populate_mixed_queue(&queue);
    queue.pendingClickPresent = 1;
    queue.pendingClickX = 22;
    queue.pendingClickY = 23;
    queue.pendingClickButtons = DM1_V1_BUTTON_LEFT;
    queue.pendingClickCommand = DM1_V1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    F0357_COMMAND_DiscardAllInput(&queue);
    assert_only_reserved_release_commands_remain(&queue);

    populate_mixed_queue(&compatQueue);
    DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(&compatQueue);
    assert_only_reserved_release_commands_remain(&compatQueue);

    expect_contains("F0357 source evidence",
                    F0357_COMMAND_DiscardAllInput_SourceEvidence(),
                    "COMMAND.C:1304-1377 F0357_COMMAND_DiscardAllInput");
    expect_contains("queue source evidence",
                    DM1_V1_InputCommandQueue_SourceEvidencePc34Compat(),
                    "1304-1377");

    F0357_COMMAND_DiscardAllInput(0);

    if (failures != 0) {
        fprintf(stderr, "F0357 input discard test failed: %d failures\n", failures);
        return 1;
    }
    puts("F0357 input discard test passed");
    return 0;
}
