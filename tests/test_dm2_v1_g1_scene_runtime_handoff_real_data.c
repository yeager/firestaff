/* Canonical-PC-G1 runtime handoff test; GDAT pixels stay callback-owned. */

#include "dm2_v1_g1_scene_runtime_bridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int resolve_calls;
    int fetch_calls;
    int palette_calls;
    int missing_material;
    int last_index;
} Calls;

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

static int resolve_material(void *user, DM2_V1_G1SceneTileClass tile,
                            DM2_V1_G1SceneRootClass root, int *out_index)
{
    Calls *calls = (Calls *)user;
    ++calls->resolve_calls;
    if (root != DM2_V1_G1_SCENE_ROOT_GENERIC ||
        (tile != DM2_V1_G1_SCENE_TILE_WALL &&
         tile != DM2_V1_G1_SCENE_TILE_FLOOR)) return 0;
    *out_index = tile == DM2_V1_G1_SCENE_TILE_WALL ? 901 : 902;
    return 1;
}

static int fetch_material(void *user, int index, const uint8_t **out_pixels,
                          int *out_width, int *out_height, int *out_stride)
{
    static const uint8_t receipt_only_pixel[] = { 0x2a };
    Calls *calls = (Calls *)user;
    ++calls->fetch_calls;
    if (calls->missing_material || index == 0) return -1;
    calls->last_index = index;
    *out_pixels = receipt_only_pixel;
    *out_width = 1;
    *out_height = 1;
    *out_stride = 1;
    return 0;
}

static int fetch_palette(void *user, int index, uint8_t out_palette16[16],
                         uint32_t *out_hash)
{
    Calls *calls = (Calls *)user;
    ++calls->palette_calls;
    if (calls->missing_material || index == 0) return -1;
    memset(out_palette16, 0, 16u);
    out_palette16[1] = 0x2a;
    *out_hash = (uint32_t)(0x47315300u + (unsigned)index);
    return 0;
}

int main(int argc, char **argv)
{
    char path_buf[1024];
    const char *path = resolve_dungeon_dat_path(argc, argv, path_buf, sizeof(path_buf));
    unsigned char *bytes = NULL;
    int size;
    Calls calls;
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1SceneRuntimeHandoffReceipt receipt;
    DM2_V1_G1SceneRuntimeHandoffReceipt sentinel;

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
    memset(&calls, 0, sizeof(calls));
    if (!dm2_v1_g1_scene_runtime_handoff(
            &dungeon, 5, 12, 0, resolve_material, &calls, fetch_material,
            &calls, fetch_palette, &calls, &receipt) || !receipt.valid ||
        receipt.blocked || receipt.scene.tile_class != DM2_V1_G1_SCENE_TILE_WALL ||
        receipt.gdat_index != 901 || receipt.material_palette_hash == 0u) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: real wall route did not bind its material receipt\n", stderr);
        return 1;
    }
    if (!receipt.material_pixels || receipt.material_palette16[1] != 0x2a) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: handoff did not retain its decoded GDAT material\n", stderr);
        return 1;
    }
    if (!dm2_v1_g1_scene_runtime_handoff(
            &dungeon, 0, 0, 4, resolve_material, &calls, fetch_material,
            &calls, fetch_palette, &calls, &receipt) || !receipt.valid ||
        receipt.scene.tile_class != DM2_V1_G1_SCENE_TILE_FLOOR ||
        receipt.gdat_index != 902) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: real floor route did not bind its material receipt\n", stderr);
        return 1;
    }
    if (dm2_v1_g1_scene_runtime_handoff(
            &dungeon, 3, 14, 12, resolve_material, &calls, fetch_material,
            &calls, fetch_palette, &calls, &receipt) != 0 || !receipt.blocked ||
        receipt.scene.tile_class != DM2_V1_G1_SCENE_TILE_DOOR) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: door route did not fail closed without DB0 material proof\n", stderr);
        return 1;
    }
    if (!dm2_v1_g1_scene_runtime_handoff(
            &dungeon, 5, 10, 5, resolve_material, &calls, fetch_material,
            &calls, fetch_palette, &calls, &receipt) || !receipt.valid ||
        receipt.scene.root_class != DM2_V1_G1_SCENE_ROOT_CREATURE ||
        receipt.creature_type != 192 ||
        receipt.gdat_index != DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE -
            (192 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT) ||
        calls.last_index != receipt.gdat_index) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: creature route did not bind DB4 b4 to CREATURES/F9\n", stderr);
        return 1;
    }
    calls.missing_material = 1;
    if (dm2_v1_g1_scene_runtime_handoff(
            &dungeon, 0, 0, 4, resolve_material, &calls, fetch_material,
            &calls, fetch_palette, &calls, &receipt) != 0 || !receipt.blocked) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: missing GDAT material did not block runtime handoff\n", stderr);
        return 1;
    }
    sentinel.valid = -1;
    if (dm2_v1_g1_scene_runtime_handoff(
            &dungeon, 9, 7, 3, resolve_material, &calls, fetch_material,
            &calls, fetch_palette, &calls, &sentinel) != 0 || sentinel.valid != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: malformed chain mutated runtime handoff receipt\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: G1 scene handoff binds only available GDAT material receipts");
    return 0;
}
