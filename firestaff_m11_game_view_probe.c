#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    PROBE_COLOR_BLACK = 0,
    PROBE_COLOR_BROWN = 6,
    PROBE_COLOR_DARK_GRAY = 8,
    PROBE_COLOR_LIGHT_GREEN = 10,
    PROBE_COLOR_LIGHT_CYAN = 11,
    PROBE_COLOR_LIGHT_RED = 12,
    PROBE_COLOR_MAGENTA = 13,
    PROBE_COLOR_YELLOW = 14,
    PROBE_COLOR_LIGHT_BLUE = 9,
    PROBE_COLOR_WHITE = 15
};

enum {
    PROBE_VIEWPORT_X = 12,
    PROBE_VIEWPORT_Y = 24,
    PROBE_VIEWPORT_W = 196,
    PROBE_VIEWPORT_H = 118,
    PROBE_LANE_STRIP_X = 38,
    PROBE_LANE_STRIP_Y = 30,
    PROBE_LANE_STRIP_W = 148,
    PROBE_LANE_STRIP_H = 12,
    PROBE_SIDEBAR_X = 214,
    PROBE_SIDEBAR_Y = 24,
    PROBE_SIDEBAR_W = 94,
    PROBE_SIDEBAR_H = 120,
    PROBE_MAP_BOX_X = 218,
    PROBE_MAP_BOX_Y = 74,
    PROBE_MAP_BOX_W = 86,
    PROBE_MAP_BOX_H = 68,
    PROBE_BOTTOM_PANEL_X = 12,
    PROBE_BOTTOM_PANEL_Y = 146,
    PROBE_BOTTOM_PANEL_W = 296,
    PROBE_BOTTOM_PANEL_H = 46,
    PROBE_PARTY_PANEL_Y = 160,
    PROBE_PARTY_PANEL_H = 28,
    PROBE_PROMPT_STRIP_X = 14,
    PROBE_PROMPT_STRIP_Y = 165,
    PROBE_PROMPT_STRIP_W = 292,
    PROBE_PROMPT_STRIP_H = 14,
    PROBE_FEEDBACK_STRIP_X = 24,
    PROBE_FEEDBACK_STRIP_Y = 130,
    PROBE_FEEDBACK_STRIP_W = 172,
    PROBE_FEEDBACK_STRIP_H = 10,
    PROBE_DEPTH_STRIP_X = 44,
    PROBE_DEPTH_STRIP_Y = 112,
    PROBE_DEPTH_STRIP_W = 144,
    PROBE_DEPTH_STRIP_H = 18
};

typedef struct {
    int total;
    int passed;
} ProbeTally;

static void probe_record(ProbeTally* tally,
                         const char* id,
                         int ok,
                         const char* message) {
    tally->total += 1;
    if (ok) {
        tally->passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

static size_t probe_count_non_zero(const unsigned char* framebuffer,
                                   int width,
                                   int x,
                                   int y,
                                   int w,
                                   int h) {
    size_t count = 0;
    int xx;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            if (framebuffer[(y + yy) * width + (x + xx)] != 0U) {
                ++count;
            }
        }
    }
    return count;
}

static size_t probe_count_color(const unsigned char* framebuffer,
                                int width,
                                int x,
                                int y,
                                int w,
                                int h,
                                unsigned char color) {
    size_t count = 0;
    int xx;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            if (framebuffer[(y + yy) * width + (x + xx)] == color) {
                ++count;
            }
        }
    }
    return count;
}

static void probe_set_square(struct DungeonDatState_Compat* dungeon,
                             int mapX,
                             int mapY,
                             unsigned char square) {
    int height;
    if (!dungeon || !dungeon->tiles || !dungeon->tiles[0].squareData) {
        return;
    }
    height = dungeon->maps[0].height;
    dungeon->tiles[0].squareData[mapX * height + mapY] = square;
}

static void probe_set_next(unsigned char* raw,
                           unsigned short nextThing) {
    if (!raw) {
        return;
    }
    raw[0] = (unsigned char)(nextThing & 0xFFu);
    raw[1] = (unsigned char)((nextThing >> 8) & 0xFFu);
}

