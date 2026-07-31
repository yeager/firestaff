#include "csb_p4_lighting_metadata.h"

/*
 * Phase 4 has no source-owned command-to-material transaction yet.  Earlier
 * versions kept an otherwise unreachable host particle, glow and torch
 * implementation here.  Keeping it callable made a future caller one gate
 * mistake away from presenting invented CSB pixels.  Preserve the ABI but
 * make every presentation entry point fail closed.
 *
 * ReDMCSB: CASTER.C F0394 and ANIM.C G3567 define gameplay/animation state;
 * neither supplies the host RGB colours, radii, speeds or particle curves
 * that were formerly fabricated in this module.
 */
void csb_p4_binding_init(void) {}
void csb_p4_binding_reset(void) {}

int csb_p4_binding_fire_projectile(const CSB_V2_PhaseGateConfig *cfg,
                                   int spell_id, float sx, float sy,
                                   float tx, float ty)
{
    (void)cfg; (void)spell_id; (void)sx; (void)sy; (void)tx; (void)ty;
    return -1;
}

int csb_p4_binding_add_field(const CSB_V2_PhaseGateConfig *cfg,
                             int spell_id, int tile_x, int tile_y)
{
    (void)cfg; (void)spell_id; (void)tile_x; (void)tile_y;
    return -1;
}

void csb_p4_binding_trigger_chaos(const CSB_V2_PhaseGateConfig *cfg,
                                  int spell_id)
{
    (void)cfg; (void)spell_id;
}

int csb_p4_binding_add_torch_light(const CSB_V2_PhaseGateConfig *cfg,
                                   float x, float y, int torch_type)
{
    (void)cfg; (void)x; (void)y; (void)torch_type;
    return -1;
}

void csb_p4_binding_tick(float dt_seconds) { (void)dt_seconds; }
int csb_p4_binding_active_projectile_count(void) { return 0; }
int csb_p4_binding_active_field_count(void) { return 0; }
int csb_p4_binding_any_active(void) { return 0; }
