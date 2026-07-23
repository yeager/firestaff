/*
 * Real-data-only DM2 G1 world-model handoff test.
 *
 * Invoke with the hash-verified PC G1 DUNGEON.DAT path.  This test never
 * creates a fixture because the retained record-pool boundary is useful only
 * when it matches the original 39,437-byte PC corpus.
 */

#include "dm2_v1_world_model.h"

#include <stdio.h>
#include <stdlib.h>

#define DM2_G1_CANONICAL_SIZE 39437L

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long size;
    unsigned char *bytes;

    *out_size = 0;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) != DM2_G1_CANONICAL_SIZE ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static const char *resolve_dungeon_dat_path(int argc, char **argv,
                                            char *buf, size_t buf_size)
{
    const char *root;
    const char *home;
    if (argc >= 2) return argv[1];
    root = getenv("FIRESTAFF_DM2_DATA_DIR");
    if (root && root[0]) {
        snprintf(buf, buf_size, "%s/dungeon.dat", root);
        return buf;
    }
    home = getenv("HOME");
    if (home && home[0]) {
        snprintf(buf, buf_size, "%s/.firestaff/data/dm2/data/dungeon.dat", home);
        return buf;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    char path_buf[1024];
    const char *path = resolve_dungeon_dat_path(argc, argv, path_buf, sizeof(path_buf));
    unsigned char *bytes;
    size_t size;
    dm2_dungeon_world_t *world;
    const DM2_V1_DungeonData *source;

    if (!path) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    bytes = read_file(path, &size);
    if (!bytes || bytes[2] != 0x47 || bytes[3] != 0x31 || bytes[6] != 28) {
        free(bytes);
        fputs("FAIL: not the canonical PC G1 DUNGEON.DAT corpus\n", stderr);
        return 1;
    }

    world = dm2_world_from_mem(bytes, size);
    free(bytes);
    source = dm2_world_get_verified_g1_map_source(world);
    if (!world || !source || !dm2_world_has_verified_g1_record_pools(world) ||
        source->raw_size != DM2_G1_CANONICAL_SIZE ||
        source->raw_map_data_base != 31667 || source->text_data_base != 6428 ||
        source->thing_data_bases[0] != 6942 ||
        source->g1_extension_base != 23826 ||
        source->partial_map_boot.committed != 1 ||
        source->partial_map_boot.incomplete != 1 ||
        source->partial_map_boot.materialized_root_count != 878 ||
        world->g1_record_graph_complete != 0) {
        dm2_world_free(world);
        fputs("FAIL: G1 source handoff promoted or lost unverified record state\n",
              stderr);
        return 1;
    }

    dm2_world_free(world);
    puts("PASS: retained verified G1 map/c_record address source without traversal");
    return 0;
}