static int probe_init_synthetic_view(M11_GameViewState* state) {
    struct DungeonDatState_Compat* dungeon;
    struct DungeonThings_Compat* things;
    int i;
    int squareCount = 25;

    if (!state) {
        return 0;
    }

    M11_GameView_Init(state);
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;
    snprintf(state->title, sizeof(state->title), "SYNTHETIC");
    snprintf(state->sourceId, sizeof(state->sourceId), "probe");
    snprintf(state->lastAction, sizeof(state->lastAction), "PROBE");
    snprintf(state->lastOutcome, sizeof(state->lastOutcome), "SYNTHETIC VIEW");

    dungeon = (struct DungeonDatState_Compat*)calloc(1, sizeof(*dungeon));
    things = (struct DungeonThings_Compat*)calloc(1, sizeof(*things));
    if (!dungeon || !things) {
        free(dungeon);
        free(things);
        return 0;
    }

    dungeon->header.mapCount = 1;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(struct DungeonMapDesc_Compat));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(struct DungeonMapTiles_Compat));
    if (!dungeon->maps || !dungeon->tiles) {
        free(dungeon->maps);
        free(dungeon->tiles);
        free(dungeon);
        free(things);
        return 0;
    }

    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    dungeon->maps[0].width = 5;
    dungeon->maps[0].height = 5;
    dungeon->tiles[0].squareCount = squareCount;
    dungeon->tiles[0].squareData = (unsigned char*)calloc((size_t)squareCount, sizeof(unsigned char));
    things->squareFirstThings = (unsigned short*)calloc((size_t)squareCount, sizeof(unsigned short));
    things->doors = (struct DungeonDoor_Compat*)calloc(1, sizeof(struct DungeonDoor_Compat));
    things->rawThingData[THING_TYPE_DOOR] = (unsigned char*)calloc(4, sizeof(unsigned char));
    things->groups = (struct DungeonGroup_Compat*)calloc(1, sizeof(struct DungeonGroup_Compat));
    things->weapons = (struct DungeonWeapon_Compat*)calloc(1, sizeof(struct DungeonWeapon_Compat));
    things->projectiles = (struct DungeonProjectile_Compat*)calloc(1, sizeof(struct DungeonProjectile_Compat));
    things->rawThingData[THING_TYPE_GROUP] = (unsigned char*)calloc(16, sizeof(unsigned char));
    things->rawThingData[THING_TYPE_WEAPON] = (unsigned char*)calloc(4, sizeof(unsigned char));
    things->rawThingData[THING_TYPE_PROJECTILE] = (unsigned char*)calloc(8, sizeof(unsigned char));
    if (!dungeon->tiles[0].squareData || !things->squareFirstThings ||
        !things->doors || !things->rawThingData[THING_TYPE_DOOR] ||
        !things->groups || !things->weapons || !things->projectiles ||
        !things->rawThingData[THING_TYPE_GROUP] ||
        !things->rawThingData[THING_TYPE_WEAPON] ||
        !things->rawThingData[THING_TYPE_PROJECTILE]) {
        free(dungeon->tiles[0].squareData);
        free(dungeon->maps);
        free(dungeon->tiles);
        free(dungeon);
        free(things->squareFirstThings);
        free(things->doors);
        free(things->rawThingData[THING_TYPE_DOOR]);
        free(things->groups);
        free(things->weapons);
        free(things->projectiles);
        free(things->rawThingData[THING_TYPE_GROUP]);
        free(things->rawThingData[THING_TYPE_WEAPON]);
        free(things->rawThingData[THING_TYPE_PROJECTILE]);
        free(things);
        return 0;
    }

    for (i = 0; i < squareCount; ++i) {
        things->squareFirstThings[i] = THING_ENDOFLIST;
        dungeon->tiles[0].squareData[i] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    }

    things->squareFirstThingCount = squareCount;
    things->doorCount = 1;
    things->groupCount = 1;
    things->weaponCount = 1;
    things->projectileCount = 1;
    things->thingCounts[THING_TYPE_DOOR] = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->thingCounts[THING_TYPE_WEAPON] = 1;
    things->thingCounts[THING_TYPE_PROJECTILE] = 1;
    probe_set_next(things->rawThingData[THING_TYPE_DOOR], THING_ENDOFLIST);
    probe_set_next(things->rawThingData[THING_TYPE_GROUP], (unsigned short)((THING_TYPE_WEAPON << 10) | 0));
    probe_set_next(things->rawThingData[THING_TYPE_WEAPON], (unsigned short)((THING_TYPE_PROJECTILE << 10) | 0));
    probe_set_next(things->rawThingData[THING_TYPE_PROJECTILE], THING_ENDOFLIST);
    things->doors[0].next = THING_ENDOFLIST;
    things->doors[0].vertical = 1;
    things->loaded = 1;

    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 2;
    state->world.party.mapY = 3;
    state->world.party.direction = DIR_NORTH;
    state->world.party.championCount = 2;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    memcpy(state->world.party.champions[0].name, "TIGGY", 5);
    state->world.party.champions[0].hp.current = 72;
    state->world.party.champions[0].hp.maximum = 100;
    state->world.party.champions[0].stamina.current = 48;
    state->world.party.champions[0].stamina.maximum = 80;
    state->world.party.champions[0].food = 180;
    state->world.party.champions[0].water = 140;
    state->world.party.champions[1].present = 1;
    memcpy(state->world.party.champions[1].name, "HALK", 4);
    state->world.party.champions[1].hp.current = 91;
    state->world.party.champions[1].hp.maximum = 110;
    state->world.party.champions[1].stamina.current = 60;
    state->world.party.champions[1].stamina.maximum = 90;
    state->world.party.champions[1].food = 200;
    state->world.party.champions[1].water = 155;

    probe_set_square(dungeon, 2, 3, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5));
    probe_set_square(dungeon, 1, 2, (unsigned char)(DUNGEON_ELEMENT_WALL << 5));
    probe_set_square(dungeon, 2, 2, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5));
    probe_set_square(dungeon, 3, 2, (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | 0x0B));
    probe_set_square(dungeon, 2, 1, (unsigned char)(DUNGEON_ELEMENT_STAIRS << 5));
    probe_set_square(dungeon, 3, 1, (unsigned char)(DUNGEON_ELEMENT_PIT << 5));
    probe_set_square(dungeon, 1, 0, (unsigned char)(DUNGEON_ELEMENT_TELEPORTER << 5));
    probe_set_square(dungeon, 2, 0, (unsigned char)(DUNGEON_ELEMENT_FAKEWALL << 5));
    probe_set_square(dungeon, 3, 0, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5));
    things->squareFirstThings[2 * dungeon->maps[0].height + 2] = (unsigned short)((THING_TYPE_GROUP << 10) | 0);
    things->squareFirstThings[3 * dungeon->maps[0].height + 2] = 0;

    return 1;
}

