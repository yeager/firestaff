/*
 * firestaff_theron_v1_track02_descriptor_entry_semantic_probe.c
 *
 * Theron's Quest V1 -- current Track 02 descriptor-entry semantic gate.
 *
 * Scope:
 *   Keeps the descriptor-entry semantic path executable after the
 *   descriptor API was promoted from the older per-entry classifier to the
 *   current two-step model:
 *
 *     1. theron_v1_track02_bind_descriptor_entry_roles()
 *        classifies the nine 0x0400-byte descriptor windows as zero-fill,
 *        pre-descriptor data, post-descriptor data, or the descriptor-table
 *        window.
 *
 *     2. theron_v1_track02_bind_semantic_descriptor()
 *        binds the currently claimed gameplay semantic: entry 0 is the
 *        dungeon-seed table.  Other entries are still honest no-claim unless
 *        the source model later promotes them.
 *
 * Non-claim boundary:
 *   This probe does not decode real Track 02 dungeon records, maps, objects,
 *   text, palettes, graphics, audio banks, or runtime level handoff.  It
 *   proves the startup descriptor semantic contract remains callable,
 *   bounded, and shape-gated.
 */

#include "theron_v1_track02.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DESCRIPTOR_OFFSET 0x1584u
#define TRACK_FIXTURE_BYTES 0x4000u
#define DESCRIPTOR_WINDOW_SIZE 0x0400u

static int g_failures = 0;

static const uint8_t g_descriptor_bytes[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES * 2u] = {
    0x20, 0x00,
    0x20, 0x04,
    0x20, 0x08,
    0x20, 0x0c,
    0x20, 0x10,
    0x20, 0x14,
    0x20, 0x18,
    0x20, 0x1c,
    0x20, 0x20
};

static const uint32_t g_seed_values[THERON_TRACK02_DUNGEON_COUNT] = {
    313u, 414u, 527u, 632u, 749u, 856u, 967u
};

static void check_int(const char *label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        ++g_failures;
    }
}

static void check_size(const char *label, size_t got, size_t want) {
    if (got != want) {
        printf("FAIL %s: got %zu want %zu\n", label, got, want);
        ++g_failures;
    }
}

static void check_u32(const char *label, uint32_t got, uint32_t want) {
    if (got != want) {
        printf("FAIL %s: got 0x%08x want 0x%08x\n",
               label, (unsigned)got, (unsigned)want);
        ++g_failures;
    }
}

static void check_role(const char *label,
                       Theron_Track02DescriptorEntryRole got,
                       Theron_Track02DescriptorEntryRole want) {
    if (got != want) {
        printf("FAIL %s: got %s want %s\n",
               label,
               theron_v1_track02_descriptor_entry_role_name(got),
               theron_v1_track02_descriptor_entry_role_name(want));
        ++g_failures;
    }
}

static void check_semantic_status(const char *label,
                                  Theron_Track02SemanticBindingStatus got,
                                  Theron_Track02SemanticBindingStatus want) {
    if (got != want) {
        printf("FAIL %s: got %s want %s\n",
               label,
               theron_v1_track02_semantic_binding_status_name(got),
               theron_v1_track02_semantic_binding_status_name(want));
        ++g_failures;
    }
}

static void put_le32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)(value & 0xffu);
    p[1] = (uint8_t)((value >> 8) & 0xffu);
    p[2] = (uint8_t)((value >> 16) & 0xffu);
    p[3] = (uint8_t)((value >> 24) & 0xffu);
}

static void put_le16(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)(value & 0xffu);
    p[1] = (uint8_t)((value >> 8) & 0xffu);
}

