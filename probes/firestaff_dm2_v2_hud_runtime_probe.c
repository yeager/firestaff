/* DM2-GDAT-FB-06: V2 HUD must consume boot-owned original GDAT pixels. */
#include "dm2_v2_hud_runtime.h"
#include "dm2_v1_viewport_renderer.h"
#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;
#define CHECK(c) do { ++assertions; if (!(c)) { ++failures; \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

static uint8_t source_pixel = 0x5a;
static int fetch_calls;
static int source_enabled;

static int original_gdat_fetch(void *user, int key, const uint8_t **pixels,
                               int *w, int *h, int *stride)
{
    (void)user;
    ++fetch_calls;
    if (!source_enabled || key == 0) return -1;
    *pixels = &source_pixel;
    *w = 1;
    *h = 1;
    *stride = 1;
    return 0;
}

int main(void)
{
    uint8_t fb[320 * 200];
    DM2_V2_PhaseGateConfig gate = { 1, 1 };
    const char *evidence;

    dm2_v2_hud_runtime_init();
    dm2_v2_hud_runtime_set_gate_config(&gate);
    dm2_v2_hud_runtime_set_gdat_source(original_gdat_fetch, NULL, 1);

    memset(fb, 0, sizeof(fb));
    source_enabled = 1;
    fetch_calls = 0;
    dm2_v2_hud_runtime_render(fb, 320, 200);
    CHECK(fetch_calls > 0);
    CHECK(fb[0] == source_pixel); /* INTERFACE_GENERAL top bar */
    CHECK(fb[172 * 320] == source_pixel); /* action strip */
    CHECK(dm2_v2_hud_runtime_is_active() == 1);

    /* Source index 0 is transparent; V2 must not replace it with chrome. */
    source_pixel = 0;
    memset(fb, 0x33, sizeof(fb));
    dm2_v2_hud_runtime_render(fb, 320, 200);
    CHECK(fb[0] == 0x33);
    CHECK(fb[172 * 320] == 0x33);

    /* No decoded original images means no procedural/fixed-pixel fallback. */
    source_pixel = 0x5a;
    source_enabled = 0;
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_runtime_render(fb, 320, 200);
    for (size_t i = 0; i < sizeof(fb); ++i) CHECK(fb[i] == 0);

    gate.v2LaunchEnabled = 0;
    source_enabled = 1;
    memset(fb, 0x44, sizeof(fb));
    dm2_v2_hud_runtime_render(fb, 320, 200);
    CHECK(fb[0] == 0x44);

    evidence = dm2_v2_hud_runtime_source_evidence();
    CHECK(evidence && strstr(evidence, "INTERFACE_GENERAL"));
    CHECK(evidence && strstr(evidence, "not synthesized"));
    dm2_v2_hud_runtime_shutdown();
    printf("%d/%d assertions passed\n", assertions - failures, assertions);
    return failures ? 1 : 0;
}
