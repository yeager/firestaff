#include "dm1_v2_spell_effect_overlay_pc34.h"

static M11_V2_SpellOverlay s_overlay = {0};

void v2_spell_overlay_init(void) {
    memset(&s_overlay, 0, sizeof(s_overlay));
}

void v2_spell_overlay_trigger(M11_V2_SpellVFX type, float speed) {
    s_overlay.type = type;
    s_overlay.speed = speed > 0.0f ? speed : 1.0f;
    s_overlay.progress = 0.0f;
    s_overlay.active = true;

    switch(type) {
        case VFX_FIREBALL_BURST: s_overlay.color = 220; s_overlay.intensity = 200; break;
        case VFX_POISON_CLOUD:   s_overlay.color = 140; s_overlay.intensity = 180; break;
        case VFX_LIGHTNING_BOLT: s_overlay.color = 255; s_overlay.intensity = 255; break;
        case VFX_HEAL_GLOW:      s_overlay.color = 100; s_overlay.intensity = 160; break;
        case VFX_FREEZE_FLASH:   s_overlay.color = 200; s_overlay.intensity = 220; break;
        case VFX_DARKNESS:       s_overlay.color = 0;   s_overlay.intensity = 255; break;
        case VFX_SHIELD_PULSE:   s_overlay.color = 180; s_overlay.intensity = 190; break;
        case VFX_DISPEL_WAVE:    s_overlay.color = 120; s_overlay.intensity = 170; break;
        default: s_overlay.color = 128; s_overlay.intensity = 128; break;
    }
}

/*
 * DM1 explosion thing -> V2 overlay family binding.
 * Source audit:
 * - ReDMCSB PROJEXPL.C F0213_EXPLOSION_Create stores Explosion->Type as
 *   thing - C0xFF80_THING_FIRST_EXPLOSION (lines 149-150).
 * - PROJEXPL.C lines 151-166 distinguish damaging explosion, smoke/spell
 *   sound, Fireball (0xFF80), Lightning Bolt (0xFF82), Poison Bolt/Cloud
 *   families used by impact handling.
 * - Existing Firestaff V1 rendering keeps explosion graphic families in
 *   dm1_v1_projectile_explosion_render_pc34_compat.c
 *   dm1_v1_explosion_pattern_graphic_index().
 *
 * V2 only chooses the overlay family here; it does not mutate V1 projectile,
 * damage, timeline, or spell-cast behavior.
 */
#define DM1_THING_EXPLOSION_FIREBALL       ((int16_t)0xFF80)
#define DM1_THING_EXPLOSION_SLIME          ((int16_t)0xFF81)
#define DM1_THING_EXPLOSION_LIGHTNING_BOLT ((int16_t)0xFF82)
#define DM1_THING_EXPLOSION_POISON_BOLT    ((int16_t)0xFF86)
#define DM1_THING_EXPLOSION_POISON_CLOUD   ((int16_t)0xFF87)
#define DM1_THING_EXPLOSION_SMOKE          ((int16_t)0xFFA8)

bool v2_spell_overlay_type_for_dm1_explosion_thing(int16_t dm1ExplosionThing, M11_V2_SpellVFX* outType) {
    M11_V2_SpellVFX mapped;
    switch ((uint16_t)dm1ExplosionThing) {
        case 0xFF80u: mapped = VFX_FIREBALL_BURST; break;
        case 0xFF82u: mapped = VFX_LIGHTNING_BOLT; break;
        case 0xFF86u:
        case 0xFF87u:
        case 0xFFA8u: mapped = VFX_POISON_CLOUD; break;
        case 0xFF81u: mapped = VFX_FREEZE_FLASH; break;
        default: return false;
    }
    if (outType) *outType = mapped;
    return true;
}

bool v2_spell_overlay_trigger_dm1_explosion_thing(int16_t dm1ExplosionThing, float speed) {
    M11_V2_SpellVFX type;
    if (!v2_spell_overlay_type_for_dm1_explosion_thing(dm1ExplosionThing, &type)) return false;
    v2_spell_overlay_trigger(type, speed);
    return true;
}

M11_V2_SpellOverlay v2_spell_overlay_snapshot(void) {
    return s_overlay;
}

void v2_spell_overlay_update(float dt) {
    if (!s_overlay.active) return;
    s_overlay.progress += s_overlay.speed * dt;
    if (s_overlay.progress >= 1.0f) {
        s_overlay.progress = 1.0f;
        s_overlay.active = false;
    }
}

void v2_spell_overlay_render(uint8_t* fb, int w, int h) {
    if (!fb || !s_overlay.active) return;

    float p = s_overlay.progress;
    uint8_t base = s_overlay.color;
    uint8_t intensity = s_overlay.intensity;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float factor = 1.0f;
            float nx = (float)x / (float)w;
            float ny = (float)y / (float)h;

            switch(s_overlay.type) {
                case VFX_FIREBALL_BURST:
                    factor = 1.0f - (nx - 0.5f)*(nx - 0.5f) - (ny - 0.5f)*(ny - 0.5f);
                    if (factor < 0.0f) factor = 0.0f;
                    break;
                case VFX_POISON_CLOUD:
                    factor = 0.5f + 0.5f * sinf(nx * 10.0f + p * 5.0f) * sinf(ny * 10.0f + p * 3.0f);
                    break;
                case VFX_LIGHTNING_BOLT:
                    factor = (fmodf(nx * 20.0f + p * 10.0f, 2.0f) < 0.1f) ? 1.0f : 0.0f;
                    break;
                case VFX_HEAL_GLOW:
                    factor = 1.0f - p;
                    break;
                case VFX_FREEZE_FLASH:
                    factor = (p < 0.1f || p > 0.9f) ? 1.0f : 0.2f;
                    break;
                case VFX_DARKNESS:
                    factor = p;
                    break;
                case VFX_SHIELD_PULSE:
                    factor = 0.5f + 0.5f * sinf(p * 6.28318f);
                    break;
                case VFX_DISPEL_WAVE:
                    factor = 0.5f + 0.5f * sinf((nx + ny) * 15.0f - p * 10.0f);
                    break;
                default:
                    factor = 1.0f;
                    break;
            }

            uint8_t val = (uint8_t)(base * (intensity / 255.0f) * factor * p);
            fb[y * w + x] = val;
        }
    }
}

bool v2_spell_overlay_is_active(void) {
    return s_overlay.active;
}

void v2_spell_overlay_cancel(void) {
    s_overlay.active = false;
    s_overlay.progress = 0.0f;
}