static void probe_free_synthetic_view(M11_GameViewState* state) {
    if (!state) {
        return;
    }
    if (state->world.dungeon) {
        free(state->world.dungeon->tiles ? state->world.dungeon->tiles[0].squareData : NULL);
        free(state->world.dungeon->maps);
        free(state->world.dungeon->tiles);
        free(state->world.dungeon);
    }
    if (state->world.things) {
        free(state->world.things->squareFirstThings);
        free(state->world.things->doors);
        free(state->world.things->groups);
        free(state->world.things->weapons);
        free(state->world.things->projectiles);
        free(state->world.things->rawThingData[THING_TYPE_DOOR]);
        free(state->world.things->rawThingData[THING_TYPE_GROUP]);
        free(state->world.things->rawThingData[THING_TYPE_WEAPON]);
        free(state->world.things->rawThingData[THING_TYPE_PROJECTILE]);
        free(state->world.things);
    }
    memset(state, 0, sizeof(*state));
}

int main(int argc, char** argv) {
    const char* dataDir = NULL;
    M12_StartupMenuState menuState;
    M11_GameViewState gameView;
    M11_GameViewState syntheticView;
    ProbeTally tally = {0, 0};
    unsigned char framebuffer[320 * 200];
    unsigned char turnedFramebuffer[320 * 200];
    unsigned char movedFramebuffer[320 * 200];
    unsigned char syntheticFramebuffer[320 * 200];
    uint32_t initialHash = 0;
    int initialDirection = 0;
    uint32_t initialTick = 0;
    int moved = 0;
    size_t litPixels = 0;
    size_t i;

    if (argc > 1) {
        dataDir = argv[1];
    } else {
        dataDir = getenv("FIRESTAFF_DATA");
    }
    if (!dataDir || dataDir[0] == '\0') {
        static char fallback[1024];
        const char* home = getenv("HOME");
        if (!home || home[0] == '\0') {
            fprintf(stderr, "No HOME or FIRESTAFF_DATA available\n");
            return 2;
        }
        snprintf(fallback, sizeof(fallback), "%s/.firestaff/data", home);
        dataDir = fallback;
    }

    memset(&syntheticView, 0, sizeof(syntheticView));

    M12_StartupMenu_InitWithDataDir(&menuState, dataDir);
    probe_record(&tally,
                 "INV_GV_01",
                 menuState.selectedIndex == 0 &&
                     menuState.entries[0].available == 1 &&
                     menuState.entries[0].gameId &&
                     strcmp(menuState.entries[0].gameId, "dm1") == 0 &&
                     menuState.entries[0].sourceKind == M12_MENU_SOURCE_BUILTIN_CATALOG,
                 "launcher exposes DM1 as the default builtin launch source with verified assets");

    M11_GameView_Init(&gameView);
    probe_record(&tally,
                 "INV_GV_02",
                 M11_GameView_OpenSelectedMenuEntry(&gameView, &menuState) == 1 &&
                     gameView.active == 1 &&
                     strcmp(gameView.title, "DUNGEON MASTER") == 0 &&
                     strcmp(gameView.sourceId, "dm1") == 0,
                 "launcher selection transitions into a real game view through the source hook");

    initialHash = gameView.lastWorldHash;
    initialDirection = gameView.world.party.direction;
    initialTick = gameView.world.gameTick;

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&gameView, framebuffer, 320, 200);
    for (i = 0; i < sizeof(framebuffer); ++i) {
        if (framebuffer[i] != 0U) {
            ++litPixels;
        }
    }
    probe_record(&tally,
                 "INV_GV_03",
                 litPixels > 4000U,
                 "game view renders a non-empty dungeon-backed frame");

    probe_record(&tally,
                 "INV_GV_04",
                 M11_GameView_HandleInput(&gameView, M12_MENU_INPUT_RIGHT) == M11_GAME_INPUT_REDRAW &&
                     gameView.world.party.direction != initialDirection &&
                     gameView.world.gameTick == initialTick + 1 &&
                     gameView.lastWorldHash != initialHash,
                 "game view input turns the party through the real tick orchestrator");

    memset(turnedFramebuffer, 0, sizeof(turnedFramebuffer));
    M11_GameView_Draw(&gameView, turnedFramebuffer, 320, 200);
    probe_record(&tally,
                 "INV_GV_05",
                 memcmp(framebuffer, turnedFramebuffer, sizeof(framebuffer)) != 0,
                 "turning changes the rendered pseudo-viewport frame, not just the inspector text");

    memset(movedFramebuffer, 0, sizeof(movedFramebuffer));
    if (M11_GameView_HandleInput(&gameView, M12_MENU_INPUT_UP) == M11_GAME_INPUT_REDRAW) {
        M11_GameView_Draw(&gameView, movedFramebuffer, 320, 200);
        moved = memcmp(turnedFramebuffer, movedFramebuffer, sizeof(movedFramebuffer)) != 0;
    }
    probe_record(&tally,
                 "INV_GV_06",
                 moved,
                 "movement changes the rendered pseudo-viewport with real world movement");

    probe_record(&tally,
                 "INV_GV_08",
                 probe_count_non_zero(framebuffer,
                                      320,
                                      PROBE_MAP_BOX_X,
                                      PROBE_MAP_BOX_Y,
                                      PROBE_MAP_BOX_W,
                                      PROBE_MAP_BOX_H) > 250U,
                 "small minimap inset still renders in the corner");

    probe_record(&tally,
                 "INV_GV_09",
                 probe_init_synthetic_view(&syntheticView),
                 "synthetic viewport harness initialises a focused 3x3 sample state");

    probe_record(&tally,
                 "INV_GV_07",
                 M11_GameView_HandleInput(&syntheticView, M12_MENU_INPUT_ACCEPT) == M11_GAME_INPUT_REDRAW &&
                     syntheticView.world.gameTick == 0 &&
                     strcmp(syntheticView.lastAction, "INSPECT") == 0 &&
                     strcmp(syntheticView.lastOutcome, "ENEMY SPOTTED") == 0,
                 "enter now inspects the front-cell target without spending a real tick");

    probe_record(&tally,
                 "INV_GV_07B",
                 M11_GameView_HandleInput(&syntheticView, M12_MENU_INPUT_CYCLE_CHAMPION) == M11_GAME_INPUT_REDRAW &&
                     syntheticView.world.party.activeChampionIndex == 1 &&
                     strcmp(syntheticView.lastAction, "CHAMP") == 0 &&
                     strstr(syntheticView.inspectTitle, "HALK READY") != NULL,
                 "tab cycles the active front champion and updates the in-view readout");

    initialTick = syntheticView.world.gameTick;
    probe_record(&tally,
                 "INV_GV_07C",
                 M11_GameView_HandleInput(&syntheticView, M12_MENU_INPUT_ACTION) == M11_GAME_INPUT_REDRAW &&
                     syntheticView.world.gameTick == initialTick + 1 &&
                     strcmp(syntheticView.lastAction, "ATTACK") == 0 &&
                     strstr(syntheticView.inspectTitle, "ATTACKS") != NULL &&
                     strstr(syntheticView.inspectTitle, "HALK") != NULL,
                 "space turns front-cell creature contact into a real strike tick for the selected champion");

    memset(syntheticFramebuffer, 0, sizeof(syntheticFramebuffer));
    M11_GameView_Draw(&syntheticView, syntheticFramebuffer, 320, 200);

    probe_record(&tally,
                 "INV_GV_10",
                 probe_count_color(syntheticFramebuffer,
                                   320,
                                   PROBE_VIEWPORT_X,
                                   PROBE_VIEWPORT_Y,
                                   PROBE_VIEWPORT_W,
                                   PROBE_VIEWPORT_H,
                                   PROBE_COLOR_LIGHT_RED) > 1U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_VIEWPORT_X,
                                       PROBE_VIEWPORT_Y,
                                       PROBE_VIEWPORT_W,
                                       PROBE_VIEWPORT_H,
                                       PROBE_COLOR_YELLOW) > 20U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_VIEWPORT_X,
                                       PROBE_VIEWPORT_Y,
                                       PROBE_VIEWPORT_W,
                                       PROBE_VIEWPORT_H,
                                       PROBE_COLOR_LIGHT_GREEN) > 0U,
                 "synthetic feature cells add door, stair, and occupancy cues inside the viewport");

    probe_record(&tally,
                 "INV_GV_11",
                 probe_count_non_zero(syntheticFramebuffer, 320, 74, 66, 72, 34) > 300U &&
                     probe_count_color(syntheticFramebuffer, 320, 150, 48, 40, 48, PROBE_COLOR_LIGHT_RED) > 3U,
                 "a side door accent stays visible without collapsing the forward corridor window");

    probe_record(&tally,
                 "INV_GV_12",
                 probe_count_non_zero(syntheticFramebuffer, 320,
                                      PROBE_VIEWPORT_X, PROBE_VIEWPORT_Y,
                                      PROBE_VIEWPORT_W, PROBE_VIEWPORT_H) > 4000U &&
                     probe_count_non_zero(syntheticFramebuffer, 320,
                                         PROBE_MAP_BOX_X, PROBE_MAP_BOX_Y,
                                         PROBE_MAP_BOX_W, PROBE_MAP_BOX_H) > 250U,
                 "viewport slice and minimap inset coexist in the same frame");

    probe_record(&tally,
                 "INV_GV_12B",
                 probe_count_color(syntheticFramebuffer,
                                   320,
                                   PROBE_VIEWPORT_X,
                                   PROBE_VIEWPORT_Y,
                                   PROBE_VIEWPORT_W,
                                   PROBE_VIEWPORT_H,
                                   PROBE_COLOR_WHITE) > 6U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_VIEWPORT_X,
                                       PROBE_VIEWPORT_Y,
                                       PROBE_VIEWPORT_W,
                                       PROBE_VIEWPORT_H,
                                       PROBE_COLOR_LIGHT_CYAN) > 10U,
                 "viewport item and effect cues appear when real thing chains include loot and projectiles");

    probe_record(&tally,
                 "INV_GV_13",
                 M11_GameView_HandleInput(&gameView, M12_MENU_INPUT_BACK) == M11_GAME_INPUT_RETURN_TO_MENU,
                 "escape from the game view returns control to the launcher");

    probe_record(&tally,
                 "INV_GV_14",
                 probe_count_non_zero(framebuffer,
                                      320,
                                      PROBE_SIDEBAR_X,
                                      PROBE_SIDEBAR_Y,
                                      PROBE_SIDEBAR_W,
                                      PROBE_SIDEBAR_H) > 1200U &&
                     probe_count_color(framebuffer,
                                       320,
                                       PROBE_SIDEBAR_X,
                                       PROBE_SIDEBAR_Y,
                                       PROBE_SIDEBAR_W,
                                       PROBE_SIDEBAR_H,
                                       PROBE_COLOR_LIGHT_CYAN) > 40U,
                 "sidebar HUD renders separate status and map framing beside the viewport");

    probe_record(&tally,
                 "INV_GV_15",
                 probe_count_non_zero(framebuffer,
                                      320,
                                      PROBE_BOTTOM_PANEL_X,
                                      PROBE_BOTTOM_PANEL_Y,
                                      PROBE_BOTTOM_PANEL_W,
                                      PROBE_BOTTOM_PANEL_H) > 1800U &&
                     probe_count_color(framebuffer,
                                       320,
                                       PROBE_BOTTOM_PANEL_X,
                                       PROBE_PARTY_PANEL_Y,
                                       PROBE_BOTTOM_PANEL_W,
                                       PROBE_PARTY_PANEL_H,
                                       PROBE_COLOR_DARK_GRAY) > 120U,
                 "bottom HUD renders a dedicated party/status strip instead of a single inspector blob");

    probe_record(&tally,
                 "INV_GV_15B",
                 probe_count_color(syntheticFramebuffer,
                                   320,
                                   PROBE_BOTTOM_PANEL_X,
                                   PROBE_PARTY_PANEL_Y,
                                   PROBE_BOTTOM_PANEL_W,
                                   PROBE_PARTY_PANEL_H,
                                   PROBE_COLOR_LIGHT_RED) > 30U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_BOTTOM_PANEL_X,
                                       PROBE_PARTY_PANEL_Y,
                                       PROBE_BOTTOM_PANEL_W,
                                       PROBE_PARTY_PANEL_H,
                                       PROBE_COLOR_LIGHT_GREEN) > 30U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_BOTTOM_PANEL_X,
                                       PROBE_PARTY_PANEL_Y,
                                       PROBE_BOTTOM_PANEL_W,
                                       PROBE_PARTY_PANEL_H,
                                       PROBE_COLOR_YELLOW) > 20U,
                 "party strip reflects real champion bars and active-slot framing when champion data exists");

    probe_record(&tally,
                 "INV_GV_16",
                 probe_count_color(syntheticFramebuffer,
                                   320,
                                   PROBE_VIEWPORT_X,
                                   PROBE_VIEWPORT_Y,
                                   PROBE_VIEWPORT_W,
                                   PROBE_VIEWPORT_H,
                                   PROBE_COLOR_YELLOW) > 20U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_VIEWPORT_X,
                                       PROBE_VIEWPORT_Y,
                                       PROBE_VIEWPORT_W,
                                       PROBE_VIEWPORT_H,
                                       PROBE_COLOR_LIGHT_CYAN) > 20U,
                 "viewport framing uses layered face bands and bright dungeon edges");

    probe_record(&tally,
                 "INV_GV_17",
                 probe_count_color(syntheticFramebuffer,
                                   320,
                                   PROBE_VIEWPORT_X,
                                   PROBE_VIEWPORT_Y,
                                   PROBE_VIEWPORT_W,
                                   PROBE_VIEWPORT_H,
                                   PROBE_COLOR_LIGHT_RED) > 8U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_PROMPT_STRIP_X,
                                       PROBE_PROMPT_STRIP_Y,
                                       PROBE_PROMPT_STRIP_W,
                                       PROBE_PROMPT_STRIP_H,
                                       PROBE_COLOR_LIGHT_RED) > 10U &&
                     probe_count_non_zero(syntheticFramebuffer,
                                          320,
                                          PROBE_SIDEBAR_X,
                                          112,
                                          PROBE_SIDEBAR_W,
                                          24) > 60U &&
                     probe_count_non_zero(syntheticFramebuffer,
                                          320,
                                          PROBE_PROMPT_STRIP_X,
                                          PROBE_PROMPT_STRIP_Y,
                                          PROBE_PROMPT_STRIP_W,
                                          PROBE_PROMPT_STRIP_H) > 200U,
                 "front-cell focus adds a threat-colored viewport reticle plus contextual inspect readout");

    probe_record(&tally,
                 "INV_GV_18",
                 probe_count_non_zero(syntheticFramebuffer,
                                      320,
                                      PROBE_LANE_STRIP_X,
                                      PROBE_LANE_STRIP_Y,
                                      PROBE_LANE_STRIP_W,
                                      PROBE_LANE_STRIP_H) > 250U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_LANE_STRIP_X,
                                       PROBE_LANE_STRIP_Y,
                                       PROBE_LANE_STRIP_W,
                                       PROBE_LANE_STRIP_H,
                                       PROBE_COLOR_LIGHT_RED) > 6U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_LANE_STRIP_X,
                                       PROBE_LANE_STRIP_Y,
                                       PROBE_LANE_STRIP_W,
                                       PROBE_LANE_STRIP_H,
                                       PROBE_COLOR_YELLOW) > 6U,
                 "near-lane scanner chips surface left, front, and right contact state inside the viewport");

    probe_record(&tally,
                 "INV_GV_19",
                 probe_count_non_zero(syntheticFramebuffer,
                                      320,
                                      PROBE_FEEDBACK_STRIP_X,
                                      PROBE_FEEDBACK_STRIP_Y,
                                      PROBE_FEEDBACK_STRIP_W,
                                      PROBE_FEEDBACK_STRIP_H) > 180U &&
                     probe_count_color(syntheticFramebuffer,
                                       320,
                                       PROBE_FEEDBACK_STRIP_X,
                                       PROBE_FEEDBACK_STRIP_Y,
                                       PROBE_FEEDBACK_STRIP_W,
                                       PROBE_FEEDBACK_STRIP_H,
                                       PROBE_COLOR_LIGHT_RED) > 8U,
                 "post-tick feedback strip renders attack-colored event telemetry inside the HUD");

    probe_record(&tally,
                 "INV_GV_20",
                 probe_count_non_zero(syntheticFramebuffer,
                                      320,
                                      PROBE_DEPTH_STRIP_X,
                                      PROBE_DEPTH_STRIP_Y,
                                      PROBE_DEPTH_STRIP_W,
                                      PROBE_DEPTH_STRIP_H) > 180U,
                 "forward depth chips summarize the next three center-lane cells with live threat and traversal cues");

    probe_free_synthetic_view(&syntheticView);
    M11_GameView_Shutdown(&gameView);
    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