static void make_fixture(uint8_t track[TRACK_FIXTURE_BYTES]) {
    size_t i;

    memset(track, 0, TRACK_FIXTURE_BYTES);
    memcpy(track + DESCRIPTOR_OFFSET, g_descriptor_bytes, sizeof(g_descriptor_bytes));

    /* Entry 0 is the currently claimed semantic window. */
    for (i = 0; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        put_le32(track + 0x0020u + (i * 4u), g_seed_values[i]);
    }

    /* Entries 2 and 7 carry non-zero payload bytes on either side of the
     * descriptor window, so the role binder proves ordering as well as
     * zero-fill handling. */
    track[0x0820u + 17u] = 0x4au;
    put_le16(track + 0x1820u, 2u);
    track[0x1820u + 2u] = 0x11u;
    track[0x1820u + 3u] = 0x01u;
    track[0x1820u + 4u] = 4u;
    track[0x1820u + 5u] = 5u;
    track[0x1820u + 6u] = 0u;
    track[0x1820u + 7u] = 0x80u;
    put_le16(track + 0x1820u + 8u, 0x1234u);
    track[0x1820u + 10u] = 0x12u;
    track[0x1820u + 11u] = 0x02u;
    track[0x1820u + 12u] = 7u;
    track[0x1820u + 13u] = 9u;
    track[0x1820u + 14u] = 1u;
    track[0x1820u + 15u] = 0x40u;
    put_le16(track + 0x1820u + 16u, 0x5678u);
    track[0x1c20u + 31u] = 0x91u;
    track[0x1c20u + 32u] = 0x23u;

    /* Entry 7 is a complete big-endian dungeon record.  Together with
     * entry 6's compact object rows it drives the production route builder. */
    track[0x1c20u + 1u] = 3u;
    track[0x1c20u + 3u] = 3u;
    track[0x1c20u + 4u] = 0x01u;
    track[0x1c20u + 5u] = 0x08u;
    track[0x1c20u + 6u] = 0xe9u;
    track[0x1c20u + 7u] = 0x38u;
    track[0x1c20u + 12u] = THERON_SQUARE_WALL;
    track[0x1c20u + 13u] = THERON_SQUARE_WALL;
    track[0x1c20u + 14u] = THERON_SQUARE_WALL;
    track[0x1c20u + 15u] = THERON_SQUARE_WALL;
    track[0x1c20u + 16u] = THERON_SQUARE_FLOOR;
    track[0x1c20u + 17u] = THERON_SQUARE_EXIT;
    track[0x1c20u + 18u] = THERON_SQUARE_WALL;
    track[0x1c20u + 19u] = THERON_SQUARE_WALL;
    track[0x1c20u + 20u] = THERON_SQUARE_WALL;

    /* The descriptor-bearing window has source-shape markers around the
     * descriptor bytes.  0x60 is the HuC6280 RTS marker documented by the
     * production semantic binder. */
    track[DESCRIPTOR_OFFSET - 1u] = 0x60u;
}

static void make_complete_bitmap_atlas(Theron_Track02StartupBitmapAtlas *atlas) {
    unsigned int i;
    memset(atlas, 0, sizeof(*atlas));
    atlas->route_count = THERON_TRACK02_STARTUP_BITMAP_ATLAS_ROUTE_MAX;
    atlas->route_mask = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    atlas->checksum = 0x53424d50u;
    for (i = 0u; i < THERON_TRACK02_STARTUP_BITMAP_ATLAS_ROUTE_MAX; ++i) {
        atlas->routes[i].route_bit = 1u << i;
        atlas->routes[i].width = 96u;
        atlas->routes[i].height = 8u;
    }
}

static int decode_fixture_table(const uint8_t *track,
                                Theron_Track02DescriptorTable *out_table) {
    Theron_Track02TableDecodeStatus status =
        theron_v1_track02_decode_descriptor_table(
            track + DESCRIPTOR_OFFSET,
            sizeof(g_descriptor_bytes),
            DESCRIPTOR_WINDOW_SIZE,
            out_table);
    check_int("decode fixture descriptor table",
              status,
              THERON_TRACK02_TABLE_DECODE_OK);
    return status == THERON_TRACK02_TABLE_DECODE_OK;
}

