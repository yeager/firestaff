#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    M11_Dm1FloorItemHostPresentationReceipt receipt;
    memset(&receipt, 0xff, sizeof(receipt));
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    if (receipt.valid || receipt.graphicsId != 0 || receipt.destinationW != 0 ||
        receipt.destinationH != 0 || receipt.assetWidth != 0 ||
        receipt.assetHeight != 0) {
        return 1;
    }
    puts("ok: M11 DM1 floor-item host receipt is invalid before a real draw");
    return 0;
}
