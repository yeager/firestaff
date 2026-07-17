/* CSBWin compact-ParameterB saved-timer DSA regression.
 * Source: data.cpp DB3::MakeBig/ParameterB:1319-1351; DSA.cpp
 * GetState:548-571; SaveGame.cpp:1844-1858; CSBCode.cpp:6430-6470. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void put_le16(uint8_t *bytes, size_t offset, uint16_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
}

static void put_le32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
    bytes[offset + 2u] = (uint8_t)(value >> 16);
    bytes[offset + 3u] = (uint8_t)(value >> 24);
}

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

#define LOCALSTATE1_DSA_OFFSET CSB_V1_CSBWIN_EXTENDED_FEATURES_BYTES
#define LOCALSTATE1_DSA_STATE_OFFSET (LOCALSTATE1_DSA_OFFSET + 84u)
#define LOCALSTATE1_DSA_CHECKSUM_OFFSET (LOCALSTATE1_DSA_OFFSET + 130u)
#define LOCALSTATE1_TAIL_BYTES (LOCALSTATE1_DSA_CHECKSUM_OFFSET + 4u)

static uint32_t rcs_checksum(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 0xffffu;
    size_t i;

    for (i = 0u; i < size; ++i) hash = hash * 0xbb40e62du + 11u + bytes[i];
    return hash;
}

static uint32_t form_checksum(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 0u;
    size_t i;

    for (i = 0u; i < size; ++i) hash = hash * 0xbb40e62du + 11u + bytes[i];
    return hash;
}

static void make_localstate_tail(uint8_t bytes[LOCALSTATE1_TAIL_BYTES],
                                 uint32_t local_state)
{
    uint8_t header[CSB_V1_CSBWIN_EXTENDED_FEATURES_BYTES];

    memset(bytes, 0, LOCALSTATE1_TAIL_BYTES);
    memcpy(bytes, " Extended Features ", sizeof(" Extended Features "));
    put_le16(bytes, 38u, 1u);
    memcpy(header, bytes, sizeof(header));
    memset(header + 32u, 0, 4u);
    put_le32(bytes, 32u, form_checksum(header, sizeof(header)));
    put_le32(bytes, LOCALSTATE1_DSA_OFFSET, 7u);
    put_le32(bytes, LOCALSTATE1_DSA_STATE_OFFSET, 1u);
    put_le32(bytes, LOCALSTATE1_DSA_OFFSET + 88u, local_state);
    put_le32(bytes, LOCALSTATE1_DSA_OFFSET + 92u, 0u);
    put_le32(bytes, LOCALSTATE1_DSA_OFFSET + 96u, 4u);
    put_le32(bytes, LOCALSTATE1_DSA_OFFSET + 104u, 1u);
    put_le32(bytes, LOCALSTATE1_DSA_OFFSET + 108u, 1u);
    put_le32(bytes, LOCALSTATE1_DSA_OFFSET + 112u, 1u);
    put_le32(bytes, LOCALSTATE1_DSA_OFFSET + 116u, 0u);
    put_le32(bytes, LOCALSTATE1_DSA_OFFSET + 120u, 3u);
    put_le16(bytes, LOCALSTATE1_DSA_OFFSET + 124u, 0x0686u);
    put_le16(bytes, LOCALSTATE1_DSA_OFFSET + 126u, 2u);
    put_le16(bytes, LOCALSTATE1_DSA_OFFSET + 128u, 0x068bu);
    put_le32(bytes, LOCALSTATE1_DSA_CHECKSUM_OFFSET,
             rcs_checksum(bytes + LOCALSTATE1_DSA_OFFSET,
                          LOCALSTATE1_DSA_CHECKSUM_OFFSET -
                              LOCALSTATE1_DSA_OFFSET));
}

static void make_localstate1_tail(uint8_t bytes[LOCALSTATE1_TAIL_BYTES])
{
    make_localstate_tail(bytes, 1u);
}

static void test_localstate0_queued_db3_receipt(void)
{
    uint8_t tail[LOCALSTATE1_TAIL_BYTES];
    uint8_t raw[16] = { 0 };
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_DSAFilterLocation location;
    CSB_V1_CSBWin512TimerSummary timer;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;

    make_localstate_tail(tail, 0u);
    memset(&profile, 0, sizeof(profile));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&location, 0, sizeof(location));
    memset(&timer, 0, sizeof(timer));
    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    check(csb_v1_chaos_import_extended_save_dsas(
              &profile.csbwin_extended_dsa_state,
              profile.csbwin_appended_tail, sizeof(tail)) == 1,
          "LocalState=0 queued fixture imports its source DSA save stream");

    /* DB3 word2: source type 47, selector two, and DSAstate one. */
    raw[10u] = 0x2fu;
    raw[11u] = 0x11u;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 2;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 8;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    location.actuator_thing = (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_extended_level_index_present = 1;
    profile.csbwin_extended_level_dsa_index[0][2] = 7u;
    timer.valid = 1;
    timer.source_index = 0u;
    timer.function = 6u;
    timer.time = 456u;
    profile.csbwin_timer_summary_count = 1u;
    profile.csbwin_timer_summary_total = 1u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    profile.csbwin_max_timers = 1u;
    profile.csbwin_num_timer = 1u;
    profile.csbwin_first_avail_timer = 1u;
    profile.csbwin_timers[0] = timer;
    profile.csbwin_timer_queue[0] = 0u;

    memset(&receipt, 0, sizeof(receipt));
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 1 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.saved_timer_scope_valid &&
              receipt.saved_dsa_state_transition_valid &&
              receipt.saved_dsa_state_storage_kind ==
                  CSB_V1_CSBWIN_DSA_STATE_STORAGE_DB3_DSASTATE &&
              receipt.saved_timer_queue_slot == 0u &&
              receipt.saved_timer_index == 0u &&
              receipt.saved_timer_time == 456u &&
              receipt.saved_dsa_state_before == 1u &&
              receipt.saved_dsa_state_after == 2u &&
              receipt.saved_dsa_state_tail_fnv1a ==
                  profile.csbwin_appended_tail_fnv1a &&
              raw[10u] == 0x2fu && raw[11u] == 0x21u,
          "queued LocalState=0 writes only authenticated DB3 DSAstate with TIMER receipt");

    profile.csbwin_appended_tail[LOCALSTATE1_DSA_CHECKSUM_OFFSET] ^= 1u;
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 0 && raw[11u] == 0x21u,
          "corrupt LocalState=0 RCS rejects before DB3 or queued receipt publication");
    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);
}