static void probe_entry_roles(void) {
    uint8_t track[TRACK_FIXTURE_BYTES];
    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorEntrySemanticBinding entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    Theron_Track02TableDecodeStatus status;
    int descriptor_index;

    make_fixture(track);
    if (!decode_fixture_table(track, &table)) {
        return;
    }

    status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        &table,
        entries);
    check_int("bind descriptor entry roles",
              status,
              THERON_TRACK02_TABLE_DECODE_OK);
    if (status != THERON_TRACK02_TABLE_DECODE_OK) {
        return;
    }

    descriptor_index = theron_v1_track02_find_descriptor_window_entry_index(
        entries,
        THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES);
    check_int("descriptor window index", descriptor_index, 5);

    check_role("entry 0 role",
               entries[0].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA);
    check_role("entry 1 role",
               entries[1].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL);
    check_role("entry 2 role",
               entries[2].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA);
    check_role("entry 5 role",
               entries[5].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE);
    check_role("entry 6 role",
               entries[6].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA);
    check_role("entry 7 role",
               entries[7].role,
               THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA);

    check_size("entry 0 relative offset", entries[0].relative_offset, 0x0020u);
    check_size("entry 0 absolute offset", entries[0].absolute_offset, 0x0020u);
    check_size("entry 0 byte count", entries[0].byte_count, DESCRIPTOR_WINDOW_SIZE);
    check_int("entry 5 descriptor flag", entries[5].is_descriptor_window, 1);
    check_int("entry 5 rts before descriptor",
              entries[5].byte_before_descriptor_is_rts,
              1);
    check_int("entry 5 all zero after descriptor",
              entries[5].all_zero_after_descriptor,
              1);
    check_size("entry 5 first nonzero after descriptor",
               entries[5].first_nonzero_after_descriptor,
               0u);
}

static void probe_semantic_seed_binding(void) {
    uint8_t track[TRACK_FIXTURE_BYTES];
    Theron_Track02SemanticBinding binding;
    Theron_Track02SemanticBindingStatus status;
    size_t i;

    make_fixture(track);
    status = theron_v1_track02_bind_semantic_descriptor(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        0u,
        &binding);
    check_semantic_status("entry 0 semantic status",
                          status,
                          THERON_TRACK02_SEMANTIC_BINDING_OK);
    check_int("entry 0 semantic role",
              binding.role,
              THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE);
    check_int("entry 0 seed shape",
              binding.dungeon_seed_table.shape_ok,
              1);
    for (i = 0; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        char label[64];
        snprintf(label, sizeof(label), "entry 0 seed[%zu]", i);
        check_u32(label, binding.dungeon_seed_table.seeds[i], g_seed_values[i]);
    }

    memset(&binding, 0, sizeof(binding));
    status = theron_v1_track02_bind_semantic_descriptor(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        1u,
        &binding);
    check_semantic_status("entry 1 semantic status",
                          status,
                          THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND);
    check_int("entry 1 semantic role",
              binding.role,
              THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN);

    memset(&binding, 0, sizeof(binding));
    status = theron_v1_track02_bind_semantic_descriptor(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        5u,
        &binding);
    check_semantic_status("entry 5 semantic status",
                          status,
                          THERON_TRACK02_SEMANTIC_BINDING_OK);
    check_int("entry 5 semantic role",
              binding.role,
              THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE);

    memset(&binding, 0, sizeof(binding));
    status = theron_v1_track02_bind_semantic_descriptor(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        6u,
        &binding);
    check_semantic_status("entry 6 object semantic status",
                          status,
                          THERON_TRACK02_SEMANTIC_BINDING_OK);
    check_int("entry 6 semantic role",
              binding.role,
              THERON_TRACK02_SEMANTIC_OBJECT_TABLE);
    check_size("entry 6 object record count",
               binding.object_table.record_count,
               2u);
    check_size("entry 6 object declared count",
               binding.object_table.declared_record_count,
               2u);
    check_size("entry 6 object required bytes",
               binding.object_table.required_byte_count,
               18u);
    check_int("entry 6 object shape",
              binding.object_table.shape_ok,
              1);
    check_int("entry 6 object reject reason",
              binding.object_table.reject_reason,
              THERON_TRACK02_OBJECT_TABLE_REJECT_NONE);
    check_int("entry 6 object[0] x", binding.object_table.records[0].x, 4);
    check_int("entry 6 object[0] y", binding.object_table.records[0].y, 5);
    check_int("entry 6 object[1] level",
              binding.object_table.records[1].level_index,
              1);
    check_int("entry 6 object[1] kind",
              binding.object_table.records[1].kind,
              2);
    check_u32("entry 6 object level coverage", binding.object_table.level_mask,
              (1u << 0) | (1u << 1));
    check_size("entry 6 object level 0 count",
               binding.object_table.level_record_counts[0], 1u);
    check_size("entry 6 object level 1 count",
               binding.object_table.level_record_counts[1], 1u);
    check_int("entry 6 object level 0 hash present",
              binding.object_table.level_record_hashes[0] != 0u, 1);
    check_int("entry 6 object level 1 hash present",
              binding.object_table.level_record_hashes[1] != 0u, 1);
}

