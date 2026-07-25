#include "dm1_v1_input_command_queue_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void assert_peek_command(
    const struct Dm1V1InputCommandQueuePc34Compat* queue,
    int expectedCommand,
    int expectedX,
    int expectedY)
{
    (void)expectedY;
    (void)expectedX;
    (void)expectedCommand;
    (void)queue;
    struct Dm1V1QueuedCommandPc34Compat out;
    (void)out;

    assert(DM1_V1_InputCommandQueue_PeekPc34Compat(queue, &out) == 1);
    assert(out.command == expectedCommand);
    assert(out.x == expectedX);
    assert(out.y == expectedY);
}

int main(void)
{
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1UsioDataPc34Compat batch[3];
    struct Dm1V1UsioDataPc34Compat pending;
    struct Dm1V1QueuedCommandPc34Compat out;
    (void)out;
    int pendingPresent;
    (void)pendingPresent;
    int i;
    const char* evidence;
    (void)evidence;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    memset(batch, 0, sizeof(batch));
    batch[0].usioType = DM1_V1_USIO_DATA_TYPE_KEYBOARD;
    batch[0].rawKeyCode = 0x004B;
    batch[1].usioType = DM1_V1_USIO_DATA_TYPE_MOUSE;
    batch[1].mouseX = 263;
    batch[1].mouseY = 125;
    batch[1].mouseButtons = DM1_V1_BUTTON_LEFT;
    batch[2].usioType = DM1_V1_USIO_DATA_TYPE_NONE;

    assert(F1172_QueueMouseAndKeyboardInput(&queue, batch, 3u) == 2);
    assert(queue.count == 2u);
    assert_peek_command(&queue, DM1_V1_COMMAND_TURN_LEFT, 0, 0);
    assert(DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
               &queue, 0, 0, 0, 0).command == DM1_V1_COMMAND_TURN_LEFT);
    assert_peek_command(&queue, DM1_V1_COMMAND_MOVE_FORWARD, 263, 125);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    pending.usioType = DM1_V1_USIO_DATA_TYPE_MOUSE;
    pending.rawKeyCode = 0;
    pending.mouseX = 233;
    pending.mouseY = 42;
    pending.mouseButtons = DM1_V1_BUTTON_LEFT;
    pendingPresent = 1;
    assert(F1174_AddPendingUsioDataToInputQueue(
               &queue, &pending, &pendingPresent) == 1);
    assert(pendingPresent == 0);
    assert(pending.usioType == DM1_V1_USIO_DATA_TYPE_NONE);
    assert_peek_command(&queue, DM1_V1_COMMAND_CLICK_IN_SPELL_AREA, 233, 42);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    for (i = 0; i < 5; ++i) {
        batch[0].usioType = DM1_V1_USIO_DATA_TYPE_KEYBOARD;
        batch[0].rawKeyCode = 0x004C;
        assert(F1173_AddUsioDataToInputQueue(&queue, &batch[0]) == 1);
    }
    pending.usioType = DM1_V1_USIO_DATA_TYPE_KEYBOARD;
    pending.rawKeyCode = 0x004D;
    pendingPresent = 1;
    assert(F1174_AddPendingUsioDataToInputQueue(
               &queue, &pending, &pendingPresent) == 0);
    assert(pendingPresent == 1);
    assert(pending.usioType == DM1_V1_USIO_DATA_TYPE_KEYBOARD);
    assert(queue.droppedFullCount == 1u);
    assert(DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &out) == 1);
    assert(out.command == DM1_V1_COMMAND_MOVE_FORWARD);

    assert(F1172_QueueMouseAndKeyboardInput(0, batch, 1u) == 0);
    assert(F1172_QueueMouseAndKeyboardInput(&queue, 0, 1u) == 0);
    assert(F1173_AddUsioDataToInputQueue(&queue, 0) == 0);
    assert(F1174_AddPendingUsioDataToInputQueue(&queue, &pending, 0) == 0);

    evidence = F1172_QueueMouseAndKeyboardInput_SourceEvidence();
    assert(evidence != 0);
    assert(strstr(evidence, "USIO2.C:13") != 0);
    assert(strstr(evidence, "does not poll host input") != 0);
    evidence = F1173_AddUsioDataToInputQueue_SourceEvidence();
    assert(evidence != 0);
    assert(strstr(evidence, "USIO1.C:177") != 0);
    assert(strstr(evidence, "COMMAND.C") != 0);
    evidence = F1174_AddPendingUsioDataToInputQueue_SourceEvidence();
    assert(evidence != 0);
    assert(strstr(evidence, "USIO2.C:264") != 0);
    assert(strstr(evidence, "cleared only after an accepted enqueue") != 0);

    return 0;
}
