/*
 * firestaff_m11_dm1_v2_effects_framepath_probe.c
 *
 * Data-free M11 wire-up proof for the DM1 V2 Phase 4 enhanced-effects
 * runtime. A short-lived V2 particle is used as the observable: V1
 * presentation must leave it alive, while DM1 V2 presentation must tick
 * it from M11_GameView_Draw after the source viewport render.
 *
 * Source-lock: ReDMCSB DUNVIEW.C F0128/F0115 and PROJEXPL.C F0213/F0220
 * own source visuals; Firestaff's V2 effect tick is presentation-only
 * behind DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION.
 */

#include "dm1_v2_particle_system_pc34.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

static int s_pass = 0;
static int s_fail = 0;

#define CHECK(expr, msg) do {                                             \
    ++s_pass;                                                             \
    if (!(expr)) {                                                        \
        fprintf(stderr, "FAIL %s:%d: %s -- %s\n",                         \
                __FILE__, __LINE__, #expr, (msg));                        \
        ++s_fail;                                                         \
    }                                                                     \
} while (0)

static void seed_short_lived_particle(void)
{
    int emitter;
    v2_particle_init();
    v2_particle_set_seed(1u);
    emitter = v2_particle_emitter_create(
        10.0f, 12.0f, 1.0f, 0.0f, 0.001f, 1.0f, 0.0f, 0x00ff00ffu, 1);
    CHECK(emitter == 0, "emitter created");
    v2_particle_emit(emitter, 10.0f, 12.0f);
    CHECK(v2_particle_active_count() == 1, "particle initially alive");
}

static void init_dm1_state(M11_GameViewState* state, int presentationMode)
{
    memset(state, 0, sizeof(*state));
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state->presentationMode = presentationMode;
    state->presentationWidth = 320;
    state->presentationHeight = 200;
}

static void draw_once(M11_GameViewState* state)
{
    static unsigned char framebuffer[320 * 200];
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(state, framebuffer, 320, 200);
}

int main(void)
{
    M11_GameViewState state;

    printf("=== M11 DM1 V2 enhanced-effects framepath probe ===\n");

    seed_short_lived_particle();
    init_dm1_state(&state, M12_PRESENTATION_V1_ORIGINAL);
    draw_once(&state);
    CHECK(v2_particle_active_count() == 1,
          "V1 original draw does not tick V2 particles");

    seed_short_lived_particle();
    init_dm1_state(&state, M12_PRESENTATION_V20_FILTERED);
    draw_once(&state);
    CHECK(v2_particle_active_count() == 0,
          "V2.0 draw ticks enhanced-effects runtime");

    seed_short_lived_particle();
    init_dm1_state(&state, M12_PRESENTATION_V22_MODERN);
    draw_once(&state);
    CHECK(v2_particle_active_count() == 0,
          "V2.2 draw ticks enhanced-effects runtime");

    if (s_fail) {
        fprintf(stderr, "Summary: %d passed, %d failed\n", s_pass, s_fail);
        return 1;
    }

    printf("Summary: %d passed, 0 failed\n", s_pass);
    return 0;
}
