#include "dm2_v1_creature_animation_gdat.h"

#include "dm2_v1_creature.h"
#include "dm2_v1_skproject_core.h"
#include "dm2_v1_fmtowns_graphics_dat.h"

#include <string.h>

static uint16_t dm2_v1_read_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t dm2_v1_hash_bytes(uint32_t hash, const uint8_t *bytes,
                                  size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int dm2_v1_creature_animation_gdat_query_0958(
    const DM2_V1_AssetLoader *loader,
    int creature_type,
    uint16_t animation_base,
    uint16_t *io_timer_word,
    uint32_t game_tick,
    DM2_V1_CreatureAnimation0958Receipt *out_receipt)
{
    const uint8_t *table;
    size_t table_size = 0u;
    DM2_V1_SkprojectQuery4e26Receipt query_receipt;
    DM2_V1_CreatureAnimation0958Receipt receipt;
    uint16_t query_index = 0u;
    size_t row;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.creature_type = (uint8_t)(creature_type & 0xff);
    receipt.animation_base = animation_base;
    if (!loader || !io_timer_word || creature_type < 0 || creature_type > 0xff) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.timer_word_before = *io_timer_word;
    if (!dm2_v1_skproject_query_4e26(io_timer_word, game_tick,
                                     &query_index, &query_receipt)) {
        receipt.blocked_out_of_range = query_receipt.blocked_zero_divisor;
        receipt.timer_word_after = *io_timer_word;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.timer_word_after = *io_timer_word;
    receipt.query_index = query_index;

    table = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE,
        &table_size);
    if (!table || table_size < 4u || (table_size % 4u) != 0u) {
        receipt.blocked_missing_gdat = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    row = (size_t)animation_base + query_index;
    if (row >= table_size / 4u) {
        receipt.blocked_out_of_range = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    /* SKProject SK1C9A.cpp:5377-5400: DM2_4DEA copies four bytes from
     * GDAT 0xfc, then CUTLX8(value) & 0x80 is shifted right by seven. */
    receipt.blended_value = (uint32_t)table[row * 4u] |
                            ((uint32_t)table[row * 4u + 1u] << 8) |
                            ((uint32_t)table[row * 4u + 2u] << 16) |
                            ((uint32_t)table[row * 4u + 3u] << 24);
    receipt.frame_bit14 = (uint8_t)((receipt.blended_value & 0x80u) >> 7);
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_creature_animation_gdat_query_0958_record(
    const DM2_V1_AssetLoader *loader,
    uint8_t *creature_record,
    size_t creature_record_size,
    const DM2_AIDefinition *ai_spec,
    uint8_t *caii_slots,
    size_t caii_capacity,
    uint32_t game_tick,
    DM2_V1_CreatureAnimation0958Receipt *out_receipt)
{
    DM2_V1_CreatureAnimation0958Receipt receipt;
    uint8_t *cursor;
    uint8_t creature_type;
    uint8_t slot_index = 0xffu;
    uint16_t timer_word;
    DM2_V1_CreatureAnimation0958Receipt gdat_receipt;
    int is_static;

    memset(&receipt, 0, sizeof(receipt));
    if (!creature_record || creature_record_size < 12u || !ai_spec) {
        receipt.blocked_record_owner = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    creature_type = creature_record[4];
    is_static = (ai_spec->w0AIFlags & DM2_AIFLAG_STATIC) != 0u;
    if (is_static) {
        /* c_querydb.cpp:2981-2983 — static AI points at the DB4 record. */
        cursor = creature_record + 8u;
        receipt.cursor_static_owner = 1;
    } else {
        /* c_querydb.cpp:2984-2990 — live creature AI points at
         * creatures[record byte@5], whose stride is c_creature 0x22. */
        slot_index = creature_record[5];
        if (slot_index == 0xffu || !caii_slots ||
            (size_t)slot_index >= caii_capacity) {
            receipt.blocked_caii_owner = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        cursor = caii_slots + (size_t)slot_index * 34u + 8u;
    }
    receipt.cursor_owner_bound = 1;
    receipt.caii_slot = slot_index;
    receipt.cursor_w0 = (uint16_t)cursor[0] | ((uint16_t)cursor[1] << 8);
    timer_word = (uint16_t)cursor[2] | ((uint16_t)cursor[3] << 8);
    receipt.cursor_w2 = timer_word;
    if (!dm2_v1_creature_animation_gdat_query_0958(
            loader, creature_type, receipt.cursor_w0,
            &timer_word, game_tick, &gdat_receipt)) {
        gdat_receipt.cursor_owner_bound = receipt.cursor_owner_bound;
        gdat_receipt.cursor_static_owner = receipt.cursor_static_owner;
        gdat_receipt.blocked_record_owner = receipt.blocked_record_owner;
        gdat_receipt.blocked_caii_owner = receipt.blocked_caii_owner;
        gdat_receipt.caii_slot = receipt.caii_slot;
        gdat_receipt.cursor_w0 = receipt.cursor_w0;
        gdat_receipt.cursor_w2 = timer_word;
        if (out_receipt) *out_receipt = gdat_receipt;
        return 0;
    }
    cursor[2] = (uint8_t)(timer_word & 0xffu);
    cursor[3] = (uint8_t)(timer_word >> 8);
    gdat_receipt.cursor_owner_bound = receipt.cursor_owner_bound;
    gdat_receipt.cursor_static_owner = receipt.cursor_static_owner;
    gdat_receipt.blocked_record_owner = receipt.blocked_record_owner;
    gdat_receipt.blocked_caii_owner = receipt.blocked_caii_owner;
    gdat_receipt.caii_slot = receipt.caii_slot;
    gdat_receipt.cursor_w0 = receipt.cursor_w0;
    gdat_receipt.cursor_w2 = timer_word;
    if (out_receipt) {
        *out_receipt = gdat_receipt;
    }
    return 1;
}

int dm2_v1_creature_animation_gdat_image_field(
    const DM2_V1_AssetLoader *loader,
    int creature_type,
    uint16_t animation_info,
    uint8_t face_dir_img,
    uint8_t *out_image_field)
{
    const uint8_t *table;
    size_t table_size = 0u;
    size_t row;

    if (out_image_field) *out_image_field = 0u;
    if (!loader || !out_image_field || creature_type < 0 ||
        creature_type > 0xff || face_dir_img > 3u) {
        return 0;
    }
    table = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_FRAME_SEQUENCE,
        &table_size);
    /* v4/skcrture.cpp:1969-1978 — FD rows contain four direction bytes
     * followed by the source offset/metadata byte span. */
    if (!table || table_size < 8u || (table_size % 8u) != 0u) return 0;
    /* skcrture.cpp:1974 normalizes the source's unset animation info. */
    row = animation_info == 0xffffu ? 0u : (size_t)animation_info;
    if (row >= table_size / 8u) return 0;
    *out_image_field = table[row * 8u + face_dir_img];
    return *out_image_field != 0xffu;
}

int dm2_v1_creature_animation_gdat_select_dynamic_v5(
    const DM2_V1_AssetLoader *loader,
    int creature_type,
    uint16_t command,
    uint16_t previous_frame,
    uint16_t ai_flags,
    int direction,
    DM2_V1_CreatureAnimationGdatReceipt *out_receipt)
{
    const uint8_t *attribution;
    const uint8_t *info_sequence;
    const uint8_t *frame_sequence;
    size_t attribution_size = 0u;
    size_t info_size = 0u;
    size_t frame_size = 0u;
    size_t attribution_count;
    size_t info_count;
    size_t frame_count;
    size_t command_row = 0u;
    uint16_t sequence_offset;
    uint16_t selected_frame;
    int found = 0;
    DM2_V1_CreatureAnimationGdatReceipt candidate;

    if (!out_receipt || !loader || creature_type < 0 || creature_type > 0xff ||
        (ai_flags & DM2_AIFLAG_STATIC) != 0u) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    attribution = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW8, DM2_GDAT_CREATURE_ANIM_ATTRIBUTION,
        &attribution_size);
    info_sequence = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE,
        &info_size);
    frame_sequence = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_CREATURES, creature_type,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_CREATURE_ANIM_FRAME_SEQUENCE,
        &frame_size);
    if (!attribution || !info_sequence || !frame_sequence ||
        attribution_size < 4u || attribution_size % 4u != 0u ||
        info_size < 4u || info_size % 4u != 0u ||
        frame_size < 8u || frame_size % 8u != 0u) {
        return 0;
    }

    attribution_count = attribution_size / 4u;
    info_count = info_size / 4u;
    frame_count = frame_size / 8u;
    for (size_t i = 0; i < attribution_count; ++i) {
        uint16_t row_command = dm2_v1_read_le16(attribution + i * 4u);
        if (row_command == command || row_command == 0xffffu) {
            command_row = i;
            found = 1;
            break;
        }
    }
    if (!found) return 0;

    sequence_offset = dm2_v1_read_le16(attribution + command_row * 4u + 2u);
    if (sequence_offset >= info_count) return 0;
    /* v4/skcrture.cpp:CREATURE_STEP_ANIMATION_V5 starts at the caller's
     * iAnimInfo and only advances while the current FC.seqnext is not the
     * terminal marker.  Do not pre-increment the source-owned frame: doing
     * so skips a terminal frame and can walk into the next sequence. */
    selected_frame = previous_frame == 0xffffu ? 0u : previous_frame;
    if ((size_t)sequence_offset + selected_frame >= info_count) return 0;

    /* CREATURE_STEP_ANIMATION_V5 advances until FC.seqnext is the source
     * terminal marker 0x0f. Bound the walk by the original table length. */
    while (info_sequence[((size_t)sequence_offset + selected_frame) * 4u + 1u] != 0x0fu) {
        ++selected_frame;
        if ((size_t)sequence_offset + selected_frame >= info_count) return 0;
    }
    if (selected_frame >= frame_count) return 0;

    candidate.dynamic = 1;
    candidate.creature_type = (uint8_t)creature_type;
    candidate.command = command;
    candidate.sequence_offset = sequence_offset;
    candidate.previous_frame = previous_frame;
    candidate.selected_frame = selected_frame;
    candidate.direction = (uint8_t)(direction & 3);
    candidate.image_id = frame_sequence[(size_t)selected_frame * 8u + candidate.direction];
    candidate.table_hash = dm2_v1_hash_bytes(2166136261u, attribution,
                                              attribution_size);
    candidate.table_hash = dm2_v1_hash_bytes(candidate.table_hash,
                                              info_sequence, info_size);
    candidate.table_hash = dm2_v1_hash_bytes(candidate.table_hash,
                                              frame_sequence, frame_size);
    candidate.valid = candidate.table_hash != 0u;
    if (!candidate.valid) return 0;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_creature_animation_gdat_static_frame_fmtowns(
    const DM2_V1_AssetLoader *loader,
    uint8_t creature_type,
    uint16_t packed_position,
    uint16_t *out_frame)
{
    const uint8_t *table;
    size_t table_size = 0u;
    size_t trailer;
    size_t start;
    size_t rows = 0u;
    unsigned offset;
    uint16_t animation_index;

    if (out_frame) *out_frame = 0xffffu;
    if (!loader || loader->gdat_version != DM2_FMTOWNS_GDAT_VERSION ||
        !out_frame) return 0;
    table = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0, DM2_GDAT_ENTRY_TYPE_RAW6,
        0, &table_size);
    /* HME-242's type-06 item ends with 0x54, followed by 42 u16 offsets.
     * The final two bytes are the last offset; the bounds are checked from
     * the actual item length so another authenticated FM Towns build cannot
     * make this walk escape the item. */
    if (!table || table_size < 0x58u) return 0;
    trailer = table_size - 0x58u;
    if (dm2_v1_read_le16(table + trailer) != 0x0054u)
        return 0;
    if (!dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_CREATURES,
                                      creature_type, 0x00,
                                      &animation_index) &&
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_CREATURES,
                                      creature_type, 0x05,
                                      &animation_index)) return 0;
    if (animation_index >= 42u) return 0;
    offset = dm2_v1_read_le16(table + trailer + 2u +
                              (size_t)animation_index * 2u);
    start = (size_t)offset;
    if (start + 4u > trailer) return 0;
    while (start + (rows + 1u) * 4u <= trailer && rows < 0x3fffu) {
        const uint8_t high = table[start + rows * 4u + 1u] & 0xf0u;
        ++rows; /* The source counts the terminating row as a frame. */
        if (high == 0u) break;
    }
    if (rows == 0u || start + rows * 4u > trailer) return 0;
    *out_frame = (uint16_t)(rows | (packed_position == 0u ? 0x9000u :
        (((uint32_t)packed_position & 0x3fu) << 6) | 0x8000u));
    return 1;
}
