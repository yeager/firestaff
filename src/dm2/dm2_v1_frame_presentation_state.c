#include "dm2_v1_runtime.h"

#include <stdlib.h>
#include <string.h>

static uint32_t dm2_v1_composition_mix(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static int dm2_v1_composition_members_valid(
    const DM2_V1_GdatSceneM11CommandPlan *scene,
    const DM2_V1_GdatSceneLightM11Receipt *scene_light,
    const DM2_V1_CLightM11Receipt *c_light,
    const DM2_V1_GdatWallM11CommandPlan *wall,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *door,
    const DM2_V1_OutdoorWeatherM11Receipt *weather,
    const DM2_V1_StaticObjectM11DeliveryPlan *static_object,
    const DM2_V1_FlyingItemM11DeliveryPlan *flying_item,
    uint32_t session_identity, uint32_t data_epoch)
{
    return scene && scene_light && c_light && wall && door && weather &&
        static_object && flying_item && session_identity && data_epoch &&
        scene->valid && scene->command_hash && scene->draw_order_hash &&
        scene_light->valid && scene_light->receipt_hash &&
        scene_light->scene_control_hash &&
        c_light->valid && c_light->receipt_hash && c_light->source_state_hash &&
        c_light->scene_control_hash == scene_light->scene_control_hash &&
        wall->valid && wall->command_count && wall->command_hash &&
        door->valid && door->command_count && door->command_hash &&
        dm2_v1_gdat_door_overlay_m11_command_plan_draw_controls_valid(door) &&
        weather->valid && weather->receipt_hash && weather->timer_owner_hash &&
        weather->weather_receipt_hash && weather->renderer_hash &&
        weather->raw_material_hash && weather->command_count &&
        static_object->valid && static_object->no_draw &&
        !static_object->pixel_decoder_ready && static_object->m11_delivery_ready &&
        static_object->session_identity == session_identity &&
        static_object->identity_hash && flying_item->valid && flying_item->no_draw &&
        !flying_item->pixel_decoder_ready && flying_item->m11_delivery_ready &&
        flying_item->identity_hash;
}

uint32_t dm2_v1_runtime_dm2_viewport_session_identity(
    const DM2_V1_SessionState *session)
{
    uint8_t *serialized;
    int size;
    uint32_t hash = 2166136261u;
    if (!session || !dm2_v1_session_validate(session)) return 0u;
    serialized = (uint8_t *)malloc(DM2_SESSION_MAX_SIZE);
    if (!serialized) return 0u;
    size = dm2_v1_session_serialize(session, serialized, DM2_SESSION_MAX_SIZE);
    if (size > 0) {
        for (int i = 0; i < size; ++i)
            hash = dm2_v1_composition_mix(hash, serialized[i]);
    }
    free(serialized);
    return size > 0 ? (hash ? hash : 1u) : 0u;
}

int dm2_v1_runtime_enumerate_dm2_viewport_m11_live_plans(
    const DM2_V1_SessionState *session, uint8_t graphicsset,
    const DM2_V1_Dm2ViewportM11LivePlanSet *plans,
    DM2_V1_Dm2ViewportM11LiveCompositionReceipt *out_receipt,
    const DM2_V1_ViewportState *owner)
{
    DM2_V1_Dm2ViewportM11LiveCompositionReceipt receipt;
    uint32_t session_identity;
    uint32_t map_load_token;
    int outdoor;
    if (!out_receipt || !owner) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!session || !plans || session->party_dir > 3u) return 0;
    if (!dm2_v1_viewport_surface_snapshot(owner,
                                           &receipt.composition.surface_before)) return 0;
    outdoor = session->outdoor_mode == 1u;
    session_identity = dm2_v1_runtime_dm2_viewport_session_identity(session);
    map_load_token = dm2_v1_runtime_g1_scene_map_token(
        session->party_level, graphicsset, outdoor);
    if (!session_identity || !map_load_token || !plans->scene ||
        !plans->scene_light || !plans->c_light || !plans->wall ||
        !plans->door || !plans->weather || !plans->static_object ||
        !plans->flying_item || plans->scene->graphicsset != graphicsset ||
        plans->scene_light->graphicsset != graphicsset ||
        plans->c_light->graphicsset != graphicsset ||
        plans->wall->graphicsset != graphicsset ||
        plans->weather->graphicsset != graphicsset ||
        plans->static_object->session_identity != session_identity) return 0;
    {
        DM2_V1_ViewportSurfaceSnapshot surface_before =
            receipt.composition.surface_before;
        if (!dm2_v1_runtime_build_dm2_viewport_m11_composition(
                plans->scene, plans->scene_light, plans->c_light, plans->wall,
                plans->door, plans->weather, plans->static_object,
                plans->flying_item, session_identity, map_load_token,
                &receipt.composition)) {
            return 0;
        }
        receipt.composition.surface_before = surface_before;
    }
    if (!dm2_v1_viewport_surface_snapshot(owner,
                                           &receipt.composition.surface_after) ||
        receipt.composition.surface_before.framebuffer !=
            receipt.composition.surface_after.framebuffer ||
        receipt.composition.surface_before.generation !=
            receipt.composition.surface_after.generation) {
        return 0;
    }
    receipt.composition.surface_generation_hash = dm2_v1_composition_mix(
        receipt.composition.surface_before.generation,
        receipt.composition.surface_after.generation);
    receipt.valid = 1;
    receipt.no_draw = 1;
    receipt.graphicsset = graphicsset;
    receipt.outdoor = (uint8_t)outdoor;
    receipt.level = session->party_level;
    receipt.session_identity = session_identity;
    receipt.map_load_token = map_load_token;
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_runtime_dm2_viewport_m11_live_composition_matches(
    const DM2_V1_Dm2ViewportM11LiveCompositionReceipt *receipt,
    const DM2_V1_SessionState *session, uint8_t graphicsset,
    const DM2_V1_Dm2ViewportM11LivePlanSet *plans,
    const DM2_V1_ViewportState *owner)
{
    DM2_V1_Dm2ViewportM11LiveCompositionReceipt candidate;
    return receipt && receipt->valid && receipt->no_draw &&
        dm2_v1_runtime_enumerate_dm2_viewport_m11_live_plans(
            session, graphicsset, plans, &candidate, owner) &&
        candidate.session_identity == receipt->session_identity &&
        candidate.map_load_token == receipt->map_load_token &&
        candidate.composition.identity_hash == receipt->composition.identity_hash &&
        candidate.composition.surface_generation_hash ==
            receipt->composition.surface_generation_hash &&
        candidate.composition.surface_before.generation ==
            receipt->composition.surface_before.generation &&
        candidate.composition.surface_after.generation ==
            receipt->composition.surface_after.generation;
}

int dm2_v1_runtime_build_dm2_viewport_m11_composition(
    const DM2_V1_GdatSceneM11CommandPlan *scene,
    const DM2_V1_GdatSceneLightM11Receipt *scene_light,
    const DM2_V1_CLightM11Receipt *c_light,
    const DM2_V1_GdatWallM11CommandPlan *wall,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *door,
    const DM2_V1_OutdoorWeatherM11Receipt *weather,
    const DM2_V1_StaticObjectM11DeliveryPlan *static_object,
    const DM2_V1_FlyingItemM11DeliveryPlan *flying_item,
    uint32_t session_identity, uint32_t data_epoch,
    DM2_V1_Dm2ViewportM11CompositionReceipt *out_receipt)
{
    DM2_V1_Dm2ViewportM11CompositionReceipt receipt;
    uint32_t ordered_hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_composition_members_valid(
            scene, scene_light, c_light, wall, door, weather, static_object,
            flying_item, session_identity, data_epoch)) return 0;

    /* Source-owned viewport order: scene/light, walls, doors, statics,
     * flying items, then the outdoor weather overlay. */
    ordered_hash = dm2_v1_composition_mix(ordered_hash, scene->command_hash);
    ordered_hash = dm2_v1_composition_mix(ordered_hash, scene_light->receipt_hash);
    ordered_hash = dm2_v1_composition_mix(ordered_hash, c_light->receipt_hash);
    ordered_hash = dm2_v1_composition_mix(ordered_hash, wall->command_hash);
    ordered_hash = dm2_v1_composition_mix(ordered_hash, door->command_hash);
    ordered_hash = dm2_v1_composition_mix(ordered_hash, static_object->identity_hash);
    ordered_hash = dm2_v1_composition_mix(ordered_hash, flying_item->identity_hash);
    ordered_hash = dm2_v1_composition_mix(ordered_hash, weather->receipt_hash);

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.no_draw = 1;
    receipt.m11_delivery_ready = 1;
    receipt.session_identity = session_identity;
    receipt.data_epoch = data_epoch;
    receipt.scene_command_hash = scene->command_hash;
    receipt.scene_light_receipt_hash = scene_light->receipt_hash;
    receipt.c_light_receipt_hash = c_light->receipt_hash;
    receipt.wall_command_hash = wall->command_hash;
    receipt.door_command_hash = door->command_hash;
    receipt.weather_receipt_hash = weather->receipt_hash;
    receipt.static_object_identity_hash = static_object->identity_hash;
    receipt.flying_item_identity_hash = flying_item->identity_hash;
    receipt.ordered_member_hash = ordered_hash;
    receipt.identity_hash = dm2_v1_composition_mix(ordered_hash, session_identity);
    receipt.identity_hash = dm2_v1_composition_mix(receipt.identity_hash, data_epoch);
    if (!receipt.identity_hash) receipt.identity_hash = 1u;
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_runtime_dm2_viewport_m11_composition_matches(
    const DM2_V1_Dm2ViewportM11CompositionReceipt *receipt,
    const DM2_V1_GdatSceneM11CommandPlan *scene,
    const DM2_V1_GdatSceneLightM11Receipt *scene_light,
    const DM2_V1_CLightM11Receipt *c_light,
    const DM2_V1_GdatWallM11CommandPlan *wall,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *door,
    const DM2_V1_OutdoorWeatherM11Receipt *weather,
    const DM2_V1_StaticObjectM11DeliveryPlan *static_object,
    const DM2_V1_FlyingItemM11DeliveryPlan *flying_item,
    uint32_t session_identity, uint32_t data_epoch)
{
    DM2_V1_Dm2ViewportM11CompositionReceipt candidate;
    return receipt && receipt->valid &&
        dm2_v1_runtime_build_dm2_viewport_m11_composition(
            scene, scene_light, c_light, wall, door, weather, static_object,
            flying_item, session_identity, data_epoch, &candidate) &&
        candidate.identity_hash == receipt->identity_hash &&
        candidate.ordered_member_hash == receipt->ordered_member_hash;
}

uint32_t dm2_v1_runtime_frame_presentation_state_hash(
    uint32_t scene_light_hash, uint16_t ambient_light,
    uint32_t c_light_receipt_hash, uint32_t c_light_source_state_hash,
    uint8_t c_light_level, int c_light_consumed,
    int weather_graphicsset_bound, uint8_t weather_graphicsset,
    uint32_t weather_source_receipt_hash,
    uint32_t weather_destination_receipt_hash,
    uint32_t weather_material_hash)
{
    uint32_t hash = 2166136261u;
#define DM2_PRESENTATION_MIX(value_) \
    do { hash ^= (uint32_t)(value_); hash *= 16777619u; } while (0)
    if (!scene_light_hash ||
        ((c_light_receipt_hash != 0u || c_light_source_state_hash != 0u ||
          c_light_level != 0u || c_light_consumed) &&
         (!c_light_receipt_hash || !c_light_source_state_hash ||
          c_light_level > 5u || !c_light_consumed)) ||
        (weather_graphicsset_bound &&
        (!weather_source_receipt_hash || !weather_destination_receipt_hash ||
         !weather_material_hash))) return 0u;
    DM2_PRESENTATION_MIX(scene_light_hash);
    DM2_PRESENTATION_MIX(ambient_light);
    DM2_PRESENTATION_MIX(c_light_receipt_hash);
    DM2_PRESENTATION_MIX(c_light_source_state_hash);
    DM2_PRESENTATION_MIX(c_light_level);
    DM2_PRESENTATION_MIX(c_light_consumed ? 1u : 0u);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? 1u : 0u);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? weather_graphicsset : 0u);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? weather_source_receipt_hash : 0u);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? weather_destination_receipt_hash : 0u);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? weather_material_hash : 0u);
#undef DM2_PRESENTATION_MIX
    return hash ? hash : 1u;
}
