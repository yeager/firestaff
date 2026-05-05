#include <stdio.h>
#include <string.h>

#include "dm1_v1_movement_command_core_pc34_compat.h"

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static unsigned char square_type(int elementType, int attrs)
{
    return (unsigned char)((elementType << 5) | (attrs & DUNGEON_SQUARE_MASK_ATTRIBS));
}

static void set_square(unsigned char* squares, int height, int x, int y, unsigned char value)
{
    squares[x * height + y] = value;
}

static void setup_dungeon(struct DungeonDatState_Compat* dungeon,
    struct DungeonMapDesc_Compat* map,
    struct DungeonMapTiles_Compat* tiles,
    unsigned char* squares,
    int width,
    int height)
{
    int x;
    int y;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(squares, 0, (size_t)(width * height));
    map->width = (unsigned char)width;
    map->height = (unsigned char)height;
    tiles->squareData = squares;
    tiles->squareCount = width * height;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            set_square(squares, height, x, y, square_type(DUNGEON_ELEMENT_CORRIDOR, 0));
        }
    }
}

static void setup_party(struct PartyState_Compat* party)
{
    memset(party, 0, sizeof(*party));
    party->mapIndex = 0;
    party->mapX = 2;
    party->mapY = 2;
    party->direction = DIR_NORTH;
    party->championCount = 2;
    party->champions[0].present = 1;
    party->champions[0].hp.current = 10;
    party->champions[0].maxLoad = 100;
    party->champions[0].direction = DIR_NORTH;
    party->champions[1].present = 1;
    party->champions[1].hp.current = 10;
    party->champions[1].maxLoad = 100;
    party->champions[1].direction = DIR_EAST;
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[25];
    struct DungeonThings_Compat things;
    struct PartyState_Compat party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1MovementCommandCoreResultPc34Compat result;
    int footwear[CHAMPION_MAX_PARTY] = { 0, 0, 0, 0 };
    int ok = 1;

    printf("probe=dm1_v1_movement_command_core_pc34_compat\n");
    printf("sourceEvidence=%s\n", DM1_V1_MovementCommandCore_SourceEvidencePc34Compat());

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("enqueue turn right", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
    ok &= expect_int("process turn", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 200, 100, footwear, &result), 1);
    ok &= expect_int("turn handled", result.commandHandled, 1);
    ok &= expect_int("turn applied", result.turnApplied, 1);
    ok &= expect_int("turn is no step", result.stepApplied, 0);
    ok &= expect_int("party direction rotates north to east", party.direction, DIR_EAST);
    ok &= expect_int("champion0 direction rotates with party", party.champions[0].direction, DIR_EAST);
    ok &= expect_int("champion1 direction rotates with party", party.champions[1].direction, DIR_SOUTH);
    ok &= expect_int("turn requests viewport redraw", result.viewportRedrawRequested, 1);
    ok &= expect_int("turn releases input wait", result.stopWaitingForPlayerInput, 1);
    ok &= expect_int("turn does not set movement cooldown", result.timing.disabledMovementTicks, 0);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("enqueue forward step", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("process forward step", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 260, 200, footwear, &result), 1);
    ok &= expect_int("step handled", result.commandHandled, 1);
    ok &= expect_int("step applied", result.stepApplied, 1);
    ok &= expect_int("forward x unchanged", party.mapX, 2);
    ok &= expect_int("forward y decremented", party.mapY, 1);
    ok &= expect_int("step requests viewport redraw", result.viewportRedrawRequested, 1);
    ok &= expect_int("step releases input wait", result.stopWaitingForPlayerInput, 1);
    ok &= expect_int("step records scent/last movement time", result.timing.scentRecorded, 1);
    ok &= expect_int("step updates last movement time", (int)result.timing.lastPartyMovementTime, 260);
    ok &= expect_int("step clears projectile cooldown", result.timing.projectileDisabledMovementTicks, 0);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_WALL, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("enqueue blocked forward", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("enqueue queued trailing command", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
    ok &= expect_int("process blocked step", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 320, 260, footwear, &result), 1);
    ok &= expect_int("blocked movement reported", result.movementBlocked, 1);
    ok &= expect_int("blocked input discard requested", result.inputDiscardRequested, 1);
    ok &= expect_int("blocked command does not release wait", result.stopWaitingForPlayerInput, 0);
    ok &= expect_int("blocked command does not redraw viewport", result.viewportRedrawRequested, 0);
    ok &= expect_int("blocked movement leaves x", party.mapX, 2);
    ok &= expect_int("blocked movement leaves y", party.mapY, 2);
    ok &= expect_int("blocked movement flushes queued input", (int)queue.count, 0);

    printf("dm1V1MovementCommandCoreInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
