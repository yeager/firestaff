/*
 * test_dm2_v1_record_pool_pc34_compat.c — DM2-002 c_record pool ownership.
 *
 * Verifies the source-ordered record-pool layer against skproject anchors:
 *   c_record.cpp:28-31  table_recordsizes
 *   c_record.cpp:44-52  DM2_GET_ADDRESS_OF_RECORD decode
 *   c_record.cpp:54-57  DM2_GET_NEXT_RECORD_LINK
 *   c_record.cpp:60-170 APPEND/CUT list paths
 *   c_moverec.cpp       DM2_MOVE_RECORD_TO relocation boundary
 */

#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_sksave_game_load_owner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

static void wr16(uint8_t *p, int16_t v)
{
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u & 0xffu);
    p[1] = (uint8_t)((u >> 8) & 0xffu);
}

/* Small structural fixture for c_record ownership: DB0 (4-byte) x3, DB2
 * (Text, 4-byte) x2 and DB3 (8-byte) x2. It is not game media. */
static void build_synthetic(DM2_V1_RecordPoolSet *set)
{
    memset(set, 0, sizeof(*set));
    set->pools[0].record_size = 4;
    set->pools[0].record_count = 3;
    set->pools[0].source_base = 0;
    set->pools[0].bytes = calloc(3, 4);
    set->pools[2].record_size = 4;
    set->pools[2].record_count = 2;
    set->pools[2].source_base = 12;
    set->pools[2].bytes = calloc(2, 4);
    set->pools[3].record_size = 8;
    set->pools[3].record_count = 2;
    set->pools[3].source_base = 20;
    set->pools[3].bytes = calloc(2, 8);
    set->pools[3].extension_count = 2;
    set->pools[3].extension_base = 28;
    set->pools[3].extension_bytes = calloc(2, 8);
    set->pools[4].record_size = 16;
    set->pools[4].record_count = 1;
    set->pools[4].source_base = 44;
    set->pools[4].bytes = calloc(1, 16);
    set->pools[14].record_size = 8;
    set->pools[14].record_count = 1;
    set->pools[14].source_base = 60;
    set->pools[14].bytes = calloc(1, 8);
    set->valid = 1;
    set->record_graph_complete = 1;
}

static int16_t mk_handle(int pool, int index)
{
    return (int16_t)((pool << 10) | (index & 0x3ff));
}


/* c_record.cpp::DM2_RECYCLE_A_RECORD_FROM_THE_WORLD traversal. It starts at
 * the per-DB cursor, skips the current map and the second protected map, and
 * writes the resume cursor back (source :779/:1072). DB2 is a source chain
 * barrier and is tested below; no DB remains recyclable in this bounded API. */