static void test_localstate2_queued_parameterb_receipt(void)
{
    uint8_t tail[LOCALSTATE1_TAIL_BYTES];
    uint8_t raw[16] = { 0 };
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_DSAFilterLocation location;
    CSB_V1_CSBWin512TimerSummary timer;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;

    make_localstate_tail(tail, 2u);
    memset(&profile, 0, sizeof(profile));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&location, 0, sizeof(location));
    memset(&timer, 0, sizeof(timer));
    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    check(csb_v1_chaos_import_extended_save_dsas(
              &profile.csbwin_extended_dsa_state,
              profile.csbwin_appended_tail, sizeof(tail)) == 1,
          "LocalState=2 queued fixture imports its source DSA save stream");

    raw[10u] = 0x2fu;
    raw[11u] = 0x01u;
    raw[14u] = 1u;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 2;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 8;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    location.actuator_thing = (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_extended_level_index_present = 1;
    profile.csbwin_extended_level_dsa_index[0][2] = 7u;
    timer.valid = 1;
    timer.source_index = 0u;
    timer.function = 6u;
    timer.time = 789u;
    profile.csbwin_timer_summary_count = 1u;
    profile.csbwin_timer_summary_total = 1u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    profile.csbwin_max_timers = 1u;
    profile.csbwin_num_timer = 1u;
    profile.csbwin_first_avail_timer = 1u;
    profile.csbwin_timers[0] = timer;
    profile.csbwin_timer_queue[0] = 0u;

    memset(&receipt, 0, sizeof(receipt));
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 1 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.saved_timer_scope_valid &&
              receipt.saved_dsa_state_transition_valid &&
              receipt.saved_dsa_state_storage_kind ==
                  CSB_V1_CSBWIN_DSA_STATE_STORAGE_DB3_PARAMETER_B &&
              receipt.saved_timer_queue_slot == 0u &&
              receipt.saved_timer_index == 0u &&
              receipt.saved_timer_time == 789u &&
              receipt.saved_dsa_state_before == 1u &&
              receipt.saved_dsa_state_after == 2u &&
              receipt.saved_dsa_state_tail_fnv1a ==
                  profile.csbwin_appended_tail_fnv1a &&
              raw[14u] == 2u && raw[15u] == 0u,
          "queued LocalState=2 writes compact ParameterB with exact TIMER receipt");

    profile.csbwin_appended_tail[LOCALSTATE1_DSA_CHECKSUM_OFFSET] ^= 1u;
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 0 && raw[14u] == 2u,
          "corrupt LocalState=2 RCS rejects before ParameterB or receipt publication");
    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);
}

