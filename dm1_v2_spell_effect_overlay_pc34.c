#include "dm1_v2_spell_effect_overlay_pc34.h"

static M11_V2_SpellOverlay s_overlay = {0};

void v2_spell_overlay_init(void) {
    memset(&s_overlay, 0, sizeof(s_overlay));
}

void v2_spell_overlay_trigger(M11_V2_SpellVFX type, float speed) {
    s_overlay.type = type;
    s_overlay.speed = speed;
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
