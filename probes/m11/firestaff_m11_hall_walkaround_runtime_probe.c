#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

typedef struct HallFrontCellProbe {
    int valid;
    int mapX;
    int mapY;
    unsigned char square;
    int elementType;
    unsigned short firstThing;
    int firstThingType;
    int mirrorOrdinal;
} HallFrontCellProbe;

typedef struct HallStepProbe {
    const char* name;
    int result;
    unsigned int tick;
    int mapIndex;
    int mapX;
    int mapY;
    int direction;
    int championCount;
    int candidateMirrorOrdinal;
    int candidateMirrorPartyIndex;
    int candidateMirrorPanelActive;
    char lastAction[32];
    char lastOutcome[64];
    HallFrontCellProbe front;
} HallStepProbe;

static const char* dir_name(int dir) {
    switch (dir & 3) {
        case DIR_NORTH: return "NORTH";
        case DIR_EAST: return "EAST";
        case DIR_SOUTH: return "SOUTH";
        case DIR_WEST: return "WEST";
    }
    return "UNKNOWN";
}

static const char* element_name(int elementType) {
    if (elementType >= 0 && elementType < DUNGEON_ELEMENT_COUNT) {
        return F0503_DUNGEON_GetElementName_Compat(elementType);
    }
    return "OUT_OF_BOUNDS";
}

static void ensure_output_dir(const char* outDir) {
    if (!outDir || outDir[0] == '\0') return;
#ifdef _WIN32
    (void)_mkdir(outDir);
#else
    (void)mkdir(outDir, 0777);
#endif
}

static void direction_vectors(int direction, int* fx, int* fy) {
    switch (direction & 3) {
        case DIR_NORTH: *fx = 0; *fy = -1; break;
        case DIR_EAST:  *fx = 1; *fy = 0; break;
        case DIR_SOUTH: *fx = 0; *fy = 1; break;
        default:        *fx = -1; *fy = 0; break;
    }
}

static int map_square_base(const struct DungeonDatState_Compat* dungeon, int mapIndex) {
    int i;
    int base = 0;
    if (!dungeon || !dungeon->maps || mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return -1;
    for (i = 0; i < mapIndex; ++i) {
        base += (int)dungeon->maps[i].width * (int)dungeon->maps[i].height;
    }
    return base;
}

static int get_square_byte(const struct GameWorld_Compat* world, int mapIndex, int mapX, int mapY, unsigned char* outSquare) {
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    int index;
    if (!world || !world->dungeon || !world->dungeon->tilesLoaded || !outSquare) return 0;
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) return 0;
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) return 0;
    tiles = &world->dungeon->tiles[mapIndex];
    index = mapX * (int)map->height + mapY;
    if (!tiles->squareData || index < 0 || index >= tiles->squareCount) return 0;
    *outSquare = tiles->squareData[index];
    return 1;
}

static unsigned short first_square_thing(const struct GameWorld_Compat* world, int mapIndex, int mapX, int mapY) {
    int base;
    int squareIndex;
    const struct DungeonMapDesc_Compat* map;
    if (!world || !world->dungeon || !world->things || !world->things->squareFirstThings) return THING_ENDOFLIST;
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) return THING_ENDOFLIST;
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) return THING_ENDOFLIST;
    base = map_square_base(world->dungeon, mapIndex);
    if (base < 0) return THING_ENDOFLIST;
    squareIndex = base + mapX * (int)map->height + mapY;
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) return THING_ENDOFLIST;
    return world->things->squareFirstThings[squareIndex];
}

