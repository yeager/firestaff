#include "nexus_v1_bpk_archive.h"

#include <string.h>

static uint32_t rd32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           (uint32_t)p[3];
}

static int read_header(const uint8_t *data,
                       size_t data_size,
                       uint32_t *out_count) {
    uint32_t outer_size;
    uint32_t count;

    if (!data || !out_count) return -1;
    if (data_size < 24U) return -1;

    if (rd32_be(data + 0) != NEXUS_V1_BPK_MAGIC_BPPK) return -1;
    outer_size = rd32_be(data + 4);
    if (outer_size != data_size) return -1;
    if (rd32_be(data + 12) != NEXUS_V1_BPK_MAGIC_BMPD) return -1;

    count = rd32_be(data + 20);
    if (count == 0U || count > 4096U) return -1;
    if ((size_t)count > (SIZE_MAX - 24U) / 4U) return -1;
    if (24U + (size_t)count * 4U > data_size) return -1;

    *out_count = count;
    return 0;
}

static int read_candidate_offset(const uint8_t *data,
                                 size_t data_size,
                                 uint32_t count,
                                 uint32_t index,
                                 uint32_t *out_offset) {
    uint32_t offset;

    if (!data || !out_offset || index >= count) return -1;
    offset = rd32_be(data + 24U + (size_t)index * 4U);
    if ((size_t)offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES > data_size) {
        return -1;
    }

    *out_offset = offset;
    return 0;
}

int nexus_v1_bpk_archive_get_entry(const uint8_t *data,
                                   size_t data_size,
                                   uint32_t index,
                                   Nexus_V1_BpkEntry *out_entry) {
    uint32_t count;
    uint32_t offset;
    uint32_t next_offset;
    uint32_t payload_offset;
    int has_prs3;

    if (!out_entry) return -1;
    memset(out_entry, 0, sizeof(*out_entry));

    if (read_header(data, data_size, &count) != 0) return -1;
    if (read_candidate_offset(data, data_size, count, index, &offset) != 0) {
        return -1;
    }

    if (index + 1U < count) {
        if (read_candidate_offset(data, data_size, count, index + 1U,
                                  &next_offset) != 0) {
            return -1;
        }
    } else {
        if (data_size > UINT32_MAX) return -1;
        next_offset = (uint32_t)data_size;
    }

    if (next_offset <= offset) return -1;
    has_prs3 = (rd32_be(data + offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES) ==
                NEXUS_V1_BPK_MAGIC_PRS3);
    payload_offset = offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
                     (has_prs3 ? 4U : 0U);
    if (payload_offset > next_offset) return -1;

    out_entry->offset = offset;
    out_entry->next_offset = next_offset;
    out_entry->stored_size = next_offset - offset;
    out_entry->payload_offset = payload_offset;
    out_entry->payload_size = next_offset - payload_offset;
    out_entry->has_prs3 = has_prs3;
    return 0;
}

int nexus_v1_bpk_archive_parse(const uint8_t *data,
                               size_t data_size,
                               Nexus_V1_BpkArchiveInfo *out_info) {
    uint32_t count;
    uint32_t prev = 0U;
    uint32_t prs3 = 0U;
    uint32_t raw = 0U;
    uint32_t first = 0U;
    uint32_t last = 0U;

    if (!out_info) return -1;
    memset(out_info, 0, sizeof(*out_info));

    if (read_header(data, data_size, &count) != 0) return -1;

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntry entry;
        uint32_t offset;

        if (read_candidate_offset(data, data_size, count, i, &offset) != 0) {
            return -1;
        }
        if (i > 0U && offset <= prev) return -1;
        if (i == 0U) first = offset;
        last = offset;
        prev = offset;

        if (nexus_v1_bpk_archive_get_entry(data, data_size, i, &entry) != 0) {
            return -1;
        }
        if (entry.has_prs3) {
            ++prs3;
        } else {
            ++raw;
        }
    }

    out_info->outer_size = rd32_be(data + 4);
    out_info->bmpd_size = rd32_be(data + 16);
    out_info->entry_count_hint = count;
    out_info->candidate_offset_count = count;
    out_info->first_candidate_offset = first;
    out_info->last_candidate_offset = last;
    out_info->prs3_payload_count = prs3;
    out_info->raw_payload_count = raw;
    return 0;
}
