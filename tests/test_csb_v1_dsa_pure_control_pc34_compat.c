/* CSBWin DSA pure-control and pure-stack opcode regression.
 * Source: Data.h DSAnoopCmd/DSAequalCmd; DSA.cpp EX_NOOP:574-591,
 * EX_EQUAL:1491-1515, STKOP_Loc2AbsCoord:3253-3268,
 * STKOP_FetchExCellFlg:3270-3297,
 * STKOP_BitCount:4832-4848 and STKOP_ParamFetch/ParamStore/VSET:
 * 2956-3044,4850-4887, plus STKOP_GlobalFetch:3958-3973 and
 * STKOP_PartyDistance:4057-4072,
 * STKOP_TimeFetch:2512-2518, STKOP_ThisDSAId:4822-4828,
 * STKOP_WhoHasTalent:4363-4380, STKOP_CountInjury:4798-4817, and
 * STKOP_TalentsFetch:4243-4283, STKOP_DisableSaves:2946-2955, and
 * STKOP_ChPoss/STKOP_MonPoss:3330-3386 and ExamineCell/THISCELL/NEIGHBORS
 * 2210-2309,4819-4830, EX_TYPE:1388-1511, and STKOP_NumParam:4949-4955. These
 * commands and STKOP_Fetch/Store:2473-2488 have no filter or world effect. */

#include "csb_v1_chaos_magic_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static uint32_t last_party_talents[4];
static int dsa_info_enabled;
static int excell_flags_enabled;
static int generator_delay_enabled;
static int generator_delay_stored;
static int generator_delay_store_count;
static int monster_info_enabled;
static int monster_info_store_count;
static uint32_t monster_info_stored[8];
static uint8_t monster_info_store_mask;
static int cell_info_enabled;
static int cell_info_store_count;
static uint32_t cell_info_stored[5];
static uint8_t cell_info_store_mask;
static int object_property_enabled;
static int object_property_store_count;
static CSB_V1_CSBWinDSAObjectProperty object_property_stored;
static uint32_t object_property_stored_value;
static uint32_t excell_flags_stored;
static int excell_flags_store_count;
static int champion_possession_enabled;
static int monster_possession_enabled;
static int inspect_cells_enabled;
static int thing_type_enabled;
static int is_carried_enabled;
static int random_state_enabled;
static uint32_t random_state;
static int level_multiplier_enabled;

static int wing_talents_enabled;

static int get_wing_talents(void *user, uint16_t fingerprint,
                            uint32_t *out_talents)
{
    (void)user;
    if (!out_talents || !wing_talents_enabled) return -1;
    if (fingerprint == 1u) {
        *out_talents = 0x5au;
        return 1;
    }
    *out_talents = 0u;
    return 0;
}

static int has_wing_character(void *user, uint16_t fingerprint)
{
    (void)user;
    if (!wing_talents_enabled) return -1;
    return fingerprint == 1u ? 1 : 0;
}

static int get_dsa_info(void *user, uint16_t thing, int *out_selector,
                        int *out_state, int *out_parameter_a,
                        int *out_parameter_b)
{
    (void)user;
    if (!dsa_info_enabled || thing != 0x123u) return 0;
    *out_selector = 3;
    *out_state = 9;
    *out_parameter_a = 0x4567;
    *out_parameter_b = 0x89ab;
    return 1;
}

static int get_excell_flags(void *user, uint32_t location,
                            uint32_t out_words[8])
{
    (void)user;
    if (!excell_flags_enabled || location != 0x0c82u || !out_words) return -1;
    memset(out_words, 0, 8u * sizeof(out_words[0]));
    out_words[0] = 1u << 2;
    out_words[3] = 1u << 2;
    out_words[7] = 1u << 2;
    return 1;
}

static int set_excell_flags(void *user, uint32_t location, uint32_t flags)
{
    (void)user;
    if (!excell_flags_enabled || location != 0x0c82u) return -1;
    excell_flags_stored = flags;
    ++excell_flags_store_count;
    return 1;
}

static int get_generator_delay(void *user, uint32_t location, int *out_delay)
{
    (void)user;
    if (!generator_delay_enabled || !out_delay) return 0;
    *out_delay = location == 0x0c82u ? 37 : -1;
    return 1;
}

static int set_generator_delay(void *user, uint32_t location, int delay)
{
    (void)user;
    if (!generator_delay_enabled || location != 0x0c82u) return 0;
    generator_delay_stored = delay;
    ++generator_delay_store_count;
    return 1;
}

static int get_monster_info(void *user, uint16_t thing,
                            uint32_t out_values[8])
{
    (void)user;
    if (!monster_info_enabled || !out_values || thing != 0x0123u) return 0;
    out_values[0] = 3u;
    out_values[1] = 17u;
    out_values[2] = 101u;
    out_values[3] = 102u;
    out_values[4] = 103u;
    out_values[5] = 104u;
    out_values[6] = 13u;
    out_values[7] = 5u;
    return 1;
}

static int set_monster_info(void *user, uint16_t thing,
                            const uint32_t values[8], uint8_t write_mask)
{
    (void)user;
    if (!monster_info_enabled || !values || thing != 0x0123u) return 0;
    memcpy(monster_info_stored, values, sizeof(monster_info_stored));
    monster_info_store_mask = write_mask;
    ++monster_info_store_count;
    return 1;
}

static int get_cell_info(void *user, uint32_t location,
                         uint32_t out_values[5])
{
    (void)user;
    if (!cell_info_enabled || !out_values) return 0;
    memset(out_values, 0, 5u * sizeof(out_values[0]));
    if (location == 0x0c82u) {
        out_values[0] = 4u;
        out_values[1] = 0x1du;
        out_values[2] = 5u;
        out_values[3] = 1u;
        out_values[4] = 12u;
    }
    return 1;
}

static int resolve_cell_store(void *user, uint32_t location,
                              uint32_t expected_room_type)
{
    (void)user;
    if (!cell_info_enabled) return -1;
    return location == 0x0c82u && expected_room_type == 4u ? 1 : 0;
}

