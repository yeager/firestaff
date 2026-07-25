#include "dm1_v1_input_command_queue_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void assert_first_command(
    const struct Dm1V1InputCommandQueuePc34Compat* queue,
    int command,
    int x,
    int y)
{
    (void)y;
    (void)x;
    (void)command;
    (void)queue;
    struct Dm1V1QueuedCommandPc34Compat out;
    (void)out;

    assert(DM1_V1_InputCommandQueue_PeekPc34Compat(queue, &out) == 1);
    assert(out.command == command);
    assert(out.x == x);
    assert(out.y == y);
}

int main(void)
{
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1UsioMouseStatusPc34Compat hostStatus;
    struct Dm1V1UsioMouseStatusPc34Compat outStatus;
    int i;
    const char* evidence;
    (void)evidence;

    hostStatus.mouseButtons = DM1_V1_BUTTON_LEFT;
    hostStatus.mouseX = 291;
    hostStatus.mouseY = 125;
    memset(&outStatus, 0, sizeof(outStatus));
    assert(F1684_GetMouseStatus(&outStatus, &hostStatus) == 1);
    assert(outStatus.mouseButtons == DM1_V1_BUTTON_LEFT);
    assert(outStatus.mouseX == 291);
    assert(outStatus.mouseY == 125);
    assert(F1684_GetMouseStatus(&outStatus, 0) == 0);
    assert(outStatus.mouseX == 291);
    assert(F1684_GetMouseStatus(0, &hostStatus) == 0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    assert(F1694_AddMouseInputToQueue(
               &queue, outStatus.mouseX, outStatus.mouseY,
               outStatus.mouseButtons) == 1);
    assert_first_command(&queue, DM1_V1_COMMAND_TURN_RIGHT, 291, 125);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    assert(F1694_AddMouseInputToQueue(
               &queue, -1, -1,
               DM1_V1_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION) == 1);
    assert_first_command(&queue, DM1_V1_COMMAND_RELEASE_CHAMPION_ICON, -1, -1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    for (i = 0; i < 5; ++i) {
        assert(F1694_AddMouseInputToQueue(
                   &queue, 263, 125, DM1_V1_BUTTON_LEFT) == 1);
    }
    assert(F1694_AddMouseInputToQueue(
               &queue, 291, 125, DM1_V1_BUTTON_LEFT) == 0);
    assert(queue.count == 5u);
    assert(queue.droppedFullCount == 1u);

    assert(F1694_AddMouseInputToQueue(0, 291, 125, DM1_V1_BUTTON_LEFT) == 0);

    evidence = F1684_GetMouseStatus_SourceEvidence();
    assert(evidence != 0);
    assert(strstr(evidence, "USIO1.C:80") != 0);
    assert(strstr(evidence, "no host cursor polling") != 0);

    evidence = F1694_AddMouseInputToQueue_SourceEvidence();
    assert(evidence != 0);
    assert(strstr(evidence, "USIO1.C:164") != 0);
    assert(strstr(evidence, "COMMAND.C mouse command tables") != 0);

    return 0;
}
