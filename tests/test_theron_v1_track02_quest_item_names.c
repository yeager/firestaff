#include "theron_v1_track02_item_name_source.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_retrieval_text_source.h"
#include "theron_v1_track02_campaign_mask_source.h"
#include "theron_v1_track02_thing_data.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_SIZE 2352u
#define USER_SECTOR_SIZE 2048u
#define USER_DATA_OFFSET 16u

static uint8_t *load_user_data(const char *path, size_t *out_size) {
    FILE *file = fopen(path, "rb");
    long raw_size;
    size_t sectors, i;
    uint8_t *raw, *user_data;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (raw_size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    raw = (uint8_t *)malloc((size_t)raw_size);
    if (!raw || fread(raw, 1u, (size_t)raw_size, file) != (size_t)raw_size) {
        free(raw);
        fclose(file);
        return NULL;
    }
    fclose(file);
    sectors = (size_t)raw_size / RAW_SECTOR_SIZE;
    user_data = (uint8_t *)malloc(sectors * USER_SECTOR_SIZE);
    if (!user_data) {
        free(raw);
        return NULL;
    }
    for (i = 0u; i < sectors; ++i)
        memcpy(user_data + i * USER_SECTOR_SIZE,
               raw + i * RAW_SECTOR_SIZE + USER_DATA_OFFSET,
               USER_SECTOR_SIZE);
    free(raw);
    *out_size = sectors * USER_SECTOR_SIZE;
    return user_data;
}

static uint8_t decoded_item_type(const Theron_Track02ItemRecord *record) {
    switch (record->category) {
    case THERON_CAT_WEAPON: return record->value.weapon.type;
    case THERON_CAT_CLOTHING: return record->value.clothing.type;
    case THERON_CAT_SCROLL: return record->value.scroll.type;
    case THERON_CAT_POTION: return record->value.potion.type;
    case THERON_CAT_MISC: return record->value.misc.type;
    default: return 0xffu;
    }
}

static unsigned int count_carryable_records_with_item_index(
    const uint8_t *user_data,
    size_t user_data_size,
    int variant,
    unsigned int dungeon,
    uint8_t quest_index) {
    Theron_DungeonData maps;
    Theron_ThingData *things;
    uint8_t flat_tiles[8192];
    unsigned int flat_count = 0u;
    unsigned int ground_ref_count;
    unsigned int matches = 0u;
    unsigned int category;
    Theron_Track02Variant source_variant = variant == 1 ?
        THERON_TRACK02_VARIANT_JP_BIN : variant == 3 ?
        THERON_TRACK02_VARIANT_US_CLONECD_RAW : THERON_TRACK02_VARIANT_US_BIN;

    assert(theron_v1_track02_dungeon_map_load_for_variant(
               user_data, user_data_size, source_variant, dungeon - 1u,
               &maps) == 1);
    for (unsigned int map = 0u; map < maps.map_count; ++map) {
        unsigned int width = maps.maps[map].header.x_dim + 1u;
        unsigned int height = maps.maps[map].header.y_dim + 1u;
        for (unsigned int x = 0u; x < width; ++x)
            for (unsigned int y = 0u; y < height; ++y)
                flat_tiles[flat_count++] = maps.maps[map].tiles[x][y];
    }
    ground_ref_count = theron_v1_track02_compute_ground_ref_count(
        flat_tiles, flat_count);
    things = (Theron_ThingData *)calloc(1u, sizeof(*things));
    assert(things != NULL);
    assert(theron_v1_track02_thing_data_load_for_variant(
               user_data, user_data_size, source_variant, dungeon - 1u,
               maps.object_counts, ground_ref_count, things) == 1);
    for (category = THERON_CAT_WEAPON; category <= THERON_CAT_MISC;
         ++category) {
        size_t record_size = theron_item_bytes[category];
        if (category == THERON_CAT_CHEST || record_size == 0u) continue;
        for (unsigned int index = 0u;
             index < things->object_counts[category]; ++index) {
            Theron_Track02ItemRecord record;
            const uint8_t *raw = &things->items[category][index * record_size];
            assert(theron_v1_track02_item_record_decode(
                       category, raw, record_size, &record) == 1);
            if (decoded_item_type(&record) == quest_index) {
                ++matches;
            }
        }
    }
    free(things);
    return matches;
}

static void verify_variant(const char *path, int variant) {
    static const uint8_t quest_indices[7] = {
        41u, 63u, 45u, 43u, 44u, 43u, 7u
    };
    static const uint8_t us_same_index_counts[7] = {
        0u, 1u, 2u, 2u, 1u, 3u, 2u
    };
    static const uint8_t jp_same_index_counts[7] = {
        0u, 11u, 2u, 2u, 0u, 3u, 2u
    };
    static const char *const us_names[7] = {
        "SHIELD DEFIANT", "TAZA BOOTS", "TAZA POLEYN", "SOUL CAGE",
        "TAZA ARMOR", "TAZAHELM", "THE RETALIATOR"
    };
    static const uint8_t jp_names[7][20] = {
        {0x83,0x66,0x83,0x74,0x83,0x42,0x83,0x41,0x83,0x93,
         0x83,0x67,0x83,0x56,0x81,0x5b,0x83,0x8b,0x83,0x68},
        {0x83,0x5e,0x83,0x55,0x83,0x75,0x81,0x5b,0x83,0x63},
        {0x83,0x5e,0x83,0x55,0x83,0x4f,0x83,0x8a,0x81,0x5b,0x83,0x75},
        {0x83,0x5c,0x83,0x45,0x83,0x8b,0x83,0x50,0x81,0x5b,0x83,0x57},
        {0x83,0x5e,0x83,0x55,0x83,0x41,0x81,0x5b,0x83,0x7d,0x81,0x5b},
        {0x83,0x5e,0x83,0x55,0x83,0x77,0x83,0x8b,0x83,0x81,0x83,0x62,
         0x83,0x67},
        {0x95,0x9c,0x8f,0x51,0x82,0xcc,0x8c,0x95}
    };
    static const uint8_t jp_sizes[7] = {20u, 10u, 12u, 12u, 12u, 14u, 8u};
    size_t user_data_size = 0u;
    size_t track_relative_shift = variant == 3 ? 0x70800u : 0u;
    int regional_variant = variant == 3 ? 2 : variant;
    uint8_t *user_data = load_user_data(path, &user_data_size);
    Theron_Track02RetrievalTextSource retrieval;
    Theron_Track02CampaignMaskSource campaign_mask;
    unsigned int dungeon;
    assert(user_data != NULL);
    assert(theron_v1_track02_decode_campaign_mask_source(
               user_data, user_data_size, variant, &campaign_mask) == 1);
    assert(campaign_mask.valid == 1 &&
           campaign_mask.variant == regional_variant);
    assert(campaign_mask.runtime_address == 0x267cu);
    assert(campaign_mask.campaign_bits_mask == 0x7fu);
    assert(campaign_mask.serialized_campaign_byte_load_proven == 1);
    assert(campaign_mask.dungeon_ordinal_to_bit_proven == 1);
    assert(campaign_mask.campaign_mask_merge_proven == 1);
    assert(campaign_mask.code_resource_launch_proven == 1);
    assert(campaign_mask.post_dungeon_ordinal_seed_proven == 1);
    assert(campaign_mask.post_dungeon_parameter_block_copy_proven == 1);
    assert(campaign_mask.post_dungeon_ordinal_forward_proven == 1);
    assert(campaign_mask.post_dungeon_record_base == 0x03c7u);
    assert(campaign_mask.post_dungeon_record_stride == 4u);
    assert(campaign_mask.post_dungeon_record_sector_count == 4u);
    assert(campaign_mask.post_dungeon_support_record == 0x03e3u);
    assert(campaign_mask.post_dungeon_support_sector_count == 2u);
    assert(campaign_mask.post_dungeon_shared_record == 0x03e7u);
    assert(campaign_mask.post_dungeon_shared_sector_count == 17u);
    assert(campaign_mask.post_dungeon_record_formula_proven == 1);
    assert(campaign_mask.post_dungeon_program_load_chain_proven == 1);
    assert(campaign_mask.post_dungeon_ordinal_dispatch_bytes == 0x103u);
    assert(campaign_mask.post_dungeon_text_opcode_handler_bytes == 0x21u);
    assert(campaign_mask.post_dungeon_text_selector_bytes == 0x13au);
    assert(campaign_mask.post_dungeon_text_ordinal_advance_bytes == 0x23u);
    assert(campaign_mask.post_dungeon_text_group == 2u);
    assert(campaign_mask.post_dungeon_ordinal_text_dispatch_proven == 1);
    assert(campaign_mask.post_dungeon_cd_base_track_bcd == 0x19u);
    assert(campaign_mask.post_dungeon_cd_base_proven == 1);
    assert(campaign_mask.descriptor_record_cd_read_proven == 1);
    assert(campaign_mask.descriptor_loader_offset ==
           campaign_mask.common_source_offset);
    assert(campaign_mask.descriptor_loader_bytes == 0x800u);
    assert(campaign_mask.descriptor_loader_fnv1a == 0x09ca5445u);
    assert(campaign_mask.artifact_collection_relation_proven == 0);
    {
        size_t offset = campaign_mask.common_source_offset + 0x8du;
        uint8_t saved = user_data[offset];
        user_data[offset] ^= 1u;
        assert(theron_v1_track02_decode_campaign_mask_source(
                   user_data, user_data_size, variant, &campaign_mask) == 0);
        user_data[offset] = saved;
    }
    assert(theron_v1_track02_decode_campaign_mask_source(
               user_data, user_data_size, variant, &campaign_mask) == 1);
    {
        size_t offset = campaign_mask.post_dungeon_ordinal_dispatch_offset +
            0x23u + 4u * 0x20u + 18u;
        uint8_t saved = user_data[offset];
        user_data[offset] ^= 1u;
        assert(theron_v1_track02_decode_campaign_mask_source(
                   user_data, user_data_size, variant, &campaign_mask) == 0);
        user_data[offset] = saved;
    }
    assert(theron_v1_track02_decode_campaign_mask_source(
               user_data, user_data_size, variant, &campaign_mask) == 1);
    {
        size_t offset = campaign_mask.post_dungeon_shared_program_offset +
            0x8243u;
        uint8_t saved = user_data[offset];
        user_data[offset] ^= 1u;
        assert(theron_v1_track02_decode_campaign_mask_source(
                   user_data, user_data_size, variant, &campaign_mask) == 0);
        user_data[offset] = saved;
    }
    assert(theron_v1_track02_decode_campaign_mask_source(
               user_data, user_data_size, variant, &campaign_mask) == 1);
    {
        size_t offset = campaign_mask.descriptor_loader_offset + 0x1e0u;
        uint8_t saved = user_data[offset];
        user_data[offset] ^= 1u;
        assert(theron_v1_track02_decode_campaign_mask_source(
                   user_data, user_data_size, variant, &campaign_mask) == 0);
        user_data[offset] = saved;
    }
    assert(theron_v1_track02_decode_campaign_mask_source(
               user_data, user_data_size, variant, &campaign_mask) == 1);
    {
        size_t offset = campaign_mask.post_dungeon_shared_program_offset +
            0x81d4u;
        uint8_t saved = user_data[offset];
        user_data[offset] ^= 1u;
        assert(theron_v1_track02_decode_campaign_mask_source(
                   user_data, user_data_size, variant, &campaign_mask) == 0);
        user_data[offset] = saved;
    }
    assert(theron_v1_track02_decode_campaign_mask_source(
               user_data, user_data_size, variant, &campaign_mask) == 1);
    {
        size_t offset = campaign_mask.post_dungeon_program_offsets[5] + 6u;
        uint8_t saved = user_data[offset];
        user_data[offset] ^= 1u;
        assert(theron_v1_track02_decode_campaign_mask_source(
                   user_data, user_data_size, variant, &campaign_mask) == 0);
        user_data[offset] = saved;
    }
    assert(theron_v1_track02_decode_campaign_mask_source(
               user_data, user_data_size, variant, &campaign_mask) == 1);
    {
        size_t offset = campaign_mask.dungeon_source_offsets[3] + 10u;
        uint8_t saved = user_data[offset];
        user_data[offset] ^= 1u;
        assert(theron_v1_track02_decode_campaign_mask_source(
                   user_data, user_data_size, variant, &campaign_mask) == 0);
        user_data[offset] = saved;
    }
    assert(theron_v1_track02_decode_retrieval_text_source(
               user_data, user_data_size, variant, &retrieval) == 1);
    assert(retrieval.valid == 1 && retrieval.variant == regional_variant);
    assert(retrieval.track02_resource_block ==
           (variant == 1 ? 0x040cu : 0x040du));
    assert(retrieval.resource_offset ==
           (variant == 1 ? 0x276800u : 0x277000u - track_relative_shift));
    assert(retrieval.resource_bytes == 2048u);
    assert(retrieval.resource_fnv1a ==
           (variant == 1 ? 0x851c05b3u : 0xeeb43e74u));
    assert(retrieval.resource_record_authenticated == 1);
    assert(retrieval.source_offset ==
           (variant == 1 ? 0x27696du : 0x27713du - track_relative_shift));
    assert(retrieval.source_span_bytes == (variant == 1 ? 364u : 331u));
    assert(retrieval.source_span_fnv1a ==
           (variant == 1 ? 0xcb874921u : 0x4777d500u));
    assert(retrieval.text_group == 2u);
    assert(retrieval.text_group_script_relative_offset == 0x00cau);
    assert(retrieval.message_list_relative_offset ==
           (variant == 1 ? 0x016du : 0x013du));
    assert(retrieval.retrieval_event_relation_proven == 1);
    assert(retrieval.host_text_rendering_proven == 0);
    assert(retrieval.raw_message_sizes[0] > 2u);
    assert(retrieval.raw_messages[0][0] ==
           (variant == 1 ? 0x81u : 0x05u));
    {
        uint8_t saved = user_data[retrieval.resource_offset];
        user_data[retrieval.resource_offset] ^= 1u;
        assert(theron_v1_track02_decode_retrieval_text_source(
                   user_data, user_data_size, variant, &retrieval) == 0);
        user_data[variant == 1 ? 0x276800u :
                  0x277000u - track_relative_shift] = saved;
    }
    assert(theron_v1_track02_decode_retrieval_text_source(
               user_data, user_data_size, variant, &retrieval) == 1);
    {
        uint8_t saved = user_data[retrieval.source_offset];
        user_data[retrieval.source_offset] ^= 1u;
        assert(theron_v1_track02_decode_retrieval_text_source(
                   user_data, user_data_size, variant, &retrieval) == 0);
        user_data[variant == 1 ? 0x27696du :
                  0x27713du - track_relative_shift] = saved;
    }
    for (dungeon = 1u; dungeon <= 7u; ++dungeon) {
        Theron_Track02ItemNameSource source, rejected;
        const uint8_t *name = NULL;
        size_t name_size = 0u;
        assert(theron_v1_track02_decode_item_name_source(
                   user_data, user_data_size, variant, dungeon, &source) == 1);
        assert(theron_v1_track02_quest_item_name_raw(
                   &source, &name, &name_size) == 1);
        assert(count_carryable_records_with_item_index(
                   user_data, user_data_size, variant, dungeon,
                   quest_indices[dungeon - 1u]) ==
               (variant == 1 ? jp_same_index_counts[dungeon - 1u] :
                               us_same_index_counts[dungeon - 1u]));
        if (variant == 1) {
            assert(name_size == jp_sizes[dungeon - 1u]);
            assert(memcmp(name, jp_names[dungeon - 1u], name_size) == 0);
        } else {
            assert(name_size == strlen(us_names[dungeon - 1u]));
            assert(memcmp(name, us_names[dungeon - 1u], name_size) == 0);
        }
        rejected = source;
        rejected.valid = 0;
        assert(theron_v1_track02_quest_item_name_raw(
                   &rejected, &name, &name_size) == 0);
        rejected = source;
        rejected.raw_name_sizes[dungeon == 1u ? 41u :
            dungeon == 2u ? 63u : dungeon == 3u ? 45u :
            dungeon == 5u ? 44u : dungeon == 7u ? 7u : 43u] = 0u;
        assert(theron_v1_track02_quest_item_name_raw(
                   &rejected, &name, &name_size) == 0);
    }
    free(user_data);
    printf("  PASS real %s quest-artifact names\n", variant == 1 ? "JP" : "US");
}

int main(void) {
    const char *home = getenv("HOME");
    const char *clonecd_path = getenv("FIRESTAFF_THERON_TRACK02_CLONECD_RAW");
    char us_path[1024], jp_path[1024];
    assert(home != NULL);
    snprintf(us_path, sizeof(us_path), "%s/.firestaff/data/theron/TQUS02.bin", home);
    snprintf(jp_path, sizeof(jp_path), "%s/.firestaff/data/theron/TQJP02.bin", home);
    verify_variant(us_path, 2);
    verify_variant(jp_path, 1);
    if (clonecd_path && clonecd_path[0]) verify_variant(clonecd_path, 3);
    return 0;
}
