/* Opt-in real-data HoC frame probe: M648 inscription provenance, C127 wall
 * overlay, and F0115 floor item stay separate. ReDMCSB DUNVIEW.C:3679-3706,
 * 3913-3928, and F0115:4547-4581/5645-5683. */
#include "m11_game_view.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    M11_GameViewState state;
    M11_BootProbeReceipt boot;
    M11_Dm1FloorItemHostPresentationReceipt item;
    unsigned char framebuffer[320 * 200];
    const char *data_dir = argc > 1 ? argv[1] : NULL;
    if (!data_dir || !data_dir[0]) { puts("SKIP DM1 HoC M648/C127/F0115: no data directory"); return 0; }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        puts("SKIP DM1 HoC M648/C127/F0115: DM1 data unavailable");
        M11_GameView_Shutdown(&state); return 0;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&item);
    if (!M11_GameView_GetBootProbeReceipt(&state, &boot) ||
        !state.originalFontAvailable || !state.originalFont.loaded ||
        !boot.dm1HoCLiveC127MaterialRequest || !boot.dm1HoCHallMirrorOverlay ||
        !item.valid || !boot.dm1HoCLiveF0115MaterialRequest) {
        puts("SKIP DM1 HoC M648/C127/F0115: no natural combined inscription/wall/item frame");
        M11_GameView_Shutdown(&state); return 0;
    }
    if (state.originalFont.graphicIndex <= 0 || item.transparentColor != 10 ||
        !item.usesF0791Blit || item.destinationW <= 0 || item.destinationH <= 0 ||
        M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(0) ||
        !M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(1)) {
        puts("FAIL DM1 HoC M648/C127/F0115 ownership or C10 contract");
        M11_GameView_Shutdown(&state); return 1;
    }
    if (!M11_GameView_ProbeDrawDm1ProjectileForFloorItemReceipt(&state, framebuffer, 320, 200)) {
        puts("SKIP DM1 HoC M648/C127/F0115: original projectile asset unavailable");
        M11_GameView_Shutdown(&state); return 0;
    }
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&item);
    if (!item.valid || M11_GameView_ProbeDm1HoCFloorItemCaptureObserved(0)) {
        puts("FAIL DM1 HoC M648/C127/F0115 projectile leaked into item lane");
        M11_GameView_Shutdown(&state); return 1;
    }
    printf("ok: DM1 HoC M648=%d C127 wall + F0115 item gfx=%d C10\n", state.originalFont.graphicIndex, item.graphicsId);
    M11_GameView_Shutdown(&state); return 0;
}
