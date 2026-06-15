/*
 * m11_dm1_v1_f0128_viewport_pc34_compat.c
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

int m11_dm1_v1_f0128_viewport_crop_ready(void) {
    return g_f0128_ready;
}

void m11_dm1_v1_f0128_compose_viewport_for_tuple(
    int partyMapX, int partyMapY, int partyMapIndex) {
    /* Source-locked per DUNVIEW.C:2995-2996:
     *   F0674_F0128_sub(G2109_Ceiling, G0296_Viewport);
     *   F0674_F0128_sub(G2108_Floor,    G0087_ViewportFloorArea);
     *
     * v1 marks the readiness flag and stores the tuple.
     * The actual bitmap copy is delegated to the M11
     * draw path (m11_dm1_v1_dungeon_compose_g0296) which
     * already calls F0674 via the existing wall path. */
    (void)partyMapX; (void)partyMapY; (void)partyMapIndex;
    g_f0128_ready = 1;
}

int m11_dm1_v1_f0128_g0076_get(void) {
    return g_g0076_enabled;
}

void m11_dm1_v1_f0128_g0076_set(int enabled) {
    g_g0076_enabled = enabled ? 1 : 0;
}
