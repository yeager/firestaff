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
static int generator_delay_owner_valid;
static int generator_delay_stored;
static int generator_delay_store_count;
static int skin_owner_enabled;
static uint8_t skin_owner_value;
static int skin_owner_store_count;
static int monster_info_enabled;
static int monster_info_store_count;
static uint32_t monster_info_stored[8];
static uint8_t monster_info_store_mask;
static int cell_info_enabled;
static int false_pit_enabled;
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
static int missile_info_enabled;
static int missile_info_owner_valid;
static int missile_info_store_count;
static uint32_t missile_info_stored[4];
static int mastery_enabled;
static int party_info_enabled;
static int character_info_enabled;
static int character_store_enabled;
static int character_store_count;
static int32_t character_store_selector;
static uint32_t character_store_values[59];
static uint32_t character_store_word_count;
static int experience_plus_enabled;
static int experience_plus_count;
static int32_t experience_plus_character;
static int32_t experience_plus_skill;
static int32_t experience_plus_amount;
static int character_swap_enabled;
static int character_swap_commit_count;
static int32_t character_swap_index;
static int32_t character_swap_fingerprint;
static int cause_poison_enabled;
static int cause_poison_commit_count;
static int32_t cause_poison_character;
static int32_t cause_poison_value;
static int discard_text_enabled;
static int discard_text_count;
static int sound_enabled;
static int sound_count;
static int32_t sound_number;
static int32_t sound_volume;
static int32_t sound_flags;
static int monster_move_inhibit_enabled;
static int most_recent_interesting_object_enabled;
static int adjust_skills_parameters_enabled;
static int adjust_skills_parameters_count;
static uint32_t adjust_skills_parameters_values[5];
static int describe_enabled;
static int describe_count;
static int32_t describe_location, describe_index, describe_color;
static int actuator_copy_enabled;
static int actuator_copy_store_count;
static int actuator_copy_owner_valid;
static uint8_t actuator_copy_payloads[3][6];
static int switch_action_enabled;
static int switch_action_count;
static uint32_t switch_action_delay;
static uint32_t switch_action_kind;
static uint32_t switch_action_target;
static int switch_action_route;
static int teleporter_copy_enabled;
static int teleporter_copy_count;
static uint32_t teleporter_copy_source;
static uint32_t teleporter_copy_destination;

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

static int queue_switch_action(void *user, uint32_t delay, uint32_t action,
                               uint32_t target_location, int message_route,
                               uint8_t *out_event_type)
{
    (void)user;
    if (!switch_action_enabled) return -1;
    ++switch_action_count;
    switch_action_delay = delay;
    switch_action_kind = action;
    switch_action_target = target_location;
    switch_action_route = message_route;
    if (out_event_type) *out_event_type = 102u;
    return 1;
}

static int copy_teleporter(void *user, uint32_t source_location,
                           uint32_t destination_location)
{
    (void)user;
    if (!teleporter_copy_enabled) return -1;
    ++teleporter_copy_count;
    teleporter_copy_source = source_location;
    teleporter_copy_destination = destination_location;
    return 1;
}

static int actuator_copy_index(uint16_t thing)
{
    if (thing == 0x0123u) return 0;
    if (thing == 0x0456u) return 1;
    if (thing == 0x0789u) return 2;
    return -1;
}

static int get_actuator_payload(void *user, uint16_t thing,
                                uint8_t out_payload[6])
{
    int index;

    (void)user;
    if (!actuator_copy_enabled || !out_payload) return -1;
    index = actuator_copy_index(thing);
    if (index < 0) return 0;
    memcpy(out_payload, actuator_copy_payloads[index], 6u);
    return 1;
}

static int set_actuator_payload(void *user, uint16_t thing,
                                const uint8_t payload[6])
{
    int index;

    (void)user;
    if (!actuator_copy_enabled || !payload) return -1;
    index = actuator_copy_index(thing);
    if (index < 0) return 0;
    memcpy(actuator_copy_payloads[index], payload, 6u);
    ++actuator_copy_store_count;
    return 1;
}

static int copy_actuator_payload(void *user, uint16_t source_thing,
                                 uint16_t destination_thing,
                                 const uint8_t source_payload[6])
{
    int source_index;
    int destination_index;

    (void)user;
    if (!actuator_copy_enabled || !actuator_copy_owner_valid ||
        !source_payload) return -1;
    source_index = actuator_copy_index(source_thing);
    destination_index = actuator_copy_index(destination_thing);
    if (source_index < 0 || destination_index < 0) return 0;
    if (memcmp(actuator_copy_payloads[source_index], source_payload, 6u) != 0) {
        return 0;
    }
    memcpy(actuator_copy_payloads[destination_index], source_payload, 6u);
    ++actuator_copy_store_count;
    return 1;
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

static int get_skin(void *user, uint32_t location, uint8_t *out_skin)
{
    (void)user;
    if (!skin_owner_enabled || !out_skin || location != 0x0c82u) return 0;
    *out_skin = skin_owner_value;
    return 1;
}

static int set_skin(void *user, uint32_t location, uint8_t skin)
{
    (void)user;
    if (!skin_owner_enabled || location != 0x0c82u) return 0;
    skin_owner_value = skin;
    ++skin_owner_store_count;
    return 1;
}

static int commit_generator_delay(void *user, uint32_t location,
                                  int expected_delay, int delay)
{
    (void)user;
    if (!generator_delay_enabled || !generator_delay_owner_valid ||
        location != 0x0c82u || expected_delay != 37) return 0;
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
        if (false_pit_enabled) {
            out_values[0] = 3u;
            out_values[1] = 0x0cu;
        } else {
            out_values[0] = 4u;
            out_values[1] = 0x1du;
            out_values[2] = 5u;
            out_values[3] = 1u;
            out_values[4] = 12u;
        }
    }
    return 1;
}

