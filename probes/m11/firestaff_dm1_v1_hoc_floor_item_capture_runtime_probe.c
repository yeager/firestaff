/* Opt-in real-data HoC F0115 capture gate probe.
 * ReDMCSB DUNVIEW.C F0115:4547-4581 draws objects before the later
 * projectile pass at 5645-5683. No world item is spawned or injected. */
#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    M11_GameViewState state;
    M11_GameViewState empty_frame;
    M11_Dm1FloorItemHostPresentationReceipt receipt;
    unsigned char framebuffer[320 * 200];
    const char *data_dir = argc > 1 ? argv[1] : NULL;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP DM1 HoC F0115 capture: no data directory");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        puts("SKIP DM1 HoC F0115 capture: DM1 data unavailable");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1)) {
        puts("FAIL DM1 HoC F0115 capture opened before a frame");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&receipt);
    if (!receipt.valid) {
        if (M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1) ||
            M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(0)) {
            puts("FAIL DM1 HoC F0115 projectile/midair opened capture");
            M11_GameView_Shutdown(&state);
            return 1;
        }
        puts("SKIP DM1 HoC F0115 capture: no naturally visible floor item");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (!M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1) ||
        M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(0)) {
        puts("FAIL DM1 HoC F0115 receipt did not gate capture correctly");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* A following no-item frame clears the frame-local receipt. */
    M11_GameView_Init(&empty_frame);
    M11_GameView_Draw(&empty_frame, framebuffer, 320, 200);
    if (M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1) ||
        M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(0)) {
        puts("FAIL DM1 HoC F0115 stale receipt opened capture");
        M11_GameView_Shutdown(&empty_frame);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&empty_frame);
    printf("ok: DM1 HoC F0115 capture gfx=%d dst=%d,%d %dx%d\n",
           receipt.graphicsId, receipt.destinationX, receipt.destinationY,
           receipt.destinationW, receipt.destinationH);
    M11_GameView_Shutdown(&state);
    return 0;
}