static void probe_negative_fixtures(void) {
    uint8_t track[TRACK_FIXTURE_BYTES];
    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorEntrySemanticBinding entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
    Theron_Track02TableDecodeStatus table_status;
    Theron_Track02SemanticBinding binding;
    Theron_Track02SemanticBindingStatus semantic_status;

    make_fixture(track);
    (void)decode_fixture_table(track, &table);

    table_status = theron_v1_track02_bind_descriptor_entry_roles(
        NULL,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        &table,
        entries);
    check_int("NULL data role binding",
              table_status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);

    table_status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        NULL,
        entries);
    check_int("NULL table role binding",
              table_status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);

    table_status = theron_v1_track02_bind_descriptor_entry_roles(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        &table,
        NULL);
    check_int("NULL out role binding",
              table_status,
              THERON_TRACK02_TABLE_DECODE_BAD_INPUT);

    semantic_status = theron_v1_track02_bind_semantic_descriptor(
        NULL,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        0u,
        &binding);
    check_semantic_status("NULL data semantic binding",
                          semantic_status,
                          THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT);

    semantic_status = theron_v1_track02_bind_semantic_descriptor(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES,
        &binding);
    check_semantic_status("out-of-range semantic binding",
                          semantic_status,
                          THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT);

    track[0x0020u] = 0u;
    track[0x0021u] = 0u;
    track[0x0022u] = 0u;
    track[0x0023u] = 0u;
    semantic_status = theron_v1_track02_bind_semantic_descriptor(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        0u,
        &binding);
    check_semantic_status("zero seed semantic binding",
                          semantic_status,
                          THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE);

    make_fixture(track);
    track[0x1820u + 4u] = 32u;
    semantic_status = theron_v1_track02_bind_semantic_descriptor(
        track,
        sizeof(track),
        DESCRIPTOR_OFFSET,
        6u,
        &binding);
    check_semantic_status("bad object x semantic binding",
                          semantic_status,
                          THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE);
    check_size("bad object x first bad row",
               binding.object_table.first_bad_record_index,
               0u);
    check_int("bad object x reject reason",
              binding.object_table.reject_reason,
              THERON_TRACK02_OBJECT_TABLE_REJECT_X_OUT_OF_RANGE);
}

static void probe_dungeon_route(void) {
    uint8_t track[TRACK_FIXTURE_BYTES];
    Theron_Track02StartupBitmapAtlas atlas;
    Theron_Track02DungeonRoute route;
    Theron_Track02DungeonRouteStatus status;

    make_fixture(track);
    make_complete_bitmap_atlas(&atlas);
    status = theron_v1_track02_build_dungeon_route(
        track, sizeof(track), DESCRIPTOR_OFFSET, 1, 0, &atlas, &route);
    check_int("dungeon route status", status, THERON_TRACK02_DUNGEON_ROUTE_OK);
    check_int("dungeon route valid", route.valid, 1);
    check_size("dungeon route level entry", route.level_entry_index, 7u);
    check_size("dungeon route object entry", route.object_entry_index, 6u);
    check_int("dungeon route level width", route.level.width, 3);
    check_size("dungeon route objects", route.objects.record_count, 2u);

    atlas.route_mask = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE;
    status = theron_v1_track02_build_dungeon_route(
        track, sizeof(track), DESCRIPTOR_OFFSET, 1, 0, &atlas, &route);
    check_int("dungeon route incomplete bitmap", status,
              THERON_TRACK02_DUNGEON_ROUTE_BITMAP_REJECTED);
}

