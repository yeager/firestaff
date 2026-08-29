/* Canonical GRAPHICS.DAT proof that M11 consumes one exact floor/ceiling/
 * wall material family. No generated image, fallback surface, or guessed
 * graphics set participates in this receipt. */
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "dm2_v1_gdat_wall_m11_command.h"
#include "m11_dm2_runtime_frame_receipt_gate.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_original_member(const char *archive, const char *name,
                                uint8_t **out, size_t *out_size)
{
    if (!archive || !archive[0] || !name || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    return firestaff_zip_extract_by_suffix(archive, name, out, out_size) == 0 &&
           *out && *out_size;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
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

    if (!archive || !archive[0]) {
        puts("SKIP: no DM2 DOS archive is configured");
        return 0;
    }
    if (!read_original_member(archive, "data/graphics.dat", &graphics,
                              &graphics_size)) {
        fprintf(stderr,
                "FAIL: original DM2 data/graphics.dat is unreadable from %s\n",
                archive);
        return 1;
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
        !hud.valid || hud.command_count !=
            DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT ||
        hud.command_hash == 0u) {
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
    boot.runtime_m11_frame_interface_action_palette_required = 1;
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
    frame.interface_action_palette_required = 1;
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
