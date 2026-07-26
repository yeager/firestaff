#ifndef FIRESTAFF_DM1_V1_CURSOR_ICON_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CURSOR_ICON_SWAP_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_CURSOR_ARROW_PC34 = 0,
    DM1_V1_CURSOR_HAND_PC34 = 1,
    DM1_V1_CURSOR_OBJECT_PC34 = 2,
    DM1_V1_CURSOR_CHAMPION_PC34 = 3
};

typedef struct DM1_V1_CursorIconSwapStatePc34 {
    int16_t activePointerType;
    int16_t previousPointerType;
    int swapOccurred;
    int useHandForAction;
    int useObjectForDrag;
    uint16_t championIconOrdinal;
} DM1_V1_CursorIconSwapStatePc34;

typedef struct DM1_V1_CursorIconSwapInputPc34 {
    int16_t mouseX;
    int16_t mouseY;
    int actionMenuOpen;
    int objectInHand;
    uint16_t inventoryChampionOrdinal;
} DM1_V1_CursorIconSwapInputPc34;

typedef struct DM1_V1_CursorIconSwapReceiptPc34 {
    int valid;
    int16_t resolvedPointerType;
    int swapTriggered;
} DM1_V1_CursorIconSwapReceiptPc34;

void dm1_v1_cursor_icon_swap_init_pc34(
    DM1_V1_CursorIconSwapStatePc34 *outState);

int dm1_v1_cursor_icon_swap_update_pc34(
    DM1_V1_CursorIconSwapStatePc34 *state,
    const DM1_V1_CursorIconSwapInputPc34 *input,
    DM1_V1_CursorIconSwapReceiptPc34 *outReceipt);

#endif