static void test_recycle_scan_traversal(void)
{
    DM2_V1_OriginalRawDungeonReceipt dungeon;
    DM2_V1_SksaveMapOwner owner;
    DM2_V1_RecordPoolSet set;
    DM2_V1_SksaveRecycleScanReceipt receipt;
    uint16_t link = 0u;
    uint16_t ground[8];
    uint16_t columns[4] = { 0u, 1u, 2u, 3u };
    /* Every tile has bit 0x10 clear, so ground_index reports "no ground
     * stack here" and the walk proceeds without needing column indices. */
    uint8_t tiles[4];
    int rc;

    build_synthetic(&set);

    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.valid = 1;
    dungeon.map_count = 4;
    for (int i = 0; i < 4; ++i) {
        dungeon.map_widths[i] = 1;
        dungeon.map_heights[i] = 1;
    }

    memset(&owner, 0, sizeof(owner));
    memset(ground, 0xff, sizeof(ground));   /* every stack empty (END) */
    owner.valid = 1;
    /* ground_index requires the immutable source body pointer. */
    owner.raw_body = tiles;
    owner.raw_body_size = sizeof(tiles);
    owner.dungeon = &dungeon;
    owner.ground_stack_links = ground;
    owner.ground_stack_count = 8u;
    owner.column_indices = columns;
    owner.column_index_count = 4u;
    memset(tiles, 0, sizeof(tiles));
    owner.map_tiles = tiles;
    owner.map_tiles_size = sizeof(tiles);
    for (int i = 0; i < 4; ++i) owner.map_tile_offsets[i] = (uint16_t)i;
    owner.current_map = 1;

    /* db 0x0f is rejected outright by the source. */
    rc = dm2_v1_sksave_map_owner_recycle_scan(&owner, &set, 15, -1,
                                              &receipt, &link);
    CHECK(rc == 0 && receipt.valid == 0,
          "recycle scan rejects db 0x0f like the source");

    /* Cursor starts at 0; current_map 1 and protected map 3 are skipped, so
     * only maps 0 and 2 are walked. */
    owner.recycle_scan_map[4] = 0;
    rc = dm2_v1_sksave_map_owner_recycle_scan(&owner, &set, 4, 3,
                                              &receipt, &link);
    CHECK(rc == 0, "non-DB2 recycle scan remains fail-closed");
    CHECK(receipt.valid == 1 && receipt.eligibility_ported == 0,
          "receipt reports the unported selection half for DB4");
    CHECK(receipt.maps_scanned == 2,
          "current map and the second protected map are both skipped");
    CHECK(link == (uint16_t)DM2_V1_RECORD_HANDLE_END,
          "no record handle is produced");

    /* c_record.cpp:779 writes this only on a completed recycler transaction.
     * The diagnostic fail-closed walk reports it without mutating the owner. */
    CHECK(owner.recycle_scan_map[4] == 0 && receipt.resume_map < dungeon.map_count,
          "failed recycle scan retains its cursor while reporting the source next map");

    /* A cursor past the map count wraps rather than reading out of range. */
    owner.recycle_scan_map[5] = 200;
    rc = dm2_v1_sksave_map_owner_recycle_scan(&owner, &set, 5, -1,
                                              &receipt, &link);
    CHECK(rc == 0 && receipt.valid == 1 && receipt.maps_scanned == 3,
          "an out-of-range cursor wraps and skips only the current map");

    /* Per-DB cursors are independent. */
    CHECK(owner.recycle_scan_map[4] != 200,
          "each db keeps its own cursor");

    /* SKWINDOS/dm2byg.cpp 0CEE:10EC-112A treats ordinary DB2 Text as a
     * chain barrier, then advances to the next record. It must never be
     * returned as an allocation candidate. */
    tiles[0] = 0x10u;
    ground[0] = (uint16_t)mk_handle(2, 0);
    wr16(set.pools[2].bytes, (int16_t)DM2_V1_RECORD_HANDLE_END);
    wr16(set.pools[2].bytes + 2u, 0);
    owner.recycle_scan_map[2] = 0;
    rc = dm2_v1_sksave_map_owner_recycle_scan(&owner, &set, 2, -1,
                                              &receipt, &link);
    CHECK(rc == 0 && receipt.valid == 1 && receipt.eligibility_ported == 0,
          "DB2 Text remains a source recycler chain barrier");
    CHECK(link == (uint16_t)DM2_V1_RECORD_HANDLE_END,
          "DB2 Text is never returned as a recycler handle");

    /* Text mode with extension 4 is the source-protected case and skips the
     * entire tile chain instead of recycling that record. */
    wr16(set.pools[2].bytes + 2u, (int16_t)(0x0002u | (4u << 11)));
    owner.recycle_scan_map[2] = 0;
    rc = dm2_v1_sksave_map_owner_recycle_scan(&owner, &set, 2, -1,
                                              &receipt, &link);
    CHECK(rc == 0 && receipt.valid == 1 &&
          link == (uint16_t)DM2_V1_RECORD_HANDLE_END,
          "DB2 recycler preserves source-protected map text");

    /* OBJECT_NULL is not an empty-chain terminator.  The read-only scan must
     * reject a malformed tail before it can report any recycler state. */
    wr16(set.pools[2].bytes + 0u, (int16_t)DM2_V1_RECORD_HANDLE_NULL);
    owner.recycle_scan_map[2] = 0;
    rc = dm2_v1_sksave_map_owner_recycle_scan(&owner, &set, 2, -1,
                                              &receipt, &link);
    CHECK(rc == 0 && receipt.valid == 0 &&
              link == (uint16_t)DM2_V1_RECORD_HANDLE_END,
          "recycle scan rejects a mid-chain OBJECT_NULL tail");

    dm2_v1_record_pool_set_free(&set);
    printf("  recycle scan traversal OK\n");
}

