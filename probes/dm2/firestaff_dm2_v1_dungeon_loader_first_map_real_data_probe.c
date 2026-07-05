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
    int load_rc;

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

    CHECK(dungeon.level_count == DM2_MAP_DESCRIPTOR_COUNT,
          "loader preserves the 28-level map count");
    CHECK(dungeon.level_offsets[0] == 0,
          "loader preserves map-0 tile-data offset");
    CHECK(dungeon.square_bytes == 1,
          "loader selects the PC G1 byte-square layout");
    CHECK(dungeon.raw_map_data_base == 31667,
          "loader starts byte-square data at the trailing PC G1 map-data block");
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
