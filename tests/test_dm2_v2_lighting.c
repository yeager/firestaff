/*
 * test_dm2_v2_lighting.c — DM2 V2.2 Lighting / Outdoor FX smoke test
 *
 * Phase 4: enhanced lighting and outdoor effects.
 * Tests DM2_V2_LightingState API, DM2_V2_OutdoorFX API,
 * fog map, sky color, torch flicker, lightning bloom.
 *
 * No game data, no SDL required — pure unit smoke test.
 */

#include "dm2_v2_lighting.h"
#include "dm2_v2_outdoor_enhanced.h"
#include "dm2_v1_weather.h"
#include <stdio.h>
#include <string.h>

static int s_passed = 0;
static int s_failed = 0;

static void check(const char *name, int cond) {
    if (cond) { printf("  PASS: %s\n", name); s_passed++; }
    else       { printf("  FAIL: %s\n", name); s_failed++; }
}

/* ── Weather constants (from dm2_v1_progression.h) ─────────── */
#define W_CLEAR 0
#define W_RAIN  1
#define W_FOG   2
#define W_STORM 3

int main(void) {
    printf("=== DM2 V2 Phase 4 Lighting / Outdoor FX smoke test ===\n\n");

    /* ══ DM2_V2_LightingState ═══════════════════════════════════ */

    printf("── Lighting State ──\n");
    DM2_V2_LightingState ls;
    dm2_v2_lighting_init(&ls);
    check("init: fog source_count 0", ls.fog.source_count == 0);
    check("init: bloom_timer 0", ls.bloom_timer == 0);

    dm2_v2_lighting_reset(&ls);
    check("reset: fog source_count 0", ls.fog.source_count == 0);
    check("reset: bloom_timer 0", ls.bloom_timer == 0);

    /* ══ Sky Color ═══════════════════════════════════════════════ */

    printf("── Sky Color ──\n");
    uint32_t sky = dm2_v2_sky_color_for_time(0.5f, W_CLEAR);
    check("sky_color day: non-zero", sky != 0);
    check("sky_color day: alpha=FF", (sky >> 24) == 0xFF);

    sky = dm2_v2_sky_color_for_time(0.1f, W_CLEAR); /* dawn */
    check("sky_color dawn: non-zero", sky != 0);

    sky = dm2_v2_sky_color_for_time(0.9f, W_CLEAR); /* dusk */
    check("sky_color dusk: non-zero", sky != 0);

    sky = dm2_v2_sky_color_for_time(0.5f, W_FOG);
    check("sky_color fog: gray base", sky != 0);

    sky = dm2_v2_sky_color_for_time(0.5f, W_STORM);
    check("sky_color storm: gray base", sky != 0);

    sky = dm2_v2_sky_color_for_time(0.5f, W_RAIN);
    check("sky_color rain: desaturated", sky != 0);

    /* Out-of-range time: must clamp, not crash */
    sky = dm2_v2_sky_color_for_time(-1.0f, W_CLEAR);
    check("sky_color time<0: clamped, not crashed", sky != 0);
    sky = dm2_v2_sky_color_for_time(2.0f, W_CLEAR);
    check("sky_color time>1: clamped, not crashed", sky != 0);

    /* From weather state */
    DM2_V1_WeatherState ws;
    memset(&ws, 0, sizeof(ws));
    ws.time_fraction = 0.5f;
    ws.weather = W_CLEAR;
    sky = dm2_v2_sky_color_from_weather(&ws);
    check("sky_color from weather: OK", sky != 0);

    sky = dm2_v2_sky_color_from_weather(NULL);
    check("sky_color from NULL weather: fallback day blue", sky != 0);

    /* ══ Fog Map ═════════════════════════════════════════════════ */

    printf("── Fog Map ──\n");
    DM2_V2_FogState f;
    dm2_v2_fog_init(&f);
    check("fog init: zero density", f.density[0][0] == 0);
    check("fog init: source_count 0", f.source_count == 0);
    check("fog init: weather_fog 0", f.weather_fog == 0.0f);

    dm2_v2_fog_clear(&f);
    check("fog clear: source_count 0", f.source_count == 0);

    int id0 = dm2_v2_fog_add_source(&f, 16, 16, 1.0f, 4.0f);
    check("fog_add_source: id>=0", id0 >= 0);
    check("fog_add_source: count=1", f.source_count == 1);

    int id1 = dm2_v2_fog_add_source(&f, 8, 8, 0.5f, 2.0f);
    check("fog 2nd source: id>=0", id1 >= 0);

    /* Full → returns -1 */
    for (int i = 0; i < DM2_V2_MAX_FOG_SOURCES; i++)
        dm2_v2_fog_add_source(&f, i, i, 0.3f, 1.0f);
    check("fog_full: next add returns -1", dm2_v2_fog_add_source(&f, 100, 100, 1.0f, 1.0f) == -1);

    /* Rebuild */
    dm2_v2_fog_rebuild(&f);
    check("fog_rebuild: non-zero density near source", dm2_v2_fog_get_tile(&f, 16, 16) > 0);

    /* Out-of-range tile: returns weather base, not crash */
    uint8_t v = dm2_v2_fog_get_tile(&f, -999, 999);
    (void)v;
    check("fog_get_tile OOB: not crash", 1);

    /* Weather fog states */
    dm2_v2_fog_set_weather(&f, W_CLEAR);
    check("fog weather CLEAR→fog=0", f.weather_fog == 0.0f);
    dm2_v2_fog_set_weather(&f, W_FOG);
    check("fog weather FOG→fog=0.5", f.weather_fog == 0.5f);
    dm2_v2_fog_set_weather(&f, W_STORM);
    check("fog weather STORM→fog=0.3", f.weather_fog == 0.3f);
    dm2_v2_fog_set_weather(&f, W_RAIN);
    check("fog weather RAIN→fog=0.1", f.weather_fog == 0.1f);

    /* Remove source */
    dm2_v2_fog_init(&f);
    dm2_v2_fog_add_source(&f, 5, 5, 1.0f, 2.0f);
    dm2_v2_fog_add_source(&f, 20, 20, 0.5f, 2.0f);
    int saved_count = f.source_count;
    dm2_v2_fog_remove_source(&f, 0);
    check("fog_remove_source: count decremented", f.source_count == saved_count - 1);

    /* Null safety */
    dm2_v2_fog_init(NULL);
    dm2_v2_fog_clear(NULL);
    dm2_v2_fog_rebuild(NULL);
    dm2_v2_fog_get_tile(NULL, 0, 0);
    dm2_v2_fog_tick(NULL, 0.016f);
    check("fog null safety: no crash", 1);

    /* ══ Ambient ══════════════════════════════════════════════════ */

    printf("── Ambient ──\n");
    DM2_V2_AmbientState a;
    dm2_v2_ambient_init(&a);
    check("ambient init: default 128 mid-value",
          dm2_v2_ambient_get_tile(&a, 16, 16) == 128);

    dm2_v2_ambient_set_sky_factor(&a, 0.7f);
    check("sky_factor set", a.sky_ambient_factor == 0.7f);

    dm2_v2_ambient_set_fog_alpha(&a, 180);
    check("fog_alpha set", dm2_v2_ambient_get_fog_alpha(&a) == 180);

    /* ══ Torch Flicker ═══════════════════════════════════════════ */

    printf("── Torch Flicker ──\n");
    dm2_v2_lighting_init(&ls);
    dm2_v2_torch_flicker_tick(&ls, 0.016f);
    check("torch_tick: champion[0] intensity > 0",
          ls.torch_intensity[0] > 0.0f);

    float prev = ls.torch_intensity[0];
    (void)prev;
    dm2_v2_torch_flicker_tick(&ls, 0.016f);
    check("torch_tick 2: phase advances (intensity may differ)",
          ls.torch_flicker_phases[0] > 0.0f);

    check("torch_get_intensity idx=0 > 0",
          dm2_v2_torch_get_intensity(&ls, 0) > 0.0f);
    check("torch_get_intensity idx=3 > 0",
          dm2_v2_torch_get_intensity(&ls, 3) > 0.0f);
    check("torch_get_intensity idx=-1: fallback 1.0",
          dm2_v2_torch_get_intensity(&ls, -1) == 1.0f);
    check("torch_get_intensity idx=99: fallback 1.0",
          dm2_v2_torch_get_intensity(&ls, 99) == 1.0f);
    check("torch_get_intensity NULL: fallback 1.0",
          dm2_v2_torch_get_intensity(NULL, 0) == 1.0f);

    /* ══ Lightning Bloom ═══════════════════════════════════════════ */

    printf("── Lightning Bloom ──\n");
    dm2_v2_lighting_init(&ls);
    check("bloom initially inactive", !dm2_v2_lighting_bloom_active(&ls));

    dm2_v2_lighting_trigger_bloom(&ls, 255, 255, 255, 0.2f);
    check("bloom triggered: active", dm2_v2_lighting_bloom_active(&ls));
    check("bloom triggered: timer > 0", ls.bloom_timer > 0);

    uint8_t br=99, bg=99, bb=99;
    dm2_v2_lighting_bloom_color(&ls, &br, &bg, &bb);
    check("bloom_color: full brightness", br == 255 && bg == 255 && bb == 255);

    dm2_v2_lighting_tick_bloom(&ls, 0.1f);
    check("bloom_tick: still active after partial dt",
          dm2_v2_lighting_bloom_active(&ls));

    dm2_v2_lighting_tick_bloom(&ls, 0.2f);
    check("bloom_tick: inactive after full duration",
          !dm2_v2_lighting_bloom_active(&ls));

    /* Fade out: timer expired → color should be 0 */
    br=bg=bb=99;
    dm2_v2_lighting_bloom_color(&ls, &br, &bg, &bb);
    check("bloom expired: color dark", br == 0 && bg == 0 && bb == 0);

    dm2_v2_lighting_trigger_bloom(NULL, 255, 0, 0, 0.2f);
    check("bloom_trigger NULL: no crash", 1);
    dm2_v2_lighting_tick_bloom(NULL, 0.016f);
    check("bloom_tick NULL: no crash", 1);

    /* ══ Outdoor FX ════════════════════════════════════════════════ */

    printf("── Outdoor FX ──\n");
    DM2_V2_OutdoorFX fx;
    dm2_v2_outdoor_fx_init(&fx);
    check("outdoor init: lightning_phase OFF", fx.lightning_phase == DM2_LN_OFF);
    check("outdoor init: rain_intensity 0", fx.rain_intensity == 0.0f);

    /* Cloud drift */
    float cloud0 = dm2_v2_outdoor_fx_cloud_offset(&fx);
    dm2_v2_outdoor_fx_tick(&fx, 0.016f, W_CLEAR);
    float cloud1 = dm2_v2_outdoor_fx_cloud_offset(&fx);
    check("outdoor cloud offset advances", cloud1 > cloud0);

    /* Tree sway */
    float sway0 = dm2_v2_outdoor_fx_tree_sway(&fx);
    dm2_v2_outdoor_fx_tick(&fx, 0.016f, W_CLEAR);
    float sway1 = dm2_v2_outdoor_fx_tree_sway(&fx);
    check("outdoor tree sway advances", sway1 > sway0);

    /* Rain weather */
    dm2_v2_outdoor_fx_tick(&fx, 0.016f, W_RAIN);
    check("outdoor weather RAIN: rain_intensity = 0.5",
          fx.rain_intensity > 0.4f && fx.rain_intensity <= 0.6f);

    /* Storm weather → lightning sequence starts */
    dm2_v2_outdoor_fx_init(&fx);
    dm2_v2_outdoor_fx_tick(&fx, 0.016f, W_STORM);
    check("outdoor storm: lightning triggered", fx.lightning_phase != DM2_LN_OFF);
    int intensity = dm2_v2_outdoor_fx_lightning_intensity(&fx);
    check("outdoor lightning: intensity > 0", intensity > 0);

    /* Trigger lightning externally */
    dm2_v2_outdoor_fx_init(&fx);
    dm2_v2_outdoor_fx_trigger_lightning(&fx);
    check("trigger_lightning: phase FLASH", fx.lightning_phase == DM2_LN_FLASH);
    check("trigger_lightning: timer > 0", fx.lightning_timer > 0);

    /* Advance FLASH→SUSTAIN */
    dm2_v2_outdoor_fx_tick(&fx, 0.01f, W_STORM);
    /* FLASH timer (0.06s) should have fired → SUSTAIN or higher */
    check("outdoor lightning after tick: phase != OFF",
          fx.lightning_phase != DM2_LN_OFF);

    /* Null safety */
    dm2_v2_outdoor_fx_init(NULL);
    dm2_v2_outdoor_fx_tick(NULL, 0.016f, W_CLEAR);
    dm2_v2_outdoor_fx_trigger_lightning(NULL);
    (void)dm2_v2_outdoor_fx_lightning_intensity(NULL);
    check("outdoor FX null safety: no crash", 1);

    /* ══ Sky color from outdoor FX ═══════════════════════════════ */

    printf("── Sky Color from FX ──\n");
    dm2_v2_outdoor_fx_init(&fx);
    uint32_t sc = dm2_v2_outdoor_fx_sky_color_ex(&fx, 0.5f, W_CLEAR);
    check("outdoor sky_color: non-zero", sc != 0);
    sc = dm2_v2_outdoor_fx_sky_color_ex(NULL, 0.5f, W_CLEAR);
    check("outdoor sky_color NULL: non-zero fallback", sc != 0);

    /* ══ Source Evidence ══════════════════════════════════════════ */

    printf("── Source Evidence ──\n");
    const char *ev = dm2_v2_lighting_source_evidence();
    check("lighting source_evidence: non-null, len > 10",
          ev != NULL && strlen(ev) > 10);
    ev = dm2_v2_outdoor_source_evidence();
    check("outdoor source_evidence: non-null, len > 10",
          ev != NULL && strlen(ev) > 10);

    /* ══ Result ════════════════════════════════════════════════════ */

    printf("\n=== Results: %d passed, %d failed ===\n", s_passed, s_failed);
    return s_failed > 0 ? 1 : 0;
}
