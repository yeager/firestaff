/* skproject/SKULLWIN/c_gui_vp.cpp DM2_DRAW_WALL routes every visible
 * GRAPHICSSET field and its local palette under one active map context.
 * Runtime marks the atomic receipt invalid when that complete material pass
 * fails; M11 must reject it before it presents any framebuffer surface. */
#include "m11_dm2_runtime_frame_receipt_gate.h"

int M11_Dm2RuntimeFrameReceipt_ShouldPresent(
    const DM2_V1_BootRuntimeRenderReceipt *boot_receipt,
    const DM2_V1_ViewportM11FrameReceipt *runtime_receipt)
{
    if (!boot_receipt || !boot_receipt->runtime_m11_frame_receipt_consumed ||
        boot_receipt->runtime_m11_frame_map_load_token == 0u ||
        boot_receipt->runtime_m11_frame_scene_control_hash == 0u ||
        boot_receipt->runtime_m11_frame_scene_light_hash == 0u ||
        boot_receipt->runtime_m11_frame_floor_material_hash == 0u ||
        boot_receipt->runtime_m11_frame_ceiling_material_hash == 0u ||
        /* dm2_v1_runtime marks outdoor map tokens in bit 31.  UPDATE_GFXSET
         * resolves WALL_GFX for indoor frames; an outdoor frame instead
         * carries its real sky/ground planes and is not made invalid merely
         * because there is no indoor wall plan. */
         (((boot_receipt->runtime_m11_frame_map_load_token &
           UINT32_C(0x80000000)) == 0u) &&
         (boot_receipt->runtime_m11_frame_wall_material_plan_hash == 0u ||
          boot_receipt->runtime_m11_frame_wall_material_plan_command_count <= 0)) ||
        (boot_receipt->runtime_m11_frame_door_material_plan_required &&
         (boot_receipt->runtime_m11_frame_door_material_plan_hash == 0u ||
          boot_receipt->runtime_m11_frame_door_material_plan_command_count <= 0 ||
          !boot_receipt->runtime_m11_frame_door_material_plan_consumed)) ||
        (boot_receipt->runtime_m11_frame_hud_material_plan_required &&
         (boot_receipt->runtime_m11_frame_hud_material_plan_hash == 0u ||
          boot_receipt->runtime_m11_frame_hud_scene_control_hash == 0u ||
          boot_receipt->runtime_m11_frame_hud_material_plan_command_count <= 0 ||
          !boot_receipt->runtime_m11_frame_hud_material_plan_consumed)) ||
        (boot_receipt->runtime_m11_frame_creature_material_plan_required &&
         (boot_receipt->runtime_m11_frame_creature_material_plan_hash == 0u ||
          boot_receipt->runtime_m11_frame_creature_material_plan_command_count <= 0 ||
          !boot_receipt->runtime_m11_frame_creature_material_plan_consumed)) ||
        (boot_receipt->runtime_m11_frame_projectile_material_plan_required &&
         (boot_receipt->runtime_m11_frame_projectile_material_plan_hash == 0u ||
          boot_receipt->runtime_m11_frame_projectile_material_plan_command_count <= 0 ||
          !boot_receipt->runtime_m11_frame_projectile_material_plan_consumed)) ||
        (boot_receipt->runtime_m11_frame_item_material_plan_required &&
         (boot_receipt->runtime_m11_frame_item_material_plan_hash == 0u ||
          boot_receipt->runtime_m11_frame_item_material_plan_command_count <= 0 ||
          !boot_receipt->runtime_m11_frame_item_material_plan_consumed)) ||
        (boot_receipt->runtime_m11_frame_weather_material_plan_required &&
         (boot_receipt->runtime_m11_frame_weather_material_plan_hash == 0u ||
          boot_receipt->runtime_m11_frame_weather_material_plan_command_count <= 0 ||
          !boot_receipt->runtime_m11_frame_weather_material_plan_consumed)) ||
        (boot_receipt->runtime_m11_frame_teleporter_material_plan_required &&
         (boot_receipt->runtime_m11_frame_teleporter_material_plan_hash == 0u ||
          !boot_receipt->runtime_m11_frame_teleporter_material_plan_consumed)) ||
        (boot_receipt->runtime_m11_frame_floor_gfx_map_chip_material_plan_required &&
         (boot_receipt->runtime_m11_frame_floor_gfx_map_chip_material_plan_hash == 0u ||
          !boot_receipt->runtime_m11_frame_floor_gfx_map_chip_material_plan_consumed)) ||
        (boot_receipt->runtime_m11_frame_wall_gfx_map_chip_material_plan_required &&
         (boot_receipt->runtime_m11_frame_wall_gfx_map_chip_material_plan_hash == 0u ||
          !boot_receipt->runtime_m11_frame_wall_gfx_map_chip_material_plan_consumed)) ||
        (boot_receipt->runtime_m11_frame_door_map_chip_material_plan_required &&
         (boot_receipt->runtime_m11_frame_door_map_chip_material_plan_hash == 0u ||
          !boot_receipt->runtime_m11_frame_door_map_chip_material_plan_consumed)) ||
        boot_receipt->runtime_m11_frame_palette_hash == 0u ||
        boot_receipt->runtime_m11_frame_interface_action_palette_hash == 0u ||
        !boot_receipt->runtime_m11_frame_interface_action_palette_consumed ||
        /* skproject DRAW_DUNGEON resolves both GRAPHICSSET ceiling and floor
         * before M11 owns the frame.  A zero-fallback identity alone is not
         * evidence: both real planes must have been consumed and no source
         * material pass may have blocked. */
        boot_receipt->runtime_render_asset_floor_ceiling_count < 2 ||
        boot_receipt->runtime_render_fallback_floor_ceiling_count != 0 ||
        boot_receipt->runtime_render_blocked_material_draw_count != 0 ||
        !boot_receipt->runtime_render_no_core_fallbacks ||
        !runtime_receipt) {
        return 0;
    }
    return runtime_receipt->source_materials_required &&
        runtime_receipt->valid && runtime_receipt->m11_consume_frame &&
        runtime_receipt->map_load_token ==
            boot_receipt->runtime_m11_frame_map_load_token &&
        runtime_receipt->scene_control_hash ==
            boot_receipt->runtime_m11_frame_scene_control_hash &&
        runtime_receipt->scene_light_hash ==
            boot_receipt->runtime_m11_frame_scene_light_hash &&
        runtime_receipt->floor_material_hash ==
            boot_receipt->runtime_m11_frame_floor_material_hash &&
        runtime_receipt->ceiling_material_hash ==
            boot_receipt->runtime_m11_frame_ceiling_material_hash &&
        runtime_receipt->wall_material_plan_hash ==
            boot_receipt->runtime_m11_frame_wall_material_plan_hash &&
        runtime_receipt->wall_material_plan_command_count > 0 &&
        runtime_receipt->wall_material_plan_command_count ==
            boot_receipt->runtime_m11_frame_wall_material_plan_command_count &&
        runtime_receipt->door_material_plan_required ==
            boot_receipt->runtime_m11_frame_door_material_plan_required &&
        (!runtime_receipt->door_material_plan_required ||
         (runtime_receipt->door_material_plan_hash != 0u &&
          runtime_receipt->door_material_plan_consumed &&
          runtime_receipt->door_material_plan_hash ==
              boot_receipt->runtime_m11_frame_door_material_plan_hash &&
          runtime_receipt->door_material_plan_command_count > 0 &&
          runtime_receipt->door_material_plan_command_count ==
              boot_receipt->runtime_m11_frame_door_material_plan_command_count)) &&
        runtime_receipt->hud_material_plan_required ==
            boot_receipt->runtime_m11_frame_hud_material_plan_required &&
        (!runtime_receipt->hud_material_plan_required ||
         (runtime_receipt->hud_material_plan_hash != 0u &&
          runtime_receipt->hud_scene_control_hash == runtime_receipt->scene_control_hash &&
          runtime_receipt->hud_scene_control_hash == boot_receipt->runtime_m11_frame_hud_scene_control_hash &&
          runtime_receipt->hud_material_plan_command_count > 0 &&
          runtime_receipt->hud_material_plan_consumed &&
          runtime_receipt->hud_material_plan_hash ==
              boot_receipt->runtime_m11_frame_hud_material_plan_hash &&
          runtime_receipt->hud_material_plan_command_count ==
              boot_receipt->runtime_m11_frame_hud_material_plan_command_count)) &&
        runtime_receipt->creature_material_plan_required ==
            boot_receipt->runtime_m11_frame_creature_material_plan_required &&
        (!runtime_receipt->creature_material_plan_required ||
         (runtime_receipt->creature_material_plan_hash != 0u &&
          runtime_receipt->creature_material_plan_command_count > 0 &&
          runtime_receipt->creature_material_plan_consumed &&
          runtime_receipt->creature_material_plan_hash ==
              boot_receipt->runtime_m11_frame_creature_material_plan_hash &&
          runtime_receipt->creature_material_plan_command_count ==
              boot_receipt->runtime_m11_frame_creature_material_plan_command_count)) &&
        runtime_receipt->projectile_material_plan_required ==
            boot_receipt->runtime_m11_frame_projectile_material_plan_required &&
        (!runtime_receipt->projectile_material_plan_required ||
         (runtime_receipt->projectile_material_plan_hash != 0u &&
          runtime_receipt->projectile_material_plan_command_count > 0 &&
          runtime_receipt->projectile_material_plan_consumed &&
          runtime_receipt->projectile_material_plan_hash ==
              boot_receipt->runtime_m11_frame_projectile_material_plan_hash &&
          runtime_receipt->projectile_material_plan_command_count ==
              boot_receipt->runtime_m11_frame_projectile_material_plan_command_count)) &&
        runtime_receipt->item_material_plan_required ==
            boot_receipt->runtime_m11_frame_item_material_plan_required &&
        (!runtime_receipt->item_material_plan_required ||
         (runtime_receipt->item_material_plan_hash != 0u &&
          runtime_receipt->item_material_plan_command_count > 0 &&
          runtime_receipt->item_material_plan_consumed &&
          runtime_receipt->item_material_plan_hash ==
              boot_receipt->runtime_m11_frame_item_material_plan_hash &&
          runtime_receipt->item_material_plan_command_count ==
              boot_receipt->runtime_m11_frame_item_material_plan_command_count)) &&
        runtime_receipt->weather_material_plan_required ==
            boot_receipt->runtime_m11_frame_weather_material_plan_required &&
        (!runtime_receipt->weather_material_plan_required ||
         (runtime_receipt->weather_material_plan_hash != 0u &&
          runtime_receipt->weather_material_plan_command_count > 0 &&
          runtime_receipt->weather_material_plan_consumed &&
          runtime_receipt->weather_material_plan_hash ==
              boot_receipt->runtime_m11_frame_weather_material_plan_hash &&
          runtime_receipt->weather_material_plan_command_count ==
              boot_receipt->runtime_m11_frame_weather_material_plan_command_count)) &&
        runtime_receipt->teleporter_material_plan_required ==
            boot_receipt->runtime_m11_frame_teleporter_material_plan_required &&
        (!runtime_receipt->teleporter_material_plan_required ||
         (runtime_receipt->teleporter_material_plan_hash != 0u &&
          runtime_receipt->teleporter_material_plan_consumed &&
          runtime_receipt->teleporter_material_plan_hash ==
              boot_receipt->runtime_m11_frame_teleporter_material_plan_hash)) &&
        runtime_receipt->floor_gfx_map_chip_material_plan_required ==
            boot_receipt->runtime_m11_frame_floor_gfx_map_chip_material_plan_required &&
        (!runtime_receipt->floor_gfx_map_chip_material_plan_required ||
         (runtime_receipt->floor_gfx_map_chip_material_plan_hash != 0u &&
          runtime_receipt->floor_gfx_map_chip_material_plan_consumed &&
          runtime_receipt->floor_gfx_map_chip_material_plan_hash ==
              boot_receipt->runtime_m11_frame_floor_gfx_map_chip_material_plan_hash)) &&
        runtime_receipt->wall_gfx_map_chip_material_plan_required ==
            boot_receipt->runtime_m11_frame_wall_gfx_map_chip_material_plan_required &&
        (!runtime_receipt->wall_gfx_map_chip_material_plan_required ||
         (runtime_receipt->wall_gfx_map_chip_material_plan_hash != 0u &&
          runtime_receipt->wall_gfx_map_chip_material_plan_consumed &&
          runtime_receipt->wall_gfx_map_chip_material_plan_hash ==
              boot_receipt->runtime_m11_frame_wall_gfx_map_chip_material_plan_hash)) &&
        runtime_receipt->door_map_chip_material_plan_required ==
            boot_receipt->runtime_m11_frame_door_map_chip_material_plan_required &&
        (!runtime_receipt->door_map_chip_material_plan_required ||
         (runtime_receipt->door_map_chip_material_plan_hash != 0u &&
          runtime_receipt->door_map_chip_material_plan_consumed &&
          runtime_receipt->door_map_chip_material_plan_hash ==
              boot_receipt->runtime_m11_frame_door_map_chip_material_plan_hash)) &&
        runtime_receipt->palette_hash ==
            boot_receipt->runtime_m11_frame_palette_hash &&
        runtime_receipt->interface_action_palette_hash ==
            boot_receipt->runtime_m11_frame_interface_action_palette_hash &&
        runtime_receipt->interface_action_palette_consumed;
}
