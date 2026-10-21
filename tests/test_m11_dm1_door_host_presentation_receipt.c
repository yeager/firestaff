#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char* archivePath = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    M11_GameViewState state;
    M11_Dm1DoorHostPresentationReceipt receipt;
    unsigned char framebuffer[320 * 200];

    if (!archivePath || !archivePath[0]) {
        puts("skip: FIRESTAFF_DM1_PC34_ARCHIVE is not set");
        return 77;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, archivePath)) {
        fprintf(stderr, "could not start DM1 from PC34 archive: %s\n", archivePath);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    if (!M11_GameView_ProbeDrawDm1CenterDoorHostReceipt(
            &state, framebuffer, 320, 200)) {
        fprintf(stderr, "real PC34 center-door material draw failed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1DoorHostPresentationReceipt(&receipt);
    if (!receipt.valid || receipt.depthIndex != 0 || receipt.doorState != 4 ||
        !receipt.panelVisible || receipt.frameCount != 3 || receipt.blitCount != 4 ||
        receipt.graphicsId[0] != 91 || receipt.graphicsId[1] != 87 ||
        receipt.graphicsId[2] != 87 || receipt.graphicsId[3] != 248 ||
        receipt.width[3] != 96 || receipt.height[3] != 86) {
        fprintf(stderr, "real PC34 center-door receipt incomplete\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 F0111 center-door material uses mounted dungeon wallset");
    return 0;
}
