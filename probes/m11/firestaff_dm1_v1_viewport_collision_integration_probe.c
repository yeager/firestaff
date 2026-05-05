/*
 * DM1 V1 viewport/collision integration probe.
 *
 * Source evidence (primary ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   COMMAND.C:F0380 -> CLIKMENU.C:F0365/F0366 dispatches one queued command.
 *   CLIKMENU.C:F0366 checks wall/door/fake-wall/group before applying steps.
 *   MOVESENS.C:F0267 applies movement side effects and post-move chains.
 *   GAMELOOP.C/DRAWVIEW.C redraws the dungeon view after accepted turns/steps.
 *   DUNVIEW.C:F0128 computes viewport parity from mapX + mapY + direction.
 *   VIEWPORT.C:F0564-F0566 owns 224x136 viewport bitplanes and screen blit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "dm1_v1_collision_door_pc34_compat.h"
#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

#define MAP_W 6
#define MAP_H 6
#define ROUTE_MAX 8

static unsigned char sq(int elementType, int attrs)
{
    return (unsigned char)(((elementType & 7) << 5) | (attrs & 0x1F));
}

static void set_sq(unsigned char* squares, int x, int y, unsigned char value)
{
    squares[x * MAP_H + y] = value;
}

static void setup_dungeon(struct DungeonDatState_Compat* dungeon,
                          struct DungeonThings_Compat* things,
                          struct DungeonMapDesc_Compat* map,
                          struct DungeonMapTiles_Compat* tiles,
                          unsigned char* squares,
                          unsigned short* firstThings)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(things, 0, sizeof(*things));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    for (int i = 0; i < MAP_W * MAP_H; ++i) squares[i] = sq(DUNGEON_ELEMENT_CORRIDOR, 0);

    /* Route fixtures: closed door at (3,1), group thing-list at (4,1). */
    set_sq(squares, 3, 1, sq(DUNGEON_ELEMENT_DOOR, DM1_DOOR_STATE_CLOSED));
    set_sq(squares, 4, 1, sq(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST));

    map->width = MAP_W;
    map->height = MAP_H;
    tiles->squareData = squares;
    tiles->squareCount = MAP_W * MAP_H;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    firstThings[0] = (unsigned short)((THING_TYPE_GROUP << 10) | 0);
    things->squareFirstThings = firstThings;
    things->squareFirstThingCount = 1;
    things->groupCount = 1;
    things->loaded = 1;
}

static void setup_party(struct PartyState_Compat* party)
{
    memset(party, 0, sizeof(*party));
    party->mapIndex = 0;
    party->mapX = 2;
    party->mapY = 2;
    party->direction = DIR_NORTH;
    party->championCount = 1;
    party->champions[0].present = 1;
    party->champions[0].hp.current = 100;
    party->champions[0].hp.maximum = 100;
    party->champions[0].load = 100;
    party->champions[0].maxLoad = 500;
}

static struct Dm1V1InputEventPc34Compat key_event(int keyCode)
{
    struct Dm1V1InputEventPc34Compat ev;
    memset(&ev, 0, sizeof(ev));
    ev.kind = DM1_V1_INPUT_KIND_KEY;
    ev.keyCode = keyCode;
    return ev;
}

static int action_for_label(const char* label)
{
    if (strcmp(label, "forward") == 0) return MOVE_FORWARD;
    if (strcmp(label, "back") == 0) return MOVE_BACKWARD;
    if (strcmp(label, "right") == 0) return MOVE_RIGHT;
    if (strcmp(label, "left") == 0) return MOVE_LEFT;
    return -1;
}

static int key_for_label(const char* label)
{
    if (strcmp(label, "turn_right") == 0) return 0xAB36;
    if (strcmp(label, "turn_left") == 0) return 0xAB34;
    if (strcmp(label, "forward") == 0) return 0xAB35;
    if (strcmp(label, "back") == 0) return 0xAB33;
    if (strcmp(label, "right") == 0) return 0xAB32;
    if (strcmp(label, "left") == 0) return 0xAB31;
    return 0;
}

