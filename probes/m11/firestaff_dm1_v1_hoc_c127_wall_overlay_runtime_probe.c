/* Opt-in real-data C127 champion-mirror wall-overlay probe.
 * ReDMCSB DUNGEON.C F0172:2573/2608-2612 publishes C127 only for the
 * visible wall; DUNVIEW.C:3913-3928 consumes it as the C346/C026 overlay. */
#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    M11_GameViewState state;
    M11_BootProbeReceipt receipt;
    M11_Dm1FloorItemHostPresentationReceipt floor_receipt;
    unsigned char framebuffer[320 * 200];
    const char *data_dir = argc > 1 ? argv[1] : NULL;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP DM1 HoC C127 mirror: no data directory");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        puts("SKIP DM1 HoC C127 mirror: DM1 data unavailable");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    if (!M11_GameView_GetBootProbeReceipt(&state, &receipt) ||
        !receipt.dm1HoCLiveC127MaterialRequest ||
        !receipt.dm1HoCHallMirrorOverlay) {
        puts("SKIP DM1 HoC C127 mirror: no naturally visible wall mirror");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&floor_receipt);
    if (!M11_GameView_GetBootProbeReceipt(&state, &receipt) ||
        !receipt.dm1HoCLiveC127MaterialRequest ||
        !receipt.dm1HoCHallMirrorOverlay || floor_receipt.valid ||
        receipt.dm1HoCLiveF0115MaterialRequest) {
        puts("FAIL DM1 HoC C127 mirror was classified as floor-item material");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!M11_GameView_ProbeDrawDm1ProjectileForFloorItemReceipt(
            &state, framebuffer, 320, 200)) {
        puts("SKIP DM1 HoC C127 mirror: original projectile asset unavailable");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&floor_receipt);
    if (floor_receipt.valid || receipt.dm1HoCLiveF0115MaterialRequest) {
        puts("FAIL DM1 HoC C127 mirror accepted projectile as floor-item material");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    printf("ok: DM1 HoC C127 wall overlay map=%dx%d floor-item=0 projectile=0\n",
           receipt.dm1HoCMapWidth, receipt.dm1HoCMapHeight);
    M11_GameView_Shutdown(&state);
    return 0;
}
