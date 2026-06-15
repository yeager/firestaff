/*
 * DM1 V1 open-chest eye object-description runtime probe.
 *
 * Firestaff-side runtime evidence: opens an action-hand chest with a hidden
 * ninth tail item, puts a weapon in the leader hand, clicks C546 through the
 * real M11 pointer route, then renders one frame.  ReDMCSB closes the current
 * G0426 chest before drawing a non-container object-description panel, so the
 * old chest must be rewritten from visible C537..C544 slots while C020/C029
 * object-description pixels replace stale C025 open-chest panel pixels.
 *
 * Source evidence:
 *   ReDMCSB PANEL.C F0352 lines 2123-2159 routes eye-with-leader-hand-object
 *   to F0342.
 *   ReDMCSB PANEL.C F0342 lines 1126-1138 closes an already-open chest,
 *   rejects scroll/container branches, and enters object-description mode for
 *   ordinary objects.
 *   ReDMCSB PANEL.C F0342 lines 1136-1145 blits C020 empty panel and C029
 *   object-description circle.
 *   ReDMCSB CHEST.C F0334 lines 112-132 rewrites the container from non-empty
 *   visible G0425/C537..C544 slots, truncating any hidden tail.
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
    PROBE_CHAIN_COUNT = 9,
    PROBE_VISIBLE_CHEST_SLOTS = 8,
    PROBE_EYE_X = 12 + 8,
    PROBE_EYE_Y = 33 + 13 + 8,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    PROBE_PANEL_EMPTY_GFX = 20,
    PROBE_PANEL_OPEN_CHEST_GFX = 25,
    PROBE_CIRCLE_GFX = 29,
    PROBE_CHEST_CLOSED_ICON = 144
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
                          unsigned short actionChestThing)
{
    int i;

    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memcpy(champ->name, "TIGGY   ", 8);
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
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] = actionChestThing;
}

static unsigned short next_seeded_thing(const M11_GameViewState* game,
                                        unsigned short thing)
{
    int index;

    if (!game || !game->world.things ||
        thing == THING_NONE || thing == THING_ENDOFLIST ||
        THING_GET_TYPE(thing) != THING_TYPE_JUNK) {
        return THING_ENDOFLIST;
    }
    index = (int)THING_GET_INDEX(thing);
    if (!game->world.things->junks || index < 0 ||
        index >= game->world.things->junkCount) {
        return THING_ENDOFLIST;
    }
    return game->world.things->junks[index].next;
}

static int chain_count(const M11_GameViewState* game,
                       unsigned short first,
                       int maxWalk)
{
    int count = 0;
    unsigned short thing = first;

    while (thing != THING_NONE && thing != THING_ENDOFLIST &&
           count < maxWalk) {
        ++count;
        thing = next_seeded_thing(game, thing);
    }
    return count;
}

static int chain_contains(const M11_GameViewState* game,
                          unsigned short first,
                          unsigned short target,
                          int maxWalk)
{
    int count = 0;
    unsigned short thing = first;

    while (thing != THING_NONE && thing != THING_ENDOFLIST &&
           count < maxWalk) {
        if (thing == target) return 1;
        thing = next_seeded_thing(game, thing);
        ++count;
    }
    return 0;
}

static int seed_records(M11_GameViewState* game,
                        unsigned short actionChestThing,
                        unsigned short weaponThing,
                        const unsigned short actionItems[PROBE_CHAIN_COUNT])
{
    struct DungeonThings_Compat* things;
    int i;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded ||
        !things->containers || things->containerCount < 1 ||
        !things->junks || things->junkCount < PROBE_CHAIN_COUNT ||
        !things->weapons || things->weaponCount < 1) {
        return 0;
    }

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = actionItems[0];
    things->containers[0].type = 0;

    for (i = 0; i < PROBE_CHAIN_COUNT; ++i) {
        memset(&things->junks[i], 0, sizeof(things->junks[i]));
        things->junks[i].type = (unsigned char)((i % 2) + 1);
        things->junks[i].next =
            (i + 1 < PROBE_CHAIN_COUNT) ?
            actionItems[i + 1] : THING_ENDOFLIST;
    }

    memset(&things->weapons[0], 0, sizeof(things->weapons[0]));
    things->weapons[0].next = THING_ENDOFLIST;
    things->weapons[0].type = 4; /* STAFF OF CLAWS named metadata path. */
    things->weapons[0].cursed = 1;
    things->weapons[0].poisoned = 1;
    things->weapons[0].broken = 1;
    things->weapons[0].chargeCount = 7;

    seed_champion(&game->world.party.champions[0], actionChestThing);
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->inventoryPanelActive = 1;
    game->showDebugHUD = 0;
    game->spellPanelOpen = 0;
    game->v1FoodWaterPanelActive = 0;
    return M11_GameView_SetV1LeaderHandObject(game, weaponThing);
}

