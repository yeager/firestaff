#include "asset_status_m12.h"
#include "theron_v1_startup_runtime_entry.h"
#include "theron_v1_track02.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *find_jp_track02(void) {
    static const char *const candidates[] = {
        "/home/yeager/.firestaff/data/theron/Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 02).bin",
        NULL
    };
    unsigned int i;
    for (i = 0u; candidates[i]; ++i) {
        FILE *f = fopen(candidates[i], "rb");
        if (f) { fclose(f); return candidates[i]; }
    }
    return NULL;
}

static unsigned char *read_file(const char *path, size_t *out_size) {
    FILE *f;
    long end;
    unsigned char *bytes;
    if (out_size) *out_size = 0u;
    f = fopen(path, "rb");
    if (!f || fseek(f, 0, SEEK_END) != 0 || (end = ftell(f)) <= 0 ||
        fseek(f, 0, SEEK_SET) != 0) {
        if (f) fclose(f);
        return NULL;
    }
    bytes = (unsigned char *)malloc((size_t)end);
    if (!bytes || fread(bytes, 1u, (size_t)end, f) != (size_t)end) {
        free(bytes); fclose(f); return NULL;
    }
    fclose(f);
    if (out_size) *out_size = (size_t)end;
    return bytes;
}

int main(void) {
    const char *path = find_jp_track02();
    unsigned char *track02;
    size_t track02_size;
    Theron_V1_World world;
    char receipt[256];
    char md5[33];

    if (!path) {
        puts("SKIP: authentic Theron JP Track 02 is not staged");
        return 77;
    }
    track02 = read_file(path, &track02_size);
    if (!track02) return 1;
    if (!m12_file_md5_hex(path, md5) ||
        strcmp(md5, THERON_TRACK02_MD5_JP_BIN) != 0) {
        fprintf(stderr, "FAIL: JP Track 02 identity is not authentic: %s\n", md5);
        free(track02);
        return 1;
    }
    theron_v1_world_init(&world);
    memset(receipt, 0, sizeof(receipt));
    if (!theron_v1_startup_runtime_load_source_dungeon(
            &world, track02, track02_size, md5,
            THERON_DUNGEON_2_DRATOR, receipt, sizeof(receipt)) ||
        world.current_dungeon != THERON_DUNGEON_2_DRATOR ||
        world.current_level != 0 || !world.level_loaded[1][0] ||
        world.levels[1][0].width == 0 || world.levels[1][0].height == 0 ||
        world.source_object_count == 0u ||
        !strstr(receipt, "dungeon=2") || !strstr(receipt, "visual capture remains gated")) {
        fprintf(stderr,
                "FAIL: JP Track 02 Drator source-dungeon handoff: %s (dungeon=%d level=%d loaded=%d size=%dx%d objects=%u)\n",
                receipt, world.current_dungeon, world.current_level,
                world.level_loaded[1][0], world.levels[1][0].width,
                world.levels[1][0].height, world.source_object_count);
        free(track02);
        return 1;
    }
    printf("PASS: authentic JP Track 02 binds later Drator dungeon (%u source objects)\n",
           world.source_object_count);
    free(track02);
    return 0;
}
