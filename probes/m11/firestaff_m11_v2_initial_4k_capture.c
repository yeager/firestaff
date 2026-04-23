#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

#define FB_W 320
#define FB_H 200

typedef struct ProbeAllocations {
    struct DungeonDatState_Compat* dungeon;
    struct DungeonThings_Compat* things;
} ProbeAllocations;

static const unsigned char kPalette[16][3] = {
    {0,0,0},{0,0,170},{0,170,0},{0,170,170},{170,0,0},{170,85,0},{170,170,170},{85,85,85},
    {255,85,85},{85,85,255},{85,255,85},{85,255,255},{255,85,255},{255,255,85},{210,210,210},{255,255,255}
};

static void probe_set_next(unsigned char* raw, unsigned short nextThing) {
    if (!raw) return;
    raw[0] = (unsigned char)(nextThing & 0xFFu);
    raw[1] = (unsigned char)((nextThing >> 8) & 0xFFu);
}

static void probe_set_square(struct DungeonDatState_Compat* dungeon,
                             int mapX,
                             int mapY,
                             unsigned char square) {
    int height;
    if (!dungeon || !dungeon->tiles || !dungeon->tiles[0].squareData) return;
    height = dungeon->maps[0].height;
    dungeon->tiles[0].squareData[mapX * height + mapY] = square;
}

static int init_synthetic_view(M11_GameViewState* state, ProbeAllocations* allocs, const char* dataDir) {
    struct DungeonDatState_Compat* dungeon;
    struct DungeonThings_Compat* things;
    int i;
    const int squareCount = 25;
    char graphicsDatPath[1024];

    if (!state || !allocs) return 0;
    memset(allocs, 0, sizeof(*allocs));
    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;
    snprintf(state->title, sizeof(state->title), "V2 4K CAPTURE");
    snprintf(state->sourceId, sizeof(state->sourceId), "v2-initial-4k");
    snprintf(state->lastAction, sizeof(state->lastAction), "CAPTURE");
    snprintf(state->lastOutcome, sizeof(state->lastOutcome), "SYNTHETIC");

    if (dataDir && dataDir[0]) {
        snprintf(graphicsDatPath, sizeof(graphicsDatPath), "%s/GRAPHICS.DAT", dataDir);
        if (M11_AssetLoader_Init(&state->assetLoader, graphicsDatPath)) {
            state->assetsAvailable = 1;
        }
    }

    dungeon = (struct DungeonDatState_Compat*)calloc(1, sizeof(*dungeon));
    things = (struct DungeonThings_Compat*)calloc(1, sizeof(*things));
    if (!dungeon || !things) return 0;

    dungeon->header.mapCount = 1;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(struct DungeonMapDesc_Compat));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(struct DungeonMapTiles_Compat));
    things->squareFirstThings = (unsigned short*)calloc((size_t)squareCount, sizeof(unsigned short));
    things->groups = (struct DungeonGroup_Compat*)calloc(1, sizeof(struct DungeonGroup_Compat));
    things->rawThingData[THING_TYPE_GROUP] = (unsigned char*)calloc(16, sizeof(unsigned char));
    if (!dungeon->maps || !dungeon->tiles || !things->squareFirstThings || !things->groups || !things->rawThingData[THING_TYPE_GROUP]) {
        return 0;
    }

    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    dungeon->maps[0].width = 5;
    dungeon->maps[0].height = 5;
    dungeon->tiles[0].squareCount = squareCount;
    dungeon->tiles[0].squareData = (unsigned char*)calloc((size_t)squareCount, sizeof(unsigned char));
    if (!dungeon->tiles[0].squareData) return 0;

    for (i = 0; i < squareCount; ++i) {
        things->squareFirstThings[i] = THING_ENDOFLIST;
        dungeon->tiles[0].squareData[i] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    }

    things->loaded = 1;
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    probe_set_next(things->rawThingData[THING_TYPE_GROUP], THING_ENDOFLIST);

    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 2;
    state->world.party.mapY = 3;
    state->world.party.direction = DIR_NORTH;
    state->world.party.championCount = 4;
    state->world.party.activeChampionIndex = 0;
    for (i = 0; i < 4; ++i) {
        struct ChampionState_Compat* c = &state->world.party.champions[i];
        memset(c, 0, sizeof(*c));
        c->present = 1;
        c->hp.current = 42 + i * 9;
        c->hp.maximum = 60 + i * 10;
        c->stamina.current = 30 + i * 7;
        c->stamina.maximum = 50 + i * 8;
        c->mana.current = 18 + i * 4;
        c->mana.maximum = 28 + i * 5;
        c->food = 140 - i * 10;
        c->water = 120 - i * 8;
        c->portraitIndex = i;
    }
    memcpy(state->world.party.champions[0].name, "TIGGY", 5);
    memcpy(state->world.party.champions[1].name, "HALK", 4);
    memcpy(state->world.party.champions[2].name, "DARO", 4);
    memcpy(state->world.party.champions[3].name, "WUUF", 4);

    probe_set_square(dungeon, 2, 3, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5));
    probe_set_square(dungeon, 1, 3, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5));
    probe_set_square(dungeon, 3, 3, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5));
    probe_set_square(dungeon, 1, 2, (unsigned char)(DUNGEON_ELEMENT_WALL << 5));
    probe_set_square(dungeon, 2, 2, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5));
    probe_set_square(dungeon, 3, 2, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5));
    probe_set_square(dungeon, 2, 1, (unsigned char)(DUNGEON_ELEMENT_WALL << 5));
    probe_set_square(dungeon, 3, 1, (unsigned char)(DUNGEON_ELEMENT_PIT << 5));
    probe_set_square(dungeon, 1, 1, (unsigned char)(DUNGEON_ELEMENT_WALL << 5));
    state->world.magic.magicalLightAmount = 192;

    allocs->dungeon = dungeon;
    allocs->things = things;
    return 1;
}

