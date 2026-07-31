#ifndef NEXUS_V1_RLOWFIX_TEXT_H
#define NEXUS_V1_RLOWFIX_TEXT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int valid;
    uint16_t resource_index;
    uint16_t string_count;
    uint32_t resource_offset;
    uint32_t table_end;
    uint32_t resource_end;
} Nexus_V1_RlowfixText;

typedef struct {
    int valid;
    uint16_t entry_count;
    uint32_t resource_offset;
    uint16_t code[216];
} Nexus_V1_RlowfixTabl;

/* DMWeb DecodeRES/DecodeTEXT receipt.  This exposes offsets and bytes only;
 * TABL character decoding and Saturn presentation remain separate gates. */
int nexus_v1_rlowfix_text_parse(const uint8_t *data, size_t size,
                                uint32_t resource_offset,
                                Nexus_V1_RlowfixText *out);

int nexus_v1_rlowfix_text_span(const uint8_t *data, size_t size,
                               const Nexus_V1_RlowfixText *text,
                               uint16_t string_index,
                               const uint8_t **bytes, size_t *byte_count);

int nexus_v1_rlowfix_tabl_parse(const uint8_t *data, size_t size,
                                uint32_t resource_offset,
                                Nexus_V1_RlowfixTabl *out);

#endif
