/* Development/test-only staging of authenticated original payloads.
 * Runtime uses the native in-memory reader and never writes these files. */
#include "dm2_v1_boot.h"
#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Identity {
    const char* graphics;
    size_t graphics_size;
    const char* dungeon;
    size_t dungeon_size;
} Identity;

static const Identity identities[] = {
    {"25247ede4dabb6a71e5dabdfbcd5907d", 8639757u,
     "6caccd7875009e82fe2e28e7f6d6adc0", 39437u},
    {"5cab25f6b975957eae4a203174e7f2a6", 8157169u,
     "719ae78bc124027806c65491a256827d", 39411u},
    {"027ff3b8ddc2c4c4cdda7ada0b0bc46c", 2783791u,
     "74c7549f174574201988bf936385841a", 37954u},
    {"1c940ea95703eaea0ecdf84d17e954b9", 3493879u,
     "719ae78bc124027806c65491a256827d", 39411u}
};

static int write_original(const char* directory, const char* leaf,
                           const char* source, const unsigned char* owned,
                           size_t owned_size, const char* expected_md5,
                           size_t expected_size) {
    unsigned char* allocated = NULL;
    const unsigned char* bytes = owned;
    size_t size = owned_size;
    char path[FSP_PATH_MAX];
    FILE* output;
    int ok;
    if (!bytes && !asset_read_path_alloc(source, &allocated, &size)) return 0;
    if (!bytes) bytes = allocated;
    if (!bytes || size != expected_size ||
        !FSP_JoinPath(path, sizeof(path), directory, leaf)) {
        free(allocated);
        return 0;
    }
    output = fopen(path, "wbx");
    if (!output) {
        free(allocated);
        return 0;
    }
    ok = fwrite(bytes, 1u, size, output) == size;
    if (fclose(output) != 0) ok = 0;
    free(allocated);
    return ok && asset_file_matches_md5(path, expected_md5);
}

static int inspect_copies(const char* mode, const char* directory,
                            const Identity* identity) {
    char (*paths)[ASSET_PATH_MAX];
    char graphics[512], dungeon[512];
    int result;
    paths = calloc(256u, sizeof(*paths));
    if (!paths) return 1;
    if (strcmp(mode, "--orphan-order") == 0) {
        const char* hashes[3] = {identity->graphics, identity->graphics, NULL};
        int matched[2] = {0};
        result = asset_find_all_files_by_md5_list(directory, hashes, paths,
                                                  matched, 2, 8);
        if (result == 2 && matched[0] && matched[1] &&
            strcmp(paths[0], paths[1]) != 0) {
            printf("%s\n%s\n", paths[0], paths[1]);
            free(paths);
            return 0;
        }
    } else {
        result = asset_collect_files_by_md5(directory, identity->graphics,
                                              paths, 256u, 8);
        if (result == -2) {
            size_t i;
            for (i = 0u; i < 256u; ++i) {
                if (paths[i][0] != '\0') break;
            }
            if (i == 256u && dm2_v1_boot_find_loose_pair(directory,
                    identity->graphics, 8, graphics, dungeon) == -2 &&
                graphics[0] == '\0' && dungeon[0] == '\0') {
                free(paths);
                return 0;
            }
        }
    }
    free(paths);
    return 1;
}

int main(int argc, char** argv) {
    DM2_V1_BootProfile profile;
    unsigned index;
    const Identity* identity;
    int ok;
    if (argc != 4) return 2;
    if (strlen(argv[3]) != 1u || argv[3][0] < '0' || argv[3][0] > '3') return 2;
    index = (unsigned)(argv[3][0] - '0');
    identity = &identities[index];
    if (strcmp(argv[1], "--orphan-order") == 0 ||
        strcmp(argv[1], "--capacity") == 0) {
        return inspect_copies(argv[1], argv[2], identity);
    }
    if (!FSP_FileExists(argv[1])) return 77;
    dm2_v1_boot_profile_init(&profile);
    ok = dm2_v1_boot_scan_assets(&profile, argv[1]) == 0 &&
        profile.assets_verified &&
        strcmp(profile.graphics_md5, identity->graphics) == 0 &&
        strcmp(profile.dungeon_md5, identity->dungeon) == 0 &&
        profile.graphics_size == identity->graphics_size &&
        profile.dungeon_size == identity->dungeon_size;
    if (ok) {
        ok = write_original(argv[2], "Maze-art.dat", profile.graphics_path,
                            profile.graphics_mem, profile.graphics_mem_size,
                            identity->graphics, identity->graphics_size) &&
             write_original(argv[2], "Level-layout.dat", profile.dungeon_path,
                            profile.dungeon_mem, profile.dungeon_mem_size,
                            identity->dungeon, identity->dungeon_size);
    }
    dm2_v1_boot_cleanup(&profile);
    if (!ok) fprintf(stderr, "FAIL: original DM2 pair staging\n");
    return ok ? 0 : 1;
}
