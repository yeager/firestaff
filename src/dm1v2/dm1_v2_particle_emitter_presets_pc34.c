#include "dm1_v2_particle_system_pc34.h"
/* Forward declaration for cross-module call */
extern int v2_particle_emitter_create(float x, float y, float rate,
    float spread, float life, float size, float gravity,
    unsigned int color, int max_count);

#include "dm1_v2_particle_emitter_presets_pc34.h"

void v2_emitter_preset_init(void) {
    /* No source-owned DM1 particle emitter exists. */
}

M11_V2_EmitterConfig v2_emitter_preset_get(M11_V2_EmitterPreset preset) {
    (void)preset;
    M11_V2_EmitterConfig cfg = {0.0f, 0.0f, 0.0f, 0x00000000, 0.0f, 0.0f, 0};
    return cfg;
}

int v2_emitter_preset_count(void) {
    return M11_V2_EmitterPreset_COUNT;
}


const char *v2_emitter_preset_name(M11_V2_EmitterPreset preset) {
    switch (preset) {
        case TORCH_FLAME: return "Torch Flame";
        case TORCH_SMOKE: return "Torch Smoke";
        case SPELL_FIREBALL: return "Spell Fireball";
        case SPELL_POISON: return "Spell Poison Cloud";
        case BLOOD_SPLAT: return "Blood Splat";
        case WATER_DRIP: return "Water Drip";
        case DUST_PUFF: return "Dust Puff";
        case MAGIC_SPARKLE: return "Magic Sparkle";
        default: return "Unknown";
    }
}

int v2_emitter_preset_validate(const M11_V2_EmitterConfig *cfg) {
    if (!cfg) return 0;
    if (cfg->rate < 0.0f || cfg->rate > 1000.0f) return 0;
    if (cfg->life <= 0.0f) return 0;
    if (cfg->count <= 0 || cfg->count > 10000) return 0;
    if (cfg->size <= 0.0f) return 0;
    return 1;
}

/* V2.2 Emitter Presets — create emitter from preset with position */

typedef enum {
    V22_BLEND_ADDITIVE = 0,
    V22_BLEND_ALPHA,
    V22_BLEND_MULTIPLY,
} V22_BlendMode;

static V22_BlendMode g_preset_blend[M11_V2_EmitterPreset_COUNT] = {
    V22_BLEND_ADDITIVE,  /* TORCH_FLAME */
    V22_BLEND_ALPHA,     /* TORCH_SMOKE */
    V22_BLEND_ADDITIVE,  /* SPELL_FIREBALL */
    V22_BLEND_ALPHA,     /* SPELL_POISON */
    V22_BLEND_ALPHA,     /* BLOOD_SPLAT */
    V22_BLEND_ALPHA,     /* WATER_DRIP */
    V22_BLEND_ALPHA,     /* DUST_PUFF */
    V22_BLEND_ADDITIVE,  /* MAGIC_SPARKLE */
};

V22_BlendMode v22_emitter_preset_blend(M11_V2_EmitterPreset preset) {
    if (preset < 0 || preset >= M11_V2_EmitterPreset_COUNT)
        return V22_BLEND_ALPHA;
    return g_preset_blend[preset];
}

int v22_emitter_create_from_preset(M11_V2_EmitterPreset preset,
    float x, float y)
{
    (void)preset; (void)x; (void)y;
    return -1;
}
