#ifndef THERON_V1_TRACK02_ACTUATOR_H
#define THERON_V1_TRACK02_ACTUATOR_H

#include <stdint.h>

typedef enum {
    TQ_ACT_FLOOR_NONE            = 0,
    TQ_ACT_FLOOR_EVERYTHING      = 1,
    TQ_ACT_FLOOR_PARTY_MONSTER   = 2,
    TQ_ACT_FLOOR_PARTY           = 3,
    TQ_ACT_FLOOR_PAD_ITEM        = 4,
    TQ_ACT_FLOOR_MONSTER_GEN     = 6,
    TQ_ACT_FLOOR_MONSTER         = 7,
    TQ_ACT_FLOOR_CARRIED_ITEM    = 8,
} Theron_ActuatorFloorType;

typedef enum {
    TQ_ACT_WALL_NONE             = 0,
    TQ_ACT_WALL_SOMETHING        = 1,
    TQ_ACT_WALL_ALCOVE_ITEM      = 2,
    TQ_ACT_WALL_ITEM             = 3,
    TQ_ACT_WALL_ITEM_EATER       = 4,
    TQ_ACT_WALL_TRIGGER          = 5,
    TQ_ACT_WALL_COUNTING_PAD     = 6,
    TQ_ACT_WALL_SPELL_SHOOTER    = 8,
    TQ_ACT_WALL_WEAPON_SHOOTER   = 9,
    TQ_ACT_WALL_DOUBLE_SPELL     = 10,
    TQ_ACT_WALL_ITEM_EATER_TOGGLE= 11,
    TQ_ACT_WALL_CHAMPION_MIRROR  = 0x7F,
} Theron_ActuatorWallType;

typedef enum {
    TQ_ACT_EFFECT_SET = 0,
    TQ_ACT_EFFECT_CLEAR = 1,
    TQ_ACT_EFFECT_TOGGLE = 2,
    TQ_ACT_EFFECT_HOLD = 3,
} Theron_ActuatorEffect;

typedef struct {
    uint16_t next_ref;
    uint8_t  type;
    uint16_t value;
    uint8_t  once;
    uint8_t  effect;
    uint8_t  revert_effect;
    uint8_t  sound;
    uint8_t  delay;
    uint8_t  local_effect;
    uint8_t  graphism;
    uint8_t  target_x;
    uint8_t  target_y;
    uint8_t  target_facing;
    uint16_t local_multiple;
    /* Type 6 replaces the generic target word with the generator overlay.
     * These are source-record fields only; the original runtime consumer is
     * still gated until the HuC6280 trace proves when they are consumed. */
    uint8_t  generator_fields_valid;
    uint8_t  generator_generation;
    uint8_t  generator_toughness;
    uint8_t  generator_pause;
} Theron_Actuator;

typedef struct {
    uint8_t triggered;
    uint8_t resolved_effect;
    uint8_t disable_after_dispatch;
} Theron_ActuatorPartyEvent;

/* Source-record plan for a type-6 floor monster generator.  Count is kept in
 * the original zero-based Group encoding: a fixed generation N produces
 * N-1, while bit 3 requests `$4644/$4667 mod (N & 7)`.  This plan does not
 * consume RNG and therefore cannot by itself publish a group. */
typedef struct {
    uint16_t creature_type_value;
    uint8_t count_is_random;
    uint8_t fixed_count_minus_one;
    uint8_t random_count_bound;
    uint8_t toughness;
    uint8_t pause;
} Theron_ActuatorGeneratorPlan;

int theron_v1_track02_actuator_decode(
    const uint8_t *raw8, Theron_Actuator *out);

int theron_v1_track02_actuator_needs_value_fix(
    uint8_t type, int is_wall);

/* Source-compatible gate for DM/CSB F0276 floor-party sensors.  This only
 * decides whether an event is dispatched; it does not mutate a target. */
int theron_v1_track02_actuator_evaluate_party_event(
    const Theron_Actuator *actuator, int is_addition,
    int party_already_on_square, int champion_count,
    unsigned int party_direction, Theron_ActuatorPartyEvent *out);

int theron_v1_track02_actuator_generator_plan(
    const Theron_Actuator *actuator, Theron_ActuatorGeneratorPlan *out);

#endif
