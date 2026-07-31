#include "csb_v2_chaos_enhanced.h"

/*
 * ReDMCSB and CSBWin provide DSA execution and original raster ownership, but
 * no mapping from a DSA script id to an RGB glow, a particle family, a modern
 * projectile speed or a light-event curve. The previous implementation
 * invented those values. Keep this ABI surface as a strict no-draw boundary
 * until a source-command receipt supplies every displayed material fact.
 */

void csb_v2_chaos_init(void) {}

void csb_v2_chaos_on_trigger(int script_id, int flag_index) {
    (void)script_id;
    (void)flag_index;
}

void csb_v2_chaos_tick(float dt) {
    (void)dt;
}

int csb_v2_chaos_active_count(void) {
    return 0;
}

int csb_v2_chaos_particle_count(void) {
    return 0;
}

int csb_v2_chaos_fire_projectile(float sx, float sy,
                                 float tx, float ty,
                                 float speed,
                                 int vfx_type) {
    (void)sx;
    (void)sy;
    (void)tx;
    (void)ty;
    (void)speed;
    (void)vfx_type;
    return -1;
}

void csb_v2_chaos_render_overlay(float *outR,
                                 float *outG,
                                 float *outB,
                                 float *outAlpha) {
    if (outR) *outR = 0.0f;
    if (outG) *outG = 0.0f;
    if (outB) *outB = 0.0f;
    if (outAlpha) *outAlpha = 0.0f;
}

const char *csb_v2_chaos_source_evidence(void) {
    return "ReDMCSB DSA execution remains source-owned; "
           "no source receipt admits a modern chaos overlay or projectile.";
}
