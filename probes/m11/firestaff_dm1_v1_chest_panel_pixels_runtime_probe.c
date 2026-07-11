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
 *   ReDMCSB CHEST.C F0333 lines 30-32 returns before that P0694_B_PressingEye
 *   branch when the same G0426 chest is already open, so a redundant normal
 *   action-hand open must not convert an eye-opened chest into C145/arrow
 *   chrome.
 *   ReDMCSB CHEST.C F0333 lines 53-76 copies only the first eight linked
 *   contents into G0425 and draws C537..C544 chest slot boxes, including the
 *   eighth visible object while leaving a ninth linked tail item hidden.
 *   ReDMCSB CHEST.C F0334 lines 112-132 clears G0426 and rewrites the
 *   container from non-empty visible slots; after close the next inventory
 *   draw must return C101 to the normal inventory panel, not stale C025.
 *   ReDMCSB PANEL.C F0352 lines 2123-2159 routes an eye click with a
 *   leader-hand container through F0342/F0333 with P0707_B_PressingEye true,
 *   so the C025/C537 panel appears without repainting C09 to C145.
 *   ReDMCSB PANEL.C F0339 lines 505-514 and F0342 line 1472 draw C018
 *   arrow chrome for normal chest content, or C019 eye chrome when
 *   P0707_B_PressingEye is true, into C503 after panel contents.
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
    PROBE_VISIBLE_CHEST_SLOTS = 8,
    PROBE_CHAINED_CHEST_ITEMS = 9,
    PROBE_ACTION_HAND_SLOTBOX = 9,
    PROBE_OPEN_CHEST_PANEL_GRAPHIC = 25,
    PROBE_CHEST_CLOSED_ICON = 144,
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
                              const unsigned short items[PROBE_CHAINED_CHEST_ITEMS],
                              int itemCount)
{
    struct DungeonThings_Compat* things;
    int i;

    if (!game) return 0;
    things = game->world.things;
    if (!things || !things->loaded) {
        fprintf(stderr, "FAIL dungeon things unavailable\n");
        return 0;
    }
    if (!things->containers || things->containerCount < 1 ||
        !things->junks || things->junkCount < itemCount ||
        itemCount < 1) {
        fprintf(stderr, "FAIL container/junk records unavailable\n");
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = items[0];
    things->containers[0].type = 0;

    for (i = 0; i < itemCount; ++i) {
        memset(&things->junks[i], 0, sizeof(things->junks[i]));
        things->junks[i].next =
            (i + 1 < itemCount) ?
            items[i + 1] : THING_ENDOFLIST;
        things->junks[i].type = (unsigned char)((i % 2) + 1);
    }

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

static int seed_eye_runtime_chests(M11_GameViewState* game,
                                   unsigned short actionChestThing,
                                   unsigned short leaderChestThing,
                                   const unsigned short items[PROBE_VISIBLE_CHEST_SLOTS])
{
    struct DungeonThings_Compat* things;
    int i;

    if (!game) return 0;
    things = game->world.things;
    if (!things || !things->loaded ||
        !things->containers || things->containerCount < 2 ||
        !things->junks || things->junkCount < PROBE_VISIBLE_CHEST_SLOTS) {
        fprintf(stderr, "FAIL eye-path container/junk records unavailable\n");
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = THING_ENDOFLIST;
    things->containers[0].type = 0;

    memset(&things->containers[1], 0, sizeof(things->containers[1]));
    things->containers[1].next = THING_ENDOFLIST;
    things->containers[1].slot = items[0];
    things->containers[1].type = 0;

    for (i = 0; i < PROBE_VISIBLE_CHEST_SLOTS; ++i) {
        memset(&things->junks[i], 0, sizeof(things->junks[i]));
        things->junks[i].next =
            (i + 1 < PROBE_VISIBLE_CHEST_SLOTS) ?
            items[i + 1] : THING_ENDOFLIST;
        things->junks[i].type = (unsigned char)((i % 2) + 1);
    }

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
    game->v1OpenChestThing = THING_NONE;
    game->v1FoodWaterPanelActive = 0;
    seed_champion(&game->world.party.champions[0], actionChestThing);
    return M11_GameView_SetV1LeaderHandObject(game, leaderChestThing);
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

static int check_closed_panel_clears_c025(const M11_GameViewState* game,
                                          const unsigned char* openFb,
                                          const unsigned char* closedFb)
{
    const M11_AssetSlot* panel;
    int x = 0, y = 0, w = 0, h = 0;
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int openMatches;
    int diff;
    int ok = 1;

    ok &= expect_true("closed-panel viewport origin",
                      M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh));
    ok &= expect_true("closed-panel C101 inventory panel zone",
                      M11_GameView_GetV1InventoryPanelZone(&x, &y, &w, &h) &&
                      w > 0 && h > 0);
    panel = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 PROBE_OPEN_CHEST_PANEL_GRAPHIC);
    ok &= expect_true("closed-panel C025 open chest panel asset",
                      panel && panel->loaded && panel->pixels &&
                      panel->width == (unsigned short)w &&
                      panel->height == (unsigned short)h);
    if (!ok) return 0;

    openMatches = count_panel_matches(panel, openFb, vx + x, vy + y);
    diff = rect_diff_count(openFb, closedFb, vx + x, vy + y, w, h);
    ok &= expect_true("open C101 starts as source C025 chest panel",
                      openMatches > (w * h) / 2);
    ok &= expect_true("closed C101 repaints away from open chest panel pixels",
                      diff > (w * h) / 4);
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

static int check_arrow_or_eye_graphic(const M11_GameViewState* game,
                                      const unsigned char* fb,
                                      int pressingEye,
                                      const char* label)
{
    const M11_AssetSlot* graphic;
    int graphicId;
    int x = 0, y = 0, w = 0, h = 0;
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int matched = 0;
    int total = 0;
    int yy;
    int ok = 1;

    ok &= expect_true("C503 arrow/eye viewport origin",
                      M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh));
    ok &= expect_true("C503 arrow/eye zone",
                      M11_GameView_GetV1ArrowOrEyeZone(&x, &y, &w, &h) &&
                      w > 0 && h > 0);
    graphicId = M11_GameView_GetV1ArrowOrEyeGraphicId(pressingEye);
    graphic = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                   (unsigned int)graphicId);
    ok &= expect_true("C018/C019 arrow/eye source graphic",
                      graphic && graphic->loaded && graphic->pixels &&
                      graphic->width == (unsigned short)w &&
                      graphic->height == (unsigned short)h);
    if (!ok || !graphic || !graphic->pixels) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src = (unsigned char)
                (graphic->pixels[yy * (int)graphic->width + xx] & 0x0F);
            unsigned char dst = (unsigned char)
                M11_FB_DECODE_INDEX(fb[(vy + y + yy) * PROBE_FB_W +
                                       vx + x + xx]);
            if (src != 8) {
                ++total;
                if (src == dst) {
                    ++matched;
                }
            }
        }
    }
    if (!(total > 0 && matched == total)) {
        fprintf(stderr,
                "FAIL %s C503 pixels matched=%d total=%d graphic=%d\n",
                label, matched, total, graphicId);
        return 0;
    }
    printf("PASS %s C503 pixels matched=%d total=%d graphic=%d\n",
           label, matched, total, graphicId);
    return 1;
}

