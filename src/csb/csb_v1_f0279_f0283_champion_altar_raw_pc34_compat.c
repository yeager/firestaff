#include "csb_v1_f0279_f0283_champion_altar_raw_pc34_compat.h"

#include <string.h>

static int raw_range_valid(const CSB_V1_ChampionRawMaterialPc34 *raw,
                           size_t offset, size_t count)
{
    return raw && raw->authenticated_pc34 && raw->raw_record_identity != 0 &&
        raw->bytes && offset <= raw->byte_count && count <= raw->byte_count - offset;
}

int csb_v1_f0279_decode_raw_value_pc34(
    const CSB_V1_ChampionRawMaterialPc34 *raw,
    size_t byte_offset,
    uint8_t character_count,
    CSB_V1_F0279DecodedValueReceiptPc34 *out)
{
    CSB_V1_F0279DecodedValueReceiptPc34 receipt;
    uint16_t value = 0;
    uint8_t i;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (character_count == 0 ||
        character_count > CSB_V1_F0279_MAX_ENCODED_CHARACTERS ||
        !raw_range_valid(raw, byte_offset, character_count)) return 0;
    for (i = 0; i < character_count; ++i) {
        uint8_t encoded = raw->bytes[byte_offset + i];
        if (encoded < 'A' || encoded > 'P') return 0;
        value = (uint16_t)((value << 4) + (encoded - 'A'));
    }
    receipt.admitted = 1;
    receipt.decoded_value = value;
    receipt.character_count = character_count;
    receipt.raw_record_identity = raw->raw_record_identity;
    receipt.source_evidence =
        "ReDMCSB REVIVE.C F0279 A..P raw champion-value decode";
    *out = receipt;
    return 1;
}

int csb_v1_f0280_candidate_raw_receipt_pc34(
    const CSB_V1_ChampionRawMaterialPc34 *raw,
    uint8_t portrait_index,
    size_t encoded_values_offset,
    CSB_V1_F0280CandidateRawReceiptPc34 *out)
{
    CSB_V1_F0280CandidateRawReceiptPc34 receipt;
    static const uint8_t widths[] = {4, 4, 4, 2, 2, 2, 2, 2, 2, 2};
    size_t cursor = encoded_values_offset;
    size_t i;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (portrait_index > CSB_V1_F0280_MAX_PORTRAIT_INDEX ||
        !raw_range_valid(raw, encoded_values_offset,
                         CSB_V1_F0280_ENCODED_VALUE_BYTES)) return 0;
    for (i = 0; i < sizeof(widths); ++i) {
        CSB_V1_F0279DecodedValueReceiptPc34 value;
        if (!csb_v1_f0279_decode_raw_value_pc34(raw, cursor, widths[i], &value)) {
            return 0;
        }
        cursor += widths[i];
    }
    receipt.admitted = 1;
    receipt.party_append_blocked = 1;
    receipt.portrait_index = portrait_index;
    receipt.decoded_value_field_count = (uint8_t)sizeof(widths);
    receipt.raw_record_identity = raw->raw_record_identity;
    receipt.source_evidence =
        "ReDMCSB REVIVE.C F0280 raw health/stamina/mana/stat fields; append blocked";
    *out = receipt;
    return 1;
}

int csb_v1_f0281_f0283_mutation_audit_pc34(
    const CSB_V1_ChampionRawMaterialPc34 *raw,
    int command,
    CSB_V1_F0281F0283MutationAuditPc34 *out)
{
    CSB_V1_F0281F0283MutationAuditPc34 audit;

    if (!out) return 0;
    memset(&audit, 0, sizeof(audit));
    *out = audit;
    if (!raw_range_valid(raw, 0, 1)) return 0;
    audit.raw_material_admitted = 1;
    audit.known_command = command == CSB_V1_F0282_COMMAND_RESURRECT ||
        command == CSB_V1_F0282_COMMAND_REINCARNATE ||
        command == CSB_V1_F0282_COMMAND_CANCEL;
    audit.rename_ui_blocked = 1;
    audit.panel_mutation_blocked = 1;
    audit.altar_mutation_blocked = 1;
    audit.raw_record_identity = raw->raw_record_identity;
    audit.source_evidence =
        "ReDMCSB REVIVE.C F0281/F0282/F0283 raw audit; runtime owner absent";
    *out = audit;
    return 1;
}