static int set_cell_info(void *user, uint32_t location,
                         const uint32_t values[5], uint8_t write_mask)
{
    (void)user;
    if (!cell_info_enabled || location != 0x0c82u || !values) return 0;
    memcpy(cell_info_stored, values, sizeof(cell_info_stored));
    cell_info_store_mask = write_mask;
    ++cell_info_store_count;
    return 1;
}

static int get_object_property(void *user, uint16_t thing,
                               CSB_V1_CSBWinDSAObjectProperty property,
                               uint32_t *out_value)
{
    (void)user;
    if (!object_property_enabled || !out_value) return -1;
    if (thing != 0x0456u) return 0;
    switch (property) {
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CURSE: *out_value = 0u; break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_BROKEN: *out_value = 1u; break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_POISONED: *out_value = 0u; break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES: *out_value = 4u; break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE: *out_value = 17u; break;
    default: return -1;
    }
    return 1;
}

static int set_object_property(void *user, uint16_t thing,
                               CSB_V1_CSBWinDSAObjectProperty property,
                               uint32_t value)
{
    (void)user;
    if (!object_property_enabled || thing != 0x0456u) return 0;
    object_property_stored = property;
    object_property_stored_value = value;
    ++object_property_store_count;
    return 1;
}

static int get_champion_possession(void *user, int champion_index,
                                   uint32_t slot_index, int32_t *out_thing)
{
    (void)user;
    if (!champion_possession_enabled || !out_thing) return -1;
    *out_thing = -1;
    if (champion_index == 2 && slot_index == 7u) *out_thing = 0x0456;
    if (champion_index < 0 && slot_index == 7u) *out_thing = 0x0789;
    return 1;
}

static int get_monster_possession(void *user, uint16_t monster_thing,
                                  uint32_t possession_index,
                                  int32_t *out_thing)
{
    (void)user;
    if (!monster_possession_enabled || !out_thing) return -1;
    *out_thing = -1;
    if (monster_thing == 0x0123u && possession_index == 0u) {
        *out_thing = 0x0456;
    } else if (monster_thing == 0x0123u && possession_index == 1u) {
        *out_thing = 0x0789;
    }
    return 1;
}

static int inspect_cells(void *user, uint32_t location,
                         uint32_t criteria_mask, uint32_t first_cell,
                         uint32_t last_cell, uint32_t *out_result)
{
    (void)user;
    if (!inspect_cells_enabled || !out_result || location != 0x0c82u ||
        criteria_mask != 0xa0000002u) {
        return -1;
    }
    if (first_cell == 4u && last_cell == 4u) *out_result = 1u;
    else if (first_cell == 0u && last_cell == 3u) *out_result = 0x05u;
    else return -1;
    return 1;
}

static int get_thing_type(void *user, int32_t thing_index, int32_t *out_type)
{
    (void)user;
    if (!thing_type_enabled || !out_type) return -1;
    *out_type = thing_index == 0x0456 ? 50023 : -1;
    return 1;
}
static int is_carried(void *user, int32_t character, int32_t object, int32_t *out)
{
    (void)user;
    if (!is_carried_enabled || !out) return -1;
    *out = character == 4 && object == 0x0456 ? 767 : -1;
    return 1;
}

static int get_level_multiplier(void *user, int32_t level, int32_t *out)
{
    (void)user;
    if (!level_multiplier_enabled || !out) return -1;
    *out = level == 2 ? 7 : 1;
    return 1;
}

static int normalize_object_property(void *user, uint16_t thing,
                                     CSB_V1_CSBWinDSAObjectProperty property,
                                     uint32_t input_value,
                                     uint32_t *out_value)
{
    (void)user;
    if (!object_property_enabled || !out_value) return -1;
    if (thing != 0x0456u) return 0;
    switch (property) {
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CURSE:
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_BROKEN:
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_POISONED:
        *out_value = input_value != 0u;
        break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES:
        *out_value = input_value & 0x0fu;
        break;
    case CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE:
        *out_value = input_value & 0x1fu;
        break;
    default:
        return -1;
    }
    return 1;
}

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static CSB_V1_CSBWinDSAStackResult run(
    CSB_V1_ChaosMagicState *state, CSB_V1_DSAImportedAction *action,
    uint16_t *words, int word_count, uint32_t *parameters,
    CSB_V1_CSBWinDSAStackExecution *out_execution)
{
    CSB_V1_CSBWinDSAStackContext context;

    memset(&context, 0, sizeof(context));
    action->program_words = words;
    action->program_word_count = word_count;
    context.parameters = parameters;
    context.parameter_count = 4;
    context.party_location_valid = 1;
    context.party_level = 5;
    context.party_x = 10;
    context.party_y = 12;
    context.party_direction = 3;
    context.game_time_valid = 1;
    context.game_time = 12345u;
    if (random_state_enabled) {
        context.random_state_valid = 1;
        context.random_state = random_state;
    }
    context.dsa_slave_thing_valid = 1;
    context.dsa_slave_thing = 0x8123u;
    context.party_champions_valid = 1;
    context.party_champion_count = 4;
    context.party_leader_index = 2;
    context.party_champion_talents[0] = 0x3u;
    context.party_champion_talents[1] = 0x1u;
    context.party_champion_talents[2] = 0x7u;
    context.party_champion_fingerprints[0] = 0x1010u;
    context.party_champion_fingerprints[1] = 0x2020u;
    context.party_champion_fingerprints[2] = 0x2020u;
    context.party_champion_fingerprints[3] = 0x4040u;
    context.party_champion_wounds[0] = 0x0003u;
    context.party_champion_wounds[1] = 0x000fu;
    context.party_champion_wounds[2] = 0x0006u;
    context.party_champion_wounds[3] = 0x8000u;
    context.party_champion_health[0] = 10;
    context.party_champion_health[1] = 0;
    context.party_champion_health[2] = 20;
    context.party_champion_health[3] = 30;
    if (wing_talents_enabled) {
        context.get_wing_talents = get_wing_talents;
        context.has_wing_character = has_wing_character;
    }
    if (dsa_info_enabled) context.get_dsa_info = get_dsa_info;
    if (excell_flags_enabled) {
        context.get_excell_flags = get_excell_flags;
        context.set_excell_flags = set_excell_flags;
    }
    if (generator_delay_enabled) {
        context.get_generator_delay = get_generator_delay;
        context.set_generator_delay = set_generator_delay;
    }
    if (monster_info_enabled) {
        context.get_monster_info = get_monster_info;
        context.set_monster_info = set_monster_info;
    }
    if (cell_info_enabled) {
        context.get_cell_info = get_cell_info;
        context.resolve_cell_store = resolve_cell_store;
        context.set_cell_info = set_cell_info;
    }
    if (object_property_enabled) {
        context.get_object_property = get_object_property;
        context.set_object_property = set_object_property;
        context.normalize_object_property = normalize_object_property;
    }
    if (champion_possession_enabled) {
        context.get_champion_possession = get_champion_possession;
    }
    if (monster_possession_enabled) {
        context.get_monster_possession = get_monster_possession;
    }
    if (inspect_cells_enabled) context.inspect_cells = inspect_cells;
    if (thing_type_enabled) context.get_thing_type = get_thing_type;
    if (is_carried_enabled) context.is_carried = is_carried;
    if (level_multiplier_enabled) {
        context.get_level_multiplier = get_level_multiplier;
    }
    {
        CSB_V1_CSBWinDSAStackResult result =
            csb_v1_csbwin_dsa_execute_authenticated_stack_action(
                state, 7, 1u, 0, &context, out_execution);
        if (result == CSB_V1_CSBWIN_DSA_STACK_OK && random_state_enabled) {
            random_state = context.random_state;
        }
        memcpy(last_party_talents, context.party_champion_talents,
               sizeof(last_party_talents));
        return result;
    }
}

