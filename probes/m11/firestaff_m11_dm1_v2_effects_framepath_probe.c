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

static struct DungeonDatState_Compat s_dungeon;
static struct DungeonMapDesc_Compat s_map;
static struct DungeonMapTiles_Compat s_tiles;
static unsigned char s_square_data[2];

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

static void seed_visible_particle(void)
{
    int emitter;
    v2_particle_init();
    v2_particle_set_seed(1u);
    emitter = v2_particle_emitter_create(
        10.0f, 12.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0x00ff00ffu, 1);
    CHECK(emitter == 0, "visible emitter created");
    v2_particle_emit(emitter, 10.0f, 12.0f);
    CHECK(v2_particle_active_count() == 1, "visible particle initially alive");
}

static void init_dm1_state(M11_GameViewState* state, int presentationMode)
{
    memset(state, 0, sizeof(*state));
    M11_GameView_Init(state);
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state->presentationMode = presentationMode;
    state->presentationWidth = 320;
    state->presentationHeight = 200;
}

static void attach_two_cell_corridor(M11_GameViewState* state)
{
    memset(&s_dungeon, 0, sizeof(s_dungeon));
    memset(&s_map, 0, sizeof(s_map));
    memset(&s_tiles, 0, sizeof(s_tiles));
    memset(s_square_data, 0, sizeof(s_square_data));

    s_map.width = 1;
    s_map.height = 2;
    s_tiles.squareData = s_square_data;
    s_tiles.squareCount = 2;
    s_dungeon.header.mapCount = 1;
    s_dungeon.maps = &s_map;
    s_dungeon.tiles = &s_tiles;
    s_dungeon.tilesLoaded = 1;

    state->world.dungeon = &s_dungeon;
    state->world.partyMapIndex = 0;
    state->world.newPartyMapIndex = 0;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 0;
    state->world.party.mapY = 1;
    state->world.party.direction = 0;
}

static void seed_runtime_fireball_ahead(M11_GameViewState* state)
{
    attach_two_cell_corridor(state);
    state->world.projectiles.count = 1;
    state->world.projectiles.entries[0].reserved3 = 1;
    state->world.projectiles.entries[0].slotIndex = 0;
    state->world.projectiles.entries[0].mapIndex = 0;
    state->world.projectiles.entries[0].mapX = 0;
    state->world.projectiles.entries[0].mapY = 0;
    state->world.projectiles.entries[0].direction = 0;
    state->world.projectiles.entries[0].cell = 0;
    state->world.projectiles.entries[0].projectileSubtype =
        PROJECTILE_SUBTYPE_FIREBALL;
}

static void draw_once(M11_GameViewState* state, unsigned char* framebuffer)
{
    memset(framebuffer, 0, 320 * 200);
    M11_GameView_Draw(state, framebuffer, 320, 200);
}

static int count_particle_region_diff(const unsigned char* a,
                                      const unsigned char* b)
{
    int changed = 0;
    int y;
    for (y = 33 + 11; y <= 33 + 13; y++) {
        int x;
        for (x = 9; x <= 11; x++) {
            if (a[y * 320 + x] != b[y * 320 + x]) {
                changed++;
            }
        }
    }
    return changed;
}

static int count_live_effect_region_diff(const unsigned char* a,
                                         const unsigned char* b)
{
    int changed = 0;
    int y;
    for (y = 33; y < 33 + 136; y++) {
        int x;
        for (x = 0; x < 224; x++) {
            if (a[y * 320 + x] != b[y * 320 + x]) {
                changed++;
            }
        }
    }
    return changed;
}

int main(void)
{
    M11_GameViewState state;
    unsigned char framebuffer[320 * 200];
    unsigned char baseline[320 * 200];

    printf("=== M11 DM1 V2 enhanced-effects framepath probe ===\n");

    seed_short_lived_particle();
    init_dm1_state(&state, M12_PRESENTATION_V1_ORIGINAL);
    draw_once(&state, framebuffer);
    CHECK(v2_particle_active_count() == 1,
          "V1 original draw does not tick V2 particles");

    seed_short_lived_particle();
    init_dm1_state(&state, M12_PRESENTATION_V20_FILTERED);
    draw_once(&state, framebuffer);
    CHECK(v2_particle_active_count() == 0,
          "V2.0 draw ticks enhanced-effects runtime");

    seed_short_lived_particle();
    init_dm1_state(&state, M12_PRESENTATION_V22_MODERN);
    draw_once(&state, framebuffer);
    CHECK(v2_particle_active_count() == 0,
          "V2.2 draw ticks enhanced-effects runtime");

    seed_visible_particle();
    init_dm1_state(&state, M12_PRESENTATION_V1_ORIGINAL);
    v2_particle_init();
    draw_once(&state, baseline);
    seed_visible_particle();
    draw_once(&state, framebuffer);
    CHECK(count_particle_region_diff(baseline, framebuffer) == 0,
          "V1 original draw does not paint V2 particle overlay");

    v2_particle_init();
    init_dm1_state(&state, M12_PRESENTATION_V20_FILTERED);
    draw_once(&state, baseline);
    seed_visible_particle();
    draw_once(&state, framebuffer);
    CHECK(count_particle_region_diff(baseline, framebuffer) > 0,
          "V2.0 draw paints visible particle overlay into viewport");

    v2_particle_init();
    init_dm1_state(&state, M12_PRESENTATION_V22_MODERN);
    draw_once(&state, baseline);
    seed_visible_particle();
    draw_once(&state, framebuffer);
    CHECK(count_particle_region_diff(baseline, framebuffer) > 0,
          "V2.2 draw paints visible particle overlay into viewport");

    init_dm1_state(&state, M12_PRESENTATION_V1_ORIGINAL);
    seed_runtime_fireball_ahead(&state);
    v2_particle_init();
    draw_once(&state, baseline);
    init_dm1_state(&state, M12_PRESENTATION_V20_FILTERED);
    seed_runtime_fireball_ahead(&state);
    v2_particle_init();
    draw_once(&state, framebuffer);
    CHECK(v2_particle_active_count() > 0,
          "V2.0 live runtime projectile seeds transient particles");
    CHECK(count_live_effect_region_diff(baseline, framebuffer) > 0,
          "V2.0 live runtime projectile seeds additional effect overlay");

    if (s_fail) {
        fprintf(stderr, "Summary: %d passed, %d failed\n", s_pass, s_fail);
        return 1;
    }

    printf("Summary: %d passed, 0 failed\n", s_pass);
    return 0;
}
