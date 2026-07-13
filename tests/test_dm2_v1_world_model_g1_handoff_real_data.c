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

int main(int argc, char **argv)
{
    unsigned char *bytes;
    size_t size;
    dm2_dungeon_world_t *world;
    const DM2_V1_DungeonData *source;

    if (argc != 2) {
        fputs("usage: test_dm2_v1_world_model_g1_handoff_real_data DUNGEON.DAT\n",
              stderr);
        return 2;
    }
    bytes = read_file(argv[1], &size);
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
