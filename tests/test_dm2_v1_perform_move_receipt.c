/*
 * test_dm2_v1_perform_move_receipt.c
 *
 * Focused C11 coverage for the skproject DM2_PERFORM_MOVE admission plan.
 */

#include "dm2_v1_perform_move.h"
#include "dm2_v1_world_model.h"
#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <string.h>

int dm2_v1_skproject_append_record_to(
    DM2_V1_DungeonData *d, uint16_t record, uint16_t *parent,
    int level, int x, int y, DM2_V1_SkprojectAppendRecordReceipt *out)
{ (void)d;(void)record;(void)parent;(void)level;(void)x;(void)y;
  if (out) memset(out, 0, sizeof(*out)); return 1; }

int dm2_v1_skproject_cut_record_from(
    DM2_V1_DungeonData *d, uint16_t record, uint16_t *parent,
    int level, int x, int y, DM2_V1_SkprojectCutRecordReceipt *out)
{ (void)d;(void)record;(void)parent;(void)level;(void)x;(void)y;
  if (out) memset(out, 0, sizeof(*out)); return 1; }

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static DM2_V1_PerformMoveRequest base_request(void)
{
    DM2_V1_PerformMoveRequest request;
    memset(&request, 0, sizeof(request));
    request.runtime_ready = 1;
    request.can_move = 1;
    request.current_level = 2;
    request.from_x = 10;
    request.from_y = 12;
    request.from_dir = 0;
    request.direction = 1;
    request.target_raw_valid = 1;
    request.target_raw = 0x20;
    request.target_square_type = DM2_SQUARE_FLOOR;
    return request;
}

int main(void)
{
    DM2_V1_PerformMoveRequest request = base_request();
    DM2_V1_PerformMoveReceipt receipt;

    printf("=== DM2 V1 PERFORM_MOVE Receipt Test ===\n");

    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.valid && receipt.accepted && !receipt.blocked &&
              receipt.to_x == 11 && receipt.to_y == 12 &&
              receipt.to_dir == 1 && receipt.movement_hash != 0u,
          "PERFORM_MOVE accepts a passable dungeon floor step");

    request = base_request();
    request.direction = 4;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.accepted && receipt.to_dir == 0 &&
              receipt.to_x == 10 && receipt.to_y == 11,
          "PERFORM_MOVE normalizes direction to the source N/E/S/W range");

    request = base_request();
    request.can_move = 0;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.blocked &&
              receipt.block_reason == DM2_V1_PERFORM_MOVE_BLOCK_COOLDOWN,
          "PERFORM_MOVE blocks while movement cooldown is active");

    request = base_request();
    request.target_raw_valid = 0;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.blocked &&
              receipt.block_reason == DM2_V1_PERFORM_MOVE_BLOCK_NO_TARGET_TILE,
          "PERFORM_MOVE blocks missing dungeon target tiles");

    request = base_request();
    request.target_square_type = DM2_SQUARE_WALL;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.blocked &&
              receipt.block_reason == DM2_V1_PERFORM_MOVE_BLOCK_WALL,
          "PERFORM_MOVE blocks wall squares");

    request = base_request();
    request.target_square_type = DM2_SQUARE_SECRET_DOOR;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.blocked &&
              receipt.block_reason == DM2_V1_PERFORM_MOVE_BLOCK_WALL,
          "PERFORM_MOVE blocks secret-door wall-family squares");

    request = base_request();
    request.target_square_type = DM2_SQUARE_FAKE_WALL;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.blocked &&
              receipt.block_reason == DM2_V1_PERFORM_MOVE_BLOCK_WALL,
          "PERFORM_MOVE blocks fake-wall wall-family squares");

    request = base_request();
    request.target_square_type = DM2_SQUARE_PIT;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.block_reason == DM2_V1_PERFORM_MOVE_BLOCK_PIT,
          "PERFORM_MOVE blocks pit squares");

    request = base_request();
    request.target_square_type = DM2_SQUARE_LAVA;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.block_reason == DM2_V1_PERFORM_MOVE_BLOCK_LAVA,
          "PERFORM_MOVE blocks lava squares");

    request = base_request();
    request.target_square_type = DM2_SQUARE_INACCESSIBLE;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.block_reason == DM2_V1_PERFORM_MOVE_BLOCK_INACCESSIBLE,
          "PERFORM_MOVE blocks inaccessible squares");

    request = base_request();
    request.target_is_door = 1;
    request.target_door_state = 4;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.block_reason == DM2_V1_PERFORM_MOVE_BLOCK_DOOR_CLOSED,
          "PERFORM_MOVE blocks non-open doors");

    request = base_request();
    request.target_is_door = 1;
    request.target_door_state = 0;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.accepted && !receipt.blocked,
          "PERFORM_MOVE accepts open doors");

    request = base_request();
    request.outdoor = 1;
    request.target_raw_valid = 0;
    request.target_square_type = 0;
    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(&request, &receipt) == 1 &&
              receipt.accepted && receipt.outdoor,
          "PERFORM_MOVE outdoor route does not fabricate dungeon tile blocking");

    CHECK(dm2_v1_DM2_PERFORM_MOVE_plan(NULL, &receipt) == 0,
          "PERFORM_MOVE rejects missing request");
    CHECK(dm2_v1_DM2_PERFORM_MOVE_source_evidence()[0] != '\0',
          "PERFORM_MOVE exposes skproject source evidence");

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
