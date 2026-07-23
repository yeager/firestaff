#ifndef FIRESTAFF_CSB_V1_F0279_F0283_CHAMPION_ALTAR_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0279_F0283_CHAMPION_ALTAR_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

enum {
    CSB_V1_F0279_MAX_ENCODED_CHARACTERS = 4,
    CSB_V1_F0280_ENCODED_VALUE_BYTES = 26,
    CSB_V1_F0280_MAX_PORTRAIT_INDEX = 23,
    CSB_V1_F0282_COMMAND_RESURRECT = 160,
    CSB_V1_F0282_COMMAND_REINCARNATE = 161,
    CSB_V1_F0282_COMMAND_CANCEL = 162
};

/* All inputs are caller-owned bytes from an authenticated PC34 champion
 * record/capture. No function in this header mutates those bytes or runtime. */
typedef struct CSB_V1_ChampionRawMaterialPc34 {
    const uint8_t *bytes;
    size_t byte_count;
    uint32_t raw_record_identity;
    int authenticated_pc34;
} CSB_V1_ChampionRawMaterialPc34;

typedef struct CSB_V1_F0279DecodedValueReceiptPc34 {
    int admitted;
    uint16_t decoded_value;
    uint8_t character_count;
    uint32_t raw_record_identity;
    const char *source_evidence;
} CSB_V1_F0279DecodedValueReceiptPc34;

typedef struct CSB_V1_F0280CandidateRawReceiptPc34 {
    int admitted;
    int party_append_blocked;
    uint8_t portrait_index;
    uint8_t decoded_value_field_count;
    uint32_t raw_record_identity;
    const char *source_evidence;
} CSB_V1_F0280CandidateRawReceiptPc34;

typedef struct CSB_V1_F0281F0283MutationAuditPc34 {
    int raw_material_admitted;
    int known_command;
    int rename_ui_blocked;
    int panel_mutation_blocked;
    int altar_mutation_blocked;
    uint32_t raw_record_identity;
    const char *source_evidence;
} CSB_V1_F0281F0283MutationAuditPc34;

int csb_v1_f0279_decode_raw_value_pc34(
    const CSB_V1_ChampionRawMaterialPc34 *raw,
    size_t byte_offset,
    uint8_t character_count,
    CSB_V1_F0279DecodedValueReceiptPc34 *out);

int csb_v1_f0280_candidate_raw_receipt_pc34(
    const CSB_V1_ChampionRawMaterialPc34 *raw,
    uint8_t portrait_index,
    size_t encoded_values_offset,
    CSB_V1_F0280CandidateRawReceiptPc34 *out);

int csb_v1_f0281_f0283_mutation_audit_pc34(
    const CSB_V1_ChampionRawMaterialPc34 *raw,
    int command,
    CSB_V1_F0281F0283MutationAuditPc34 *out);

#endif
