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
    track[0x1c20u + 31u] = 0x91u;
    track[0x1c20u + 32u] = 0x23u;

    /* The descriptor-bearing window has source-shape markers around the
     * descriptor bytes.  0x60 is the HuC6280 RTS marker documented by the
     * production semantic binder. */
    track[DESCRIPTOR_OFFSET - 1u] = 0x60u;
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
                          THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND);
    check_int("entry 5 semantic role",
              binding.role,
              THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE);
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

int main(void) {
    printf("=== Theron V1 Track 02 descriptor-entry semantic probe ===\n");

    probe_entry_roles();
    probe_semantic_seed_binding();
    probe_negative_fixtures();

    if (g_failures) {
        printf("FAIL: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: Theron Track 02 descriptor-entry semantics are registered and current");
    return 0;
}
