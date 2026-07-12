/*
 * firestaff_dm2_v1_dungeon_loader_first_map_real_data_probe.c
 *
 * Skip-safe real-data sibling for the synthetic
 * test_dm2_v1_dungeon_loader_first_map_gate. It finds the canonical DM2
 * DUNGEON.DAT by MD5 under a user data root, materializes archive-backed
 * matches into a temporary file when needed, and pins the loader's map-0
 * metadata plus a few deterministic byte-square reads.
 *
 * Source-lock:
 *   ReDMCSB DEFS.H lines 989-998: DUNGEON_HEADER.MapCount.
 *   skproject SKWIN/DME.h Map_definitions: map offset and w8 dimensions.
 *   dm2_v1_dungeon_loader.c: DM2 PC G1 preamble, 28 map definitions, and
 *   column-major byte-square access for the first map.
 *
 * Exit code: 0 on PASS or SKIP, 1 on a verified-data regression.
 */

#include "asset_find_by_hash.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_game.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#define firestaff_getpid _getpid
#define FIRESTAFF_PATH_SEP '\\'
#else
#include <unistd.h>
#define firestaff_getpid getpid
#define FIRESTAFF_PATH_SEP '/'
#endif

#define DM2_DUNGEON_MD5 "6caccd7875009e82fe2e28e7f6d6adc0"
#define DM2_CANONICAL_DUNGEON_SIZE 39437
#define DM2_HEADER_SIZE 44
#define DM2_MAP_DESCRIPTOR_SIZE 16
#define DM2_MAP_DESCRIPTOR_COUNT 28
#define DM2_TILE_DATA_START \
    (DM2_HEADER_SIZE + DM2_MAP_DESCRIPTOR_COUNT * DM2_MAP_DESCRIPTOR_SIZE)

static const unsigned char k_skproject_record_sizes[16] = {
    4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
};

static int g_checks;
static int g_failures;

#define CHECK(cond, msg) do {                                          \
    ++g_checks;                                                        \
    if (cond) {                                                        \
        printf("  PASS: %s\n", msg);                                   \
    } else {                                                           \
        ++g_failures;                                                  \
        printf("  FAIL: %s\n", msg);                                   \
    }                                                                  \
} while (0)

static unsigned read16le(const unsigned char *p)
{
    return (unsigned)p[0] | ((unsigned)p[1] << 8);
}

static unsigned g1_declared_record_pool_bytes(const unsigned char *raw)
{
    unsigned total = 0;
    int type;

    for (type = 0; type < 16; ++type)
        total += read16le(raw + 14 + type * 2) * k_skproject_record_sizes[type];
    return total;
}

static int g1_source_ordered_pool_bases_match(
    const unsigned char *raw,
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_G1RecordPoolEvidence *evidence)
{
    unsigned cursor;
    int type;

    if (!raw || !dungeon || !evidence || evidence->candidate_base < 0)
        return 0;
    cursor = (unsigned)evidence->candidate_base;
    for (type = 0; type < 16; ++type) {
        unsigned count = read16le(raw + 14 + type * 2);
        unsigned bytes = count * k_skproject_record_sizes[type];
        int expected_base = (count != 0 &&
                             k_skproject_record_sizes[type] != 0)
                                ? (int)cursor : -1;
        if (dungeon->thing_data_bases[type] != expected_base ||
            evidence->candidate_pool_bases[type] != expected_base) {
            return 0;
        }
        cursor += bytes;
    }
    return cursor == (unsigned)evidence->candidate_end;
}

/* skproject c_map.cpp supplies one ground-stack word for every byte-square
 * with bit 0x10. c_record.cpp dispatches it as type=(link>>10)&0xf and
 * index=link&0x3ff, then adds index * table_recordsizes[type]. These are the
 * only five map-owned roots not covered by the declared pools or the proven
 * DB3/DB4 continuation. The nine bytes after that continuation cannot make
 * five distinct 4-byte addresses; keep this as an exclusion proof, not a
 * synthetic continuation. */
