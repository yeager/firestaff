/*
 * DM1 V1 champion-panel status-hand owner gate runtime probe.
 *
 * Firestaff runtime evidence: opens the hash-verified DM1 V1 M11 path,
 * seeds a four-champion party and a transient leader-hand object, then
 * clicks status-row hand slots through M11_GameView_HandlePointer().
 *
 * Source evidence:
 *   ReDMCSB CLIKCHAM.C F0367 line 32 dispatches C020..C027 status
 *   hand slot-box commands to CHAMPION.C F0302.
 *   ReDMCSB CHAMPION.C F0302:662-710 maps slotBoxIndex >> 1 to the
 *   owning champion, rejects the currently open inventory champion at
 *   lines 679-680, rejects dead champions at line 681, and uses
 *   M070_HAND_SLOT_INDEX(slotBoxIndex) before the F0302:695-708 swap.
 *   ReDMCSB CHAMDRAW.C F0292:543-549 binds the same status-row ready
 *   and action hand boxes to C08_SLOT_BOX_INVENTORY_FIRST_SLOT -
 *   (championIndex << 1) - slotIndex.
 *   ReDMCSB DEFS.H:1878 defines M070_HAND_SLOT_INDEX(slotBoxIndex) as
 *   slotBoxIndex & 1.
 *
 * Scope: live M11 pointer route + framebuffer smoke for the owner gate.
 * It does not claim original DOS pixel parity.
 */

#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_PARTY_COUNT = 4
};

typedef struct ProbeRectStats {
    int nonBlack;
    uint32_t hash;
} ProbeRectStats;

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

static unsigned char px_index(const unsigned char* fb, int width, int x, int y)
{
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static ProbeRectStats rect_stats(const unsigned char* fb,
                                 int width,
                                 int x,
                                 int y,
                                 int w,
                                 int h)
{
    ProbeRectStats stats;
    int yy;

    stats.nonBlack = 0;
    stats.hash = UINT32_C(2166136261);
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            uint32_t px = (uint32_t)px_index(fb, width, x + xx, y + yy);
            if (px != 0U) {
                ++stats.nonBlack;
            }
            stats.hash ^= px + (uint32_t)(xx * 17) + (uint32_t)(yy * 31);
            stats.hash *= UINT32_C(16777619);
        }
    }
    return stats;
}

static int status_hand_center(int championIndex, int handIndex, int* outX, int* outY)
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    if (!M11_GameView_GetV1StatusHandSlotBoxZone(
            championIndex, handIndex, &x, &y, &w, &h)) {
        return 0;
    }
    if (outX) *outX = x + w / 2;
    if (outY) *outY = y + h / 2;
    return 1;
}

static void seed_champion(struct ChampionState_Compat* champ,
                          const char* name,
                          int portraitIndex,
                          unsigned short readyThing,
                          unsigned short actionThing)
{
    int i;

    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = (unsigned char)name[i];
    }
    champ->portraitIndex = portraitIndex;
    champ->direction = portraitIndex & 3;
    champ->hp.current = 100;
    champ->hp.maximum = 100;
    champ->stamina.current = 80;
    champ->stamina.maximum = 80;
    champ->mana.current = 60;
    champ->mana.maximum = 60;
    champ->food = 1200;
    champ->water = 900;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
    champ->inventory[CHAMPION_SLOT_HAND_LEFT] = readyThing;
    champ->inventory[CHAMPION_SLOT_ACTION_HAND] = actionThing;
}

