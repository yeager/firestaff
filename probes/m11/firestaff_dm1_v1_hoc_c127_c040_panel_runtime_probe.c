/* Opt-in real-data HoC C127/C040/F0115 ownership probe.
 * ReDMCSB MOVESENS.C F0275 enters REVIVE.C F0280 for C127; C040 owns the
 * resurrect/reincarnate panel, while DUNVIEW.C F0115 keeps floor lanes apart. */
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
    if (!data_dir || !data_dir[0]) { puts("SKIP DM1 HoC C127/C040: no data directory"); return 0; }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        puts("SKIP DM1 HoC C127/C040: DM1 data unavailable"); M11_GameView_Shutdown(&state); return 0;
    }
    memset(framebuffer, 0, sizeof(framebuffer)); M11_GameView_Draw(&state, framebuffer, 320, 200);
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&item);
    if (!M11_GameView_GetBootProbeReceipt(&state, &boot) ||
        !boot.dm1HoCLiveC127MaterialRequest || !boot.dm1HoCHallMirrorOverlay ||
        !state.candidateMirrorPanelActive) {
        puts("SKIP DM1 HoC C127/C040: no naturally restored mirror panel");
        M11_GameView_Shutdown(&state); return 0;
    }
    if (state.candidateMirrorOrdinal < 0 || state.candidateMirrorPartyIndex < 0 ||
        item.valid || boot.dm1HoCLiveF0115MaterialRequest) {
        puts("FAIL DM1 HoC C127/C040 panel leaked into F0115 floor lane");
        M11_GameView_Shutdown(&state); return 1;
    }
    if (!M11_GameView_ProbeDrawDm1ProjectileForFloorItemReceipt(&state, framebuffer, 320, 200)) {
        puts("SKIP DM1 HoC C127/C040: original projectile asset unavailable");
        M11_GameView_Shutdown(&state); return 0;
    }
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&item);
    if (item.valid || !state.candidateMirrorPanelActive) {
        puts("FAIL DM1 HoC C127/C040 projectile disturbed wall panel ownership");
        M11_GameView_Shutdown(&state); return 1;
    }
    printf("ok: DM1 HoC C127 wall + C040 panel ordinal=%d party=%d F0115=0\n",
           state.candidateMirrorOrdinal, state.candidateMirrorPartyIndex);
    M11_GameView_Shutdown(&state); return 0;
}