static CSB_V1_CSBWinDSAStackResult run_save_policy(
    CSB_V1_ChaosMagicState *state, CSB_V1_DSAImportedAction *action,
    uint16_t *words, int word_count, int initial, int *out_value)
{
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;

    memset(&context, 0, sizeof(context));
    memset(&execution, 0, sizeof(execution));
    action->program_words = words;
    action->program_word_count = word_count;
    context.saves_disabled_valid = 1;
    context.saves_disabled = initial;
    {
        CSB_V1_CSBWinDSAStackResult result =
            csb_v1_csbwin_dsa_execute_authenticated_stack_action(
                state, 7, 1u, 0, &context, &execution);
        if (out_value) *out_value = context.saves_disabled;
        return result;
    }
}

static CSB_V1_CSBWinDSAStackResult run_without_party_location(
    CSB_V1_ChaosMagicState *state, CSB_V1_DSAImportedAction *action,
    uint16_t *words, int word_count, uint32_t *parameters)
{
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;

    memset(&context, 0, sizeof(context));
    memset(&execution, 0, sizeof(execution));
    action->program_words = words;
    action->program_word_count = word_count;
    context.parameters = parameters;
    context.parameter_count = 4;
    return csb_v1_csbwin_dsa_execute_authenticated_stack_action(
        state, 7, 1u, 0, &context, &execution);
}

