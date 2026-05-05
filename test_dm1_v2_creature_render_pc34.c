#include "dm1_v2_creature_animation_pc34.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_SOURCE_DIR
#define FIRESTAFF_SOURCE_DIR "."
#endif

static int file_contains(const char* rel, const char* needle) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", FIRESTAFF_SOURCE_DIR, rel);
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char* data = (char*)malloc((size_t)size + 1u);
    if (!data) { fclose(f); return 0; }
    size_t got = fread(data, 1, (size_t)size, f);
    fclose(f);
    data[got] = '\0';
    int ok = strstr(data, needle) != NULL;
    free(data);
    return ok;
}

static void require(int cond, const char* msg) {
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); exit(1); }
}

int main(void) {
    M11_V2_AnimFrame frames[18];
    for (int i = 0; i < 18; ++i) {
        frames[i].sprite_idx = 1000 + i;
        frames[i].duration = 0.1f;
    }

    v2_creature_anim_init();
    v2_creature_anim_define(CANIM_WALK, frames, 18, true);
    v2_creature_anim_play(42, CANIM_WALK);
    require(v2_creature_anim_is_playing(42), "walk animation starts");
    require(v2_creature_anim_get_sprite(42) == 1000, "first frame selected");
    v2_creature_anim_update(1.7f);
    require(v2_creature_anim_is_playing(42), "looping clamped 16-frame animation remains active");
    require(v2_creature_anim_get_sprite(42) == 1001, "define clamps to 16 frames and wraps safely");

    require(file_contains("assets-v2/manifests/firestaff-v2-wave1-creatures.manifest.json", "fs.v2.creature.demon.front-near"), "creature manifest still exposes demon front-near id");
    require(file_contains("assets-v2/manifests/firestaff-v2-wave1-items-starter.manifest.json", "fs.v2.item.starter.empty-hand"), "item starter manifest contains empty-hand id");
    require(file_contains("assets-v2/items/wave1/specs/starter-icons.md", "DUNVIEW.C"), "item spec cites DUNVIEW.C object rendering audit");
    return 0;
}
