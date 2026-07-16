#ifndef DM2_V1_PREDICATE_HELPERS_H
#define DM2_V1_PREDICATE_HELPERS_H

#include <stddef.h>
#include <stdint.h>

typedef struct DM2_V1_PredicateReceipt {
    int handled;
    int source_locked;
    int valid;
    int result;
    int blocked;
    const char* symbol;
    const char* source_path;
} DM2_V1_PredicateReceipt;

typedef struct DM2_V1_PlayerAbility {
    uint16_t current;
    uint16_t maximum;
    int16_t enhanced;
} DM2_V1_PlayerAbility;

void dm2_v1_predicate_receipt_clear(DM2_V1_PredicateReceipt* receipt);

int dm2_v1_IS_TILE_PASSAGE(
    uint8_t raw_tile,
    int has_teleporter_record,
    int teleporter_w4_0_0,
    DM2_V1_PredicateReceipt* out_receipt);

uint16_t dm2_v1_QUERY_GDAT_DBSPEC_WORD_VALUE(
    uint16_t record_id,
    uint8_t cls1,
    uint8_t cls2,
    uint8_t cls4,
    const uint16_t* word_values,
    size_t word_value_count,
    DM2_V1_PredicateReceipt* out_receipt);

uint16_t dm2_v1_GET_PLAYER_ABILITY(
    const DM2_V1_PlayerAbility* abilities,
    size_t ability_count,
    uint16_t ability_index,
    uint16_t get_max,
    uint16_t enchantment_power,
    uint8_t enchantment_aura,
    uint16_t deterministic_bonus,
    DM2_V1_PredicateReceipt* out_receipt);

int16_t dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
    uint8_t cls4,
    int16_t def_color,
    const uint16_t* charsheet_word_values,
    size_t charsheet_word_value_count,
    DM2_V1_PredicateReceipt* out_receipt);

int16_t dm2_v1_QUERY_3STAT_BAR_COLOR(
    uint8_t cls4,
    int16_t def_color,
    const uint16_t* interface_general_word_values,
    size_t interface_general_word_value_count,
    DM2_V1_PredicateReceipt* out_receipt);

const char* dm2_v1_predicate_helpers_source_evidence(void);

#endif