static int check_action_hand_chest_icon(const M11_GameViewState* game,
                                        const unsigned char* fb,
                                        int expectedIcon,
                                        const char* label)
{
    int x = 0, y = 0, w = 0, h = 0;
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int ok = 1;

    ok &= expect_true("open chest icon viewport origin",
                      M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh));
    ok &= expect_true("C09 action-hand slotbox zone",
                      M11_GameView_GetV1InventorySourceSlotBoxZone(
                          PROBE_ACTION_HAND_SLOTBOX, &x, &y, &w, &h) &&
                      w == 16 && h == 16);
    if (!ok) return 0;

    /* ReDMCSB CHEST.C F0333 lines 43-48 draws C145 into C09 before
     * blitting the open-chest C025 panel when the eye is not pressed.
     * PANEL.C F0352 lines 2123-2159 reaches F0333 with
     * P0707_B_PressingEye true, so the C09 action-hand slot must retain
     * its own icon instead of being repainted to C145. */
    return count_icon_matches(game, fb, expectedIcon, vx + x, vy + y, label);
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    unsigned char openFb[PROBE_FB_W * PROBE_FB_H];
    unsigned char closedFb[PROBE_FB_W * PROBE_FB_H];
    unsigned short chestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    unsigned short eyeChestThing = thing_ref(THING_TYPE_CONTAINER, 1);
    unsigned short items[PROBE_CHAINED_CHEST_ITEMS];
    int actionIcon;
    int itemAIcon;
    int itemBIcon;
    int itemHIcon;
    int itemTailIcon;
    int i;
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
    for (i = 0; i < PROBE_CHAINED_CHEST_ITEMS; ++i) {
        items[i] = thing_ref(THING_TYPE_JUNK, i);
    }

    if (!seed_runtime_chest(&game, chestThing, items,
                            PROBE_CHAINED_CHEST_ITEMS)) {
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
    memcpy(openFb, fb, sizeof(openFb));

    actionIcon = M11_GameView_GetV1InventorySlotIconIndex(
        &game, CHAMPION_SLOT_ACTION_HAND);
    ok &= expect_int("action-hand chest icon stays open",
                     actionIcon, PROBE_CHEST_OPEN_ICON);
    ok &= check_action_hand_chest_icon(&game, fb, PROBE_CHEST_OPEN_ICON,
                                       "action-hand open chest");
    itemAIcon = M11_GameView_GetObjectIconIndexForThing(&game, items[0]);
    itemBIcon = M11_GameView_GetObjectIconIndexForThing(&game, items[1]);
    itemHIcon = M11_GameView_GetObjectIconIndexForThing(
        &game, items[PROBE_VISIBLE_CHEST_SLOTS - 1]);
    itemTailIcon = M11_GameView_GetObjectIconIndexForThing(
        &game, items[PROBE_VISIBLE_CHEST_SLOTS]);
    ok &= expect_true("first visible chest item icon resolves", itemAIcon >= 0);
    ok &= expect_true("second visible chest item icon resolves", itemBIcon >= 0);
    ok &= expect_true("eighth visible chest item icon resolves", itemHIcon >= 0);
    ok &= expect_true("ninth hidden tail icon resolves", itemTailIcon >= 0);
    ok &= expect_true("ninth hidden tail icon differs from C544 item",
                      itemHIcon >= 0 && itemTailIcon >= 0 &&
                      itemHIcon != itemTailIcon);
    ok &= check_panel_pixels(&game, fb);
    ok &= check_arrow_or_eye_graphic(&game, fb, 0,
                                     "normal-open arrow for chest content");
    ok &= check_chest_slot_icon(&game, fb, 0, itemAIcon,
                                "first visible chest item");
    ok &= check_chest_slot_icon(&game, fb, 1, itemBIcon,
                                "second visible chest item");
    ok &= check_chest_slot_icon(&game, fb, PROBE_VISIBLE_CHEST_SLOTS - 1,
                                itemHIcon,
                                "eighth visible chest item at C544");

    M11_GameView_CloseV1OpenChest(&game);
    ok &= expect_int("closed G0426 bridge open chest thing",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)THING_NONE);
    actionIcon = M11_GameView_GetV1InventorySlotIconIndex(
        &game, CHAMPION_SLOT_ACTION_HAND);
    ok &= expect_int("closed action-hand chest icon returns to C144",
                     actionIcon, PROBE_CHEST_CLOSED_ICON);
    memset(closedFb, 0, sizeof(closedFb));
    M11_GameView_Draw(&game, closedFb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_action_hand_chest_icon(&game, closedFb,
                                       PROBE_CHEST_CLOSED_ICON,
                                       "post-close action-hand closed chest");
    ok &= check_closed_panel_clears_c025(&game, openFb, closedFb);

    ok &= expect_true("seed eye-path leader-hand chest with separate action chest",
                      seed_eye_runtime_chests(&game, chestThing, eyeChestThing,
                                              items));
    ok &= expect_true("eye click opens leader-hand chest panel",
                      M11_GameView_HandlePointer(&game, 12 + 8, 33 + 13 + 8, 1) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("G0426 bridge eye-open chest thing",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)eyeChestThing);
    actionIcon = M11_GameView_GetV1InventorySlotIconIndex(
        &game, CHAMPION_SLOT_ACTION_HAND);
    ok &= expect_int("eye-open leaves action-hand chest icon closed",
                     actionIcon, PROBE_CHEST_CLOSED_ICON);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_panel_pixels(&game, fb);
    ok &= check_arrow_or_eye_graphic(&game, fb, 1,
                                     "eye-open eye for object description");
    ok &= check_action_hand_chest_icon(&game, fb, PROBE_CHEST_CLOSED_ICON,
                                       "eye-open action-hand closed chest");
    ok &= check_chest_slot_icon(&game, fb, 0, itemAIcon,
                                "eye-open first visible chest item");
    ok &= check_chest_slot_icon(&game, fb, 1, itemBIcon,
                                "eye-open second visible chest item");
    ok &= check_chest_slot_icon(&game, fb, PROBE_VISIBLE_CHEST_SLOTS - 1,
                                itemHIcon,
                                "eye-open eighth visible chest item at C544");

    M11_GameView_CloseV1OpenChest(&game);
    ok &= expect_true("leader hand accepts same chest for pressing-eye route",
                      M11_GameView_SetV1LeaderHandObject(&game, chestThing));
    ok &= expect_true("pressing-eye chest open redraws panel",
                      M11_GameView_HandlePointer(&game, 12 + 8, 33 + 13 + 8, 1) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("pressing-eye keeps action-hand chest icon closed",
                     M11_GameView_GetV1InventorySlotIconIndex(
                         &game, CHAMPION_SLOT_ACTION_HAND),
                     144);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_panel_pixels(&game, fb);
    ok &= check_arrow_or_eye_graphic(&game, fb, 1,
                                     "same-action-hand pressing-eye eye");
    ok &= check_action_hand_chest_icon(
        &game, fb, PROBE_CHEST_CLOSED_ICON,
        "same-action-hand pressing-eye closed chest");

    M11_GameView_CloseV1OpenChest(&game);
    ok &= expect_true("leader hand accepts same chest for pressing-eye route",
                      M11_GameView_SetV1LeaderHandObject(&game, chestThing));
    ok &= expect_true("pressing-eye same action-hand chest redraws panel",
                      M11_GameView_HandlePointer(&game, 12 + 8, 33 + 13 + 8, 1) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("pressing-eye keeps same action-hand chest icon closed",
                     M11_GameView_GetV1InventorySlotIconIndex(
                         &game, CHAMPION_SLOT_ACTION_HAND),
                     PROBE_CHEST_CLOSED_ICON);
    /* ReDMCSB CHEST.C F0333 lines 30-32 returns before lines 43-48 can draw
     * C145 or reinterpret P0694_B_PressingEye.  Keep the runtime bridge's
     * eye-open provenance stable across a redundant normal open request. */
    ok &= expect_true("same-open normal action-hand request is accepted",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("same-open preserves eye-open provenance",
                     game.v1OpenChestOpenedByEye, 1);
    ok &= expect_int("same-open keeps action-hand chest icon closed",
                     M11_GameView_GetV1InventorySlotIconIndex(
                         &game, CHAMPION_SLOT_ACTION_HAND),
                     PROBE_CHEST_CLOSED_ICON);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_arrow_or_eye_graphic(&game, fb, 1,
                                     "same-open redundant request keeps eye");
    ok &= check_action_hand_chest_icon(
        &game, fb, PROBE_CHEST_CLOSED_ICON,
        "same-open redundant request keeps closed chest");
    M11_GameView_CloseV1OpenChest(&game);
    ok &= expect_int("closing pressing-eye chest clears G0426",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)THING_NONE);
    ok &= expect_true("action-hand reopen after pressing-eye close succeeds",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("action-hand reopen after eye close remaps C09 to C145",
                     M11_GameView_GetV1InventorySlotIconIndex(
                         &game, CHAMPION_SLOT_ACTION_HAND),
                     PROBE_CHEST_OPEN_ICON);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_action_hand_chest_icon(
        &game, fb, PROBE_CHEST_OPEN_ICON,
        "post-eye-close action-hand reopened chest");

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 chest panel runtime pixel probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
