#ifndef FIRESTAFF_DM1_V2_SPELL_EFFECT_OVERLAY_PC34_H
#define FIRESTAFF_DM1_V2_SPELL_EFFECT_OVERLAY_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

typedef enum {
    VFX_FIREBALL_BURST,
    VFX_POISON_CLOUD,
    VFX_LIGHTNING_BOLT,
    VFX_HEAL_GLOW,
    VFX_FREEZE_FLASH,
    VFX_DARKNESS,
    VFX_SHIELD_PULSE,
    VFX_DISPEL_WAVE
} M11_V2_SpellVFX;

typedef struct {
    M11_V2_SpellVFX type;
    float progress;
    float speed;
    bool active;
    uint8_t color;
    uint8_t intensity;
} M11_V2_SpellOverlay;

void v2_spell_overlay_init(void);
void v2_spell_overlay_trigger(M11_V2_SpellVFX type, float speed);
bool v2_spell_overlay_type_for_dm1_explosion_thing(int16_t dm1ExplosionThing, M11_V2_SpellVFX* outType);
bool v2_spell_overlay_trigger_dm1_explosion_thing(int16_t dm1ExplosionThing, float speed);
M11_V2_SpellOverlay v2_spell_overlay_snapshot(void);
void v2_spell_overlay_update(float dt);
void v2_spell_overlay_render(uint8_t* fb, int w, int h);
bool v2_spell_overlay_is_active(void);
void v2_spell_overlay_cancel(void);

#endif