static void probe_validated_level_transition(void) {
    uint8_t track[TRACK_FIXTURE_BYTES];
    Theron_Track02StartupBitmapAtlas atlas;
    Theron_Track02DungeonRoute source;
    Theron_Track02DungeonRoute target;
    Theron_Track02LevelTransitionReceipt receipt;
    Theron_V1_World world;
    Theron_Track02LevelTransitionStatus status;

    make_fixture(track);
    make_complete_bitmap_atlas(&atlas);
    check_int("transition source route",
              theron_v1_track02_build_dungeon_route(
                  track, sizeof(track), DESCRIPTOR_OFFSET, 1, 0,
                  &atlas, &source),
              THERON_TRACK02_DUNGEON_ROUTE_OK);
    check_int("transition target route",
              theron_v1_track02_build_dungeon_route(
                  track, sizeof(track), DESCRIPTOR_OFFSET, 1, 1,
                  &atlas, &target),
              THERON_TRACK02_DUNGEON_ROUTE_OK);

    theron_v1_world_init(&world);
    world.current_dungeon = 1;
    world.current_level = 0;
    world.levels[0][0] = source.level;
    world.level_loaded[0][0] = 1;
    world.transition_pending = 1;
    world.transition_type = THERON_TRANSITION_STAIRS;
    world.transition_target_level = 1;
    status = theron_v1_track02_apply_level_transition(
        &world, &source, &target, &receipt);
    check_int("validated transition status",
              status, THERON_TRACK02_LEVEL_TRANSITION_OK);
    check_int("validated transition applied", receipt.applied, 1);
    check_int("validated transition level", world.current_level, 1);
    check_int("validated transition loaded", world.level_loaded[0][1], 1);
    check_int("validated transition clears queue", world.transition_pending, 0);
    check_size("validated transition objects",
               receipt.target_object_record_count, 2u);

    world.transition_pending = 1;
    world.transition_type = THERON_TRANSITION_STAIRS;
    world.transition_target_level = 2;
    status = theron_v1_track02_apply_level_transition(
        &world, &target, &target, &receipt);
    check_int("mismatched transition status",
              status, THERON_TRACK02_LEVEL_TRANSITION_TARGET_MISMATCH);
    check_int("mismatched transition keeps queue", world.transition_pending, 1);
    check_int("mismatched transition keeps level", world.current_level, 1);
}

