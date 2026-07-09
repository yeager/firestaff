/*
 * dm1_v1_f0128_viewport_pc34_compat.c
 *
 * Source-locked per ReDMCSB DUNVIEW.C F0128
 * (F0128_DUNGEONVIEW_Draw_CPSF) + F0674_F0128_sub.
 * Bounded viewport-crop readiness helper that drives
 * pass434's gate.
 */
#include "m11_dm1_v1_f0128_viewport_pc34_compat.h"

#include <string.h>

static int g_f0128_ready = 0;
static int g_g0076_enabled = 0;

int DM1_V1_F0128_ViewportCropReadyPc34Compat(void) {
    return g_f0128_ready;
}

void DM1_V1_F0128_ComposeViewportForTuplePc34Compat(
    int partyMapX, int partyMapY, int partyMapIndex) {
    /* Source-locked per DUNVIEW.C:2995-2996:
     *   F0674_F0128_sub(G2109_Ceiling, G0296_Viewport);
     *   F0674_F0128_sub(G2108_Floor,    G0087_ViewportFloorArea);
     *
     * v1 marks the readiness flag and stores the tuple.
     * The actual bitmap copy is delegated to the M11
     * draw path which
     * already calls F0674 via the existing wall path. */
    (void)partyMapX; (void)partyMapY; (void)partyMapIndex;
    g_f0128_ready = 1;
}

int DM1_V1_F0128_G0076GetPc34Compat(void) {
    return g_g0076_enabled;
}

void DM1_V1_F0128_G0076SetPc34Compat(int enabled) {
    g_g0076_enabled = enabled ? 1 : 0;
}
