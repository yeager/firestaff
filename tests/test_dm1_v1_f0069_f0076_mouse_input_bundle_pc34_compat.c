#include "dm1_v1_f0069_f0076_mouse_input_bundle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static uint32_t hash(const uint8_t *pixels, size_t count)
{
    return dm1_v1_f0069_f0070_f0073_fnv1a_pc34(pixels, count);
}

int main(void)
{
    static uint8_t screenPixels[DM1_V1_MOUSE_SOURCE_WIDTH_PC34 * DM1_V1_MOUSE_SOURCE_HEIGHT_PC34];
    uint8_t original[sizeof(screenPixels)];
    uint8_t pointerPixels[3 * 3] = {0, 5, 0, 5, 5, 5, 0, 5, 0};
    uint8_t iconPixels[DM1_V1_F0070_C028_WIDTH_PC34 * DM1_V1_F0070_C028_HEIGHT_PC34];
    DM1_V1_MouseSourceSurfacePc34 sourceScreen;
    DM1_V1_MouseMutableSurfacePc34 mutableScreen;
    DM1_V1_MouseSourceSurfacePc34 pointer;
    DM1_V1_MousePointerStatePc34 state;
    DM1_V1_MousePacketPc34 packet;
    DM1_V1_F0069F0070RuntimeInputPc34 runtimeInput;
    DM1_V1_F0069F0076DispatchReceiptPc34 receipt;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1QueuedCommandPc34Compat command;
    size_t i;

    for (i = 0; i < sizeof(screenPixels); ++i) screenPixels[i] = (uint8_t)(i & 15U);
    memcpy(original, screenPixels, sizeof(original));
    memset(iconPixels, 3, sizeof(iconPixels));
    sourceScreen.sourceOwned = 1;
    sourceScreen.width = DM1_V1_MOUSE_SOURCE_WIDTH_PC34;
    sourceScreen.height = DM1_V1_MOUSE_SOURCE_HEIGHT_PC34;
    sourceScreen.pixels = screenPixels;
    sourceScreen.pixelCount = sizeof(screenPixels);
    sourceScreen.pixelsFnv1a = hash(screenPixels, sizeof(screenPixels));
    mutableScreen.sourceOwned = 1;
    mutableScreen.width = sourceScreen.width;
    mutableScreen.height = sourceScreen.height;
    mutableScreen.pixels = screenPixels;
    mutableScreen.pixelCount = sizeof(screenPixels);
    pointer.sourceOwned = 1;
    pointer.width = 3;
    pointer.height = 3;
    pointer.pixels = pointerPixels;
    pointer.pixelCount = sizeof(pointerPixels);
    pointer.pixelsFnv1a = hash(pointerPixels, sizeof(pointerPixels));

    dm1_v1_f0069_f0076_mouse_state_init_pc34(&state, 1, 1);
    check(F0073_MOUSE_BuildPointerScreenArea(&state, &sourceScreen, 3, 3, 1, 1),
          "F0073 captures source-owned logical pointer area");
    check(S0074_MOUSE_DrawPointerScreenArea(&state, &mutableScreen, &pointer, 1, 1, 0),
          "S0074 captures then draws source pointer pixels");
    check(memcmp(original, screenPixels, sizeof(original)) != 0,
          "pointer pixels changed only the captured source screen");
    check(S0072_MOUSE_DrawPointerHiddenScreenArea(&state, &mutableScreen),
          "S0072 restores exact saved pointer area");
    check(memcmp(original, screenPixels, sizeof(original)) == 0,
          "S0072 restore yields original screen bytes");

    packet.deltaX = 10000;
    packet.deltaY = -10000;
    packet.buttonStatus = 0;
    check(S0075_MOUSE_Exception70Handler_IKBD_MIDI_MouseStatus(&state, &packet),
          "S0075 samples caller packet");
    check(state.pointerX == 319 && state.pointerY == 0,
          "S0075 clamps source coordinates before dispatch");
    packet.deltaX = -37;
    packet.deltaY = 1;
    check(S0075_MOUSE_Exception70Handler_IKBD_MIDI_MouseStatus(&state, &packet),
          "S0075 moves to champion icon source coordinate");
    check(state.pointerX == 282 && state.pointerY == 1,
          "pointer lands in source C125 geometry");

    memset(&runtimeInput, 0, sizeof(runtimeInput));
    runtimeInput.championIcons.graphicsDatAuthenticated = 1;
    runtimeInput.championIcons.graphicIndex = DM1_V1_F0070_C028_GRAPHIC_PC34;
    runtimeInput.championIcons.width = DM1_V1_F0070_C028_WIDTH_PC34;
    runtimeInput.championIcons.height = DM1_V1_F0070_C028_HEIGHT_PC34;
    runtimeInput.championIcons.indexedPixels = iconPixels;
    runtimeInput.championIcons.indexedPixelCount = sizeof(iconPixels);
    runtimeInput.championIcons.indexedPixelsFnv1a = hash(iconPixels, sizeof(iconPixels));
    runtimeInput.leaderEmptyHanded = 1;
    runtimeInput.leaderIndex = -1;
    runtimeInput.partyDirection = 0;
    runtimeInput.championIndexByCell[0] = 7;
    runtimeInput.championIndexByCell[1] = -1;
    runtimeInput.championIndexByCell[2] = -1;
    runtimeInput.championIndexByCell[3] = -1;
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    check(S0076_MOUSE_OnMouseButtonsStatusChange(&state, &runtimeInput, &queue,
                                                  DM1_V1_BUTTON_LEFT, &receipt),
          "S0076 authenticates F0069/F0070 before it queues C125");
    check(receipt.valid && receipt.coordinateOrderValid && receipt.f0070ChampionCommand == 125 &&
              receipt.f0069Pointer == DM1_V1_F0069_POINTER_CHAMPION_PC34 && receipt.queued,
          "F0069/F0070 receipt preserves pointer and command source ordering");
    check(DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &command) && command.command == 125,
          "F0070 only queues the original C125 command through COMMAND.C owner");
    check(!S0076_MOUSE_OnMouseButtonsStatusChange(&state, &runtimeInput, &queue,
                                                   DM1_V1_BUTTON_LEFT, &receipt),
          "S0076 rejects duplicate held-button samples");
    check(S0076_MOUSE_OnMouseButtonsStatusChange(&state, &runtimeInput, &queue, 0, &receipt),
          "S0076 routes left release through existing queue owner");

    runtimeInput.championIcons.indexedPixelsFnv1a ^= 1U;
    state.buttonStatus = 0;
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    check(!S0076_MOUSE_OnMouseButtonsStatusChange(&state, &runtimeInput, &queue,
                                                   DM1_V1_BUTTON_LEFT, &receipt) && queue.count == 0U,
          "invalid C028 provenance fails before queue mutation");
    return failures ? 1 : 0;
}
