#include "dm1_v2_spell_effect_overlay_pc34.h"

#include <stdint.h>
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
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        exit(1);
    }
}

int main(void) {
    M11_V2_SpellVFX mapped = VFX_HEAL_GLOW;

    require(v2_spell_overlay_type_for_dm1_explosion_thing((int16_t)0xFF80, &mapped), "fireball maps");
    require(mapped == VFX_FIREBALL_BURST, "0xFF80 -> fireball burst");
    require(v2_spell_overlay_type_for_dm1_explosion_thing((int16_t)0xFF82, &mapped), "lightning maps");
    require(mapped == VFX_LIGHTNING_BOLT, "0xFF82 -> lightning bolt");
    require(v2_spell_overlay_type_for_dm1_explosion_thing((int16_t)0xFF87, &mapped), "poison cloud maps");
    require(mapped == VFX_POISON_CLOUD, "0xFF87 -> poison cloud");
    require(!v2_spell_overlay_type_for_dm1_explosion_thing((int16_t)0x1234, &mapped), "unknown explosion is rejected");

    v2_spell_overlay_init();
    require(v2_spell_overlay_trigger_dm1_explosion_thing((int16_t)0xFF80, 2.0f), "trigger fireball overlay");
    M11_V2_SpellOverlay snap = v2_spell_overlay_snapshot();
    require(snap.active && snap.type == VFX_FIREBALL_BURST, "snapshot reflects active fireball");
    v2_spell_overlay_update(0.25f);
    snap = v2_spell_overlay_snapshot();
    require(snap.active && snap.progress > 0.49f && snap.progress < 0.51f, "progress is speed * dt");

    uint8_t fb[16];
    memset(fb, 0, sizeof(fb));
    v2_spell_overlay_render(fb, 4, 4);
    int nonzero = 0;
    for (size_t i = 0; i < sizeof(fb); ++i) nonzero += fb[i] != 0;
    require(nonzero > 0, "fireball overlay writes visible pixels");

    require(file_contains("assets-v2/manifests/firestaff-v2-wave1-effects-starter.manifest.json", "fs.v2.effect.fireball-burst.overlay"), "effect manifest contains fireball overlay id");
    require(file_contains("assets-v2/effects/wave1/specs/starter-spell-effects.md", "PROJEXPL.C"), "effect spec cites PROJEXPL.C");
    return 0;
}
