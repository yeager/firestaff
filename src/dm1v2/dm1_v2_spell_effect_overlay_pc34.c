#include "dm1_v2_spell_effect_overlay_pc34.h"

static M11_V2_SpellOverlay s_overlay = {0};

void v2_spell_overlay_init(void) {
    memset(&s_overlay, 0, sizeof(s_overlay));
}

void v2_spell_overlay_trigger(M11_V2_SpellVFX type, float speed) {
    (void)type; (void)speed;
    /* DM1 has no source-owned full-screen post-process overlay. */
    memset(&s_overlay, 0, sizeof(s_overlay));
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
    (void)fb; (void)w; (void)h;
    /* No generated full-screen pixels: V1 source effects own the frame. */
}

bool v2_spell_overlay_is_active(void) {
    return s_overlay.active;
}

void v2_spell_overlay_cancel(void) {
    s_overlay.active = false;
    s_overlay.progress = 0.0f;
}
