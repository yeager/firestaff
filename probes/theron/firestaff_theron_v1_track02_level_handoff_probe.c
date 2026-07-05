/*
 * firestaff_theron_v1_track02_level_handoff_probe.c
 *
 * Theron's Quest V1 -- bounded Track 02 descriptor-window to level-loader
 * handoff gate.
 *
 * Scope:
 *   This probe proves a narrow API bridge: a decoded Track 02 descriptor
 *   table window can be selected, classified as DATA, and handed to the
 *   existing theron_v1_level_load() parser.  Synthetic fixtures provide the
 *   only positive level load.  Real Track 02 fixtures, when staged locally,
 *   are allowed to report "not a level yet"; that is the current honest
 *   state until the dungeon record format is known.
 */

#include "asset_status_m12.h"
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
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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
    check_int("synthetic initial candidate expected offset ok",
              theron_v1_track02_initial_candidate_expected_offset(
                  descriptor_offset,
                  &expected_candidate_offset),
              1);
    check_size("synthetic initial candidate expected offset",
               expected_candidate_offset,
               candidate_offset);

    status = theron_v1_track02_load_initial_level_candidate(
        track,
        sizeof(track),
        descriptor_offset,
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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
    check_int("synthetic initial candidate start dir", level.start_dir, 1);

    candidate[4] = 0x11u; /* corrupt the source-locked seed gate */
    status = theron_v1_track02_load_initial_level_candidate(
        track,
        sizeof(track),
        descriptor_offset,
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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

    status = theron_v1_track02_load_initial_level_candidate(
        track,
        sizeof(track),
        descriptor_offset,
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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
    Theron_Track02LevelHandoffStatus status;
    size_t expected_candidate_offset = 0u;

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

    status = theron_v1_track02_load_initial_level_candidate(
        data,
        size,
        signal.descriptor_offsets[0],
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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
    check_int("real initial candidate loaded", handoff.loaded, 1);
    check_int("real initial candidate level width", level.width, 32);
    check_int("real initial candidate level height", level.height, 27);
    check_int("real initial candidate start x", level.start_x, 2);
    check_int("real initial candidate start y", level.start_y, 1);
    check_int("real initial candidate start dir", level.start_dir, 1);
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
                    THERON_DUNGEON_1_HALL_OF_RECORDS,
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

int main(void) {
    printf("=== Theron V1 Track 02 Level Handoff Probe ===\n");
    printf("%s\n", theron_v1_track02_source_evidence());

    probe_synthetic_positive_handoff();
    probe_synthetic_initial_candidate_handoff();
    probe_synthetic_initial_candidate_wrong_anchor_rejected();
    probe_negative_handoffs();
    probe_real_data_if_present();

    printf("summary: fail=%d skip=%d\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