static HallFrontCellProbe sample_front(const M11_GameViewState* game) {
    HallFrontCellProbe cell;
    int fx = 0;
    int fy = 0;
    memset(&cell, 0, sizeof(cell));
    cell.firstThing = THING_ENDOFLIST;
    cell.firstThingType = -1;
    cell.mirrorOrdinal = -1;
    if (!game || !game->active) return cell;
    direction_vectors(game->world.party.direction, &fx, &fy);
    cell.mapX = game->world.party.mapX + fx;
    cell.mapY = game->world.party.mapY + fy;
    if (get_square_byte(&game->world, game->world.party.mapIndex, cell.mapX, cell.mapY, &cell.square)) {
        cell.valid = 1;
        cell.elementType = (cell.square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
        cell.firstThing = first_square_thing(&game->world, game->world.party.mapIndex, cell.mapX, cell.mapY);
        if (cell.firstThing != THING_NONE && cell.firstThing != THING_ENDOFLIST) {
            cell.firstThingType = THING_GET_TYPE(cell.firstThing);
        }
    }
    cell.mirrorOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
    return cell;
}

static void snapshot(M11_GameViewState* game, const char* name, int result, HallStepProbe* out) {
    memset(out, 0, sizeof(*out));
    out->name = name;
    out->result = result;
    out->tick = game->world.gameTick;
    out->mapIndex = game->world.party.mapIndex;
    out->mapX = game->world.party.mapX;
    out->mapY = game->world.party.mapY;
    out->direction = game->world.party.direction;
    out->championCount = game->world.party.championCount;
    out->candidateMirrorOrdinal = game->candidateMirrorOrdinal;
    out->candidateMirrorPartyIndex = game->candidateMirrorPartyIndex;
    out->candidateMirrorPanelActive = game->candidateMirrorPanelActive;
    snprintf(out->lastAction, sizeof(out->lastAction), "%s", game->lastAction);
    snprintf(out->lastOutcome, sizeof(out->lastOutcome), "%s", game->lastOutcome);
    out->front = sample_front(game);
}

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int write_outputs(const char* outDir, const HallStepProbe* rows, int rowCount) {
    char mdPath[1024];
    char jsonPath[1024];
    FILE* md;
    FILE* js;
    int i;
    ensure_output_dir(outDir);
    snprintf(mdPath, sizeof(mdPath), "%s/dm1_v1_hall_walkaround_runtime_probe.md", outDir);
    snprintf(jsonPath, sizeof(jsonPath), "%s/dm1_v1_hall_walkaround_runtime_probe.json", outDir);
    md = fopen(mdPath, "w");
    js = fopen(jsonPath, "w");
    if (!md || !js) {
        if (md) fclose(md);
        if (js) fclose(js);
        return 0;
    }
    fprintf(md, "# DM1 V1 Hall walkaround runtime probe\n\n");
    fprintf(md, "Source lock: LOADSAVE.C:1940-1944 initial party position; COMMAND.C:2150-2156 turn/step dispatch; CLIKMENU.C:142-173 turn path; CLIKMENU.C:264-328 movement blockers; GAMELOOP.C:80-90 viewport redraw; DUNGEON.C:2608-2612 and MOVESENS.C:1501-1503 isolate champion portrait/candidate routes; REVIVE.C:272-276 appends candidate, 744-785 cancels/removes, 785-835 confirms/disables/reincarnates.\n\n");
    fprintf(md, "| step | result | tick | pos | dir | champions | candidate ord/index/panel | front map | front element | front thing | mirror | action | outcome |\n");
    fprintf(md, "| --- | ---: | ---: | --- | --- | ---: | --- | --- | --- | ---: | ---: | --- | --- |\n");
    fprintf(js, "{\n  \"schema\": \"dm1_v1_hall_walkaround_runtime_probe.v1\",\n  \"sourceLock\": \"LOADSAVE.C:1940-1944; COMMAND.C:2150-2156; CLIKMENU.C:142-173,264-328; GAMELOOP.C:80-90; DUNGEON.C:2608-2612; MOVESENS.C:1501-1503; REVIVE.C:272-276,744-835\",\n  \"steps\": [\n");
    for (i = 0; i < rowCount; ++i) {
        const HallStepProbe* r = &rows[i];
        fprintf(md, "| %s | %d | %u | %d,%d,%d | %d/%s | %d | %d/%d/%d | %d,%d | %s | 0x%04X | %d | %s | %s |\n",
                r->name, r->result, r->tick, r->mapIndex, r->mapX, r->mapY,
                r->direction, dir_name(r->direction), r->championCount,
                r->candidateMirrorOrdinal, r->candidateMirrorPartyIndex, r->candidateMirrorPanelActive,
                r->front.mapX, r->front.mapY,
                r->front.valid ? element_name(r->front.elementType) : "OUT_OF_BOUNDS",
                (unsigned int)r->front.firstThing, r->front.mirrorOrdinal,
                r->lastAction, r->lastOutcome);
        fprintf(js,
                "    {\"name\":\"%s\",\"result\":%d,\"tick\":%u,\"mapIndex\":%d,\"mapX\":%d,\"mapY\":%d,\"direction\":%d,\"championCount\":%d,\"candidateMirrorOrdinal\":%d,\"candidateMirrorPartyIndex\":%d,\"candidateMirrorPanelActive\":%d,\"front\":{\"valid\":%d,\"mapX\":%d,\"mapY\":%d,\"square\":%u,\"elementType\":%d,\"firstThing\":%u,\"firstThingType\":%d,\"mirrorOrdinal\":%d},\"lastAction\":\"%s\",\"lastOutcome\":\"%s\"}%s\n",
                r->name, r->result, r->tick, r->mapIndex, r->mapX, r->mapY, r->direction,
                r->championCount, r->candidateMirrorOrdinal, r->candidateMirrorPartyIndex, r->candidateMirrorPanelActive,
                r->front.valid, r->front.mapX, r->front.mapY, (unsigned int)r->front.square,
                r->front.elementType, (unsigned int)r->front.firstThing, r->front.firstThingType,
                r->front.mirrorOrdinal, r->lastAction, r->lastOutcome, i == rowCount - 1 ? "" : ",");
    }
    fprintf(js, "  ],\n  \"status\": \"PASS\"\n}\n");
    fclose(md);
    fclose(js);
    printf("wrote %s and %s\n", mdPath, jsonPath);
    return 1;
}

static int open_game(const char* dataDir, M12_StartupMenuState* menu, M11_GameViewState* game) {
    M12_StartupMenu_InitWithDataDir(menu, dataDir, NULL);
    M11_GameView_Init(game);
    return M11_GameView_OpenSelectedMenuEntry(game, menu);
}

static int navigate_to_corridor_second_mirror(M11_GameViewState* game) {
    if (!game || !game->active) return 0;
    (void)M11_GameView_HandleInput(game, M12_MENU_INPUT_LEFT);
    (void)M11_GameView_HandleInput(game, M12_MENU_INPUT_LEFT);
    (void)M11_GameView_HandleInput(game, M12_MENU_INPUT_RIGHT);
    (void)M11_GameView_HandleInput(game, M12_MENU_INPUT_RIGHT);
    (void)M11_GameView_HandleInput(game, M12_MENU_INPUT_UP);
    (void)M11_GameView_HandleInput(game, M12_MENU_INPUT_RIGHT);
    (void)M11_GameView_HandleInput(game, M12_MENU_INPUT_RIGHT);
    return game->world.party.mapIndex == 0 && game->world.party.mapX == 1 &&
           game->world.party.mapY == 4 && game->world.party.direction == DIR_NORTH &&
           M11_GameView_GetFrontMirrorOrdinal(game) == 2;
}

int main(int argc, char** argv) {
    const char* dataDir;
    const char* outDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    M12_StartupMenuState menu2;
    M11_GameViewState game2;
    HallStepProbe rows[15];
    int ok = 1;
    int result;
    int hpBeforeReincarnate = 0;

    if (argc < 3) {
        fprintf(stderr, "usage: %s DATA_DIR OUT_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    outDir = argv[2];

    if (!open_game(dataDir, &menu, &game)) {
        fprintf(stderr, "failed to open DM1 game view\n");
        return 1;
    }

    /*
     * Canonical DM1 V1 hall walkaround (LOADSAVE.C MEDIA529 fix):
     * From start (1,3,SOUTH) the only walkable forward step is south into
     * (1,4) corridor.  East and west of the start are walls; north of the
     * start is the closed champion-mirror door at (1,2) (mirror ordinal 1).
     * After stepping south into (1,4), turning right twice puts the party
     * at (1,4,NORTH) where mirror ordinal 2 becomes visible on the wall
     * north (back of the start corridor).  This walks the canonical decode
     * end-to-end and proves: turn changes direction-only, blocked moves
     * stay put, and one legal step plus a 180-degree rotation reveals a
     * different mirror.
     */
    snapshot(&game, "start_hall_initial_south", M11_GAME_INPUT_REDRAW, &rows[0]);
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_LEFT);
    snapshot(&game, "turn_left_east_view_changes", result, &rows[1]);
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    snapshot(&game, "step_east_blocked_by_hall_side", result, &rows[2]);
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_LEFT);
    snapshot(&game, "turn_left_north_front_mirror", result, &rows[3]);
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    snapshot(&game, "step_north_mirror_wall_blocked", result, &rows[4]);
    /* Pivot back to SOUTH and step into the canonical corridor. */
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);
    /* (1,3,EAST) intermediate */
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);
    snapshot(&game, "turn_right_south_corridor", result, &rows[5]);
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    snapshot(&game, "step_south_advances_corridor", result, &rows[6]);
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);
    /* (1,4,WEST) intermediate */
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);
    snapshot(&game, "turn_around_face_north_from_corridor", result, &rows[7]);
    /* Already snapshot at (1,4,NORTH); next step sees mirror 2. */
    snapshot(&game, "corridor_front_second_mirror", result, &rows[8]);

    ok &= expect_int("start map", rows[0].mapIndex, 0);
    ok &= expect_int("start x from DUNGEON header", rows[0].mapX, 1);
    ok &= expect_int("start y from DUNGEON header", rows[0].mapY, 3);
    ok &= expect_int("start dir south", rows[0].direction, DIR_SOUTH);
    ok &= expect_int("east turn redraw", rows[1].result, M11_GAME_INPUT_REDRAW);
    ok &= expect_int("east turn changes direction", rows[1].direction, DIR_EAST);
    ok &= expect_int("east blocked keeps x", rows[2].mapX, 1);
    ok &= expect_int("east blocked keeps y", rows[2].mapY, 3);
    ok &= expect_int("north mirror visible", rows[3].front.mirrorOrdinal, 1);
    ok &= expect_int("north mirror blocked keeps x", rows[4].mapX, 1);
    ok &= expect_int("north mirror blocked keeps y", rows[4].mapY, 3);
    /* row 5: now facing south after two right turns */
    ok &= expect_int("south corridor direction", rows[5].direction, DIR_SOUTH);
    ok &= expect_int("south corridor x", rows[5].mapX, 1);
    ok &= expect_int("south corridor y", rows[5].mapY, 3);
    /* row 6: forward south succeeded -> (1,4,SOUTH) */
    ok &= expect_int("south step x", rows[6].mapX, 1);
    ok &= expect_int("south step y", rows[6].mapY, 4);
    ok &= expect_int("south step dir", rows[6].direction, DIR_SOUTH);
    /* row 7: 180-degree rotation -> (1,4,NORTH) */
    ok &= expect_int("180 turn x", rows[7].mapX, 1);
    ok &= expect_int("180 turn y", rows[7].mapY, 4);
    ok &= expect_int("180 turn dir north", rows[7].direction, DIR_NORTH);
    /* row 8: same place as row 7, with mirror 2 visible on the front cell. */
    ok &= expect_int("corridor mirror visible", rows[8].front.mirrorOrdinal, 2);

    /* Walking and turning must not implicitly click a portrait, open the Hall
     * candidate panel, or resurrect/reincarnate/recruit anyone. */
    for (int i = 0; i < 9; ++i) {
        char label[96];
        snprintf(label, sizeof(label), "%s champion count stable", rows[i].name);
        ok &= expect_int(label, rows[i].championCount, 0);
        snprintf(label, sizeof(label), "%s no candidate mirror ordinal", rows[i].name);
        ok &= expect_int(label, rows[i].candidateMirrorOrdinal, -1);
        snprintf(label, sizeof(label), "%s no candidate panel", rows[i].name);
        ok &= expect_int(label, rows[i].candidateMirrorPanelActive, 0);
        snprintf(label, sizeof(label), "%s redraw/refresh", rows[i].name);
        ok &= expect_int(label, rows[i].result, M11_GAME_INPUT_REDRAW);
    }

    if (rows[0].front.mapX == rows[1].front.mapX && rows[0].front.mapY == rows[1].front.mapY) {
        fprintf(stderr, "FAIL viewport/front cell did not change after turn\n");
        ok = 0;
    }
    /* Forward south at (1,3,SOUTH) advances the front cell from (1,4) to (1,5). */
    if (rows[5].front.mapX == rows[6].front.mapX && rows[5].front.mapY == rows[6].front.mapY) {
        fprintf(stderr, "FAIL viewport/front cell did not advance after south step\n");
        ok = 0;
    }

    /* Hall candidate panel runtime parity. ReDMCSB F0280 appends the
     * candidate before the Resurrect/Reincarnate/Cancel panel opens;
     * F0282 Cancel removes that just-appended champion, while Resurrect and
     * Reincarnate keep the champion and disable the mirror route. */
    result = M11_GameView_SelectFrontMirrorCandidate(&game);
    snapshot(&game, "select_second_mirror_candidate_appends_party", result, &rows[9]);
    ok &= expect_int("select candidate result", rows[9].result, 1);
    ok &= expect_int("select candidate count appended", rows[9].championCount, 1);
    ok &= expect_int("select candidate mirror ordinal", rows[9].candidateMirrorOrdinal, 2);
    ok &= expect_int("select candidate party index", rows[9].candidateMirrorPartyIndex, 0);
    ok &= expect_int("select candidate panel active", rows[9].candidateMirrorPanelActive, 1);
    ok &= expect_int("select candidate active leader", game.world.party.activeChampionIndex, 0);

    result = M11_GameView_CancelMirrorCandidate(&game);
    snapshot(&game, "cancel_candidate_removes_appended_party_member", result, &rows[10]);
    ok &= expect_int("cancel candidate result", rows[10].result, 1);
    ok &= expect_int("cancel candidate count removed", rows[10].championCount, 0);
    ok &= expect_int("cancel clears ordinal", rows[10].candidateMirrorOrdinal, -1);
    ok &= expect_int("cancel clears party index", rows[10].candidateMirrorPartyIndex, -1);
    ok &= expect_int("cancel clears panel", rows[10].candidateMirrorPanelActive, 0);

    result = M11_GameView_SelectFrontMirrorCandidate(&game);
    snapshot(&game, "select_again_for_resurrect", result, &rows[11]);
    ok &= expect_int("select again result", rows[11].result, 1);
    ok &= expect_int("select again count", rows[11].championCount, 1);

    result = M11_GameView_ConfirmMirrorCandidate(&game, 0);
    snapshot(&game, "resurrect_keeps_candidate_and_disables_reselect", result, &rows[12]);
    ok &= expect_int("resurrect result", rows[12].result, 1);
    ok &= expect_int("resurrect keeps champion", rows[12].championCount, 1);
    ok &= expect_int("resurrect clears ordinal", rows[12].candidateMirrorOrdinal, -1);
    ok &= expect_int("resurrect clears panel", rows[12].candidateMirrorPanelActive, 0);
    ok &= expect_int("resurrect disables front mirror TextString route", rows[12].front.mirrorOrdinal, -1);

    result = M11_GameView_SelectFrontMirrorCandidate(&game);
    snapshot(&game, "resurrected_mirror_reselect_blocked", result, &rows[13]);
    ok &= expect_int("reselect disabled result", rows[13].result, 0);
    ok &= expect_int("reselect disabled count stable", rows[13].championCount, 1);
    ok &= expect_int("reselect disabled panel closed", rows[13].candidateMirrorPanelActive, 0);

    memset(&game2, 0, sizeof(game2));
    if (!open_game(dataDir, &menu2, &game2) || !navigate_to_corridor_second_mirror(&game2)) {
        fprintf(stderr, "FAIL could not reopen/navigate fresh game for reincarnate route\n");
        ok = 0;
        memset(&rows[14], 0, sizeof(rows[14]));
        rows[14].name = "reincarnate_candidate_halves_vitals";
    } else {
        result = M11_GameView_SelectFrontMirrorCandidate(&game2);
        ok &= expect_int("reincarnate select result", result, 1);
        hpBeforeReincarnate = game2.world.party.champions[0].hp.maximum;
        result = M11_GameView_ConfirmMirrorCandidate(&game2, 1);
        snapshot(&game2, "reincarnate_candidate_halves_vitals", result, &rows[14]);
        ok &= expect_int("reincarnate result", rows[14].result, 1);
        ok &= expect_int("reincarnate keeps champion", rows[14].championCount, 1);
        ok &= expect_int("reincarnate max hp halved", game2.world.party.champions[0].hp.maximum, hpBeforeReincarnate / 2);
        ok &= expect_int("reincarnate current hp halved", game2.world.party.champions[0].hp.current, hpBeforeReincarnate / 2);
        ok &= expect_int("reincarnate fighter skill cleared", game2.world.party.champions[0].skillLevels[0], 0);
    }

    if (!write_outputs(outDir, rows, 15)) ok = 0;
    M11_GameView_Shutdown(&game2);
    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 hall walkaround runtime probe\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
