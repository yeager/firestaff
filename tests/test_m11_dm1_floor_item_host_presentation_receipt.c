#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    M11_GameViewState state;
    M11_Dm1FloorItemHostPresentationReceipt receipt;
    unsigned char framebuffer[320 * 200];

    M11_GameView_Init(&state);
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0xff, sizeof(receipt));
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    if (receipt.valid || receipt.graphicsId != 0 || receipt.destinationW != 0 ||
        receipt.destinationH != 0 || receipt.assetWidth != 0 ||
        receipt.assetHeight != 0) {
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: M11 DM1 floor-item host receipt remains invalid without a real draw");
    return 0;
}