static int seed_runtime(M11_GameViewState* game,
                        unsigned short leaderThing,
                        unsigned short ownerActionThing,
                        unsigned short otherActionThing)
{
    struct DungeonThings_Compat* things;
    int i;

    if (!game || !game->world.things) return 0;
    things = game->world.things;
    if (!things->loaded || !things->junks || things->junkCount < 3) {
        printf("SKIP loaded DM1 runtime has fewer than 3 junk records\n");
        return 0;
    }

    for (i = 0; i < 3; ++i) {
        memset(&things->junks[i], 0, sizeof(things->junks[i]));
        things->junks[i].type = (unsigned char)i;
        things->junks[i].next = THING_ENDOFLIST;
    }

    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->world.party.championCount = PROBE_PARTY_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 1;
    game->inventorySelectedSlot = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->spellPanelOpen = 0;
    game->actingChampionOrdinal = 0;

    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, THING_NONE, ownerActionThing);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, THING_NONE, otherActionThing);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, THING_NONE, THING_NONE);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, THING_NONE, THING_NONE);

    M11_GameView_ClearV1LeaderHandObject(game);
    return M11_GameView_SetV1LeaderHandObject(game, leaderThing);
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fbBefore[PROBE_FB_W * PROBE_FB_H];
    unsigned char fbAfterReject[PROBE_FB_W * PROBE_FB_H];
    unsigned char fbAfterSwap[PROBE_FB_W * PROBE_FB_H];
    const unsigned short leaderThing = thing_ref(THING_TYPE_JUNK, 0);
    const unsigned short ownerActionThing = thing_ref(THING_TYPE_JUNK, 1);
    const unsigned short otherActionThing = thing_ref(THING_TYPE_JUNK, 2);
    int ownerX = 0;
    int ownerY = 0;
    int otherX = 0;
    int otherY = 0;
    int zoneX = 0;
    int zoneY = 0;
    int zoneW = 0;
    int zoneH = 0;
    ProbeRectStats otherBefore;
    ProbeRectStats otherAfterReject;
    ProbeRectStats otherAfterSwap;
    int opened;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    opened = M11_GameView_OpenSelectedMenuEntry(&game, &menu);
    if (!opened) {
        printf("SKIP could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    if (!seed_runtime(&game, leaderThing, ownerActionThing, otherActionThing)) {
        M11_GameView_Shutdown(&game);
        return 0;
    }

    ok &= expect_true("owner action-hand click center available",
                      status_hand_center(0, 1, &ownerX, &ownerY));
    ok &= expect_true("other action-hand click center available",
                      status_hand_center(1, 1, &otherX, &otherY));
    ok &= expect_true("other action-hand icon zone available",
                      M11_GameView_GetV1StatusHandIconZone(1, 1,
                                                           &zoneX, &zoneY,
                                                           &zoneW, &zoneH) &&
                      zoneW == 16 && zoneH == 16);

    M11_GameView_Draw(&game, fbBefore, PROBE_FB_W, PROBE_FB_H);
    otherBefore = rect_stats(fbBefore, PROBE_FB_W, zoneX, zoneY, zoneW, zoneH);
    ok &= expect_true("seeded other-owner icon zone is visible",
                      otherBefore.nonBlack > 0);
    ok &= expect_int("owner starts as active inventory champion",
                     game.world.party.activeChampionIndex, 0);
    ok &= expect_int("leader hand starts with transient thing",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)leaderThing);
    ok &= expect_int("owner action hand starts unchanged",
                     (int)game.world.party.champions[0].
                         inventory[CHAMPION_SLOT_ACTION_HAND],
                     (int)ownerActionThing);
    ok &= expect_int("other action hand starts with swappable thing",
                     (int)game.world.party.champions[1].
                         inventory[CHAMPION_SLOT_ACTION_HAND],
                     (int)otherActionThing);

    ok &= expect_int("owner status-hand click is ignored",
                     (int)M11_GameView_HandlePointer(&game, ownerX, ownerY, 1),
                     (int)M11_GAME_INPUT_IGNORED);
    ok &= expect_int("owner reject keeps leader hand",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)leaderThing);
    ok &= expect_int("owner reject keeps owner action hand",
                     (int)game.world.party.champions[0].
                         inventory[CHAMPION_SLOT_ACTION_HAND],
                     (int)ownerActionThing);
    ok &= expect_int("owner reject keeps other action hand",
                     (int)game.world.party.champions[1].
                         inventory[CHAMPION_SLOT_ACTION_HAND],
                     (int)otherActionThing);

    M11_GameView_Draw(&game, fbAfterReject, PROBE_FB_W, PROBE_FB_H);
    otherAfterReject = rect_stats(fbAfterReject, PROBE_FB_W,
                                  zoneX, zoneY, zoneW, zoneH);
    ok &= expect_int("owner reject leaves other icon hash stable",
                     (int)otherAfterReject.hash, (int)otherBefore.hash);

    ok &= expect_int("non-owner status-hand click redraws",
                     (int)M11_GameView_HandlePointer(&game, otherX, otherY, 1),
                     (int)M11_GAME_INPUT_REDRAW);
    ok &= expect_int("non-owner action hand receives leader thing",
                     (int)game.world.party.champions[1].
                         inventory[CHAMPION_SLOT_ACTION_HAND],
                     (int)leaderThing);
    ok &= expect_int("leader hand receives non-owner previous thing",
                     (int)M11_GameView_GetV1LeaderHandThing(&game),
                     (int)otherActionThing);
    ok &= expect_int("inventory owner action hand still unchanged after non-owner swap",
                     (int)game.world.party.champions[0].
                         inventory[CHAMPION_SLOT_ACTION_HAND],
                     (int)ownerActionThing);

    M11_GameView_Draw(&game, fbAfterSwap, PROBE_FB_W, PROBE_FB_H);
    otherAfterSwap = rect_stats(fbAfterSwap, PROBE_FB_W,
                                zoneX, zoneY, zoneW, zoneH);
    ok &= expect_true("non-owner swap changes other icon pixels",
                      otherAfterSwap.hash != otherBefore.hash);
    ok &= expect_true("non-owner icon remains visible after swap",
                      otherAfterSwap.nonBlack > 0);

    printf("sourceEvidence=CLIKCHAM.C:F0367:line32;CHAMPION.C:F0302:662-710;CHAMPION.C:F0302:679-680;CHAMDRAW.C:F0292:543-549;DEFS.H:M070_HAND_SLOT_INDEX:1878\n");
    printf("scope=live M11 pointer route and framebuffer owner-gate smoke; no original DOS pixel parity claim\n");
    printf("%s dm1 v1 champion-panel status-hand owner gate runtime probe\n",
           ok ? "PASS" : "FAIL");

    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
