/*
 * DM1 V1 champion panel bar-graph runtime pixel probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, renders deterministic live/dead
 * champion status boxes through the real M11 V1 draw stack, and checks the
 * HP/stamina/mana bar graph pixel slices inside the party HUD/status boxes.
 * It does not claim original DOS screenshot parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0287 lines 72-155 computes three 25px bar heights
 *   from current/max HP, stamina, and mana.
 *   ReDMCSB CHAMDRAW.C F0292 lines 771-905 redraws live C151..C154 status
 *   boxes, then calls F0287 for live champions only.
 *   ReDMCSB CHAMDRAW.C F0292 lines 816-842 blits C008 for dead champions
 *   and branches past F0287, leaving the dead-box asset unobscured.
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
    PROBE_CHAMPION_COUNT = 4,
    PROBE_STALE_PIXEL = M11_FB_ENCODE(0, 15)
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static int count_color(const unsigned char* fb,
                       int width,
                       int x,
                       int y,
                       int w,
                       int h,
                       int color) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if ((int)px_index(fb, width, x + xx, y + yy) == color) {
                ++count;
            }
        }
    }
    return count;
}

static int count_raw_pixel(const unsigned char* fb,
                           int width,
                           int x,
                           int y,
                           int w,
                           int h,
                           unsigned char rawPixel) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (fb[(y + yy) * width + x + xx] == rawPixel) {
                ++count;
            }
        }
    }
    return count;
}

static int expect_true(const char* label, int ok) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static int expected_fill_height(int current, int maximum, int fullHeight) {
    long scaled;
    if (current <= 0 || maximum <= 0 || fullHeight <= 0) {
        return 0;
    }
    scaled = (long)fullHeight * (long)current / (long)maximum;
    if (scaled < 1) {
        scaled = 1;
    }
    if (scaled > fullHeight) {
        scaled = fullHeight;
    }
    return (int)scaled;
}

static void seed_champion(struct ChampionState_Compat* champ,
                          const char* name,
                          int direction,
                          int hp,
                          int hpMax,
                          int stamina,
                          int staminaMax,
                          int mana,
                          int manaMax) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->direction = direction;
    champ->portraitIndex = direction & 3;
    champ->hp.current = (unsigned short)hp;
    champ->hp.maximum = (unsigned short)hpMax;
    champ->stamina.current = (unsigned short)stamina;
    champ->stamina.maximum = (unsigned short)staminaMax;
    champ->mana.current = (unsigned short)mana;
    champ->mana.maximum = (unsigned short)manaMax;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game) {
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->resting = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;

    seed_champion(&game->world.party.champions[0],
                  "TIGGY", DIR_NORTH, 100, 100, 80, 80, 60, 60);
    seed_champion(&game->world.party.champions[1],
                  "HALK", DIR_EAST, 50, 100, 40, 80, 30, 60);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", DIR_SOUTH, 1, 100, 1, 80, 0, 60);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", DIR_WEST, 0, 100, 0, 80, 0, 60);
}

static int champion_stat_current(const struct ChampionState_Compat* champ,
                                 int statIndex) {
    if (statIndex == 0) return (int)champ->hp.current;
    if (statIndex == 1) return (int)champ->stamina.current;
    return (int)champ->mana.current;
}

static int champion_stat_maximum(const struct ChampionState_Compat* champ,
                                 int statIndex) {
    if (statIndex == 0) return (int)champ->hp.maximum;
    if (statIndex == 1) return (int)champ->stamina.maximum;
    return (int)champ->mana.maximum;
}

static int check_live_bar_graph(const M11_GameViewState* game,
                                const unsigned char* fb,
                                int slot,
                                int statIndex) {
    const struct ChampionState_Compat* champ = &game->world.party.champions[slot];
    int x, y, w, h;
    int fillHeight;
    int blankHeight;
    int fillColor = M11_GameView_GetV1ChampionBarColor(slot);
    int blankColor = M11_GameView_GetV1StatusBarBlankColor();
    int ok = 1;
    char label[160];

    snprintf(label, sizeof(label), "slot%d stat%d bar zone", slot, statIndex);
    ok &= expect_true(label,
                      M11_GameView_GetV1StatusBarZone(slot, statIndex,
                                                      &x, &y, &w, &h) &&
                      w == 4 && h == 25);
    if (!ok) return 0;

    fillHeight = expected_fill_height(champion_stat_current(champ, statIndex),
                                      champion_stat_maximum(champ, statIndex),
                                      h);
    blankHeight = h - fillHeight;

    snprintf(label, sizeof(label), "slot%d stat%d no stale bar pixels",
             slot, statIndex);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, x, y, w, h,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);

    if (blankHeight > 0) {
        snprintf(label, sizeof(label), "slot%d stat%d blank top",
                 slot, statIndex);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     x, y, w, blankHeight, blankColor),
                         w * blankHeight);
    }
    if (fillHeight > 0) {
        snprintf(label, sizeof(label), "slot%d stat%d colored bottom",
                 slot, statIndex);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     x, y + blankHeight, w, fillHeight,
                                     fillColor),
                         w * fillHeight);
    } else {
        snprintf(label, sizeof(label), "slot%d stat%d zero-value all blank",
                 slot, statIndex);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     x, y, w, h, blankColor),
                         w * h);
    }

    printf("slot%d stat%d current=%d max=%d fill=%d blank=%d\n",
           slot, statIndex,
           champion_stat_current(champ, statIndex),
           champion_stat_maximum(champ, statIndex),
           fillHeight, blankHeight);
    return ok;
}

static int check_dead_slot_has_no_bar_graph_overdraw(const M11_GameViewState* game,
                                                     const unsigned char* fb,
                                                     int slot) {
    int stat;
    int ok = 1;
    char label[160];

    for (stat = 0; stat < 3; ++stat) {
        int x, y, w, h;
        int fillColor = M11_GameView_GetV1ChampionBarColor(slot);
        snprintf(label, sizeof(label), "dead slot%d stat%d bar zone",
                 slot, stat);
        ok &= expect_true(label,
                          M11_GameView_GetV1StatusBarZone(slot, stat,
                                                          &x, &y, &w, &h) &&
                          w == 4 && h == 25);
        if (!ok) return 0;

        snprintf(label, sizeof(label),
                 "dead slot%d stat%d no champion-color bar overdraw",
                 slot, stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W, x, y, w, h, fillColor),
                         0);
    }

    (void)game;
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!game.assetsAvailable) {
        fprintf(stderr,
                "FAIL DM1 V1 GRAPHICS.DAT assets unavailable from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    seed_party(&game);
    memset(fb, PROBE_STALE_PIXEL, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    ok &= check_live_bar_graph(&game, fb, 0, 0);
    ok &= check_live_bar_graph(&game, fb, 0, 1);
    ok &= check_live_bar_graph(&game, fb, 0, 2);
    ok &= check_live_bar_graph(&game, fb, 1, 0);
    ok &= check_live_bar_graph(&game, fb, 1, 1);
    ok &= check_live_bar_graph(&game, fb, 1, 2);
    ok &= check_live_bar_graph(&game, fb, 2, 0);
    ok &= check_live_bar_graph(&game, fb, 2, 1);
    ok &= check_live_bar_graph(&game, fb, 2, 2);
    ok &= check_dead_slot_has_no_bar_graph_overdraw(&game, fb, 3);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel bar-graph pixel-slice probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
