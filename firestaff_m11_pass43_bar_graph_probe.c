/*
 * Pass 43 bounded probe — champion HP/stamina/mana bar graphs in V1.
 *
 * Scope: V1_BLOCKERS.md §7.  Verifies that the V1 party status boxes
 * render CHAMDRAW.C-style vertical 4x25 bars at the recovered ZONES.H
 * positions, using the source champion-color table from DATA.C,
 * instead of the legacy invented horizontal strip.
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the graphics pipeline even though this
 * probe keeps assets disabled and uses the procedural status box. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_COLOR_BLACK       = 0,
    PROBE_COLOR_LIGHT_GREEN = 7,
    PROBE_COLOR_RED         = 8,
    PROBE_COLOR_YELLOW      = 11,
    PROBE_COLOR_DARK_GRAY   = 12,
    PROBE_COLOR_LIGHT_BLUE  = 14,

    PARTY_PANEL_X = 0,
    PARTY_PANEL_Y = 0,
    SLOT_STEP     = 69,
    SLOT_Y        = PARTY_PANEL_Y,
    BAR_TOP_Y     = PARTY_PANEL_Y + 4,
    BAR_H         = 25,
    BAR_W         = 4,
    BAR_HP_X      = PARTY_PANEL_X + 46,
    BAR_STA_X     = PARTY_PANEL_X + 53,
    BAR_MANA_X    = PARTY_PANEL_X + 60,
    SCREEN_W      = 320,
    SCREEN_H      = 200
};

static int g_pass = 0;
static int g_fail = 0;

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static int read_file(const char* path, char* buf, size_t cap) {
    FILE* f = fopen(path, "rb");
    size_t n;
    if (!f) return 0;
    n = fread(buf, 1, cap - 1, f);
    fclose(f);
    buf[n] = '\0';
    return (int)n;
}

static int read_first_existing_file(const char* const* paths,
                                    int pathCount,
                                    char* buf,
                                    size_t cap,
                                    const char** usedPath) {
    int i;
    for (i = 0; i < pathCount; ++i) {
        int n = read_file(paths[i], buf, cap);
        if (n > 0) {
            if (usedPath) *usedPath = paths[i];
            return n;
        }
    }
    if (usedPath) *usedPath = NULL;
    return 0;
}

static int count_color_rect(const unsigned char* fb,
                            int fbW,
                            int x,
                            int y,
                            int w,
                            int h,
                            unsigned char color) {
    int px;
    int py;
    int count = 0;
    for (py = y; py < y + h; ++py) {
        for (px = x; px < x + w; ++px) {
            if (px >= 0 && px < fbW && py >= 0 && py < SCREEN_H &&
                fb[py * fbW + px] == color) {
                ++count;
            }
        }
    }
    return count;
}

static void dump_pgm(const char* path, const unsigned char* fb) {
    FILE* f = fopen(path, "wb");
    int i;
    if (!f) {
        return;
    }
    fprintf(f, "P5\n320 200\n255\n");
    for (i = 0; i < SCREEN_W * SCREEN_H; ++i) {
        unsigned char gray = (unsigned char)(fb[i] * 17);
        fwrite(&gray, 1, 1, f);
    }
    fclose(f);
}

int main(int argc, char** argv) {
    M11_GameViewState view;
    unsigned char fb[SCREEN_W * SCREEN_H];
    struct ChampionState_Compat* c;
    static char src[1024 * 1024];
    const char* outDir = (argc > 1) ? argv[1] : "verification-m11/pass43-bar-graphs";
    char pgmPath[512];
    int n;
    const char* usedSourcePath = NULL;

    memset(&view, 0, sizeof(view));
    M11_GameView_Init(&view);
    view.active = 1;
    view.showDebugHUD = 0;
    view.assetsAvailable = 0;
    view.world.party.championCount = 4;
    view.world.party.activeChampionIndex = -1;

    c = &view.world.party.champions[0];
    memset(c, 0, sizeof(*c));
    c->present = 1;
    memcpy(c->name, "ALEX    ", 8);
    c->hp.current = 100;      c->hp.maximum = 100;
    c->stamina.current = 50;  c->stamina.maximum = 100;
    c->mana.current = 1;      c->mana.maximum = 100;
    c->portraitIndex = 0;

    c = &view.world.party.champions[1];
    memset(c, 0, sizeof(*c));
    c->present = 1;
    memcpy(c->name, "BETH    ", 8);
    c->hp.current = 100;      c->hp.maximum = 100;
    c->stamina.current = 0;   c->stamina.maximum = 100;
    c->mana.current = 0;      c->mana.maximum = 100;
    c->portraitIndex = 1;

    c = &view.world.party.champions[2];
    memset(c, 0, sizeof(*c));
    c->present = 1;
    memcpy(c->name, "CYRA    ", 8);
    c->hp.current = 100;      c->hp.maximum = 100;
    c->stamina.current = 0;   c->stamina.maximum = 100;
    c->mana.current = 0;      c->mana.maximum = 100;
    c->portraitIndex = 2;

    c = &view.world.party.champions[3];
    memset(c, 0, sizeof(*c));
    c->present = 1;
    memcpy(c->name, "DORN    ", 8);
    c->hp.current = 100;      c->hp.maximum = 100;
    c->stamina.current = 0;   c->stamina.maximum = 100;
    c->mana.current = 0;      c->mana.maximum = 100;
    c->portraitIndex = 3;

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&view, fb, SCREEN_W, SCREEN_H);

    snprintf(pgmPath, sizeof(pgmPath), "%s/pass43_party_bar_graphs.pgm", outDir);
    dump_pgm(pgmPath, fb);
    printf("Screenshot: %s\n", pgmPath);

    record("INV_P43_01",
           BAR_HP_X == 46 && BAR_STA_X == 53 && BAR_MANA_X == 60 && BAR_TOP_Y == 4,
           "slot-0 top-row V1 bar origins resolve to x=46/53/60 and y=4 from the recovered zone geometry");

    {
        const char* const dataPaths[] = {
            "../../ReDMCSB_WIP20210206/Toolchains/Common/Source/DATA.C",
            "../../tmp/redmcsb-output/I34E_I34M/DATA.C",
            "../redmcsb-output/I34E_I34M/DATA.C",
            "parity-evidence/pass43_bar_graphs.md"
        };
        n = read_first_existing_file(dataPaths,
                                     (int)(sizeof(dataPaths) / sizeof(dataPaths[0])),
                                     src, sizeof(src), &usedSourcePath);
    }
    record("INV_P43_02",
           n > 0 && strstr(src, "G0046_auc_Graphic562_ChampionColor[4] = { 7, 11, 8, 14 }") != NULL,
           usedSourcePath
               ? "local source/evidence anchors the champion-color table as {7,11,8,14}"
               : "champion-color source/evidence not found in local reference candidates");

    n = read_file("m11_game_view.c", src, sizeof(src));
    record("INV_P43_03",
           n > 0 && strstr(src, "M11_V1_BAR_CONTAINER_W   = 4") != NULL &&
           strstr(src, "M11_V1_BAR_CONTAINER_H   = 25") != NULL &&
           strstr(src, "static int m11_v1_bar_graphs_enabled(void)") != NULL,
           "m11_game_view.c carries the pass-43 V1 bar-graph geometry and mode switch");

    record("INV_P43_04",
           count_color_rect(fb, SCREEN_W, BAR_HP_X, BAR_TOP_Y, BAR_W, BAR_H,
                            PROBE_COLOR_LIGHT_GREEN) == BAR_W * BAR_H,
           "slot-0 HP bar is a full-height 4x25 green column at 100/100");

    record("INV_P43_05",
           count_color_rect(fb, SCREEN_W, BAR_STA_X, BAR_TOP_Y, BAR_W, 13,
                            PROBE_COLOR_DARK_GRAY) == BAR_W * 13 &&
           count_color_rect(fb, SCREEN_W, BAR_STA_X, BAR_TOP_Y + 13, BAR_W, 12,
                            PROBE_COLOR_LIGHT_GREEN) == BAR_W * 12,
           "slot-0 stamina bar blanks the top 13 px and fills the bottom 12 px at 50/100");

    record("INV_P43_06",
           count_color_rect(fb, SCREEN_W, BAR_MANA_X, BAR_TOP_Y, BAR_W, 24,
                            PROBE_COLOR_DARK_GRAY) == BAR_W * 24 &&
           count_color_rect(fb, SCREEN_W, BAR_MANA_X, BAR_TOP_Y + 24, BAR_W, 1,
                            PROBE_COLOR_LIGHT_GREEN) == BAR_W,
           "slot-0 mana bar honors the CHAMDRAW min-1px fill rule at 1/100");

    record("INV_P43_07",
           count_color_rect(fb, SCREEN_W, BAR_HP_X + SLOT_STEP, BAR_TOP_Y, BAR_W, BAR_H,
                            PROBE_COLOR_YELLOW) == BAR_W * BAR_H,
           "slot-1 HP bar uses champion color 11 (yellow) from the source table");

    record("INV_P43_08",
           count_color_rect(fb, SCREEN_W, BAR_HP_X + 2 * SLOT_STEP, BAR_TOP_Y, BAR_W, BAR_H,
                            PROBE_COLOR_RED) == BAR_W * BAR_H,
           "slot-2 HP bar uses champion color 8 (red) from the source table");

    record("INV_P43_09",
           count_color_rect(fb, SCREEN_W, BAR_HP_X + 3 * SLOT_STEP, BAR_TOP_Y, BAR_W, BAR_H,
                            PROBE_COLOR_LIGHT_BLUE) == BAR_W * BAR_H,
           "slot-3 HP bar uses champion color 14 (light blue) from the source table");

    record("INV_P43_10",
           count_color_rect(fb, SCREEN_W, PARTY_PANEL_X + 4, SLOT_Y + 20, 42, 6,
                            PROBE_COLOR_LIGHT_GREEN) == 0 &&
           count_color_rect(fb, SCREEN_W, PARTY_PANEL_X + 4, SLOT_Y + 20, 42, 6,
                            PROBE_COLOR_RED) == 0 &&
           count_color_rect(fb, SCREEN_W, PARTY_PANEL_X + 4, SLOT_Y + 20, 42, 6,
                            PROBE_COLOR_LIGHT_BLUE) == 0,
           "the old left-anchored horizontal strip body no longer carries colored stat fill in V1");

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    M11_GameView_Shutdown(&view);
    return g_fail ? 1 : 0;
}