static int g1_unresolved_roots_and_tail_match(
    const unsigned char *raw, const DM2_V1_DungeonData *dungeon)
{
    static const unsigned char expected_tail[9] = {
        0x00, 0xe0, 0x00, 0x00, 0x30, 0x00, 0x00, 0x10, 0x20
    };
    static const struct {
        int map;
        int x;
        int y;
        unsigned stack;
        unsigned root;
        unsigned type;
        unsigned index;
    } expected[] = {
        {16, 10,  1,  872, 0x2818, 10,  24},
        {16, 11, 12,  873, 0x2814, 10,  20},
        {16, 17,  2,  889, 0xe88a, 10, 138},
        {23, 13, 16, 1053, 0x285d, 10,  93},
        {26, 11, 10, 1154, 0xa037,  8,  55}
    };
    unsigned found = 0;
    unsigned column_index = 0;
    int level;

    if (!raw || !dungeon || dungeon->g1_extension_base != 23826 ||
        dungeon->g1_extension_size != 7841 || dungeon->raw_map_data_base != 31667 ||
        memcmp(raw + 31658, expected_tail, sizeof(expected_tail)) != 0) {
        return 0;
    }

    for (level = 0; level < dungeon->level_count; ++level) {
        int x;
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            unsigned stack = read16le(raw + dungeon->column_index_base +
                                      (column_index + (unsigned)x) * 2u);
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                unsigned raw_tile = raw[dungeon->raw_map_data_base +
                                        dungeon->level_offsets[level] +
                                        x * dungeon->level_heights[level] + y];
                unsigned root;
                unsigned type;
                unsigned index;
                int declared;
                int extension;

                if ((raw_tile & 0x10u) == 0) continue;
                root = read16le(raw + dungeon->square_first_thing_base +
                                  stack * 2u);
                type = (root >> 10) & 0x0fu;
                index = root & 0x03ffu;
                declared = type < 16 && index < read16le(raw + 14 + type * 2) &&
                           k_skproject_record_sizes[type] != 0;
                extension = (type == 3 && index >= 299 && index < 1024) ||
                            (type == 4 && index >= 173 && index < 300);
                if (!declared && !extension) {
                    if (found >= sizeof(expected) / sizeof(expected[0]) ||
                        expected[found].map != level || expected[found].x != x ||
                        expected[found].y != y || expected[found].stack != stack ||
                        expected[found].root != root || expected[found].type != type ||
                        expected[found].index != index) {
                        return 0;
                    }
                    ++found;
                }
                ++stack;
            }
        }
        column_index += (unsigned)dungeon->level_widths[level];
    }
    return found == sizeof(expected) / sizeof(expected[0]);
}

/* SkWinCore.cpp _2fcf_0434 (51052-51057) reads a DB1 payload as a movement
 * teleporter only on ttTeleporter (5) tiles. Map 0's 22 direct DB1 roots are
 * deliberately retained as payload evidence, but the canonical corpus must
 * prove they do not fabricate a map transition from ordinary cells. */
static int g1_map0_db1_roots_are_not_teleporter_tiles(
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_G1FirstMapRuntimeReceipt *receipt)
{
    int roots = 0;

    if (!dungeon || !receipt || receipt->map != 0 ||
        receipt->root_count != receipt->teleporter_root_count) {
        return 0;
    }
    for (int i = 0; i < receipt->root_count; ++i) {
        const DM2_V1_G1VerifiedRoot *root = &receipt->roots[i];
        if (root->type != 1 ||
            dm2_v1_dungeon_get_square_type(dungeon, 0, root->x, root->y) == 5) {
            return 0;
        }
        ++roots;
    }
    return roots == 22;
}

