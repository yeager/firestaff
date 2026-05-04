#ifndef FIRESTAFF_DM1_V2_PARTICLE_EMITTER_PRESETS_PC34_H
#define FIRESTAFF_DM1_V2_PARTICLE_EMITTER_PRESETS_PC34_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TORCH_FLAME,
    TORCH_SMOKE,
    SPELL_FIREBALL,
    SPELL_POISON,
    BLOOD_SPLAT,
    WATER_DRIP,
    DUST_PUFF,
    MAGIC_SPARKLE,
    M11_V2_EmitterPreset_COUNT
} M11_V2_EmitterPreset;

typedef struct {
    float rate;
    float spread;
    float life;
    uint32_t color;
    float size;
    float gravity;
    int count;
} M11_V2_EmitterConfig;

void v2_emitter_preset_init(void);
M11_V2_EmitterConfig v2_emitter_preset_get(M11_V2_EmitterPreset preset);
int v2_emitter_preset_count(void);

#ifdef __cplusplus
}
#endif

#endif