static const char* collision_code_name(int code)
{
    switch (code) {
    case DM1_COLLISION_PASSABLE: return "passable";
    case DM1_COLLISION_BLOCKED_WALL: return "blocked_wall";
    case DM1_COLLISION_BLOCKED_DOOR: return "blocked_door";
    case DM1_COLLISION_BLOCKED_FAKEWALL: return "blocked_fakewall";
    case DM1_COLLISION_BLOCKED_BOUNDS: return "blocked_bounds";
    case DM1_COLLISION_BLOCKED_GROUP: return "blocked_group";
    default: return "unknown";
    }
}

struct Row {
    int frame;
    const char* label;
    const char* command;
    int cooldownDrained;
    int beforeX, beforeY, beforeDir;
    int targetX, targetY;
    int collisionCode;
    int doorState;
    int doorPassable;
    int turnApplied;
    int stepApplied;
    int movementBlocked;
    int blockedByGroup;
    int viewportDirty;
    int viewportDrawn;
    int presented;
    int afterX, afterY, afterDir;
    int parityFlip;
    int floorDirty;
};

static void drain_cooldown(struct Dm1V1MovementPipelinePc34Compat* pipeline, int* drained)
{
    *drained = 0;
    while (pipeline->disabledMovementTicks > 0 || pipeline->projectileDisabledMovementTicks > 0) {
        DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(pipeline);
        (*drained)++;
        if (*drained > 1000) break;
    }
}

static void write_outputs(const char* outdir, const struct Row* rows, int count)
{
    char path[512];
    FILE* f;

    snprintf(path, sizeof(path), "%s/viewport_collision_route.tsv", outdir);
    f = fopen(path, "w");
    if (!f) exit(2);
    fprintf(f, "frame\tlabel\tcommand\tcooldown_drained\tbefore_x\tbefore_y\tbefore_dir\ttarget_x\ttarget_y\tcollision\tdoor_state\tdoor_passable\tturn_applied\tstep_applied\tmovement_blocked\tblocked_by_group\tviewport_dirty\tviewport_drawn\tpresented\tafter_x\tafter_y\tafter_dir\tparity_flip\tfloor_dirty\n");
    for (int i = 0; i < count; ++i) {
        fprintf(f, "%d\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                rows[i].frame, rows[i].label, rows[i].command, rows[i].cooldownDrained,
                rows[i].beforeX, rows[i].beforeY, rows[i].beforeDir,
                rows[i].targetX, rows[i].targetY, collision_code_name(rows[i].collisionCode),
                rows[i].doorState, rows[i].doorPassable, rows[i].turnApplied, rows[i].stepApplied,
                rows[i].movementBlocked, rows[i].blockedByGroup, rows[i].viewportDirty,
                rows[i].viewportDrawn, rows[i].presented, rows[i].afterX, rows[i].afterY,
                rows[i].afterDir, rows[i].parityFlip, rows[i].floorDirty);
    }
    fclose(f);

    snprintf(path, sizeof(path), "%s/viewport_collision_route.json", outdir);
    f = fopen(path, "w");
    if (!f) exit(2);
    fprintf(f, "{\n  \"source\": \"ReDMCSB WIP20210206 Toolchains/Common/Source COMMAND.C CLIKMENU.C MOVESENS.C DUNVIEW.C DRAWVIEW.C VIEWPORT.C\",\n  \"rows\": [\n");
    for (int i = 0; i < count; ++i) {
        fprintf(f, "    {\"frame\":%d,\"label\":\"%s\",\"command\":\"%s\",\"cooldown_drained\":%d,\"before\":[%d,%d,%d],\"target\":[%d,%d],\"collision\":\"%s\",\"door_state\":%d,\"door_passable\":%d,\"turn_applied\":%d,\"step_applied\":%d,\"movement_blocked\":%d,\"blocked_by_group\":%d,\"viewport_dirty\":%d,\"viewport_drawn\":%d,\"presented\":%d,\"after\":[%d,%d,%d],\"parity_flip\":%d,\"floor_dirty\":%d}%s\n",
                rows[i].frame, rows[i].label, rows[i].command, rows[i].cooldownDrained,
                rows[i].beforeX, rows[i].beforeY, rows[i].beforeDir,
                rows[i].targetX, rows[i].targetY, collision_code_name(rows[i].collisionCode),
                rows[i].doorState, rows[i].doorPassable, rows[i].turnApplied, rows[i].stepApplied,
                rows[i].movementBlocked, rows[i].blockedByGroup, rows[i].viewportDirty,
                rows[i].viewportDrawn, rows[i].presented, rows[i].afterX, rows[i].afterY,
                rows[i].afterDir, rows[i].parityFlip, rows[i].floorDirty,
                (i + 1 == count) ? "" : ",");
    }
    fprintf(f, "  ]\n}\n");
    fclose(f);
}