static int count_asset_matches(const M11_AssetSlot* asset,
                               const unsigned char* fb,
                               int dstX,
                               int dstY,
                               int transparentColor)
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
            if (src != (unsigned char)transparentColor && src == dst) {
                ++matched;
            }
        }
    }
    return matched;
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    const unsigned short actionChestThing = thing_ref(THING_TYPE_CONTAINER, 0);
    const unsigned short weaponThing = thing_ref(THING_TYPE_WEAPON, 0);
    unsigned short actionItems[PROBE_CHAIN_COUNT];
    const M11_AssetSlot* emptyPanel;
    const M11_AssetSlot* chestPanel;
    const M11_AssetSlot* circle;
    int panelX = 0, panelY = 0, panelW = 0, panelH = 0;
    int circleX = 0, circleY = 0, circleW = 0, circleH = 0;
    int emptyMatches;
    int chestMatches;
    int circleMatches;
    int ok = 1;
    int i;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    for (i = 0; i < PROBE_CHAIN_COUNT; ++i) {
        actionItems[i] = thing_ref(THING_TYPE_JUNK, i);
    }

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_true("GRAPHICS.DAT-backed runtime assets available",
                      game.assetsAvailable);
    ok &= expect_true("source-backed records seeded",
                      seed_records(&game, actionChestThing, weaponThing,
                                   actionItems));
    ok &= expect_int("action chest starts with hidden tail",
                     chain_count(&game, game.world.things->containers[0].slot,
                                 PROBE_CHAIN_COUNT + 2),
                     PROBE_CHAIN_COUNT);
    ok &= expect_true("open action-hand chest before eye description",
                      M11_GameView_OpenV1ActionHandChest(&game));
    ok &= expect_int("G0426 initially names action chest",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)actionChestThing);

    ok &= expect_true("eye click routes weapon to object-description panel",
                      M11_GameView_HandlePointer(
                          &game, PROBE_EYE_X, PROBE_EYE_Y, 1) ==
                      M11_GAME_INPUT_REDRAW);
    ok &= expect_int("G0426 closed by object-description route",
                     (int)M11_GameView_GetV1OpenChestThing(&game),
                     (int)THING_NONE);
    ok &= expect_int("object-description panel active after eye click",
                     game.v1ObjectDescriptionPanelActive, 1);
    ok &= expect_int("object-description thing records weapon",
                     (int)game.v1ObjectDescriptionThing, (int)weaponThing);
    ok &= expect_true("weapon description names source object",
                      strcmp(game.v1ObjectDescriptionName,
                             "STAFF OF CLAWS") == 0);
    ok &= expect_true("weapon description records cursed flag",
                      strstr(game.v1ObjectDescriptionBody, "CURSED") != NULL);
    ok &= expect_true("weapon description records poisoned flag",
                      strstr(game.v1ObjectDescriptionBody, "POISONED") != NULL);
    ok &= expect_true("weapon description records broken flag",
                      strstr(game.v1ObjectDescriptionBody, "BROKEN") != NULL);
    ok &= expect_int("action-hand chest icon is closed after object eye",
                     M11_GameView_GetV1InventorySlotIconIndex(
                         &game, CHAMPION_SLOT_ACTION_HAND),
                     PROBE_CHEST_CLOSED_ICON);
    ok &= expect_int("object eye compacted chest to visible slots",
                     chain_count(&game, game.world.things->containers[0].slot,
                                 PROBE_CHAIN_COUNT + 2),
                     PROBE_VISIBLE_CHEST_SLOTS);
    ok &= expect_true("ninth hidden tail dropped by object eye close",
                      !chain_contains(&game, game.world.things->containers[0].slot,
                                      actionItems[PROBE_VISIBLE_CHEST_SLOTS],
                                      PROBE_CHAIN_COUNT + 2));

    game.dialogOverlayActive = 0;
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    emptyPanel = M11_AssetLoader_Load(&game.assetLoader, PROBE_PANEL_EMPTY_GFX);
    chestPanel = M11_AssetLoader_Load(&game.assetLoader, PROBE_PANEL_OPEN_CHEST_GFX);
    circle = M11_AssetLoader_Load(&game.assetLoader, PROBE_CIRCLE_GFX);
    ok &= expect_true("panel and circle assets loaded",
                      emptyPanel && emptyPanel->pixels &&
                      chestPanel && chestPanel->pixels &&
                      circle && circle->pixels);
    ok &= expect_true("object-description panel zones available",
                      M11_GameView_GetV1InventoryPanelZone(
                          &panelX, &panelY, &panelW, &panelH) &&
                      M11_GameView_GetV1ObjectDescriptionCircleZone(
                          &circleX, &circleY, &circleW, &circleH));
    (void)panelW;
    (void)panelH;
    (void)circleW;
    (void)circleH;
    emptyMatches = count_asset_matches(
        emptyPanel, fb, PROBE_VIEWPORT_X + panelX,
        PROBE_VIEWPORT_Y + panelY, 8);
    chestMatches = count_asset_matches(
        chestPanel, fb, PROBE_VIEWPORT_X + panelX,
        PROBE_VIEWPORT_Y + panelY, 8);
    circleMatches = count_asset_matches(
        circle, fb, PROBE_VIEWPORT_X + circleX,
        PROBE_VIEWPORT_Y + circleY, 12);
    printf("panelMatches empty=%d chest=%d circle=%d\n",
           emptyMatches, chestMatches, circleMatches);
    ok &= expect_true("C020 empty panel dominates stale C025 panel",
                      emptyMatches > 3000 && emptyMatches > chestMatches + 500);
    ok &= expect_true("C029 object-description circle rendered",
                      circleMatches >= 60);

    printf("sourceEvidence=PANEL.C:F0352:2123-2159;PANEL.C:F0342:1126-1145;CHEST.C:F0334:112-132\n");
    printf("%s dm1 v1 chest eye object-description runtime probe\n",
           ok ? "PASS" : "FAIL");
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
