/*
 * firestaff_theron_v1_track02_level_handoff_probe.c
 *
 * Theron's Quest V1 -- bounded Track 02 descriptor-window to level-loader
 * handoff gate.
 *
 * Scope:
 *   This probe proves a narrow API bridge: a decoded Track 02 descriptor
 *   table window can be selected, classified as DATA, and handed to the
 *   existing theron_v1_level_load() parser.  Synthetic fixtures and the
 *   hash-verified retail US ISO startup envelope provide the only positive
 *   level loads; all other real-media descriptor windows remain no-claim
 *   until the dungeon record format is known.
 */

#include "asset_status_m12.h"
#include "theron_v1_startup_runtime_entry.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

#define DOCUMENTED_STRIDE 0x0400u
#define DESCRIPTOR_BYTE_COUNT (THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES * 2u)

static int g_fail = 0;
static int g_skip = 0;

static const uint8_t g_canonical_descriptor[DESCRIPTOR_BYTE_COUNT] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static void check_int(const char *label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        ++g_fail;
    }
}

static void check_size(const char *label, size_t got, size_t want) {
    if (got != want) {
        printf("FAIL %s: got %zu want %zu\n", label, got, want);
        ++g_fail;
    }
}

static void check_u16(const char *label, uint16_t got, uint16_t want) {
    if (got != want) {
        printf("FAIL %s: got 0x%04x want 0x%04x\n",
               label, (unsigned)got, (unsigned)want);
        ++g_fail;
    }
}

static void check_u32(const char *label, uint32_t got, uint32_t want) {
    if (got != want) {
        printf("FAIL %s: got 0x%08x want 0x%08x\n",
               label, (unsigned)got, (unsigned)want);
        ++g_fail;
    }
}

static void write_be16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)((v >> 8) & 0xffu);
    p[1] = (uint8_t)(v & 0xffu);
}

static void write_be32(uint8_t *p, uint32_t v) {
    write_be16(p, (uint16_t)((v >> 16) & 0xffffu));
    write_be16(p + 2, (uint16_t)(v & 0xffffu));
}

static void write_le32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
    p[2] = (uint8_t)((v >> 16) & 0xffu);
    p[3] = (uint8_t)((v >> 24) & 0xffu);
}

static int file_exists(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && st.st_size > 0;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *fp;
    long size;
    uint8_t *data;

    if (!path || !out_data || !out_size) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(fp);
        return 0;
    }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static void default_data_path(const char *relative_name, char out_path[512]) {
    const char *home = getenv("HOME");
    if (!home || !home[0]) home = ".";
    snprintf(out_path, 512, "%s%s.firestaff%sdata%s%s",
             home, PATH_SEP, PATH_SEP, PATH_SEP, relative_name);
}

