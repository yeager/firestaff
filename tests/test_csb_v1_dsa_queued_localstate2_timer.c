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

static void test_experience_plus_runtime_bridge(void)
{
    uint16_t accepted_program[] = {
        0x0686u, 0u, 0x0686u, 7u, 0x0686u, 200u, 0x1c4bu
    };
    uint16_t level_up_program[] = {
        0x0686u, 0u, 0x0686u, 7u, 0x0686u, 1000u, 0x1c4bu
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
    const uint32_t global_record_id = (5u << 24) | (4u << 16);
    const uint32_t global_bucket = 32u +
        ((global_record_id * 0xbb40e62du) >> 27);
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DSAFilterLocation location;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWin512TimerSummary timer;
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
    test_experience_plus_runtime_bridge();
    return failures == 0 ? 0 : 1;
}
