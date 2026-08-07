#include "dm1_v2_particle_system_pc34.h"

/* PC34 renders explosions, fields and projectiles from its authenticated
 * bitmap/aspect path.  It has no generic particle emitter or post-process
 * pixel layer, so this compatibility surface intentionally has no state. */
void v2_particle_init(void) {}
void v2_particle_set_seed(uint32_t seed) { (void)seed; }

int v2_particle_emitter_create(float x, float y, float rate,
                               float spread, float life, float size,
                               float gravity, uint32_t color, int max_count) {
    (void)x;
    (void)y;
    (void)rate;
    (void)spread;
    (void)life;
    (void)size;
    (void)gravity;
    (void)color;
    (void)max_count;
    return -1;
}

void v2_particle_emit(int emitter_idx, float x, float y) {
    (void)emitter_idx;
    (void)x;
    (void)y;
}

int v2_particle_add_direct(float x, float y, float life, float size,
                           float gravity, uint32_t color) {
    (void)x;
    (void)y;
    (void)life;
    (void)size;
    (void)gravity;
    (void)color;
    return -1;
}

void v2_particle_update(float dt) { (void)dt; }
void v2_particle_draw_all(void) {}

int v2_particle_blit_indexed(unsigned char* framebuffer,
                             int framebuffer_width,
                             int framebuffer_height,
                             int origin_x,
                             int origin_y,
                             unsigned char palette_index) {
    (void)framebuffer;
    (void)framebuffer_width;
    (void)framebuffer_height;
    (void)origin_x;
    (void)origin_y;
    (void)palette_index;
    return 0;
}

void v2_particle_tick(float dt) { (void)dt; }
int v2_particle_active_count(void) { return 0; }
void v2_particle_emitter_remove(int emitter_idx) { (void)emitter_idx; }
void v2_particle_clear(void) {}