static const char *dm2_data_root(int argc, char **argv,
                                 char fallback[ASSET_PATH_MAX])
{
    const char *env;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') return argv[1];

    env = getenv("FIRESTAFF_DM2_V1_DATA_DIR");
    if (env && env[0] != '\0') return env;

    env = getenv("FIRESTAFF_DM2_CANONICAL_DIR");
    if (env && env[0] != '\0') return env;

    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') return env;

    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(fallback, ASSET_PATH_MAX, "%s%c.firestaff%cdata",
             home, FIRESTAFF_PATH_SEP, FIRESTAFF_PATH_SEP);
    return fallback;
}

static const char *temp_dir(void)
{
    const char *dir = getenv("TMPDIR");
    if (dir && dir[0] != '\0') return dir;
#ifdef _WIN32
    dir = getenv("TEMP");
    if (dir && dir[0] != '\0') return dir;
    return ".";
#else
    return "/tmp";
#endif
}

static int resolve_materialized_dungeon(const char *data_root,
                                        char found[ASSET_PATH_MAX],
                                        char materialized[ASSET_PATH_MAX],
                                        int *needs_cleanup)
{
    *needs_cleanup = 0;
    materialized[0] = '\0';
    found[0] = '\0';

    if (!asset_find_by_md5(data_root, DM2_DUNGEON_MD5,
                           found, ASSET_PATH_MAX, 32)) {
        printf("SKIP: no hash-verified DM2 DUNGEON.DAT under %s\n",
               data_root ? data_root : "(null)");
        return 0;
    }

    if (!strstr(found, "::")) {
        snprintf(materialized, ASSET_PATH_MAX, "%s", found);
        return 1;
    }

    snprintf(materialized, ASSET_PATH_MAX,
             "%s%cfirestaff_dm2_v1_first_map_%ld.dat",
             temp_dir(), FIRESTAFF_PATH_SEP, (long)firestaff_getpid());
    if (!asset_extract_virtual_path(found, materialized)) {
        printf("SKIP: matched %s but could not materialize it; "
               "ZIP inflate may be unavailable in this build\n", found);
        materialized[0] = '\0';
        return 0;
    }

    *needs_cleanup = 1;
    return 1;
}

static unsigned char *read_file(const char *path, int *out_size)
{
    FILE *fp;
    long size;
    unsigned char *buf;
    size_t got;

    *out_size = 0;
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    size = ftell(fp);
    if (size <= 0 || size > 1024L * 1024L) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    buf = (unsigned char *)malloc((size_t)size);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    got = fread(buf, 1U, (size_t)size, fp);
    fclose(fp);
    if (got != (size_t)size) {
        free(buf);
        return NULL;
    }
    *out_size = (int)size;
    return buf;
}

