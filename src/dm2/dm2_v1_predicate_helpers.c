#include "dm2_v1_predicate_helpers.h"

enum {
    DM2_TILE_WALL = 0,
    DM2_TILE_TELEPORTER = 5,
    DM2_TILE_MAP_EXIT = 7,
    DM2_OBJECT_NULL = 0xffff,
    DM2_ENCHANTMENT_AURA_FIRST = 2,
    DM2_ENCHANTMENT_AURA_LAST = 8
};

void dm2_v1_predicate_receipt_clear(DM2_V1_PredicateReceipt* receipt) {
    if (!receipt) {
        return;
    }
    receipt->handled = 0;
    receipt->source_locked = 0;
    receipt->valid = 0;
    receipt->result = 0;
    receipt->blocked = 0;
    receipt->symbol = 0;
    receipt->source_path = 0;
}

int dm2_v1_IS_TILE_PASSAGE(
    uint8_t raw_tile,
    int has_teleporter_record,
    int teleporter_w4_0_0,
    DM2_V1_PredicateReceipt* out_receipt)
{
    uint8_t tile_type = (uint8_t)(raw_tile >> 5);
    int result = 1;

    dm2_v1_predicate_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "IS_TILE_PASSAGE";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:2846";
    }
    if (tile_type == DM2_TILE_TELEPORTER &&
        has_teleporter_record && teleporter_w4_0_0 != 0) {
        result = 0;
    } else if (tile_type == DM2_TILE_WALL || tile_type == DM2_TILE_MAP_EXIT) {
        result = 0;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = result;
        out_receipt->blocked = result ? 0 : 1;
    }
    return result;
}

uint16_t dm2_v1_QUERY_GDAT_DBSPEC_WORD_VALUE(
    uint16_t record_id,
    uint8_t cls1,
    uint8_t cls2,
    uint8_t cls4,
    const uint16_t* word_values,
    size_t word_value_count,
    DM2_V1_PredicateReceipt* out_receipt)
{
    size_t index;
    uint16_t value = 0;

    dm2_v1_predicate_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "QUERY_GDAT_DBSPEC_WORD_VALUE";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:2275";
    }
    if (record_id == DM2_OBJECT_NULL) {
        if (out_receipt) {
            out_receipt->valid = 1;
        }
        return 0;
    }
    index = ((size_t)cls1 * 256U + (size_t)cls2) * 256U + (size_t)cls4;
    if (word_values && index < word_value_count) {
        value = word_values[index];
        if (out_receipt) {
            out_receipt->valid = 1;
        }
    } else if (out_receipt) {
        out_receipt->blocked = 1;
    }
    if (out_receipt) {
        out_receipt->result = value;
    }
    return value;
}

static int16_t dm2_clamp_ability(int32_t value) {
    if (value < 10) {
        return 10;
    }
    if (value > 220) {
        return 220;
    }
    return (int16_t)value;
}

uint16_t dm2_v1_GET_PLAYER_ABILITY(
    const DM2_V1_PlayerAbility* abilities,
    size_t ability_count,
    uint16_t ability_index,
    uint16_t get_max,
    uint16_t enchantment_power,
    uint8_t enchantment_aura,
    uint16_t deterministic_bonus,
    DM2_V1_PredicateReceipt* out_receipt)
{
    int32_t base;
    uint16_t power;

    dm2_v1_predicate_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "GET_PLAYER_ABILITY";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:14379";
    }
    if (!abilities || ability_index >= ability_count || get_max > 1U) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    base = get_max ? abilities[ability_index].maximum : abilities[ability_index].current;
    if (get_max == 0U && enchantment_power != 0U &&
        enchantment_aura >= DM2_ENCHANTMENT_AURA_FIRST &&
        enchantment_aura <= DM2_ENCHANTMENT_AURA_LAST &&
        (uint16_t)(enchantment_aura - 2U) == ability_index) {
        power = enchantment_power > 100U ? 100U : enchantment_power;
        base += (int32_t)(deterministic_bonus %
                (uint16_t)(((base * power) >> 7) + 1U)) + 4;
    }
    base += abilities[ability_index].enhanced;
    base = dm2_clamp_ability(base);
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = (int)base;
    }
    return (uint16_t)base;
}

int16_t dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
    uint8_t cls4,
    int16_t def_color,
    const uint16_t* charsheet_word_values,
    size_t charsheet_word_value_count,
    DM2_V1_PredicateReceipt* out_receipt)
{
    int16_t result = def_color;
    dm2_v1_predicate_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "QUERY_FOOD_WATER_BAR_COLOR";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:13194";
    }
    if (charsheet_word_values && cls4 < charsheet_word_value_count) {
        result = (int16_t)(256 + charsheet_word_values[cls4]);
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = result;
    }
    return result;
}

int16_t dm2_v1_QUERY_3STAT_BAR_COLOR(
    uint8_t cls4,
    int16_t def_color,
    const uint16_t* interface_general_word_values,
    size_t interface_general_word_value_count,
    DM2_V1_PredicateReceipt* out_receipt)
{
    int16_t result = def_color;
    dm2_v1_predicate_receipt_clear(out_receipt);
    if (out_receipt) {
        out_receipt->handled = 1;
        out_receipt->source_locked = 1;
        out_receipt->symbol = "QUERY_3STAT_BAR_COLOR";
        out_receipt->source_path = "SKWIN/SkWinCore.cpp:13203";
    }
    if (interface_general_word_values && cls4 < interface_general_word_value_count) {
        result = (int16_t)interface_general_word_values[cls4];
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->result = result;
    }
    return result;
}

const char* dm2_v1_predicate_helpers_source_evidence(void) {
    return "skproject SKWIN/SkWinCore.cpp IS_TILE_PASSAGE:2846 "
           "QUERY_GDAT_DBSPEC_WORD_VALUE:2275 GET_PLAYER_ABILITY:14379 "
           "QUERY_FOOD_WATER_BAR_COLOR:13194 QUERY_3STAT_BAR_COLOR:13203";
}
