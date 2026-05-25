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

typedef struct TurnViewportCellProbe {
    int relSide;
    int mapX;
    int mapY;
    int valid;
    unsigned char square;
    int elementType;
    unsigned short firstThing;
    int firstThingType;
    int doorObserved;
    int pitObserved;
    int teleporterObserved;
} TurnViewportCellProbe;

typedef struct TurnViewportSnapshotProbe {
    const char* name;
    int result;
    unsigned int gameTick;
    int mapIndex;
    int mapX;
    int mapY;
    int direction;
    TurnViewportCellProbe left;
    TurnViewportCellProbe center;
    TurnViewportCellProbe right;
    TurnViewportCellProbe viewportRows[19];
    int viewportRowCount;
} TurnViewportSnapshotProbe;

static void ensure_output_dir(const char* outDir) {
    if (!outDir || outDir[0] == '\0') return;
#ifdef _WIN32
    (void)_mkdir(outDir);
#else
    (void)mkdir(outDir, 0777);
#endif
}

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
    return "Unknown";
}

static void direction_vectors(int direction, int* fx, int* fy, int* rx, int* ry) {
    switch (direction & 3) {
        case DIR_NORTH: *fx = 0; *fy = -1; *rx = 1; *ry = 0; break;
        case DIR_EAST:  *fx = 1; *fy = 0;  *rx = 0; *ry = 1; break;
        case DIR_SOUTH: *fx = 0; *fy = 1;  *rx = -1; *ry = 0; break;
        default:        *fx = -1; *fy = 0; *rx = 0; *ry = -1; break;
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

static TurnViewportCellProbe sample_relative_cell(const struct GameWorld_Compat* world, int relForward, int relSide) {
    TurnViewportCellProbe cell;
    int fx = 0, fy = 0, rx = 0, ry = 0;
    memset(&cell, 0, sizeof(cell));
    cell.relSide = relSide;
    cell.firstThing = THING_ENDOFLIST;
    cell.firstThingType = -1;
    if (!world || !world->dungeon) return cell;
    direction_vectors(world->party.direction, &fx, &fy, &rx, &ry);
    cell.mapX = world->party.mapX + relForward * fx + relSide * rx;
    cell.mapY = world->party.mapY + relForward * fy + relSide * ry;
    if (!get_square_byte(world, world->party.mapIndex, cell.mapX, cell.mapY, &cell.square)) return cell;
    cell.valid = 1;
    cell.elementType = (cell.square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    cell.firstThing = first_square_thing(world, world->party.mapIndex, cell.mapX, cell.mapY);
    if (cell.firstThing != THING_NONE && cell.firstThing != THING_ENDOFLIST) {
        cell.firstThingType = THING_GET_TYPE(cell.firstThing);
    }
    cell.doorObserved = (cell.elementType == DUNGEON_ELEMENT_DOOR || cell.firstThingType == THING_TYPE_DOOR) ? 1 : 0;
    cell.pitObserved = (cell.elementType == DUNGEON_ELEMENT_PIT) ? 1 : 0;
    cell.teleporterObserved = (cell.elementType == DUNGEON_ELEMENT_TELEPORTER || cell.firstThingType == THING_TYPE_TELEPORTER) ? 1 : 0;
    return cell;
}

static TurnViewportCellProbe sample_cell(const struct GameWorld_Compat* world, int relSide) {
    return sample_relative_cell(world, 1, relSide);
}

static int viewport_relative_rows(TurnViewportCellProbe* outRows, int maxRows, const struct GameWorld_Compat* world) {
    static const int coords[][2] = {
        {4, -1}, {4, 1}, {4, 0},
        {3, -2}, {3, 2}, {3, -1}, {3, 1}, {3, 0},
        {2, -2}, {2, 2}, {2, -1}, {2, 1}, {2, 0},
        {1, -1}, {1, 1}, {1, 0},
        {0, -1}, {0, 1}, {0, 0}
    };
    int i;
    int count = (int)(sizeof(coords) / sizeof(coords[0]));
    if (!outRows || maxRows < count) return 0;
    for (i = 0; i < count; ++i) {
        outRows[i] = sample_relative_cell(world, coords[i][0], coords[i][1]);
    }
    return count;
}

static void snapshot(M11_GameViewState* game, const char* name, int result, TurnViewportSnapshotProbe* out) {
    memset(out, 0, sizeof(*out));
    out->name = name;
    out->result = result;
    out->gameTick = game->world.gameTick;
    out->mapIndex = game->world.party.mapIndex;
    out->mapX = game->world.party.mapX;
    out->mapY = game->world.party.mapY;
    out->direction = game->world.party.direction;
    out->left = sample_cell(&game->world, -1);
    out->center = sample_cell(&game->world, 0);
    out->right = sample_cell(&game->world, 1);
    out->viewportRowCount = viewport_relative_rows(out->viewportRows, 19, &game->world);
}

static int write_outputs(const char* outDir, const TurnViewportSnapshotProbe* rows, int rowCount) {
    char mdPath[1024];
    char jsonPath[1024];
    FILE* md;
    FILE* js;
    int i;
    ensure_output_dir(outDir);
    snprintf(mdPath, sizeof(mdPath), "%s/pass127_turn_viewport_orientation_probe.md", outDir);
    snprintf(jsonPath, sizeof(jsonPath), "%s/pass127_turn_viewport_orientation_probe.json", outDir);
    md = fopen(mdPath, "w");
    js = fopen(jsonPath, "w");
    if (!md || !js) {
        if (md) fclose(md);
        if (js) fclose(js);
        return 0;
    }
    fprintf(md, "# Pass 127 turn viewport orientation probe\n\n");
    fprintf(md, "Source lock: ReDMCSB COMMAND.C:2150-2156 dispatches turn/step commands; CLIKMENU.C:156-173/237-347 sets StopWaitingForPlayerInput after successful turn/step; GAMELOOP.C:90 and DUNVIEW.C:8318-8616 redraw/present from current party direction/map coordinates.\n\n");
    fprintf(md, "| snapshot | tick | pos | dir | lane | map | square | element | firstThing | door | pit | teleporter |\n");
    fprintf(md, "| --- | ---: | --- | --- | ---: | --- | --- | --- | --- | ---: | ---: | ---: |\n");
    fprintf(js, "{\n  \"schema\": \"pass127_turn_viewport_orientation_probe.v4\",\n  \"sourceLock\": \"COMMAND.C:2150-2156 dispatches turn/step commands; CLIKMENU.C:156-173/237-347 sets StopWaitingForPlayerInput after successful turn/step; GAMELOOP.C:90 and DUNVIEW.C:8318-8616 redraw/present from current party direction/map coordinates; DUNVIEW.C:8318-8618 traverses viewport rows D4/D3/D2/D1/D0 far-to-near.\",\n  \"viewportRowOrder\": \"D4L,D4R,D4C,D3L2,D3R2,D3L,D3R,D3C,D2L2,D2R2,D2L,D2R,D2C,D1L,D1R,D1C,D0L,D0R,D0C\",\n  \"snapshots\": [\n");
    for (i = 0; i < rowCount; ++i) {
        const TurnViewportSnapshotProbe* r = &rows[i];
        const TurnViewportCellProbe* cells[3] = { &r->left, &r->center, &r->right };
        int c;
        for (c = 0; c < 3; ++c) {
            const TurnViewportCellProbe* cell = cells[c];
            fprintf(md, "| %s | %u | %d,%d,%d | %d/%s | %+d | %d,%d | 0x%02X | %s | 0x%04X | %d | %d | %d |\n",
                    r->name, r->gameTick, r->mapIndex, r->mapX, r->mapY,
                    r->direction, dir_name(r->direction), cell->relSide, cell->mapX, cell->mapY,
                    (unsigned int)cell->square, cell->valid ? element_name(cell->elementType) : "OUT_OF_BOUNDS",
                    (unsigned int)cell->firstThing, cell->doorObserved, cell->pitObserved, cell->teleporterObserved);
        }
        fprintf(js,
                "    {\"name\":\"%s\",\"tick\":%u,\"mapIndex\":%d,\"mapX\":%d,\"mapY\":%d,\"direction\":%d,\"front\":[{\"lane\":-1,\"mapX\":%d,\"mapY\":%d,\"square\":%u,\"elementType\":%d,\"firstThing\":%u,\"door\":%d,\"pit\":%d,\"teleporter\":%d},{\"lane\":0,\"mapX\":%d,\"mapY\":%d,\"square\":%u,\"elementType\":%d,\"firstThing\":%u,\"door\":%d,\"pit\":%d,\"teleporter\":%d},{\"lane\":1,\"mapX\":%d,\"mapY\":%d,\"square\":%u,\"elementType\":%d,\"firstThing\":%u,\"door\":%d,\"pit\":%d,\"teleporter\":%d}],\"viewportRows\":[",
                r->name, r->gameTick, r->mapIndex, r->mapX, r->mapY, r->direction,
                r->left.mapX, r->left.mapY, (unsigned int)r->left.square, r->left.elementType, (unsigned int)r->left.firstThing, r->left.doorObserved, r->left.pitObserved, r->left.teleporterObserved,
                r->center.mapX, r->center.mapY, (unsigned int)r->center.square, r->center.elementType, (unsigned int)r->center.firstThing, r->center.doorObserved, r->center.pitObserved, r->center.teleporterObserved,
                r->right.mapX, r->right.mapY, (unsigned int)r->right.square, r->right.elementType, (unsigned int)r->right.firstThing, r->right.doorObserved, r->right.pitObserved, r->right.teleporterObserved);
        for (c = 0; c < r->viewportRowCount; ++c) {
            const TurnViewportCellProbe* cell = &r->viewportRows[c];
            static const char* names[19] = {"D4L","D4R","D4C","D3L2","D3R2","D3L","D3R","D3C","D2L2","D2R2","D2L","D2R","D2C","D1L","D1R","D1C","D0L","D0R","D0C"};
            fprintf(js, "%s{\"row\":\"%s\",\"mapX\":%d,\"mapY\":%d,\"valid\":%d,\"square\":%u,\"elementType\":%d,\"firstThing\":%u,\"door\":%d,\"pit\":%d,\"teleporter\":%d}",
                    c ? "," : "", names[c], cell->mapX, cell->mapY, cell->valid, (unsigned int)cell->square, cell->elementType, (unsigned int)cell->firstThing, cell->doorObserved, cell->pitObserved, cell->teleporterObserved);
        }
        fprintf(js, "]}%s\n", i == rowCount - 1 ? "" : ",");
    }
    fprintf(js, "  ]\n}\n");
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

int main(int argc, char** argv) {
    const char* dataDir;
    const char* outDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    TurnViewportSnapshotProbe rows[5];
    int ok = 1;
    int result;

    if (argc < 3) {
        fprintf(stderr, "usage: %s DATA_DIR OUT_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    outDir = argv[2];

    /*
     * Canonical DM1 V1 layout (post LOADSAVE.C MEDIA529 fix):
     *   start (1,3,SOUTH); walls flank east (2,3) and west (0,3) and the
     *   only legal forward-from-start is south to corridor (1,4).
     *
     * Routes exercised:
     *   row 0 start_south       : (1,3,SOUTH).
     *   row 1 turn_right_west   : turn right -> (1,3,WEST).
     *   row 2 forward_west_blocked
     *                            : forward attempt at (1,3,WEST) is blocked
     *                              by wall at (0,3); party stays at
     *                              (1,3,WEST) and the front cell remains the
     *                              wall (0,3).  This proves CLIKMENU.C
     *                              F0366 movement-blocker handling on the
     *                              canonical decode.
     *   row 3 turn_left_east    : fresh game, turn left -> (1,3,EAST).
     *   row 4 forward_south_corridor
     *                            : fresh game, forward south at (1,3,SOUTH)
     *                              succeeds -> (1,4,SOUTH); front cell
     *                              advances to (1,5).  This proves the
     *                              canonical decode actually plays a real
     *                              forward step.
     */
    if (!open_game(dataDir, &menu, &game)) {
        fprintf(stderr, "failed to open DM1 game view\n");
        return 1;
    }
    snapshot(&game, "start_south", M11_GAME_INPUT_REDRAW, &rows[0]);
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);
    snapshot(&game, "turn_right_west", result, &rows[1]);
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    snapshot(&game, "forward_west_blocked", result, &rows[2]);
    M11_GameView_Shutdown(&game);

    if (!open_game(dataDir, &menu, &game)) {
        fprintf(stderr, "failed to reopen DM1 game view\n");
        return 1;
    }
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_LEFT);
    snapshot(&game, "turn_left_east", result, &rows[3]);
    M11_GameView_Shutdown(&game);

    if (!open_game(dataDir, &menu, &game)) {
        fprintf(stderr, "failed to reopen DM1 game view for forward south probe\n");
        return 1;
    }
    result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    snapshot(&game, "forward_south_corridor", result, &rows[4]);

    if (rows[0].result != M11_GAME_INPUT_REDRAW ||
        rows[1].result != M11_GAME_INPUT_REDRAW ||
        rows[2].result != M11_GAME_INPUT_REDRAW ||
        rows[3].result != M11_GAME_INPUT_REDRAW ||
        rows[4].result != M11_GAME_INPUT_REDRAW) ok = 0;

    if (rows[0].mapIndex != 0 || rows[0].mapX != 1 || rows[0].mapY != 3 || rows[0].direction != DIR_SOUTH) ok = 0;
    if (rows[1].mapIndex != 0 || rows[1].mapX != 1 || rows[1].mapY != 3 || rows[1].direction != DIR_WEST) ok = 0;
    /* forward_west_blocked: blocked at (0,3) wall, party stays at (1,3,WEST). */
    if (rows[2].mapIndex != 0 || rows[2].mapX != 1 || rows[2].mapY != 3 || rows[2].direction != DIR_WEST) ok = 0;
    if (rows[3].mapIndex != 0 || rows[3].mapX != 1 || rows[3].mapY != 3 || rows[3].direction != DIR_EAST) ok = 0;
    /* forward_south_corridor: forward south succeeds, party at (1,4,SOUTH). */
    if (rows[4].mapIndex != 0 || rows[4].mapX != 1 || rows[4].mapY != 4 || rows[4].direction != DIR_SOUTH) ok = 0;

    if (rows[0].center.mapX != 1 || rows[0].center.mapY != 4) ok = 0;
    if (rows[0].left.mapX != 2 || rows[0].left.mapY != 4) ok = 0;
    if (rows[0].right.mapX != 0 || rows[0].right.mapY != 4) ok = 0;
    if (rows[1].center.mapX != 0 || rows[1].center.mapY != 3) ok = 0;
    if (rows[1].left.mapX != 0 || rows[1].left.mapY != 4) ok = 0;
    if (rows[1].right.mapX != 0 || rows[1].right.mapY != 2) ok = 0;
    /* row 2 (blocked at (1,3,WEST)): same lane geometry as row 1. */
    if (rows[2].center.mapX != 0 || rows[2].center.mapY != 3) ok = 0;
    if (rows[2].left.mapX != 0 || rows[2].left.mapY != 4) ok = 0;
    if (rows[2].right.mapX != 0 || rows[2].right.mapY != 2) ok = 0;
    if (rows[3].center.mapX != 2 || rows[3].center.mapY != 3) ok = 0;
    if (rows[3].left.mapX != 2 || rows[3].left.mapY != 2) ok = 0;
    if (rows[3].right.mapX != 2 || rows[3].right.mapY != 4) ok = 0;
    /* row 4 (forward south succeeded, party at (1,4,SOUTH)): front=(1,5). */
    if (rows[4].center.mapX != 1 || rows[4].center.mapY != 5) ok = 0;
    if (rows[4].left.mapX != 2 || rows[4].left.mapY != 5) ok = 0;
    if (rows[4].right.mapX != 0 || rows[4].right.mapY != 5) ok = 0;

    /* All sampled cells are within the 18x19 canonical map. */
    if (!rows[0].left.valid || !rows[0].center.valid || !rows[0].right.valid ||
        !rows[1].left.valid || !rows[1].center.valid || !rows[1].right.valid ||
        !rows[2].left.valid || !rows[2].center.valid || !rows[2].right.valid ||
        !rows[3].left.valid || !rows[3].center.valid || !rows[3].right.valid ||
        !rows[4].left.valid || !rows[4].center.valid || !rows[4].right.valid) ok = 0;

    if (rows[0].viewportRowCount != 19 || rows[1].viewportRowCount != 19 || rows[2].viewportRowCount != 19 ||
        rows[3].viewportRowCount != 19 || rows[4].viewportRowCount != 19) ok = 0;
    if (rows[0].viewportRows[15].mapX != 1 || rows[0].viewportRows[15].mapY != 4) ok = 0;
    if (rows[1].viewportRows[15].mapX != 0 || rows[1].viewportRows[15].mapY != 3) ok = 0;
    /* Row 2 stays at (1,3,WEST), so the D1C front-cell sample equals row 1's. */
    if (rows[2].viewportRows[15].mapX != 0 || rows[2].viewportRows[15].mapY != 3) ok = 0;
    if (rows[4].viewportRows[15].mapX != 1 || rows[4].viewportRows[15].mapY != 5) ok = 0;

    if (!write_outputs(outDir, rows, 5)) ok = 0;
    M11_GameView_Shutdown(&game);
    printf("%s turn viewport orientation probe\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