static void test_localstate1_queued_save_receipt(void)
{
    uint8_t tail[LOCALSTATE1_TAIL_BYTES];
    uint8_t raw[16] = { 0 };
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_DSAFilterLocation location;
    CSB_V1_CSBWin512TimerSummary timer;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;

    make_localstate1_tail(tail);
    memset(&profile, 0, sizeof(profile));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&location, 0, sizeof(location));
    memset(&timer, 0, sizeof(timer));
    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    check(csb_v1_chaos_import_extended_save_dsas(
              &profile.csbwin_extended_dsa_state,
              profile.csbwin_appended_tail, sizeof(tail)) == 1,
          "LocalState=1 queued fixture imports its source DSA save stream");

    /* Compact DB3 type-47 record: selector two maps to DSA seven. */
    raw[10u] = 0x2fu;
    raw[11u] = 0x01u;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 2;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 8;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    location.actuator_thing = (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);
    profile.dungeon_handle = &dungeon;
    profile.csbwin_extended_level_index_present = 1;
    profile.csbwin_extended_level_dsa_index[0][2] = 7u;
    timer.valid = 1;
    timer.source_index = 0u;
    timer.function = 6u;
    timer.time = 123u;
    profile.csbwin_timer_summary_count = 1u;
    profile.csbwin_timer_summary_total = 1u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    profile.csbwin_max_timers = 1u;
    profile.csbwin_num_timer = 1u;
    profile.csbwin_first_avail_timer = 1u;
    profile.csbwin_timers[0] = timer;
    profile.csbwin_timer_queue[0] = 0u;

    memset(&receipt, 0, sizeof(receipt));
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 1 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.saved_timer_scope_valid &&
              receipt.saved_dsa_state_transition_valid &&
              receipt.saved_timer_queue_slot == 0u &&
              receipt.saved_timer_index == 0u &&
              receipt.saved_timer_time == 123u &&
              receipt.saved_dsa_state_before == 1u &&
              receipt.saved_dsa_state_after == 2u &&
              receipt.saved_dsa_state_tail_fnv1a ==
                  profile.csbwin_appended_tail_fnv1a &&
              profile.csbwin_extended_dsa_state.imported_headers[7]
                  .persistent_state == 2u &&
              profile.csbwin_appended_tail[LOCALSTATE1_DSA_STATE_OFFSET] == 2u,
          "queued LocalState=1 state publication keeps exact TIMER and save receipts");

    profile.csbwin_appended_tail[LOCALSTATE1_DSA_CHECKSUM_OFFSET] ^= 1u;
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 0 &&
              profile.csbwin_extended_dsa_state.imported_headers[7]
                  .persistent_state == 2u,
          "corrupt LocalState=1 save RCS rejects before a new queued receipt");
    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);
}

