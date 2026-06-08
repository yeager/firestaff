/*
 * DM1 V1 open-chest inventory panel runtime pixel probe.
 *
 * Firestaff-side evidence only. It opens the hash-verified DM1 V1 runtime
 * when local assets are available, seeds a deterministic action-hand chest,
 * opens it through the M11 V1 bridge, renders one frame, and exact-matches
 * visible panel/icon pixels against GRAPHICS.DAT-backed assets.
 *
 * Source evidence:
 *   ReDMCSB CHEST.C F0333 lines 43-48 sets G0426, draws the open action-hand
 *   chest icon unless the eye is pressed, then blits C025 open-chest panel.
 *   ReDMCSB CHEST.C F0333 lines 53-76 copies the first eight linked contents
 *   into G0425 and draws C537..C544 chest slot boxes.
 *   ReDMCSB PANEL.C F0347/F0354 routes the open-chest panel through the
 *   current inventory panel draw.
 */
#include "asset_loader_m11.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
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
    PROBE_CHAMPION_COUNT = 1,
    PROBE_OPEN_CHEST_PANEL_GRAPHIC = 25,
    PROBE_CHEST_OPEN_ICON = 145
};

static unsigned short thing_ref(int thingType, int thingIndex)
{
    return (unsigned short)(((thingType & 0x0F) << 10) |
                            (thingIndex & 0x03FF));
}

static int expect_true(const char* label, int ok)
{
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static void seed_champion(struct ChampionState_Compat* champ,
                          unsigned short chestThing)
{
    int i;

    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    memcpy(champ->name, "TIGGY", 5);
    champ->portraitIndex = 0;
    champ->direction = DIR_NORTH;
    champ->hp.current = 100;
    champ->hp.maximum = 100;
    champ->stamina.current = 80;
    champ->stamina.maximum = 80;
    champ->mana.current = 60;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;
}

static int seed_runtime_chest(M11_GameViewState* game,
                              unsigned short chestThing,
                              unsigned short itemA,
                              unsigned short itemB)
{
    struct DungeonThings_Compat* things;

    if (!game) return 0;
    things = game->world.things;
    if (!things || !things->loaded) {
        fprintf(stderr, "FAIL dungeon things unavailable\n");
        return 0;
    }
    if (!things->containers || things->containerCount < 1 ||
        !things->junks || things->junkCount < 2) {
        fprintf(stderr, "FAIL container/junk records unavailable\n");
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = itemA;
    things->containers[0].type = 0;

    memset(&things->junks[0], 0, sizeof(things->junks[0]));
    things->junks[0].next = itemB;
    things->junks[0].type = 1; /* OBJECT.C info 128 resolves to icon 8. */

    memset(&things->junks[1], 0, sizeof(things->junks[1]));
    things->junks[1].next = THING_ENDOFLIST;
    things->junks[1].type = 2; /* OBJECT.C info 129 resolves to icon 9. */

    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 1;
    game->spellPanelOpen = 0;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    seed_champion(&game->world.party.champions[0], chestThing);
    return 1;
}

static int count_panel_matches(const M11_AssetSlot* asset,
                               const unsigned char* fb,
                               int dstX,
                               int dstY)
{
    int matched = 0;
    int yy;

    if (!asset || !asset->pixels || !fb) return 0;
    for (yy = 0; yy < (int)asset->height; ++yy) {
        int xx;
        for (xx = 0; xx < (int)asset->width; ++xx) {
            unsigned char src =
                (unsigned char)(asset->pixels[yy * (int)asset->width + xx] &
                                0x0F);
            unsigned char dst = (unsigned char)
                M11_FB_DECODE_INDEX(fb[(dstY + yy) * PROBE_FB_W + dstX + xx]);
            if (src != 8 && src == dst) {
                ++matched;
            }
        }
    }
    return matched;
}

static int check_panel_pixels(const M11_GameViewState* game,
                              const unsigned char* fb)
{
    const M11_AssetSlot* panel;
    int x = 0, y = 0, w = 0, h = 0;
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int matches;
    int ok = 1;

    ok &= expect_true("V1 viewport origin",
                      M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh));
    ok &= expect_true("C101 inventory panel zone",
                      M11_GameView_GetV1InventoryPanelZone(&x, &y, &w, &h) &&
                      w > 0 && h > 0);
    panel = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 PROBE_OPEN_CHEST_PANEL_GRAPHIC);
    ok &= expect_true("C025 open chest panel asset",
                      panel && panel->loaded && panel->pixels &&
                      panel->width == (unsigned short)w &&
                      panel->height == (unsigned short)h);
    if (!ok) return 0;

    matches = count_panel_matches(panel, fb, vx + x, vy + y);
    ok &= expect_true("C025 open chest panel pixels match framebuffer",
                      matches > (w * h) / 2);
    return ok;
}

