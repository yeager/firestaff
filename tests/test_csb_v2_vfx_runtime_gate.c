#include "csb_v2_vfx_particles.h"

#include <stdio.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    float x = 1.0f;
    float y = 1.0f;
    int type = -1;
    uint8_t alpha = 255u;
    uint8_t frame = 255u;

    csb_v2_vfx_init();
    check(csb_v2_vfx_active_particle_count() == 0,
          "product VFX starts without particles");
    check(csb_v2_vfx_active_emitter_count() == 0,
          "product VFX starts without emitters");
    check(csb_v2_vfx_add_emitter(1.0f, 2.0f, 3.0f, 4.0f,
                                 CSB_V2_VFX_FIRE, 1, 1.0f) == -1,
          "product VFX rejects a host emitter");
    check(csb_v2_vfx_fire_projectile(1.0f, 2.0f, 3.0f, 4.0f,
                                     5.0f, CSB_V2_VFX_LIGHTNING) == -1,
          "product VFX rejects a host projectile");
    check(csb_v2_vfx_add_field(1, 2, CSB_V2_VFX_CHAOS_MIST) == -1,
          "product VFX rejects a host field");
    check(!csb_v2_vfx_get_projectile(0, &x, &y, &type, &alpha) &&
              x == 0.0f && y == 0.0f && type == CSB_V2_VFX_NONE && alpha == 0u,
          "product projectile query stays transparent");
    check(!csb_v2_vfx_get_field(0, &frame, &type, &alpha) &&
              frame == 0u && type == CSB_V2_VFX_NONE && alpha == 0u,
          "product field query stays transparent");
    csb_v2_vfx_tick(10.0f);
    check(csb_v2_vfx_active_particle_count() == 0 &&
              csb_v2_vfx_active_emitter_count() == 0,
          "product VFX tick cannot revive synthetic state");
    check(csb_v2_vfx_source_evidence() != NULL,
          "product VFX source boundary names original owners");

    if (failures != 0) {
        fprintf(stderr, "%d failures\n", failures);
        return 1;
    }
    printf("CSB V2 VFX runtime gate: 9 checks passed\n");
    return 0;
}
