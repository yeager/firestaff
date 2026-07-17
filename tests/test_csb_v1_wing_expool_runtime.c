/* CSBWin Character.cpp CHARDESC::GetFromWings EXPOOL regression.
 * The fixture is the original eight-record EDT_Character shape, not a
 * substitute character layout. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    character_records = 8,
    character_record_bytes = 100,
    character_node_words = 27
};

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

static uint32_t read_le32(const uint8_t *bytes, size_t offset)
{
    return (uint32_t)bytes[offset] |
        ((uint32_t)bytes[offset + 1u] << 8) |
        ((uint32_t)bytes[offset + 2u] << 16) |
        ((uint32_t)bytes[offset + 3u] << 24);
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

static void build_wing_tail(uint8_t *tail, size_t size, uint16_t fingerprint,
                            uint32_t talents)
{
    uint32_t i;

    memset(tail, 0, size);
    for (i = 0u; i < character_records; ++i) {
        const uint32_t record_id = (8u << 24) | (i << 16) | fingerprint;
        const uint32_t bucket = 32u +
            ((record_id * 0xbb40e62du) >> 27);
        const uint32_t block_base = i * 64u;
        const uint32_t node = block_base + 1u;
        const uint32_t prior = read_le32(tail, (size_t)bucket * 4u);
        uint8_t *payload = tail + (size_t)(node + 2u) * 4u;

        put_le16(tail, (size_t)block_base * 4u + 2u,
                 character_node_words);
        put_le32(tail, (size_t)node * 4u, prior);
        put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
        memset(payload, (int)i, character_record_bytes);
        put_le32(tail, (size_t)bucket * 4u, node);
    }
    /* CHARDESC layout offsets from Character.cpp: talents=276,
     * fingerPrint=280. Both live in the third 100-byte record. */
    put_le32(tail, (size_t)(2u * 64u + 3u) * 4u + 76u, talents);
    put_le16(tail, (size_t)(2u * 64u + 3u) * 4u + 80u, fingerprint);
}

static void build_extended_cell_flags_tail(uint8_t *tail, size_t size)
{
    const uint32_t record_id = (2u << 24) | (3u << 5) | 4u;
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);
    const uint32_t node = 1u;
    uint8_t *payload;

    memset(tail, 0, size);
    put_le16(tail, 2u, 10u);
    put_le32(tail, (size_t)node * 4u, 0u);
    put_le32(tail, (size_t)(node + 1u) * 4u, record_id);
    payload = tail + (size_t)(node + 2u) * 4u;
    put_le32(payload, 0u, 1u << 2);
    put_le32(payload, 12u, 1u << 2);
    put_le32(payload, 28u, 1u << 2);
    put_le32(tail, (size_t)bucket * 4u, node);
}

static void prepare_profile(CSB_V1_RuntimeProfile *profile,
                            const uint8_t *tail, size_t size)
{
    csb_v1_runtime_init(profile, NULL);
    memcpy(profile->csbwin_appended_tail, tail, size);
    profile->csbwin_appended_tail_valid = 1;
    profile->csbwin_appended_tail_size = size;
    profile->csbwin_appended_tail_preserved_size = size;
    profile->csbwin_appended_tail_fnv1a = fnv1a32(tail, size);
}

