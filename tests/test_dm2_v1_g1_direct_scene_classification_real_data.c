/* Real-data source-classification test for canonical PC G1 dungeon scenes. */

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

static int expect_scene(const DM2_V1_G1DungeonSceneClassificationReceipt *scene,
                        DM2_V1_G1SceneTileClass tile,
                        DM2_V1_G1SceneRootClass root,
                        uint16_t first_object)
{
    return scene->committed == 1 && scene->incomplete_world == 1 &&
           scene->tile_class == tile && scene->root_class == root &&
           scene->chain.committed == 1 && scene->chain.node_count > 0 &&
           scene->chain.nodes[0].object_id == first_object;
}

int main(int argc, char **argv)
{
    char path_buf[1024];
    const char *path = resolve_dungeon_dat_path(argc, argv, path_buf, sizeof(path_buf));
    unsigned char *bytes = NULL;
    int size;
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1DungeonSceneClassificationReceipt scene;
    DM2_V1_G1DungeonSceneClassificationReceipt sentinel;

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

    if (!dm2_v1_dungeon_classify_g1_direct_root_scene(
            &dungeon, 5, 12, 0, &scene) ||
        !expect_scene(&scene, DM2_V1_G1_SCENE_TILE_WALL,
                      DM2_V1_G1_SCENE_ROOT_GENERIC, 0x0c6f)) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: wall scene classification changed\n", stderr);
        return 1;
    }
    if (!dm2_v1_dungeon_classify_g1_direct_root_scene(
            &dungeon, 0, 0, 4, &scene) ||
        !expect_scene(&scene, DM2_V1_G1_SCENE_TILE_FLOOR,
                      DM2_V1_G1_SCENE_ROOT_GENERIC, 0x04a5)) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: floor scene classification changed\n", stderr);
        return 1;
    }
    if (!dm2_v1_dungeon_classify_g1_direct_root_scene(
            &dungeon, 3, 14, 12, &scene) ||
        !expect_scene(&scene, DM2_V1_G1_SCENE_TILE_DOOR,
                      DM2_V1_G1_SCENE_ROOT_GENERIC, 0x05a9)) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: door tile scene classification changed\n", stderr);
        return 1;
    }
    if (!dm2_v1_dungeon_classify_g1_direct_root_scene(
            &dungeon, 5, 10, 5, &scene) ||
        !expect_scene(&scene, DM2_V1_G1_SCENE_TILE_FLOOR,
                      DM2_V1_G1_SCENE_ROOT_CREATURE, 0x1003)) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: creature root scene classification changed\n", stderr);
        return 1;
    }
    sentinel.committed = -1;
    if (dm2_v1_dungeon_classify_g1_direct_root_scene(
            &dungeon, 9, 7, 3, &sentinel) != 0 || sentinel.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: malformed chain mutated scene receipt\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: direct G1 scenes keep source tile and root classification");
    return 0;
}
