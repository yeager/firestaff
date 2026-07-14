#ifndef NEXUS_V1_SAL_CORPUS_RECEIPT_H
#define NEXUS_V1_SAL_CORPUS_RECEIPT_H

#include "nexus_v1_audio_receipt.h"

/* A corpus-level SAL comparison. This records only byte equality across the
 * canonical level-bank set; the shared region is not a header, sample table,
 * or decoder input until independent Saturn-driver evidence establishes that.
 */
typedef struct {
    int expected_bank_count;
    int supplied_bank_count;
    int present_bank_count;
    int missing_bank_count;
    uint32_t shortest_bank_size;
    uint32_t shared_prefix_byte_count;
    int first_divergent_bank_index;
    uint32_t first_divergent_offset;
    int complete;
    int codec_or_playback_authorized;
} Nexus_V1_SalSharedPrefixReceipt;

int nexus_v1_audio_sal_shared_prefix_receipt(
    const uint8_t *const banks[NEXUS_V1_AUDIO_LEVEL_COUNT],
    const uint32_t sizes[NEXUS_V1_AUDIO_LEVEL_COUNT],
    Nexus_V1_SalSharedPrefixReceipt *out);

#endif