int main(void)
{
    uint16_t noop_relative[] = { 0xffc3u };
    uint16_t noop_extended[] = { 0x8003u, 0xfffeu };
    uint16_t equal_true[] = {
        0x0686u, 5u, 0x0686u, 5u, 0x0048u, 0x080du
    };
    uint16_t equal_false[] = {
        0x0686u, 5u, 0x0686u, 6u, 0x0048u, 0x080du
    };
    uint16_t equal_underflow[] = { 0x0008u };
    uint16_t question_true[] = { 0x0686u, 1u, 0x0049u };
    uint16_t question_false[] = { 0x0686u, 0u, 0x03c9u };
    uint16_t question_extended[] = { 0x0686u, 1u, 0x0389u, 0xfffeu };
    uint16_t question_jump[] = { 0x0686u, 1u, 0x0849u, 2u };
    uint16_t loc2abscoord[] = {
        0x0786u, 0x9629u, 0x0002u, 0x0c0bu,
        0x00cdu, 0x008du, 0x004du, 0x000du
    };
    uint16_t bitcount[] = {
        0x0786u, 0x0f0fu, 0xf0f0u, 0x1b0bu, 0x000du
    };
    uint16_t bitcount_underflow[] = { 0x1b0bu };
    uint16_t parameter_round_trip[] = {
        0x0686u, 4u, 0x0686u, 0u, 0x0a0bu,
        0x0686u, 1u, 0x0686u, 0u, 0x0686u, 3u, 0x0215u,
        0x0686u, 3u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t parameter_constant[] = {
        0x0686u, 107u, 0x0686u, 0u, 0x0686u, 4u, 0x0215u,
        0x0686u, 4u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t parameter_bad_variable_span[] = {
        0x0686u, 4u, 0x0686u, 98u, 0x0a0bu
    };
    uint16_t party_distance_same_level[] = {
        0x0786u, 0x14f4u, 0u, 0x0055u, 0x000du
    };
    uint16_t party_distance_other_level[] = {
        0x0786u, 0x08f4u, 0u, 0x0055u, 0x000du
    };
    uint16_t global_fetch_party_location[] = {
        0x0686u, 1u, 0x0ecbu, 0x000du
    };
    uint16_t global_fetch_unknown_selector[] = {
        0x0686u, 2u, 0x0ecbu, 0x000du
    };
    uint16_t excell_flags_fetch[] = {
        0x0686u, 0x0c82u, 0x0b0bu, 0x000du
    };
    uint16_t excell_flags_store[] = {
        0x0686u, 0x89u, 0x0686u, 0x0c82u, 0x0b4bu
    };
    uint16_t excell_flags_store_then_bad[] = {
        0x0686u, 0x89u, 0x0686u, 0x0c82u, 0x0b4bu, 0x0000u
    };
    uint16_t generator_delay_fetch[] = {
        0x0686u, 0x0c82u, 0x190bu, 0x000du
    };
    uint16_t generator_delay_absent[] = {
        0x0686u, 0x0c83u, 0x190bu, 0x000du
    };
    uint16_t generator_delay_store_then_fetch[] = {
        0x0686u, 91u, 0x0686u, 0x0c82u, 0x088bu,
        0x0686u, 0x0c82u, 0x190bu,
        0x000du
    };
    uint16_t generator_delay_store_then_bad[] = {
        0x0686u, 91u, 0x0686u, 0x0c82u, 0x088bu, 0x0000u
    };
    uint16_t monster_fetch[] = {
        0x0686u, 0x0123u, 0x0686u, 0u, 0x0686u, 8u, 0x0f0bu,
        0x0686u, 2u, 0x098bu, 0x000du
    };
    uint16_t monster_fetch_bad_span[] = {
        0x0686u, 0x0123u, 0x0686u, 99u, 0x0686u, 2u, 0x0f0bu,
        0x0686u, 99u, 0x098bu, 0x000du
    };
    uint16_t monster_store_then_fetch[] = {
        0x0686u, 555u, 0x0686u, 2u, 0x09cbu,
        0x0686u, 0x0123u, 0x0686u, 0u, 0x0686u, 3u, 0x0fcbu,
        0x0686u, 0x0123u, 0x0686u, 0u, 0x0686u, 3u, 0x0f0bu,
        0x0686u, 2u, 0x098bu, 0x000du
    };
    uint16_t monster_store_then_bad[] = {
        0x0686u, 555u, 0x0686u, 2u, 0x09cbu,
        0x0686u, 0x0123u, 0x0686u, 0u, 0x0686u, 3u, 0x0fcbu,
        0x0000u
    };
    uint16_t cell_fetch[] = {
        0x0686u, 0x0c82u, 0x0686u, 0u, 0x0686u, 5u, 0x0e4bu,
        0x0686u, 4u, 0x098bu, 0x000du
    };
    uint16_t cell_fetch_bad_span[] = {
        0x0686u, 0x0c82u, 0x0686u, 99u, 0x0686u, 2u, 0x0e4bu,
        0x0686u, 99u, 0x098bu, 0x000du
    };
    uint16_t cell_store_then_fetch[] = {
        0x0686u, 4u, 0x0686u, 0u, 0x09cbu,
        0x0686u, 0x1eu, 0x0686u, 1u, 0x09cbu,
        0x0686u, 5u, 0x0686u, 2u, 0x09cbu,
        0x0686u, 1u, 0x0686u, 3u, 0x09cbu,
        0x0686u, 12u, 0x0686u, 4u, 0x09cbu,
        0x0686u, 0x0c82u, 0x0686u, 0u, 0x0686u, 5u, 0x0e8bu,
        0x0686u, 0x0c82u, 0x0686u, 0u, 0x0686u, 5u, 0x0e4bu,
        0x0686u, 2u, 0x098bu, 0x000du
    };
    uint16_t cell_store_then_bad[] = {
        0x0686u, 4u, 0x0686u, 0u, 0x09cbu,
        0x0686u, 0x1eu, 0x0686u, 1u, 0x09cbu,
        0x0686u, 0x0c82u, 0x0686u, 0u, 0x0686u, 2u, 0x0e8bu,
        0x0000u
    };
    uint16_t object_get_broken[] = {
        0x0686u, 0x0456u, 0x0dcbu, 0x000du
    };
    uint16_t object_set_charges_then_fetch[] = {
        0x0686u, 0x0456u, 0x0686u, 0x2fu, 0x0d8bu,
        0x0686u, 0x0456u, 0x0d4bu, 0x000du
    };
    uint16_t object_set_curse_then_bad[] = {
        0x0686u, 0x0456u, 0x0686u, 1u, 0x0d0bu, 0x0000u
    };
    uint16_t champion_possession[] = {
        0x0686u, 4u, 0x0686u, 7u, 0x0b8bu, 0x000du
    };
    uint16_t champion_hand_possession[] = {
        0x0786u, 0xffffu, 0xffffu, 0x0686u, 7u, 0x0b8bu, 0x000du
    };
    uint16_t monster_possession[] = {
        0x0686u, 0x0123u, 0x0686u, 1u, 0x134bu, 0x000du
    };
    uint16_t this_cell[] = {
        0x0686u, 0x0c82u, 0x0786u, 0x0002u, 0xa000u, 0x1b4bu, 0x000du
    };
    uint16_t neighbors[] = {
        0x0686u, 0x0c82u, 0x0786u, 0x0002u, 0xa000u, 0x1acbu, 0x000du
    };
    uint16_t type_fetch[] = {
        0x0686u, 0x0456u, 0x020bu, 0x000du
    };
    uint16_t num_param[] = { 0x02d5u, 0x000du };
    uint16_t is_carried_query[] = {
        0x0686u, 4u, 0x0686u, 0x0456u, 0x1c8bu, 0x000du
    };
    uint16_t multiplier_fetch[] = {
        0x0686u, 2u, 0x1c0bu, 0x000du
    };
    uint16_t multiplier_default[] = {
        0x0686u, 99u, 0x1c0bu, 0x000du
    };
    uint16_t random_fetch[] = {
        0x0686u, 10u, 0x10cbu, 0x000du
    };
    uint16_t random_then_bad_opcode[] = {
        0x0686u, 10u, 0x10cbu, 0x0000u
    };
    uint16_t time_fetch[] = { 0x184bu, 0x000du };
    uint16_t this_dsa_id[] = { 0x0155u, 0x000du };
    uint16_t local_fetch_store[] = {
        0x0686u, 3u, 0x0686u, 0u, 0x0a0bu,
        0x0686u, 1u, 0x098bu, 0x0686u, 2u, 0x09cbu,
        0x0686u, 3u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t local_fetch_bad_index[] = { 0x0686u, 100u, 0x098bu };
    uint16_t dsa_info_fetch[] = {
        0x0686u, 0x0123u, 0x0095u, 0x00cdu, 0x008du, 0x004du, 0x000du
    };
    uint16_t dsa_info_invalid[] = {
        0x0686u, 0x0124u, 0x0095u, 0x00cdu, 0x008du, 0x004du, 0x000du
    };
    uint16_t who_has_talent[] = { 0x0686u, 1u, 0x1d4bu, 0x000du };
    uint16_t where_is_party_character[] = {
        0x0686u, 0x2020u, 0x1d0bu, 0x000du
    };
    uint16_t where_is_wing_character[] = {
        0x0686u, 1u, 0x1d0bu, 0x000du
    };
    uint16_t where_is_absent_character[] = {
        0x0686u, 0x3030u, 0x1d0bu, 0x000du
    };
    uint16_t count_injury[] = {
        0x0686u, 15u, 0x0686u, 7u, 0x1a8bu, 0x000du
    };
    uint16_t talents_fetch_leader[] = {
        0x0686u, 4u, 0x0195u, 0x000du
    };
    uint16_t talents_fetch_missing[] = {
        0x0686u, 7u, 0x0195u, 0x000du
    };
    uint16_t talents_fetch_wing[] = {
        0x0786u, 1u, 1u, 0x0195u, 0x000du
    };
    uint16_t talents_store_party[] = {
        0x0686u, 0x55u, 0x0686u, 1u, 0x01d5u
    };
    uint16_t talents_store_then_bad[] = {
        0x0686u, 0x55u, 0x0686u, 1u, 0x01d5u, 0x0000u
    };
    uint16_t disable_saves[] = { 0x0686u, 1u, 0x090bu };
    uint16_t enable_saves[] = { 0x0686u, 0u, 0x090bu };
    uint16_t disable_saves_then_bad[] = {
        0x0686u, 1u, 0x090bu, 0x0000u
    };
    uint32_t parameters[4] = { 77u, 0u, 0u, 0u };
    CSB_V1_DSAImportedAction action;
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAStackExecution execution;
    int saves_disabled = -1;

    memset(&action, 0, sizeof(action));
    memset(&execution, 0, sizeof(execution));
    csb_v1_chaos_init(&state);
    action.dsa_id = 7u;
    action.state_index = 1u;
    action.column = 0u;
    state.imported_actions = &action;
    state.imported_action_count = 1;

    check(run(&state, &action, noop_relative,
              (int)(sizeof(noop_relative) / sizeof(noop_relative[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == -1 && execution.words_consumed == 1u &&
              execution.stack_depth == 0u && parameters[0] == 77u,
          "NOOP preserves a signed inline next-state without mutation");

    check(run(&state, &action, noop_extended,
              (int)(sizeof(noop_extended) / sizeof(noop_extended[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == 65534 && execution.words_consumed == 2u &&
              execution.stack_depth == 0u && parameters[0] == 77u,
          "NOOP MAXSTATE consumes its exact raw extension word");

    check(run(&state, &action, equal_true,
              (int)(sizeof(equal_true) / sizeof(equal_true[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 1u && execution.next_state == 1 &&
              execution.stack_depth == 0u,
          "EQUAL stores one for equal source stack words");

    parameters[0] = 77u;
    check(run(&state, &action, equal_false,
              (int)(sizeof(equal_false) / sizeof(equal_false[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0u && execution.next_state == 1 &&
              execution.stack_depth == 0u,
          "EQUAL stores zero for unequal source stack words");

    parameters[0] = 77u;
    check(run(&state, &action, equal_underflow,
              (int)(sizeof(equal_underflow) / sizeof(equal_underflow[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_MALFORMED &&
              parameters[0] == 77u,
          "EQUAL stack underflow rejects without parameter publication");

    parameters[0] = 77u;
    check(run(&state, &action, question_true,
              (int)(sizeof(question_true) / sizeof(question_true[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == 1 && execution.stack_depth == 0u &&
              parameters[0] == 77u,
          "QUESTION consumes a true source condition without side effects");

    check(run(&state, &action, question_false,
              (int)(sizeof(question_false) / sizeof(question_false[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == -1 && execution.stack_depth == 0u &&
              parameters[0] == 77u,
          "QUESTION consumes a false source condition without side effects");

    check(run(&state, &action, question_extended,
              (int)(sizeof(question_extended) / sizeof(question_extended[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.next_state == 65534 && execution.words_consumed == 4u,
          "QUESTION MAXSTATE consumes its exact raw extension word");

    check(run(&state, &action, question_jump,
              (int)(sizeof(question_jump) / sizeof(question_jump[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u,
          "QUESTION jump branch remains closed without an action-chain owner");

    parameters[0] = parameters[1] = parameters[2] = parameters[3] = 77u;
    check(run(&state, &action, loc2abscoord,
              (int)(sizeof(loc2abscoord) / sizeof(loc2abscoord[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 37u && parameters[1] == 17u &&
              parameters[2] == 9u && parameters[3] == 2u &&
              execution.stack_depth == 0u,
          "LOC2ABSCOORD decodes an original packed location in source order");

    parameters[0] = 77u;
    check(run(&state, &action, bitcount,
              (int)(sizeof(bitcount) / sizeof(bitcount[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 16u && execution.stack_depth == 0u,
          "BITCOUNT consumes and counts all 32 original source bits");

    parameters[0] = 77u;
    check(run(&state, &action, bitcount_underflow,
              (int)(sizeof(bitcount_underflow) /
                    sizeof(bitcount_underflow[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_MALFORMED &&
              parameters[0] == 77u,
          "BITCOUNT underflow rejects without parameter publication");

    parameters[0] = 77u;
    check(run(&state, &action, global_fetch_party_location,
              (int)(sizeof(global_fetch_party_location) /
                    sizeof(global_fetch_party_location[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x3154cu && execution.stack_depth == 0u,
          "GLOBAL@ returns the live CSBWin packed party LOCATIONREL");

    parameters[0] = 77u;
    check(run(&state, &action, global_fetch_unknown_selector,
              (int)(sizeof(global_fetch_unknown_selector) /
                    sizeof(global_fetch_unknown_selector[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0u && execution.stack_depth == 0u,
          "GLOBAL@ preserves CSBWin zero for an unrecognized selector");

    parameters[0] = 77u;
    check(run_without_party_location(&state, &action,
              global_fetch_party_location,
              (int)(sizeof(global_fetch_party_location) /
                    sizeof(global_fetch_party_location[0])),
              parameters) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u,
          "GLOBAL@ refuses selector one without a runtime-owned party pose");

    excell_flags_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, excell_flags_fetch,
              (int)(sizeof(excell_flags_fetch) / sizeof(excell_flags_fetch[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x89u && execution.stack_depth == 0u,
          "ECF@ folds the selected source cell bit from all eight EXPOOL words");
    excell_flags_enabled = 0;

    generator_delay_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, generator_delay_fetch,
              (int)(sizeof(generator_delay_fetch) /
                    sizeof(generator_delay_fetch[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 37u && execution.stack_depth == 0u,
          "GeneratorDelay@ reads the first source type-six generator delay");
    parameters[0] = 77u;
    check(run(&state, &action, generator_delay_absent,
              (int)(sizeof(generator_delay_absent) /
                    sizeof(generator_delay_absent[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == UINT32_MAX && execution.stack_depth == 0u,
          "GeneratorDelay@ preserves source minus one when no generator exists");
    generator_delay_stored = -1;
    generator_delay_store_count = 0;
    parameters[0] = 77u;
    check(run(&state, &action, generator_delay_store_then_fetch,
              (int)(sizeof(generator_delay_store_then_fetch) /
                    sizeof(generator_delay_store_then_fetch[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 91u && generator_delay_stored == 91 &&
              generator_delay_store_count == 1,
          "GeneratorDelay! stages DB3 delay, exposes it to same-action fetch, and commits once");
    generator_delay_stored = -1;
    generator_delay_store_count = 0;
    check(run(&state, &action, generator_delay_store_then_bad,
              (int)(sizeof(generator_delay_store_then_bad) /
                    sizeof(generator_delay_store_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              generator_delay_store_count == 0,
          "GeneratorDelay! rejects a later unsupported source word without DB3 mutation");
    generator_delay_enabled = 0;
    monster_info_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, monster_fetch,
              (int)(sizeof(monster_fetch) / sizeof(monster_fetch[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 101u && execution.stack_depth == 0u,
          "Monster@ copies real DB4 result words into source DSAVARS order");
    parameters[0] = 77u;
    check(run(&state, &action, monster_fetch_bad_span,
              (int)(sizeof(monster_fetch_bad_span) /
                    sizeof(monster_fetch_bad_span[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0u && execution.stack_depth == 0u,
          "Monster@ keeps CSBWin's out-of-bank local span as a no-op with undefined zero");
    monster_info_store_count = 0;
    monster_info_store_mask = 0u;
    memset(monster_info_stored, 0, sizeof(monster_info_stored));
    parameters[0] = 77u;
    check(run(&state, &action, monster_store_then_fetch,
              (int)(sizeof(monster_store_then_fetch) /
                    sizeof(monster_store_then_fetch[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 555u && monster_info_store_count == 1 &&
              monster_info_stored[2] == 555u &&
              monster_info_store_mask == (1u << 2),
          "Monster! stages DB4 hit points, exposes them to same-action Monster@, and commits once");
    monster_info_store_count = 0;
    check(run(&state, &action, monster_store_then_bad,
              (int)(sizeof(monster_store_then_bad) /
                    sizeof(monster_store_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              monster_info_store_count == 0,
          "Monster! rejects a later unsupported source word without DB4 mutation");
    monster_info_enabled = 0;
    cell_info_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, cell_fetch,
              (int)(sizeof(cell_fetch) / sizeof(cell_fetch[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 12u && execution.stack_depth == 0u,
          "Cell@ copies real CELLFLAG and DB0/DB1 words into source DSAVARS order");
    parameters[0] = 77u;
    check(run(&state, &action, cell_fetch_bad_span,
              (int)(sizeof(cell_fetch_bad_span) /
                    sizeof(cell_fetch_bad_span[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0u && execution.stack_depth == 0u,
          "Cell@ retains CSBWin's out-of-bank local span as an undefined zero");
    cell_info_store_count = 0;
    cell_info_store_mask = 0u;
    memset(cell_info_stored, 0, sizeof(cell_info_stored));
    parameters[0] = 77u;
    check(run(&state, &action, cell_store_then_fetch,
              (int)(sizeof(cell_store_then_fetch) /
                    sizeof(cell_store_then_fetch[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 5u && cell_info_store_count == 1 &&
              cell_info_stored[1] == 0x1fu && cell_info_stored[2] == 5u &&
              cell_info_stored[3] == 1u && cell_info_stored[4] == 12u &&
              cell_info_store_mask == 0x1eu,
          "Cell! stages DB0 door fields, exposes them to same-action Cell@, and commits once");
    cell_info_store_count = 0;
    check(run(&state, &action, cell_store_then_bad,
              (int)(sizeof(cell_store_then_bad) /
                    sizeof(cell_store_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              cell_info_store_count == 0,
          "Cell! rejects a later unsupported source word without cell or DB0 mutation");
    cell_info_enabled = 0;
    object_property_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, object_get_broken,
              (int)(sizeof(object_get_broken) / sizeof(object_get_broken[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 1u && execution.stack_depth == 0u,
          "object property fetch reads the source DB5/DB6 boolean field");
    object_property_store_count = 0;
    object_property_stored_value = 0u;
    parameters[0] = 77u;
    check(run(&state, &action, object_set_charges_then_fetch,
              (int)(sizeof(object_set_charges_then_fetch) /
                    sizeof(object_set_charges_then_fetch[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 15u && object_property_store_count == 1 &&
              object_property_stored ==
                  CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES &&
              object_property_stored_value == 15u,
          "Get/SetCharges stages source bitfield truncation and exposes it within the action");
    object_property_store_count = 0;
    check(run(&state, &action, object_set_curse_then_bad,
              (int)(sizeof(object_set_curse_then_bad) /
                    sizeof(object_set_curse_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              object_property_store_count == 0,
          "SetCurse rejects a later unsupported word without DB5/DB6/DB10 mutation");
    object_property_enabled = 0;
    champion_possession_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, champion_possession,
              (int)(sizeof(champion_possession) /
                    sizeof(champion_possession[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x0456u && execution.stack_depth == 0u,
          "CHPOSS maps selector four through the live CSBWin hand character");
    parameters[0] = 77u;
    check(run(&state, &action, champion_hand_possession,
              (int)(sizeof(champion_hand_possession) /
                    sizeof(champion_hand_possession[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x0789u && execution.stack_depth == 0u,
          "CHPOSS retains a signed negative selector for the cursor hand");
    champion_possession_enabled = 0;
    monster_possession_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, monster_possession,
              (int)(sizeof(monster_possession) /
                    sizeof(monster_possession[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x0789u && execution.stack_depth == 0u,
          "MONPOSS preserves source DB4 possession-chain indexing");
    monster_possession_enabled = 0;
    inspect_cells_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, this_cell,
              (int)(sizeof(this_cell) / sizeof(this_cell[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 1u && execution.stack_depth == 0u,
          "THISCELL uses the source center-cell ExamineCell range");
    parameters[0] = 77u;
    check(run(&state, &action, neighbors,
              (int)(sizeof(neighbors) / sizeof(neighbors[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x05u && execution.stack_depth == 0u,
          "NEIGHBORS uses the source cardinal ExamineCell range");
    inspect_cells_enabled = 0;
    thing_type_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, type_fetch,
              (int)(sizeof(type_fetch) / sizeof(type_fetch[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 50023u && execution.stack_depth == 0u,
          "TYPE returns the source dbType-plus-raw-record object code");
    thing_type_enabled = 0;
    parameters[0] = 77u;
    check(run(&state, &action, num_param,
              (int)(sizeof(num_param) / sizeof(num_param[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 4u && execution.stack_depth == 0u,
          "NUMPARAM returns the caller-owned source parameter count");
    is_carried_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, is_carried_query,
              (int)(sizeof(is_carried_query) / sizeof(is_carried_query[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 767u,
          "ISCARRIED preserves source character and object stack order");
    is_carried_enabled = 0;
    level_multiplier_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, multiplier_fetch,
              (int)(sizeof(multiplier_fetch) / sizeof(multiplier_fetch[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 7u && execution.stack_depth == 0u,
          "MULTIPLIER@ reads the source LEVELDESC multiplier");
    parameters[0] = 77u;
    check(run(&state, &action, multiplier_default,
              (int)(sizeof(multiplier_default) / sizeof(multiplier_default[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 1u && execution.stack_depth == 0u,
          "MULTIPLIER@ returns the source out-of-range default");
    level_multiplier_enabled = 0;
    random_state_enabled = 1;
    random_state = 0x12345678u;
    parameters[0] = 77u;
    check(run(&state, &action, random_fetch,
              (int)(sizeof(random_fetch) / sizeof(random_fetch[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 5u && random_state == 0x7ee30323u,
          "RANDOM advances the source CSBWin seed and returns its modulo");
    random_state = 0x12345678u;
    parameters[0] = 77u;
    check(run(&state, &action, random_then_bad_opcode,
              (int)(sizeof(random_then_bad_opcode) /
                    sizeof(random_then_bad_opcode[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u && random_state == 0x12345678u,
          "RANDOM does not publish a seed after a later rejected opcode");
    random_state_enabled = 0;
    parameters[0] = 77u;
    check(run(&state, &action, excell_flags_fetch,
              (int)(sizeof(excell_flags_fetch) / sizeof(excell_flags_fetch[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u,
          "ECF@ rejects without an authenticated EXPOOL owner");

    excell_flags_enabled = 1;
    excell_flags_stored = 0u;
    excell_flags_store_count = 0;
    check(run(&state, &action, excell_flags_store,
              (int)(sizeof(excell_flags_store) / sizeof(excell_flags_store[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              excell_flags_store_count == 1 && excell_flags_stored == 0x89u,
          "ECF! commits the source flag byte only after complete execution");
    excell_flags_stored = 0u;
    excell_flags_store_count = 0;
    check(run(&state, &action, excell_flags_store_then_bad,
              (int)(sizeof(excell_flags_store_then_bad) /
                    sizeof(excell_flags_store_then_bad[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              excell_flags_store_count == 0 && excell_flags_stored == 0u,
          "ECF! rejects without publishing before a later bad opcode");
    excell_flags_enabled = 0;

    parameters[0] = 10u;
    parameters[1] = 20u;
    parameters[2] = 30u;
    parameters[3] = 40u;
    check(run(&state, &action, parameter_round_trip,
              (int)(sizeof(parameter_round_trip) /
                    sizeof(parameter_round_trip[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 20u && parameters[1] == 30u &&
              parameters[2] == 40u && parameters[3] == 40u &&
              execution.stack_depth == 0u,
          "PARAM@ VSET PARAM! preserves source variable and parameter order");

    parameters[0] = 10u;
    parameters[1] = 20u;
    parameters[2] = 30u;
    parameters[3] = 40u;
    check(run(&state, &action, parameter_constant,
              (int)(sizeof(parameter_constant) /
                    sizeof(parameter_constant[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 7u && parameters[1] == 7u &&
              parameters[2] == 7u && parameters[3] == 7u,
          "VSET positive constant writes a bounded source DSAVARS range");

    parameters[0] = 10u;
    parameters[1] = 20u;
    parameters[2] = 30u;
    parameters[3] = 40u;
    check(run(&state, &action, parameter_bad_variable_span,
              (int)(sizeof(parameter_bad_variable_span) /
                    sizeof(parameter_bad_variable_span[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL &&
              parameters[0] == 10u && parameters[1] == 20u &&
              parameters[2] == 30u && parameters[3] == 40u,
          "PARAM@ rejects an out-of-bank DSAVARS span without publication");

    parameters[0] = 77u;
    check(run(&state, &action, party_distance_same_level,
              (int)(sizeof(party_distance_same_level) /
                    sizeof(party_distance_same_level[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 11u && execution.stack_depth == 0u,
          "PARTYDISTANCE returns source Manhattan distance on the party level");

    parameters[0] = 77u;
    check(run(&state, &action, party_distance_other_level,
              (int)(sizeof(party_distance_other_level) /
                    sizeof(party_distance_other_level[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0xfffffffdu && execution.stack_depth == 0u,
          "PARTYDISTANCE returns negative source level distance off-level");

    parameters[0] = 77u;
    check(run(&state, &action, time_fetch,
              (int)(sizeof(time_fetch) / sizeof(time_fetch[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 12345u && execution.stack_depth == 0u,
          "TIME@ reads the runtime-owned CSBWin game clock");

    parameters[0] = 77u;
    check(run(&state, &action, this_dsa_id,
              (int)(sizeof(this_dsa_id) / sizeof(this_dsa_id[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x8123u && execution.stack_depth == 0u,
          "THIS_DSA_ID returns the verified source actuator Thing identity");

    parameters[0] = 10u;
    parameters[1] = 20u;
    parameters[2] = 30u;
    parameters[3] = 40u;
    check(run(&state, &action, local_fetch_store,
              (int)(sizeof(local_fetch_store) / sizeof(local_fetch_store[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 10u && parameters[1] == 20u &&
              parameters[2] == 20u && parameters[3] == 40u &&
              execution.stack_depth == 0u,
          "FETCH and STORE preserve CSBWin DSAVARS stack order");

    parameters[0] = 77u;
    check(run(&state, &action, local_fetch_bad_index,
              (int)(sizeof(local_fetch_bad_index) /
                    sizeof(local_fetch_bad_index[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL &&
              parameters[0] == 77u,
          "FETCH rejects an out-of-bank source local without publication");

    dsa_info_enabled = 1;
    parameters[0] = parameters[1] = parameters[2] = parameters[3] = 0u;
    check(run(&state, &action, dsa_info_fetch,
              (int)(sizeof(dsa_info_fetch) / sizeof(dsa_info_fetch[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 3u && parameters[1] == 9u &&
              parameters[2] == 0x4567u && parameters[3] == 0x89abu,
          "DSAINFO@ returns source DB3 type-47 fields in stack order");
    parameters[0] = parameters[1] = parameters[2] = parameters[3] = 0u;
    check(run(&state, &action, dsa_info_invalid,
              (int)(sizeof(dsa_info_invalid) /
                    sizeof(dsa_info_invalid[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == UINT32_MAX && parameters[1] == UINT32_MAX &&
              parameters[2] == UINT32_MAX && parameters[3] == UINT32_MAX,
          "DSAINFO@ returns four source minus-one words for an invalid object");
    dsa_info_enabled = 0;

    parameters[0] = 77u;
    check(run(&state, &action, who_has_talent,
              (int)(sizeof(who_has_talent) / sizeof(who_has_talent[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 7u && execution.stack_depth == 0u,
          "WHO_HAS_TALENT returns the source party talent mask");

    parameters[0] = 77u;
    check(run(&state, &action, where_is_party_character,
              (int)(sizeof(where_is_party_character) /
                    sizeof(where_is_party_character[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 2u && execution.stack_depth == 0u,
          "WHEREISCHAR retains CSBWin's final matching party index");

    parameters[0] = 77u;
    check(run(&state, &action, count_injury,
              (int)(sizeof(count_injury) / sizeof(count_injury[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 4u && execution.stack_depth == 0u,
          "COUNT_INJURY skips dead champions and counts selected source wounds");

    parameters[0] = 77u;
    check(run(&state, &action, talents_fetch_leader,
              (int)(sizeof(talents_fetch_leader) /
                    sizeof(talents_fetch_leader[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x7u && execution.stack_depth == 0u,
          "TALENTS@ resolves source hand character through the live party");

    parameters[0] = 77u;
    check(run(&state, &action, talents_fetch_missing,
              (int)(sizeof(talents_fetch_missing) /
                    sizeof(talents_fetch_missing[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0u && execution.stack_depth == 0u,
          "TALENTS@ returns source zero for a missing in-party index");

    parameters[0] = 77u;
    check(run(&state, &action, talents_store_party,
              (int)(sizeof(talents_store_party) /
                    sizeof(talents_store_party[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              last_party_talents[1] == 0x55u && execution.stack_depth == 0u,
          "TALENTS! stores a source party champion talent word");

    parameters[0] = 77u;
    check(run(&state, &action, talents_store_then_bad,
              (int)(sizeof(talents_store_then_bad) /
                    sizeof(talents_store_then_bad[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              last_party_talents[1] == 0x1u,
          "TALENTS! rejects without publishing before a later bad opcode");

    parameters[0] = 77u;
    check(run(&state, &action, talents_fetch_wing,
              (int)(sizeof(talents_fetch_wing) /
                    sizeof(talents_fetch_wing[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u,
          "TALENTS@ keeps unbound CSBWin wing records unavailable");

    wing_talents_enabled = 1;
    parameters[0] = 77u;
    check(run(&state, &action, talents_fetch_wing,
              (int)(sizeof(talents_fetch_wing) /
                    sizeof(talents_fetch_wing[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0x5au && execution.stack_depth == 0u,
          "TALENTS@ reads a runtime-authenticated CSBWin wing record");
    parameters[0] = 77u;
    check(run(&state, &action, where_is_wing_character,
              (int)(sizeof(where_is_wing_character) /
                    sizeof(where_is_wing_character[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 5u && execution.stack_depth == 0u,
          "WHEREISCHAR reports a source-owned CSBWin wing character");
    parameters[0] = 77u;
    check(run(&state, &action, where_is_absent_character,
              (int)(sizeof(where_is_absent_character) /
                    sizeof(where_is_absent_character[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 4u && execution.stack_depth == 0u,
          "WHEREISCHAR retains source absent-character status");
    wing_talents_enabled = 0;

    check(run_save_policy(&state, &action, disable_saves,
                          (int)(sizeof(disable_saves) /
                                sizeof(disable_saves[0])), 0,
                          &saves_disabled) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              saves_disabled == 1,
          "DISABLESAVES stages the source save-policy disable state");

    check(run_save_policy(&state, &action, enable_saves,
                          (int)(sizeof(enable_saves) /
                                sizeof(enable_saves[0])), 1,
                          &saves_disabled) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              saves_disabled == 0,
          "DISABLESAVES clears the source save-policy state with zero");

    check(run_save_policy(&state, &action, disable_saves_then_bad,
                          (int)(sizeof(disable_saves_then_bad) /
                                sizeof(disable_saves_then_bad[0])), 0,
                          &saves_disabled) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              saves_disabled == 0,
          "DISABLESAVES rejects without publishing before a later bad opcode");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
    return failures == 0 ? 0 : 1;
}
