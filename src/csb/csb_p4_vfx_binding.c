#include "csb_p4_lighting_metadata.h"
#include "csb_v2_vfx_particles.h"
#include "csb_v2_lighting_dynamic.h"
#include "csb_v2_phase_gate_pc34.h"

#include <string.h>

/* ================================================================
 * CSB Phase 4 — Field / Projectile VFX Binding Gates
 * ================================================================
 *
 * Phase gate: CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION and
 * CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION.
 *
 * This module binds spell/projectile metadata from csb_p4_lighting_metadata
 * to the csb_v2_vfx_particles particle system and the csb_v2_lighting_dynamic
 * light-event system.
 *
 * Presentation-only. V1 projectile hit detection, DSA script execution,
 * and chaos magic trigger logic are unaffected.
 *
 * Source-lock anchors:
 * - CSBWin/Graphics.cpp — fireball, lightning, explosion rendering
 * - CSBWin/Chaos.cpp    — spell cast visual triggers
 * - ReDMCSB CASTER.C:1-103 F0394_MENUS_SetMagicCasterAndDrawSpellArea
 * - ReDMCSB ANIM.C:20   G3567_as_AnimationItems[200]
 * ================================================================ */

/* ---- Static state ---- */

#define CSB_P4_BINDING_MAX_ACTIVE_FIELDS 32
#define CSB_P4_BINDING_MAX_ACTIVE_PROJECTILES 8

typedef struct {
    int spell_id;
    int emitter_index;   /* -1 if no emitter, just a field */
    int field_index;
    uint8_t active;
} CSB_P4_BoundField;

typedef struct {
    int spell_id;
    int projectile_index;
    int field_on_hit_index;
    uint8_t active;
} CSB_P4_BoundProjectile;

static CSB_P4_BoundField  g_bound_fields[CSB_P4_BINDING_MAX_ACTIVE_FIELDS];
static CSB_P4_BoundProjectile g_bound_projectiles[CSB_P4_BINDING_MAX_ACTIVE_PROJECTILES];
static int g_binding_initialized = 0;

/* ---- Internal helpers ---- */

static void binding_ensure_init(void) {
    if (!g_binding_initialized) {
        csb_v2_vfx_init();
        memset(g_bound_fields, 0, sizeof(g_bound_fields));
        memset(g_bound_projectiles, 0, sizeof(g_bound_projectiles));
        g_binding_initialized = 1;
    }
}

static CSB_P4_BoundField *binding_alloc_field(void) {
    int i;
    for (i = 0; i < CSB_P4_BINDING_MAX_ACTIVE_FIELDS; i++) {
        if (!g_bound_fields[i].active) {
            g_bound_fields[i].active = 1;
            return &g_bound_fields[i];
        }
    }
    return 0;
}

static CSB_P4_BoundProjectile *binding_alloc_projectile(void) {
    int i;
    for (i = 0; i < CSB_P4_BINDING_MAX_ACTIVE_PROJECTILES; i++) {
        if (!g_bound_projectiles[i].active) {
            g_bound_projectiles[i].active = 1;
            return &g_bound_projectiles[i];
        }
    }
    return 0;
}

/* ---- Public API ---- */

void csb_p4_binding_init(void) {
    binding_ensure_init();
}

/* Fire a spell projectile from (sx,sy) toward (tx,ty).
 * Looks up the spell metadata and delegates to csb_v2_vfx_fire_projectile.
 * Also spawns an emitter at the origin tile for glow/trail.
 * Returns binding index or -1 on failure. */
int csb_p4_binding_fire_projectile(
    const CSB_V2_PhaseGateConfig *cfg,
    int spell_id,
    float sx, float sy,
    float tx, float ty)
{
    const CSB_P4_SpellProjectileMetadata *meta;

    if (!csb_p4_vfx_gate_projectile_enabled(cfg)) {
        return -1;
    }

    binding_ensure_init();

    meta = csb_p4_get_spell_projectile_metadata(spell_id);
    if (!meta || meta->vfx_type == CSB_V2_VFX_NONE) {
        return -1;
    }

    CSB_P4_BoundProjectile *bp = binding_alloc_projectile();
    if (!bp) {
        return -1;
    }

    /* Fire the projectile VFX */
    int vfx_idx = csb_v2_vfx_fire_projectile(
        sx, sy, tx, ty,
        (float)meta->speed_tiles_per_sec,
        meta->vfx_type);

    if (vfx_idx < 0) {
        bp->active = 0;
        return -1;
    }

    bp->spell_id = spell_id;
    bp->projectile_index = vfx_idx;
    bp->field_on_hit_index = -1;
    bp->active = 1;

    /* Spawn a trail/glow emitter at the origin if the spell has a trail */
    if (meta->has_trail) {
        csb_v2_vfx_add_emitter(
            sx, sy,
            (float)meta->light_radius * 0.5f,
            8.0f,
            meta->vfx_type,
            0,    /* not looping */
            0.4f  /* duration seconds */
        );
    }

    return (int)(bp - g_bound_projectiles);
}