static void probe_catalog_level_transition(void) {
    uint8_t track[TRACK_FIXTURE_BYTES];
    Theron_Track02StartupBitmapAtlas atlas;
    Theron_Track02DungeonRoute routes[2];
    Theron_Track02LevelTransitionReceipt transition_receipt;
    Theron_Track02RouteCatalogReceipt source_receipt;
    Theron_Track02RouteCatalogReceipt target_receipt;
    Theron_V1_World world;
    const Theron_Track02DungeonRoute *selected = NULL;
    Theron_Track02RouteCatalogStatus catalog_status;
    Theron_Track02LevelTransitionStatus transition_status;

    make_fixture(track);
    make_complete_bitmap_atlas(&atlas);
    check_int("catalog route 0",
              theron_v1_track02_build_dungeon_route(
                  track, sizeof(track), DESCRIPTOR_OFFSET, 1, 0,
                  &atlas, &routes[0]),
              THERON_TRACK02_DUNGEON_ROUTE_OK);
    check_int("catalog route 1",
              theron_v1_track02_build_dungeon_route(
                  track, sizeof(track), DESCRIPTOR_OFFSET, 1, 1,
                  &atlas, &routes[1]),
              THERON_TRACK02_DUNGEON_ROUTE_OK);

    catalog_status = theron_v1_track02_select_dungeon_route(
        routes, 2u, 1, 1, &selected, &target_receipt);
    check_int("catalog selects exact target", catalog_status,
              THERON_TRACK02_ROUTE_CATALOG_OK);
    check_int("catalog selected target pointer", selected == &routes[1], 1);
    check_int("catalog complete mask", (int)target_receipt.level_mask, 3);

    theron_v1_world_init(&world);
    world.current_dungeon = 1;
    world.current_level = 0;
    world.levels[0][0] = routes[0].level;
    world.level_loaded[0][0] = 1;
    world.transition_pending = 1;
    world.transition_type = THERON_TRANSITION_STAIRS;
    world.transition_target_level = 1;
    transition_status = theron_v1_track02_apply_level_transition_from_catalog(
        &world, routes, 2u, &transition_receipt,
        &source_receipt, &target_receipt);
    check_int("catalog transition status", transition_status,
              THERON_TRACK02_LEVEL_TRANSITION_OK);
    check_int("catalog transition applied", transition_receipt.applied, 1);
    check_int("catalog transition level", world.current_level, 1);
    check_int("catalog source selected", source_receipt.selected, 1);
    check_int("catalog target selected", target_receipt.selected, 1);

    world.transition_pending = 0;
    transition_status = theron_v1_track02_apply_level_transition_from_catalog(
        &world, routes, 2u, &transition_receipt,
        &source_receipt, &target_receipt);
    check_int("catalog no pending status", transition_status,
              THERON_TRACK02_LEVEL_TRANSITION_NOT_PENDING);
    check_int("catalog no pending leaves level", world.current_level, 1);

    routes[1].level_index = 2;
    world.current_level = 0;
    world.transition_pending = 1;
    world.transition_type = THERON_TRANSITION_STAIRS;
    world.transition_target_level = 1;
    transition_status = theron_v1_track02_apply_level_transition_from_catalog(
        &world, routes, 2u, &transition_receipt,
        &source_receipt, &target_receipt);
    check_int("catalog gap rejects target", transition_status,
              THERON_TRACK02_LEVEL_TRANSITION_SOURCE_REJECTED);
    check_int("catalog gap leaves queue", world.transition_pending, 1);
    check_int("catalog gap leaves level", world.current_level, 0);
    check_int("catalog gap receipt", source_receipt.status,
              THERON_TRACK02_ROUTE_CATALOG_NONCONTIGUOUS);

    routes[1] = routes[0];
    catalog_status = theron_v1_track02_select_dungeon_route(
        routes, 2u, 1, 0, &selected, &target_receipt);
    check_int("catalog duplicate rejects", catalog_status,
              THERON_TRACK02_ROUTE_CATALOG_DUPLICATE_LEVEL);
    check_int("catalog duplicate no selection", selected == NULL, 1);

    routes[1].valid = 0;
    catalog_status = theron_v1_track02_select_dungeon_route(
        routes, 2u, 1, 0, &selected, &target_receipt);
    check_int("catalog rejected route", catalog_status,
              THERON_TRACK02_ROUTE_CATALOG_ROUTE_REJECTED);
    check_int("catalog rejected no selection", selected == NULL, 1);
}

