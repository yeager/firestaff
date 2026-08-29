/*
 * Real-data-only DM2 G1 world-model handoff test.
 *
 * Invoke with the hash-verified PC G1 DUNGEON.DAT path.  This test never
 * creates a fixture because the canonical File_header record-pool handoff is
 * useful only when it matches the original 39,437-byte PC corpus.
 */

#include "dm2_v1_world_model.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>

#define DM2_G1_CANONICAL_SIZE 39437L

static unsigned char *read_source(const char *path, size_t *out_size)
{
    unsigned char *bytes;

    *out_size = 0;
    bytes = NULL;
    if (!asset_read_path_alloc(path, &bytes, out_size) ||
        !bytes || *out_size != (size_t)DM2_G1_CANONICAL_SIZE) {
        free(bytes);
        return NULL;
    }
    return bytes;
}

static const char *resolve_dungeon_dat_path(int argc, char **argv,
                                            char *buf, size_t buf_size)
{
    const char *archive;
    const char *root;
    const char *home;
    if (argc >= 2) return argv[1];
    archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    if (archive && archive[0]) {
        snprintf(buf, buf_size, "%s::data/dungeon.dat", archive);
        return buf;
    }
    root = getenv("FIRESTAFF_DM2_DATA_DIR");
    if (root && root[0]) {
        snprintf(buf, buf_size, "%s/dungeon.dat", root);
        return buf;
    }
    home = getenv("HOME");
    if (home && home[0]) {
        snprintf(buf, buf_size,
                 "%s/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip::data/dungeon.dat",
                 home);
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
    const DM2_V1_RecordPoolSet *pools;

    if (!path) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    bytes = read_source(path, &size);
    if (!bytes || bytes[2] != 0x47 || bytes[3] != 0x31 || bytes[6] != 28) {
        free(bytes);
        fputs("FAIL: not the canonical PC G1 DUNGEON.DAT corpus\n", stderr);
        return 1;
    }

    world = dm2_world_from_mem(bytes, size);
    free(bytes);
    source = dm2_world_get_verified_g1_map_source(world);
    pools = dm2_world_get_record_pools(world);
    if (!world || !source || !dm2_world_has_verified_g1_record_pools(world) ||
        source->raw_size != DM2_G1_CANONICAL_SIZE ||
        !source->record_graph_complete ||
        !world->g1_record_graph_complete ||
        !pools || !pools->valid || !pools->record_graph_complete) {
        dm2_world_free(world);
        fputs("FAIL: canonical G1 source did not retain its verified record pools\n",
              stderr);
        return 1;
    }

    dm2_world_free(world);
    puts("PASS: canonical G1 source retained verified File_header record pools");
    return 0;
}
