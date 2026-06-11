/*
 * DM1 V1 open-chest action-hand pickup close runtime pixel probe.
 *
 * Firestaff-side runtime evidence: opens a seeded action-hand chest, renders
 * C145 in the C09 action-hand slot, clicks that visible slot through the real
 * pointer/COMMAND route, then proves the chest panel closes and the container
 * moves into the leader hand.
 *
 * Source evidence:
 *   ReDMCSB CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 43-48 sets
 *   G0426_T_OpenChest and draws C145 into C09 when opening from the action
 *   hand without the eye route.
 *   ReDMCSB CHAMPION.C F0300/F0302 lines 513-562 remove the clicked
 *   inventory action-hand object and call F0334 when it is the open chest.
 *   ReDMCSB CHEST.C F0334_INVENTORY_CloseChest lines 112-132 clears G0426
 *   and rebuilds the container from the visible G0425 chest slots.
 *   ReDMCSB COMMAND.C F0359 lines 2174-2176 dispatches C028..C065 slot-box
 *   commands to CHAMPION.C F0302.
 */
#include "asset_loader_m11.h"
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
    PROBE_ACTION_HAND_SOURCE_SLOTBOX = 9,
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
    memcpy(champ->name, "TIGGY   ", 8);
    champ->portraitIndex = 0;
    champ->direction = DIR_NORTH;
    champ->hp.current = 90;
    champ->hp.maximum = 100;
    champ->stamina.current = 80;
    champ->stamina.maximum = 100;
    champ->mana.current = 40;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;
}

static int seed_records(M11_GameViewState* game,
                        unsigned short chestThing,
                        unsigned short itemThing)
{
    struct DungeonThings_Compat* things;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->junks || things->junkCount < 1) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = itemThing;
    things->containers[0].type = 0;

    memset(&things->junks[0], 0, sizeof(things->junks[0]));
    things->junks[0].type = 1;
    things->junks[0].next = THING_ENDOFLIST;

    seed_champion(&game->world.party.champions[0], chestThing);
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->inventoryPanelActive = 1;
    game->showDebugHUD = 0;
    game->spellPanelOpen = 0;
    game->v1FoodWaterPanelActive = 0;
    M11_GameView_ClearV1LeaderHandObject(game);
    return 1;
}

static int rect_diff_count(const unsigned char* a,
                           const unsigned char* b,
                           int x,
                           int y,
                           int w,
                           int h)
{
    int diff = 0;
    int yy;

    if (!a || !b) return 0;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (a[(y + yy) * PROBE_FB_W + x + xx] !=
                b[(y + yy) * PROBE_FB_W + x + xx]) {
                ++diff;
            }
        }
    }
    return diff;
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

