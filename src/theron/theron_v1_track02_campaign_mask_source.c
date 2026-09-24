#include "theron_v1_track02_campaign_mask_source.h"

#include <string.h>

#define CAMPAIGN_MASK_COMMON_BYTES 0x150u
#define DESCRIPTOR_LOADER_BYTES 0x800u
#define CAMPAIGN_MASK_DUNGEON_BYTES 0x130u
#define POST_DUNGEON_PROGRAM_BYTES 0x2000u
#define POST_DUNGEON_SHARED_PROGRAM_BYTES 0x8800u
#define POST_DUNGEON_ORDINAL_DISPATCH_BYTES 0x103u
#define POST_DUNGEON_TEXT_OPCODE_HANDLER_BYTES 0x21u
#define POST_DUNGEON_TEXT_SELECTOR_BYTES 0x13au
#define POST_DUNGEON_TEXT_ORDINAL_ADVANCE_BYTES 0x23u

static const size_t g_us_offsets[7] = {
    0x08de01u, 0x0cde01u, 0x10de07u, 0x14de01u,
    0x18de01u, 0x1cde01u, 0x20de01u
};
static const uint32_t g_us_hashes[7] = {
    0xf79db535u, 0xf79db535u, 0x53d7a138u, 0xf79db535u,
    0xf79db535u, 0xf79db535u, 0xf79db535u
};
static const size_t g_jp_offsets[7] = {
    0x08d44fu, 0x0cd44fu, 0x10d44fu, 0x14d44fu,
    0x18d44fu, 0x1cd44fu, 0x20d44fu
};
static const uint32_t g_jp_hashes[7] = {
    0x80020fc1u, 0x80020fc1u, 0x111937a2u, 0x80020fc1u,
    0x111937a2u, 0x80020fc1u, 0x2cd93490u
};
static const uint32_t g_us_post_dungeon_hashes[7] = {
    0xfc4ed1ddu, 0x337de858u, 0x52b4e49eu, 0x72e8db9fu,
    0xb2f52cdeu, 0x243f09b8u, 0x4ecb9c8du
};
static const uint32_t g_jp_post_dungeon_hashes[7] = {
    0xe1c0915eu, 0xc5a76147u, 0x5e7a1511u, 0xbf074800u,
    0x9644cdd5u, 0xe88b231bu, 0xe455a456u
};

