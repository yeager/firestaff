/*
 * Product CSB V2 particle boundary.
 *
 * The historical particle book assigns RGB ramps, trajectories, durations,
 * and tile speeds that are not present in PC3.4 GRAPHICS.DAT/DUNGEON.DAT.
 * ReDMCSB DUNVIEW.C F0115/F0128 and CSBWin Graphics.cpp select and compose
 * source bitmaps; neither grants a host particle vocabulary. Until a reviewed
 * source-art binding carries every displayed frame, the playable route keeps
 * the original V1/V2.1 pixels unchanged.
 *
 * `csb_v2_vfx_particles.c` remains available only to its contract probes.
 */

#include "csb_v2_vfx_particles.h"

void csb_v2_vfx_init(void) {}

void csb_v2_vfx_tick(float dtSeconds) { (void)dtSeconds; }

int csb_v2_vfx_add_emitter(float x, float y, float radius,
                           float emit_rate, int vfx_type,
                           int looping, float duration) {
    (void)x; (void)y; (void)radius; (void)emit_rate;
    (void)vfx_type; (void)looping; (void)duration;
    return -1;
}

void csb_v2_vfx_remove_emitter(int emitter_index) { (void)emitter_index; }

int csb_v2_vfx_fire_projectile(float sx, float sy, float tx, float ty,
                                float speed, int vfx_type) {
    (void)sx; (void)sy; (void)tx; (void)ty; (void)speed; (void)vfx_type;
    return -1;
}

int csb_v2_vfx_get_projectile(int projectile_index, float *outX, float *outY,
                              int *outType, uint8_t *outAlpha) {
    (void)projectile_index;
    if (outX) *outX = 0.0f;
    if (outY) *outY = 0.0f;
    if (outType) *outType = CSB_V2_VFX_NONE;
    if (outAlpha) *outAlpha = 0u;
    return 0;
}

int csb_v2_vfx_add_field(int tx, int ty, int vfx_type) {
    (void)tx; (void)ty; (void)vfx_type;
    return -1;
}

void csb_v2_vfx_remove_field(int field_index) { (void)field_index; }

int csb_v2_vfx_get_field(int field_index, uint8_t *outFrame,
                         int *outType, uint8_t *outAlpha) {
    (void)field_index;
    if (outFrame) *outFrame = 0u;
    if (outType) *outType = CSB_V2_VFX_NONE;
    if (outAlpha) *outAlpha = 0u;
    return 0;
}

int csb_v2_vfx_active_particle_count(void) { return 0; }

int csb_v2_vfx_active_emitter_count(void) { return 0; }

const char *csb_v2_vfx_source_evidence(void) {
    return "ReDMCSB DUNVIEW.C F0115/F0128 and CSBWin Graphics.cpp own "
           "CSB projectile/field pixels; no host particle material admitted.";
}
