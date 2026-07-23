#include "csb_v1_f0279_f0283_champion_altar_raw_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int assertions;

static void check(int condition, const char *expression, int line)
{
    ++assertions;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL:%d: %s\n", line, expression);
    }
}

#define CHECK(condition) check((condition), #condition, __LINE__)

static CSB_V1_ChampionRawMaterialPc34 raw_material(uint8_t *bytes, size_t count)
{
    CSB_V1_ChampionRawMaterialPc34 raw;
    raw.bytes = bytes;
    raw.byte_count = count;
    raw.raw_record_identity = 0x27902830u;
    raw.authenticated_pc34 = 1;
    return raw;
}

static void test_f0279_reads_authenticated_ap_values(void)
{
    uint8_t bytes[] = {'B', 'C', 'D', 'E'};
    uint8_t before[sizeof(bytes)];
    CSB_V1_ChampionRawMaterialPc34 raw = raw_material(bytes, sizeof(bytes));
    CSB_V1_F0279DecodedValueReceiptPc34 receipt;

    memcpy(before, bytes, sizeof(bytes));
    CHECK(csb_v1_f0279_decode_raw_value_pc34(&raw, 0, 4, &receipt) == 1);
    CHECK(receipt.admitted && receipt.decoded_value == 0x1234u);
    CHECK(receipt.character_count == 4);
    CHECK(memcmp(bytes, before, sizeof(bytes)) == 0);
    bytes[2] = 'Q';
    CHECK(csb_v1_f0279_decode_raw_value_pc34(&raw, 0, 4, &receipt) == 0);
    raw.authenticated_pc34 = 0;
    CHECK(csb_v1_f0279_decode_raw_value_pc34(&raw, 0, 2, &receipt) == 0);
}

static void test_f0280_raw_fields_do_not_append_a_party(void)
{
    uint8_t bytes[CSB_V1_F0280_ENCODED_VALUE_BYTES];
    uint8_t before[sizeof(bytes)];
    CSB_V1_ChampionRawMaterialPc34 raw;
    CSB_V1_F0280CandidateRawReceiptPc34 receipt;

    memset(bytes, 'A', sizeof(bytes));
    memcpy(before, bytes, sizeof(bytes));
    raw = raw_material(bytes, sizeof(bytes));
    CHECK(csb_v1_f0280_candidate_raw_receipt_pc34(&raw, 12, 0, &receipt) == 1);
    CHECK(receipt.admitted && receipt.party_append_blocked);
    CHECK(receipt.decoded_value_field_count == 10);
    CHECK(memcmp(bytes, before, sizeof(bytes)) == 0);
    CHECK(csb_v1_f0280_candidate_raw_receipt_pc34(&raw, 24, 0, &receipt) == 0);
}

static void test_f0281_to_f0283_are_explicitly_blocked(void)
{
    uint8_t bytes[] = {0xaa};
    uint8_t before[sizeof(bytes)];
    CSB_V1_ChampionRawMaterialPc34 raw = raw_material(bytes, sizeof(bytes));
    CSB_V1_F0281F0283MutationAuditPc34 audit;

    memcpy(before, bytes, sizeof(bytes));
    CHECK(csb_v1_f0281_f0283_mutation_audit_pc34(
        &raw, CSB_V1_F0282_COMMAND_REINCARNATE, &audit) == 1);
    CHECK(audit.raw_material_admitted && audit.known_command);
    CHECK(audit.rename_ui_blocked && audit.panel_mutation_blocked);
    CHECK(audit.altar_mutation_blocked);
    CHECK(memcmp(bytes, before, sizeof(bytes)) == 0);
    CHECK(csb_v1_f0281_f0283_mutation_audit_pc34(&raw, 999, &audit) == 1);
    CHECK(!audit.known_command && audit.panel_mutation_blocked);
    raw.raw_record_identity = 0;
    CHECK(csb_v1_f0281_f0283_mutation_audit_pc34(&raw, 160, &audit) == 0);
}

int main(void)
{
    test_f0279_reads_authenticated_ap_values();
    test_f0280_raw_fields_do_not_append_a_party();
    test_f0281_to_f0283_are_explicitly_blocked();
    printf("csb_v1_f0279_f0283_champion_altar_raw: %d/%d assertions passed\n",
           assertions - failures, assertions);
    return failures == 0 ? 0 : 1;
}