int main(int argc, char** argv)
{
    const char* outdir = argc > 1 ? argv[1] : ".";
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[MAP_W * MAP_H];
    unsigned short firstThings[1];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t temp_bitmap[160 * 111];
    uint8_t screen_pixels[320 * 200];
    DM1_Viewport3DState viewport;
    struct Row rows[ROUTE_MAX];
    int rowCount = 0;

    const char* route[][2] = {
        {"open_step", "forward"},
        {"turn_to_door", "turn_right"},
        {"closed_door_block", "forward"},
        {"open_door_step", "forward"},
        {"group_block", "forward"},
        {"turn_after_blocks", "turn_right"}
    };

    setup_dungeon(&dungeon, &things, &map, &tiles, squares, firstThings);
    setup_party(&party);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    memset(screen_pixels, 0, sizeof(screen_pixels));
    memset(temp_bitmap, 0x0A, sizeof(temp_bitmap));
    dm1_viewport_3d_init(&viewport, viewport_pixels, DM1_VIEWPORT_WIDTH);
    viewport.temp_bitmap = temp_bitmap;
    viewport.temp_bitmap_size = (int)sizeof(temp_bitmap);

    for (int i = 0; i < 6; ++i) {
        struct Dm1V1CollisionResult collision;
        struct Row* r = &rows[rowCount++];
        int drained = 0;
        int action = action_for_label(route[i][1]);
        int key = key_for_label(route[i][1]);
        memset(r, 0, sizeof(*r));
        memset(&collision, 0, sizeof(collision));
        collision.code = -1;
        collision.doorState = -1;

        drain_cooldown(&pipeline, &drained);
        if (strcmp(route[i][0], "open_door_step") == 0) {
            set_sq(squares, 3, 1, sq(DUNGEON_ELEMENT_DOOR, DM1_DOOR_STATE_OPEN));
        }

        r->frame = i + 1;
        r->label = route[i][0];
        r->command = route[i][1];
        r->cooldownDrained = drained;
        r->beforeX = party.mapX;
        r->beforeY = party.mapY;
        r->beforeDir = party.direction;
        r->targetX = -1;
        r->targetY = -1;
        r->collisionCode = -1;
        r->doorState = -1;
        r->doorPassable = -1;

        if (action >= 0) {
            (void)DM1_V1_Collision_CheckStep(&dungeon, &things, &party, action, &collision);
            r->targetX = collision.mapX;
            r->targetY = collision.mapY;
            r->collisionCode = collision.code;
            r->doorState = collision.doorState;
            r->doorPassable = collision.doorPassable;
        }

        if (!key || !DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline, key_event(key))) {
            fprintf(stderr, "failed to enqueue %s\n", route[i][1]);
            return 1;
        }
        if (!DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(&pipeline, &dungeon, &things, &party, NULL, &result)) {
            fprintf(stderr, "failed to process %s\n", route[i][1]);
            return 1;
        }

        r->turnApplied = result.core.turnApplied;
        r->stepApplied = result.core.stepApplied;
        r->movementBlocked = result.core.movementBlocked;
        r->blockedByGroup = result.core.blockedByGroup;
        r->viewportDirty = result.viewportDirty;
        if (result.viewportDirty) {
            dm1_viewport_3d_draw_frame(&viewport, party.direction, party.mapX, party.mapY);
            dm1_viewport_3d_present(&viewport, screen_pixels, 320, 1);
            r->viewportDrawn = 1;
            r->presented = 1;
        }
        r->afterX = party.mapX;
        r->afterY = party.mapY;
        r->afterDir = party.direction;
        r->parityFlip = viewport.parity_flip ? 1 : 0;
        r->floorDirty = viewport.floor_ceiling_dirty ? 1 : 0;
    }

    write_outputs(outdir, rows, rowCount);
    printf("# summary: %d viewport/collision route frames written to %s\n", rowCount, outdir);
    printf("# evidence: %s\n", DM1_V1_MovementPipeline_SourceEvidencePc34Compat());
    printf("# collision: %s\n", DM1_V1_CollisionDoor_SourceEvidence());
    return 0;
}
