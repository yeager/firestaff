#include "csb_v1_amiga_swsh_ftl.h"

#include "firestaff_ftl_container.h"
#include "firestaff_ftl_decode.h"

#include <string.h>

/* ReDMCSB SWSH.C F0904 (Amiga F31E source, lines 2902-2920) consumes the
 * 27 signed-command/value rows; SWSHGDAT.C's logo and SWSHSDAT.C's sample
 * are embedded in the decoded DATA hunk. FTL.H defines the container. */

enum {
    CSB_SWSH_DATA_BYTES = 11780,
    CSB_SWSH_PALETTE_OFFSET = 154,
    CSB_SWSH_LOGO_OFFSET = 262,
    CSB_SWSH_SAMPLE_PERIOD_OFFSET = 2644,
    CSB_SWSH_SAMPLE_COUNT_OFFSET = 2646,
    CSB_SWSH_SAMPLE_OFFSET = 2650
};

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
}

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(14695981039346656037);
    size_t index;
    for (index = 0u; index < size; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int csb_v1_amiga_swsh_ftl_decode(const uint8_t *bytes, size_t byte_count,
                                  CSB_V1_AmigaSwshFtlAssets *out_assets)
{
    static const uint64_t palette_hash = UINT64_C(0x4ff098bc53e8f432);
    static const uint64_t logo_hash = UINT64_C(0x08e34132eb706e05);
    static const uint64_t sample_hash = UINT64_C(0xd8a7920cbad582f5);
    FirestaffFtlContainer container;
    FirestaffFtlChecksumReport checksums;
    FirestaffFtl ftl;
    CSB_V1_AmigaSwshFtlAssets assets;
    const uint8_t *data;
    size_t index;
    int parsed = 0;
    int ok = 0;

    if (!out_assets) return 0;
    memset(out_assets, 0, sizeof(*out_assets));
    memset(&container, 0, sizeof(container));
    memset(&checksums, 0, sizeof(checksums));
    memset(&ftl, 0, sizeof(ftl));
    memset(&assets, 0, sizeof(assets));
    if (!bytes || byte_count == 0u || byte_count > 64u * 1024u * 1024u ||
        FirestaffFtlContainer_Parse(bytes, byte_count, &container) != 0 ||
        FirestaffFtlContainer_VerifyChecksums(bytes, byte_count, &container,
                                              &checksums) != 0 ||
        container.hunk_count != 3u || container.bss_index != 0 ||
        container.data_index != 1 || container.code_index != 2 ||
        checksums.common_header.status != FIRESTAFF_FTL_CHECK_MATCH ||
        checksums.bss_hunk.status != FIRESTAFF_FTL_CHECK_MATCH ||
        checksums.data_hunk.status != FIRESTAFF_FTL_CHECK_MATCH ||
        checksums.code_hunk.status != FIRESTAFF_FTL_CHECK_MATCH ||
        FirestaffFtl_Parse(bytes, byte_count, &ftl) != 0) {
        return 0;
    }
    parsed = 1;
    if (FirestaffFtl_Decode(&ftl) != 0 ||
        ftl.hunk_data_decoded_size != CSB_SWSH_DATA_BYTES) goto cleanup;
    data = ftl.hunk_data_decoded;
    if (!data || read_be16(data + CSB_SWSH_LOGO_OFFSET) !=
                     CSB_V1_AMIGA_SWSH_WIDTH ||
        read_be16(data + CSB_SWSH_LOGO_OFFSET + 2u) !=
                     CSB_V1_AMIGA_SWSH_HEIGHT ||
        read_be16(data + CSB_SWSH_SAMPLE_PERIOD_OFFSET) != 334u ||
        read_be32(data + CSB_SWSH_SAMPLE_COUNT_OFFSET) !=
                     CSB_V1_AMIGA_SWSH_SAMPLE_BYTES ||
        fnv1a64(data + CSB_SWSH_PALETTE_OFFSET, 108u) != palette_hash ||
        fnv1a64(data + CSB_SWSH_LOGO_OFFSET,
                CSB_V1_AMIGA_SWSH_GRAPHIC_BYTES) != logo_hash ||
        fnv1a64(data + CSB_SWSH_SAMPLE_OFFSET,
                CSB_V1_AMIGA_SWSH_SAMPLE_BYTES) != sample_hash) {
        goto cleanup;
    }
    for (index = 0u; index < CSB_V1_AMIGA_SWSH_PALETTE_EVENT_COUNT; ++index) {
        const uint8_t *event = data + CSB_SWSH_PALETTE_OFFSET + index * 4u;
        assets.palette[index].command = (int16_t)read_be16(event);
        assets.palette[index].parameter = read_be16(event + 2u);
        if (assets.palette[index].command != -1 &&
            (assets.palette[index].command < 0 ||
             assets.palette[index].command >= 16)) goto cleanup;
        if (assets.palette[index].command == -1 &&
            assets.palette[index].parameter > 60u) goto cleanup;
        if (assets.palette[index].command >= 0 &&
            assets.palette[index].parameter > 0x0fffu) goto cleanup;
    }
    memcpy(assets.logo_graphic, data + CSB_SWSH_LOGO_OFFSET,
           sizeof(assets.logo_graphic));
    memcpy(assets.sample, data + CSB_SWSH_SAMPLE_OFFSET,
           sizeof(assets.sample));
    assets.sample_period = read_be16(data + CSB_SWSH_SAMPLE_PERIOD_OFFSET);
    *out_assets = assets;
    ok = 1;

cleanup:
    if (parsed) FirestaffFtl_Free(&ftl);
    return ok;
}