int main(void)
{
    uint8_t tail[character_records * CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 receipt;
    CSB_V1_CSBWinDSACoreProgramReceipt core_receipt;
    uint16_t store_excell_flags[] = {
        0x0686u, 0x52u, 0x0686u, 0x0c82u, 0x0b4bu
    };
    uint16_t store_wing_talents[] = {
        0x0686u, 0x55u, 0x0786u, 0x1234u, 1u, 0x01d5u
    };
    uint16_t store_wing_talents_then_bad[] = {
        0x0686u, 0x66u, 0x0786u, 0x1234u, 1u, 0x01d5u, 0x0000u
    };
    uint32_t talents = 0u;
    uint32_t flags[8];
    uint32_t tail_fnv_before;
    int dsa_run_result;

    build_wing_tail(tail, sizeof(tail), 0x1234u, 0x89abcdefu);
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_read_csbwin_wing_talents(
              &profile, 0x1234u, &talents) == 1 &&
              talents == 0x89abcdefu,
          "CSBWin restores a complete eight-record EDT_Character wing");
    check(csb_v1_runtime_has_csbwin_wing_character(&profile, 0x1234u) == 1,
          "CSBWin WHEREISCHAR finds its source first EDT_Character record");
    check(csb_v1_runtime_set_csbwin_wing_talents(
              &profile, 0x1234u, 0x10203040u) == 1 &&
              csb_v1_runtime_read_csbwin_wing_talents(
                  &profile, 0x1234u, &talents) == 1 &&
              talents == 0x10203040u,
          "CSBWin SaveToWings rewrites all existing character records");
    check(csb_v1_runtime_read_csbwin_wing_talents(
              &profile, 0x9999u, &talents) == 0 && talents == 0u,
          "CSBWin reports an authenticated absent wing as source zero");

    memset(&action, 0, sizeof(action));
    memset(&runner, 0, sizeof(runner));
    memset(&receipt, 0, sizeof(receipt));
    action.dsa_id = 7u;
    action.state_index = 1u;
    action.column = 0u;
    action.program_words = store_wing_talents;
    action.program_word_count = (int)(sizeof(store_wing_talents) /
                                      sizeof(store_wing_talents[0]));
    profile.csbwin_extended_features_valid = 1;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 0;
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = 7;
    runner.state_index = 1u;
    runner.action_ordinal = 0;
    tail_fnv_before = profile.csbwin_appended_tail_fnv1a;
    dsa_run_result = csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
        &profile, &runner, &action, NULL, 0, NULL);
    check(dsa_run_result == 1,
          "CSBWin DSA TALENTS! accepts the verified wing action");
    check(csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
              &profile, &receipt) == 1 &&
              receipt.wing_talents_store_count == 1u &&
              receipt.last_wing_talents_fingerprint == 0x1234u &&
              receipt.last_wing_talents_before == 0x10203040u &&
              receipt.last_wing_talents_after == 0x55u &&
              receipt.wing_talents_tail_fnv1a_before == tail_fnv_before &&
              receipt.wing_talents_tail_fnv1a_after ==
                  profile.csbwin_appended_tail_fnv1a &&
              receipt.wing_talents_tail_fnv1a_before !=
                  receipt.wing_talents_tail_fnv1a_after &&
              csb_v1_runtime_read_csbwin_wing_talents(
                  &profile, 0x1234u, &talents) == 1 && talents == 0x55u,
          "CSBWin DSA TALENTS! commits one authenticated wing bundle receipt");
    action.program_words = store_wing_talents_then_bad;
    action.program_word_count = (int)(sizeof(store_wing_talents_then_bad) /
                                      sizeof(store_wing_talents_then_bad[0]));
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 0,
          "CSBWin DSA TALENTS! rejects a later unknown opcode");
    check(csb_v1_runtime_read_csbwin_wing_talents(
              &profile, 0x1234u, &talents) == 1 && talents == 0x55u,
          "CSBWin DSA TALENTS! rejects a later unknown opcode before wing publication");
    profile.csbwin_appended_tail_fnv1a ^= 1u;
    check(csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
              &profile, &receipt) == 0,
          "CSBWin DSA TALENTS! receipt expires when its wing tail identity drifts");

    prepare_profile(&profile, tail,
                    sizeof(tail) - CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES);
    check(csb_v1_runtime_has_csbwin_wing_character(&profile, 0x1234u) == 1,
          "CSBWin WHEREISCHAR keeps the source first-record lookup");
    check(csb_v1_runtime_read_csbwin_wing_talents(
              &profile, 0x1234u, &talents) == -1 && talents == 0u,
          "CSBWin rejects a partial EDT_Character wing bundle");

    prepare_profile(&profile, tail, sizeof(tail));
    profile.csbwin_appended_tail_fnv1a ^= 1u;
    check(csb_v1_runtime_read_csbwin_wing_talents(
              &profile, 0x1234u, &talents) == -1 && talents == 0u,
          "CSBWin rejects an altered wing EXPOOL receipt");

    build_extended_cell_flags_tail(tail, sizeof(tail));
    prepare_profile(&profile, tail, sizeof(tail));
    check(csb_v1_runtime_read_csbwin_extended_cell_flags(
              &profile, 0x0c82u, flags) == 1 &&
              flags[0] == (1u << 2) && flags[3] == (1u << 2) &&
              flags[7] == (1u << 2),
          "CSBWin reads one real-shaped eight-word EDT_ExtendedCellFlags record");
    memset(&action, 0, sizeof(action));
    memset(&runner, 0, sizeof(runner));
    memset(&receipt, 0, sizeof(receipt));
    action.dsa_id = 7u;
    action.state_index = 1u;
    action.column = 0u;
    action.program_words = store_excell_flags;
    action.program_word_count = (int)(sizeof(store_excell_flags) /
                                      sizeof(store_excell_flags[0]));
    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_extended_dsa_state.imported_actions = &action;
    profile.csbwin_extended_dsa_state.imported_action_count = 1;
    runner.programs = &profile.csbwin_extended_dsa_state;
    runner.dsa_id = 7;
    runner.state_index = 1u;
    runner.action_ordinal = 0;
    tail_fnv_before = profile.csbwin_appended_tail_fnv1a;
    check(csb_v1_csbwin_dsa_verify_authenticated_core_program(
              &profile.csbwin_extended_dsa_state, 7, 1u, 0,
              &core_receipt) == CSB_V1_CSBWIN_DSA_CORE_OK &&
              core_receipt.valid && core_receipt.requires_runtime_owner,
          "CSBWin ECF! program is an admitted runtime-owned source action");
    dsa_run_result = csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
        &profile, &runner, &action, NULL, 0, NULL);
    check(dsa_run_result == 1,
          "CSBWin ECF! runtime accepts the verified source action");
    check(dsa_run_result == 1 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 1 &&
              receipt.excell_store_count == 1u &&
              receipt.last_excell_store_location == 0x0c82u &&
              receipt.excell_tail_fnv1a_before == tail_fnv_before &&
              receipt.excell_tail_fnv1a_after ==
                  profile.csbwin_appended_tail_fnv1a &&
              receipt.excell_tail_fnv1a_before !=
                  receipt.excell_tail_fnv1a_after &&
              csb_v1_runtime_read_csbwin_extended_cell_flags(
                  &profile, 0x0c82u, flags) == 1 &&
              flags[0] == 0u && flags[1] == (1u << 2) &&
              flags[4] == (1u << 2) && flags[6] == (1u << 2),
          "CSBWin DSA ECF! publishes EXPOOL words and exact tail FNV receipt atomically");
    profile.csbwin_appended_tail_fnv1a ^= 1u;
    check(csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
              &profile, &receipt) == 0,
          "CSBWin DSA ECF! receipt expires when its EXPOOL tail identity drifts");
    check(csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
              &profile, &runner, &action, NULL, 0, NULL) == 0 &&
              csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                  &profile, &receipt) == 0,
          "CSBWin DSA ECF! rejects a mutated EXPOOL tail before receipt publication");
    check(csb_v1_runtime_read_csbwin_extended_cell_flags(
              &profile, 0x0c82u, flags) == -1,
          "CSBWin rejects an altered ExtendedCellFlags EXPOOL receipt");

    return failures == 0 ? 0 : 1;
}
