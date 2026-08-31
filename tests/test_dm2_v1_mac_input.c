#include "dm2_v1_mac_input.h"

#include <assert.h>
#include <stdio.h>

static void expect(uint32_t key, uint32_t mods, DM2_V1_MacInputPhase phase,
                   DM2_V1_MacInputAction action, int command)
{
    DM2_V1_MacInputReceipt r;
    assert(dm2_v1_mac_input_resolve(key, mods, phase, &r));
    assert(r.accepted && r.action == action && r.runtime_command == command);
}

int main(void)
{
    expect('1', 0, DM2_V1_MAC_INPUT_GAMEPLAY, DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_0, 7);
    expect(DM2_V1_MAC_KEY_F4, 0, DM2_V1_MAC_INPUT_GAMEPLAY, DM2_V1_MAC_ACTION_TOGGLE_CHAMPION_3, 10);
    expect(' ', 0, DM2_V1_MAC_INPUT_GAMEPLAY, DM2_V1_MAC_ACTION_TOGGLE_LEADER, 83);
    expect(DM2_V1_MAC_KEY_NUMPAD_4, 0, DM2_V1_MAC_INPUT_GAMEPLAY, DM2_V1_MAC_ACTION_TURN_LEFT, 1);
    expect('k', 0, DM2_V1_MAC_INPUT_GAMEPLAY, DM2_V1_MAC_ACTION_MOVE_FORWARD, 3);
    expect(DM2_V1_MAC_KEY_PAGE_DOWN, 0, DM2_V1_MAC_INPUT_GAMEPLAY, DM2_V1_MAC_ACTION_MOVE_RIGHT, 4);
    expect('`', 0, DM2_V1_MAC_INPUT_GAMEPLAY, DM2_V1_MAC_ACTION_FREEZE, 147);
    expect('s', DM2_V1_MAC_MOD_COMMAND, DM2_V1_MAC_INPUT_GAMEPLAY, DM2_V1_MAC_ACTION_SAVE_GAME, 140);
    expect('o', DM2_V1_MAC_MOD_COMMAND, DM2_V1_MAC_INPUT_GAMEPLAY, DM2_V1_MAC_ACTION_OPEN_GAME, 0);
    expect(DM2_V1_MAC_KEY_RETURN, 0, DM2_V1_MAC_INPUT_ENTRANCE, DM2_V1_MAC_ACTION_NEW_GAME, 0);
    expect(DM2_V1_MAC_KEY_ENTER, 0, DM2_V1_MAC_INPUT_CREDITS, DM2_V1_MAC_ACTION_CLOSE_CREDITS, 0);

    {
        struct Dm1V1InputCommandQueuePc34Compat queue;
        DM2_V1_MacInputReceipt r;
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        assert(dm2_v1_mac_input_resolve('k', 0, DM2_V1_MAC_INPUT_GAMEPLAY, &r));
        assert(dm2_v1_mac_input_enqueue(&r, &queue));
        assert(queue.count == 1u && queue.commands[0].command == DM1_V1_COMMAND_MOVE_FORWARD);
        assert(dm2_v1_mac_input_resolve('o', 0, DM2_V1_MAC_INPUT_GAMEPLAY, &r));
        assert(!dm2_v1_mac_input_enqueue(&r, &queue));
    }

    {
        DM2_V1_MacInputReceipt r;
        assert(!dm2_v1_mac_input_resolve('5', 0,
                                         DM2_V1_MAC_INPUT_GAMEPLAY, &r));
        assert(!dm2_v1_mac_input_resolve('n', DM2_V1_MAC_MOD_COMMAND,
                                         DM2_V1_MAC_INPUT_GAMEPLAY, &r));
        assert(!dm2_v1_mac_input_resolve('1', DM2_V1_MAC_MOD_COMMAND,
                                         DM2_V1_MAC_INPUT_GAMEPLAY, &r));
    }
    puts("PASS: DM2 Macintosh English input table is source-locked");
    puts(dm2_v1_mac_input_source_evidence());
    return 0;
}
