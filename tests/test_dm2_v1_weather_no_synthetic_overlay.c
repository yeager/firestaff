/* DM2 real-GDAT weather boundary: GRAPHICSSET weather words are controls,
 * not drawable pixels. See skproject SKWIN/SkWinCore.cpp GDAT word queries. */

#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *name)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    } else {
        fprintf(stderr, "PASS: %s\n", name);
    }
}

static void check_weather(int weather, int intensity, int tick,
                          uint16_t rain, uint16_t mist, uint16_t thunder,
                          const char *name)
{
    uint8_t framebuffer[320 * 200];
    uint8_t before[320 * 200];
    DM2_V1_ViewportState viewport;

    memset(framebuffer, 0x5a, sizeof(framebuffer));
    memcpy(before, framebuffer, sizeof(before));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_viewport_set_weather(&viewport, weather, intensity);
    viewport.tick_count = tick;
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, 0, 0x12345678u, 0, 0, 0, 0, 0, 0,
        rain, mist, thunder, 0);
    dm2_v1_render_weather_overlay(&viewport);

    check(memcmp(framebuffer, before, sizeof(framebuffer)) == 0 &&
              viewport.gdat_scene_weather_consumed_count == 0,
          name);
}

int main(void)
{
    check_weather(DM2_V1_WEATHER_OVERLAY_RAIN, 64, 3, 1, 0, 0,
                  "rain control does not create procedural pixels");
    check_weather(DM2_V1_WEATHER_OVERLAY_FOG, 32, 0, 0, 1, 0,
                  "mist control does not create procedural pixels");
    check_weather(DM2_V1_WEATHER_OVERLAY_STORM, 70, 121, 1, 0, 1,
                  "thunder control does not create procedural pixels");

    fprintf(stderr, "DM2 weather no-synthetic boundary: %d failure(s)\n",
            failures);
    return failures ? 1 : 0;
}
