#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    M11_GameViewState state;
    M11_Dm1FloorItemHostPresentationReceipt receipt;
    unsigned char framebuffer[320 * 200];
    const char *data_dir = argc > 1 ? argv[1] : NULL;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP DM1 floor-item host receipt: no data directory");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        puts("SKIP DM1 floor-item host receipt: DM1 data unavailable");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    M11_GameView_Shutdown(&state);
    if (!receipt.valid) {
        puts("SKIP DM1 floor-item host receipt: no naturally visible F0115 item");
        return 0;
    }
    if (receipt.graphicsId <= 0 || receipt.transparentColor != 10 ||
        !receipt.usesF0791Blit || receipt.destinationW <= 0 ||
        receipt.destinationH <= 0 || receipt.assetWidth <= 0 ||
        receipt.assetHeight <= 0) {
        puts("FAIL DM1 floor-item host receipt is incomplete");
        return 1;
    }
    printf("ok: DM1 floor-item host receipt gfx=%d zone=%d row=%d dst=%d,%d %dx%d asset=%dx%d\n",
           receipt.graphicsId, receipt.sourceZone, receipt.sourceZoneRow,
           receipt.destinationX, receipt.destinationY, receipt.destinationW,
           receipt.destinationH, receipt.assetWidth, receipt.assetHeight);
    return 0;
}
