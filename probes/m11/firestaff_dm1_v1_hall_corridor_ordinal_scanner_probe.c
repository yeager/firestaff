/*
 * DM1 V1 Hall of Champions corridor ordinal scanner probe.
 *
 * Walks the Hall of Champions corridor at map0 and reports every
 * (x,y,dir) pose that returns a non-negative front-mirror ordinal.
 * This is a developer aid for verifying Hall ordinal coverage; it
 * is not a regression gate.  The companion ordinal-specific probes
 * (firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe and
 * its walker/zorder/candidate siblings) lock the source-cited
 * mirror positions individually.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB MOVESENS.C:1501-1503 routes a C127 click to F0280
 *   ReDMCSB REVIVE.C F0280 materializes the candidate from sensorData
 *
 * Output: one line per non-negative ordinal, in the form
 *   (mapX,mapY,DIR) ordinal=N
 * with DIR being N/E/S/W.  Corridor/negative poses are silent.
 *
 * The Hall corridor in the reference DM1 V1 DUNGEON.DAT is roughly
 * (x,y) in [0..3] x [0..7].  The probe walks that rectangle in all
 * four directions and the wider Hall rectangle [0..6] x [0..7] to
 * also catch ordinals the upstream scan missed.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static const char* dir_name(int dir) {
    switch (dir) {
        case 0: return "N";
        case 1: return "E";
        case 2: return "S";
        case 3: return "W";
        default: return "?";
    }
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int x, y, dir;
    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    M12_StartupMenu_InitWithDataDir(&menu, argv[1], NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", argv[1]);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    printf("=== DM1 V1 Hall of Champions corridor ordinal scanner ===\n");
    /* Walk a generous rectangle around the Hall corridor to surface
     * every C127 sensor ordinal reachable from a single-map pose. */
    for (y = 0; y < 12; ++y) {
        for (x = 0; x < 12; ++x) {
            for (dir = 0; dir < 4; ++dir) {
                game.world.party.mapIndex = 0;
                game.world.party.mapX = (int16_t)x;
                game.world.party.mapY = (int16_t)y;
                game.world.party.direction = (uint8_t)dir;
                game.showDebugHUD = 0;
                game.candidateMirrorPanelActive = 0;
                game.candidateMirrorOrdinal = -1;
                game.candidateMirrorPartyIndex = -1;
                int ord = M11_GameView_GetFrontMirrorOrdinal(&game);
                if (ord >= 0) {
                    char nameBuf[32];
                    nameBuf[0] = 0;
                    M11_GameView_GetMirrorNameByOrdinal(&game, ord, nameBuf, (int)sizeof(nameBuf));
                    printf("  (%2d,%2d,%s) ordinal=%d name=%s\n",
                           x, y, dir_name(dir), ord,
                           nameBuf[0] ? nameBuf : "<empty>");
                }
            }
        }
    }
    M11_GameView_Shutdown(&game);
    return 0;
}
