#include "dm1_v1_cursor_icon_swap_pc34_compat.h"

#include <string.h>

void dm1_v1_cursor_icon_swap_init_pc34(
    DM1_V1_CursorIconSwapStatePc34 *outState)
{
    if (!outState) return;
    memset(outState, 0, sizeof(*outState));
    outState->activePointerType = DM1_V1_CURSOR_ARROW_PC34;
    outState->previousPointerType = DM1_V1_CURSOR_ARROW_PC34;
}

int dm1_v1_cursor_icon_swap_update_pc34(
    DM1_V1_CursorIconSwapStatePc34 *state,
    const DM1_V1_CursorIconSwapInputPc34 *input,
    DM1_V1_CursorIconSwapReceiptPc34 *outReceipt)
{
    int16_t resolved;

    if (!state || !input || !outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));

    if (input->objectInHand) {
        resolved = DM1_V1_CURSOR_OBJECT_PC34;
    } else if (input->actionMenuOpen) {
        resolved = DM1_V1_CURSOR_HAND_PC34;
    } else if (state->championIconOrdinal > 0) {
        resolved = DM1_V1_CURSOR_CHAMPION_PC34;
    } else {
        resolved = DM1_V1_CURSOR_ARROW_PC34;
    }

    state->useHandForAction = input->actionMenuOpen;
    state->useObjectForDrag = input->objectInHand;
    state->previousPointerType = state->activePointerType;
    state->activePointerType = resolved;
    state->swapOccurred = (resolved != state->previousPointerType);

    outReceipt->valid = 1;
    outReceipt->resolvedPointerType = resolved;
    outReceipt->swapTriggered = state->swapOccurred;
    return 1;
}
