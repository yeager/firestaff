#include "theron_v1_track02_actuator.h"

#include <string.h>

int theron_v1_track02_actuator_decode(
    const uint8_t *raw8, Theron_Actuator *out)
{
    if (!raw8 || !out) return -1;

    /* Byte layout (8 bytes = 4 × 16-bit LE words):
     *   w0 (bytes 0-1): next reference link (id:10, category:4, position:2)
     *   w1 (bytes 2-3): type(7 bits) + value(9 bits)
     *   w2 (bytes 4-5): DM/CSB sensor flags (once, two-bit effect,
     *                    revert, sound, delay, local effect, ornament)
     *   w3 (bytes 6-7): target (facing, x, y) */
    uint16_t w0 = (uint16_t)raw8[0] | ((uint16_t)raw8[1] << 8);
    uint16_t w1 = (uint16_t)raw8[2] | ((uint16_t)raw8[3] << 8);
    uint16_t w2 = (uint16_t)raw8[4] | ((uint16_t)raw8[5] << 8);
    uint16_t w3 = (uint16_t)raw8[6] | ((uint16_t)raw8[7] << 8);

    out->next_ref = w0;

    out->type   = (uint8_t)(w1 & 0x7F);
    out->value  = (uint16_t)((w1 >> 7) & 0x1FF);

    out->once     = (w2 >> 2) & 1;
    out->effect   = (w2 >> 3) & 3;
    out->revert_effect = (w2 >> 5) & 1;
    out->sound    = (w2 >> 6) & 1;
    out->delay    = (w2 >> 7) & 0xF;
    out->local_effect = (w2 >> 11) & 1;
    out->graphism = (w2 >> 12) & 0xF;

    out->target_facing = (w3 >> 4) & 3;
    out->target_x      = (w3 >> 6) & 0x1F;
    out->target_y      = (w3 >> 11) & 0x1F;
    out->local_multiple = w3 & 0x0FFFu;

    /* DMBUILDER6/src/actuator.h identifies w3 as a distinct
     * dm_actuator_monster_regenerator overlay for floor type 6:
     * low byte = toughness and high byte = pause.  This is a record-layout
     * decode, not an inferred spawn algorithm.  The HuC6280 consumer remains
     * source-gated by the disassembly capture (theron-us-spawn-consumer.asm). */
    out->generator_fields_valid = (out->type == TQ_ACT_FLOOR_MONSTER_GEN);
    out->generator_generation = out->generator_fields_valid
        ? (uint8_t)((w2 >> 7) & 0x0F) : 0;
    out->generator_toughness = out->generator_fields_valid
        ? raw8[6] : 0;
    out->generator_pause = out->generator_fields_valid
        ? raw8[7] : 0;

    return 0;
}

int theron_v1_track02_actuator_needs_value_fix(
    uint8_t type, int is_wall)
{
    if (is_wall) {
        return type == TQ_ACT_WALL_ALCOVE_ITEM ||
               type == TQ_ACT_WALL_ITEM_EATER ||
               type == TQ_ACT_WALL_ITEM ||
               type == TQ_ACT_WALL_ITEM_EATER_TOGGLE;
    }
    return type == TQ_ACT_FLOOR_CARRIED_ITEM;
}

int theron_v1_track02_actuator_generator_plan(
    const Theron_Actuator *actuator, Theron_ActuatorGeneratorPlan *out)
{
    uint8_t generation;
    if (out) memset(out, 0, sizeof(*out));
    if (!actuator || !out || actuator->type != TQ_ACT_FLOOR_MONSTER_GEN ||
        !actuator->generator_fields_valid)
        return 0;
    generation = actuator->generator_generation;
    if ((generation & 0x08u) != 0u) {
        out->random_count_bound = generation & 0x07u;
        if (out->random_count_bound == 0u) return 0;
        out->count_is_random = 1u;
    } else {
        if (generation == 0u || generation > 4u) return 0;
        out->fixed_count_minus_one = (uint8_t)(generation - 1u);
    }
    out->creature_type_value = actuator->value;
    out->toughness = actuator->generator_toughness;
    out->pause = actuator->generator_pause;
    return 1;
}

int theron_v1_track02_actuator_evaluate_party_event(
    const Theron_Actuator *actuator, int is_addition,
    int party_already_on_square, int champion_count,
    unsigned int party_direction, Theron_ActuatorPartyEvent *out)
{
    int condition;

    if (!actuator || !out || party_direction > 3u || champion_count < 0)
        return -1;
    out->triggered = 0u;
    out->resolved_effect = actuator->effect;
    out->disable_after_dispatch = 0u;

    if (actuator->type != TQ_ACT_FLOOR_PARTY || champion_count == 0)
        return 0;

    if (actuator->value == 0u) {
        if (party_already_on_square) return 0;
        condition = is_addition;
    } else {
        condition = is_addition &&
            actuator->value == (uint16_t)(party_direction + 1u);
    }
    condition = !!condition ^ !!actuator->revert_effect;

    if (actuator->effect == TQ_ACT_EFFECT_HOLD) {
        out->triggered = 1u;
        out->resolved_effect = condition
            ? TQ_ACT_EFFECT_SET : TQ_ACT_EFFECT_CLEAR;
    } else if (condition) {
        out->triggered = 1u;
    }
    if (out->triggered)
        out->disable_after_dispatch = actuator->once != 0u;
    return 0;
}