/* The public map diagnostic deliberately cannot select a record. This tiny
 * source-structural fixture exercises the retained GAME_LOAD owner's DB0
 * candidate only: no pool, tile, map cursor or session is modified. The
 * production regression remains the real PC-DOS SKSAVE corpus. */
static void test_private_db0_recycler_candidate(void)
{
    DM2_V1_OriginalRawDungeonReceipt dungeon;
    DM2_V1_SksaveMapOwner map_owner;
    DM2_V1_SksaveGameLoadOwner game_owner;
    DM2_V1_RecordPoolSet set;
    DM2_V1_SksaveDb0RecyclerCandidate candidate;
    uint16_t *ground = calloc(4u, sizeof(*ground));
    uint16_t *columns = calloc(4u, sizeof(*columns));
    uint8_t *tiles = calloc(4u, sizeof(*tiles));
    uint8_t cursors_before[18];
    uint16_t ground_before[4];

    build_synthetic(&set);
    CHECK(ground != NULL && columns != NULL && tiles != NULL,
          "private recycler fixture allocates map-owner storage");
    if (!ground || !columns || !tiles) {
        dm2_v1_record_pool_set_free(&set);
        free(ground);
        free(columns);
        free(tiles);
        return;
    }
    for (int i = 0; i < 4; ++i) {
        columns[i] = (uint16_t)i;
        tiles[i] = 0x10u;
    }
    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.valid = 1;
    dungeon.map_count = 4;
    for (int i = 0; i < 4; ++i) {
        dungeon.map_widths[i] = 1;
        dungeon.map_heights[i] = 1;
    }
    memset(&map_owner, 0, sizeof(map_owner));
    map_owner.valid = 1;
    map_owner.raw_body = tiles;
    map_owner.raw_body_size = sizeof(tiles);
    map_owner.dungeon = &dungeon;
    map_owner.ground_stack_links = ground;
    map_owner.ground_stack_count = 4u;
    map_owner.column_indices = columns;
    map_owner.column_index_count = 4u;
    map_owner.map_tiles = tiles;
    map_owner.map_tiles_size = sizeof(tiles);
    for (int i = 0; i < 4; ++i) map_owner.map_tile_offsets[i] = (uint16_t)i;
    map_owner.current_map = 1;

    memset(&game_owner, 0, sizeof(game_owner));
    game_owner.valid = 1;
    game_owner.map_owner = map_owner;
    CHECK(dm2_v1_record_pool_set_clone(&game_owner.record_pools, &set),
          "private recycler fixture clones its source pool owner");
    game_owner.recycler_context.valid = 1;
    game_owner.recycler_context.map_count = 4u;
    game_owner.recycler_context.current_map = 1u;
    game_owner.recycler_context.party_map = 1u;
    game_owner.recycler_context.protected_map = -1;
    game_owner.recycler_context.party_x = 0u;
    game_owner.recycler_context.party_y = 0u;

    /* First map in the source ring owns a direct DB0 candidate. */
    for (int i = 0; i < 4; ++i) ground[i] = (uint16_t)DM2_V1_RECORD_HANDLE_END;
    ground[0] = (uint16_t)mk_handle(0, 0);
    wr16(set.pools[0].bytes, (int16_t)DM2_V1_RECORD_HANDLE_END);
    memcpy(cursors_before, game_owner.recycler_context.map_cursors,
           sizeof(cursors_before));
    memcpy(ground_before, ground, sizeof(ground_before));
    memset(&candidate, 0, sizeof(candidate));
    CHECK(dm2_v1_sksave_game_load_owner_db0_recycler_candidate(
              &game_owner, &candidate) && candidate.valid && candidate.found &&
              candidate.selected_link == (uint16_t)mk_handle(0, 0) &&
              candidate.selected_map == 0u && candidate.cursor_after == 0u &&
              candidate.maps_scanned == 1u,
          "private DB0 recycler returns the first source-ring candidate");
    CHECK(memcmp(cursors_before, game_owner.recycler_context.map_cursors,
                 sizeof(cursors_before)) == 0 &&
              memcmp(ground_before, ground, sizeof(ground_before)) == 0 &&
              game_owner.map_owner.current_map == 1 &&
              !game_owner.source_game_load_session_ready,
          "private DB0 recycler candidate leaves c_map, cursor and session untouched");

    /* c_record.cpp:DAF9-DB2A only applies DB2's TextMode barrier.  A DB2
     * request then follows DACC to the next link; it must never be exposed
     * as a direct-return candidate at DB88. */
    memset(&candidate, 0, sizeof(candidate));
    CHECK(!dm2_v1_sksave_game_load_owner_recycler_candidate(
              &game_owner, 2u, &candidate) && !candidate.valid &&
              !candidate.found,
          "private recycler rejects DB2 because source never returns Text at DB88");

    /* DB3 table1d324c stops the tile chain before its following DB0 link. */
    ground[0] = (uint16_t)mk_handle(3, 0);
    wr16(game_owner.record_pools.pools[3].bytes, mk_handle(0, 1));
    wr16(game_owner.record_pools.pools[3].bytes + 2u, 1); /* table1d324c subtype 1 blocks */
    wr16(game_owner.record_pools.pools[0].bytes + 4u, (int16_t)DM2_V1_RECORD_HANDLE_END);
    ground[2] = (uint16_t)mk_handle(0, 2);
    wr16(game_owner.record_pools.pools[0].bytes + 8u, (int16_t)DM2_V1_RECORD_HANDLE_END);
    memset(&candidate, 0, sizeof(candidate));
    CHECK(dm2_v1_sksave_game_load_owner_db0_recycler_candidate(
              &game_owner, &candidate) && candidate.found &&
              candidate.selected_link == (uint16_t)mk_handle(0, 2) &&
              candidate.selected_map == 2u,
          "private DB0 recycler keeps DB3 actuator barriers ahead of DB0");

    /* Protected map text similarly ends its tile chain before DB0. */
    ground[0] = (uint16_t)mk_handle(2, 0);
    wr16(game_owner.record_pools.pools[2].bytes, mk_handle(0, 1));
    wr16(game_owner.record_pools.pools[2].bytes + 2u, (int16_t)(0x0002u | (4u << 11)));
    memset(&candidate, 0, sizeof(candidate));
    CHECK(dm2_v1_sksave_game_load_owner_db0_recycler_candidate(
              &game_owner, &candidate) && candidate.found &&
              candidate.selected_link == (uint16_t)mk_handle(0, 2) &&
              candidate.selected_map == 2u,
          "private DB0 recycler keeps protected DB2 text ahead of DB0");

    /* A static DB4 possession chain is the one DB0 descent allowed before
     * the runtime owns DELETE_CREATURE_RECORD. */
    ground[0] = (uint16_t)mk_handle(4, 0);
    ground[2] = (uint16_t)DM2_V1_RECORD_HANDLE_END;
    wr16(game_owner.record_pools.pools[4].bytes, mk_handle(0, 1));
    game_owner.record_pools.pools[4].bytes[4] = 0x42u;
    game_owner.retained_creature_ai_valid[0x42u] = 1u;
    game_owner.retained_creature_ai_flags[0x42u] = 1u;
    memset(&candidate, 0, sizeof(candidate));
    CHECK(dm2_v1_sksave_game_load_owner_db0_recycler_candidate(
              &game_owner, &candidate) && candidate.found &&
              candidate.selected_link == (uint16_t)mk_handle(0, 1) &&
              candidate.static_possession_descents == 1u,
          "private DB0 recycler descends only retained static creature possessions");

    /* DB0's source direct-return path is now committed only inside a cloned
     * private owner: the selected record is cleared and the source cursor is
     * advanced, while Resume remains blocked. */
    /* Keep the selected DB0 behind a retained static DB4 predecessor. This
     * is the case that catches publishing an uncut pool clone: the live
     * c_map cut must be committed with the same pool graph. */
    ground[0] = (uint16_t)mk_handle(4, 0);
    ground[2] = (uint16_t)DM2_V1_RECORD_HANDLE_END;
    wr16(game_owner.record_pools.pools[4].bytes, mk_handle(0, 0));
    game_owner.record_pools.pools[4].bytes[4] = 0x42u;
    game_owner.retained_creature_ai_valid[0x42u] = 1u;
    game_owner.retained_creature_ai_flags[0x42u] = 1u;
    wr16(game_owner.record_pools.pools[0].bytes, (int16_t)DM2_V1_RECORD_HANDLE_END);
    game_owner.record_pools.pools[0].bytes[2] = 0x7au;
    game_owner.recycler_context.map_cursors[0] = 0u;
    uint16_t committed = (uint16_t)DM2_V1_RECORD_HANDLE_END;
    CHECK(dm2_v1_sksave_game_load_owner_commit_db0_recycler(
              &game_owner, &committed) && committed == (uint16_t)mk_handle(0, 0) &&
              game_owner.record_pools.pools[0].bytes[0] == 0xfeu &&
              game_owner.record_pools.pools[0].bytes[1] == 0xffu &&
              game_owner.record_pools.pools[0].bytes[2] == 0u &&
              game_owner.map_owner.ground_stack_links[0] ==
                  (uint16_t)mk_handle(4, 0) &&
              ((uint16_t)game_owner.record_pools.pools[4].bytes[0] |
               ((uint16_t)game_owner.record_pools.pools[4].bytes[1] << 8)) ==
                  (uint16_t)DM2_V1_RECORD_HANDLE_END &&
              game_owner.recycler_context.map_cursors[0] == 0u &&
              !game_owner.source_game_load_session_ready,
          "private DB0 recycler commit unlinks and clears the selected source record atomically");
    ground = game_owner.map_owner.ground_stack_links;

    /* c_record.cpp:1427 DELETE_MISSILE_RECORD first queries the creature at
     * the missile tile when its owner argument is NULL. The SKSAVE adapter
     * admits only the no-creature cut/dealloc shape; a DB4 on that tile must
     * remain blocked until the full creature/timer side effects are owned. */
    for (int i = 0; i < 4; ++i)
        ground[i] = (uint16_t)DM2_V1_RECORD_HANDLE_END;
    ground[0] = (uint16_t)mk_handle(14, 0);
    wr16(game_owner.record_pools.pools[14].bytes,
         (int16_t)DM2_V1_RECORD_HANDLE_END);
    game_owner.recycler_context.map_cursors[14] = 0u;
    memset(&candidate, 0, sizeof(candidate));
    CHECK(dm2_v1_sksave_game_load_owner_recycler_candidate(
              &game_owner, 14u, &candidate) && candidate.valid &&
              candidate.found && candidate.selected_link ==
                  (uint16_t)mk_handle(14, 0),
          "private DB14 recycler admits a no-creature tile for missile delete");

    game_owner.state.timer_count = 1u;
    game_owner.timers[0].bytes[4] = 0x1eu;
    game_owner.timers[0].bytes[6] = (uint8_t)mk_handle(14, 0);
    game_owner.timers[0].bytes[7] = (uint8_t)(mk_handle(14, 0) >> 8);
    game_owner.timer_indices[0] = 3; /* sorted heap order is not slot identity */
    DM2_V1_SksaveDb14MissileDeleteCandidate missile_candidate;
    memset(&missile_candidate, 0, sizeof(missile_candidate));
    CHECK(dm2_v1_sksave_game_load_owner_db14_missile_delete_candidate(
              &game_owner, &missile_candidate) && missile_candidate.valid &&
              missile_candidate.missile_record == (uint16_t)mk_handle(14, 0) &&
              missile_candidate.timer_index == 0,
          "private DB14 delete binds the direct 0x1e timer slot, not heap order");
    uint16_t deleted_missile = DM2_V1_RECORD_HANDLE_END;
    CHECK(dm2_v1_sksave_game_load_owner_commit_db14_missile_delete(
              &game_owner, &deleted_missile) &&
              deleted_missile == (uint16_t)mk_handle(14, 0) &&
              game_owner.record_pools.pools[14].bytes[0] == 0xffu &&
              game_owner.record_pools.pools[14].bytes[1] == 0xffu &&
              game_owner.map_owner.ground_stack_links[0] ==
                  (uint16_t)DM2_V1_RECORD_HANDLE_END &&
              game_owner.timers[0].bytes[4] == 0u &&
              game_owner.timer_queue_count == 0 &&
              !game_owner.source_game_load_session_ready,
          "private DB14 delete commits map, record and timer atomically");

    ground[0] = (uint16_t)mk_handle(4, 0);
    wr16(game_owner.record_pools.pools[4].bytes,
         (int16_t)mk_handle(14, 0));
    game_owner.record_pools.pools[4].bytes[4] = 0x42u;
    game_owner.retained_creature_ai_valid[0x42u] = 1u;
    game_owner.retained_creature_ai_flags[0x42u] = 0u;
    memset(&candidate, 0, sizeof(candidate));
    CHECK(dm2_v1_sksave_game_load_owner_recycler_candidate(
              &game_owner, 14u, &candidate) && candidate.valid &&
              !candidate.found,
          "private DB14 recycler blocks a tile with a DB4 creature owner");

    dm2_v1_sksave_game_load_owner_free(&game_owner);
    dm2_v1_record_pool_set_free(&set);
}

