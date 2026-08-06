#include "dm1_v1_atari_st_stx.h"

#include <ctype.h>
#include <string.h>

#define STX_HEADER_SIZE 16u
#define STX_TRACK_HEADER_SIZE 16u
#define STX_SECTOR_BLOCK_SIZE 16u
#define STX_FLAG_SECTOR_BLOCK 0x0001u
#define STX_FLAG_RNF 0x10u
#define SECTOR_SIZE 512u

static uint16_t le16(const uint8_t *p)
{
    return (uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8);
}

static uint32_t le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int range_ok(size_t offset, size_t length, size_t size)
{
    return offset <= size && length <= size - offset;
}

static int track_info(const DM1_V1_AtariStx *stx, uint32_t track,
                      uint32_t *block_size, uint32_t *fuzzy_size,
                      uint16_t *sector_count, uint16_t *flags)
{
    size_t offset;
    const uint8_t *p;
    if (!stx || track >= stx->track_count) return 0;
    offset = stx->track_offsets[track];
    if (!range_ok(offset, STX_TRACK_HEADER_SIZE, stx->size)) return 0;
    p = stx->data + offset;
    *block_size = le32(p);
    *fuzzy_size = le32(p + 4u);
    *sector_count = le16(p + 8u);
    *flags = le16(p + 10u);
    return *block_size >= STX_TRACK_HEADER_SIZE &&
           range_ok(offset, *block_size, stx->size);
}

int dm1_v1_atari_st_stx_open(const uint8_t *data, size_t size,
                             DM1_V1_AtariStx *out)
{
    uint32_t offset = STX_HEADER_SIZE;
    uint32_t sectors = 0u;
    uint8_t i;

    if (!data || !out || size < STX_HEADER_SIZE ||
        memcmp(data, "RSY\0", 4u) != 0 || le16(data + 4u) != 3u ||
        data[10] == 0u) return 0;
    memset(out, 0, sizeof(*out));
    out->data = data;
    out->size = size;
    out->track_count = data[10];
    for (i = 0u; i < out->track_count; ++i) {
        uint32_t block;
        uint32_t fuzzy;
        uint16_t count;
        uint16_t flags;
        out->track_offsets[i] = offset;
        if (!track_info(out, i, &block, &fuzzy, &count, &flags)) return 0;
        if (count > 64u || (flags & STX_FLAG_SECTOR_BLOCK) == 0u) {
            if (count > 64u || count > (UINT32_MAX - sectors)) return 0;
        }
        if ((flags & STX_FLAG_SECTOR_BLOCK) != 0u) {
            size_t table = STX_TRACK_HEADER_SIZE +
                           (size_t)count * STX_SECTOR_BLOCK_SIZE;
            if (!range_ok((size_t)offset + table, fuzzy, size)) return 0;
            /* Every filesystem sector must be a real 512-byte sector. */
            for (uint16_t s = 0u; s < count; ++s) {
                const uint8_t *entry = data + offset + STX_TRACK_HEADER_SIZE +
                                       (size_t)s * STX_SECTOR_BLOCK_SIZE;
                uint32_t data_offset = le32(entry);
                uint8_t size_code = entry[11];
                if ((entry[14] & STX_FLAG_RNF) != 0u ||
                    (128u << (size_code & 3u)) != SECTOR_SIZE ||
                    !range_ok((size_t)offset + table + fuzzy + data_offset,
                              SECTOR_SIZE, size)) return 0;
            }
        } else if (!range_ok((size_t)offset + STX_TRACK_HEADER_SIZE,
                             (size_t)count * SECTOR_SIZE, size)) {
            return 0;
        }
        sectors += count;
        offset += block;
    }
    if (offset != size) return 0;
    out->sector_count = sectors;
    return sectors != 0u;
}

static int read_track_sector(const DM1_V1_AtariStx *stx, uint32_t track,
                             uint32_t wanted, uint8_t *out)
{
    uint32_t block, fuzzy;
    uint16_t count, flags;
    size_t offset;
    if (!track_info(stx, track, &block, &fuzzy, &count, &flags) ||
        wanted >= count) return 0;
    offset = stx->track_offsets[track];
    if ((flags & STX_FLAG_SECTOR_BLOCK) == 0u) {
        memcpy(out, stx->data + offset + STX_TRACK_HEADER_SIZE +
               (size_t)wanted * SECTOR_SIZE, SECTOR_SIZE);
        return 1;
    }
    /* STX sector blocks are not required to be listed in disk order. */
    {
        uint32_t offsets[64];
        for (uint32_t s = 0u; s < count; ++s) {
            const uint8_t *entry = stx->data + offset + STX_TRACK_HEADER_SIZE +
                                   (size_t)s * STX_SECTOR_BLOCK_SIZE;
            offsets[s] = le32(entry);
        }
        for (uint32_t i = 1u; i < count; ++i) {
            uint32_t value = offsets[i];
            uint32_t j = i;
            while (j > 0u && offsets[j - 1u] > value) {
                offsets[j] = offsets[j - 1u];
                --j;
            }
            offsets[j] = value;
        }
        memcpy(out, stx->data + offset + STX_TRACK_HEADER_SIZE +
               (size_t)count * STX_SECTOR_BLOCK_SIZE + fuzzy + offsets[wanted],
               SECTOR_SIZE);
        return 1;
    }
}

