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
    check_semantic_status("entry 6 semantic status",
                          status,
                          THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND);
    check_int("entry 6 semantic role",
              binding.role,
              THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN);
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
    check_int("dungeon route remains blocked", status,
              THERON_TRACK02_DUNGEON_ROUTE_OBJECT_REJECTED);
    check_int("dungeon route is not valid", route.valid, 0);

    atlas.route_mask = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE;
    status = theron_v1_track02_build_dungeon_route(
        track, sizeof(track), DESCRIPTOR_OFFSET, 1, 0, &atlas, &route);
    check_int("dungeon route incomplete bitmap", status,
              THERON_TRACK02_DUNGEON_ROUTE_BITMAP_REJECTED);
}

static void probe_validated_level_transition(void) {
    uint8_t track[TRACK_FIXTURE_BYTES];
    Theron_Track02StartupBitmapAtlas atlas;
    Theron_Track02DungeonRoute route;

    make_fixture(track);
    make_complete_bitmap_atlas(&atlas);
    check_int("transition source route remains blocked",
              theron_v1_track02_build_dungeon_route(
                  track, sizeof(track), DESCRIPTOR_OFFSET, 1, 0,
                  &atlas, &route),
              THERON_TRACK02_DUNGEON_ROUTE_OBJECT_REJECTED);
}

static void probe_catalog_level_transition(void) {
    uint8_t track[TRACK_FIXTURE_BYTES];
    Theron_Track02StartupBitmapAtlas atlas;
    Theron_Track02DungeonRoute route;

    make_fixture(track);
    make_complete_bitmap_atlas(&atlas);
    check_int("catalog route remains blocked",
              theron_v1_track02_build_dungeon_route(
                  track, sizeof(track), DESCRIPTOR_OFFSET, 1, 1,
                  &atlas, &route),
              THERON_TRACK02_DUNGEON_ROUTE_OBJECT_REJECTED);
}

static void probe_track02_palette_route(void) {
    uint8_t palette_bytes[THERON_TRACK02_4BPP_PALETTE_BYTES] = {0};
    Theron_Track02Palette4Bpp palette;
    Theron_Track02StartupBitmapAtlasRoute indexed_route;
    Theron_Track02StartupBitmapRgbaRoute rgba_route;
    Theron_Track02SignalStatus status;

    /* HuC6260 CTW/CTR: blue=7, red=3, green=1 at index 1. */
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
    check_int("Track02 palette index1 red", palette.entries[1].red, 109);
    check_int("Track02 palette index1 green", palette.entries[1].green, 36);
    check_int("Track02 palette index1 blue", palette.entries[1].blue, 255);

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
    check_int("Track02 RGBA route red", rgba_route.rgba[0], 109);
    check_int("Track02 RGBA route green", rgba_route.rgba[1], 36);
    check_int("Track02 RGBA route blue", rgba_route.rgba[2], 255);
    check_int("Track02 RGBA route alpha", rgba_route.rgba[3], 255);
    check_int("Track02 RGBA index2 green", rgba_route.rgba[5], 255);

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