static void test_experience_plus_runtime_bridge(void)
{
    uint16_t accepted_program[] = {
        0x0686u, 0u, 0x0686u, 7u, 0x0686u, 200u, 0x1c4bu
    };
    uint16_t level_up_program[] = {
        0x0686u, 0u, 0x0686u, 7u, 0x0686u, 1000u, 0x1c4bu
    };
    uint16_t mastery_program[] = {
        0x0686u, 4u, 0x0686u, 7u, 0x0686u, 3u, 0x0c4bu, 0x000du
    };
    uint16_t possession_mastery_program[] = {
        0x0686u, 4u, 0x0686u, 7u, 0x0686u, 0u, 0x0c4bu, 0x000du
    };
    uint16_t party_fetch_program[] = {
        0x0686u, 0u, 0x0686u, 12u, 0x100bu,
        0x0686u, 12u, 0x0686u, 0u, 0x0a4bu
    };
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    CSB_V1_RuntimeDSAFilterBinding binding;
    CSB_V1_RuntimePartyMirrorReceipt_PC34 mirror;
    CSB_V1_CSBWin512BodyReport exported;
    int parameters[1] = { 0 };

    memset(&action, 0, sizeof(action));
    action.dsa_id = 11u;
    action.state_index = 3u;
    action.program_words = accepted_program;
    action.program_word_count = (int)(sizeof(accepted_program) /
                                      sizeof(accepted_program[0]));
    memset(&binding, 0, sizeof(binding));
    binding.dsa_id = action.dsa_id;

    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 1;
    profile.party_state.Champions[0].CurrentHealth = 100;
    profile.party_state.Champions[0].SkillExperienceValid = 1u;
    profile.party_state.Champions[0].SkillExperience[0] = 600u;
    profile.party_state.Champions[0].SkillExperience[7] = 600u;
    profile.leader_index = 0;
    profile.party_state.LeaderIndex = 0;
    profile.champion_count = 1;

    check(csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
              &profile, &binding, action.state_index, 0, 0u, &runner) == 1 &&
              csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
                  &profile, &runner, &action, parameters, 1, NULL) == 1 &&
              profile.party_state.Champions[0].SkillExperience[7] == 800u &&
              profile.party_state.Champions[0].SkillExperience[0] == 800u,
          "DSA ExperiencePlus commits source AddToSkill XP to selected and basic skills");

    memset(&mirror, 0, sizeof(mirror));
    memset(&exported, 0, sizeof(exported));
    check(csb_v1_runtime_party_mirror_receipt_from_profile_pc34(
              &profile, &mirror) == 1 && mirror.valid == 1 &&
              mirror.party.champions[0].skillLevels[0] == 2u &&
              csb_v1_runtime_get_champion_skill_level(&profile, 0, 7) == 2,
          "DSA ExperiencePlus reaches M11 base-skill and hidden-skill runtime mastery");
    check(csb_v1_runtime_export_csbwin_champion_summaries(
              &profile, &exported) == 1 && exported.champions[0].valid == 1 &&
              exported.champions[0].skill_experience[0] == 800u &&
              exported.champions[0].skill_experience[7] == 800u,
          "DSA ExperiencePlus reaches the CSBWin CHARDESC save-summary owner");

    profile.party_state.Champions[0].SkillTemporaryExperience[0] = 1000;
    profile.party_state.Champions[0].SkillTemporaryExperience[7] = 1000;
    action.program_words = mastery_program;
    action.program_word_count = (int)(sizeof(mastery_program) /
                                      sizeof(mastery_program[0]));
    parameters[0] = -1;
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, parameters, 1, NULL) == 1 &&
              parameters[0] == 2,
          "DSA Mastery uses HandChar and ignores temporary XP when source flags request it");

    profile.csbwin_party_sleeping = 1;
    parameters[0] = -1;
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, parameters, 1, NULL) == 1 &&
              parameters[0] == 1,
          "DSA Mastery returns the source sleeping mastery before skill lookup");
    profile.csbwin_party_sleeping = 0;

    action.program_words = possession_mastery_program;
    action.program_word_count = (int)(sizeof(possession_mastery_program) /
                                      sizeof(possession_mastery_program[0]));
    parameters[0] = -1;
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, parameters, 1, NULL) == 0 &&
              parameters[0] == -1,
          "DSA Mastery rejects unowned possession bonuses without a substitute item map");

    profile.csbwin_gameblock2_summary_valid = 1;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.current_level = 5;
    profile.party_x = 10;
    profile.party_y = 12;
    profile.party_dir = 3;
    profile.csbwin_party_sleeping = 1;
    profile.csbwin_character_tail_see_thru_walls = 1u;
    profile.csbwin_character_tail_magic_footprints_active = 2u;
    profile.csbwin_character_tail_invisible = 3u;
    profile.csbwin_character_tail_fire_shield = -17;
    profile.csbwin_character_tail_spell_shield = 19;
    action.program_words = party_fetch_program;
    action.program_word_count = (int)(sizeof(party_fetch_program) /
                                      sizeof(party_fetch_program[0]));
    {
        int party_values[12] = { 0 };

        check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
                  &profile, &runner, &action, party_values, 12, NULL) == 1 &&
                  party_values[0] == 1 && party_values[1] == 5 &&
                  party_values[2] == 10 && party_values[3] == 12 &&
                  party_values[4] == 3 && party_values[5] == 1 &&
                  party_values[6] == 1 && party_values[7] == 2 &&
                  party_values[8] == 0 && party_values[9] == 3 &&
                  party_values[10] == -17 && party_values[11] == 19,
              "DSA PartyFetch consumes the complete verified CSBWin party image");
        profile.csbwin_body_runtime_summary_valid = 0;
        party_values[0] = -1;
        check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
                  &profile, &runner, &action, party_values, 12, NULL) == 0 &&
                  party_values[0] == -1,
              "DSA PartyFetch rejects without its verified character-tail owner");
        profile.csbwin_body_runtime_summary_valid = 1;
    }

    action.program_words = level_up_program;
    action.program_word_count = (int)(sizeof(level_up_program) /
                                      sizeof(level_up_program[0]));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, parameters, 1, NULL) == 0 &&
              profile.party_state.Champions[0].SkillExperience[7] == 800u &&
              profile.party_state.Champions[0].SkillExperience[0] == 800u,
          "DSA ExperiencePlus rejects unimplemented LevelUp atomically");

    profile.csbwin_extended_dsa_state.imported_actions = NULL;
    profile.csbwin_extended_dsa_state.imported_action_count = 0;
    csb_v1_runtime_cleanup(&profile);
}