int dm1_v1_atari_st_stx_read_sector(const DM1_V1_AtariStx *stx,
                                    uint32_t logical_sector,
                                    uint8_t *out, size_t capacity)
{
    uint32_t cursor = 0u;
    if (!stx || !out || capacity < SECTOR_SIZE || logical_sector >= stx->sector_count)
        return 0;
    for (uint32_t track = 0u; track < stx->track_count; ++track) {
        uint32_t block, fuzzy;
        uint16_t count, flags;
        if (!track_info(stx, track, &block, &fuzzy, &count, &flags)) return 0;
        if (logical_sector < cursor + count)
            return read_track_sector(stx, track, logical_sector - cursor, out);
        cursor += count;
    }
    return 0;
}

static int name_matches(const uint8_t *entry, const char *name)
{
    const char *dot = strchr(name, '.');
    size_t base_len = dot ? (size_t)(dot - name) : strlen(name);
    size_t ext_len = dot ? strlen(dot + 1u) : 0u;
    if (base_len == 0u || base_len > 8u || ext_len > 3u) return 0;
    for (size_t i = 0u; i < 8u; ++i) {
        unsigned char expected = i < base_len
            ? (unsigned char)toupper((unsigned char)name[i]) : (unsigned char)' ';
        if (entry[i] != expected) return 0;
    }
    for (size_t i = 0u; i < 3u; ++i) {
        unsigned char expected = i < ext_len
            ? (unsigned char)toupper((unsigned char)dot[1u + i]) : (unsigned char)' ';
        if (entry[8u + i] != expected) return 0;
    }
    return entry[11] == 0u;
}

int dm1_v1_atari_st_stx_extract_file(const DM1_V1_AtariStx *stx,
                                     const char *name83,
                                     uint8_t *out, size_t capacity,
                                     size_t *out_size)
{
    uint8_t sector[SECTOR_SIZE];
    uint8_t fat[2048];
    uint32_t first_cluster = 0u, file_size = 0u;
    int found = 0;
    if (!stx || !name83 || !out || !out_size ||
        !dm1_v1_atari_st_stx_read_sector(stx, 5u, sector, sizeof(sector))) return 0;
    *out_size = 0u;
    for (uint32_t i = 0u; i < 4u; ++i) {
        if (!dm1_v1_atari_st_stx_read_sector(stx, i + 1u,
                                             fat + i * SECTOR_SIZE,
                                             SECTOR_SIZE)) return 0;
    }
    for (size_t offset = 0u; offset + 32u <= sizeof(sector); offset += 32u) {
        const uint8_t *entry = sector + offset;
        if (entry[0] == 0u) break;
        if (entry[0] == 0xe5u || !name_matches(entry, name83)) continue;
        first_cluster = (uint32_t)entry[26] | ((uint32_t)entry[27] << 8);
        file_size = le32(entry + 28u);
        found = 1;
        break;
    }
    if (!found || file_size > capacity) return 0;
    for (uint32_t cluster = first_cluster; cluster >= 2u && cluster < 0xff8u;) {
        uint32_t base = 6u + (cluster - 2u) * 2u;
        uint8_t pair[SECTOR_SIZE * 2u];
        size_t before = *out_size;
        if (!dm1_v1_atari_st_stx_read_sector(stx, base, pair, sizeof(pair)) ||
            !dm1_v1_atari_st_stx_read_sector(stx, base + 1u,
                                             pair + SECTOR_SIZE, sizeof(pair) - SECTOR_SIZE)) return 0;
        if (*out_size < file_size) {
            size_t take = file_size - *out_size;
            if (take > sizeof(pair)) take = sizeof(pair);
            memcpy(out + *out_size, pair, take);
            *out_size += take;
        }
        if (*out_size == file_size) return 1;
        uint32_t fat_offset = cluster + cluster / 2u;
        uint32_t next = (cluster & 1u) == 0u
            ? (fat[fat_offset] | ((fat[fat_offset + 1u] & 0x0fu) << 8))
            : ((fat[fat_offset] >> 4) | (fat[fat_offset + 1u] << 4));
        if (next == cluster || *out_size == before) return 0;
        cluster = next & 0xfffu;
    }
    return 0;
}
