#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    M11_GameViewState state;
    char name[64];
    unsigned short weapon = (unsigned short)(5u << 10); /* PC34 weapon type. */

    M11_GameView_Init(&state);
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(&state, weapon)) {
        fprintf(stderr, "failed to seed DM1 leader hand\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.active = 1;
    if (M11_GameView_HandlePointerMove(&state, 120, 80) !=
            M11_GAME_INPUT_REDRAW) {
        fprintf(stderr, "held DM1 object did not invalidate the cursor frame\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(name, 'X', sizeof(name));
    name[sizeof(name) - 1] = '\0';
    if (DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat(
            &state, name, (int)sizeof(name)) != 0 || name[0] != '\0') {
        fprintf(stderr, "DM1 name fallback escaped without M564\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: dm1 source item names fail closed without M564");
    return 0;
}
