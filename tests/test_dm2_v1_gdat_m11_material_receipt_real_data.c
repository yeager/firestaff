/* Canonical GRAPHICS.DAT proof that M11 consumes one exact floor/ceiling/
 * wall material family. No generated image, fallback surface, or guessed
 * graphics set participates in this receipt. */
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "dm2_v1_gdat_wall_m11_command.h"
#include "m11_dm2_runtime_frame_receipt_gate.h"

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

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    char path[1024];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatSceneM11CommandPlan scene;
    DM2_V1_GdatSceneLightM11Receipt light;
    DM2_V1_GdatWallM11CommandPlan wall;
    DM2_V1_GdatHudM11CommandPlan hud;
    DM2_V1_BootRuntimeRenderReceipt boot;
    DM2_V1_ViewportM11FrameReceipt frame;
    int style = -1;

    if (!root || !root[0]) {
        if (!home || !home[0]) {
            puts("SKIP: no DM2 data root");
            return 0;
        }
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm2/data/graphics.dat", home);
    } else {
        snprintf(path, sizeof(path), "%s/graphics.dat", root);
    }
    if (!read_file(path, &graphics, &graphics_size)) {
        puts("SKIP: no canonical DM2 GRAPHICS.DAT");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&scene, 0, sizeof(scene));
    memset(&light, 0, sizeof(light));
    memset(&wall, 0, sizeof(wall));
    memset(&hud, 0, sizeof(hud));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fputs("FAIL: canonical GRAPHICS.DAT was not admitted\n", stderr);
        free(graphics);
        return 1;
    }
    for (int candidate = 0; candidate < 256; ++candidate) {
        if (dm2_v1_gdat_scene_m11_command_plan_build(&loader, (uint8_t)candidate,
                                                      &scene) &&
            dm2_v1_gdat_wall_m11_command_plan_build(&loader, (uint8_t)candidate,
                                                     &wall)) {
            style = candidate;
            break;
        }
        dm2_v1_gdat_scene_m11_command_plan_free(&scene);
        dm2_v1_gdat_wall_m11_command_plan_free(&wall);
    }
    if (style < 0 || !scene.valid || !wall.valid || scene.command_hash == 0u ||
        scene.commands[0].raw_hash == 0u || scene.commands[1].raw_hash == 0u ||
        !scene.commands[0].material_source_bytes ||
        !scene.commands[1].material_source_bytes ||
        scene.commands[0].material_source_byte_count == 0u ||
        scene.commands[1].material_source_byte_count == 0u ||
        scene.commands[0].material_receipt_hash == 0u ||
        scene.commands[1].material_receipt_hash == 0u ||
        wall.command_hash == 0u) {
        fputs("FAIL: no complete canonical GDAT material family\n", stderr);
        dm2_v1_gdat_scene_m11_command_plan_free(&scene);
        dm2_v1_gdat_wall_m11_command_plan_free(&wall);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    if (!dm2_v1_gdat_hud_m11_command_plan_build(&loader, 0, &hud) ||
        !hud.valid || hud.command_count != 9 || hud.command_hash == 0u) {
        fputs("FAIL: no complete canonical GDAT HUD material plan\n", stderr);
        dm2_v1_gdat_scene_m11_command_plan_free(&scene);
        dm2_v1_gdat_wall_m11_command_plan_free(&wall);
        dm2_v1_gdat_hud_m11_command_plan_free(&hud);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    if (!dm2_v1_gdat_scene_light_m11_receipt(&scene, &light) ||
        !light.valid || light.receipt_hash == 0u ||
        light.scene_control_hash != scene.command_hash ||
        light.ambient_light != scene.ambient_light ||
        light.highest_light_level != scene.highest_light_level ||
        light.ambient_darkness != scene.ambient_darkness) {
        fprintf(stderr,
                "FAIL: canonical scene/light receipt style=%d valid=%d hash=%08x control=%08x/%08x ambient=%u/%u light=%u/%u darkness=%u/%u\n",
                style, light.valid, light.receipt_hash,
                light.scene_control_hash, scene.command_hash,
                light.ambient_light, scene.ambient_light,
                light.highest_light_level, scene.highest_light_level,
                light.ambient_darkness, scene.ambient_darkness);
        dm2_v1_gdat_scene_m11_command_plan_free(&scene);
        dm2_v1_gdat_wall_m11_command_plan_free(&wall);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    memset(&boot, 0, sizeof(boot));
    memset(&frame, 0, sizeof(frame));
    boot.runtime_m11_frame_receipt_consumed = 1;
    boot.runtime_m11_frame_map_load_token = (uint32_t)style + 1u;
    boot.runtime_m11_frame_scene_control_hash = scene.command_hash;
    boot.runtime_m11_frame_scene_light_hash = light.receipt_hash;
    boot.runtime_m11_frame_floor_material_hash = scene.commands[0].raw_hash;
    boot.runtime_m11_frame_ceiling_material_hash = scene.commands[1].raw_hash;
    boot.runtime_m11_frame_wall_material_plan_hash = wall.command_hash;
    boot.runtime_m11_frame_wall_material_plan_command_count =
        wall.command_count;
    boot.runtime_m11_frame_hud_material_plan_required = 1;
    boot.runtime_m11_frame_hud_material_plan_hash = hud.command_hash;
    boot.runtime_m11_frame_hud_scene_control_hash = scene.command_hash;
    boot.runtime_m11_frame_hud_material_plan_command_count = hud.command_count;
    boot.runtime_m11_frame_hud_material_plan_consumed = 1;
    boot.runtime_m11_frame_palette_hash = scene.commands[0].palette_hash;
    boot.runtime_m11_frame_interface_action_palette_hash = 1u;
    boot.runtime_m11_frame_interface_action_palette_consumed = 1;
    boot.runtime_render_asset_floor_ceiling_count = 2;
    boot.runtime_render_no_core_fallbacks = 1;
    frame.valid = 1;
    frame.m11_consume_frame = 1;
    frame.source_materials_required = 1;
    frame.map_load_token = boot.runtime_m11_frame_map_load_token;
    frame.scene_control_hash = scene.command_hash;
    frame.scene_light_hash = light.receipt_hash;
    frame.floor_material_hash = scene.commands[0].raw_hash;
    frame.ceiling_material_hash = scene.commands[1].raw_hash;
    frame.wall_material_plan_hash = wall.command_hash;
    frame.wall_material_plan_command_count = wall.command_count;
    frame.hud_material_plan_required = 1;
    frame.hud_material_plan_hash = hud.command_hash;
    frame.hud_scene_control_hash = scene.command_hash;
    frame.hud_material_plan_command_count = hud.command_count;
    frame.hud_material_plan_consumed = 1;
    frame.palette_hash = scene.commands[0].palette_hash;
    frame.interface_action_palette_hash = 1u;
    frame.interface_action_palette_consumed = 1;
    if (!M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &frame)) {
        fputs("FAIL: M11 rejected canonical floor/wall material receipt\n", stderr);
        return 1;
    }
    frame.floor_material_hash++;
    if (M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &frame)) {
        fputs("FAIL: M11 accepted mismatched canonical floor material\n", stderr);
        return 1;
    }
    frame.floor_material_hash--;
    frame.scene_light_hash++;
    if (M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &frame)) {
        fputs("FAIL: M11 accepted mismatched canonical scene/light receipt\n",
              stderr);
        return 1;
    }
    frame.scene_light_hash--;
    frame.wall_material_plan_hash++;
    if (M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &frame)) {
        fputs("FAIL: M11 accepted mismatched canonical wall material\n", stderr);
        return 1;
    }
    frame.wall_material_plan_hash--;
    frame.hud_material_plan_hash++;
    if (M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &frame)) {
        fputs("FAIL: M11 accepted mismatched canonical HUD material\n", stderr);
        return 1;
    }
    printf("PASS: GDAT style=%d light=%08x floor=%08x ceiling=%08x wall=%08x hud=%08x\n", style,
           light.receipt_hash,
           scene.commands[0].raw_hash, scene.commands[1].raw_hash,
           wall.command_hash, hud.command_hash);
    dm2_v1_gdat_scene_m11_command_plan_free(&scene);
    dm2_v1_gdat_wall_m11_command_plan_free(&wall);
    dm2_v1_gdat_hud_m11_command_plan_free(&hud);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    return 0;
}
