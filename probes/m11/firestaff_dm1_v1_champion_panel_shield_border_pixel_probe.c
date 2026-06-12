/*
 * DM1 V1 champion panel shield-border runtime pixel probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, renders deterministic champion
 * status boxes through the real M11 V1 draw stack, and checks the isolated
 * C037/C038/C039 shield-border asset slices. It does not claim original DOS
 * screenshot parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292 draws the alive C151..C154 status-box fill,
 *   then draws active C037/C038/C039 shield borders before the later
 *   name, bar, and hand-slot children overdraw their sub-zones.
 *   ReDMCSB COORD.C/layout-696 anchors C151..C154 as 67x29 boxes on a
 *   69px stride.
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
    PROBE_CHAMPION_COUNT = 4
};

typedef struct ProbeRect {
    int x;
    int y;
    int w;
    int h;
} ProbeRect;

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * width + x]);
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

static int point_in_rect(int x, int y, const ProbeRect* rect) {
    return rect && x >= rect->x && y >= rect->y &&
           x < rect->x + rect->w && y < rect->y + rect->h;
}

static void seed_champion(struct ChampionState_Compat* champ,
                          const char* name,
                          int portraitIndex,
                          int direction,
                          int hp,
                          int stamina,
                          int mana) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->portraitIndex = portraitIndex;
    champ->direction = direction;
    champ->hp.current = (unsigned short)hp;
    champ->hp.maximum = 100;
    champ->stamina.current = (unsigned short)stamina;
    champ->stamina.maximum = 80;
    champ->mana.current = (unsigned short)mana;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game) {
    int slot;
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
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        game->championDamageTimer[slot] = 0;
        game->championDamageAmount[slot] = 0;
    }

    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH, 100, 80, 60);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST, 80, 65, 45);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH, 60, 48, 30);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST, 40, 32, 15);
}

static int collect_child_overdraw_rects(int slot,
                                        ProbeRect* rects,
                                        int rectCapacity) {
    int count = 0;
    int stat;
    int hand;
    int x, y, w, h;

    if (rectCapacity < 6) {
        return 0;
    }
    if (M11_GameView_GetV1StatusNameZone(slot, &x, &y, &w, &h)) {
        rects[count].x = x;
        rects[count].y = y;
        rects[count].w = w;
        rects[count].h = h;
        ++count;
    }
    for (stat = 0; stat < 3; ++stat) {
        if (M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h)) {
            rects[count].x = x;
            rects[count].y = y;
            rects[count].w = w;
            rects[count].h = h;
            ++count;
        }
    }
    for (hand = 0; hand < 2; ++hand) {
        if (M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                    &x, &y, &w, &h)) {
            rects[count].x = x;
            rects[count].y = y;
            rects[count].w = w;
            rects[count].h = h;
            ++count;
        }
    }
    return count;
}

static int pixel_is_later_child(int x,
                                int y,
                                const ProbeRect* childRects,
                                int childRectCount) {
    int i;
    for (i = 0; i < childRectCount; ++i) {
        if (point_in_rect(x, y, &childRects[i])) {
            return 1;
        }
    }
    return 0;
}

static int check_shield_frame(M11_GameViewState* game,
                              unsigned char* fb,
                              const char* labelPrefix,
                              int expectedGfx) {
    const int slot = 0;
    int x, y, w, h;
    int count;
    int gfx;
    const M11_AssetSlot* asset;
    ProbeRect childRects[6];
    int childRectCount;
    int expected = 0;
    int matched = 0;
    int yy;
    int ok = 1;
    char label[160];

    memset(fb, 0, PROBE_FB_W * PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    snprintf(label, sizeof(label), "%s shield count", labelPrefix);
    count = M11_GameView_GetV1StatusShieldBorderGraphicCountForChampion(
        game, slot);
    ok &= expect_int(label, count, 1);
    snprintf(label, sizeof(label), "%s shield graphic id", labelPrefix);
    gfx = M11_GameView_GetV1StatusShieldBorderGraphicForChampionAt(
        game, slot, 0);
    ok &= expect_int(label, gfx, expectedGfx);
    snprintf(label, sizeof(label), "%s shield border zone", labelPrefix);
    ok &= expect_true(label, M11_GameView_GetV1StatusShieldBorderZone(
                                  slot, &x, &y, &w, &h) &&
                             w == 67 && h == 29);
    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)expectedGfx);
    snprintf(label, sizeof(label), "%s shield asset loaded", labelPrefix);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->width == 67 && asset->height == 29);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    childRectCount = collect_child_overdraw_rects(slot, childRects, 6);
    snprintf(label, sizeof(label), "%s child overdraw rects", labelPrefix);
    ok &= expect_int(label, childRectCount, 6);
    if (!ok) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src;
            unsigned char dst;
            if (pixel_is_later_child(x + xx, y + yy,
                                     childRects, childRectCount)) {
                continue;
            }
            src = (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0F);
            if (src == 0) {
                continue;
            }
            dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++expected;
            if (dst == src) {
                ++matched;
            }
        }
    }

    snprintf(label, sizeof(label), "%s shield unobscured asset pixels",
             labelPrefix);
    ok &= expect_true(label, expected > 20 && matched * 100 >= expected * 95);
    printf("%s gfx=%d matched=%d/%d unobscured pixels\n",
           labelPrefix, expectedGfx, matched, expected);
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

    game.world.magic.partyShieldDefense = 4;
    game.world.magic.spellShieldDefense = 0;
    game.world.magic.fireShieldDefense = 0;
    ok &= check_shield_frame(&game, fb, "party shield C037",
                             M11_GameView_GetV1PartyShieldBorderGraphicId());

    game.world.magic.partyShieldDefense = 0;
    game.world.magic.spellShieldDefense = 4;
    game.world.magic.fireShieldDefense = 0;
    ok &= check_shield_frame(&game, fb, "spell shield C039",
                             M11_GameView_GetV1SpellShieldBorderGraphicId());

    game.world.magic.partyShieldDefense = 0;
    game.world.magic.spellShieldDefense = 0;
    game.world.magic.fireShieldDefense = 4;
    ok &= check_shield_frame(&game, fb, "fire shield C038",
                             M11_GameView_GetV1FireShieldBorderGraphicId());

    game.world.magic.partyShieldDefense = 4;
    game.world.magic.spellShieldDefense = 4;
    game.world.magic.fireShieldDefense = 4;
    ok &= expect_int("shield stack draw count",
                     M11_GameView_GetV1StatusShieldBorderGraphicCountForChampion(
                         &game, 0),
                     3);
    ok &= expect_int("shield stack first draw is party",
                     M11_GameView_GetV1StatusShieldBorderGraphicForChampionAt(
                         &game, 0, 0),
                     M11_GameView_GetV1PartyShieldBorderGraphicId());
    ok &= expect_int("shield stack second draw is spell",
                     M11_GameView_GetV1StatusShieldBorderGraphicForChampionAt(
                         &game, 0, 1),
                     M11_GameView_GetV1SpellShieldBorderGraphicId());
    ok &= expect_int("shield stack third draw is fire",
                     M11_GameView_GetV1StatusShieldBorderGraphicForChampionAt(
                         &game, 0, 2),
                     M11_GameView_GetV1FireShieldBorderGraphicId());

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel shield-border runtime pixel probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
