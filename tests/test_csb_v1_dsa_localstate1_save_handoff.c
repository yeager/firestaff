/* CSBWin LocalState=1 DSA save/runtime regression.
 * Source: CSBWin DSA.cpp GetState/PutState:548-572, ProcessDSATimer6:
 * 5315-5465, STKOP_DisableSaves:2946-2955; SaveGame.cpp
 * ReadDSAs/WriteDSAs:211-241,775-790, save policy:856-873; data.cpp
 * RCS(ui8 *, i32):1818-1827. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define EXTENDED_HEADER_BYTES CSB_V1_CSBWIN_EXTENDED_FEATURES_BYTES
#define DSA_OFFSET EXTENDED_HEADER_BYTES
#define DSA_STATE_OFFSET (DSA_OFFSET + 84u)
#define DSA_CHECKSUM_OFFSET (DSA_OFFSET + 130u)
#define SAVE_BYTES (DSA_CHECKSUM_OFFSET + 4u)
#define TRANSFER_DSA_CHECKSUM_OFFSET (DSA_OFFSET + 126u)
#define TRANSFER_SAVE_BYTES (TRANSFER_DSA_CHECKSUM_OFFSET + 4u)

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

static uint32_t form_checksum(const uint8_t *bytes, size_t size)
{
    uint32_t result = 0u;
    size_t i;
    for (i = 0u; i < size; ++i) result = result * 0xbb40e62du + 11u + bytes[i];
    return result;
}

static uint32_t rcs_checksum(const uint8_t *bytes, size_t size)
{
    uint32_t result = 0xffffu;
    size_t i;
    for (i = 0u; i < size; ++i) result = result * 0xbb40e62du + 11u + bytes[i];
    return result;
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

static void make_real_shape(uint8_t bytes[SAVE_BYTES])
{
    uint8_t header[EXTENDED_HEADER_BYTES];

    memset(bytes, 0, SAVE_BYTES);
    memcpy(bytes, " Extended Features ", sizeof(" Extended Features "));
    put_le16(bytes, 38u, 1u);
    memcpy(header, bytes, sizeof(header));
    memset(header + 32u, 0, 4u);
    put_le32(bytes, 32u, form_checksum(header, sizeof(header)));

    put_le32(bytes, DSA_OFFSET, 7u);
    put_le32(bytes, DSA_STATE_OFFSET, 1u);
    put_le32(bytes, DSA_OFFSET + 88u, 1u);
    put_le32(bytes, DSA_OFFSET + 92u, 0u);
    put_le32(bytes, DSA_OFFSET + 96u, 4u);
    put_le32(bytes, DSA_OFFSET + 104u, 1u);
    put_le32(bytes, DSA_OFFSET + 108u, 1u);
    put_le32(bytes, DSA_OFFSET + 112u, 1u);
    put_le32(bytes, DSA_OFFSET + 116u, 0u);
    put_le32(bytes, DSA_OFFSET + 120u, 3u);
    put_le16(bytes, DSA_OFFSET + 124u, 0x0686u);
    put_le16(bytes, DSA_OFFSET + 126u, 2u);
    put_le16(bytes, DSA_OFFSET + 128u, 0x068bu);
    put_le32(bytes, DSA_CHECKSUM_OFFSET,
             rcs_checksum(bytes + DSA_OFFSET, DSA_CHECKSUM_OFFSET - DSA_OFFSET));
}

static void make_localstate2_shape(uint8_t bytes[SAVE_BYTES])
{
    make_real_shape(bytes);
    put_le32(bytes, DSA_OFFSET + 88u, 2u);
    put_le32(bytes, DSA_CHECKSUM_OFFSET,
             rcs_checksum(bytes + DSA_OFFSET,
                          DSA_CHECKSUM_OFFSET - DSA_OFFSET));
}

static void make_transfer_shape(uint8_t bytes[TRANSFER_SAVE_BYTES])
{
    uint8_t header[EXTENDED_HEADER_BYTES];

    memset(bytes, 0, TRANSFER_SAVE_BYTES);
    memcpy(bytes, " Extended Features ", sizeof(" Extended Features "));
    put_le16(bytes, 38u, 1u);
    memcpy(header, bytes, sizeof(header));
    memset(header + 32u, 0, 4u);
    put_le32(bytes, 32u, form_checksum(header, sizeof(header)));

    /* One complete DSA::Read JUMP action returns state 2 through PutState. */
    put_le32(bytes, DSA_OFFSET, 7u);
    put_le32(bytes, DSA_STATE_OFFSET, 1u);
    put_le32(bytes, DSA_OFFSET + 88u, 1u);
    put_le32(bytes, DSA_OFFSET + 92u, 0u);
    put_le32(bytes, DSA_OFFSET + 96u, 4u);
    put_le32(bytes, DSA_OFFSET + 104u, 1u);
    put_le32(bytes, DSA_OFFSET + 108u, 1u);
    put_le32(bytes, DSA_OFFSET + 112u, 1u);
    put_le32(bytes, DSA_OFFSET + 116u, 0u);
    put_le32(bytes, DSA_OFFSET + 120u, 1u);
    put_le16(bytes, DSA_OFFSET + 124u, 0x208cu);
    put_le32(bytes, TRANSFER_DSA_CHECKSUM_OFFSET,
             rcs_checksum(bytes + DSA_OFFSET,
                          TRANSFER_DSA_CHECKSUM_OFFSET - DSA_OFFSET));
}

