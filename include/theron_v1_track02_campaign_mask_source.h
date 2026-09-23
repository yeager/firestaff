#ifndef THERON_V1_TRACK02_CAMPAIGN_MASK_SOURCE_H
#define THERON_V1_TRACK02_CAMPAIGN_MASK_SOURCE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THERON_TRACK02_CAMPAIGN_MASK_DUNGEON_COUNT 7u

typedef struct {
    int valid;
    int variant; /* 1 = JP, 2 = US */
    size_t common_source_offset;
    size_t common_source_bytes;
    uint32_t common_source_fnv1a;
    size_t descriptor_loader_offset;
    size_t descriptor_loader_bytes;
    uint32_t descriptor_loader_fnv1a;
    size_t dungeon_source_offsets[THERON_TRACK02_CAMPAIGN_MASK_DUNGEON_COUNT];
    uint32_t dungeon_source_fnv1a[THERON_TRACK02_CAMPAIGN_MASK_DUNGEON_COUNT];
    size_t post_dungeon_program_offsets
        [THERON_TRACK02_CAMPAIGN_MASK_DUNGEON_COUNT];
    uint32_t post_dungeon_program_fnv1a
        [THERON_TRACK02_CAMPAIGN_MASK_DUNGEON_COUNT];
    size_t post_dungeon_shared_program_offset;
    size_t post_dungeon_shared_program_bytes;
    uint32_t post_dungeon_shared_program_fnv1a;
    size_t post_dungeon_ordinal_dispatch_offset;
    size_t post_dungeon_ordinal_dispatch_bytes;
    uint32_t post_dungeon_ordinal_dispatch_fnv1a;
    size_t post_dungeon_text_opcode_handler_offset;
    size_t post_dungeon_text_opcode_handler_bytes;
    uint32_t post_dungeon_text_opcode_handler_fnv1a;
    size_t post_dungeon_text_selector_offset;
    size_t post_dungeon_text_selector_bytes;
    uint32_t post_dungeon_text_selector_fnv1a;
    size_t post_dungeon_text_ordinal_advance_offset;
    size_t post_dungeon_text_ordinal_advance_bytes;
    uint32_t post_dungeon_text_ordinal_advance_fnv1a;
    uint16_t runtime_address;
    uint8_t campaign_bits_mask;
    int serialized_campaign_byte_load_proven;
    int dungeon_ordinal_to_bit_proven;
    int campaign_mask_merge_proven;
    int code_resource_launch_proven;
    int post_dungeon_ordinal_seed_proven;
    int post_dungeon_parameter_block_copy_proven;
    int post_dungeon_ordinal_forward_proven;
    uint16_t post_dungeon_record_base;
    uint8_t post_dungeon_record_stride;
    uint8_t post_dungeon_record_sector_count;
    uint16_t post_dungeon_support_record;
    uint8_t post_dungeon_support_sector_count;
    uint16_t post_dungeon_shared_record;
    uint8_t post_dungeon_shared_sector_count;
    int post_dungeon_record_formula_proven;
    int post_dungeon_program_load_chain_proven;
    uint8_t post_dungeon_text_group;
    int post_dungeon_ordinal_text_dispatch_proven;
    uint8_t post_dungeon_cd_base_track_bcd;
    int post_dungeon_cd_base_proven;
    int descriptor_record_cd_read_proven;
    int artifact_collection_relation_proven;
} Theron_Track02CampaignMaskSource;

/* Authenticate the common save-body consumer and all seven dungeon-local
 * transition windows. This proves that original RAM $267c participates in
 * seven-bit campaign/dungeon state; it does not prove artifact collection. */
int theron_v1_track02_decode_campaign_mask_source(
    const uint8_t *user_data,
    size_t user_data_size,
    int variant,
    Theron_Track02CampaignMaskSource *out);

#ifdef __cplusplus
}
#endif

#endif
