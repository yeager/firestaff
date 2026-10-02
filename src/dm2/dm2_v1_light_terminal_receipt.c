#include "dm2_v1_light_terminal_receipt.h"

#include <stddef.h>

static uint32_t mix_byte(uint32_t hash, uint8_t value)
{
    return (hash ^ value) * 16777619u;
}

static uint32_t mix_u16(uint32_t hash, uint16_t value)
{
    hash = mix_byte(hash, (uint8_t)value);
    return mix_byte(hash, (uint8_t)(value >> 8));
}

static uint32_t mix_u32(uint32_t hash, uint32_t value)
{
    hash = mix_u16(hash, (uint16_t)value);
    return mix_u16(hash, (uint16_t)(value >> 16));
}

uint32_t dm2_v1_light_terminal_visibility_hash(
    const DM2_V1_1c9aLightVisibility *state)
{
    uint32_t hash = 2166136261u;
    if (!state || state->current_map < 0 || state->current_width == 0u ||
        state->current_width > 32u || state->alternate_width > 32u)
        return 0u;
    hash = mix_u16(hash, (uint16_t)state->current_map);
    hash = mix_u16(hash, (uint16_t)state->alternate_map);
    hash = mix_byte(hash, state->current_width);
    hash = mix_byte(hash, state->alternate_width);
    for (size_t i = 0; i < sizeof(state->current); ++i)
        hash = mix_byte(hash, state->current[i]);
    for (size_t i = 0; i < sizeof(state->alternate); ++i)
        hash = mix_byte(hash, state->alternate[i]);
    return hash;
}

uint32_t dm2_v1_light_terminal_accumulator_hash(
    int16_t v1e0974, int16_t v1e0978)
{
    uint32_t hash = mix_u16(2166136261u, (uint16_t)v1e0974);
    return mix_u16(hash, (uint16_t)v1e0978);
}

static int proof_valid(const DM2_V1_LightWalkTerminalProof *proof,
                       uint8_t mode, uint8_t action,
                       const DM2_V1_LightTerminalSourceInputs *inputs)
{
    return proof && proof->valid && proof->mode == mode &&
        proof->action == action && proof->terminal_exhausted &&
        proof->callbacks_authenticated && !proof->unknown_branch_seen &&
        !proof->fallthrough_seen && proof->map == inputs->map &&
        proof->x == inputs->x && proof->y == inputs->y &&
        proof->result_hash != 0u;
}

int dm2_v1_light_terminal_receipt_commit(
    DM2_V1_1c9aLightVisibility *state,
    const DM2_V1_LightWalkTerminalProof *mode8,
    const DM2_V1_LightWalkTerminalProof *mode7,
    const DM2_V1_LightTerminalSourceInputs *inputs)
{
    uint32_t visibility_hash, accumulator_hash, hash;
    if (!state) return 0;
    state->mode8_complete = 0u;
    state->mode7_complete = 0u;
    state->source_state_hash = 0u;
    if (!inputs || inputs->map != state->current_map ||
        inputs->map < 0 || inputs->map >= 64 ||
        inputs->x >= state->current_width || inputs->y >= 32u ||
        inputs->radius > 8u || !inputs->map_descriptor_hash ||
        !inputs->dungeon_hash || !inputs->record_pool_hash ||
        !inputs->gdat_hash || !inputs->party_inventory_hash ||
        !inputs->weather_hash ||
        !proof_valid(mode8, 8u, 0x1bu, inputs) ||
        !proof_valid(mode7, 7u, 0x17u, inputs) ||
        mode8->radius != 25u || mode8->skipped_zero_radius ||
        !mode8->ordered_node_hash || !mode8->source_edge_hash ||
        mode8->rng_after != mode7->rng_before ||
        mode7->rng_after != inputs->rng_final ||
        mode7->radius != inputs->radius ||
        (inputs->radius == 0u ?
            (!mode7->skipped_zero_radius || mode7->ordered_node_hash ||
             mode7->source_edge_hash ||
             mode7->radius_source_receipt_hash != inputs->gdat_hash ||
             mode7->rng_before != mode7->rng_after ||
             state->v1e0974 != 0 || state->v1e0978 != 0) :
            (mode7->skipped_zero_radius || !mode7->ordered_node_hash ||
             !mode7->source_edge_hash || mode7->radius_source_receipt_hash)))
        return 0;
    visibility_hash = dm2_v1_light_terminal_visibility_hash(state);
    accumulator_hash = dm2_v1_light_terminal_accumulator_hash(
        state->v1e0974, state->v1e0978);
    if (!visibility_hash || !accumulator_hash ||
        mode8->result_hash != visibility_hash ||
        mode7->result_hash != accumulator_hash) return 0;

    hash = mix_u32(2166136261u, 0x4c495447u); /* LITG provenance domain. */
    hash = mix_u16(hash, (uint16_t)inputs->map);
    hash = mix_byte(hash, inputs->x);
    hash = mix_byte(hash, inputs->y);
    hash = mix_byte(hash, inputs->radius);
    hash = mix_u16(hash, inputs->savegame_light);
    hash = mix_u16(hash, mode8->rng_before);
    hash = mix_u16(hash, mode8->rng_after);
    hash = mix_u16(hash, mode7->rng_after);
    hash = mix_u32(hash, inputs->map_descriptor_hash);
    hash = mix_u32(hash, inputs->dungeon_hash);
    hash = mix_u32(hash, inputs->record_pool_hash);
    hash = mix_u32(hash, inputs->gdat_hash);
    hash = mix_u32(hash, inputs->party_inventory_hash);
    hash = mix_u32(hash, inputs->weather_hash);
    hash = mix_u32(hash, mode8->ordered_node_hash);
    hash = mix_u32(hash, mode8->source_edge_hash);
    hash = mix_u32(hash, mode7->ordered_node_hash);
    hash = mix_u32(hash, mode7->source_edge_hash);
    hash = mix_u32(hash, visibility_hash);
    hash = mix_u32(hash, accumulator_hash);
    if (!hash) return 0;
    state->mode8_complete = 1u;
    state->mode7_complete = 1u;
    state->source_state_hash = hash;
    return 1;
}