static int resolve_cell_store(void *user, uint32_t location,
                              uint32_t expected_room_type)
{
    (void)user;
    if (!cell_info_enabled) return -1;
    return location == 0x0c82u &&
        (expected_room_type == (false_pit_enabled ? 3u : 4u)) ? 1 : 0;
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

static int get_missile_info(void *user, uint16_t thing,
                            uint32_t out_values[4])
{
    (void)user;
    if (!missile_info_enabled || !out_values) return -1;
    if (thing != 0x014eu) return 0;
    out_values[0] = 0x0123u;
    out_values[1] = 55u;
    out_values[2] = 44u;
    out_values[3] = 3u;
    return 1;
}

static int set_missile_info(void *user, uint16_t thing,
                            const uint32_t values[4])
{
    (void)user;
    if (!missile_info_enabled || !values || thing != 0x014eu) return 0;
    memcpy(missile_info_stored, values, sizeof(missile_info_stored));
    ++missile_info_store_count;
    return 1;
}

static int commit_missile_info(void *user, uint16_t thing,
                               const uint32_t expected_values[4],
                               const uint32_t values[4])
{
    static const uint32_t expected[4] = { 0x0123u, 55u, 44u, 3u };

    (void)user;
    if (!missile_info_owner_valid || !expected_values || !values ||
        memcmp(expected_values, expected, sizeof(expected)) != 0) return 0;
    return set_missile_info(user, thing, values);
}

static int get_mastery(void *user, uint32_t champion_index,
                       uint32_t skill_index, uint32_t flags,
                       uint32_t *out_mastery)
{
    (void)user;
    if (!mastery_enabled || !out_mastery) return -1;
    if (champion_index == 4u) champion_index = 1u;
    if (champion_index >= 3u || skill_index >= 20u) return 0;
    if (champion_index != 1u || skill_index != 7u || flags != 3u) return -1;
    *out_mastery = 9u;
    return 1;
}

static int get_party_info(void *user, uint32_t out_values[12])
{
    static const uint32_t source_party[12] = {
        4u, 5u, 10u, 12u, 3u, 1u, 1u, 0u, 2u, 1u, 17u, 19u
    };

    (void)user;
    if (!party_info_enabled || !out_values) return -1;
    memcpy(out_values, source_party, sizeof(source_party));
    return 1;
}

static int get_character_info(void *user, int32_t character_selector,
                              uint32_t out_values[59])
{
    uint32_t i;

    (void)user;
    if (!character_info_enabled || !out_values) return -1;
    if (character_selector == 99) return 0;
    if (character_selector != 4) return -1;
    for (i = 0u; i < 59u; ++i) out_values[i] = 1000u + i;
    out_values[0] = 3u;
    out_values[1] = 800u;
    out_values[2] = 90u;
    out_values[57] = 0x1234u;
    out_values[58] = 0x00f0u;
    return 1;
}

static int prepare_character_store(void *user, int32_t character_selector,
                                   uint32_t values[59], uint32_t word_count)
{
    (void)user;
    if (!character_store_enabled || !values || word_count > 59u) return -1;
    if (character_selector == 99) return 0;
    if (character_selector != 4) return -1;
    if (word_count > 2u && values[2] > 100u) values[2] = 100u;
    return 1;
}

static int set_character_info(void *user, int32_t character_selector,
                              const uint32_t values[59], uint32_t word_count)
{
    (void)user;
    if (!character_store_enabled || !values || character_selector != 4 ||
        word_count > 59u) {
        return 0;
    }
    character_store_selector = character_selector;
    character_store_word_count = word_count;
    memcpy(character_store_values, values, sizeof(character_store_values));
    ++character_store_count;
    return 1;
}

static int prepare_experience_plus(void *user, int32_t character_selector,
                                   int32_t skill_number, int32_t experience)
{
    (void)user;
    if (!experience_plus_enabled) return -1;
    if (character_selector == 99) return 0;
    if (character_selector != 1 || skill_number != 7 || experience != 200) {
        return -1;
    }
    return 1;
}

static int add_experience_plus(void *user, int32_t character_selector,
                               int32_t skill_number, int32_t experience)
{
    (void)user;
    if (!experience_plus_enabled || character_selector != 1 ||
        skill_number != 7 || experience != 200) {
        return 0;
    }
    experience_plus_character = character_selector;
    experience_plus_skill = skill_number;
    experience_plus_amount = experience;
    ++experience_plus_count;
    return 1;
}

static int prepare_character_swap(void *user, int32_t party_index,
                                  int32_t fingerprint, uint32_t *out_result)
{
    (void)user;
    if (!character_swap_enabled || !out_result) return -1;
    if (party_index == 99) {
        *out_result = 3u;
        return 0;
    }
    if (party_index != 1 || fingerprint != 0x2222) return -1;
    *out_result = 1u;
    return 1;
}

static int commit_character_swap(void *user, int32_t party_index,
                                 int32_t fingerprint)
{
    (void)user;
    if (!character_swap_enabled || party_index != 1 || fingerprint != 0x2222) {
        return 0;
    }
    character_swap_index = party_index;
    character_swap_fingerprint = fingerprint;
    ++character_swap_commit_count;
    return 1;
}

static int prepare_cause_poison(void *user, int32_t character_selector,
                                int32_t poison_value)
{
    (void)user;
    if (!cause_poison_enabled) return -1;
    if (character_selector == 99) return 0;
    return character_selector == 1 && poison_value == 128 ? 1 : -1;
}

static int commit_cause_poison(void *user, int32_t character_selector,
                               int32_t poison_value)
{
    (void)user;
    if (!cause_poison_enabled || character_selector != 1 ||
        poison_value != 128) return 0;
    cause_poison_character = character_selector;
    cause_poison_value = poison_value;
    ++cause_poison_commit_count;
    return 1;
}

static int discard_text(void *user)
{
    (void)user;
    if (!discard_text_enabled) return 0;
    ++discard_text_count;
    return 1;
}

static int play_sound(void *user, int32_t requested_number,
                      int32_t requested_volume, int32_t requested_flags)
{
    (void)user;
    if (!sound_enabled) return 0;
    sound_number = requested_number;
    sound_volume = requested_volume;
    sound_flags = requested_flags;
    ++sound_count;
    return 1;
}

static int set_adjust_skills_parameters(void *user,
                                        const uint32_t values[5])
{
    (void)user;
    if (!adjust_skills_parameters_enabled || !values) return 0;
    memcpy(adjust_skills_parameters_values, values,
           sizeof(adjust_skills_parameters_values));
    ++adjust_skills_parameters_count;
    return 1;
}
static int describe(void *user, int32_t location, int32_t index, int32_t color)
{
    (void)user;
    if (!describe_enabled) return 0;
    describe_location = location; describe_index = index; describe_color = color;
    ++describe_count;
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

static CSB_V1_CSBWinDSAStackResult run_with_parameter_count(
    CSB_V1_ChaosMagicState *state, CSB_V1_DSAImportedAction *action,
    uint16_t *words, int word_count, uint32_t *parameters,
    int parameter_count, CSB_V1_CSBWinDSAStackExecution *out_execution)
{
    CSB_V1_CSBWinDSAStackContext context;

    memset(&context, 0, sizeof(context));
    action->program_words = words;
    action->program_word_count = word_count;
    context.parameters = parameters;
    context.parameter_count = parameter_count;
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
        context.commit_generator_delay = commit_generator_delay;
    }
    if (skin_owner_enabled) {
        context.get_skin = get_skin;
        context.set_skin = set_skin;
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
    if (actuator_copy_enabled) {
        context.get_actuator_payload = get_actuator_payload;
        context.set_actuator_payload = set_actuator_payload;
        context.copy_actuator_payload = copy_actuator_payload;
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
    if (missile_info_enabled) {
        context.get_missile_info = get_missile_info;
        context.set_missile_info = set_missile_info;
        context.commit_missile_info = commit_missile_info;
    }
    if (mastery_enabled) context.get_mastery = get_mastery;
    if (party_info_enabled) context.get_party_info = get_party_info;
    if (character_info_enabled) context.get_character_info = get_character_info;
    if (character_store_enabled) {
        context.prepare_character_store = prepare_character_store;
        context.set_character_info = set_character_info;
    }
    if (experience_plus_enabled) {
        context.prepare_experience_plus = prepare_experience_plus;
        context.add_experience_plus = add_experience_plus;
    }
    if (character_swap_enabled) {
        context.prepare_character_swap = prepare_character_swap;
        context.commit_character_swap = commit_character_swap;
    }
    if (cause_poison_enabled) {
        context.prepare_cause_poison = prepare_cause_poison;
        context.commit_cause_poison = commit_cause_poison;
    }
    if (discard_text_enabled) context.discard_text = discard_text;
    if (sound_enabled) context.play_sound = play_sound;
    if (adjust_skills_parameters_enabled) {
        context.set_adjust_skills_parameters = set_adjust_skills_parameters;
    }
    if (describe_enabled) context.describe = describe;
    if (switch_action_enabled) context.queue_switch_action = queue_switch_action;
    if (teleporter_copy_enabled) context.copy_teleporter = copy_teleporter;
    context.monster_move_inhibit_valid = monster_move_inhibit_enabled;
    context.most_recent_interesting_object_valid =
        most_recent_interesting_object_enabled;
    context.most_recent_interesting_object = 0x3456u;
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

static CSB_V1_CSBWinDSAStackResult run(
    CSB_V1_ChaosMagicState *state, CSB_V1_DSAImportedAction *action,
    uint16_t *words, int word_count, uint32_t *parameters,
    CSB_V1_CSBWinDSAStackExecution *out_execution)
{
    return run_with_parameter_count(state, action, words, word_count,
                                    parameters, 4, out_execution);
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

static CSB_V1_CSBWinDSAStackResult run_modify_message(
    CSB_V1_ChaosMagicState *state, CSB_V1_DSAImportedAction *action,
    uint16_t *words, int word_count, uint8_t modifiers[3])
{
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;

    memset(&context, 0, sizeof(context));
    memset(&execution, 0, sizeof(execution));
    action->program_words = words;
    action->program_word_count = word_count;
    context.timer_type_modifiers_valid = 1;
    memcpy(context.timer_type_modifiers, modifiers,
           sizeof(context.timer_type_modifiers));
    {
        CSB_V1_CSBWinDSAStackResult result =
            csb_v1_csbwin_dsa_execute_authenticated_stack_action(
                state, 7, 1u, 0, &context, &execution);
        memcpy(modifiers, context.timer_type_modifiers,
               sizeof(context.timer_type_modifiers));
        return result;
    }
}

static CSB_V1_CSBWinDSAStackResult run_jitter(
    CSB_V1_ChaosMagicState *state, CSB_V1_DSAImportedAction *action,
    uint16_t *words, int word_count, int32_t offsets[4], int *changed)
{
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;

    memset(&context, 0, sizeof(context));
    memset(&execution, 0, sizeof(execution));
    action->program_words = words;
    action->program_word_count = word_count;
    context.jitter_state_valid = 1;
    context.x_graphic_jitter = offsets[0];
    context.y_graphic_jitter = offsets[1];
    context.x_overlay_jitter = offsets[2];
    context.y_overlay_jitter = offsets[3];
    context.jitter_changed = *changed;
    {
        CSB_V1_CSBWinDSAStackResult result =
            csb_v1_csbwin_dsa_execute_authenticated_stack_action(
                state, 7, 1u, 0, &context, &execution);
        offsets[0] = context.x_graphic_jitter;
        offsets[1] = context.y_graphic_jitter;
        offsets[2] = context.x_overlay_jitter;
        offsets[3] = context.y_overlay_jitter;
        *changed = context.jitter_changed;
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
    uint16_t skin_store[] = {
        0x0686u, 42u, 0x0686u, 0x0c82u, 0x0115u
    };
    uint16_t skin_store_then_bad[] = {
        0x0686u, 42u, 0x0686u, 0x0c82u, 0x0115u, 0x0000u
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
    uint16_t false_pit_set[] = {
        0x0686u, 0x0c82u, 0x0686u, 1u, 0x084bu
    };
    uint16_t false_pit_then_bad_opcode[] = {
        0x0686u, 0x0c82u, 0x0686u, 1u, 0x084bu, 0x0000u
    };
    uint16_t missile_info_fetch[] = {
        0x0686u, 0x014eu, 0x194bu, 0x000du, 0x004du, 0x008du, 0x00cdu
    };
    uint16_t missile_info_store[] = {
        0x0686u, 77u, 0x0686u, 66u, 0x0686u, 2u, 0x0686u, 0x014eu,
        0x198bu
    };
    uint16_t missile_info_store_then_bad_opcode[] = {
        0x0686u, 77u, 0x0686u, 66u, 0x0686u, 2u, 0x0686u, 0x014eu,
        0x198bu, 0x0000u
    };
    uint16_t mastery_query[] = {
        0x0686u, 4u, 0x0686u, 7u, 0x0686u, 3u, 0x0c4bu, 0x000du
    };
    uint16_t mastery_invalid[] = {
        0x0686u, 9u, 0x0686u, 7u, 0x0686u, 3u, 0x0c4bu, 0x000du
    };
    uint16_t party_fetch[] = {
        0x0686u, 0u, 0x0686u, 12u, 0x100bu,
        0x0686u, 12u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t party_fetch_bad_span[] = {
        0x0686u, 99u, 0x0686u, 2u, 0x100bu
    };
    uint16_t character_fetch_head[] = {
        0x0686u, 4u, 0x0686u, 0u, 0x0686u, 59u, 0x104bu,
        0x0686u, 26u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t character_fetch_tail[] = {
        0x0686u, 4u, 0x0686u, 0u, 0x0686u, 59u, 0x104bu,
        0x0686u, 7u, 0x0686u, 52u, 0x0a4bu
    };
    uint16_t character_fetch_invalid[] = {
        0x0686u, 99u, 0x0686u, 0u, 0x0686u, 26u, 0x104bu,
        0x0686u, 26u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t character_fetch_bad_span[] = {
        0x0686u, 4u, 0x0686u, 99u, 0x0686u, 2u, 0x104bu,
        0x0686u, 1u, 0x0686u, 99u, 0x0a4bu
    };
    uint16_t character_fetch_negative[] = {
        0x0786u, 0xffffu, 0xffffu, 0x0686u, 0u, 0x0686u, 1u, 0x104bu,
        0x0686u, 1u, 0x0686u, 0u, 0x0a4bu
    };
    uint16_t character_store[] = {
        0x0686u, 421u, 0x0686u, 1u, 0x0686u, 1u, 0x0215u,
        0x0686u, 250u, 0x0686u, 2u, 0x0686u, 1u, 0x0215u,
        0x0686u, 144u, 0x0686u, 4u, 0x0686u, 1u, 0x0215u,
        0x0686u, 4u, 0x0686u, 0u, 0x0686u, 5u, 0x108bu,
        0x0686u, 1u, 0x0686u, 2u, 0x0a4bu
    };
    uint16_t character_store_then_bad[] = {
        0x0686u, 421u, 0x0686u, 1u, 0x0686u, 1u, 0x0215u,
        0x0686u, 250u, 0x0686u, 2u, 0x0686u, 1u, 0x0215u,
        0x0686u, 4u, 0x0686u, 0u, 0x0686u, 5u, 0x108bu, 0x0000u
    };
    uint16_t character_store_invalid[] = {
        0x0686u, 99u, 0x0686u, 0u, 0x0686u, 5u, 0x108bu
    };
    uint16_t experience_plus[] = {
        0x0686u, 1u, 0x0686u, 7u, 0x0686u, 200u, 0x1c4bu
    };
    uint16_t experience_plus_then_bad[] = {
        0x0686u, 1u, 0x0686u, 7u, 0x0686u, 200u, 0x1c4bu, 0x0000u
    };
    uint16_t experience_plus_invalid[] = {
        0x0686u, 99u, 0x0686u, 7u, 0x0686u, 200u, 0x1c4bu
    };
    uint16_t experience_plus_zero[] = {
        0x0686u, 1u, 0x0686u, 7u, 0x0686u, 0u, 0x1c4bu
    };
    uint16_t character_swap[] = {
        0x0686u, 1u, 0x0686u, 0x2222u, 0x1d8bu, 0x080du
    };
    uint16_t character_swap_then_bad[] = {
        0x0686u, 1u, 0x0686u, 0x2222u, 0x1d8bu, 0x0000u
    };
    uint16_t character_swap_full_party[] = {
        0x0686u, 99u, 0x0686u, 0x2222u, 0x1d8bu, 0x080du
    };
    uint16_t cause_poison[] = {
        0x0686u, 128u, 0x0686u, 1u, 0x1b8bu
    };
    uint16_t cause_poison_then_bad[] = {
        0x0686u, 128u, 0x0686u, 1u, 0x1b8bu, 0x0000u
    };
    uint16_t cause_poison_invalid[] = {
        0x0686u, 128u, 0x0686u, 99u, 0x1b8bu
    };
    uint16_t cause_poison_negative[] = {
        0x0686u, 128u, 0x0786u, 0xffffu, 0xffffu, 0x1b8bu
    };
    uint16_t discard_text[] = { 0x1ccbu };
    uint16_t discard_text_then_bad[] = { 0x1ccbu, 0x0000u };
    uint16_t sound[] = { 0x0686u, 7u, 0x0686u, 100u, 0x0686u, 3u, 0x114bu };
    uint16_t sound_then_bad[] = {
        0x0686u, 7u, 0x0686u, 100u, 0x0686u, 3u, 0x114bu, 0x0000u
    };
    uint16_t monblk[] = { 0x0686u, 0x0du, 0x11cbu };
    /* Data.h assigns STKOP_ObjectID 123. EX_AMPERSAND keeps the complete
     * seven-bit subcode, so the authenticated source word is 123 << 6 | 11. */
    uint16_t object_id[] = { 0x1ecbu };
    uint16_t monblk_then_bad[] = { 0x0686u, 0x0du, 0x11cbu, 0x0000u };
    uint16_t set_adjust_skills[] = {
        0x0686u, 10u, 0x0686u, 20u, 0x0686u, 30u, 0x0686u, 40u,
        0x0686u, 50u, 0x170bu
    };
    uint16_t set_adjust_skills_then_bad[] = {
        0x0686u, 10u, 0x0686u, 20u, 0x0686u, 30u, 0x0686u, 40u,
        0x0686u, 50u, 0x170bu, 0x0000u
    };
    uint16_t describe_words[] = { 0x0686u, 0x1234u, 0x0686u, 3u, 0x0686u, 9u, 0x120bu };
    uint16_t describe_then_bad[] = {
        0x0686u, 0x1234u, 0x0686u, 3u, 0x0686u, 9u, 0x120bu, 0x0000u
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
    uint16_t modify_message[] = {
        0x0686u, 2u, 0x0686u, 9u, 0x0686u, 7u, 0x0295u
    };
    uint16_t modify_message_then_bad[] = {
        0x0686u, 2u, 0x0686u, 9u, 0x0686u, 7u, 0x0295u, 0x0000u
    };
    uint16_t jitter[] = {
        0x0686u, 10u, 0x0686u, 11u, 0x0686u, 12u, 0x0686u, 13u, 0x0255u
    };
    uint16_t jitter_then_bad[] = {
        0x0686u, 10u, 0x0686u, 11u, 0x0686u, 12u, 0x0686u, 13u,
        0x0255u, 0x0000u
    };
    uint16_t actuator_copy_chain[] = {
        0x0686u, 0x0123u, 0x0686u, 0x0456u, 0x130bu,
        0x0686u, 0x0456u, 0x0686u, 0x0789u, 0x130bu
    };
    uint16_t actuator_copy_then_bad[] = {
        0x0686u, 0x0123u, 0x0686u, 0x0456u, 0x130bu, 0x0000u
    };
    uint16_t message[] = { 0x0b41u, 2u, 0u };
    uint16_t message_then_bad[] = { 0x0b41u, 2u, 0u, 0x114bu };
    uint16_t copy_teleporter[] = { 0x0284u, 0x0421u, 0x0842u };
    uint16_t copy_teleporter_then_bad[] = {
        0x0284u, 0x0421u, 0x0842u, 0x0000u
    };
    uint16_t actuator_copy_non_db3[] = {
        0x0686u, 0x0aaau, 0x0686u, 0x0456u, 0x130bu
    };
    uint32_t parameters[4] = { 77u, 0u, 0u, 0u };
    CSB_V1_DSAImportedAction action;
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAStackExecution execution;
    int saves_disabled = -1;
    uint8_t timer_type_modifiers[3] = { 0u, 1u, 2u };
    int32_t jitter_offsets[4] = { 1, 2, 3, 4 };
    int jitter_changed = 0;

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
    generator_delay_owner_valid = 1;
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
              generator_delay_store_count == 1 &&
              execution.generator_delay_store_count == 1u &&
              execution.last_generator_delay_location == 0x0c82u &&
              execution.last_generator_delay_before == 37 &&
              execution.last_generator_delay_after == 91 &&
              execution.last_generator_delay_has_generator,
          "GeneratorDelay! stages DB3 delay, exposes it to same-action fetch, and commits once");
    generator_delay_stored = -1;
    generator_delay_store_count = 0;
    check(run(&state, &action, generator_delay_store_then_bad,
              (int)(sizeof(generator_delay_store_then_bad) /
                    sizeof(generator_delay_store_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              generator_delay_store_count == 0,
          "GeneratorDelay! rejects a later unsupported source word without DB3 mutation");
    generator_delay_stored = -1;
    generator_delay_store_count = 0;
    generator_delay_owner_valid = 0;
    check(run(&state, &action, generator_delay_store_then_fetch,
              (int)(sizeof(generator_delay_store_then_fetch) /
                    sizeof(generator_delay_store_then_fetch[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              generator_delay_store_count == 0 && generator_delay_stored == -1,
          "GeneratorDelay! rejects a stale DB3 owner before candidate publication");
    generator_delay_owner_valid = 1;
    generator_delay_enabled = 0;

    skin_owner_enabled = 1;
    skin_owner_value = 4u;
    skin_owner_store_count = 0;
    check(run(&state, &action, skin_store,
              (int)(sizeof(skin_store) / sizeof(skin_store[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              skin_owner_store_count == 1 && skin_owner_value == 42u &&
              execution.skin_store_count == 1u &&
              execution.last_skin_store_location == 0x0c82u &&
              execution.last_skin_store_before == 4u &&
              execution.last_skin_store_after == 42u,
          "SETSKIN commits a source-owned EXPOOL skin byte with its pre/post receipt");
    skin_owner_value = 4u;
    skin_owner_store_count = 0;
    check(run(&state, &action, skin_store_then_bad,
              (int)(sizeof(skin_store_then_bad) /
                    sizeof(skin_store_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              skin_owner_store_count == 0 && skin_owner_value == 4u,
          "SETSKIN rejects a later unsupported opcode before EXPOOL publication");
    skin_owner_enabled = 0;
    check(run(&state, &action, skin_store,
              (int)(sizeof(skin_store) / sizeof(skin_store[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              skin_owner_store_count == 0 && skin_owner_value == 4u,
          "SETSKIN rejects a missing authenticated EXPOOL owner before staging");

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
    cell_info_enabled = 1;
    false_pit_enabled = 1;
    cell_info_store_count = 0;
    memset(cell_info_stored, 0, sizeof(cell_info_stored));
    check(run(&state, &action, false_pit_set,
              (int)(sizeof(false_pit_set) / sizeof(false_pit_set[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              cell_info_store_count == 1 && cell_info_stored[0] == 3u &&
              cell_info_stored[1] == 0x0du &&
              cell_info_store_mask == (1u << 1),
          "FALSEPIT writes only the source roomPIT false bit");
    cell_info_store_count = 0;
    check(run(&state, &action, false_pit_then_bad_opcode,
              (int)(sizeof(false_pit_then_bad_opcode) /
                    sizeof(false_pit_then_bad_opcode[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              cell_info_store_count == 0,
          "FALSEPIT remains staged after a later rejected opcode");
    false_pit_enabled = 0;
    cell_info_enabled = 0;
    missile_info_enabled = 1;
    missile_info_owner_valid = 1;
    memset(parameters, 0, sizeof(parameters));
    check(run(&state, &action, missile_info_fetch,
              (int)(sizeof(missile_info_fetch) /
                    sizeof(missile_info_fetch[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 3u && parameters[1] == 44u &&
              parameters[2] == 55u && parameters[3] == 0x0123u &&
              execution.stack_depth == 0u,
          "MISSILEINFO@ reads DB14 and source timer direction in stack order");
    missile_info_store_count = 0;
    memset(missile_info_stored, 0, sizeof(missile_info_stored));
    check(run(&state, &action, missile_info_store,
              (int)(sizeof(missile_info_store) /
                    sizeof(missile_info_store[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              missile_info_store_count == 1 && missile_info_stored[0] == 0x0123u &&
              missile_info_stored[1] == 77u && missile_info_stored[2] == 66u &&
              missile_info_stored[3] == 2u &&
              execution.missile_info_store_count == 1u &&
              execution.last_missile_info_thing == 0x014eu &&
              execution.last_missile_info_before[0] == 0x0123u &&
              execution.last_missile_info_before[1] == 55u &&
              execution.last_missile_info_before[2] == 44u &&
              execution.last_missile_info_before[3] == 3u &&
              execution.last_missile_info_after[0] == 0x0123u &&
              execution.last_missile_info_after[1] == 77u &&
              execution.last_missile_info_after[2] == 66u &&
              execution.last_missile_info_after[3] == 2u,
          "MISSILEINFO! commits DB14 range damage and timer direction together");
    missile_info_store_count = 0;
    check(run(&state, &action, missile_info_store_then_bad_opcode,
              (int)(sizeof(missile_info_store_then_bad_opcode) /
                    sizeof(missile_info_store_then_bad_opcode[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              missile_info_store_count == 0,
          "MISSILEINFO! remains staged after a later rejected opcode");
    missile_info_store_count = 0;
    missile_info_owner_valid = 0;
    check(run(&state, &action, missile_info_store,
              (int)(sizeof(missile_info_store) /
                    sizeof(missile_info_store[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              missile_info_store_count == 0,
          "MISSILEINFO! rejects stale DB14/TIMER ownership before mutation");
    missile_info_owner_valid = 1;
    missile_info_enabled = 0;
    mastery_enabled = 1;
    memset(parameters, 0, sizeof(parameters));
    check(run(&state, &action, mastery_query,
              (int)(sizeof(mastery_query) / sizeof(mastery_query[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 9u && execution.stack_depth == 0u,
          "MASTERY preserves source hand-character and flag stack order");
    memset(parameters, 0xff, sizeof(parameters));
    check(run(&state, &action, mastery_invalid,
              (int)(sizeof(mastery_invalid) / sizeof(mastery_invalid[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0u && execution.stack_depth == 0u,
          "MASTERY preserves source zero for an invalid character");
    mastery_enabled = 0;
    party_info_enabled = 1;
    {
        uint32_t party_parameters[12] = { 0u };

        check(run_with_parameter_count(
                  &state, &action, party_fetch,
                  (int)(sizeof(party_fetch) / sizeof(party_fetch[0])),
                  party_parameters, 12, &execution) ==
                  CSB_V1_CSBWIN_DSA_STACK_OK &&
              party_parameters[0] == 4u && party_parameters[1] == 5u &&
              party_parameters[2] == 10u && party_parameters[3] == 12u &&
              party_parameters[4] == 3u && party_parameters[5] == 1u &&
              party_parameters[6] == 1u && party_parameters[7] == 0u &&
              party_parameters[8] == 2u && party_parameters[9] == 1u &&
              party_parameters[10] == 17u && party_parameters[11] == 19u,
              "PARTY@ copies the complete source GAMEBLOCK2 word order");
        memset(party_parameters, 0x5a, sizeof(party_parameters));
        check(run_with_parameter_count(
                  &state, &action, party_fetch_bad_span,
                  (int)(sizeof(party_fetch_bad_span) /
                        sizeof(party_fetch_bad_span[0])), party_parameters,
                  12, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              party_parameters[0] == 0x5a5a5a5au,
              "PARTY@ preserves the source out-of-bank no-op");
    }
    party_info_enabled = 0;
    parameters[0] = 77u;
    check(run(&state, &action, party_fetch,
              (int)(sizeof(party_fetch) / sizeof(party_fetch[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u,
          "PARTY@ rejects without a complete GAMEBLOCK2 owner");
    character_info_enabled = 1;
    {
        uint32_t character_parameters[26] = { 0u };

        check(run_with_parameter_count(
                  &state, &action, character_fetch_head,
                  (int)(sizeof(character_fetch_head) /
                        sizeof(character_fetch_head[0])), character_parameters,
                  26, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              character_parameters[0] == 3u &&
              character_parameters[1] == 800u &&
              character_parameters[2] == 90u &&
              character_parameters[25] == 1025u,
              "CHAR@ copies source CHARDESC head words in order");
        memset(character_parameters, 0, sizeof(character_parameters));
        check(run_with_parameter_count(
                  &state, &action, character_fetch_tail,
                  (int)(sizeof(character_fetch_tail) /
                        sizeof(character_fetch_tail[0])), character_parameters,
                  26, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              character_parameters[0] == 1052u &&
              character_parameters[4] == 1056u &&
              character_parameters[5] == 0x1234u &&
              character_parameters[6] == 0x00f0u,
              "CHAR@ copies source skill tail, fingerprint, and talents");
        memset(character_parameters, 0xff, sizeof(character_parameters));
        check(run_with_parameter_count(
                  &state, &action, character_fetch_invalid,
                  (int)(sizeof(character_fetch_invalid) /
                        sizeof(character_fetch_invalid[0])), character_parameters,
                  26, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              character_parameters[0] == 0u && character_parameters[25] == 0u,
              "CHAR@ preserves the source invalid-character zero image");
        memset(character_parameters, 0x5a, sizeof(character_parameters));
        check(run_with_parameter_count(
                  &state, &action, character_fetch_bad_span,
                  (int)(sizeof(character_fetch_bad_span) /
                        sizeof(character_fetch_bad_span[0])), character_parameters,
                  26, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              character_parameters[0] == 0u,
              "CHAR@ zeroes the source oversized destination tail");
    }
    character_info_enabled = 0;
    parameters[0] = 77u;
    check(run(&state, &action, character_fetch_negative,
              (int)(sizeof(character_fetch_negative) /
                    sizeof(character_fetch_negative[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 0u,
          "CHAR@ preserves the source negative-selector zero image");
    parameters[0] = 77u;
    check(run(&state, &action, character_fetch_head,
              (int)(sizeof(character_fetch_head) /
                    sizeof(character_fetch_head[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u,
          "CHAR@ rejects without a complete CHARDESC owner");
    character_store_enabled = 1;
    character_store_count = 0;
    memset(character_store_values, 0, sizeof(character_store_values));
    parameters[0] = 77u;
    check(run(&state, &action, character_store,
              (int)(sizeof(character_store) / sizeof(character_store[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              character_store_count == 1 && character_store_selector == 4 &&
              character_store_word_count == 5u &&
              character_store_values[1] == 321u &&
              character_store_values[2] == 100u &&
              character_store_values[4] == 44u && parameters[0] == 100u,
          "CHAR! stages source CHARDESC fields and exposes clamped health");
    character_store_count = 0;
    parameters[0] = 77u;
    check(run(&state, &action, character_store_then_bad,
              (int)(sizeof(character_store_then_bad) /
                    sizeof(character_store_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              character_store_count == 0 && parameters[0] == 77u,
          "CHAR! does not publish before a later rejected source word");
    character_store_count = 0;
    check(run(&state, &action, character_store_invalid,
              (int)(sizeof(character_store_invalid) /
                    sizeof(character_store_invalid[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              character_store_count == 0,
          "CHAR! preserves the source invalid-character no-op");
    character_store_enabled = 0;
    parameters[0] = 77u;
    check(run(&state, &action, character_store,
              (int)(sizeof(character_store) / sizeof(character_store[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              parameters[0] == 77u,
          "CHAR! rejects without a complete CHARDESC owner");
    experience_plus_enabled = 1;
    experience_plus_count = 0;
    check(run(&state, &action, experience_plus,
              (int)(sizeof(experience_plus) / sizeof(experience_plus[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              experience_plus_count == 1 && experience_plus_character == 1 &&
              experience_plus_skill == 7 && experience_plus_amount == 200,
          "EXPERIENCE+ preserves CSBWin character, skill, and XP stack order");
    experience_plus_count = 0;
    check(run(&state, &action, experience_plus_then_bad,
              (int)(sizeof(experience_plus_then_bad) /
                    sizeof(experience_plus_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              experience_plus_count == 0,
          "EXPERIENCE+ does not publish before a later rejected source word");
    check(run(&state, &action, experience_plus_invalid,
              (int)(sizeof(experience_plus_invalid) /
                    sizeof(experience_plus_invalid[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              experience_plus_count == 0,
          "EXPERIENCE+ preserves the source unavailable-character no-op");
    check(run(&state, &action, experience_plus_zero,
              (int)(sizeof(experience_plus_zero) /
                    sizeof(experience_plus_zero[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              experience_plus_count == 0,
          "EXPERIENCE+ preserves AddToSkill's nonpositive-XP no-op");
    experience_plus_enabled = 0;
    check(run(&state, &action, experience_plus,
              (int)(sizeof(experience_plus) / sizeof(experience_plus[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "EXPERIENCE+ rejects without a complete skill owner");
    character_swap_enabled = 1;
    character_swap_commit_count = 0;
    parameters[0] = 77u;
    check(run(&state, &action, character_swap,
              (int)(sizeof(character_swap) / sizeof(character_swap[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 1u && character_swap_commit_count == 1 &&
              character_swap_index == 1 && character_swap_fingerprint == 0x2222,
          "SWAPCHARACTER preserves source stack result and staged roster call");
    character_swap_commit_count = 0;
    parameters[0] = 77u;
    check(run(&state, &action, character_swap_then_bad,
              (int)(sizeof(character_swap_then_bad) /
                    sizeof(character_swap_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              character_swap_commit_count == 0 && parameters[0] == 77u,
          "SWAPCHARACTER does not publish before a later rejected source word");
    parameters[0] = 77u;
    check(run(&state, &action, character_swap_full_party,
              (int)(sizeof(character_swap_full_party) /
                    sizeof(character_swap_full_party[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              parameters[0] == 3u && character_swap_commit_count == 0,
          "SWAPCHARACTER preserves the source full-party return code");
    character_swap_enabled = 0;
    check(run(&state, &action, character_swap,
              (int)(sizeof(character_swap) / sizeof(character_swap[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "SWAPCHARACTER rejects without a complete roster owner");
    cause_poison_enabled = 1;
    cause_poison_commit_count = 0;
    check(run(&state, &action, cause_poison,
              (int)(sizeof(cause_poison) / sizeof(cause_poison[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              cause_poison_commit_count == 1 && cause_poison_character == 1 &&
              cause_poison_value == 128,
          "CAUSEPOISON preserves CSBWin poison-value and character stack order");
    cause_poison_commit_count = 0;
    check(run(&state, &action, cause_poison_then_bad,
              (int)(sizeof(cause_poison_then_bad) /
                    sizeof(cause_poison_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              cause_poison_commit_count == 0,
          "CAUSEPOISON does not publish before a later rejected source word");
    check(run(&state, &action, cause_poison_invalid,
              (int)(sizeof(cause_poison_invalid) /
                    sizeof(cause_poison_invalid[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              cause_poison_commit_count == 0,
          "CAUSEPOISON preserves the source unavailable-character no-op");
    cause_poison_enabled = 0;
    check(run(&state, &action, cause_poison_negative,
              (int)(sizeof(cause_poison_negative) /
                    sizeof(cause_poison_negative[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              cause_poison_commit_count == 0,
          "CAUSEPOISON preserves the source negative-character no-op");
    check(run(&state, &action, cause_poison,
              (int)(sizeof(cause_poison) / sizeof(cause_poison[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "CAUSEPOISON rejects without a complete poison owner");
    discard_text_enabled = 1;
    discard_text_count = 0;
    check(run(&state, &action, discard_text, 1, parameters, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && discard_text_count == 1,
          "DISCARDTEXT commits the source UI discard after accepted bytecode");
    discard_text_count = 0;
    check(run(&state, &action, discard_text_then_bad, 2, parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              discard_text_count == 0,
          "DISCARDTEXT does not publish before a later rejected source word");
    discard_text_enabled = 0;

    sound_enabled = 1;
    sound_count = 0;
    check(run(&state, &action, sound,
              (int)(sizeof(sound) / sizeof(sound[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              sound_count == 1 && sound_number == 7 && sound_volume == 100 &&
              sound_flags == 3,
          "SOUND preserves CSBWin sound/volume/flags pop order");
    sound_count = 0;
    check(run(&state, &action, sound_then_bad,
              (int)(sizeof(sound_then_bad) / sizeof(sound_then_bad[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              sound_count == 0,
          "SOUND does not publish before a later rejected source word");
    sound_enabled = 0;
    check(run(&state, &action, sound,
              (int)(sizeof(sound) / sizeof(sound[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "SOUND remains closed without a CSB audio owner");

    switch_action_enabled = 1;
    switch_action_count = 0;
    check(run(&state, &action, message,
              (int)(sizeof(message) / sizeof(message[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              switch_action_count == 1 && switch_action_delay == 2u &&
              switch_action_kind == 0u && switch_action_target == 0u &&
              switch_action_route == 'M' &&
              execution.timer_scheduled_count == 1u,
          "MESSAGE commits its source QueueDSASwitchAction only after acceptance");
    switch_action_count = 0;
    check(run(&state, &action, message_then_bad,
              (int)(sizeof(message_then_bad) / sizeof(message_then_bad[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              switch_action_count == 0,
          "MESSAGE does not queue a timer before a later rejected opcode");
    switch_action_enabled = 0;

    teleporter_copy_enabled = 1;
    teleporter_copy_count = 0;
    check(run(&state, &action, copy_teleporter,
              (int)(sizeof(copy_teleporter) / sizeof(copy_teleporter[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              teleporter_copy_count == 1 && teleporter_copy_source == 0x0421u &&
              teleporter_copy_destination == 0x0842u &&
              execution.teleporter_copy_count == 1u,
          "COPYTELEPORTER commits the original source and destination only after acceptance");
    teleporter_copy_count = 0;
    check(run(&state, &action, copy_teleporter_then_bad,
              (int)(sizeof(copy_teleporter_then_bad) /
                    sizeof(copy_teleporter_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              teleporter_copy_count == 0,
          "COPYTELEPORTER does not mutate before a later rejected opcode");
    teleporter_copy_enabled = 0;

    monster_move_inhibit_enabled = 1;
    check(run(&state, &action, monblk,
              (int)(sizeof(monblk) / sizeof(monblk[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              execution.stack_depth == 0u,
          "MONBLK accepts CSBWin's four-direction mask");
    check(run(&state, &action, monblk_then_bad,
              (int)(sizeof(monblk_then_bad) / sizeof(monblk_then_bad[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "MONBLK rejects a later malformed source word");
    monster_move_inhibit_enabled = 0;
    most_recent_interesting_object_enabled = 1;
    check(run(&state, &action, object_id, 1, parameters, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && execution.stack_depth == 1u,
          "OBJECTID reads the source DSA-bank interesting object");
    most_recent_interesting_object_enabled = 0;
    check(run(&state, &action, object_id, 1, parameters, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "OBJECTID remains closed without a DSA-bank owner");
    adjust_skills_parameters_enabled = 1;
    adjust_skills_parameters_count = 0;
    memset(adjust_skills_parameters_values, 0,
           sizeof(adjust_skills_parameters_values));
    check(run(&state, &action, set_adjust_skills,
              (int)(sizeof(set_adjust_skills) / sizeof(set_adjust_skills[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              adjust_skills_parameters_count == 1 &&
              adjust_skills_parameters_values[0] == 10u &&
              adjust_skills_parameters_values[1] == 20u &&
              adjust_skills_parameters_values[2] == 30u &&
              adjust_skills_parameters_values[3] == 40u &&
              adjust_skills_parameters_values[4] == 50u,
          "SETADJUSTSKILLSPARAMETERS preserves CSBWin's five-value pop order");
    adjust_skills_parameters_count = 0;
    check(run(&state, &action, set_adjust_skills_then_bad,
              (int)(sizeof(set_adjust_skills_then_bad) /
                    sizeof(set_adjust_skills_then_bad[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              adjust_skills_parameters_count == 0,
          "SETADJUSTSKILLSPARAMETERS does not publish before later rejection");
    adjust_skills_parameters_enabled = 0;
    check(run(&state, &action, set_adjust_skills,
              (int)(sizeof(set_adjust_skills) / sizeof(set_adjust_skills[0])),
              parameters, &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "SETADJUSTSKILLSPARAMETERS remains closed without Magic owner");
    describe_enabled = 1; describe_count = 0;
    check(run(&state, &action, describe_words,
              (int)(sizeof(describe_words) / sizeof(describe_words[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK && describe_count == 1 &&
              describe_location == 0x1234 && describe_index == 3 && describe_color == 9,
          "DESCRIBE preserves Character.cpp location/index/color pop order");
    describe_count = 0;
    check(run(&state, &action, describe_then_bad,
              (int)(sizeof(describe_then_bad) / sizeof(describe_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED && describe_count == 0,
          "DESCRIBE does not publish before later rejection");
    describe_enabled = 0;
    check(run(&state, &action, describe_words,
              (int)(sizeof(describe_words) / sizeof(describe_words[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "DESCRIBE remains closed without a DB2 phrase owner");
    check(run(&state, &action, discard_text, 1, parameters, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "DISCARDTEXT rejects without the source UI owner");
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
              excell_flags_store_count == 1 && excell_flags_stored == 0x89u &&
              execution.excell_store_count == 1u &&
              execution.last_excell_store_location == 0x0c82u &&
              execution.last_excell_store_before[0] == (1u << 2) &&
              execution.last_excell_store_before[3] == (1u << 2) &&
              execution.last_excell_store_before[7] == (1u << 2) &&
              execution.last_excell_store_after[0] == (1u << 2) &&
              execution.last_excell_store_after[3] == (1u << 2) &&
              execution.last_excell_store_after[7] == (1u << 2),
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

    timer_type_modifiers[0] = 0u;
    timer_type_modifiers[1] = 1u;
    timer_type_modifiers[2] = 2u;
    check(run_modify_message(&state, &action, modify_message,
              (int)(sizeof(modify_message) / sizeof(modify_message[0])),
              timer_type_modifiers) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              timer_type_modifiers[0] == 3u &&
              timer_type_modifiers[1] == 3u &&
              timer_type_modifiers[2] == 2u,
          "MODIFYMESSAGE preserves source SET/CLEAR/TOGGLE stack order and clamps above three");
    timer_type_modifiers[0] = 0u;
    timer_type_modifiers[1] = 1u;
    timer_type_modifiers[2] = 2u;
    check(run_modify_message(&state, &action, modify_message_then_bad,
              (int)(sizeof(modify_message_then_bad) /
                    sizeof(modify_message_then_bad[0])),
              timer_type_modifiers) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              timer_type_modifiers[0] == 0u &&
              timer_type_modifiers[1] == 1u &&
              timer_type_modifiers[2] == 2u,
          "MODIFYMESSAGE does not publish a timer remap before a later rejected word");

    jitter_offsets[0] = 1;
    jitter_offsets[1] = 2;
    jitter_offsets[2] = 3;
    jitter_offsets[3] = 4;
    jitter_changed = 0;
    check(run_jitter(&state, &action, jitter,
              (int)(sizeof(jitter) / sizeof(jitter[0])), jitter_offsets,
              &jitter_changed) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              jitter_offsets[0] == 10 && jitter_offsets[1] == 11 &&
              jitter_offsets[2] == 12 && jitter_offsets[3] == 13 &&
              jitter_changed == 1,
          "JITTER preserves CSBWin graphic/overlay stack order and raises the redraw latch");
    jitter_offsets[0] = 1;
    jitter_offsets[1] = 2;
    jitter_offsets[2] = 3;
    jitter_offsets[3] = 4;
    jitter_changed = 0;
    check(run_jitter(&state, &action, jitter_then_bad,
              (int)(sizeof(jitter_then_bad) / sizeof(jitter_then_bad[0])),
              jitter_offsets, &jitter_changed) ==
              CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              jitter_offsets[0] == 1 && jitter_offsets[1] == 2 &&
              jitter_offsets[2] == 3 && jitter_offsets[3] == 4 &&
              jitter_changed == 0,
          "JITTER does not publish offsets or redraw state before a later rejected word");

    memcpy(actuator_copy_payloads[0], "ABCDEF", 6u);
    memcpy(actuator_copy_payloads[1], "ghijkl", 6u);
    memcpy(actuator_copy_payloads[2], "mnopqr", 6u);
    actuator_copy_enabled = 1;
    actuator_copy_owner_valid = 1;
    actuator_copy_store_count = 0;
    check(run(&state, &action, actuator_copy_chain,
              (int)(sizeof(actuator_copy_chain) /
                    sizeof(actuator_copy_chain[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              actuator_copy_store_count == 2 &&
              memcmp(actuator_copy_payloads[1], "ABCDEF", 6u) == 0 &&
              memcmp(actuator_copy_payloads[2], "ABCDEF", 6u) == 0 &&
              execution.actuator_copy_count == 2 &&
              execution.last_actuator_copy_source_thing == 0x0456u &&
              execution.last_actuator_copy_destination_thing == 0x0789u,
          "COPY stages CSBWin DB3 payloads and receipts chained source bytes");
    memcpy(actuator_copy_payloads[0], "ABCDEF", 6u);
    memcpy(actuator_copy_payloads[1], "ghijkl", 6u);
    actuator_copy_store_count = 0;
    check(run(&state, &action, actuator_copy_then_bad,
              (int)(sizeof(actuator_copy_then_bad) /
                    sizeof(actuator_copy_then_bad[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              actuator_copy_store_count == 0 &&
              memcmp(actuator_copy_payloads[1], "ghijkl", 6u) == 0,
          "COPY does not publish a DB3 payload before a later rejected word");
    actuator_copy_store_count = 0;
    check(run(&state, &action, actuator_copy_non_db3,
              (int)(sizeof(actuator_copy_non_db3) /
                    sizeof(actuator_copy_non_db3[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_OK &&
              actuator_copy_store_count == 0 &&
              execution.actuator_copy_count == 0 &&
              memcmp(actuator_copy_payloads[1], "ghijkl", 6u) == 0,
          "COPY retains CSBWin's non-DB3 source no-op");
    memcpy(actuator_copy_payloads[0], "ABCDEF", 6u);
    memcpy(actuator_copy_payloads[1], "ghijkl", 6u);
    actuator_copy_store_count = 0;
    actuator_copy_owner_valid = 0;
    check(run(&state, &action, actuator_copy_chain,
              (int)(sizeof(actuator_copy_chain) /
                    sizeof(actuator_copy_chain[0])), parameters,
              &execution) == CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              actuator_copy_store_count == 0 &&
              memcmp(actuator_copy_payloads[1], "ghijkl", 6u) == 0,
          "COPY rejects a stale DB3 source/destination owner before mutation");
    actuator_copy_owner_valid = 1;
    actuator_copy_enabled = 0;

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
    return failures == 0 ? 0 : 1;
}