static int count_icon_matches(const M11_GameViewState* game,
                              const unsigned char* fb,
                              int iconIndex,
                              int dstX,
                              int dstY,
                              const char* label)
{
    const M11_AssetSlot* iconSheet;
    int graphicIndex = 0;
    int srcX = 0;
    int srcY = 0;
    int srcW = 0;
    int srcH = 0;
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

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char openFb[PROBE_FB_W * PROBE_FB_H];
    unsigned char closedFb[PROBE_FB_W * PROBE_FB_H];
    const unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short itemThing = thing_ref(THING_TYPE_JUNK, 0);
    const M11_AssetSlot* openPanel;
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int slotX = 0, slotY = 0, slotW = 0, slotH = 0;
    int panelX = 0, panelY = 0, panelW = 0, panelH = 0;
    int commandSpace = 0;
    int commandZone = 0;
    int openPanelMatches;
    int closedPanelMatches;
    int panelDiff;
    int actionSlotDiff;
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

    ok &= expect_true("source-backed chest and junk records available",
                      seed_records(&game, chestThing, itemThing));
    ok &= expect_true("open seeded action-hand chest",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("open chest bridge thing",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)chestThing);
    ok &= expect_true("viewport zone available",
                      M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh));
    ok &= expect_true("C09 action-hand source slotbox zone available",
                      M11_GameView_GetV1InventorySourceSlotBoxZone(
                          PROBE_ACTION_HAND_SOURCE_SLOTBOX,
                          &slotX, &slotY, &slotW, &slotH) &&
                      slotW == 16 && slotH == 16);
    ok &= expect_true("C101 inventory panel zone available",
                      M11_GameView_GetV1InventoryPanelZone(
                          &panelX, &panelY, &panelW, &panelH) &&
                      panelW > 0 && panelH > 0);
    openPanel = M11_AssetLoader_Load(&game.assetLoader,
                                     PROBE_OPEN_CHEST_PANEL_GRAPHIC);
    ok &= expect_true("C025 open chest panel asset available",
                      openPanel && openPanel->loaded && openPanel->pixels &&
                      (int)openPanel->width == panelW &&
                      (int)openPanel->height == panelH);
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    memset(openFb, 0, sizeof(openFb));
    M11_GameView_Draw(&game, openFb, PROBE_FB_W, PROBE_FB_H);
    ok &= count_icon_matches(&game, openFb, PROBE_CHEST_OPEN_ICON,
                             vx + slotX, vy + slotY,
                             "pre-click C09 action-hand C145");
    openPanelMatches = count_panel_matches(openPanel, openFb,
                                           vx + panelX, vy + panelY);
    ok &= expect_true("pre-click C025 panel pixels visible",
                      openPanelMatches > (panelW * panelH) / 2);

    ok &= expect_int("click resolves to C029/C508 action hand",
                     M11_GameView_GetV1MouseCommandForPoint(
                         M11_DM1_MOUSE_LIST_INVENTORY,
                         vx + slotX + slotW / 2,
                         vy + slotY + slotH / 2,
                         M11_DM1_MOUSE_MASK_LEFT,
                         &commandSpace,
                         &commandZone),
                     29);
    ok &= expect_int("click route zone C508", commandZone, 508);
    ok &= expect_int("click route viewport-relative",
                     commandSpace, M11_DM1_MOUSE_SPACE_VIEWPORT);
    ok &= expect_int("live C145 action-hand click redraws",
                     M11_GameView_HandlePointerButton(
                         &game,
                         vx + slotX + slotW / 2,
                         vy + slotY + slotH / 2,
                         M11_DM1_MOUSE_MASK_LEFT),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("click closed G0426 bridge thing",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)THING_NONE);
    ok &= expect_int("leader hand now carries clicked chest",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)chestThing);
    ok &= expect_int("action hand inventory slot is empty after pickup",
                     (int)game.world.party.champions[0]
                         .inventory[CHAMPION_SLOT_ACTION_HAND],
                     (int)THING_NONE);

    memset(closedFb, 0, sizeof(closedFb));
    M11_GameView_Draw(&game, closedFb, PROBE_FB_W, PROBE_FB_H);
    actionSlotDiff = rect_diff_count(openFb, closedFb,
                                     vx + slotX, vy + slotY, slotW, slotH);
    ok &= expect_true("post-click C09 no longer matches open C145 pixels",
                      actionSlotDiff > 32);
    closedPanelMatches = count_panel_matches(openPanel, closedFb,
                                             vx + panelX, vy + panelY);
    ok &= expect_true("post-click C025 source-pixel match drops",
                      closedPanelMatches < openPanelMatches);
    panelDiff = rect_diff_count(openFb, closedFb,
                                vx + panelX, vy + panelY, panelW, panelH);
    ok &= expect_true("post-click C101 panel has a visible redraw delta",
                      panelDiff > 32);

    printf("sourceEvidence=CHEST.C:F0333:43-48;CHAMPION.C:F0300/F0302:513-562;CHEST.C:F0334:112-132;COMMAND.C:F0359:2174-2176\n");
    printf("%s dm1 v1 chest action-hand pickup close runtime pixel probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
