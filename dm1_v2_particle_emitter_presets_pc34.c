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