static int initialize_profile(CSB_V1_RuntimeProfile *profile,
                              const uint8_t *bytes, size_t size)
{
    csb_v1_runtime_init(profile, NULL);
    profile->csbwin_extended_features_valid = 1;
    profile->csbwin_appended_tail_valid = 1;
    profile->csbwin_appended_tail_size = size;
    profile->csbwin_appended_tail_preserved_size = size;
    memcpy(profile->csbwin_appended_tail, bytes, size);
    profile->csbwin_appended_tail_fnv1a = fnv1a32(bytes, size);
    if (csb_v1_chaos_import_extended_save_dsas(
            &profile->csbwin_extended_dsa_state,
            profile->csbwin_appended_tail, (int)size) != 1) {
        return 0;
    }
    return 1;
}

int main(void)
{
    uint8_t bytes[SAVE_BYTES];
    uint8_t transfer_bytes[TRANSFER_SAVE_BYTES];
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinExtendedDSAReport report;
    CSB_V1_CSBWinExtendedFeaturesReport features;
    CSB_V1_RuntimeProfile policy_profile;
    CSB_V1_DungeonData localstate2_dungeon;
    uint8_t localstate2_raw[16] = { 0 };
    CSB_V1_CSBWinDSAFilterStackRunnerContext policy_runner;
    CSB_V1_DSAImportedAction policy_action;
    uint16_t policy_disable_words[] = { 0x0686u, 1u, 0x090bu };
    uint16_t policy_enable_words[] = { 0x0686u, 0u, 0x090bu };
    uint16_t policy_bad_words[] = { 0x0686u, 1u, 0x090bu, 0x0000u };
    uint16_t policy_discard_text_words[] = { 0x1ccbu };
    uint16_t policy_discard_text_bad_words[] = { 0x1ccbu, 0x0000u };
    uint32_t before_hash;

    make_real_shape(bytes);
    memset(&profile, 0, sizeof(profile));
    check(initialize_profile(&profile, bytes, sizeof(bytes)) == 1,
          "real-shaped Extended Features DSA receipt imports");
    profile.party_state_valid = 1;
    profile.current_level = 5;
    profile.party_x = 10;
    profile.party_y = 12;
    profile.game_time = 991u;
    profile.party_state.ChampionCount = 2;
    profile.party_state.LeaderIndex = 0;
    profile.leader_index = 0;
    profile.party_state.Champions[0].Talents = 0x3u;
    profile.party_state.Champions[0].Wounds = 0x0003u;
    profile.party_state.Champions[0].CurrentHealth = 30;
    profile.party_state.Champions[1].Talents = 0x4u;
    profile.party_state.Champions[1].Wounds = 0x000cu;
    profile.party_state.Champions[1].CurrentHealth = 0;
    memset(&runner, 0, sizeof(runner));
    {
        CSB_V1_RuntimeDSAFilterBinding binding;
        memset(&binding, 0, sizeof(binding));
        binding.dsa_id = 7u;
        check(csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
                  &profile, &binding, 1u, 0, 0u, &runner) == 1,
              "LocalState=1 master resolves its authenticated action");
        check(runner.party_location_valid && runner.party_level == 5 &&
                  runner.party_x == 10 && runner.party_y == 12,
              "runtime runner preserves the source-owned party location");
        check(runner.game_time_valid && runner.game_time == 991u &&
                  !runner.dsa_slave_thing_valid,
              "runtime runner carries time but rejects an unverified DSA Thing");
        check(runner.party_champions_valid &&
                  runner.party_champion_count == 2 &&
                  runner.party_champion_talents[0] == 0x3u &&
                  runner.party_champion_wounds[1] == 0x000cu &&
                  runner.party_champion_health[1] == 0 &&
                  runner.party_leader_index == 0,
              "runtime runner copies the profile-owned CSBWin party query data");
    }
    action = csb_v1_chaos_find_imported_action(
        &profile.csbwin_extended_dsa_state, 7, 1u, 0);
    check(action != NULL &&
              csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
                  &profile, &runner, action, NULL, 0, NULL) == 1 &&
              profile.csbwin_extended_dsa_state.imported_headers[7]
                  .persistent_state == 2u &&
              profile.csbwin_appended_tail[DSA_STATE_OFFSET] == 2u &&
              profile.csbwin_appended_tail[DSA_STATE_OFFSET + 1u] == 0u,
          "PutState commits forced LocalState=1 state to the real DSA receipt");
    memset(&report, 0, sizeof(report));
    memset(&features, 0, sizeof(features));
    check(csb_v1_csbwin_512_inspect_extended_dsa_section(
              profile.csbwin_appended_tail,
              profile.csbwin_appended_tail_preserved_size, &report,
              &features) == CSB_V1_CSBWIN_EXTENDED_OK && report.valid &&
              report.stored_checksum == report.computed_checksum,
          "PutState recomputes the source RCS checksum before publication");

    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);
    make_real_shape(bytes);
    memset(&profile, 0, sizeof(profile));
    check(initialize_profile(&profile, bytes, sizeof(bytes)) == 1,
          "second real-shaped receipt imports for the negative path");
    before_hash = profile.csbwin_appended_tail_fnv1a;
    profile.csbwin_appended_tail[DSA_CHECKSUM_OFFSET] ^= 1u;
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail, profile.csbwin_appended_tail_size);
    memset(&runner, 0, sizeof(runner));
    {
        CSB_V1_RuntimeDSAFilterBinding binding;
        memset(&binding, 0, sizeof(binding));
        binding.dsa_id = 7u;
        check(csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
                  &profile, &binding, 1u, 0, 0u, &runner) == 1,
              "negative-path LocalState=1 action resolves before execution");
    }
    action = csb_v1_chaos_find_imported_action(
        &profile.csbwin_extended_dsa_state, 7, 1u, 0);
    check(action != NULL &&
              csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
                  &profile, &runner, action, NULL, 0, NULL) == 0 &&
              profile.csbwin_extended_dsa_state.imported_headers[7]
                  .persistent_state == 1u &&
              profile.csbwin_appended_tail_fnv1a != before_hash,
          "bad DSA RCS fails closed without publishing a new master state");
    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);

    make_localstate2_shape(bytes);
    memset(&profile, 0, sizeof(profile));
    memset(&localstate2_dungeon, 0, sizeof(localstate2_dungeon));
    check(initialize_profile(&profile, bytes, sizeof(bytes)) == 1,
          "real-shaped LocalState=2 receipt imports");
    /* Original compact DB3 type-47: ParameterB/word6 starts at state 1. */
    localstate2_raw[10] = 0x2fu;
    localstate2_raw[11] = 0x01u;
    localstate2_raw[14] = 1u;
    localstate2_dungeon.raw_data = localstate2_raw;
    localstate2_dungeon.raw_size = (int)sizeof(localstate2_raw);
    localstate2_dungeon.thing_data_bases[CSB_V1_THING_TYPE_ACTUATOR] = 8;
    localstate2_dungeon.thing_type_counts[CSB_V1_THING_TYPE_ACTUATOR] = 1;
    profile.dungeon_handle = &localstate2_dungeon;
    memset(&runner, 0, sizeof(runner));
    {
        CSB_V1_RuntimeDSAFilterBinding binding;
        memset(&binding, 0, sizeof(binding));
        binding.dsa_id = 7u;
        binding.actuator_identity_valid = 1;
        binding.location.actuator_thing =
            (uint16_t)(CSB_V1_THING_TYPE_ACTUATOR << 10);
        check(csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
                  &profile, &binding, 1u, 0, 0u, &runner) == 1,
              "LocalState=2 runner owns its exact compact DB3");
    }
    action = csb_v1_chaos_find_imported_action(
        &profile.csbwin_extended_dsa_state, 7, 1u, 0);
    check(action != NULL &&
              csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
                  &profile, &runner, action, NULL, 0, NULL) == 1 &&
              localstate2_raw[14] == 2u && localstate2_raw[15] == 0u,
          "PutState commits compact LocalState=2 through original DB3 bytes");
    localstate2_raw[15] = 0x40u;
    check(action != NULL &&
              csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
                  &profile, &runner, action, NULL, 0, NULL) == 0 &&
              localstate2_raw[14] == 2u && localstate2_raw[15] == 0x40u,
          "widened LocalState=2 DB3 state remains fail-closed");
    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);

    make_transfer_shape(transfer_bytes);
    memset(&profile, 0, sizeof(profile));
    check(initialize_profile(&profile, transfer_bytes, sizeof(transfer_bytes)) == 1,
          "real-shaped LocalState=1 transfer receipt imports");
    memset(&runner, 0, sizeof(runner));
    {
        CSB_V1_RuntimeDSAFilterBinding binding;
        memset(&binding, 0, sizeof(binding));
        binding.dsa_id = 7u;
        check(csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
                  &profile, &binding, 1u, 0, 0u, &runner) == 1,
              "LocalState=1 transfer action resolves from the saved receipt");
    }
    action = csb_v1_chaos_find_imported_action(
        &profile.csbwin_extended_dsa_state, 7, 1u, 0);
    check(action != NULL &&
              csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
                  &profile, &runner, action, NULL, 0, NULL) == 1 &&
              runner.state_index == 2u &&
              profile.csbwin_extended_dsa_state.imported_headers[7]
                  .persistent_state == 2u &&
              profile.csbwin_appended_tail[DSA_STATE_OFFSET] == 2u &&
              profile.csbwin_appended_tail[DSA_STATE_OFFSET + 1u] == 0u,
          "JUMP transfer commits its source final state through PutState");
    memset(&report, 0, sizeof(report));
    memset(&features, 0, sizeof(features));
    check(csb_v1_csbwin_512_inspect_extended_dsa_section(
              profile.csbwin_appended_tail,
              profile.csbwin_appended_tail_preserved_size, &report,
              &features) == CSB_V1_CSBWIN_EXTENDED_OK && report.valid &&
              report.stored_checksum == report.computed_checksum,
              "JUMP PutState recomputes its complete DSA receipt checksum");
    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);

    /* The profile runner stages SaveGame's DSA policy state separately from
     * the synthetic test action; production still requires imported action
     * pointer identity before it can reach this bridge. */
    csb_v1_runtime_init(&policy_profile, NULL);
    memset(&policy_runner, 0, sizeof(policy_runner));
    memset(&policy_action, 0, sizeof(policy_action));
    policy_action.dsa_id = 11u;
    policy_action.state_index = 1u;
    policy_action.column = 0u;
    policy_action.program_words = policy_disable_words;
    policy_action.program_word_count = (int)(sizeof(policy_disable_words) /
                                             sizeof(policy_disable_words[0]));
    policy_profile.csbwin_extended_features_valid = 1;
    policy_profile.csbwin_extended_dsa_state.imported_actions = &policy_action;
    policy_profile.csbwin_extended_dsa_state.imported_action_count = 1;
    policy_runner.programs = &policy_profile.csbwin_extended_dsa_state;
    policy_runner.dsa_id = 11;
    policy_runner.state_index = 1u;
    policy_runner.action_ordinal = 0;
    policy_runner.saves_disabled_valid = 1;
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &policy_profile, &policy_runner, &policy_action,
              NULL, 0, NULL) == 1 &&
              policy_profile.csbwin_saves_disabled == 1 &&
              policy_runner.saves_disabled == 1,
          "DSA save-policy action reaches the live CSBWin save gate");
    policy_action.program_words = policy_enable_words;
    policy_action.program_word_count = (int)(sizeof(policy_enable_words) /
                                             sizeof(policy_enable_words[0]));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &policy_profile, &policy_runner, &policy_action,
              NULL, 0, NULL) == 1 &&
              policy_profile.csbwin_saves_disabled == 0 &&
              policy_runner.saves_disabled == 0,
          "DSA save-policy zero reopens the live CSBWin save gate");
    policy_action.program_words = policy_bad_words;
    policy_action.program_word_count = (int)(sizeof(policy_bad_words) /
                                             sizeof(policy_bad_words[0]));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &policy_profile, &policy_runner, &policy_action,
              NULL, 0, NULL) == 0 &&
              policy_profile.csbwin_saves_disabled == 0 &&
              policy_runner.saves_disabled == 0,
          "rejected DSA bytecode cannot alter the live CSBWin save gate");
    policy_profile.csbwin_text_message_receipt.valid = 1;
    policy_profile.csbwin_text_message_receipt.text_thing = 0x0800u;
    strcpy(policy_profile.csbwin_text_message_receipt.text, "SOURCE TEXT");
    policy_action.program_words = policy_discard_text_words;
    policy_action.program_word_count = (int)(sizeof(policy_discard_text_words) /
                                             sizeof(policy_discard_text_words[0]));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &policy_profile, &policy_runner, &policy_action,
              NULL, 0, NULL) == 1 &&
              !policy_profile.csbwin_text_message_receipt.valid &&
              policy_profile.csbwin_text_message_receipt.text[0] == '\0',
          "authenticated STKOP_DiscardText clears only the source DB2 receipt");
    policy_profile.csbwin_text_message_receipt.valid = 1;
    strcpy(policy_profile.csbwin_text_message_receipt.text, "SOURCE TEXT");
    policy_action.program_words = policy_discard_text_bad_words;
    policy_action.program_word_count = (int)(sizeof(policy_discard_text_bad_words) /
                                             sizeof(policy_discard_text_bad_words[0]));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &policy_profile, &policy_runner, &policy_action,
              NULL, 0, NULL) == 0 &&
              policy_profile.csbwin_text_message_receipt.valid &&
              strcmp(policy_profile.csbwin_text_message_receipt.text,
                     "SOURCE TEXT") == 0,
          "rejected DSA text action preserves the source DB2 receipt");
    policy_profile.csbwin_extended_dsa_state.imported_actions = NULL;
    policy_profile.csbwin_extended_dsa_state.imported_action_count = 0;
    csb_v1_runtime_cleanup(&policy_profile);
    return failures == 0 ? 0 : 1;
}