/* Place a field VFX on dungeon tile (tx,ty) triggered by spell_id.
 * Returns binding index or -1. */
int csb_p4_binding_add_field(
    const CSB_V2_PhaseGateConfig *cfg,
    int spell_id,
    int tile_x, int tile_y)
{
    const CSB_P4_SpellProjectileMetadata *meta;

    if (!csb_p4_vfx_gate_field_enabled(cfg)) {
        return -1;
    }

    binding_ensure_init();

    meta = csb_p4_get_spell_projectile_metadata(spell_id);
    if (!meta || meta->vfx_type == CSB_V2_VFX_NONE) {
        return -1;
    }

    CSB_P4_BoundField *bf = binding_alloc_field();
    if (!bf) {
        return -1;
    }

    int field_idx = csb_v2_vfx_add_field(tile_x, tile_y, meta->vfx_type);
    if (field_idx < 0) {
        bf->active = 0;
        return -1;
    }

    bf->spell_id = spell_id;
    bf->field_index = field_idx;
    bf->emitter_index = -1;

    /* Also spawn an emitter at the field tile if it has area effect */
    if (meta->light_radius > 2) {
        int emit_idx = csb_v2_vfx_add_emitter(
            (float)tile_x + 0.5f,
            (float)tile_y + 0.5f,
            (float)meta->light_radius * 0.4f,
            4.0f,
            meta->vfx_type,
            1,   /* looping */
            0.0f /* infinite */
        );
        bf->emitter_index = emit_idx;
    }

    return (int)(bf - g_bound_fields);
}

/* Trigger chaos magic visual enhancement.
 * Fires the appropriate light event (pulse/surge) based on spell family.
 * Mirrors csb_v2_chaos_on_trigger but gated on the phase config. */
void csb_p4_binding_trigger_chaos(
    const CSB_V2_PhaseGateConfig *cfg,
    int spell_id)
{
    CSB_P4_SpellCategory cat;
    const CSB_P4_SpellProjectileMetadata *meta;

    if (!csb_p4_vfx_gate_chaos_enabled(cfg)) {
        return;
    }

    meta = csb_p4_get_spell_projectile_metadata(spell_id);
    if (!meta) {
        return;
    }

    cat = meta->category;

    /* Trigger a light event based on spell category.
     * Families 1-2 (fire/ice) → magical pulse.
     * Family 6 (chaos) → chaos surge.
     * Others → no event (V1 handles the spell display). */
    switch (cat) {
        case CSB_P4_SPELL_CAT_FIRE:
        case CSB_P4_SPELL_CAT_ICE:
        case CSB_P4_SPELL_CAT_LIGHTNING:
        case CSB_P4_SPELL_CAT_MAGICAL:
            csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_MAGICAL_PULSE,
                                        0.6f, 0.5f);
            break;
        case CSB_P4_SPELL_CAT_CHAOS:
            csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_CHAOS_SURGE,
                                        1.0f, 0.7f);
            break;
        case CSB_P4_SPELL_CAT_DARKNESS:
            csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_DARKNESS_BURST,
                                        0.8f, 0.4f);
            break;
        default:
            break;
    }
}

/* Add a torch light source to the dynamic lighting system.
 * x,y = dungeon tile coords; torch_type = CSB_P4_TORCH_TYPE_*;
 * Returns light source index or -1. */