/* A rejected replacement must retain the preceding complete private owner.
 * This uses only a test-local pool owner; no game data or recycler selection
 * is fabricated. */
static void test_sksave_owner_replacement_is_atomic(void)
{
    DM2_V1_SksaveGameLoadOwner owner;
    DM2_V1_RecordPoolSet owned_pools;
    uint8_t *pool_bytes;

    memset(&owner, 0, sizeof(owner));
    build_synthetic(&owned_pools);
    owner.lifecycle_tag = DM2_V1_SKSAVE_GAME_LOAD_OWNER_LIFECYCLE_TAG;
    owner.valid = 1;
    owner.record_pools = owned_pools;
    memset(&owned_pools, 0, sizeof(owned_pools));
    pool_bytes = owner.record_pools.pools[0].bytes;

    CHECK(!dm2_v1_sksave_game_load_owner_init(&owner, NULL, 0u, 0u,
                                               NULL, NULL, NULL) &&
              owner.lifecycle_tag == DM2_V1_SKSAVE_GAME_LOAD_OWNER_LIFECYCLE_TAG &&
              owner.valid && owner.record_pools.valid &&
              owner.record_pools.pools[0].bytes == pool_bytes,
          "SKSave owner replacement preserves prior RAM pools on failure");
    dm2_v1_sksave_game_load_owner_free(&owner);
}