static void probe_synthetic_positive_handoff(void) {
    uint8_t track[0x3000u];
    uint8_t *level_window;
    Theron_V1_Level level;
    Theron_Track02LevelHandoff handoff;
    Theron_Track02LevelHandoffStatus status;
    static const uint8_t grid[12] = {
        THERON_SQUARE_WALL,  THERON_SQUARE_WALL,  THERON_SQUARE_WALL, THERON_SQUARE_WALL,
        THERON_SQUARE_WALL,  THERON_SQUARE_FLOOR, THERON_SQUARE_FLOOR, THERON_SQUARE_WALL,
        THERON_SQUARE_WALL,  THERON_SQUARE_EXIT,  THERON_SQUARE_FLOOR, THERON_SQUARE_WALL
    };

    memset(track, 0, sizeof(track));
    memcpy(track + 0x1584u, g_canonical_descriptor, sizeof(g_canonical_descriptor));

    level_window = track + 0x0420u;
    write_be16(level_window + 0, 4u);
    write_be16(level_window + 2, 3u);
    write_be32(level_window + 4, 0x12345678u);
    write_be16(level_window + 8, 2u);
    level_window[10] = 0u;
    level_window[11] = 0u;
    memcpy(level_window + 12, grid, sizeof(grid));

    status = theron_v1_track02_load_descriptor_window_level(
        track,
        sizeof(track),
        0x1584u,
        1u,
        THERON_DUNGEON_1_AKUTUBA,
        2,
        &level,
        &handoff);

    printf("synthetic handoff: status=%s map=%d offset=0x%zx size=%zu header=%ux%u seed=0x%08x\n",
           theron_v1_track02_level_handoff_status_name(status),
           handoff.map_status,
           handoff.absolute_offset,
           handoff.byte_count,
           (unsigned)handoff.header_width,
           (unsigned)handoff.header_height,
           (unsigned)handoff.header_seed);

    check_int("synthetic handoff status", status, THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_int("synthetic loaded", handoff.loaded, 1);
    check_size("synthetic entry index", handoff.entry_index, 1u);
    check_size("synthetic absolute offset", handoff.absolute_offset, 0x0420u);
    check_size("synthetic byte count", handoff.byte_count, DOCUMENTED_STRIDE);
    check_u16("synthetic header width", handoff.header_width, 4u);
    check_u16("synthetic header height", handoff.header_height, 3u);
    check_u32("synthetic header seed", handoff.header_seed, 0x12345678u);
    check_u16("synthetic header level", handoff.header_level_index, 2u);
    check_int("synthetic map status", handoff.map_status, THERON_MAP_OK);
    check_int("synthetic level width", level.width, 4);
    check_int("synthetic level height", level.height, 3);
    check_int("synthetic start x", level.start_x, 1);
    check_int("synthetic start y", level.start_y, 1);
    check_int("synthetic floor square",
              level.squares[1][1],
              THERON_SQUARE_FLOOR);
    check_int("synthetic exit square",
              level.squares[2][1],
              THERON_SQUARE_EXIT);
}

static void probe_synthetic_initial_candidate_handoff(void) {
    enum {
        descriptor_offset = 0xc000u,
        base_offset = descriptor_offset - 0x1584u,
        candidate_offset = base_offset - 0x92ceu,
        candidate_width = 32u,
        candidate_height = 27u,
        candidate_size = 12u + (candidate_width * candidate_height)
    };
    uint8_t track[0xd000u];
    uint8_t *candidate;
    Theron_V1_Level level;
    Theron_Track02LevelHandoff handoff;
    Theron_Track02LevelCandidateCatalog catalog;
    Theron_Track02InitialCandidateBinding binding;
    Theron_Track02LevelHandoffStatus status;
    size_t expected_candidate_offset = 0u;

    memset(track, 0, sizeof(track));
    memcpy(track + descriptor_offset, g_canonical_descriptor, sizeof(g_canonical_descriptor));
    candidate = track + candidate_offset;
    write_be16(candidate + 0, candidate_width);
    write_be16(candidate + 2, candidate_height);
    write_be32(candidate + 4, 0x0108e938u);
    write_be16(candidate + 8, 0x0026u);
    candidate[10] = 0u;
    candidate[11] = 0u;
    memset(candidate + 12, THERON_SQUARE_WALL, candidate_width * candidate_height);
    candidate[12 + 1u * candidate_width + 1u] = THERON_SQUARE_FLOOR;
    candidate[12 + 1u * candidate_width + 2u] = THERON_SQUARE_FLOOR;
    candidate[12 + 2u * candidate_width + 2u] = THERON_SQUARE_EXIT;

    status = theron_v1_track02_scan_level_candidates(
        track,
        sizeof(track),
        &catalog);
    check_int("synthetic initial candidate scan status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("synthetic initial candidate scan count",
               catalog.candidate_count,
               1u);
    check_size("synthetic initial candidate scan offset",
               catalog.candidates[0].absolute_offset,
               candidate_offset);
    check_int("synthetic initial candidate bind anchor",
              theron_v1_track02_bind_level_candidate_anchor(
                  descriptor_offset,
                  &catalog),
              1);
    check_size("synthetic initial candidate descriptor delta",
               catalog.candidates[0].descriptor_delta,
               0xa852u);
    check_int("synthetic initial candidate anchor match",
              catalog.candidates[0].matches_initial_anchor,
              1);
    check_int("synthetic initial candidate expected offset ok",
              theron_v1_track02_initial_candidate_expected_offset(
                  descriptor_offset,
                  &expected_candidate_offset),
              1);
    check_size("synthetic initial candidate expected offset",
               expected_candidate_offset,
               candidate_offset);
    status = theron_v1_track02_bind_initial_level_candidate(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_ISO,
        descriptor_offset,
        &binding);
    check_int("synthetic initial candidate binding status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("synthetic initial candidate binding count",
               binding.candidate_count,
               1u);
    check_size("synthetic initial candidate binding index",
               binding.candidate_index,
               0u);
    check_size("synthetic initial candidate binding offset",
               binding.candidate.absolute_offset,
               candidate_offset);
    check_size("synthetic initial candidate binding expected offset",
               binding.expected_offset,
               candidate_offset);
    check_int("synthetic initial candidate binding anchor match",
              binding.matches_initial_anchor,
              1);

    status = theron_v1_track02_load_initial_level_candidate(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_ISO,
        descriptor_offset,
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);

    printf("synthetic initial candidate: status=%s map=%d offset=0x%zx size=%zu header=%ux%u seed=0x%08x\n",
           theron_v1_track02_level_handoff_status_name(status),
           handoff.map_status,
           handoff.absolute_offset,
           handoff.byte_count,
           (unsigned)handoff.header_width,
           (unsigned)handoff.header_height,
           (unsigned)handoff.header_seed);

    check_int("synthetic initial candidate status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("synthetic initial candidate offset",
               handoff.absolute_offset,
               candidate_offset);
    check_size("synthetic initial candidate size",
               handoff.byte_count,
               candidate_size);
    check_u16("synthetic initial candidate width",
              handoff.header_width,
              candidate_width);
    check_u16("synthetic initial candidate height",
              handoff.header_height,
              candidate_height);
    check_u32("synthetic initial candidate seed",
              handoff.header_seed,
              0x0108e938u);
    check_u16("synthetic initial candidate level index",
              handoff.header_level_index,
              0x0026u);
    check_int("synthetic initial candidate handoff binding status",
              handoff.binding_status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("synthetic initial candidate handoff count",
               handoff.candidate_count,
               1u);
    check_size("synthetic initial candidate handoff expected offset",
               handoff.expected_offset,
               candidate_offset);
    check_size("synthetic initial candidate handoff descriptor delta",
               handoff.descriptor_delta,
               0xa852u);
    check_int("synthetic initial candidate handoff anchor match",
              handoff.matches_initial_anchor,
              1);
    check_int("synthetic initial candidate loaded",
              handoff.loaded,
              1);
    check_int("synthetic initial candidate level width",
              level.width,
              (int)candidate_width);
    check_int("synthetic initial candidate level height",
              level.height,
              (int)candidate_height);
    check_int("synthetic initial candidate start x", level.start_x, 1);
    check_int("synthetic initial candidate start y", level.start_y, 1);
    check_int("synthetic initial candidate start dir", level.start_dir, 0);

    candidate[4] = 0x11u; /* corrupt the source-locked seed gate */
    status = theron_v1_track02_load_initial_level_candidate(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_ISO,
        descriptor_offset,
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);
    check_int("synthetic initial candidate rejects wrong header",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL);
}

static void probe_synthetic_initial_candidate_wrong_anchor_rejected(void) {
    enum {
        descriptor_offset = 0xc000u,
        wrong_candidate_offset = 0x2000u,
        candidate_width = 32u,
        candidate_height = 27u
    };
    uint8_t track[0xd000u];
    uint8_t *candidate;
    Theron_V1_Level level;
    Theron_Track02LevelHandoff handoff;
    Theron_Track02LevelCandidateCatalog catalog;
    Theron_Track02LevelHandoffStatus status;

    memset(track, 0, sizeof(track));
    memcpy(track + descriptor_offset, g_canonical_descriptor, sizeof(g_canonical_descriptor));
    candidate = track + wrong_candidate_offset;
    write_be16(candidate + 0, candidate_width);
    write_be16(candidate + 2, candidate_height);
    write_be32(candidate + 4, 0x0108e938u);
    write_be16(candidate + 8, 0x0026u);
    candidate[10] = 0u;
    candidate[11] = 0u;
    memset(candidate + 12, THERON_SQUARE_WALL, candidate_width * candidate_height);
    candidate[12 + 1u * candidate_width + 1u] = THERON_SQUARE_FLOOR;
    candidate[12 + 1u * candidate_width + 2u] = THERON_SQUARE_FLOOR;

    status = theron_v1_track02_scan_level_candidates(
        track,
        sizeof(track),
        &catalog);
    check_int("synthetic wrong-anchor candidate scan status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("synthetic wrong-anchor candidate scan count",
               catalog.candidate_count,
               1u);
    check_size("synthetic wrong-anchor candidate scan offset",
               catalog.candidates[0].absolute_offset,
               wrong_candidate_offset);
    check_int("synthetic wrong-anchor bind anchor",
              theron_v1_track02_bind_level_candidate_anchor(
                  descriptor_offset,
                  &catalog),
              1);
    check_int("synthetic wrong-anchor candidate no anchor match",
              catalog.candidates[0].matches_initial_anchor,
              0);

    status = theron_v1_track02_load_initial_level_candidate(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_BIN,
        descriptor_offset,
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);
    check_int("synthetic wrong-anchor candidate handoff rejected",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL);
    check_int("synthetic undersized descriptor offset rejected",
              theron_v1_track02_initial_candidate_expected_offset(0x100u, NULL),
              0);
}

static void write_initial_candidate_fixture(uint8_t *candidate,
                                            size_t candidate_width,
                                            size_t candidate_height) {
    write_be16(candidate + 0, (uint16_t)candidate_width);
    write_be16(candidate + 2, (uint16_t)candidate_height);
    write_be32(candidate + 4, 0x0108e938u);
    write_be16(candidate + 8, 0x0026u);
    candidate[10] = 0u;
    candidate[11] = 0u;
    memset(candidate + 12,
           THERON_SQUARE_WALL,
           candidate_width * candidate_height);
    candidate[12 + 1u * candidate_width + 1u] = THERON_SQUARE_FLOOR;
    candidate[12 + 1u * candidate_width + 2u] = THERON_SQUARE_FLOOR;
    candidate[12 + 2u * candidate_width + 2u] = THERON_SQUARE_EXIT;
}

static void write_raw_user_data_range(uint8_t *track,
                                      size_t raw_offset,
                                      const uint8_t *bytes,
                                      size_t byte_count) {
    size_t copied = 0u;

    while (copied < byte_count) {
        const size_t sector = raw_offset / THERON_TRACK02_RAW_SECTOR_BYTES;
        const size_t within = raw_offset % THERON_TRACK02_RAW_SECTOR_BYTES;
        const size_t user_end = THERON_TRACK02_RAW_USER_DATA_OFFSET +
            THERON_TRACK02_RAW_USER_DATA_BYTES;
        size_t chunk = user_end - within;

        if (chunk > byte_count - copied) {
            chunk = byte_count - copied;
        }
        memcpy(track + raw_offset, bytes + copied, chunk);
        copied += chunk;
        if (copied < byte_count) {
            raw_offset = (sector + 1u) * THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET;
        }
    }
}

static void probe_split_raw_initial_candidate_semantic_handoff(void) {
    static const char *const md5s[] = {
        THERON_TRACK02_MD5_US_BIN,
        THERON_TRACK02_MD5_JP_BIN
    };
    static const size_t candidate_offsets[] = { 0x076cu, 0x0740u };
    enum {
        raw_sector_count = 24u,
        candidate_width = 32u,
        candidate_height = 27u,
        candidate_size = 12u + candidate_width * candidate_height
    };
    size_t layout;

    for (layout = 0u; layout < sizeof(md5s) / sizeof(md5s[0]); ++layout) {
        uint8_t track[raw_sector_count * THERON_TRACK02_RAW_SECTOR_BYTES];
        uint8_t candidate[candidate_size];
        const size_t descriptor_offset = candidate_offsets[layout] + 0xa852u;
        const size_t seed_table_offset = descriptor_offset - 0x1584u + 0x20u;
        Theron_Track02StartupSemanticHandoff semantic;
        Theron_Track02StartupRuntimeReceipt receipt;
        Theron_Track02LevelHandoff level_handoff;
        Theron_Track02LevelHandoffStatus status;
        Theron_V1_Level level;
        size_t i;

        memset(track, 0, sizeof(track));
        memset(candidate, 0, sizeof(candidate));
        write_initial_candidate_fixture(candidate, candidate_width,
                                        candidate_height);
        write_raw_user_data_range(track, candidate_offsets[layout], candidate,
                                  sizeof(candidate));
        memcpy(track + descriptor_offset, g_canonical_descriptor,
               sizeof(g_canonical_descriptor));
        for (i = 0u; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
            write_le32(track + seed_table_offset + i * 4u,
                       313u + (uint32_t)(i * 101u));
        }

        status = theron_v1_track02_load_startup_semantic_level(
            track, sizeof(track), md5s[layout], descriptor_offset,
            THERON_DUNGEON_1_AKUTUBA, 0, &level, &semantic,
            &level_handoff);
        check_int("split raw semantic level status", status,
                  THERON_TRACK02_LEVEL_HANDOFF_OK);
        check_int("split raw semantic level ready", semantic.ready_for_runtime,
                  1);
        check_int("split raw semantic level loaded", level_handoff.loaded, 1);
        check_size("split raw semantic raw offset",
                   level_handoff.absolute_offset, candidate_offsets[layout]);
        check_int("split raw semantic level width", level.width,
                  (int)candidate_width);
        check_int("split raw semantic level start x", level.start_x, 1);
        check_int("split raw semantic level start y", level.start_y, 1);
        check_int("split raw semantic receipt valid",
                  theron_v1_track02_startup_runtime_receipt_from_handoff(
                      &semantic, &receipt), 1);
        check_int("split raw semantic receipt blocks fallback visuals",
                  receipt.fallback_visuals_allowed, 0);
    }
}

static void probe_synthetic_initial_candidate_user_data_offsets(void) {
    enum {
        raw_sector_count = 21u,
        candidate_offset = THERON_TRACK02_RAW_SECTOR_BYTES +
            THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x40u,
        descriptor_offset = candidate_offset + 0xa852u,
        descriptor_base_offset = descriptor_offset - 0x1584u,
        seed_table_offset = descriptor_base_offset + 0x20u,
        candidate_width = 32u,
        candidate_height = 27u,
        candidate_size = 12u + candidate_width * candidate_height,
        expected_user_offset = THERON_TRACK02_RAW_USER_DATA_BYTES + 0x40u
    };
    uint8_t track[raw_sector_count * THERON_TRACK02_RAW_SECTOR_BYTES];
    uint8_t copied[candidate_size];
    Theron_Track02LevelCandidateCatalog catalog;
    Theron_Track02SignalStatus signal_status;
    Theron_Track02LevelHandoffStatus status;
    Theron_Track02StartupSemanticHandoff startup_handoff;
    Theron_Track02StartupSemanticHandoff loaded_semantic_handoff;
    Theron_Track02StartupRuntimeReceipt startup_receipt;
    Theron_Track02StartupRuntimeReceipt loaded_startup_receipt;
    Theron_Track02LevelHandoff loaded_level_handoff;
    Theron_V1_Level loaded_level;
    Theron_V1_World runtime_world;
    Theron_StartupAction runtime_action;
    Theron_StartupActionPlan runtime_plan;
    Theron_V1StartupRuntimeEntryResult runtime_result;
    Theron_StartupHostReceipt runtime_host_receipt;
    Theron_StartupStateReceipt runtime_state_receipt;
    char runtime_receipt[512];
    uint8_t *runtime_track = NULL;
    const size_t runtime_candidate_offset = 0x7015b4u;
    const size_t runtime_descriptor_base_offset = 0x70be06u - 0x1584u;
    const size_t runtime_seed_table_offset =
        runtime_descriptor_base_offset + 0x20u;
    const size_t runtime_track_size =
        ((0x712840u + 44u + THERON_TRACK02_RAW_SECTOR_BYTES - 1u) /
         THERON_TRACK02_RAW_SECTOR_BYTES) *
        THERON_TRACK02_RAW_SECTOR_BYTES;
    size_t copied_size = 0u;
    size_t copied_user_offset = 0u;
    static const uint32_t progression_seeds[THERON_TRACK02_DUNGEON_COUNT] = {
        313u, 414u, 527u, 632u, 749u, 856u, 967u
    };
    static const size_t runtime_descriptor_offsets[3] = {
        0x70be06u, 0x70e2c6u, 0x710904u
    };
    static const size_t runtime_span_offsets[3] = {
        0x2d53e0u, 0x47d040u, 0x712840u
    };
    static const uint8_t runtime_post_boundary_span[44] = {
        0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
        0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
        0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
        0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
        0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
        0x93, 0x80, 0x00, 0x3f
    };

    memset(track, 0, sizeof(track));
    memcpy(track + descriptor_offset,
           g_canonical_descriptor,
           sizeof(g_canonical_descriptor));
    for (size_t i = 0u; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        write_le32(track + seed_table_offset + i * 4u, progression_seeds[i]);
    }
    write_initial_candidate_fixture(track + candidate_offset,
                                    candidate_width,
                                    candidate_height);

    status = theron_v1_track02_scan_level_candidates(track,
                                                     sizeof(track),
                                                     &catalog);
    check_int("synthetic user-data candidate scan status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("synthetic user-data candidate count",
               catalog.candidate_count,
               1u);
    signal_status = theron_v1_track02_bind_level_candidate_user_offsets(
        sizeof(track),
        THERON_TRACK02_MD5_US_BIN,
        &catalog);
    check_int("synthetic user-data bind status",
              signal_status,
              THERON_TRACK02_SIGNAL_OK);
    check_int("synthetic user-data candidate offset valid",
              catalog.candidates[0].user_data_offset_valid,
              1);
    check_size("synthetic user-data candidate offset",
               catalog.candidates[0].user_data_offset,
               expected_user_offset);
    status = theron_v1_track02_copy_initial_level_user_data_window(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_BIN,
        descriptor_offset,
        copied,
        sizeof(copied),
        &copied_size,
        &copied_user_offset);
    check_int("synthetic user-data initial window copy status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("synthetic user-data initial window size",
               copied_size,
               candidate_size);
    check_size("synthetic user-data initial window offset",
               copied_user_offset,
               expected_user_offset);
    check_int("synthetic user-data initial window bytes",
              memcmp(copied, track + candidate_offset, candidate_size),
              0);
    status = theron_v1_track02_bind_startup_semantic_handoff(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_BIN,
        descriptor_offset,
        &startup_handoff);
    check_int("synthetic startup semantic handoff status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_int("synthetic startup semantic handoff ready",
              startup_handoff.ready_for_runtime,
              1);
    check_int("synthetic startup semantic seed-table status",
              startup_handoff.seed_table_status,
              THERON_TRACK02_SEMANTIC_BINDING_OK);
    check_size("synthetic startup semantic candidate count",
               startup_handoff.initial_candidate.candidate_count,
               1u);
    check_size("synthetic startup semantic user-data offset",
               startup_handoff.user_data_offset,
               expected_user_offset);
    check_int("synthetic startup semantic user-data valid",
              startup_handoff.user_data_offset_valid,
              1);
    check_u32("synthetic startup semantic payload seed",
              startup_handoff.startup_seed,
              0x0108e938u);
    check_u16("synthetic startup semantic payload level",
              startup_handoff.startup_level_index,
              0x0026u);
    check_u32("synthetic startup semantic progression seed",
              startup_handoff.seed_table_binding.dungeon_seed_table.seeds[0],
              313u);
    check_int("synthetic startup semantic seed concepts distinct",
              startup_handoff.startup_seed_in_seed_table,
              0);
    check_int("synthetic startup semantic runtime receipt",
              theron_v1_track02_startup_runtime_receipt_from_handoff(
                  &startup_handoff,
                  &startup_receipt),
              1);
    check_int("synthetic startup semantic receipt valid",
              startup_receipt.valid,
              1);
    check_int("synthetic startup semantic receipt no fallback visuals",
              startup_receipt.fallback_visuals_allowed,
              0);
    check_size("synthetic startup semantic receipt raw offset",
               startup_receipt.raw_offset,
               candidate_offset);
    check_size("synthetic startup semantic receipt user-data offset",
               startup_receipt.user_data_offset,
               expected_user_offset);
    check_u16("synthetic startup semantic receipt width",
              startup_receipt.header_width,
              candidate_width);
    check_u32("synthetic startup semantic receipt header seed",
              startup_receipt.header_seed,
              0x0108e938u);
    check_u32("synthetic startup semantic receipt progression seed0",
              startup_receipt.progression_seed0,
              313u);
    status = theron_v1_track02_load_startup_semantic_level(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_BIN,
        descriptor_offset,
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &loaded_level,
        &loaded_semantic_handoff,
        &loaded_level_handoff);
    check_int("synthetic startup semantic level load status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_int("synthetic startup semantic level load ready",
              loaded_semantic_handoff.ready_for_runtime,
              1);
    check_size("synthetic startup semantic level load offset",
               loaded_level_handoff.absolute_offset,
               candidate_offset);
    check_size("synthetic startup semantic level load user-data offset",
               loaded_semantic_handoff.user_data_offset,
               expected_user_offset);
    check_int("synthetic startup semantic level load receipt",
              theron_v1_track02_startup_runtime_receipt_from_handoff(
                  &loaded_semantic_handoff,
                  &loaded_startup_receipt),
              1);
    check_int("synthetic startup semantic level load receipt ready",
              loaded_startup_receipt.ready_for_runtime,
              1);
    check_int("synthetic startup semantic level load receipt no fallback visuals",
              loaded_startup_receipt.fallback_visuals_allowed,
              0);
    check_int("synthetic startup semantic level load map status",
              loaded_level_handoff.map_status,
              THERON_MAP_OK);
    check_int("synthetic startup semantic level load width",
              loaded_level.width,
              (int)candidate_width);
    check_int("synthetic startup semantic level load start x",
              loaded_level.start_x,
              1);
    check_int("synthetic startup semantic level load start y",
              loaded_level.start_y,
              1);
    runtime_track = (uint8_t *)calloc(1u, runtime_track_size);
    check_int("synthetic startup semantic raw-US runtime fixture alloc",
              runtime_track != NULL,
              1);
    if (runtime_track) {
        for (size_t i = 0u; i < 3u; ++i) {
            memcpy(runtime_track + runtime_descriptor_offsets[i],
                   g_canonical_descriptor,
                   sizeof(g_canonical_descriptor));
            memcpy(runtime_track + runtime_span_offsets[i],
                   runtime_post_boundary_span,
                   sizeof(runtime_post_boundary_span));
        }
        for (size_t i = 0u; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
            write_le32(runtime_track + runtime_seed_table_offset + i * 4u,
                       progression_seeds[i]);
        }
        write_initial_candidate_fixture(runtime_track + runtime_candidate_offset,
                                        candidate_width,
                                        candidate_height);
        {
            Theron_Track02StartupSemanticHandoff runtime_handoff;
            status = theron_v1_track02_bind_startup_semantic_handoff(
                runtime_track,
                runtime_track_size,
                THERON_TRACK02_MD5_US_BIN,
                runtime_descriptor_offsets[0],
                &runtime_handoff);
            check_int("synthetic startup semantic raw-US bind status",
                      status,
                      THERON_TRACK02_LEVEL_HANDOFF_OK);
            check_int("synthetic startup semantic raw-US seed status",
                      runtime_handoff.seed_table_status,
                      THERON_TRACK02_SEMANTIC_BINDING_OK);
        }
        theron_v1_world_init(&runtime_world);
        theron_v1_startup_action_init(&runtime_action);
        runtime_action.kind = THERON_STARTUP_ACTION_ENTER_FORCEFIELD;
        check_int("synthetic startup semantic runtime plan",
                  theron_v1_startup_plan_for_action(&runtime_action,
                                                     &runtime_plan),
                  1);
        memset(runtime_receipt, 0, sizeof(runtime_receipt));
        /* f019ce24d contract (restored; a later merge had reverted the
         * probe to the pre-gate success expectations): synthetic fixtures
         * without authenticated stage-three loader bytes are blocked at the
         * runtime route — no handoff, no bank identity, no state receipt,
         * fallback visuals stay blocked. */
        check_int(
            "synthetic startup semantic runtime rejects non-authenticated media",
            theron_v1_startup_runtime_load_initial_level_with_host_receipts(
                &runtime_world,
                runtime_track,
                runtime_track_size,
                THERON_TRACK02_MD5_US_BIN,
                THERON_DUNGEON_1_AKUTUBA,
                &runtime_plan,
                &runtime_result,
                &runtime_host_receipt,
                &runtime_state_receipt,
                runtime_receipt,
                sizeof(runtime_receipt)),
            0);
        check_int("synthetic startup semantic runtime route is blocked",
                  runtime_result.runtime_level_source,
                  THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED);
        check_int("synthetic startup semantic runtime has no handoff",
                  runtime_result.track02_semantic_handoff,
                  0);
        check_int("synthetic startup semantic runtime has no bank identity",
                  runtime_world.runtime_media.identity.ready &&
                  runtime_world.runtime_media.identity.track02_variant ==
                      THERON_TRACK02_VARIANT_US_BIN &&
                  runtime_world.runtime_media.identity.bank_anchor_index == 0u &&
                  runtime_world.runtime_media.identity.bank_descriptor_offset ==
                      runtime_descriptor_offsets[0] &&
                  runtime_world.runtime_media.identity.bank_stride == 0x0400u,
            0);
        check_int("synthetic startup semantic runtime blocks fallback",
                  runtime_result.fallback_visuals_blocked,
                  1);
        check_int("synthetic startup semantic runtime emits no state receipt",
                  runtime_state_receipt.runtime_level_source,
                  THERON_V1_STARTUP_RUNTIME_LEVEL_NONE);
        check_int("synthetic startup semantic runtime state has no handoff",
                  runtime_state_receipt.runtime_track02_semantic_handoff,
                  0);
        check_int("synthetic startup semantic runtime receipt is rejected",
                  strstr(runtime_receipt,
                         "stage-three loader bytes rejected") != NULL,
                  1);
        check_int("synthetic startup semantic runtime host text is rejected",
                  strstr(runtime_host_receipt.inspect_detail,
                         "stage-three loader bytes rejected") != NULL,
                  1);
        check_int("synthetic startup semantic runtime host route is blocked",
                  strstr(runtime_host_receipt.inspect_detail,
                         "route=track02-blocked") != NULL,
                  1);
        free(runtime_track);
    }
    status = theron_v1_track02_copy_initial_level_user_data_window(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_BIN,
        descriptor_offset,
        copied,
        sizeof(copied) - 1u,
        &copied_size,
        &copied_user_offset);
    check_int("synthetic user-data initial window copy capacity guard",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT);

    signal_status = theron_v1_track02_bind_level_candidate_user_offsets(
        sizeof(track),
        THERON_TRACK02_MD5_US_ISO,
        &catalog);
    check_int("synthetic ISO user-data candidate bind unsupported",
              signal_status,
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT);
}

static void probe_synthetic_multiple_initial_candidates_rejected(void) {
    enum {
        descriptor_offset = 0xc000u,
        base_offset = descriptor_offset - 0x1584u,
        candidate_offset = base_offset - 0x92ceu,
        extra_candidate_offset = 0x2400u,
        candidate_width = 32u,
        candidate_height = 27u
    };
    uint8_t track[0xd000u];
    Theron_V1_Level level;
    Theron_Track02LevelHandoff handoff;
    Theron_Track02LevelCandidateCatalog catalog;
    Theron_Track02InitialCandidateBinding binding;
    Theron_Track02LevelHandoffStatus status;

    memset(track, 0, sizeof(track));
    memcpy(track + descriptor_offset,
           g_canonical_descriptor,
           sizeof(g_canonical_descriptor));
    write_initial_candidate_fixture(track + candidate_offset,
                                    candidate_width,
                                    candidate_height);
    write_initial_candidate_fixture(track + extra_candidate_offset,
                                    candidate_width,
                                    candidate_height);

    status = theron_v1_track02_scan_level_candidates(
        track,
        sizeof(track),
        &catalog);
    check_int("synthetic multi-candidate scan status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("synthetic multi-candidate scan count",
               catalog.candidate_count,
               2u);

    status = theron_v1_track02_bind_initial_level_candidate(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_BIN,
        descriptor_offset,
        &binding);
    check_int("synthetic multi-candidate binding rejected",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_AMBIGUOUS_CANDIDATES);
    check_size("synthetic multi-candidate binding count",
               binding.candidate_count,
               2u);
    check_size("synthetic multi-candidate binding index sentinel",
               binding.candidate_index,
               (size_t)-1);

    status = theron_v1_track02_load_initial_level_candidate(
        track,
        sizeof(track),
        THERON_TRACK02_MD5_US_BIN,
        descriptor_offset,
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);
    check_int("synthetic multi-candidate handoff rejected",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_AMBIGUOUS_CANDIDATES);
    check_int("synthetic multi-candidate handoff binding status",
              handoff.binding_status,
              THERON_TRACK02_LEVEL_HANDOFF_AMBIGUOUS_CANDIDATES);
    check_size("synthetic multi-candidate handoff count",
               handoff.candidate_count,
               2u);
    check_int("synthetic multi-candidate handoff not loaded",
              handoff.loaded,
              0);
}

/* The envelope receipt has no synthetic-positive route.  Its facts are only
 * meaningful after the authenticated raw JP/US corpus passes the IPL and
 * descriptor gates; these checks keep test fixtures from becoming a fallback
 * source of level or object semantics. */
static void probe_initial_level_envelope_rejected_without_corpus(void) {
    uint8_t sector[THERON_TRACK02_RAW_SECTOR_BYTES];
    Theron_Track02InitialLevelEnvelopeReceipt envelope;
    Theron_Track02InitialLevelLoaderSemanticReceipt semantics;
    Theron_Track02SignalStatus status;

    memset(sector, 0, sizeof(sector));
    memset(&envelope, 0xff, sizeof(envelope));

    status = theron_v1_track02_decode_initial_level_envelope(
        NULL, sizeof(sector), THERON_TRACK02_MD5_US_BIN, &envelope);
    check_int("initial envelope null media rejected",
              status,
              THERON_TRACK02_SIGNAL_BAD_INPUT);
    check_int("initial envelope null media clears receipt", envelope.valid, 0);

    memset(&envelope, 0xff, sizeof(envelope));
    status = theron_v1_track02_decode_initial_level_envelope(
        sector, sizeof(sector), "not-a-track02-md5", &envelope);
    check_int("initial envelope unknown media rejected",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    check_int("initial envelope unknown media clears receipt", envelope.valid, 0);
    check_int("initial envelope unknown media has no grid", envelope.grid_byte_count, 0);
    check_int("initial envelope unknown media has no object claim",
              envelope.object_tail_semantics_proven, 0);

    memset(&envelope, 0xff, sizeof(envelope));
    status = theron_v1_track02_decode_initial_level_envelope(
        sector, sizeof(sector), THERON_TRACK02_MD5_US_BIN, &envelope);
    check_int("initial envelope non-corpus bytes rejected",
              status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    check_int("initial envelope non-corpus bytes clear receipt", envelope.valid, 0);
    check_int("initial envelope non-corpus bytes have no grid",
              envelope.grid_byte_count, 0);
    check_int("initial envelope non-corpus bytes have no object claim",
              envelope.object_tail_semantics_proven, 0);
    check_int("initial envelope non-corpus bytes allow no fallback",
              envelope.fallback_visuals_allowed, 0);

    memset(&semantics, 0xff, sizeof(semantics));
    status = theron_v1_track02_decode_initial_level_loader_semantics(
        sector, sizeof(sector), THERON_TRACK02_MD5_US_BIN, &semantics);
    check_int("initial loader semantics non-corpus bytes rejected",
              status, THERON_TRACK02_SIGNAL_NOT_FOUND);
    check_int("initial loader semantics clears receipt", semantics.valid, 0);
    check_int("initial loader semantics has no object claim",
              semantics.object_tail_semantics_proven, 0);
    check_int("initial loader semantics allows no fallback",
              semantics.fallback_visuals_allowed, 0);
}

static void probe_negative_handoffs(void) {
    uint8_t track[0x3000u];
    Theron_V1_Level level;
    Theron_Track02LevelHandoff handoff;
    Theron_Track02LevelHandoffStatus status;

    memset(track, 0, sizeof(track));
    memcpy(track + 0x1584u, g_canonical_descriptor, sizeof(g_canonical_descriptor));

    status = theron_v1_track02_load_descriptor_window_level(
        track,
        sizeof(track),
        0x1584u,
        0u,
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);
    check_int("zero-fill window rejected",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_WINDOW_NOT_DATA);

    track[0x0c20u] = 0x7fu; /* data window, but invalid level header */
    status = theron_v1_track02_load_descriptor_window_level(
        track,
        sizeof(track),
        0x1584u,
        3u,
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);
    check_int("invalid data window reaches loader",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED);
    check_int("invalid data map status",
              handoff.map_status,
              THERON_MAP_ERR_INVALID_GRID);

    status = theron_v1_track02_load_descriptor_window_level(
        track,
        sizeof(track),
        0x1584u,
        THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES,
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);
    check_int("out-of-range entry rejected",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT);

    status = theron_v1_track02_load_descriptor_window_level(
        track,
        0x1600u,
        0x1584u,
        1u,
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);
    check_int("truncated track rejected",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND);
}

static void probe_real_data_initial_candidate(const char *label,
                                              const char *md5_hex,
                                              const char *env_name,
                                              const char *default_file) {
    char path[512];
    const char *env_path = getenv(env_name);
    const char *path_to_read;
    uint8_t *data = NULL;
    size_t size = 0;
    char local_md5[33] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    Theron_V1_Level level;
    Theron_Track02LevelHandoff handoff;
    Theron_Track02LevelCandidateCatalog catalog;
    Theron_Track02InitialCandidateBinding binding;
    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    Theron_Track02InitialLevelEnvelopeReceipt envelope;
    Theron_Track02InitialLevelLoaderSemanticReceipt semantics;
    Theron_Track02InitialLevelLoaderRoute loader_route;
    Theron_Track02LevelHandoffStatus status;
    Theron_Track02SignalStatus user_offset_status;
    size_t expected_candidate_offset = 0u;
    size_t expected_user_data_offset = 0u;
    uint8_t copied_candidate[12u + 32u * 27u];
    size_t copied_candidate_size = 0u;
    size_t copied_candidate_user_offset = 0u;

    path_to_read = (env_path && env_path[0]) ? env_path : NULL;
    if (!path_to_read) {
        default_data_path(default_file, path);
        path_to_read = path;
    }

    if (!file_exists(path_to_read)) {
        printf("SKIP %s initial candidate: no Track 02 image at %s\n",
               label, path_to_read);
        ++g_skip;
        return;
    }
    if (!m12_file_md5_hex(path_to_read, local_md5)) {
        printf("FAIL %s initial candidate: could not compute MD5 for %s\n",
               label, path_to_read);
        ++g_fail;
        return;
    }
    if (strcmp(local_md5, md5_hex) != 0) {
        if (strcmp(local_md5, THERON_TRACK02_MD5_US_ISO_TAIL) == 0) {
            printf("SKIP %s initial candidate: tail-only US ISO (need concatenated TQUS19+TQUS02End)\n", label);
            ++g_skip;
            return;
        }
        printf("FAIL %s initial candidate: MD5 %s does not match expected %s\n",
               label, local_md5, md5_hex);
        ++g_fail;
        return;
    }
    if (!read_file(path_to_read, &data, &size)) {
        printf("FAIL %s initial candidate: could not read %s\n",
               label, path_to_read);
        ++g_fail;
        return;
    }

    signal_status = theron_v1_track02_find_bank_signal(data, size, local_md5, &signal);
    if (signal_status != THERON_TRACK02_SIGNAL_OK || signal.anchor_count == 0u) {
        printf("FAIL %s initial candidate: bank signal status %s anchors=%zu\n",
               label,
               theron_v1_track02_signal_status_name(signal_status),
               signal.anchor_count);
        ++g_fail;
        free(data);
        return;
    }

    status = theron_v1_track02_scan_level_candidates(
        data,
        size,
        &catalog);
    check_int("real initial candidate scan status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("real initial candidate scan count",
               catalog.candidate_count,
               1u);
    check_int("real initial candidate bind anchor",
              theron_v1_track02_bind_level_candidate_anchor(
                  signal.descriptor_offsets[0],
                  &catalog),
              1);
    check_size("real initial candidate descriptor delta",
               catalog.candidates[0].descriptor_delta,
               0xa852u);
    check_int("real initial candidate anchor match",
              catalog.candidates[0].matches_initial_anchor,
              1);
    expected_user_data_offset =
        (strcmp(md5_hex, THERON_TRACK02_MD5_US_BIN) == 0)
            ? 0x619914u
            : 0x619114u;
    user_offset_status = theron_v1_track02_bind_level_candidate_user_offsets(
        size,
        local_md5,
        &catalog);
    check_int("real initial candidate user-data bind status",
              user_offset_status,
              THERON_TRACK02_SIGNAL_OK);
    check_int("real initial candidate user-data offset valid",
              catalog.candidates[0].user_data_offset_valid,
              1);
    check_size("real initial candidate user-data offset",
               catalog.candidates[0].user_data_offset,
               expected_user_data_offset);
    status = theron_v1_track02_bind_initial_level_candidate(
        data,
        size,
        local_md5,
        signal.descriptor_offsets[0],
        &binding);
    check_int("real initial candidate binding status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("real initial candidate binding count",
               binding.candidate_count,
               1u);
    check_size("real initial candidate binding index",
               binding.candidate_index,
               0u);
    check_size("real initial candidate binding offset",
               binding.candidate.absolute_offset,
               catalog.candidates[0].absolute_offset);
    check_size("real initial candidate binding expected offset",
               binding.expected_offset,
               catalog.candidates[0].absolute_offset);
    check_int("real initial candidate binding anchor match",
              binding.matches_initial_anchor,
              1);

    signal_status = theron_v1_track02_capture_initial_level_object_boundary(
        data, size, local_md5, &boundary);
    check_int("real initial level/object boundary status",
              signal_status,
              THERON_TRACK02_SIGNAL_OK);
    check_int("real initial level/object boundary valid", boundary.valid, 1);
    check_u32("real initial level/object CD record", boundary.track02_record, 0x0b52u);
    check_size("real initial level/object level offset in record",
               boundary.level_user_data_offset_in_record, 0x114u);
    check_size("real initial level/object level bytes", boundary.level_byte_count, 0x36cu);
    check_u16("real initial level/object opaque header extension",
              boundary.level_header_extension_be,
              0x0103u);
    check_size("real initial level/object opaque boundary in record",
               boundary.object_boundary_user_data_offset_in_record, 0x480u);
    check_size("real initial level/object opaque tail bytes",
               boundary.following_user_data_bytes_in_record, 0x380u);
    check_int("real initial level/object opaque tail fingerprinted",
              boundary.following_user_data_hash != 0u, 1);
    check_int("real initial level/object table stays unparsed",
              boundary.object_table_parsed, 0);
    check_int("real initial level/object table stays unproven",
              boundary.object_table_semantics_proven, 0);
    check_int("real initial level/object boundary promotion blocked",
              boundary.promotion_blocked, 1);
    check_int("real initial level/object receipt fingerprinted",
              boundary.receipt_hash != 0u, 1);
    printf("%s CD boundary: record=0x%04x level-user=0x%zx bytes=0x%zx "
           "opaque-boundary=0x%zx tail=0x%zx hash=0x%08x\n",
           label, (unsigned)boundary.track02_record,
           boundary.level_user_data_offset_in_record, boundary.level_byte_count,
           boundary.object_boundary_user_data_offset_in_record,
           boundary.following_user_data_bytes_in_record,
           (unsigned)boundary.receipt_hash);

    signal_status = theron_v1_track02_decode_initial_level_envelope(
        data, size, local_md5, &envelope);
    check_int("real initial envelope decode status", signal_status,
              THERON_TRACK02_SIGNAL_OK);
    check_int("real initial envelope valid", envelope.valid, 1);
    check_u32("real initial envelope record", envelope.track02_record, 0x0b52u);
    check_size("real initial envelope bytes", envelope.level_byte_count, 0x36cu);
    check_u16("real initial envelope width", envelope.width, 32u);
    check_u16("real initial envelope height", envelope.height, 27u);
    check_u32("real initial envelope seed", envelope.seed, 0x0108e938u);
    check_u16("real initial envelope level index", envelope.level_index, 0x0026u);
    check_u16("real initial envelope opaque extension",
              envelope.header_extension_be, 0x0103u);
    check_size("real initial envelope grid offset",
               envelope.grid_offset_in_envelope, 12u);
    check_size("real initial envelope grid bytes", envelope.grid_byte_count, 0x360u);
    check_int("real initial envelope grid fingerprinted", envelope.grid_hash != 0u, 1);
    check_int("real initial envelope header semantics proven",
              envelope.header_semantics_proven, 1);
    check_int("real initial envelope grid semantics remain unproven",
              envelope.grid_semantics_proven, 0);
    check_int("real initial envelope extension remains opaque",
              envelope.header_extension_semantics_proven, 0);
    check_int("real initial envelope object tail remains opaque",
              envelope.object_tail_semantics_proven, 0);
    check_int("real initial envelope blocks fallback visuals",
              envelope.fallback_visuals_allowed, 0);
    check_int("real initial envelope receipt fingerprinted",
              envelope.receipt_hash != 0u, 1);

    signal_status = theron_v1_track02_decode_initial_level_loader_semantics(
        data, size, local_md5, &semantics);
    check_int("real initial loader semantics remain unproven", signal_status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    check_int("real initial loader semantics receipt cleared", semantics.valid, 0);

    signal_status = theron_v1_track02_load_initial_level_loader_route(
        data, size, local_md5, THERON_DUNGEON_1_AKUTUBA, 0,
        &loader_route);
    check_int("real initial loader route remains unproven", signal_status,
              THERON_TRACK02_SIGNAL_NOT_FOUND);
    check_int("real initial loader route cleared", loader_route.valid, 0);
    signal_status = theron_v1_track02_load_initial_level_loader_route(
        data, size, local_md5, THERON_DUNGEON_2_DRATOR, 0,
        &loader_route);
    check_int("real initial loader route rejects unproven dungeon",
              signal_status, THERON_TRACK02_SIGNAL_NOT_FOUND);

    status = theron_v1_track02_load_initial_level_candidate(
        data,
        size,
        local_md5,
        signal.descriptor_offsets[0],
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);
    printf("%s initial candidate: status=%s map=%d offset=0x%zx size=%zu header=%ux%u seed=0x%08x start=(%d,%d,%d)\n",
           label,
           theron_v1_track02_level_handoff_status_name(status),
           handoff.map_status,
           handoff.absolute_offset,
           handoff.byte_count,
           (unsigned)handoff.header_width,
           (unsigned)handoff.header_height,
           (unsigned)handoff.header_seed,
           level.start_x,
           level.start_y,
           level.start_dir);

    check_int("real initial candidate status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_u16("real initial candidate width", handoff.header_width, 32u);
    check_u16("real initial candidate height", handoff.header_height, 27u);
    check_u32("real initial candidate seed", handoff.header_seed, 0x0108e938u);
    check_u16("real initial candidate level", handoff.header_level_index, 0x0026u);
    check_int("real initial candidate handoff binding status",
              handoff.binding_status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("real initial candidate handoff count", handoff.candidate_count, 1u);
    check_size("real initial candidate handoff expected offset",
               handoff.expected_offset,
               handoff.absolute_offset);
    check_size("real initial candidate handoff descriptor delta",
               handoff.descriptor_delta,
               0xa852u);
    check_int("real initial candidate handoff anchor match",
              handoff.matches_initial_anchor,
              1);
    check_int("real initial candidate handoff user-data offset valid",
              handoff.user_data_offset_valid,
              1);
    check_size("real initial candidate handoff user-data offset",
               handoff.user_data_offset,
               expected_user_data_offset);
    status = theron_v1_track02_copy_initial_level_user_data_window(
        data,
        size,
        local_md5,
        signal.descriptor_offsets[0],
        copied_candidate,
        sizeof(copied_candidate),
        &copied_candidate_size,
        &copied_candidate_user_offset);
    check_int("real initial candidate user-data window copy status",
              status,
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    check_size("real initial candidate user-data window copy size",
               copied_candidate_size,
               handoff.byte_count);
    check_size("real initial candidate user-data window copy offset",
               copied_candidate_user_offset,
               expected_user_data_offset);
    check_int("real initial candidate user-data window bytes",
              memcmp(copied_candidate,
                     data + handoff.absolute_offset,
                     handoff.byte_count),
              0);
    check_int("real initial candidate loaded", handoff.loaded, 1);
    check_int("real initial candidate level width", level.width, 32);
    check_int("real initial candidate level height", level.height, 27);
    check_u32("real initial candidate level seed",
              level.dungeon_seed,
              0x0108e938u);
    check_int("real initial candidate source header level",
              level.source_header_level_index,
              0x0026);
    check_int("real initial candidate start x", level.start_x, 4);
    check_int("real initial candidate start y", level.start_y, 0);
    check_int("real initial candidate start dir", level.start_dir, 0);
    check_size("real initial candidate scan/loader offset",
               catalog.candidates[0].absolute_offset,
               handoff.absolute_offset);
    check_int("real initial candidate expected offset ok",
              theron_v1_track02_initial_candidate_expected_offset(
                  signal.descriptor_offsets[0],
                  &expected_candidate_offset),
              1);
    check_size("real initial candidate expected offset",
               expected_candidate_offset,
               handoff.absolute_offset);
    check_size("real initial candidate scan/loader size",
               catalog.candidates[0].byte_count,
               handoff.byte_count);

    free(data);
}

static void probe_real_data_handoff(const char *label,
                                    const char *md5_hex,
                                    const char *env_name,
                                    const char *default_file) {
    char path[512];
    const char *env_path = getenv(env_name);
    const char *path_to_read;
    uint8_t *data = NULL;
    size_t size = 0;
    char local_md5[33] = {0};
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    size_t ok_count = 0;
    size_t no_claim_count = 0;

    path_to_read = (env_path && env_path[0]) ? env_path : NULL;
    if (!path_to_read) {
        default_data_path(default_file, path);
        path_to_read = path;
    }

    if (!file_exists(path_to_read)) {
        printf("SKIP %s: no Track 02 image at %s\n", label, path_to_read);
        ++g_skip;
        return;
    }
    if (!m12_file_md5_hex(path_to_read, local_md5)) {
        printf("FAIL %s: could not compute MD5 for %s\n", label, path_to_read);
        ++g_fail;
        return;
    }
    if (strcmp(local_md5, md5_hex) != 0) {
        if (strcmp(local_md5, THERON_TRACK02_MD5_US_ISO_TAIL) == 0) {
            printf("SKIP %s: tail-only US ISO (need concatenated TQUS19+TQUS02End)\n", label);
            ++g_skip;
            return;
        }
        printf("FAIL %s: MD5 %s does not match expected %s\n",
               label, local_md5, md5_hex);
        ++g_fail;
        return;
    }
    if (!read_file(path_to_read, &data, &size)) {
        printf("FAIL %s: could not read %s\n", label, path_to_read);
        ++g_fail;
        return;
    }

    signal_status = theron_v1_track02_find_bank_signal(data, size, local_md5, &signal);
    if (signal_status != THERON_TRACK02_SIGNAL_OK) {
        printf("FAIL %s: bank signal status %s\n",
               label,
               theron_v1_track02_signal_status_name(signal_status));
        ++g_fail;
        free(data);
        return;
    }

    if (strcmp(md5_hex, THERON_TRACK02_MD5_US_ISO) == 0) {
        enum { retail_level_bytes = 12u + 32u * 27u };
        uint8_t copied[retail_level_bytes];
        size_t copied_size = 0u;
        size_t copied_user_offset = 0u;
        Theron_V1_Level level;
        Theron_Track02LevelHandoff handoff;
        Theron_Track02LevelHandoffStatus status;

        status = theron_v1_track02_load_initial_level_candidate(
            data, size, local_md5, signal.descriptor_offsets[0],
            THERON_DUNGEON_1_AKUTUBA, 0, &level, &handoff);
        check_int("retail US ISO initial level status", status,
                  THERON_TRACK02_LEVEL_HANDOFF_OK);
        check_size("retail US ISO initial level offset",
                   handoff.absolute_offset, 0x5a9114u);
        check_size("retail US ISO initial level descriptor delta",
                   handoff.descriptor_delta, 0x92f2u);
        check_size("retail US ISO initial level bytes",
                   handoff.byte_count, retail_level_bytes);
        check_int("retail US ISO initial level width", level.width, 32);
        check_int("retail US ISO initial level height", level.height, 27);
        status = theron_v1_track02_copy_initial_level_user_data_window(
            data, size, local_md5, signal.descriptor_offsets[0], copied,
            sizeof(copied), &copied_size, &copied_user_offset);
        check_int("retail US ISO initial level copy status", status,
                  THERON_TRACK02_LEVEL_HANDOFF_OK);
        check_size("retail US ISO initial level copy size", copied_size,
                   retail_level_bytes);
        check_size("retail US ISO initial level copy offset",
                   copied_user_offset, 0x5a9114u);
        check_int("retail US ISO initial level copy bytes",
                  memcmp(copied, data + 0x5a9114u, retail_level_bytes), 0);
    }

    for (size_t anchor = 0; anchor < signal.anchor_count; ++anchor) {
        for (size_t entry = 0; entry < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++entry) {
            Theron_V1_Level level;
            Theron_Track02LevelHandoff handoff;
            Theron_Track02LevelHandoffStatus status =
                theron_v1_track02_load_descriptor_window_level(
                    data,
                    size,
                    signal.descriptor_offsets[anchor],
                    entry,
                    THERON_DUNGEON_1_AKUTUBA,
                    (int)entry,
                    &level,
                    &handoff);

            if (status == THERON_TRACK02_LEVEL_HANDOFF_OK) {
                ++ok_count;
            } else if (status == THERON_TRACK02_LEVEL_HANDOFF_WINDOW_NOT_DATA ||
                       status == THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED) {
                ++no_claim_count;
            } else {
                printf("FAIL %s anchor=%zu entry=%zu unexpected status=%s\n",
                       label,
                       anchor,
                       entry,
                       theron_v1_track02_level_handoff_status_name(status));
                ++g_fail;
            }
        }
    }

    printf("%s: real-data handoff scanned anchors=%zu ok=%zu no-claim=%zu\n",
           label, signal.anchor_count, ok_count, no_claim_count);
    free(data);
}

static void probe_real_data_if_present(void) {
    probe_real_data_initial_candidate(
        "US raw BIN",
        THERON_TRACK02_MD5_US_BIN,
        "FIRESTAFF_THERON_TRACK02_US_BIN",
        "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin");
    probe_real_data_initial_candidate(
        "JP raw BIN",
        THERON_TRACK02_MD5_JP_BIN,
        "FIRESTAFF_THERON_TRACK02_JP_BIN",
        "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin");

    probe_real_data_handoff("US ISO descriptor handoff",
                            THERON_TRACK02_MD5_US_ISO,
                            "FIRESTAFF_THERON_TRACK02_US",
                            "theron/TQUS02End.iso");
    probe_real_data_handoff("US raw BIN descriptor handoff",
                            THERON_TRACK02_MD5_US_BIN,
                            "FIRESTAFF_THERON_TRACK02_US_BIN",
                            "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin");
    probe_real_data_handoff("JP raw BIN descriptor handoff",
                            THERON_TRACK02_MD5_JP_BIN,
                            "FIRESTAFF_THERON_TRACK02_JP_BIN",
                            "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin");
}

static void probe_media_gated_level_bank_selection(void) {
    Theron_V1_World world;
    Theron_StartupMediaStateReceipt media;
    const unsigned int route_bits[4] = {
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD
    };
    size_t i;

    memset(&media, 0, sizeof(media));
    media.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    strcpy(media.track02_md5, THERON_TRACK02_MD5_US_BIN);
    media.startup_media_ready = 1;
    media.startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    media.startup_bitmap_sample_count = 48;
    media.startup_bitmap_route_mask = 0x0fu;
    media.startup_bitmap_nonzero_pixel_count = 128u;
    media.startup_bitmap_checksum = 0x100u;
    media.startup_bitmap_atlas_ready = 1;
    media.startup_bitmap_atlas_route_count = 4;
    media.startup_bitmap_atlas_route_mask = 0x0fu;
    media.startup_bitmap_atlas_tile_count = 48u;
    media.startup_bitmap_atlas_nonzero_pixel_count = 128u;
    media.startup_bitmap_atlas_checksum = 0x200u;
    media.startup_bitmap_wide_route_count = 4u;
    media.startup_bitmap_wide_route_mask = 0x0fu;
    media.startup_bitmap_wide_atlas_tile_count = 48u;
    media.startup_bitmap_raw_route_count = 4u;
    media.startup_bitmap_raw_route_mask = 0x0fu;
    media.startup_bitmap_raw_atlas_tile_count = 48u;
    media.startup_bitmap_title_route_ready = 1;
    media.startup_bitmap_stage_route_ready = 1;
    media.startup_bitmap_soul_room_route_ready = 1;
    media.startup_bitmap_forcefield_route_ready = 1;
    media.startup_bitmap_title_sample_count = 12;
    media.startup_bitmap_stage_sample_count = 12;
    media.startup_bitmap_soul_room_sample_count = 12;
    media.startup_bitmap_forcefield_sample_count = 12;
    media.startup_bitmap_title_nonzero_pixel_count = 32u;
    media.startup_bitmap_stage_nonzero_pixel_count = 32u;
    media.startup_bitmap_soul_room_nonzero_pixel_count = 32u;
    media.startup_bitmap_forcefield_nonzero_pixel_count = 32u;
    media.startup_bitmap_title_checksum = 1u;
    media.startup_bitmap_stage_checksum = 2u;
    media.startup_bitmap_soul_room_checksum = 3u;
    media.startup_bitmap_forcefield_checksum = 4u;
    media.startup_bitmap_title_atlas_tile_count = 12u;
    media.startup_bitmap_stage_atlas_tile_count = 12u;
    media.startup_bitmap_soul_room_atlas_tile_count = 12u;
    media.startup_bitmap_forcefield_atlas_tile_count = 12u;
    media.startup_bitmap_title_atlas_width = 96u;
    media.startup_bitmap_stage_atlas_width = 96u;
    media.startup_bitmap_soul_room_atlas_width = 96u;
    media.startup_bitmap_forcefield_atlas_width = 96u;
    media.startup_bitmap_atlas.variant = THERON_TRACK02_VARIANT_US_BIN;
    media.startup_bitmap_atlas.route_count = 4u;
    for (i = 0u; i < 4u; ++i) {
        Theron_Track02StartupBitmapAtlasRoute *route =
            &media.startup_bitmap_atlas.routes[i];
        route->route_bit = route_bits[i];
        route->tile_count = 12u;
        route->width = 96u;
        route->height = 8u;
        route->nonzero_pixel_count = 32u;
        route->checksum = (uint32_t)(i + 1u);
        route->first_raw_offset = 0x1000u + i * 0x100u;
        route->last_raw_offset = route->first_raw_offset + 0x1fu;
        route->first_user_data_offset = 0x2000u + i * 0x100u;
        route->pixels[0] = (uint8_t)(i + 1u);
    }
    media.runtime_media_identity.ready = 1;
    media.runtime_media_identity.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    media.runtime_media_identity.bank_descriptor_offset = 0x70be06u;
    media.runtime_media_identity.bank_stride = 0x0400u;
    media.runtime_media_identity.checksum = 0x71a3b1c2u;

    theron_v1_world_init(&world);
    check_int("media gate rejects unbound later semantic level",
              theron_v1_world_runtime_media_select_level_bank(
                  &world, THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL,
                  THERON_DUNGEON_2_DRATOR, 1), 0);
    check_int("media gate accepts complete Track 02 receipt",
              theron_v1_startup_media_bind_runtime_receipt(&world, &media), 1);
    check_int("forcefield transition is media gated",
              theron_v1_world_runtime_media_select_level_bank(
                  &world, THERON_RUNTIME_LEVEL_BANK_STARTUP_FORCEFIELD,
                  THERON_DUNGEON_2_DRATOR, 1), 1);
    check_int("stage transition replaces selected bank",
              theron_v1_world_runtime_media_select_level_bank(
                  &world, THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL,
                  THERON_DUNGEON_2_DRATOR, 1), 1);
    check_int("stage selection retains real-media receipt",
              world.runtime_media.level_bank.real_media_gate, 1);
}

int main(int argc, char **argv) {
    printf("=== Theron V1 Track 02 Level Handoff Probe ===\n");
    printf("%s\n", theron_v1_track02_source_evidence());

    if (argc == 2 && strcmp(argv[1], "--loader-route-only") == 0) {
        probe_real_data_initial_candidate("US raw BIN",
                                          THERON_TRACK02_MD5_US_BIN,
                                          "FIRESTAFF_THERON_US_TRACK02",
                                          "theron/TQUS02.bin");
        probe_real_data_initial_candidate("JP raw BIN",
                                          THERON_TRACK02_MD5_JP_BIN,
                                          "FIRESTAFF_THERON_JP_TRACK02",
                                          "theron/TQJP02.bin");
        printf("summary: fail=%d skip=%d\n", g_fail, g_skip);
        return g_fail ? 1 : 0;
    }

    probe_synthetic_positive_handoff();
    probe_synthetic_initial_candidate_handoff();
    probe_synthetic_initial_candidate_wrong_anchor_rejected();
    probe_split_raw_initial_candidate_semantic_handoff();
    probe_synthetic_initial_candidate_user_data_offsets();
    probe_synthetic_multiple_initial_candidates_rejected();
    probe_initial_level_envelope_rejected_without_corpus();
    probe_negative_handoffs();
    probe_media_gated_level_bank_selection();
    probe_real_data_if_present();

    printf("summary: fail=%d skip=%d\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