static int count_icon_matches(const M11_GameViewState* game,
                              const unsigned char* fb,
                              int iconIndex,
                              int dstX,
                              int dstY,
                              const char* label)
{
    const M11_AssetSlot* iconSheet;
    int graphicIndex = 0;
    int srcX = 0, srcY = 0, srcW = 0, srcH = 0;
    int matched = 0;
    int total = 0;
    int yy;
    int ok = 1;
    char zoneLabel[128];

    snprintf(zoneLabel, sizeof(zoneLabel), "%s icon source zone", label);
    ok &= expect_true(zoneLabel,
                      M11_GameView_GetV1ObjectIconSourceZone(iconIndex,
                                                             &graphicIndex,
                                                             &srcX, &srcY,
                                                             &srcW, &srcH) &&
                      srcW == 16 && srcH == 16);
    iconSheet = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                     (unsigned int)graphicIndex);
    snprintf(zoneLabel, sizeof(zoneLabel), "%s icon sheet", label);
    ok &= expect_true(zoneLabel,
                      iconSheet && iconSheet->loaded && iconSheet->pixels &&
                      srcX + srcW <= (int)iconSheet->width &&
                      srcY + srcH <= (int)iconSheet->height);
    if (!ok || !iconSheet || !iconSheet->pixels) {
        return 0;
    }

    for (yy = 0; yy < srcH; ++yy) {
        int xx;
        for (xx = 0; xx < srcW; ++xx) {
            unsigned char src = (unsigned char)
                (iconSheet->pixels[(srcY + yy) * (int)iconSheet->width +
                                   srcX + xx] & 0x0F);
            unsigned char dst = (unsigned char)
                M11_FB_DECODE_INDEX(fb[(dstY + yy) * PROBE_FB_W +
                                       dstX + xx]);
            if (src != 12) {
                ++total;
                if (src == dst) {
                    ++matched;
                }
            }
        }
    }
    snprintf(zoneLabel, sizeof(zoneLabel), "%s visible icon pixels", label);
    if (!(total > 0 && matched == total)) {
        fprintf(stderr, "FAIL %s matched=%d total=%d icon=%d dst=%d,%d\n",
                zoneLabel, matched, total, iconIndex, dstX, dstY);
        return 0;
    }
    printf("PASS %s matched=%d total=%d icon=%d\n",
           zoneLabel, matched, total, iconIndex);
    return 1;
}

static int check_chest_slot_icon(const M11_GameViewState* game,
                                 const unsigned char* fb,
                                 int chestOrdinal,
                                 int expectedIcon,
                                 const char* label)
{
    int x = 0, y = 0, w = 0, h = 0;
    int vx = 0, vy = 0, vw = 0, vh = 0;
    char zoneLabel[128];
    int ok = 1;

    snprintf(zoneLabel, sizeof(zoneLabel), "%s viewport origin", label);
    ok &= expect_true(zoneLabel,
                      M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh));
    snprintf(zoneLabel, sizeof(zoneLabel), "%s C537+Cn zone", label);
    ok &= expect_true(zoneLabel,
                      M11_GameView_GetV1ChestSlotBoxZone(chestOrdinal,
                                                         &x, &y, &w, &h) &&
                      w == 16 && h == 16);
    if (!ok) return 0;
    return count_icon_matches(game, fb, expectedIcon, vx + x, vy + y, label);
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    unsigned short itemA = thing_ref(THING_TYPE_JUNK, 0);
    unsigned short itemB = thing_ref(THING_TYPE_JUNK, 1);
    int actionIcon;
    int itemAIcon;
    int itemBIcon;
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
    if (!seed_runtime_chest(&game, chestThing, itemA, itemB)) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("open action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("G0426 bridge open chest thing",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)chestThing);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    actionIcon = M11_GameView_GetV1InventorySlotIconIndex(
        &game, CHAMPION_SLOT_ACTION_HAND);
    ok &= expect_int("action-hand chest icon stays open",
                     actionIcon, PROBE_CHEST_OPEN_ICON);
    itemAIcon = M11_GameView_GetObjectIconIndexForThing(&game, itemA);
    itemBIcon = M11_GameView_GetObjectIconIndexForThing(&game, itemB);
    ok &= expect_true("first visible chest item icon resolves", itemAIcon >= 0);
    ok &= expect_true("second visible chest item icon resolves", itemBIcon >= 0);
    ok &= check_panel_pixels(&game, fb);
    ok &= check_chest_slot_icon(&game, fb, 0, itemAIcon,
                                "first visible chest item");
    ok &= check_chest_slot_icon(&game, fb, 1, itemBIcon,
                                "second visible chest item");

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 chest panel runtime pixel probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
