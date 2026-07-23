#include "dm2_v1_runtime.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static void make_plans(DM2_V1_Dm2ViewportM11LivePlanSet *plans,
                       DM2_V1_GdatSceneM11CommandPlan *scene,
                       DM2_V1_GdatSceneLightM11Receipt *scene_light,
                       DM2_V1_CLightM11Receipt *c_light,
                       DM2_V1_GdatWallM11CommandPlan *wall,
                       DM2_V1_GdatDoorOverlayM11CommandPlan *door,
                       DM2_V1_OutdoorWeatherM11Receipt *weather,
                       DM2_V1_StaticObjectM11DeliveryPlan *static_object,
                       DM2_V1_FlyingItemM11DeliveryPlan *flying_item,
                       uint32_t session_identity)
{
    memset(scene, 0, sizeof(*scene)); memset(scene_light, 0, sizeof(*scene_light));
    memset(c_light, 0, sizeof(*c_light)); memset(wall, 0, sizeof(*wall));
    memset(door, 0, sizeof(*door)); memset(weather, 0, sizeof(*weather));
    memset(static_object, 0, sizeof(*static_object)); memset(flying_item, 0, sizeof(*flying_item));
    scene->valid = 1; scene->graphicsset = 2; scene->command_hash = 11; scene->draw_order_hash = 12;
    scene_light->valid = 1; scene_light->graphicsset = 2; scene_light->scene_control_hash = 13; scene_light->receipt_hash = 14;
    c_light->valid = 1; c_light->graphicsset = 2; c_light->scene_control_hash = 13; c_light->source_state_hash = 15; c_light->receipt_hash = 16;
    wall->valid = 1; wall->graphicsset = 2; wall->command_count = 1; wall->command_hash = 17;
    door->command_count = 1;
    door->commands[0].kind = DM2_V1_GDAT_DOOR_PANEL;
    door->commands[0].view_square = DM2_SQ_D0C;
    door->commands[0].field = 0u;
    door->commands[0].draw_distance = 0u;
    door->commands[0].stretch_dual = 0x71u;
    door->commands[0].light_palette = 0u;
    door->commands[0].raw_hash = 18; door->commands[0].decoded_hash = 19;
    door->commands[0].palette_hash = 20; door->commands[0].material_receipt_hash = 21;
    door->commands[0].selection_hash = 22; door->commands[0].geometry_hash = 23;
    door->commands[0].palette_transform_hash = 24;
    (void)dm2_v1_gdat_door_overlay_m11_command_plan_refresh_hash(door);
    weather->valid = 1; weather->graphicsset = 2; weather->timer_owner_hash = 25;
    weather->weather_receipt_hash = 26; weather->renderer_hash = 27;
    weather->raw_material_hash = 28; weather->command_count = 1; weather->receipt_hash = 29;
    static_object->valid = 1; static_object->no_draw = 1;
    static_object->m11_delivery_ready = 1; static_object->session_identity = session_identity;
    static_object->identity_hash = 30;
    flying_item->valid = 1; flying_item->no_draw = 1;
    flying_item->m11_delivery_ready = 1; flying_item->identity_hash = 31;
    plans->scene = scene; plans->scene_light = scene_light; plans->c_light = c_light;
    plans->wall = wall; plans->door = door; plans->weather = weather;
    plans->static_object = static_object; plans->flying_item = flying_item;
}

int main(void)
{
    DM2_V1_SessionState session;
    DM2_V1_Dm2ViewportM11LivePlanSet plans;
    DM2_V1_GdatSceneM11CommandPlan scene;
    DM2_V1_GdatSceneLightM11Receipt scene_light;
    DM2_V1_CLightM11Receipt c_light;
    DM2_V1_GdatWallM11CommandPlan wall;
    DM2_V1_GdatDoorOverlayM11CommandPlan door;
    DM2_V1_OutdoorWeatherM11Receipt weather;
    DM2_V1_StaticObjectM11DeliveryPlan static_object;
    DM2_V1_FlyingItemM11DeliveryPlan flying_item;
    DM2_V1_Dm2ViewportM11LiveCompositionReceipt receipt;
    DM2_V1_ViewportState owner;
    uint8_t framebuffer[320u * 200u];
    uint32_t session_identity;
    int ok;

    dm2_v1_session_new(&session);
    session.party_level = 1; session.party_dir = 2;
    dm2_v1_viewport_init(&owner, framebuffer, 320);
    session_identity = dm2_v1_runtime_dm2_viewport_session_identity(&session);
    memset(&plans, 0, sizeof(plans));
    make_plans(&plans, &scene, &scene_light, &c_light, &wall, &door, &weather,
               &static_object, &flying_item, session_identity);
    ok = session_identity && dm2_v1_runtime_enumerate_dm2_viewport_m11_live_plans(
        &session, 2, &plans, &receipt, &owner) && receipt.valid && receipt.no_draw &&
        receipt.map_load_token == dm2_v1_runtime_g1_scene_map_token(1, 2, 0) &&
        dm2_v1_runtime_dm2_viewport_m11_live_composition_matches(
            &receipt, &session, 2, &plans, &owner);
    session.game_tick++;
    ok &= !dm2_v1_runtime_dm2_viewport_m11_live_composition_matches(
        &receipt, &session, 2, &plans, &owner);
    session.game_tick--;
    static_object.session_identity = dm2_v1_runtime_dm2_viewport_session_identity(&session);
    wall.command_hash ^= 1u;
    ok &= !dm2_v1_runtime_dm2_viewport_m11_live_composition_matches(
        &receipt, &session, 2, &plans, &owner);
    wall.command_hash ^= 1u;
    scene.graphicsset = 3;
    ok &= !dm2_v1_runtime_enumerate_dm2_viewport_m11_live_plans(&session, 2, &plans, &receipt, &owner);
    scene.graphicsset = 2;
    plans.weather = NULL;
    ok &= !dm2_v1_runtime_enumerate_dm2_viewport_m11_live_plans(&session, 2, &plans, &receipt, &owner);
    if (!ok) fputs("DM2 live M11 plan enumeration failed\n", stderr);
    return ok ? 0 : 1;
}
