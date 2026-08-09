#include "dm1_v2_creature_animation_pc34.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_SOURCE_DIR
#define FIRESTAFF_SOURCE_DIR "."
#endif

static int file_contains(const char* rel, const char* needle) {
    char path[2048];
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
    require(!v2_creature_anim_is_playing(42), "unbound V2 animation is blocked");
    require(v2_creature_anim_get_sprite(42) == -1, "no synthetic sprite index");
    v2_creature_anim_update(1.7f);
    require(!v2_creature_anim_is_playing(42), "update cannot start an unbound animation");
    require(v2_creature_anim_get_sprite(42) == -1, "update cannot select a synthetic frame");

    require(file_contains("src/dm1/dm1_v1_creature_render_pc34_compat.c", "G0219"), "V1 creature aspects remain source-locked");
    require(file_contains("src/dm1/dm1_v1_group_active_state_pc34_compat.c", "F0179"), "V1 aspect timing remains source-locked");

    require(file_contains("assets-v2/manifests/firestaff-v2-wave1-creatures.manifest.json", "fs.v2.creature.demon.front-near"), "creature manifest still exposes demon front-near id");
    /* The items-starter manifest was migrated off the invented "fs.v2.*"
     * naming onto real ReDMCSB identifiers; the empty-hand action icon is
     * C201_ICON_ACTION_ICON_EMPTY_HAND (DEFS.H:1952). Assert the source id
     * rather than the superseded synthetic one. The wave1 creature manifests
     * still carry fs.v2.* ids and are checked as such on the line above. */
    require(file_contains("assets-v2/manifests/firestaff-v2-wave1-items-starter.manifest.json", "C201_ICON_ACTION_ICON_EMPTY_HAND"), "item starter manifest contains the source empty-hand icon id");
    require(file_contains("assets-v2/items/wave1/specs/starter-icons.md", "DUNVIEW.C"), "item spec cites DUNVIEW.C object rendering audit");
    return 0;
}