int main(void)
{
    DM2_V1_RecordPoolSet set;
    int16_t next;
    int16_t head_a;
    int16_t head_b;

    /* ── table_recordsizes (c_record.cpp:28-31) ─────────────────── */
    {
        static const int expect[16] = {
            4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
        };
        for (int i = 0; i < 16; ++i) {
            CHECK(dm2_v1_record_pool_record_size(i) == expect[i],
                  "record size table matches skproject table_recordsizes");
        }
        CHECK(dm2_v1_record_pool_record_size(-1) == 0, "pool -1 size 0");
        CHECK(dm2_v1_record_pool_record_size(16) == 0, "pool 16 size 0");
    }

    /* READ_SKSAVE_DUNGEON writes each decoded direct root into the already
     * source-materialised c_hero inventory, followed by leader hand. */
    {
        DM2_V1_Hero heroes[DM2_MAX_HEROES];
        DM2_V1_SksaveDirectRootReceipt roots;
        uint16_t leader_hand = 0u;
        uint32_t root_hash = 0u;
        size_t i;

        memset(heroes, 0, sizeof(heroes));
        memset(&roots, 0, sizeof(roots));
        roots.valid = 1;
        roots.root_count = (uint16_t)(2u * DM2_NUM_ITEMS + 1u);
        for (i = 0u; i < roots.root_count; ++i) {
            roots.roots[i] = (uint16_t)(0x4000u | (uint16_t)i);
        }
        CHECK(dm2_v1_sksave_apply_direct_roots_to_heroes(
                  heroes, DM2_MAX_HEROES, 2u, &roots, &leader_hand,
                  &root_hash),
              "direct roots bind source c_hero inventory owners");
        CHECK((uint16_t)heroes[0].item[0] == 0x4000u &&
                  (uint16_t)heroes[1].item[DM2_NUM_ITEMS - 1] == 0x403bu &&
                  leader_hand == 0x403cu && root_hash != 0u,
              "direct-root order retains all 30 hero slots then leader hand");
        roots.root_count--;
        CHECK(!dm2_v1_sksave_apply_direct_roots_to_heroes(
                   heroes, DM2_MAX_HEROES, 2u, &roots, NULL, NULL),
              "short direct-root owner fails before c_hero mutation");
    }

    /* ── handle decode (c_record.cpp:44-52) ─────────────────────── */
    CHECK(dm2_v1_record_handle_pool(mk_handle(4, 173)) == 4, "pool decode");
    CHECK(dm2_v1_record_handle_index(mk_handle(4, 173)) == 173,
          "index decode");
    /* Direction bits 14-15 shift out exactly like the source. */
    CHECK(dm2_v1_record_handle_pool((int16_t)(0xC000u | (4u << 10) | 7u)) == 4,
          "direction bits masked from pool");
    CHECK(dm2_v1_record_handle_index((int16_t)(0xC000u | (4u << 10) | 7u)) == 7,
          "direction bits masked from index");

    /* ── address / link semantics on synthetic pools ────────────── */
    build_synthetic(&set);
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(0, 0)) == set.pools[0].bytes,
          "DB0 record 0 address");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(0, 2)) ==
              set.pools[0].bytes + 8,
          "DB0 record 2 address (ofs = size * index)");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(0, 3)) == NULL,
          "out-of-range index fails closed");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(3, 2)) ==
              set.pools[3].extension_bytes,
          "G1 continuation starts at the declared pool count");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(3, 3)) ==
              set.pools[3].extension_bytes + 8,
          "G1 continuation preserves record stride");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(3, 4)) == NULL,
          "G1 continuation bound fails closed");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(11, 0)) == NULL,
          "zero-sized DB11 fails closed");
    CHECK(dm2_v1_record_pool_address(&set, mk_handle(1, 0)) == NULL,
          "absent DB1 pool fails closed");
    CHECK(dm2_v1_record_pool_address(&set, DM2_V1_RECORD_HANDLE_NULL) == NULL,
          "OBJECT_NULL fails closed");
    CHECK(dm2_v1_record_pool_address(&set, DM2_V1_RECORD_HANDLE_END) == NULL,
          "OBJECT_END_MARKER fails closed");

    /* Link walk: chain 0 -> 1 -> END within DB0. */
    wr16(set.pools[0].bytes + 0, mk_handle(0, 1));
    wr16(set.pools[0].bytes + 4, DM2_V1_RECORD_HANDLE_END);
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(0, 0), &next) &&
              next == mk_handle(0, 1),
          "next link follows first word");
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(0, 1), &next) &&
              next == DM2_V1_RECORD_HANDLE_END,
          "chain terminates at OBJECT_END_MARKER");
    CHECK(!dm2_v1_record_pool_next_link(&set, mk_handle(11, 0), &next),
          "next link on zero-sized pool fails closed");

    /* ── APPEND/CUT list paths (c_record.cpp:60-170) ────────────── */
    head_a = DM2_V1_RECORD_HANDLE_END;
    head_b = DM2_V1_RECORD_HANDLE_END;
    CHECK(dm2_v1_record_pool_append_to_list(&set, &head_a, mk_handle(3, 0)),
          "append into empty list");
    CHECK(head_a == mk_handle(3, 0), "list head updated");
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(3, 0), &next) &&
              next == DM2_V1_RECORD_HANDLE_END,
          "appended record terminates");
    CHECK(dm2_v1_record_pool_append_to_list(&set, &head_a, mk_handle(3, 1)),
          "append second record");
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(3, 0), &next) &&
              next == mk_handle(3, 1),
          "append chains at list end");
    CHECK(!dm2_v1_record_pool_append_to_list(&set, &head_a,
                                             DM2_V1_RECORD_HANDLE_NULL),
          "append of OBJECT_NULL rejected");
    CHECK(!dm2_v1_record_pool_append_to_list(&set, &head_a, mk_handle(0, 9)),
          "append of unresolvable record rejected");

    /* Relocate record (3,0) from list A to list B (c_moverec.cpp). */
    CHECK(dm2_v1_record_pool_relocate(&set, &head_a, &head_b, mk_handle(3, 0)),
          "relocate across lists");
    CHECK(head_a == mk_handle(3, 1), "source list spliced");
    CHECK(head_b == mk_handle(3, 0), "destination list owns record");
    CHECK(dm2_v1_record_pool_next_link(&set, mk_handle(3, 0), &next) &&
              next == DM2_V1_RECORD_HANDLE_END,
          "relocated record terminates destination");

    /* Cut the remaining record from list A. */
    CHECK(dm2_v1_record_pool_cut_from_list(&set, &head_a, mk_handle(3, 1)),
          "cut last record");
    CHECK(head_a == DM2_V1_RECORD_HANDLE_END, "list head becomes END");
    CHECK(!dm2_v1_record_pool_cut_from_list(&set, &head_a, mk_handle(3, 1)),
          "cut of absent record fails");

    dm2_v1_record_pool_set_free(&set);
    CHECK(set.valid == 0 && set.pools[0].bytes == NULL,
          "free releases owned pools");

    /* init_from_world without a verified world fails closed. */
    memset(&set, 0, sizeof(set));
    CHECK(!dm2_v1_record_pool_set_init_from_world(&set, NULL),
          "init from NULL world fails closed");

    test_recycle_scan_traversal();
    test_private_db0_recycler_candidate();
    test_sksave_owner_replacement_is_atomic();

    CHECK(strstr(dm2_v1_record_pool_source_evidence(), "c_record.cpp") != NULL,
          "source evidence cites c_record.cpp");

    if (g_failures != 0) {
        fprintf(stderr, "dm2_v1_record_pool_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_record_pool_pc34_compat: all checks passed\n");
    return 0;
}
