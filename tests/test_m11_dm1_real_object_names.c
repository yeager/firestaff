#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    unsigned short thing;
    char name[64];
    unsigned char framebuffer[320 * 200];
    size_t cursorPixels = 0u;
    int y;

    if (!data_dir || !data_dir[0]) {
        puts("skip: FIRESTAFF_DM1_DATA_DIR is not set");
        return 0;
    }

    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        fprintf(stderr, "DM1 real-data launch failed: %s\n", data_dir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!state.dm1ObjectNameTableValid) {
        fprintf(stderr, "DM1 real GRAPHICS.DAT did not bind the M564 name table\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!state.world.things || state.world.things->weaponCount <= 0) {
        fprintf(stderr, "DM1 real DUNGEON.DAT contains no weapon record\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* ReDMCSB OBJECT.C F0031/F0033 resolves the visible name from the
     * icon-indexed M564 stream, not from the decoded subtype catalog. */
    thing = (unsigned short)(THING_TYPE_WEAPON << 10);
    if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(&state, thing)) {
        fprintf(stderr, "could not move first real weapon to leader hand\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(name, 0, sizeof(name));
    if (!DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat(
            &state, name, (int)sizeof(name)) || name[0] == '\0' ||
        strncmp(name, "WEAPON ", 7) == 0) {
        fprintf(stderr, "real M564 leader-hand name invalid: '%s'\n", name);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* ReDMCSB IO.C F0702 replaces the host arrow with the held source
     * object. Verify the final indexed framebuffer, not merely the transient
     * leader-hand state or the name resolver. */
    memset(framebuffer, 0, sizeof(framebuffer));
    state.pointerPositionKnown = 1;
    state.pointerX = 120;
    state.pointerY = 80;
    M11_GameView_DrawLeaderHandCursor(&state, framebuffer,
                                      320, 200);
    for (y = state.pointerY;
         y < state.pointerY + 18 && y < 200; ++y) {
        int x;
        for (x = state.pointerX;
             x < state.pointerX + 18 && x < 320; ++x) {
            if (framebuffer[y * 320 + x] != 0u) {
                ++cursorPixels;
            }
        }
    }
    if (cursorPixels == 0u) {
        fprintf(stderr, "real F0702 held-object cursor did not write pixels\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(&state);

    /* ReDMCSB CLIKVIEW.C F0373 puts a floor pickup in the transient mouse
     * hand first.  Use a real decoded weapon record to verify the keyboard
     * pickup route does not skip that hand and silently write an inventory
     * slot. */
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] = thing;
    {
        int dropped = M11_GameView_DropItem(&state);
        if (!dropped ||
            state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] !=
                THING_NONE ||
            !M11_GameView_PickupItem(&state) ||
            DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != thing) {
            fprintf(stderr,
                    "real floor pickup skipped the source mouse-hand route drop=%d map=%d x=%d y=%d detail=%s\n",
                    dropped, state.world.party.mapIndex, state.world.party.mapX,
                    state.world.party.mapY, state.inspectDetail);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }

    M11_GameView_Shutdown(&state);
    printf("ok: real DM1 M564 leader-hand name = %s; F0702 cursor pixels=%zu; floor pickup -> mouse hand\n",
           name, cursorPixels);
    return 0;
}
