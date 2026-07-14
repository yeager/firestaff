/* Source weather pixels require the complete c_weather DistantEnvironment
 * receipt: c_weather.cpp DM2_UPDATE_WEATHER:221-266, c_querydb.cpp
 * DM2_RETRIEVE_ENVIRONMENT_CMD_CD_FW:2144 and DM2_QUERY_TEMP_PICST:2381. */

#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int omit_rain;

static int fetch_asset(void *user, int gdat_index, const uint8_t **pixels,
                       int *width, int *height, int *stride)
{
    static const uint8_t cloud[2] = { 1, 2 };
    static const uint8_t rain[2] = { 3, 4 };
    int graphicsset;
    int field;
    (void)user;
    if (!dm2_v1_viewport_weather_environment_graphic_address(
            gdat_index, &graphicsset, &field) || graphicsset != 3 ||
        (field != DM2_V1_WEATHER_CLOUD_HEAVY_CMD &&
         field != DM2_V1_WEATHER_RAIN_HEAVY_CMD) ||
        (omit_rain && field == DM2_V1_WEATHER_RAIN_HEAVY_CMD)) return -1;
    *pixels = field == DM2_V1_WEATHER_CLOUD_HEAVY_CMD ? cloud : rain;
    *width = 2;
    *height = 1;
    *stride = 2;
    return 0;
}

static int fetch_palette(void *user, int gdat_index, uint8_t palette[16],
                         uint32_t *hash)
{
    int graphicsset;
    int field;
    (void)user;
    if (!dm2_v1_viewport_weather_environment_graphic_address(
            gdat_index, &graphicsset, &field) || graphicsset != 3) return -1;
    memset(palette, 0, 16u);
    palette[1] = 0x41u;
    palette[2] = 0x42u;
    palette[3] = 0x43u;
    palette[4] = 0x44u;
    *hash = field == DM2_V1_WEATHER_CLOUD_HEAVY_CMD ? 0xc10du : 0xa11eu;
    return 0;
}

static void setup_receipt(DM2_V1_WeatherRendererReceipt *receipt)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->renderer_hash = 0x57454154u;
    receipt->command_count = 2u;
    receipt->draws[0].valid = 1;
    receipt->draws[0].image_field = DM2_V1_WEATHER_CLOUD_HEAVY_CMD;
    receipt->draws[0].source_right = 2;
    receipt->draws[0].source_bottom = 1;
    receipt->draws[0].material_hash = 0x6801u;
    receipt->clips[0].valid = 1;
    receipt->clips[0].x = 10;
    receipt->clips[0].y = 30;
    receipt->clips[0].w = 2;
    receipt->clips[0].h = 1;
    receipt->draws[1].valid = 1;
    receipt->draws[1].image_field = DM2_V1_WEATHER_RAIN_HEAVY_CMD;
    receipt->draws[1].source_right = 2;
    receipt->draws[1].source_bottom = 1;
    receipt->draws[1].material_hash = 0x6b01u;
    receipt->clips[1].valid = 1;
    receipt->clips[1].x = 10;
    receipt->clips[1].y = 31;
    receipt->clips[1].w = 2;
    receipt->clips[1].h = 1;
}

int main(void)
{
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_ViewportState viewport;
    DM2_V1_WeatherRendererReceipt receipt;

    setup_receipt(&receipt);
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_outdoor(&viewport, 1);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_palette, NULL);
    dm2_v1_viewport_set_gdat_weather_renderer_receipt(&viewport, 3u, &receipt);
    omit_rain = 0;
    dm2_v1_render_weather_overlay(&viewport);
    if (viewport.asset_weather_drawn_count != 2 ||
        viewport.gdat_scene_weather_consumed_count != 4 ||
        framebuffer[30 * DM2_VP_WIDTH + 10] != 0x41u ||
        framebuffer[30 * DM2_VP_WIDTH + 11] != 0x42u ||
        framebuffer[31 * DM2_VP_WIDTH + 10] != 0x43u ||
        framebuffer[31 * DM2_VP_WIDTH + 11] != 0x44u) {
        fputs("FAIL: complete source weather receipt did not consume GDAT pixels\n", stderr);
        return 1;
    }

    memset(framebuffer, 0x5a, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_outdoor(&viewport, 1);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_asset_provider(&viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(&viewport, fetch_palette, NULL);
    dm2_v1_viewport_set_gdat_weather_renderer_receipt(&viewport, 3u, &receipt);
    omit_rain = 1;
    dm2_v1_render_weather_overlay(&viewport);
    if (viewport.asset_weather_drawn_count != 0 ||
        framebuffer[30 * DM2_VP_WIDTH + 10] != 0x5a ||
        framebuffer[31 * DM2_VP_WIDTH + 10] != 0x5a ||
        (viewport.blocked_material_mask & DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WEATHER) == 0u) {
        fputs("FAIL: incomplete weather receipt leaked a partial source layer\n", stderr);
        return 1;
    }

    puts("PASS: source weather receipt atomically owns ENVIRONMENT pixels");
    return 0;
}