int main(void)
{
    uint8_t raw[16] = { 0 };
    uint8_t tail[CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES] = { 0 };
    uint16_t store_global[] = { 0x0686u, 0x55aau, 0x0054u };
    uint16_t modify_message[] = {
        0x0686u, 2u, 0x0686u, 9u, 0x0686u, 7u, 0x0295u
    };
    const uint32_t global_record_id = (5u << 24) | (4u << 16);
    const uint32_t global_bucket = 32u +
        ((global_record_id * 0xbb40e62du) >> 27);
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DSAFilterLocation location;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWin512TimerSummary timer;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;
    uint32_t before;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&profile, 0, sizeof(profile));
    memset(&location, 0, sizeof(location));
    memset(&action, 0, sizeof(action));
    memset(&timer, 0, sizeof(timer));

    /* Compact DB3: word2 is DSA type/selector/state, word6 is ParameterB.
     * DB3::MakeBig moves word6's high bits before reading its state. */
    raw[10] = 0x2fu; raw[11] = 0x01u;
    raw[14] = 1u;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 2;
    dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 8;
    dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    location.actuator_thing = (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);

    put_le16(tail, 2u, 18u);
    put_le32(tail, (size_t)global_bucket * 4u, 1u);
    put_le32(tail, 1u * 4u, 0u);
    put_le32(tail, 2u * 4u, global_record_id);

    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_extended_level_index_present = 1;
    profile.csbwin_extended_level_dsa_index[0][2] = 7u;
    profile.csbwin_global_variables_valid = 1;
    profile.csbwin_global_variable_count = 16u;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(tail);
    memcpy(profile.csbwin_appended_tail, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(tail, sizeof(tail));
    action.dsa_id = 7u;
    action.state_index = 1u;
    action.column = 0u;
    action.program_words = store_global;
    action.program_word_count = 3;
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    profile.csbwin_extended_dsa_state.imported_headers[7].valid = 1;
    profile.csbwin_extended_dsa_state.imported_headers[7].local_state = 2u;
    profile.csbwin_extended_dsa_state.imported_headers[7].state_slot_count = 2u;
    timer.valid = 1;
    timer.source_index = 0u;
    timer.function = 6u;
    profile.csbwin_timer_summary_count = 1u;
    profile.csbwin_timer_summary_total = 1u;
    profile.csbwin_timer_queue_summary_count = 1u;
    profile.csbwin_timer_queue_summary_total = 1u;
    profile.csbwin_max_timers = 1u;
    profile.csbwin_num_timer = 1u;
    profile.csbwin_first_avail_timer = 1u;
    profile.csbwin_timers[0] = timer;
    profile.csbwin_timer_queue[0] = 0u;

    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "queued TT_STONEROOM executes the compact ParameterB-selected action");

    profile.csbwin_extended_dsa_state.imported_headers[7].valid = 0;
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "missing authenticated DSA header rejects before queued dispatch");
    profile.csbwin_extended_dsa_state.imported_headers[7].valid = 1;

    profile.csbwin_appended_tail_fnv1a ^= 1u;
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              profile.csbwin_global_variables[1] == 0x55aau,
          "stale Extended Features tail rejects before queued dispatch");
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);

    before = profile.csbwin_global_variables[1];
    raw[15] = 0x80u;
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 1 &&
              profile.csbwin_global_variables[1] == before,
          "DB3 MakeBig masks raw word6 high bits before ParameterB dispatch");
    raw[15] = 0u;

    profile.csbwin_timer_queue[0] = 1u;
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              profile.csbwin_global_variables[1] == before,
          "out-of-range saved timer-queue entries reject cleanly");
    profile.csbwin_timer_queue[0] = 0u;

    profile.csbwin_timers[0].function = 101u;
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              profile.csbwin_global_variables[1] == before,
          "parameter-message timers stay on their authenticated payload route");

    /* The native ProcessTimers route owns the default map and the saved
     * queue/TIMER identity.  The STKOP effect is transient, but its receipt
     * must not outlive this exact restored dispatch. */
    profile.csbwin_timers[0].function = 6u;
    action.program_words = modify_message;
    action.program_word_count = (int)(sizeof(modify_message) /
                                      sizeof(modify_message[0]));
    memset(&receipt, 0, sizeof(receipt));
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 1 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.timer_type_modifiers_valid &&
              receipt.timer_type_modifiers[0] == 3u &&
              receipt.timer_type_modifiers[1] == 3u &&
              receipt.timer_type_modifiers[2] == 2u &&
              receipt.saved_timer_scope_valid &&
              receipt.saved_timer_queue_slot == 0u &&
              receipt.saved_timer_index == 0u &&
              receipt.saved_timer_function == 6u &&
              receipt.saved_timer_action == 0u &&
              receipt.saved_timer_position == 0u &&
              receipt.saved_timer_time == timer.time,
          "ModifyMessage binds its source map to the authenticated saved TIMER receipt");

    profile.csbwin_timer_queue[0] = 1u;
    check(csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
              &profile, &dungeon, &location, 0u) == 0 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 0,
          "stale saved timer queue clears the transient ModifyMessage receipt");
    profile.csbwin_timer_queue[0] = 0u;
    test_experience_plus_runtime_bridge();
    test_localstate0_queued_db3_receipt();
    test_localstate2_queued_parameterb_receipt();
    test_localstate1_queued_save_receipt();
    return failures == 0 ? 0 : 1;
}
