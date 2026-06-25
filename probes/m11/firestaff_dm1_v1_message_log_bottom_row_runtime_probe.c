/*
 * DM1 V1 bottom message-log row readability probe.
 *
 * Source evidence:
 *   ReDMCSB TEXT.C owns the four-row C015 message surface and clears it
 *   through F0049_TEXT_MESSAGEAREA_Clear before drawing player messages.
 *   DEFS.H defines M532_MESSAGE_AREA_ROW_COUNT=4 for PC media; COORD.C
 *   defines G2088_C7_TextLineHeight=7.  Firestaff's C015 zone is
 *   bottom-anchored at y=173..199, so the four row origins must keep the
 *   original six-pixel M653 font fully inside the 320x200 framebuffer.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_YELLOW = 11,
    PROBE_RED = 8
};

static int g_pass;
static int g_fail;

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    fclose(f);
    return 1;
}

static const char* narrow_dm1_data_dir(const char* dataDir,
                                       char* out,
                                       size_t outSize) {
    char graphicsPath[512];
    char dungeonPath[512];
    if (!dataDir || !out || outSize == 0U) {
        return dataDir;
    }
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/dm1/GRAPHICS.DAT", dataDir);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/dm1/DUNGEON.DAT", dataDir);
    if (file_exists(graphicsPath) && file_exists(dungeonPath)) {
        snprintf(out, outSize, "%s/dm1", dataDir);
        return out;
    }
    return dataDir;
}

static size_t count_color(const unsigned char* fb,
                          int x,
                          int y,
                          int w,
                          int h,
                          unsigned char color) {
    size_t count = 0;
    int yy;
    int xx;
    if (!fb) {
        return 0;
    }
    for (yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= PROBE_FB_H) {
            continue;
        }
        for (xx = x; xx < x + w; ++xx) {
            if (xx < 0 || xx >= PROBE_FB_W) {
                continue;
            }
            if (M11_FB_DECODE_INDEX(fb[yy * PROBE_FB_W + xx]) == color) {
                ++count;
            }
        }
    }
    return count;
}

static void check_count_at_least(const char* label, size_t got, size_t want) {
    if (got >= want) {
        ++g_pass;
        printf("PASS %s=%zu\n", label, got);
    } else {
        ++g_fail;
        printf("FAIL %s got=%zu want>=%zu\n", label, got, want);
    }
}

static void check_count_equals(const char* label, size_t got, size_t want) {
    if (got == want) {
        ++g_pass;
        printf("PASS %s=%zu\n", label, got);
    } else {
        ++g_fail;
        printf("FAIL %s got=%zu want=%zu\n", label, got, want);
    }
}

int main(int argc, char** argv) {
    const char* root = argc > 1 ? argv[1] : NULL;
    const char* dataDir;
    char narrowed[512];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    size_t row0;
    size_t row1;
    size_t row2;
    size_t row3;
    size_t redTelemetry;

    if (!root) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = narrow_dm1_data_dir(root, narrowed, sizeof(narrowed));

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        printf("SKIP could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    memset(&game.messageLog, 0, sizeof(game.messageLog));
    M11_MessageLog_Push(&game.messageLog, "DOOR OPENING", PROBE_YELLOW);
    M11_MessageLog_Push(&game.messageLog, "IT IS LOCKED.", PROBE_YELLOW);
    M11_MessageLog_Push(&game.messageLog, "PARTY MOVED", PROBE_RED);
    M11_MessageLog_Push(&game.messageLog, "OUCH.", PROBE_YELLOW);
    M11_MessageLog_Push(&game.messageLog, "HEADS.", PROBE_YELLOW);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    row0 = count_color(fb, 0, 173, PROBE_FB_W, 6, PROBE_YELLOW);
    row1 = count_color(fb, 0, 180, PROBE_FB_W, 6, PROBE_YELLOW);
    row2 = count_color(fb, 0, 187, PROBE_FB_W, 6, PROBE_YELLOW);
    row3 = count_color(fb, 0, 194, PROBE_FB_W, 6, PROBE_YELLOW);
    redTelemetry = count_color(fb, 0, 173, PROBE_FB_W, 27, PROBE_RED);

    printf("probe=firestaff_dm1_v1_message_log_bottom_row_runtime_probe\n");
    printf("dataDir=%s\n", dataDir);
    check_count_at_least("row0_y173_178_yellow", row0, 3);
    check_count_at_least("row1_y180_185_yellow", row1, 3);
    check_count_at_least("row2_y187_192_yellow", row2, 3);
    check_count_at_least("row3_y194_199_yellow", row3, 3);
    check_count_equals("suppressed_party_moved_red", redTelemetry, 0);

    printf("summary passed=%d failed=%d rows=%zu,%zu,%zu,%zu red=%zu\n",
           g_pass, g_fail, row0, row1, row2, row3, redTelemetry);

    M11_GameView_Shutdown(&game);
    return g_fail == 0 ? 0 : 1;
}