static uint32_t mask_fnv1a(const uint8_t *bytes, size_t count) {
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int theron_v1_track02_decode_campaign_mask_source(
    const uint8_t *user_data,
    size_t user_data_size,
    int variant,
    Theron_Track02CampaignMaskSource *out) {
    static const uint8_t marker_and_bits[17] = {
        'D','M','S','-','S','G','.','0','0','1',
        0x01u,0x02u,0x04u,0x08u,0x10u,0x20u,0x40u
    };
    static const uint8_t common_mask_sequence[16] = {
        0xadu,0x7cu,0x26u,0x29u,0x80u,0x8du,0x7cu,0x26u,
        0x68u,0x0du,0x7cu,0x26u,0x8du,0x7cu,0x26u,0x29u
    };
    static const uint8_t descriptor_record_read[39] = {
        0x44u,0xc6u,0x8au,0x18u,0x65u,0x14u,0x85u,0xfeu,
        0x98u,0x65u,0x15u,0x85u,0xfdu,0x64u,0xfcu,0xa0u,
        0x03u,0xb1u,0x01u,0x85u,0xffu,0xc8u,0xb1u,0x01u,
        0x85u,0xfau,0xc8u,0xb1u,0x01u,0x85u,0xfbu,0xc8u,
        0xb1u,0x01u,0x85u,0xf8u,0x20u,0x09u,0xe0u
    };
    static const uint8_t post_dungeon_cd_base[16] = {
        0xa9u,0x80u,0x85u,0xfbu,0xa9u,0x19u,0x85u,0xf8u,
        0xa9u,0x02u,0x85u,0xfcu,0x20u,0x06u,0xe0u,0x60u
    };
    static const uint8_t ordinal_record_arithmetic[10] = {
        0x0au,0x0au,0x18u,0x69u,0xc7u,0xa8u,0x62u,0x69u,0x03u,0xaau
    };
    static const uint8_t ordinal_record_read[30] = {
        0x00u,0x48u,0x64u,0xfcu,0x86u,0xfdu,0x84u,0xfeu,
        0xa9u,0x01u,0x85u,0xffu,0xa9u,0x00u,0x85u,0xfau,
        0xa9u,0x40u,0x85u,0xfbu,0xa9u,0x04u,0x85u,0xf8u,
        0x20u,0x09u,0xe0u,0x68u,0x4cu,0x00u
    };
    static const uint8_t shared_program_read[4] = {
        0x00u,0xe7u,0x03u,0x11u
    };
    static const uint8_t support_program_read[4] = {
        0x00u,0xe3u,0x03u,0x02u
    };
    const size_t *offsets;
    size_t clonecd_offsets[7];
    const uint32_t *hashes;
    const uint32_t *post_dungeon_hashes;
    size_t common_offset;
    size_t post_dungeon_first_offset;
    size_t post_dungeon_shared_program_offset;
    uint32_t post_dungeon_shared_program_hash;
    size_t ordinal_dispatch_relative_offset;
    uint32_t ordinal_dispatch_hash;
    size_t text_opcode_handler_relative_offset;
    uint32_t text_opcode_handler_hash;
    size_t text_selector_relative_offset;
    uint32_t text_selector_hash;
    size_t text_ordinal_advance_relative_offset;
    uint32_t text_ordinal_advance_hash;
    unsigned int i;

    if (out) memset(out, 0, sizeof(*out));
    if (!user_data || !out) return 0;
    if (variant == 2 || variant == 3) {
        common_offset = 0x071800u;
        offsets = g_us_offsets;
        hashes = g_us_hashes;
        post_dungeon_first_offset = 0x254000u;
        post_dungeon_hashes = g_us_post_dungeon_hashes;
        post_dungeon_shared_program_offset = 0x264000u;
        post_dungeon_shared_program_hash = 0x113c8278u;
        ordinal_dispatch_relative_offset = 0x284au;
        ordinal_dispatch_hash = 0x815cbce4u;
        text_opcode_handler_relative_offset = 0x0653u;
        text_opcode_handler_hash = 0xcafb5d7fu;
        text_selector_relative_offset = 0x16afu;
        text_selector_hash = 0x46cd7f6cu;
        text_ordinal_advance_relative_offset = 0x11d2u;
        text_ordinal_advance_hash = 0x5813b731u;
    } else if (variant == 1) {
        common_offset = 0x071000u;
        offsets = g_jp_offsets;
        hashes = g_jp_hashes;
        post_dungeon_first_offset = 0x253800u;
        post_dungeon_hashes = g_jp_post_dungeon_hashes;
        post_dungeon_shared_program_offset = 0x263800u;
        post_dungeon_shared_program_hash = 0x834bede1u;
        ordinal_dispatch_relative_offset = 0x284au;
        ordinal_dispatch_hash = 0x7700654cu;
        text_opcode_handler_relative_offset = 0x0653u;
        text_opcode_handler_hash = 0xf7f547ccu;
        text_selector_relative_offset = 0x1729u;
        text_selector_hash = 0xc8086d7bu;
        text_ordinal_advance_relative_offset = 0x1209u;
        text_ordinal_advance_hash = 0x69e37389u;
    } else return 0;
    if (variant == 3) {
        for (i = 0u; i < 7u; ++i)
            clonecd_offsets[i] = g_us_offsets[i] - 0x70800u;
        offsets = clonecd_offsets;
        common_offset -= 0x70800u;
        post_dungeon_first_offset -= 0x70800u;
        post_dungeon_shared_program_offset -= 0x70800u;
    }
    if (common_offset > user_data_size ||
        DESCRIPTOR_LOADER_BYTES > user_data_size - common_offset ||
        mask_fnv1a(user_data + common_offset, DESCRIPTOR_LOADER_BYTES) !=
            0x09ca5445u ||
        mask_fnv1a(user_data + common_offset, CAMPAIGN_MASK_COMMON_BYTES) !=
            0x72af456fu ||
        memcmp(user_data + common_offset + 0x8du,
               common_mask_sequence, sizeof(common_mask_sequence)) != 0 ||
        user_data[common_offset + 0x9du] != 0x7fu ||
        memcmp(user_data + common_offset + 0x1e0u,
               descriptor_record_read, sizeof(descriptor_record_read)) != 0)
        return 0;
    for (i = 0u; i < 7u; ++i) {
        const uint8_t *window;
        uint16_t source_cpu_address;
        uint16_t bit_table_cpu_address;
        uint16_t self_modified_cpu_address;
        if (offsets[i] > user_data_size ||
            CAMPAIGN_MASK_DUNGEON_BYTES > user_data_size - offsets[i])
            return 0;
        window = user_data + offsets[i];
        self_modified_cpu_address = (uint16_t)(window[0x26u] |
                                               (window[0x27u] << 8));
        source_cpu_address = (uint16_t)(self_modified_cpu_address - 0x10du);
        bit_table_cpu_address = (uint16_t)(source_cpu_address + 0x0au);
        if (mask_fnv1a(window, CAMPAIGN_MASK_DUNGEON_BYTES) != hashes[i] ||
            memcmp(window, marker_and_bits, sizeof(marker_and_bits)) != 0 ||
            window[0x20u] != 0xadu || window[0x21u] != 0x7cu ||
            window[0x22u] != 0x26u || window[0x23u] != 0x29u ||
            window[0x24u] != 0x7fu || window[0x25u] != 0x8du ||
            window[0x37u] != 0xc9u || window[0x38u] != 0x06u ||
            window[0x39u] != 0x90u || window[0x3au] != 0x03u ||
            window[0x3eu] != 0xaau || window[0x3fu] != 0xbdu ||
            window[0x40u] != (uint8_t)bit_table_cpu_address ||
            window[0x41u] != (uint8_t)(bit_table_cpu_address >> 8) ||
            window[0x72u] != 0x68u || window[0x73u] != 0x0du ||
            window[0x74u] != 0x7cu || window[0x75u] != 0x26u ||
            window[0x76u] != 0x8du || window[0x77u] != 0x7cu ||
            window[0x78u] != 0x26u ||
            window[0xfau] != 0xadu ||
            window[0xfbu] != (uint8_t)self_modified_cpu_address ||
            window[0xfcu] != (uint8_t)(self_modified_cpu_address >> 8) ||
            memcmp(window + 0xfdu, ordinal_record_arithmetic,
                   sizeof(ordinal_record_arithmetic)) != 0 ||
            window[0x107u] != 0xadu ||
            window[0x108u] != (uint8_t)self_modified_cpu_address ||
            window[0x109u] != (uint8_t)(self_modified_cpu_address >> 8) ||
            window[0x10au] != 0x4cu ||
            window[0x10bu] != (uint8_t)(self_modified_cpu_address + 1u) ||
            window[0x10cu] != (uint8_t)(self_modified_cpu_address >> 8) ||
            memcmp(window + 0x10du, ordinal_record_read,
                   sizeof(ordinal_record_read)) != 0 ||
            window[0x12bu] != 0x40u)
            return 0;
        out->dungeon_source_offsets[i] = offsets[i];
        out->dungeon_source_fnv1a[i] = hashes[i];
    }
    for (i = 0u; i < 7u; ++i) {
        size_t offset = post_dungeon_first_offset +
            (size_t)i * POST_DUNGEON_PROGRAM_BYTES;
        const uint8_t *program;
        uint8_t screen_selector = i == 6u ? 0x0fu : 0x09u;
        if (offset > user_data_size ||
            POST_DUNGEON_PROGRAM_BYTES > user_data_size - offset)
            return 0;
        program = user_data + offset;
        if (mask_fnv1a(program, POST_DUNGEON_PROGRAM_BYTES) !=
                post_dungeon_hashes[i] ||
            program[0x00u] != 0xa9u ||
            program[0x01u] != screen_selector ||
            program[0x02u] != 0x8du || program[0x03u] != 0x00u ||
            program[0x04u] != 0x27u || program[0x05u] != 0xa9u ||
            program[0x06u] != (uint8_t)i || program[0x07u] != 0x8du ||
            program[0x08u] != 0x01u || program[0x09u] != 0x27u ||
            program[0x9eu] != 0x20u || program[0x9fu] != 0x0fu ||
            program[0xa0u] != 0xe0u ||
            program[0xc7u] != 0x20u || program[0xc8u] != 0x09u ||
            program[0xc9u] != 0xe0u ||
            memcmp(program + 0xcfu, shared_program_read,
                   sizeof(shared_program_read)) != 0 ||
            memcmp(program + 0xd6u, support_program_read,
                   sizeof(support_program_read)) != 0)
            return 0;
        out->post_dungeon_program_offsets[i] = offset;
        out->post_dungeon_program_fnv1a[i] = post_dungeon_hashes[i];
    }
    {
        static const uint8_t parameter_copy[22] = {
            0xadu,0x00u,0x27u,0x8du,0x57u,0x82u,
            0xadu,0x01u,0x27u,0x8du,0x58u,0x82u,
            0x73u,0x57u,0x82u,0x80u,0x27u,0x20u,0x00u,
            0x60u,0x00u,0x00u
        };
        static const uint8_t ordinal_forward[7] = {
            0x73u,0x81u,0x27u,0x8fu,0x6du,0x07u,0x00u
        };
        const uint8_t *shared;
        const uint8_t *dispatch;
        if (post_dungeon_shared_program_offset > user_data_size ||
            POST_DUNGEON_SHARED_PROGRAM_BYTES >
                user_data_size - post_dungeon_shared_program_offset)
            return 0;
        shared = user_data + post_dungeon_shared_program_offset;
        if (mask_fnv1a(shared, POST_DUNGEON_SHARED_PROGRAM_BYTES) !=
                post_dungeon_shared_program_hash ||
            shared[0x800cu] != 0x20u || shared[0x800du] != 0x43u ||
            shared[0x800eu] != 0x82u ||
            memcmp(shared + 0x8243u, parameter_copy,
                   sizeof(parameter_copy)) != 0 ||
            memcmp(shared + 0x81d4u, post_dungeon_cd_base,
                   sizeof(post_dungeon_cd_base)) != 0 ||
            memcmp(shared + 0x2da4u, ordinal_forward,
                   sizeof(ordinal_forward)) != 0 ||
            mask_fnv1a(shared + ordinal_dispatch_relative_offset,
                       POST_DUNGEON_ORDINAL_DISPATCH_BYTES) !=
                ordinal_dispatch_hash ||
            mask_fnv1a(shared + text_opcode_handler_relative_offset,
                       POST_DUNGEON_TEXT_OPCODE_HANDLER_BYTES) !=
                text_opcode_handler_hash ||
            mask_fnv1a(shared + text_selector_relative_offset,
                       POST_DUNGEON_TEXT_SELECTOR_BYTES) !=
                text_selector_hash ||
            mask_fnv1a(shared + text_ordinal_advance_relative_offset,
                       POST_DUNGEON_TEXT_ORDINAL_ADVANCE_BYTES) !=
                text_ordinal_advance_hash)
            return 0;
        dispatch = shared + ordinal_dispatch_relative_offset;
        for (i = 0u; i < 7u; ++i) {
            const size_t branch = (size_t)i * 5u;
            const size_t block = 0x23u + (size_t)i * 0x20u;
            if (dispatch[branch] != 0x01u ||
                dispatch[branch + 1u] != 0x01u ||
                dispatch[branch + 2u] != (uint8_t)i ||
                dispatch[block] != 0x1au ||
                dispatch[block + 1u] != (uint8_t)i ||
                dispatch[block + 16u] != 0x2bu ||
                dispatch[block + 17u] != 0x02u ||
                dispatch[block + 18u] != (uint8_t)i)
                return 0;
        }
        out->post_dungeon_shared_program_offset =
            post_dungeon_shared_program_offset;
        out->post_dungeon_shared_program_bytes =
            POST_DUNGEON_SHARED_PROGRAM_BYTES;
        out->post_dungeon_shared_program_fnv1a =
            post_dungeon_shared_program_hash;
        out->post_dungeon_ordinal_dispatch_offset =
            post_dungeon_shared_program_offset +
            ordinal_dispatch_relative_offset;
        out->post_dungeon_ordinal_dispatch_bytes =
            POST_DUNGEON_ORDINAL_DISPATCH_BYTES;
        out->post_dungeon_ordinal_dispatch_fnv1a = ordinal_dispatch_hash;
        out->post_dungeon_text_opcode_handler_offset =
            post_dungeon_shared_program_offset +
            text_opcode_handler_relative_offset;
        out->post_dungeon_text_opcode_handler_bytes =
            POST_DUNGEON_TEXT_OPCODE_HANDLER_BYTES;
        out->post_dungeon_text_opcode_handler_fnv1a =
            text_opcode_handler_hash;
        out->post_dungeon_text_selector_offset =
            post_dungeon_shared_program_offset + text_selector_relative_offset;
        out->post_dungeon_text_selector_bytes =
            POST_DUNGEON_TEXT_SELECTOR_BYTES;
        out->post_dungeon_text_selector_fnv1a = text_selector_hash;
        out->post_dungeon_text_ordinal_advance_offset =
            post_dungeon_shared_program_offset +
            text_ordinal_advance_relative_offset;
        out->post_dungeon_text_ordinal_advance_bytes =
            POST_DUNGEON_TEXT_ORDINAL_ADVANCE_BYTES;
        out->post_dungeon_text_ordinal_advance_fnv1a =
            text_ordinal_advance_hash;
    }
    out->valid = 1;
    out->variant = variant == 3 ? 2 : variant;
    out->common_source_offset = common_offset;
    out->common_source_bytes = CAMPAIGN_MASK_COMMON_BYTES;
    out->common_source_fnv1a = 0x72af456fu;
    out->descriptor_loader_offset = common_offset;
    out->descriptor_loader_bytes = DESCRIPTOR_LOADER_BYTES;
    out->descriptor_loader_fnv1a = 0x09ca5445u;
    out->runtime_address = 0x267cu;
    out->campaign_bits_mask = 0x7fu;
    out->serialized_campaign_byte_load_proven = 1;
    out->dungeon_ordinal_to_bit_proven = 1;
    out->campaign_mask_merge_proven = 1;
    out->code_resource_launch_proven = 1;
    out->post_dungeon_ordinal_seed_proven = 1;
    out->post_dungeon_parameter_block_copy_proven = 1;
    out->post_dungeon_ordinal_forward_proven = 1;
    out->post_dungeon_record_base = 0x03c7u;
    out->post_dungeon_record_stride = 4u;
    out->post_dungeon_record_sector_count = 4u;
    out->post_dungeon_support_record = 0x03e3u;
    out->post_dungeon_support_sector_count = 2u;
    out->post_dungeon_shared_record = 0x03e7u;
    out->post_dungeon_shared_sector_count = 17u;
    out->post_dungeon_record_formula_proven = 1;
    out->post_dungeon_program_load_chain_proven = 1;
    out->post_dungeon_text_group = 2u;
    out->post_dungeon_ordinal_text_dispatch_proven = 1;
    out->post_dungeon_cd_base_track_bcd = 0x19u;
    out->post_dungeon_cd_base_proven = 1;
    out->descriptor_record_cd_read_proven = 1;
    out->artifact_collection_relation_proven = 0;
    return 1;
}