static void free_synthetic_view(M11_GameViewState* state, ProbeAllocations* allocs) {
    (void)state;
    if (allocs && allocs->dungeon) {
        free(allocs->dungeon->tiles ? allocs->dungeon->tiles[0].squareData : NULL);
        free(allocs->dungeon->maps);
        free(allocs->dungeon->tiles);
        free(allocs->dungeon);
    }
    if (allocs && allocs->things) {
        free(allocs->things->squareFirstThings);
        free(allocs->things->groups);
        free(allocs->things->rawThingData[THING_TYPE_GROUP]);
        free(allocs->things);
    }
}

static int write_ppm(const unsigned char* fb, const char* path) {
    FILE* f = fopen(path, "wb");
    int x, y;
    if (!f) return 0;
    fprintf(f, "P6\n%d %d\n255\n", FB_W, FB_H);
    for (y = 0; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            const unsigned char* rgb = kPalette[fb[y * FB_W + x] & 0x0F];
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    return 1;
}

int main(int argc, char** argv) {
    const char* outDir;
    const char* dataDir;
    M11_GameViewState state;
    ProbeAllocations allocs;
    unsigned char baseFb[FB_W * FB_H];
    unsigned char creatureFb[FB_W * FB_H];
    char path[1024];
    FILE* meta;

    if (argc < 3) {
        fprintf(stderr, "usage: %s OUT_DIR DATA_DIR\n", argv[0]);
        return 2;
    }
    outDir = argv[1];
    dataDir = argv[2];

    memset(&state, 0, sizeof(state));
    if (!init_synthetic_view(&state, &allocs, dataDir)) {
        fprintf(stderr, "failed to init synthetic view\n");
        return 1;
    }

    memset(baseFb, 0, sizeof(baseFb));
    M11_GameView_Draw(&state, baseFb, FB_W, FB_H);
    snprintf(path, sizeof(path), "%s/base_scene.ppm", outDir);
    if (!write_ppm(baseFb, path)) return 1;

    state.world.things->squareFirstThings[2 * state.world.dungeon->maps[0].height + 2] = (unsigned short)((THING_TYPE_GROUP << 10) | 0);
    state.world.things->groups[0].creatureType = 12; /* Skeleton */
    state.world.things->groups[0].count = 0; /* one creature */
    state.world.things->groups[0].health[0] = 64;
    state.world.things->groups[0].direction = 2; /* facing party */

    memset(creatureFb, 0, sizeof(creatureFb));
    M11_GameView_Draw(&state, creatureFb, FB_W, FB_H);
    snprintf(path, sizeof(path), "%s/creature_scene.ppm", outDir);
    if (!write_ppm(creatureFb, path)) return 1;

    snprintf(path, sizeof(path), "%s/scene_meta.txt", outDir);
    meta = fopen(path, "w");
    if (!meta) return 1;
    fprintf(meta,
            "viewport_rect=12,24,196,112\n"
            "screen_scale=10\n"
            "viewport_frame_origin=0,16\n"
            "action_area_origin=224,45\n"
            "spell_area_origin=224,90\n"
            "party_hud_origin=12,160\n"
            "creature_family=skeleton\n"
            "creature_type=12\n"
            "assets_available=%d\n",
            state.assetsAvailable);
    fclose(meta);

    free_synthetic_view(&state, &allocs);
    printf("wrote %s/base_scene.ppm and creature_scene.ppm\n", outDir);
    return 0;
}
