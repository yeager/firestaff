#include "dm2_v1_runtime.h"

uint32_t dm2_v1_runtime_frame_presentation_state_hash(
    uint32_t scene_light_hash, uint16_t ambient_light,
    int weather_graphicsset_bound, uint8_t weather_graphicsset,
    uint32_t weather_source_receipt_hash,
    uint32_t weather_destination_receipt_hash,
    uint32_t weather_material_hash)
{
    uint32_t hash = 2166136261u;
#define DM2_PRESENTATION_MIX(value_) \
    do { hash ^= (uint32_t)(value_); hash *= 16777619u; } while (0)
    if (!scene_light_hash || (weather_graphicsset_bound &&
        (!weather_source_receipt_hash || !weather_destination_receipt_hash ||
         !weather_material_hash))) return 0u;
    DM2_PRESENTATION_MIX(scene_light_hash);
    DM2_PRESENTATION_MIX(ambient_light);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? 1u : 0u);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? weather_graphicsset : 0u);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? weather_source_receipt_hash : 0u);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? weather_destination_receipt_hash : 0u);
    DM2_PRESENTATION_MIX(weather_graphicsset_bound ? weather_material_hash : 0u);
#undef DM2_PRESENTATION_MIX
    return hash ? hash : 1u;
}
