#include "dm1_v2_particle_system_pc34.h"
#include "dm1_v2_particle_emitter_presets_pc34.h"

void v2_emitter_preset_init(void) {
    // Placeholder for future internal state initialization
}

M11_V2_EmitterConfig v2_emitter_preset_get(M11_V2_EmitterPreset preset) {
    M11_V2_EmitterConfig cfg = {0.0f, 0.0f, 0.0f, 0x00000000, 0.0f, 0.0f, 0};
    switch (preset) {
        case TORCH_FLAME:
            cfg.rate = 15.0f; cfg.spread = 0.5f; cfg.life = 0.8f; cfg.color = 0xFFAA00FF; cfg.size = 3.0f; cfg.gravity = -2.0f; cfg.count = 50; break;
        case TORCH_SMOKE:
            cfg.rate = 5.0f; cfg.spread = 0.3f; cfg.life = 1.5f; cfg.color = 0x888888FF; cfg.size = 5.0f; cfg.gravity = -1.0f; cfg.count = 30; break;
        case SPELL_FIREBALL:
            cfg.rate = 40.0f; cfg.spread = 1.0f; cfg.life = 0.5f; cfg.color = 0xFFFF00FF; cfg.size = 4.0f; cfg.gravity = 0.0f; cfg.count = 100; break;
        case SPELL_POISON:
            cfg.rate = 10.0f; cfg.spread = 0.8f; cfg.life = 2.0f; cfg.color = 0x00FF00FF; cfg.size = 2.0f; cfg.gravity = 1.5f; cfg.count = 40; break;
        case BLOOD_SPLAT:
            cfg.rate = 60.0f; cfg.spread = 1.5f; cfg.life = 0.3f; cfg.color = 0xFF0000FF; cfg.size = 2.5f; cfg.gravity = 5.0f; cfg.count = 80; break;
        case WATER_DRIP:
            cfg.rate = 2.0f; cfg.spread = 0.1f; cfg.life = 1.2f; cfg.color = 0x0088FFFF; cfg.size = 1.5f; cfg.gravity = 8.0f; cfg.count = 10; break;
        case DUST_PUFF:
            cfg.rate = 20.0f; cfg.spread = 0.6f; cfg.life = 0.6f; cfg.color = 0xCCBBAAFF; cfg.size = 3.5f; cfg.gravity = -0.5f; cfg.count = 35; break;
        case MAGIC_SPARKLE:
            cfg.rate = 25.0f; cfg.spread = 0.4f; cfg.life = 1.0f; cfg.color = 0xFF00FFFF; cfg.size = 1.0f; cfg.gravity = -1.5f; cfg.count = 60; break;
        default:
            break;
    }
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
    M11_V2_EmitterConfig cfg = v2_emitter_preset_get(preset);
    return v2_particle_emitter_create(x, y, cfg.rate, cfg.spread,
        cfg.life, cfg.size, cfg.gravity, cfg.color, cfg.count);
}