static void probe_track02_palette_route(void) {
    uint8_t palette_bytes[THERON_TRACK02_4BPP_PALETTE_BYTES] = {0};
    Theron_Track02Palette4Bpp palette;
    Theron_Track02StartupBitmapAtlasRoute indexed_route;
    Theron_Track02StartupBitmapRgbaRoute rgba_route;
    Theron_Track02SignalStatus status;

    /* HuC6270 9-bit RGB: red=7, green=3, blue=1 at index 1. */
    palette_bytes[2u] = 0x5fu;
    palette_bytes[3u] = 0x00u;
    palette_bytes[4u] = 0xc0u;
    palette_bytes[5u] = 0x01u;
    status = theron_v1_track02_decode_4bpp_palette(
        palette_bytes, sizeof(palette_bytes), &palette);
    check_int("Track02 palette status", status, THERON_TRACK02_SIGNAL_OK);
    check_int("Track02 palette valid", palette.valid, 1);
    check_size("Track02 palette nonblack entries",
               palette.nonblack_entry_count, 2u);
    check_int("Track02 palette index1 red", palette.entries[1].red, 255);
    check_int("Track02 palette index1 green", palette.entries[1].green, 109);
    check_int("Track02 palette index1 blue", palette.entries[1].blue, 36);

    memset(&indexed_route, 0, sizeof(indexed_route));
    indexed_route.route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE;
    indexed_route.tile_count = 1u;
    indexed_route.width = 8u;
    indexed_route.height = 8u;
    indexed_route.pixels[0] = 1u;
    indexed_route.pixels[1] = 2u;
    status = theron_v1_track02_colorize_startup_bitmap_route(
        &indexed_route, &palette, &rgba_route);
    check_int("Track02 RGBA route status", status, THERON_TRACK02_SIGNAL_OK);
    check_int("Track02 RGBA route valid", rgba_route.valid, 1);
    check_int("Track02 RGBA route red", rgba_route.rgba[0], 255);
    check_int("Track02 RGBA route green", rgba_route.rgba[1], 109);
    check_int("Track02 RGBA route blue", rgba_route.rgba[2], 36);
    check_int("Track02 RGBA route alpha", rgba_route.rgba[3], 255);
    check_int("Track02 RGBA index2 blue", rgba_route.rgba[7], 255);

    palette_bytes[3u] = 0x80u;
    status = theron_v1_track02_decode_4bpp_palette(
        palette_bytes, sizeof(palette_bytes), &palette);
    check_int("Track02 palette reserved bits reject",
              status, THERON_TRACK02_SIGNAL_NOT_FOUND);
    check_int("Track02 palette rejected valid", palette.valid, 0);
}

static void probe_track02_palette_window_evidence(void) {
    uint8_t track[64] = {0};
    Theron_Track02PaletteWindowEvidence evidence;
    Theron_Track02SignalStatus status;

    /* This is an explicitly supplied fixture offset, not discovery. */
    track[8u + 2u] = 0x5fu;
    track[8u + 3u] = 0x00u;
    status = theron_v1_track02_inspect_4bpp_palette_window(
        track, sizeof(track), THERON_TRACK02_MD5_US_ISO, 8u, &evidence);
    check_int("Track02 palette evidence status", status,
              THERON_TRACK02_SIGNAL_OK);
    check_int("Track02 palette evidence format", evidence.format_valid, 1);
    check_int("Track02 palette evidence user data", evidence.raw_offset_is_user_data, 1);
    check_size("Track02 palette evidence user offset", evidence.user_data_offset, 8u);
    check_int("Track02 palette evidence semantic unbound",
              evidence.semantic_binding_verified, 0);
    check_int("Track02 palette evidence promotion blocked",
              theron_v1_track02_palette_window_evidence_can_promote(&evidence),
              0);

    status = theron_v1_track02_inspect_4bpp_palette_window(
        track, sizeof(track), THERON_TRACK02_MD5_US_ISO, 32u, &evidence);
    check_int("Track02 palette evidence zero reject", status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);

    status = theron_v1_track02_inspect_4bpp_palette_window(
        track, sizeof(track), "not-a-track02-md5", 8u, &evidence);
    check_int("Track02 palette evidence unknown hash reject", status,
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
}

int main(void) {
    printf("=== Theron V1 Track 02 descriptor-entry semantic probe ===\n");

    probe_entry_roles();
    probe_semantic_seed_binding();
    probe_negative_fixtures();
    probe_dungeon_route();
    probe_validated_level_transition();
    probe_catalog_level_transition();
    probe_track02_palette_route();
    probe_track02_palette_window_evidence();

    if (g_failures) {
        printf("FAIL: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: Theron Track 02 descriptor-entry semantics are registered and current");
    return 0;
}
