/* Real-data bounded direct c_record chain test for canonical PC G1. */

#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, int *out_size)
{
    FILE *file = fopen(path, "rb");
    long size;
    unsigned char *bytes;

    *out_size = 0;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) != 39437L || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (int)size;
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

static int expect_node(const DM2_V1_G1DirectRootChainReceipt *receipt, int row,
                       uint16_t object_id, int type, int index,
                       int offset, int size)
{
    const DM2_V1_G1DirectChainNode *node = &receipt->nodes[row];
    return node->object_id == object_id && node->type == type &&
           node->index == index && node->record_offset == offset &&
           node->record_size == size;
}

int main(int argc, char **argv)
{
    char path_buf[1024];
    const char *path = resolve_dungeon_dat_path(argc, argv, path_buf, sizeof(path_buf));
    unsigned char *bytes = NULL;
    int size;
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1DirectRootChainReceipt chain;
    DM2_V1_G1DirectRootChainReceipt sentinel;

    if (!path || !(bytes = read_file(path, &size))) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    if (bytes[2] != 0x47 || bytes[3] != 0x31 || bytes[6] != 28 ||
        dm2_v1_dungeon_load(&dungeon, bytes, size) != 0) {
        free(bytes);
        fputs("FAIL: canonical G1 input was not accepted\n", stderr);
        return 1;
    }
    free(bytes);
    if (!dm2_v1_dungeon_collect_g1_direct_root_chain(
            &dungeon, 0, 0, 8, &chain) ||
        chain.committed != 1 || chain.incomplete_world != 1 ||
        chain.node_count != 2 || chain.link_word_reads != 2 ||
        !expect_node(&chain, 0, 0x04b0, 1, 176, 8866, 6) ||
        !expect_node(&chain, 1, 0x000a, 0, 10, 6982, 4)) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct DB1-to-DB0 chain changed\n", stderr);
        return 1;
    }
    if (!dm2_v1_dungeon_collect_g1_direct_root_chain(
            &dungeon, 17, 5, 8, &chain) ||
        chain.node_count != 4 || chain.link_word_reads != 4 ||
        !expect_node(&chain, 0, 0xd407, 5, 7, 20534, 4) ||
        !expect_node(&chain, 1, 0x0400, 1, 0, 7810, 6) ||
        !expect_node(&chain, 2, 0x0c6d, 3, 109, 16218, 8) ||
        !expect_node(&chain, 3, 0x0030, 0, 48, 7134, 4)) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct DB5 chain changed\n", stderr);
        return 1;
    }
    sentinel.committed = -1;
    if (dm2_v1_dungeon_collect_g1_direct_root_chain(
            &dungeon, 9, 7, 3, &sentinel) != 0 || sentinel.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: unknown next link mutated direct chain receipt\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: direct G1 c_record chains stay bounded and fail closed");
    return 0;
}
