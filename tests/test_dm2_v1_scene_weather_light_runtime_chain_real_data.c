/* Real G1 MapGraphicsStyle -> GRAPHICSSET scene/light -> c_weather runtime
 * chain.  This does not call the viewport renderer; it proves the source
 * receipts that the runtime/viewport consumer must receive. */

#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_graphics_data_open.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_weather_gdat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static int load_canonical_files(uint8_t **graphics, size_t *graphics_size,
                                uint8_t **dungeon, size_t *dungeon_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char graphics_path[1100];
    char dungeon_path[1100];

    if (!root || !root[0]) return 0;
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
    return read_file(graphics_path, graphics, graphics_size) &&
        read_file(dungeon_path, dungeon, dungeon_size);
}

static int check(int condition, const char *label)
{
    if (condition) {
        printf("PASS: %s\n", label);
        return 1;
    }
    printf("FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_GraphicsDataOpenReceipt graphics_open;
    DM2_V1_GdatSceneM11CommandPlan scene_plan;
    DM2_V1_GdatSceneLightM11Receipt scene_light;
    DM2_V1_CLightMapDescriptorReceipt map_descriptor;
    DM2_V1_CLightSourceState c_light_source;
    DM2_V1_CLightM11Receipt c_light;
    DM2_V1_WeatherGdatReceipt weather;
    DM2_V1_WeatherRuntimeAdmissionReceipt weather_admission;
    DM2_V1_EnvironmentWeatherReceipt environment;
    DM2_V1_RainfallParamReceipt rainfall;
    DM2_V1_SceneWeatherLightRuntimeReceipt chain;
    DM2_V1_SetTimerWeatherReceipt timer_owner;
    DM2_V1_WeatherRendererReceipt renderer;
    DM2_V1_OutdoorWeatherM11Receipt outdoor_m11;
    int selected_level = -1;
    int selected_style = -1;
    int failures = 0;

    if (!getenv("FIRESTAFF_DM2_DATA_DIR") ||
        !getenv("FIRESTAFF_DM2_DATA_DIR")[0]) {
        puts("SKIP: FIRESTAFF_DM2_DATA_DIR is not set");
        return 0;
    }
    if (!load_canonical_files(&graphics, &graphics_size,
                              &dungeon_bytes, &dungeon_size)) {
        fputs("FAIL: selected canonical DM2 media is unreadable\n", stderr);
        return 1;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&scene_plan, 0, sizeof(scene_plan));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) != 0) {
        puts("FAIL: canonical DM2 GRAPHICS.DAT/DUNGEON.DAT did not load");
        failures = 1;
        goto done;
    }
    failures += !check(
        dm2_v1_GRAPHICS_DATA_OPEN_receipt(
            graphics, graphics_size, &graphics_open) && graphics_open.valid,
        "real GRAPHICS_DATA_OPEN admits source GDAT");
    for (int level = 0; level < dungeon.level_count; ++level) {
        int style = dm2_v1_dungeon_get_map_graphics_style(&dungeon, level);
        if (style < 0 || style > 0xff) continue;
        memset(&scene_plan, 0, sizeof(scene_plan));
        if (!dm2_v1_gdat_scene_m11_command_plan_build(
                &loader, (uint8_t)style, &scene_plan)) {
            continue;
        }
        if (dm2_v1_weather_gdat_receipt(&loader, (uint8_t)style, &weather) &&
            dm2_v1_dungeon_c_light_map_descriptor_receipt(
                &dungeon, level, &map_descriptor)) {
            selected_level = level;
            selected_style = style;
            break;
        }
        dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
    }
    failures += !check(selected_level >= 0 && selected_style >= 0,
                       "real G1 map style has scene, light, and weather GDAT");
    if (failures) goto done;

    if (map_descriptor.dynamic_light) {
        /* Dynamic maps need live v1e0974/savegame/party/weather state from
         * the original runtime. Do not manufacture those values merely to
         * make a receipt pass against mounted source media. */
        memset(&c_light_source, 0, sizeof(c_light_source));
        failures += !check(
            !dm2_v1_c_light_m11_receipt_build_for_map(
                &scene_light, &map_descriptor, &c_light_source, &c_light),
            "real dynamic map stays blocked without runtime light state");
        goto done;
    }

    failures += !check(
        scene_plan.valid && scene_plan.command_hash != 0u &&
            scene_plan.commands[0].decoded_hash != 0u &&
            scene_plan.commands[1].decoded_hash != 0u,
        "real GRAPHICSSET floor/ceiling material is decoded");
    failures += !check(
        dm2_v1_gdat_scene_light_m11_receipt(&scene_plan, &scene_light) &&
            scene_light.valid && scene_light.graphicsset == selected_style,
        "real GRAPHICSSET scene-light receipt is built");
    memset(&c_light_source, 0, sizeof(c_light_source));
    c_light_source.valid = 1;
    c_light_source.dynamic_map = 0u;
    c_light_source.base_light = 1u;
    c_light_source.darkness_offset = 0u;
    c_light_source.source_state_hash =
        0x52434c54u ^ map_descriptor.descriptor_hash;
    failures += !check(
        dm2_v1_c_light_m11_receipt_build_for_map(
            &scene_light, &map_descriptor, &c_light_source, &c_light) &&
            c_light.valid && c_light.map_descriptor_hash ==
                map_descriptor.descriptor_hash,
        "real DUNGEON.DAT map descriptor binds c_light receipt");
    failures += !check(
        weather.valid && weather.graphicsset == selected_style &&
            weather.command_mask != 0u && weather.receipt_hash != 0u,
        "real ENVIRONMENT command text binds to active GRAPHICSSET");
    failures += !check(
        dm2_v1_weather_runtime_admission_receipt(
            &graphics_open, &weather, NULL, &weather_admission) &&
            weather_admission.valid && weather_admission.source_text_ready &&
            weather_admission.no_fallback_blit,
        "weather runtime admission consumes GRAPHICS_DATA_OPEN and GDAT text");
    memset(&timer_owner, 0, sizeof(timer_owner));
    memset(&renderer, 0, sizeof(renderer));
    memset(&outdoor_m11, 0, sizeof(outdoor_m11));
    failures += !check(
        dm2_v1_weather_set_timer_weather_receipt(1, 1u, &timer_owner) &&
            timer_owner.valid && timer_owner.outdoor && timer_owner.scheduled &&
            weather.commands[0].raw_text &&
            weather.commands[0].byte_count != 0u &&
            weather.commands[0].raw_hash != 0u &&
            !dm2_v1_weather_gdat_outdoor_m11_receipt(
                &weather, &renderer, &timer_owner, &outdoor_m11),
        "outdoor M11 weather keeps real GDAT text closed without complete picture evidence");
    timer_owner.outdoor = 0;
    failures += !check(
        !dm2_v1_weather_gdat_outdoor_m11_receipt(
            &weather, &renderer, &timer_owner, &outdoor_m11),
        "outdoor M11 weather rejects a non-outdoor timer owner");
    memset(&environment, 0, sizeof(environment));
    (void)dm2_v1_weather_gdat_environment_receipt(
        &loader, (uint8_t)selected_style,
        dm2_v1_runtime_g1_scene_map_token(
            selected_level, selected_style,
            dm2_v1_dungeon_is_outdoor(&dungeon, selected_level)),
        dm2_v1_dungeon_is_outdoor(&dungeon, selected_level), 1,
        DM2_V1_ENVIRONMENT_SKY_CLOUDS_MEDIUM,
        DM2_V1_ENVIRONMENT_WET_GROUND_MEDIUM, 1, 0, &environment);
    failures += !check(
        dm2_v1_weather_query_rainfall_param_receipt(
            0x80u, 3u, 1u, &rainfall) &&
            rainfall.valid && rainfall.image_field == 0x74u &&
            rainfall.turn_delta == 2u,
        "QUERY_RAINFALL_PARAM source formula is receipted");
    failures += !check(
        dm2_v1_scene_weather_light_runtime_receipt(
            &scene_light, &c_light, &weather, NULL, &weather_admission,
            environment.valid ? &environment : NULL, &rainfall, &chain) &&
            chain.valid && chain.graphicsset == selected_style &&
            chain.scene_light_hash == scene_light.receipt_hash &&
            chain.c_light_hash == c_light.receipt_hash &&
            chain.weather_receipt_hash == weather.receipt_hash &&
            chain.weather_admission_hash == weather_admission.admission_hash &&
            chain.draw_rain_bound && chain.no_synthetic_weather_fallback,
        "scene/light/weather runtime chain rejects synthetic fallback");
    if (!failures) {
        printf("selected level=%d graphicsset=%d material_mask=0x%08x\n",
               selected_level, selected_style, weather.material_mask);
    }

done:
    dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
    dm2_v1_dungeon_free(&dungeon);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    free(dungeon_bytes);
    return failures ? 1 : 0;
}