static void probe_first_map(const unsigned char *raw, int size)
{
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1RecordPoolEvidence evidence;
    DM2_V1_G1PartialMapBootReceipt partial_boot;
    DM2_V1_G1FirstMapRuntimeReceipt first_map_runtime;
    int load_rc;
    unsigned declared_pool_bytes;

    CHECK(size == DM2_CANONICAL_DUNGEON_SIZE,
          "canonical DM2 DUNGEON.DAT size is 39437 bytes");
    CHECK(size >= DM2_TILE_DATA_START,
          "file contains the G1 header plus 28 map descriptors");
    CHECK(read16le(raw + 2) == 0x3147U,
          "G1 marker is present at bytes 2..3");
    CHECK(read16le(raw + 4) == DM2_HEADER_SIZE,
          "DM2 header-size field is 44 bytes");
    CHECK(raw[6] == DM2_MAP_DESCRIPTOR_COUNT,
          "map count byte is 28");
    CHECK(read16le(raw + DM2_HEADER_SIZE) == 0U,
          "map-0 RawMapDataByteOffset is zero");
    CHECK((((read16le(raw + DM2_HEADER_SIZE + 8) >> 6) & 0x1fU) + 1U) == 7U,
          "map-0 skproject width is 7");
    CHECK((((read16le(raw + DM2_HEADER_SIZE + 8) >> 11) & 0x1fU) + 1U) == 10U,
          "map-0 skproject height is 10");

    load_rc = dm2_v1_dungeon_load(&dungeon, raw, size);
    CHECK(load_rc == 0, "dm2_v1_dungeon_load accepts verified real data");
    if (load_rc != 0) return;

    declared_pool_bytes = g1_declared_record_pool_bytes(raw);

    CHECK(dungeon.level_count == DM2_MAP_DESCRIPTOR_COUNT,
          "loader preserves the 28-level map count");
    CHECK(dungeon.level_offsets[0] == 0,
          "loader preserves map-0 tile-data offset");
    CHECK(dungeon.square_bytes == 1,
          "loader selects the PC G1 byte-square layout");
    CHECK(dungeon.raw_map_data_base == 31667,
          "loader starts byte-square data at the trailing PC G1 map-data block");
    CHECK(dungeon.text_word_count == 257,
          "loader preserves PC G1 shifted File_header.cwTextData");
    CHECK(dungeon.square_first_thing_count == 2360,
          "loader preserves the PC G1 ground-stack capacity");
    CHECK(dungeon.thing_type_counts[0] == 217 &&
              dungeon.thing_type_counts[1] == 576 &&
              dungeon.thing_type_counts[2] == 1020 &&
              dungeon.thing_type_counts[3] == 299,
          "loader preserves leading PC G1 DB record counts");
    CHECK(dungeon.column_index_base == 748 &&
              dungeon.square_first_thing_base == 1708 &&
              dungeon.text_data_base == 6428,
          "loader exposes proven G1 c_map column and ground-stack tables");
    CHECK(dungeon.thing_data_bases[0] == 6942 &&
              dungeon.thing_data_bases[1] == 7810 &&
              dungeon.thing_data_bases[2] == 11266 &&
              dungeon.thing_data_bases[3] == 15346,
          "loader maps PC G1 c_record pools from the text-adjacent source order");
    CHECK(dungeon.g1_extension_base == 23826 &&
              dungeon.g1_extension_size == 7841 &&
              dungeon.g1_extension_base + dungeon.g1_extension_size ==
                  dungeon.raw_map_data_base,
          "loader bounds the remaining untyped G1 pre-map segment");
    CHECK(declared_pool_bytes == 16884U &&
              (unsigned)(dungeon.raw_map_data_base - dungeon.g1_extension_base) ==
                  7841U && declared_pool_bytes !=
                  (unsigned)(dungeon.raw_map_data_base - dungeon.g1_extension_base),
          "real G1 rejects the tempting map-tail DB-pool placement by size");
    CHECK(dm2_v1_dungeon_collect_g1_record_pool_evidence(
              &dungeon, &evidence) == 1 && evidence.available == 1,
          "real G1 produces a bounded non-tail record-pool evidence receipt");
    CHECK(evidence.candidate_base == 6942 && evidence.candidate_end == 23826 &&
              evidence.candidate_bytes == (int)declared_pool_bytes &&
              evidence.candidate_end == dungeon.g1_extension_base,
          "c_record span is anchored after text data, before the untyped G1 extension");
    CHECK(g1_source_ordered_pool_bases_match(raw, &dungeon, &evidence),
          "all sixteen c_record pools retain skproject source order and bounds");
    CHECK(dm2_v1_dungeon_validate_record_pools(&dungeon) == 1,
          "G1 c_record pool locator validates for real map boot");
    CHECK(evidence.tail_pool_base == 14783 && evidence.tail_pool_base_rejected,
          "tail-aligned pool base remains rejected by the non-tail text anchor");
    CHECK(evidence.root_count == dungeon.square_first_thing_count &&
              evidence.root_end_markers + evidence.root_shape_valid +
                  evidence.root_shape_invalid == evidence.root_count &&
              evidence.candidate_record_count > 0 &&
              evidence.candidate_first_link_end_markers +
                  evidence.candidate_first_link_shape_valid +
                  evidence.candidate_first_link_shape_invalid ==
                      evidence.candidate_record_count,
          "real G1 root and c_record first-word shapes are fully accounted");
    CHECK(evidence.root_shape_invalid == 1069 &&
              evidence.candidate_first_link_shape_invalid == 1029,
          "real G1 pins every observed non-direct root and c_record link");
    CHECK(evidence.map_root_count == 883 &&
              evidence.map_root_end_markers == 0 &&
              evidence.map_root_null_markers == 0 &&
              evidence.map_root_shape_valid == 676 &&
              evidence.map_root_extension_shape_valid == 202 &&
              evidence.map_root_shape_invalid == 5 &&
              evidence.map_root_unresolved_after_extension == 5,
          "map-owned G1 roots separate declared, DB3/DB4-extension, and still-unresolved ObjectIDs");
    CHECK(dungeon.g1_extension_record_bases[3] == 23826 &&
              dungeon.g1_extension_record_counts[3] == 725 &&
              dungeon.g1_extension_record_bases[4] == 29626 &&
              dungeon.g1_extension_record_counts[4] == 127,
          "G1 extension follows the proven DB3-to-ObjectID-limit then DB4 record transform");
    CHECK(evidence.map_root_extension_by_type[3] == 174 &&
              evidence.map_root_extension_by_type[4] == 28 &&
              evidence.map_root_unresolved_by_type[8] == 1 &&
              evidence.map_root_unresolved_by_type[10] == 4,
          "G1 raw-root type clusters retain only DB8/DB10 as unresolved families");
    CHECK(g1_unresolved_roots_and_tail_match(raw, &dungeon),
          "raw G1 proves five distinct DB8/DB10 roots cannot fit the nine-byte tail");
    CHECK(dm2_v1_dungeon_materialize_g1_partial_map_boot(
              &dungeon, &partial_boot) == 1 &&
              partial_boot.committed == 1 && partial_boot.incomplete == 1 &&
              dungeon.partial_map_boot.committed == 1 &&
              dungeon.partial_map_boot.materialized_root_count == 878 &&
              partial_boot.map_root_count == 883 &&
              partial_boot.direct_root_count == 676 &&
              partial_boot.db3_root_count == 174 &&
              partial_boot.db4_root_count == 28 &&
              partial_boot.materialized_root_count == 878 &&
              partial_boot.blocked_root_count == 5,
          "real G1 commits an incomplete map boot for only the 878 proven roots");
    CHECK(partial_boot.blocked_roots[0].map == 16 &&
              partial_boot.blocked_roots[0].x == 10 &&
              partial_boot.blocked_roots[0].y == 1 &&
              partial_boot.blocked_roots[0].type == 10 &&
              partial_boot.blocked_roots[4].map == 26 &&
              partial_boot.blocked_roots[4].type == 8 &&
              partial_boot.blocked_root_count_by_map[16] == 3 &&
              partial_boot.blocked_root_count_by_map[23] == 1 &&
              partial_boot.blocked_root_count_by_map[26] == 1,
          "partial boot receipt preserves only the five blocked source roots");
    CHECK(dm2_v1_dungeon_materialize_g1_first_map_runtime(
              &dungeon, &first_map_runtime) == 1 &&
              first_map_runtime.committed == 1 &&
              first_map_runtime.incomplete_world == 1 &&
              first_map_runtime.map == 0 &&
              first_map_runtime.verified_root_count ==
                  first_map_runtime.root_count &&
              first_map_runtime.blocked_root_count == 0 &&
              first_map_runtime.object_count == 0 &&
              first_map_runtime.root_count == 22 &&
              first_map_runtime.direct_root_count == 22 &&
              first_map_runtime.teleporter_root_count == 22 &&
              first_map_runtime.teleporter_record_reads == 22 &&
              first_map_runtime.blocked_record_reads == 0 &&
              first_map_runtime.roots[0].x == 0 &&
              first_map_runtime.roots[0].y == 4 &&
              first_map_runtime.roots[0].object_id == 0x04a5 &&
              first_map_runtime.roots[0].type == 1 &&
              first_map_runtime.roots[0].index == 165 &&
              first_map_runtime.teleporters[0].x == 0 &&
              first_map_runtime.teleporters[0].y == 4 &&
              first_map_runtime.teleporters[0].object_id == 0x04a5 &&
              first_map_runtime.teleporters[0].index == 165 &&
              first_map_runtime.teleporters[0].destination_x == 10 &&
              first_map_runtime.teleporters[0].destination_y == 14 &&
              first_map_runtime.teleporters[0].destination_map == 255 &&
              first_map_runtime.teleporters[0].scope == 0 &&
              first_map_runtime.teleporters[0].sound == 1 &&
              first_map_runtime.teleporters[0].rotation == 2 &&
              first_map_runtime.teleporters[0].rotation_type == 0 &&
              first_map_runtime.teleporters[21].object_id == 0x04da &&
              first_map_runtime.teleporters[21].x == 6 &&
              first_map_runtime.teleporters[21].y == 7 &&
              first_map_runtime.teleporters[21].destination_x == 30 &&
              first_map_runtime.teleporters[21].destination_y == 31 &&
              first_map_runtime.teleporters[21].destination_map == 2,
          "first-map receipt reads only source-defined direct DB1 teleporter fields");
    CHECK(g1_map0_db1_roots_are_not_teleporter_tiles(
              &dungeon, &first_map_runtime),
          "canonical map-0 DB1 roots cannot enter the source teleporter transition path");
    {
        DM2_V1_BootProfile profile;
        DM2_V1_GameState game_state;
        DM2_V1_G1FirstMapRuntimeReceipt runtime_receipt;
        DM2_V1_G1TeleporterTransitionReceipt transition_receipt;
        DM2_V1_ViewportState viewport;
        uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];

        dm2_v1_boot_profile_init(&profile);
        memset(&game_state, 0, sizeof(game_state));
        profile.dm2_state = &game_state;
        profile.dungeon_data = &dungeon;
        CHECK(dm2_v1_runtime_bind_boot_profile(&profile) == 1 &&
                  dm2_v1_runtime_g1_first_map_receipt(&runtime_receipt) == 1 &&
                  runtime_receipt.committed == 1 &&
                  runtime_receipt.incomplete_world == 1 &&
                  runtime_receipt.verified_root_count ==
                      first_map_runtime.verified_root_count &&
                  runtime_receipt.roots[0].object_id == 0x04a5 &&
                  runtime_receipt.object_count == 0 &&
                  runtime_receipt.teleporter_root_count == 22 &&
                  runtime_receipt.teleporter_record_reads == 22 &&
                  runtime_receipt.teleporters[0].destination_map == 255 &&
                  runtime_receipt.blocked_record_reads == 0,
              "runtime retains the bounded teleporter incomplete-world receipt");
        dm2_v1_runtime_set_position(0, 0, 4, 0);
        CHECK(dm2_v1_runtime_g1_map0_teleporter_transition_receipt(
                  &transition_receipt) == 1 &&
                  transition_receipt.committed == 1 &&
                  transition_receipt.incomplete_world == 1 &&
                  transition_receipt.source_map == 0 &&
                  transition_receipt.source_x == 0 &&
                  transition_receipt.source_y == 4 &&
                  transition_receipt.source_object_id == 0x04a5 &&
                  transition_receipt.source_index == 165 &&
                  transition_receipt.destination_x == 10 &&
                  transition_receipt.destination_y == 14 &&
                  transition_receipt.destination_map == 255 &&
                  transition_receipt.scope == 0 &&
                  transition_receipt.sound == 1 &&
                  transition_receipt.rotation == 2 &&
                  transition_receipt.rotation_type == 0 &&
                  transition_receipt.generic_record_reads == 0 &&
                  transition_receipt.blocked_record_reads == 0 &&
                  transition_receipt.destination_map_valid == 0 &&
                  transition_receipt.resolved_destination_map == -1 &&
                  transition_receipt.transition_applied == 0 &&
                  transition_receipt.no_transition_reason ==
                      DM2_V1_G1_TELEPORT_NO_TRANSITION_INCOMPLETE_WORLD &&
                  game_state.current_level == 0 && game_state.party_x == 0 &&
                  game_state.party_y == 4 && game_state.party_dir == 0,
              "runtime retains a strict no-transition receipt for incomplete canonical G1");
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_g1_first_map_runtime(&viewport, &runtime_receipt);
        dm2_v1_viewport_set_g1_map0_teleporter_transition(
            &viewport, &transition_receipt);
        CHECK(viewport.g1_first_map_runtime.committed == 1 &&
                  viewport.g1_first_map_runtime.incomplete_world == 1 &&
                  viewport.g1_first_map_runtime.verified_root_count ==
                      runtime_receipt.verified_root_count &&
                  viewport.g1_first_map_runtime.roots[0].object_id == 0x04a5 &&
                  viewport.g1_first_map_runtime.object_count == 0 &&
                  viewport.g1_first_map_runtime.teleporter_root_count == 22 &&
                  viewport.g1_first_map_runtime.teleporter_record_reads == 22 &&
                  viewport.g1_first_map_runtime.teleporters[21].destination_map == 2 &&
                  viewport.g1_first_map_runtime.blocked_record_reads == 0,
              "viewport receives the bounded teleporter receipt without objects");
        CHECK(viewport.g1_map0_teleporter_transition.committed == 1 &&
                  viewport.g1_map0_teleporter_transition.source_object_id ==
                      0x04a5 &&
                  viewport.g1_map0_teleporter_transition.destination_map == 255 &&
                  viewport.g1_map0_teleporter_transition.rotation == 2 &&
                  viewport.g1_map0_teleporter_transition.generic_record_reads == 0 &&
                  viewport.g1_map0_teleporter_transition.blocked_record_reads == 0 &&
                  viewport.g1_map0_teleporter_transition.transition_applied == 0 &&
                  viewport.g1_map0_teleporter_transition.destination_map_valid == 0 &&
                  viewport.g1_map0_teleporter_transition.no_transition_reason ==
                      DM2_V1_G1_TELEPORT_NO_TRANSITION_INCOMPLETE_WORLD,
              "viewport receives the canonical strict no-transition receipt without traversal");
    }
    CHECK(evidence.map_root_extension_by_map[16] == 13 &&
              evidence.map_root_unresolved_by_map[16] == 3 &&
              evidence.map_root_extension_by_map[23] == 4 &&
              evidence.map_root_unresolved_by_map[23] == 1 &&
              evidence.map_root_extension_by_map[26] == 5 &&
              evidence.map_root_unresolved_by_map[26] == 1,
          "G1 map ownership cluster pins every remaining raw-root family to its source map");
    printf("  INFO: G1 candidate [%d,%d), roots end=%d valid=%d invalid=%d, "
           "map roots end=%d null=%d declared=%d extension=%d unresolved=%d, "
           "extension DB3=%d DB4=%d; unresolved DB8=%d DB10=%d, "
           "records end=%d valid=%d invalid=%d\n",
           evidence.candidate_base, evidence.candidate_end,
           evidence.root_end_markers, evidence.root_shape_valid,
           evidence.root_shape_invalid,
           evidence.map_root_end_markers, evidence.map_root_null_markers,
           evidence.map_root_shape_valid,
           evidence.map_root_extension_shape_valid,
           evidence.map_root_shape_invalid,
           evidence.map_root_extension_by_type[3],
           evidence.map_root_extension_by_type[4],
           evidence.map_root_unresolved_by_type[8],
           evidence.map_root_unresolved_by_type[10],
           evidence.candidate_first_link_end_markers,
           evidence.candidate_first_link_shape_valid,
           evidence.candidate_first_link_shape_invalid);
    CHECK(dungeon.record_graph_complete == 0 &&
              dm2_v1_dungeon_validate_record_graph(&dungeon) == 0,
          "PC G1 blocks map-reachable traversal when a real w0 link is invalid");
    CHECK(dungeon.level_widths[0] == 7,
          "loader reports map-0 width from Map_definitions.w8");
    CHECK(dungeon.level_heights[0] == 10,
          "loader reports map-0 height from Map_definitions.w8");
    CHECK(dm2_v1_dungeon_is_outdoor(&dungeon, 0) == 1,
          "map 0 is classified as outdoor");
    CHECK(dungeon.raw_size == size && dungeon.raw_data != NULL,
          "loader retains raw dungeon bytes for tile lookups");

    CHECK(dm2_v1_dungeon_get_tile_raw(&dungeon, 0, 0, 0) == 0x20,
          "map-0 tile(0,0) raw byte is stable");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 0, 0) == 1,
          "map-0 tile(0,0) type comes from high three bits");
    CHECK(dm2_v1_dungeon_get_tile_raw(&dungeon, 0, 0, 1) == 0x20,
          "map-0 tile(0,1) raw byte is stable");
    CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 4) == 0x04a5,
          "map-0 c_map link resolves through the proven G1 ground stack");
    CHECK(dm2_v1_dungeon_get_thing_record(&dungeon, 0x04a5,
                                           NULL, NULL, NULL) != NULL,
          "map-0 root resolves through the source-ordered c_record pool");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x04a5) == -1,
          "unvalidated PC G1 c_record links cannot enter traversal");
    CHECK(dm2_v1_dungeon_get_tile_raw(&dungeon, 0, 1, 0) == 0x00,
          "map-0 tile(1,0) confirms byte column-major stepping");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 1, 0) == 0,
          "map-0 tile(1,0) type comes from high three bits");
    CHECK(dm2_v1_dungeon_get_tile_raw(&dungeon, 0, 6, 9) == 0x20,
          "map-0 tile(6,9) last in-bounds raw byte is stable");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 7, 0) == -1,
          "map-0 x=7 is rejected");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 0, 10) == -1,
          "map-0 y=10 is rejected");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 28, 0, 0) == -1,
          "level 28 is rejected");

    dm2_v1_dungeon_free(&dungeon);
}

int main(int argc, char **argv)
{
    char fallback[ASSET_PATH_MAX];
    char found[ASSET_PATH_MAX];
    char materialized[ASSET_PATH_MAX];
    const char *root = dm2_data_root(argc, argv, fallback);
    int cleanup = 0;
    int size = 0;
    unsigned char *raw;

    printf("=== DM2 V1 Dungeon Loader First-Map Real-Data Probe ===\n");
    printf("Source: ReDMCSB DEFS.H DUNGEON_HEADER/MAP, "
           "SKULL.ASM T560/T520 boundary via dm2_v1_dungeon_loader.c\n");

    if (!root || root[0] == '\0') {
        puts("SKIP: no data root configured");
        return 0;
    }
    printf("Data root: %s\n", root);

    if (!resolve_materialized_dungeon(root, found, materialized, &cleanup)) {
        return 0;
    }
    printf("Matched dungeon: %s\n", found);

    raw = read_file(materialized, &size);
    if (!raw) {
        printf("SKIP: could not read materialized dungeon %s\n", materialized);
        if (cleanup) remove(materialized);
        return 0;
    }

    probe_first_map(raw, size);

    free(raw);
    if (cleanup) remove(materialized);

    printf("\nCHECKS: %d\nFAILED: %d\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