int csb_p4_binding_add_torch_light(
    const CSB_V2_PhaseGateConfig *cfg,
    float x, float y,
    int torch_type)
{
    if (!csb_p4_vfx_gate_field_enabled(cfg)) {
        return -1;
    }

    binding_ensure_init();

    uint8_t intensity = csb_p4_torch_type_to_intensity(torch_type);
    if (intensity == 0) {
        return -1;
    }

    /* Torch colour: warm orange-white, varies slightly by torch type */
    uint8_t r, g, b;
    switch (torch_type) {
        case CSB_P4_TORCH_TYPE_NORMAL:
            r = 200; g = 140; b = 40;
            break;
        case CSB_P4_TORCH_TYPE_BRIGHT:
            r = 230; g = 180; b = 80;
            break;
        case CSB_P4_TORCH_TYPE_MAGICAL:
            r = 255; g = 220; b = 150;
            break;
        default:
            r = 180; g = 120; b = 30;
            break;
    }

    /* Torch light radius: 2.0 normal, 3.0 bright, 4.0 magical */
    float radius;
    switch (torch_type) {
        case CSB_P4_TORCH_TYPE_BRIGHT:  radius = 3.0f; break;
        case CSB_P4_TORCH_TYPE_MAGICAL: radius = 4.0f; break;
        default:                         radius = 2.0f; break;
    }

    return csb_v2_light_add_source(x, y, radius, intensity, r, g, b, 1);
}

/* Tick all active bindings.
 * Removes stale field/projectile entries. */
void csb_p4_binding_tick(float dt_seconds) {
    int i;

    binding_ensure_init();
    csb_v2_vfx_tick(dt_seconds);

    /* Expire bound projectiles whose VFX is done */
    for (i = 0; i < CSB_P4_BINDING_MAX_ACTIVE_PROJECTILES; i++) {
        CSB_P4_BoundProjectile *bp = &g_bound_projectiles[i];
        if (!bp->active) {
            continue;
        }
        float px, py;
        int ptype;
        uint8_t palpha;
        if (!csb_v2_vfx_get_projectile(bp->projectile_index,
                                        &px, &py, &ptype, &palpha)) {
            /* Projectile VFX has finished — check if it had field-on-hit */
            const CSB_P4_SpellProjectileMetadata *meta =
                csb_p4_get_spell_projectile_metadata(bp->spell_id);
            if (meta && meta->has_field_on_hit &&
                meta->field_vfx_type != CSB_V2_VFX_NONE) {
                /* Field-on-hit was requested but we can't retroactively
                 * place it here without target coordinates.
                 * The caller of fire_projectile is responsible for
                 * calling add_field at the target coordinates after
                 * the projectile resolves. */
            }
            bp->active = 0;
        }
    }

    /* Expire bound fields whose VFX is done */
    for (i = 0; i < CSB_P4_BINDING_MAX_ACTIVE_FIELDS; i++) {
        CSB_P4_BoundField *bf = &g_bound_fields[i];
        if (!bf->active) {
            continue;
        }
        uint8_t frame, alpha;
        int ftype;
        if (!csb_v2_vfx_get_field(bf->field_index, &frame, &ftype, &alpha)) {
            if (bf->emitter_index >= 0) {
                csb_v2_vfx_remove_emitter(bf->emitter_index);
            }
            bf->active = 0;
        }
    }
}

/* Returns number of active bound projectiles. */
int csb_p4_binding_active_projectile_count(void) {
    int i, count = 0;
    for (i = 0; i < CSB_P4_BINDING_MAX_ACTIVE_PROJECTILES; i++) {
        if (g_bound_projectiles[i].active) count++;
    }
    return count;
}

/* Returns number of active bound fields. */
int csb_p4_binding_active_field_count(void) {
    int i, count = 0;
    for (i = 0; i < CSB_P4_BINDING_MAX_ACTIVE_FIELDS; i++) {
        if (g_bound_fields[i].active) count++;
    }
    return count;
}

/* Returns 1 if any Phase 4 binding is active. */
int csb_p4_binding_any_active(void) {
    return csb_p4_binding_active_projectile_count() > 0 ||
           csb_p4_binding_active_field_count() > 0 ||
           csb_v2_vfx_active_particle_count() > 0 ||
           csb_v2_vfx_active_emitter_count() > 0;
}

/* Resets all bindings without touching the VFX system. */
void csb_p4_binding_reset(void) {
    int i;
    for (i = 0; i < CSB_P4_BINDING_MAX_ACTIVE_FIELDS; i++) {
        CSB_P4_BoundField *bf = &g_bound_fields[i];
        if (bf->active && bf->emitter_index >= 0) {
            csb_v2_vfx_remove_emitter(bf->emitter_index);
        }
        bf->active = 0;
    }
    for (i = 0; i < CSB_P4_BINDING_MAX_ACTIVE_PROJECTILES; i++) {
        g_bound_projectiles[i].active = 0;
    }
}
