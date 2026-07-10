#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_dmdf_model.h"

#include <stdlib.h>
#include <stdint.h>
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
    has_prs3 = (next_offset - offset >=
                NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 4U) &&
               (rd32_be(data + offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES) ==
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

int nexus_v1_bpk_archive_get_entry_prefix(const uint8_t *data,
                                          size_t data_size,
                                          uint32_t index,
                                          Nexus_V1_BpkEntryPrefix *out_prefix) {
    Nexus_V1_BpkEntry entry;
    uint16_t width;
    uint8_t height;
    uint8_t mode;

    if (!out_prefix) return -1;
    memset(out_prefix, 0, sizeof(*out_prefix));

    if (nexus_v1_bpk_archive_get_entry(data, data_size, index, &entry) != 0) {
        return -1;
    }

    out_prefix->prefix_complete = (entry.stored_size >=
                                   NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);

    /* Even partial entries get the bytes we have; the rest stays zeroed. */
    {
        uint32_t copy = entry.stored_size;
        if (copy > NEXUS_V1_BPK_ENTRY_PREFIX_BYTES) {
            copy = NEXUS_V1_BPK_ENTRY_PREFIX_BYTES;
        }
        if (copy > 0U) {
            memcpy(out_prefix->raw,
                   data + entry.offset,
                   copy);
        }
    }

    width = (uint16_t)(((uint16_t)out_prefix->raw[NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET]
                         << 8) |
                        out_prefix->raw[NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET + 1U]);
    height = out_prefix->raw[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET];
    mode = out_prefix->raw[NEXUS_V1_BPK_PREFIX_MODE_OFFSET];

    out_prefix->width = width;
    out_prefix->height = height;
    out_prefix->mode = mode;
    return 0;
}

int nexus_v1_bpk_archive_inspect_prs3(const uint8_t *data,
                                      size_t data_size,
                                      uint32_t index,
                                      Nexus_V1_BpkPrs3Info *out_info) {
    Nexus_V1_BpkEntry entry;
    Nexus_V1_BpkEntryPrefix prefix;
    uint32_t prs3_off;
    uint32_t prs3_pixel_count;
    uint32_t version;
    uint32_t prefix_pixels;
    uint32_t payload_start;

    if (!out_info) return -1;
    memset(out_info, 0, sizeof(*out_info));

    if (nexus_v1_bpk_archive_get_entry(data, data_size, index, &entry) != 0) {
        return -1;
    }
    if (nexus_v1_bpk_archive_get_entry_prefix(data, data_size, index,
                                              &prefix) != 0) {
        return -1;
    }

    out_info->has_prs3 = entry.has_prs3;
    if (!entry.has_prs3) {
        return 0;
    }

    /* Need 20 (prefix) + 12 (PRS3 hdr) = 32 bytes minimum to inspect. */
    if (entry.stored_size <
        NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + NEXUS_V1_BPK_PRS3_HEADER_BYTES) {
        return -1;
    }

    prs3_off = entry.offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES;

    /* PRS3+0 is the magic (already verified by entry.has_prs3 == 1). */
    version = rd32_be(data + prs3_off + 4U);
    out_info->prs3_version_matches = (version == NEXUS_V1_BPK_PRS3_VERSION);

    prs3_pixel_count = rd32_be(data + prs3_off + 8U);
    out_info->prs3_pixel_count = prs3_pixel_count;

    prefix_pixels = (uint32_t)prefix.width * (uint32_t)prefix.height;
    out_info->prefix_pixels = prefix_pixels;
    out_info->pixel_count_matches = (prs3_pixel_count == prefix_pixels);

    payload_start = prs3_off + NEXUS_V1_BPK_PRS3_HEADER_BYTES;
    if (payload_start + 4U <= data_size) {
        uint32_t declared = rd32_be(data + payload_start);
        uint32_t available =
            (data_size - payload_start - 4U > UINT32_MAX)
                ? UINT32_MAX
                : (uint32_t)(data_size - payload_start - 4U);
        out_info->payload_available = 1;
        out_info->compressed_size =
            (declared <= available) ? declared : available;
    }
    return 0;
}

int nexus_v1_bpk_archive_mode_distribution(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkModeDistribution *out_dist) {
    uint32_t count;

    if (!out_dist) return -1;
    memset(out_dist, 0, sizeof(*out_dist));

    if (read_header(data, data_size, &count) != 0) return -1;

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntryPrefix prefix;
        if (nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            return -1;
        }
        if (!prefix.prefix_complete) continue;

        out_dist->mode_count[prefix.mode] += 1U;
        out_dist->total_with_prefix += 1U;
        if (prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
            out_dist->trailer_index = i;
            out_dist->trailer_found = 1;
        }
    }
    return 0;
}

/* pass1083 — per-entry surface-class contract. Maps the four observed
 * PRS3 pixel-mode tags (6/14/22/30) and the unique directory trailer
 * (10) to a stable enum + bytes-per-pixel value. PRS3 decompression is
 * still intentionally unsupported; this only describes the unpacked
 * surface shape a future decoder would have to fill. */
Nexus_V1_BpkSurfaceClass nexus_v1_bpk_mode_to_surface_class(uint8_t mode) {
    switch (mode) {
    case NEXUS_V1_BPK_MODE_8BPP:     return NEXUS_V1_BPK_SURFACE_INDEXED_8BPP;
    case NEXUS_V1_BPK_MODE_16BPP:    return NEXUS_V1_BPK_SURFACE_RGB565;
    case NEXUS_V1_BPK_MODE_24BPP:    return NEXUS_V1_BPK_SURFACE_RGB888;
    case NEXUS_V1_BPK_MODE_32BPP:    return NEXUS_V1_BPK_SURFACE_RGBA8888;
    case NEXUS_V1_BPK_MODE_TRAILER:  return NEXUS_V1_BPK_SURFACE_DIRECTORY_TRAILER;
    default:                         return NEXUS_V1_BPK_SURFACE_UNKNOWN;
    }
}

uint32_t nexus_v1_bpk_mode_to_bpp(uint8_t mode) {
    switch (mode) {
    case NEXUS_V1_BPK_MODE_8BPP:  return 1U;
    case NEXUS_V1_BPK_MODE_16BPP: return 2U;
    case NEXUS_V1_BPK_MODE_24BPP: return 3U;
    case NEXUS_V1_BPK_MODE_32BPP: return 4U;
    case NEXUS_V1_BPK_MODE_TRAILER:
    default:                      return 0U;
    }
}

int nexus_v1_bpk_archive_surface_estimate(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkSurfaceEntry *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkSurfaceEstimate *out_summary) {
    uint32_t count;
    uint32_t trailer_skip = 0U;
    uint32_t unknown_skip = 0U;
    uint32_t with_surface = 0U;
    uint32_t written = 0U;
    uint32_t truncated = 0U;
    uint64_t total_bytes = 0U;

    if (!out_summary) return -1;
    memset(out_summary, 0, sizeof(*out_summary));
    out_summary->capacity = entry_capacity;

    if (read_header(data, data_size, &count) != 0) return -1;

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntryPrefix prefix;
        uint32_t bpp;
        uint32_t rowstride;
        uint32_t surface_bytes;

        if (nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            return -1;
        }
        if (!prefix.prefix_complete) {
            /* Skip partial entries; they have no usable surface layout. */
            continue;
        }
        if (prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
            ++trailer_skip;
            continue;
        }
        bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
        if (bpp == 0U) {
            ++unknown_skip;
            continue;
        }
        rowstride = (uint32_t)prefix.width * bpp;
        surface_bytes = rowstride * (uint32_t)prefix.height;

        if (out_entries && written < entry_capacity) {
            Nexus_V1_BpkSurfaceEntry *row = &out_entries[written];
            row->entry_index = i;
            row->mode = prefix.mode;
            row->width = prefix.width;
            row->height = prefix.height;
            row->pixel_count =
                (uint32_t)prefix.width * (uint32_t)prefix.height;
            row->layout.bpp = bpp;
            row->layout.rowstride = rowstride;
            row->layout.surface_bytes = surface_bytes;
            row->layout.surface_class =
                nexus_v1_bpk_mode_to_surface_class(prefix.mode);
            ++written;
        } else if (out_entries && written >= entry_capacity) {
            truncated = 1U;
        }
        ++with_surface;
        total_bytes += surface_bytes;
    }

    out_summary->used = written;
    out_summary->total_with_surface = with_surface;
    out_summary->total_surface_bytes = total_bytes;
    out_summary->trailer_skipped = trailer_skip;
    out_summary->unknown_skipped = unknown_skip;
    out_summary->truncated = (int)truncated;
    return 0;
}

int nexus_v1_bpk_archive_runtime_render_receipt(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkRuntimeRenderReceipt *out_receipt) {
    uint32_t count;
    uint64_t expected_surface_bytes = 0U;
    uint64_t packed_payload_bytes = 0U;
    uint64_t stored_surface_bytes_available = 0U;
    uint32_t prs3_entries = 0U;
    uint32_t raw_entries = 0U;
    uint32_t surface_entries = 0U;
    uint32_t prs3_surface_entries = 0U;
    uint32_t stored_surface_entries = 0U;
    uint32_t stored_surface_short_entries = 0U;
    uint32_t trailer_entries = 0U;
    uint32_t unknown_mode_entries = 0U;
    int directory_trailer_found = 0;
    int all_prs3_versions_match = 1;
    int all_prs3_pixel_counts_match = 1;
    int all_stored_surface_payloads_fit = 1;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));

    if (read_header(data, data_size, &count) != 0) {
        out_receipt->route = NEXUS_V1_BPK_RUNTIME_ROUTE_INVALID;
        return -1;
    }

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntry entry;
        Nexus_V1_BpkEntryPrefix prefix;
        uint32_t bpp;

        if (nexus_v1_bpk_archive_get_entry(data, data_size, i, &entry) != 0) {
            out_receipt->route = NEXUS_V1_BPK_RUNTIME_ROUTE_INVALID;
            return -1;
        }
        if (nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            out_receipt->route = NEXUS_V1_BPK_RUNTIME_ROUTE_INVALID;
            return -1;
        }

        if (entry.has_prs3) {
            Nexus_V1_BpkPrs3Info prs3;
            ++prs3_entries;
            if (nexus_v1_bpk_archive_inspect_prs3(data, data_size, i,
                                                  &prs3) != 0) {
                all_prs3_versions_match = 0;
                all_prs3_pixel_counts_match = 0;
            } else {
                if (!prs3.prs3_version_matches) all_prs3_versions_match = 0;
                if (!prs3.pixel_count_matches) all_prs3_pixel_counts_match = 0;
                packed_payload_bytes += prs3.compressed_size;
            }
        } else {
            ++raw_entries;
        }

        if (!prefix.prefix_complete) continue;
        if (prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
            ++trailer_entries;
            directory_trailer_found = 1;
            continue;
        }

        bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
        if (bpp == 0U) {
            ++unknown_mode_entries;
            continue;
        }

        ++surface_entries;
        expected_surface_bytes +=
            (uint64_t)prefix.width * (uint64_t)prefix.height * bpp;
        if (entry.has_prs3) {
            ++prs3_surface_entries;
        } else {
            uint64_t payload_bytes = entry.payload_size;
            ++stored_surface_entries;
            stored_surface_bytes_available += payload_bytes;
            if (payload_bytes <
                (uint64_t)prefix.width * (uint64_t)prefix.height * bpp) {
                ++stored_surface_short_entries;
                all_stored_surface_payloads_fit = 0;
            }
        }
    }

    out_receipt->archive_entries = count;
    out_receipt->prs3_entries = prs3_entries;
    out_receipt->raw_entries = raw_entries;
    out_receipt->surface_entries = surface_entries;
    out_receipt->prs3_surface_entries = prs3_surface_entries;
    out_receipt->stored_surface_entries = stored_surface_entries;
    out_receipt->trailer_entries = trailer_entries;
    out_receipt->unknown_mode_entries = unknown_mode_entries;
    out_receipt->expected_surface_bytes = expected_surface_bytes;
    out_receipt->packed_payload_bytes = packed_payload_bytes;
    out_receipt->stored_surface_bytes_available =
        stored_surface_bytes_available;
    out_receipt->stored_surface_short_entries = stored_surface_short_entries;
    out_receipt->directory_trailer_found = directory_trailer_found;
    out_receipt->all_prs3_versions_match = all_prs3_versions_match;
    out_receipt->all_prs3_pixel_counts_match = all_prs3_pixel_counts_match;
    out_receipt->all_stored_surface_payloads_fit =
        all_stored_surface_payloads_fit;
    out_receipt->requires_prs3_decoder = (prs3_surface_entries > 0U) ? 1 : 0;

    if (surface_entries == 0U) {
        out_receipt->route = NEXUS_V1_BPK_RUNTIME_ROUTE_NO_SURFACES;
    } else if (prs3_surface_entries > 0U) {
        out_receipt->route = NEXUS_V1_BPK_RUNTIME_ROUTE_BLOCKED_PRS3;
    } else if (!all_stored_surface_payloads_fit) {
        out_receipt->route =
            NEXUS_V1_BPK_RUNTIME_ROUTE_BLOCKED_STORED_TRUNCATED;
    } else {
        out_receipt->route = NEXUS_V1_BPK_RUNTIME_ROUTE_READY_STORED;
    }

    out_receipt->fallback_visuals_permitted =
        (out_receipt->route == NEXUS_V1_BPK_RUNTIME_ROUTE_READY_STORED) ? 1 : 0;
    return 0;
}

const char *nexus_v1_bpk_runtime_render_route_name(
    Nexus_V1_BpkRuntimeRenderRoute route) {
    switch (route) {
    case NEXUS_V1_BPK_RUNTIME_ROUTE_INVALID:      return "invalid";
    case NEXUS_V1_BPK_RUNTIME_ROUTE_NO_SURFACES:  return "no-surfaces";
    case NEXUS_V1_BPK_RUNTIME_ROUTE_BLOCKED_PRS3: return "blocked-prs3";
    case NEXUS_V1_BPK_RUNTIME_ROUTE_READY_STORED: return "ready-stored";
    case NEXUS_V1_BPK_RUNTIME_ROUTE_BLOCKED_STORED_TRUNCATED:
        return "blocked-stored-truncated";
    default:                                      return "unknown";
    }
}

int nexus_v1_bpk_archive_extract_stored_surface(
    const uint8_t *data,
    size_t data_size,
    uint32_t index,
    uint8_t *out,
    size_t out_size,
    Nexus_V1_BpkSurfaceEntry *out_surface,
    size_t *out_written) {
    Nexus_V1_BpkEntry entry;
    Nexus_V1_BpkEntryPrefix prefix;
    uint32_t bpp;
    uint32_t rowstride;
    uint32_t surface_bytes;

    if (out_written) *out_written = 0U;
    if (out_surface) memset(out_surface, 0, sizeof(*out_surface));
    if (!data || !out || !out_written) {
        return NEXUS_V1_BPK_EXTRACT_ERR_NULL;
    }
    if (nexus_v1_bpk_archive_get_entry(data, data_size, index,
                                       &entry) != 0 ||
        nexus_v1_bpk_archive_get_entry_prefix(data, data_size, index,
                                              &prefix) != 0) {
        return NEXUS_V1_BPK_EXTRACT_ERR_ARCHIVE;
    }
    if (entry.has_prs3) {
        return NEXUS_V1_BPK_EXTRACT_ERR_PRS3;
    }
    if (!prefix.prefix_complete ||
        prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
        return NEXUS_V1_BPK_EXTRACT_ERR_NOT_SURFACE;
    }

    bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
    if (bpp == 0U) {
        return NEXUS_V1_BPK_EXTRACT_ERR_NOT_SURFACE;
    }
    {
        uint64_t rowstride64 = (uint64_t)prefix.width * bpp;
        uint64_t surface64 = rowstride64 * (uint64_t)prefix.height;
        if (rowstride64 > UINT32_MAX || surface64 > UINT32_MAX) {
            return NEXUS_V1_BPK_EXTRACT_ERR_ARCHIVE;
        }
        rowstride = (uint32_t)rowstride64;
        surface_bytes = (uint32_t)surface64;
    }
    if ((uint64_t)entry.payload_size < (uint64_t)surface_bytes) {
        return NEXUS_V1_BPK_EXTRACT_ERR_TRUNCATED;
    }
    if ((size_t)surface_bytes > out_size) {
        return NEXUS_V1_BPK_EXTRACT_ERR_OUTPUT_TOO_SMALL;
    }

    memcpy(out, data + entry.payload_offset, surface_bytes);
    if (out_surface) {
        out_surface->entry_index = index;
        out_surface->mode = prefix.mode;
        out_surface->width = prefix.width;
        out_surface->height = prefix.height;
        out_surface->pixel_count =
            (uint32_t)prefix.width * (uint32_t)prefix.height;
        out_surface->layout.bpp = bpp;
        out_surface->layout.rowstride = rowstride;
        out_surface->layout.surface_bytes = surface_bytes;
        out_surface->layout.surface_class =
            nexus_v1_bpk_mode_to_surface_class(prefix.mode);
    }
    *out_written = surface_bytes;
    return NEXUS_V1_BPK_EXTRACT_OK;
}

static int prs3_read_bit(const uint8_t *data, size_t size, size_t *cursor,
                         uint8_t *control, unsigned int *bits_left,
                         int *out_bit) {
    if (*bits_left == 0U) {
        if (*cursor >= size) return 0;
        *control = data[(*cursor)++];
        *bits_left = 8U;
    }
    *out_bit = *control & 1U;
    *control >>= 1U;
    --*bits_left;
    return 1;
}

static int prs3_decode_body(const uint8_t *data, size_t size,
                            uint8_t *out, size_t output_size,
                            size_t *out_written) {
    size_t cursor = 0U;
    size_t written = 0U;
    uint8_t control = 0U;
    unsigned int bits_left = 0U;

    while (written < output_size) {
        int flag;
        if (!prs3_read_bit(data, size, &cursor, &control, &bits_left, &flag))
            return 0;
        if (flag) {
            if (cursor >= size) return 0;
            out[written++] = data[cursor++];
        } else {
            int length;
            int raw_offset;
            int absolute_offset;
            int copy_index;
            uint8_t byte0;
            uint8_t byte1;
            if (cursor + 2U > size) return 0;
            byte0 = data[cursor];
            byte1 = data[cursor + 1U];
            cursor += 2U;
            length = 3 + (int)(byte1 & 0x0fU);
            raw_offset = (int)byte0 | (((int)byte1 & 0xf0) << 4);
            if (raw_offset >= 0xfdc) {
                absolute_offset = raw_offset - 0xfee;
            } else {
                absolute_offset = raw_offset + 18;
            }
            while ((int)written - absolute_offset > 4095) {
                absolute_offset += 4096;
            }
            if ((size_t)length > output_size - written) return 0;
            for (copy_index = 0; copy_index < length; ++copy_index) {
                if (absolute_offset < 0) {
                    out[written] = 0U;
                } else {
                    if ((size_t)absolute_offset >= written) return 0;
                    out[written] = out[(size_t)absolute_offset];
                }
                ++written;
                ++absolute_offset;
            }
        }
    }
    *out_written = written;
    return 1;
}

int nexus_v1_bpk_archive_decode_surface(
    const uint8_t *data, size_t data_size, uint32_t index, uint8_t *out,
    size_t out_size, Nexus_V1_BpkSurfaceEntry *out_surface,
    size_t *out_written) {
    Nexus_V1_BpkEntry entry;
    Nexus_V1_BpkEntryPrefix prefix;
    Nexus_V1_BpkPrs3Info prs3;
    uint32_t bpp;
    size_t expected;

    if (out_written) *out_written = 0U;
    if (out_surface) memset(out_surface, 0, sizeof(*out_surface));
    if (!data || !out || !out_written) return NEXUS_V1_BPK_DECODE_ERR_NULL;
    if (nexus_v1_bpk_archive_get_entry(data, data_size, index, &entry) != 0 ||
        nexus_v1_bpk_archive_get_entry_prefix(data, data_size, index, &prefix) != 0)
        return NEXUS_V1_BPK_DECODE_ERR_ARCHIVE;
    bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
    if (!prefix.prefix_complete || bpp == 0U) return NEXUS_V1_BPK_DECODE_ERR_NOT_SURFACE;

    if (!entry.has_prs3) {
        expected = (size_t)prefix.width * (size_t)prefix.height * bpp;
        if (expected > out_size) return NEXUS_V1_BPK_DECODE_ERR_OUTPUT_TOO_SMALL;
        int rc = nexus_v1_bpk_archive_extract_stored_surface(data, data_size,
                                                               index, out, out_size,
                                                               out_surface, out_written);
        return rc == NEXUS_V1_BPK_EXTRACT_OK ? NEXUS_V1_BPK_DECODE_OK :
            (rc == NEXUS_V1_BPK_EXTRACT_ERR_TRUNCATED ?
             NEXUS_V1_BPK_DECODE_ERR_TRUNCATED : NEXUS_V1_BPK_DECODE_ERR_ARCHIVE);
    }
    if (nexus_v1_bpk_archive_inspect_prs3(data, data_size, index, &prs3) != 0 ||
        !prs3.prs3_version_matches || !prs3.pixel_count_matches ||
        prs3.compressed_size == 0U ||
        entry.payload_size < NEXUS_V1_BPK_PRS3_HEADER_BYTES + 4U)
        return NEXUS_V1_BPK_DECODE_ERR_TRUNCATED;
    expected = prs3.prs3_pixel_count;
    if (expected > out_size) return NEXUS_V1_BPK_DECODE_ERR_OUTPUT_TOO_SMALL;
    if (!prs3_decode_body(data + entry.payload_offset + 12U,
                          prs3.compressed_size, out, expected, out_written))
        return NEXUS_V1_BPK_DECODE_ERR_STREAM;
    if (out_surface) {
        out_surface->entry_index = index;
        out_surface->mode = prefix.mode;
        out_surface->width = prefix.width;
        out_surface->height = prefix.height;
        out_surface->pixel_count = (uint32_t)prefix.width * prefix.height;
        out_surface->layout.bpp = 1U;
        out_surface->layout.rowstride = (uint32_t)prefix.width;
        out_surface->layout.surface_bytes = (uint32_t)expected;
        out_surface->layout.surface_class =
            NEXUS_V1_BPK_SURFACE_INDEXED_8BPP;
    }
    return NEXUS_V1_BPK_DECODE_OK;
}

const char *nexus_v1_bpk_surface_decode_status_name(int status) {
    switch (status) {
    case NEXUS_V1_BPK_DECODE_OK: return "ok";
    case NEXUS_V1_BPK_DECODE_ERR_NULL: return "null";
    case NEXUS_V1_BPK_DECODE_ERR_ARCHIVE: return "archive";
    case NEXUS_V1_BPK_DECODE_ERR_NOT_SURFACE: return "not-surface";
    case NEXUS_V1_BPK_DECODE_ERR_OUTPUT_TOO_SMALL: return "output-too-small";
    case NEXUS_V1_BPK_DECODE_ERR_TRUNCATED: return "truncated";
    case NEXUS_V1_BPK_DECODE_ERR_STREAM: return "stream";
    default: return "unknown";
    }
}

static uint32_t bpk_rgb565_to_rgba(uint16_t value) {
    uint32_t r = (uint32_t)((value >> 11) & 0x1fU);
    uint32_t g = (uint32_t)((value >> 5) & 0x3fU);
    uint32_t b = (uint32_t)(value & 0x1fU);
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    return 0xff000000U | (r << 16) | (g << 8) | b;
}

static int bpk_truecolor_pixel_rgba(const uint8_t *pixels,
                                    uint32_t bpp,
                                    size_t pixel_index,
                                    uint32_t *out_rgba) {
    const uint8_t *p;
    if (!pixels || !out_rgba || bpp < 2U || bpp > 4U) return 0;
    p = pixels + pixel_index * (size_t)bpp;
    if (bpp == 2U) {
        uint16_t value = (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
        *out_rgba = bpk_rgb565_to_rgba(value);
    } else if (bpp == 3U) {
        *out_rgba = 0xff000000U | ((uint32_t)p[0] << 16) |
                    ((uint32_t)p[1] << 8) | (uint32_t)p[2];
    } else {
        *out_rgba = ((uint32_t)p[3] << 24) | ((uint32_t)p[0] << 16) |
                    ((uint32_t)p[1] << 8) | (uint32_t)p[2];
    }
    return 1;
}

static int bpk_material_surface_from_truecolor(
    const uint8_t *pixels,
    const Nexus_V1_BpkSurfaceEntry *surface,
    Nexus_DMDFTextureSurface *out_surface) {
    size_t pixel_count;
    size_t i;
    uint32_t bpp;
    uint8_t *indices;
    uint32_t palette[256];
    uint32_t palette_count = 1U;

    if (!pixels || !surface || !out_surface ||
        surface->width == 0U || surface->height == 0U) return 0;
    bpp = surface->layout.bpp;
    if (bpp < 2U || bpp > 4U) return 0;
    pixel_count = (size_t)surface->width * (size_t)surface->height;
    indices = (uint8_t *)malloc(pixel_count);
    if (!indices) return 0;
    memset(palette, 0, sizeof(palette));
    for (i = 0; i < pixel_count; ++i) {
        uint32_t rgba = 0U;
        uint32_t slot;
        int found = 0;
        if (!bpk_truecolor_pixel_rgba(pixels, bpp, i, &rgba)) {
            free(indices);
            return 0;
        }
        if ((rgba >> 24) == 0U) {
            indices[i] = 0U;
            continue;
        }
        for (slot = 1U; slot < palette_count; ++slot) {
            if (palette[slot] == rgba) {
                indices[i] = (uint8_t)slot;
                found = 1;
                break;
            }
        }
        if (!found) {
            if (palette_count >= 256U) {
                free(indices);
                return 0;
            }
            palette[palette_count] = rgba;
            indices[i] = (uint8_t)palette_count;
            ++palette_count;
        }
    }

    out_surface->pixels = indices;
    out_surface->width = (int)surface->width;
    out_surface->height = (int)surface->height;
    memcpy(out_surface->palette, palette, sizeof(out_surface->palette));
    out_surface->valid = 1;
    return 1;
}

int nexus_v1_dmdf_import_bpk_material_bank(const uint8_t *data,
                                           size_t data_size,
                                           Nexus_DMDFMaterialBank *out) {
    Nexus_V1_BpkArchiveInfo archive;
    uint32_t index;
    int imported = 0;

    if (!data || !out || nexus_v1_bpk_archive_parse(data, data_size, &archive) != 0)
        return 0;
    for (index = 0U; index < archive.candidate_offset_count &&
                    index < NEXUS_DMDF_MATERIAL_COUNT; ++index) {
        Nexus_V1_BpkEntryPrefix prefix;
        Nexus_V1_BpkEntry entry;
        Nexus_V1_BpkSurfaceEntry surface;
        uint8_t *pixels;
        size_t written = 0U;
        size_t bytes;
        uint32_t bpp;

        if (out->surfaces[index].valid ||
            nexus_v1_bpk_archive_get_entry(data, data_size, index, &entry) != 0 ||
            nexus_v1_bpk_archive_get_entry_prefix(data, data_size, index, &prefix) != 0 ||
            prefix.width == 0U || prefix.height == 0U) continue;
        bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
        if (bpp == 0U) continue;
        bytes = (size_t)prefix.width * (size_t)prefix.height *
                (entry.has_prs3 ? 1U : (size_t)bpp);
        if (bytes == 0U) continue;
        pixels = (uint8_t *)malloc(bytes);
        if (!pixels) break;
        if (nexus_v1_bpk_archive_decode_surface(data, data_size, index, pixels,
                                                bytes, &surface, &written) !=
                NEXUS_V1_BPK_DECODE_OK || written != bytes) {
            free(pixels);
            continue;
        }
        bpp = surface.layout.bpp;
        if (bpp == 1U) {
            int palette_source = -1;
            int candidate;
            /* BPK 8bpp indices need a real CLUT already supplied by DMDF.
             * Do not synthesize colours for an otherwise opaque Saturn surface. */
            for (candidate = 0; candidate < NEXUS_DMDF_MATERIAL_COUNT; ++candidate) {
                if (out->surfaces[candidate].valid) {
                    palette_source = candidate;
                    break;
                }
            }
            if (palette_source < 0) { free(pixels); continue; }
            memcpy(out->surfaces[index].palette,
                   out->surfaces[palette_source].palette,
                   sizeof(out->surfaces[index].palette));
            out->surfaces[index].pixels = pixels;
            out->surfaces[index].width = (int)surface.width;
            out->surfaces[index].height = (int)surface.height;
            out->surfaces[index].valid = 1;
        } else if (!bpk_material_surface_from_truecolor(
                       pixels, &surface, &out->surfaces[index])) {
            free(pixels);
            continue;
        } else {
            free(pixels);
        }
        ++out->surface_count;
        ++out->bpk_imported_surface_count;
        if (bpp == 1U) {
            ++out->bpk_indexed_surface_count;
        } else {
            ++out->bpk_truecolor_surface_count;
        }
        if (entry.has_prs3) {
            ++out->bpk_prs3_surface_count;
        }
        ++imported;
    }
    if (imported > 0) out->valid = 1;
    return imported;
}

const char *nexus_v1_bpk_surface_extract_status_name(int status) {
    switch (status) {
    case NEXUS_V1_BPK_EXTRACT_OK: return "ok";
    case NEXUS_V1_BPK_EXTRACT_ERR_NULL: return "null";
    case NEXUS_V1_BPK_EXTRACT_ERR_ARCHIVE: return "archive";
    case NEXUS_V1_BPK_EXTRACT_ERR_PRS3: return "prs3";
    case NEXUS_V1_BPK_EXTRACT_ERR_NOT_SURFACE: return "not-surface";
    case NEXUS_V1_BPK_EXTRACT_ERR_OUTPUT_TOO_SMALL:
        return "output-too-small";
    case NEXUS_V1_BPK_EXTRACT_ERR_TRUNCATED: return "truncated";
    default: return "unknown";
    }
}

int nexus_v1_bpk_archive_runtime_surface_handoff(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkRuntimeSurfaceHandoff *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkRuntimeSurfaceHandoffSummary *out_summary) {
    uint32_t count;
    uint32_t used = 0U;
    uint32_t surface_entries = 0U;
    uint32_t ready_stored = 0U;
    uint32_t blocked_prs3 = 0U;
    uint32_t blocked_truncated = 0U;
    uint32_t trailer_skipped = 0U;
    uint32_t unknown_skipped = 0U;
    uint64_t expected_surface_bytes = 0U;
    uint64_t extractable_surface_bytes = 0U;
    int truncated = 0;

    if (!out_summary) return -1;
    memset(out_summary, 0, sizeof(*out_summary));
    out_summary->capacity = entry_capacity;
    if (out_entries && entry_capacity > 0U) {
        memset(out_entries, 0,
               (size_t)entry_capacity * sizeof(out_entries[0]));
    }

    if (read_header(data, data_size, &count) != 0) return -1;

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntry entry;
        Nexus_V1_BpkEntryPrefix prefix;
        Nexus_V1_BpkRuntimeSurfaceHandoff row;
        uint32_t bpp;
        uint64_t rowstride64;
        uint64_t surface64;

        if (nexus_v1_bpk_archive_get_entry(data, data_size, i,
                                           &entry) != 0 ||
            nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            return -1;
        }
        if (!prefix.prefix_complete) continue;
        if (prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
            ++trailer_skipped;
            continue;
        }

        bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
        if (bpp == 0U) {
            ++unknown_skipped;
            continue;
        }

        rowstride64 = (uint64_t)prefix.width * bpp;
        surface64 = rowstride64 * (uint64_t)prefix.height;
        if (rowstride64 > UINT32_MAX || surface64 > UINT32_MAX) {
            return -1;
        }

        memset(&row, 0, sizeof(row));
        row.entry_index = i;
        row.payload_offset = entry.payload_offset;
        row.payload_size = entry.payload_size;
        row.surface.entry_index = i;
        row.surface.mode = prefix.mode;
        row.surface.width = prefix.width;
        row.surface.height = prefix.height;
        row.surface.pixel_count =
            (uint32_t)prefix.width * (uint32_t)prefix.height;
        row.surface.layout.bpp = bpp;
        row.surface.layout.rowstride = (uint32_t)rowstride64;
        row.surface.layout.surface_bytes = (uint32_t)surface64;
        row.surface.layout.surface_class =
            nexus_v1_bpk_mode_to_surface_class(prefix.mode);

        ++surface_entries;
        expected_surface_bytes += surface64;

        if (entry.has_prs3) {
            row.status = NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_PRS3;
            ++blocked_prs3;
        } else if ((uint64_t)entry.payload_size < surface64) {
            row.status = NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_TRUNCATED;
            ++blocked_truncated;
        } else {
            row.status = NEXUS_V1_BPK_SURFACE_HANDOFF_READY_STORED;
            row.extractable = 1;
            ++ready_stored;
            extractable_surface_bytes += surface64;
        }

        if (out_entries && used < entry_capacity) {
            out_entries[used] = row;
        } else if (out_entries && used >= entry_capacity) {
            truncated = 1;
        }
        ++used;
    }

    out_summary->archive_entries = count;
    out_summary->surface_entries = surface_entries;
    out_summary->ready_stored_surfaces = ready_stored;
    out_summary->blocked_prs3_surfaces = blocked_prs3;
    out_summary->blocked_truncated_surfaces = blocked_truncated;
    out_summary->trailer_skipped = trailer_skipped;
    out_summary->unknown_skipped = unknown_skipped;
    out_summary->expected_surface_bytes = expected_surface_bytes;
    out_summary->extractable_surface_bytes = extractable_surface_bytes;
    out_summary->used = used;
    out_summary->requires_prs3_decoder = (blocked_prs3 > 0U) ? 1 : 0;
    out_summary->truncated = truncated;
    return 0;
}

const char *nexus_v1_bpk_surface_handoff_status_name(
    Nexus_V1_BpkSurfaceHandoffStatus status) {
    switch (status) {
    case NEXUS_V1_BPK_SURFACE_HANDOFF_INVALID: return "invalid";
    case NEXUS_V1_BPK_SURFACE_HANDOFF_READY_STORED:
        return "ready-stored";
    case NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_PRS3:
        return "blocked-prs3";
    case NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_TRUNCATED:
        return "blocked-truncated";
    default: return "unknown";
    }
}

int nexus_v1_bpk_archive_prs3_stream_plan(
    const uint8_t *data,
    size_t data_size,
    uint32_t index,
    Nexus_V1_BpkPrs3StreamPlan *out_plan) {
    Nexus_V1_BpkEntry entry;
    Nexus_V1_BpkEntryPrefix prefix;
    Nexus_V1_BpkPrs3Info prs3;
    uint32_t bpp;
    uint32_t stream_offset;
    uint32_t stream_size;
    uint64_t output_bytes;

    if (!out_plan) return NEXUS_V1_BPK_PRS3_STREAM_ERR_NULL;
    memset(out_plan, 0, sizeof(*out_plan));
    if (!data) return NEXUS_V1_BPK_PRS3_STREAM_ERR_NULL;

    if (nexus_v1_bpk_archive_get_entry(data, data_size, index,
                                       &entry) != 0 ||
        nexus_v1_bpk_archive_get_entry_prefix(data, data_size, index,
                                              &prefix) != 0 ||
        nexus_v1_bpk_archive_inspect_prs3(data, data_size, index,
                                          &prs3) != 0) {
        return NEXUS_V1_BPK_PRS3_STREAM_ERR_ARCHIVE;
    }
    if (!entry.has_prs3 || !prs3.has_prs3) {
        return NEXUS_V1_BPK_PRS3_STREAM_ERR_NOT_PRS3;
    }
    bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
    if (bpp == 0U) {
        return NEXUS_V1_BPK_PRS3_STREAM_ERR_UNSUPPORTED_MODE;
    }

    stream_offset = entry.offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
                    NEXUS_V1_BPK_PRS3_HEADER_BYTES;
    if (stream_offset > entry.next_offset) {
        return NEXUS_V1_BPK_PRS3_STREAM_ERR_TRUNCATED;
    }
    stream_size = entry.next_offset - stream_offset;
    if (stream_size < 4U) {
        return NEXUS_V1_BPK_PRS3_STREAM_ERR_TRUNCATED;
    }

    output_bytes = (uint64_t)prefix.width * (uint64_t)prefix.height * bpp;
    if (output_bytes > UINT32_MAX) {
        return NEXUS_V1_BPK_PRS3_STREAM_ERR_ARCHIVE;
    }

    out_plan->entry_index = index;
    out_plan->mode = prefix.mode;
    out_plan->width = prefix.width;
    out_plan->height = prefix.height;
    out_plan->bpp = bpp;
    out_plan->pixel_count = (uint32_t)prefix.width * (uint32_t)prefix.height;
    out_plan->expected_output_bytes = (uint32_t)output_bytes;
    out_plan->stream_offset = stream_offset;
    out_plan->stream_size = stream_size;
    out_plan->body_offset = stream_offset + 4U;
    out_plan->body_size = stream_size - 4U;
    out_plan->header_first_u32 = rd32_be(data + stream_offset);
    out_plan->header_first_readable = 1;
    if (out_plan->header_first_u32 >= stream_size) {
        out_plan->header_minus_payload =
            out_plan->header_first_u32 - stream_size;
        out_plan->bounded_header_candidate =
            (out_plan->header_minus_payload <= 16U) ? 1 : 0;
    } else {
        out_plan->header_underflow = 1;
        out_plan->header_minus_payload = UINT32_MAX;
    }
    out_plan->decode_blocked = 1;
    return NEXUS_V1_BPK_PRS3_STREAM_OK;
}

const char *nexus_v1_bpk_prs3_stream_status_name(int status) {
    switch (status) {
    case NEXUS_V1_BPK_PRS3_STREAM_OK: return "ok";
    case NEXUS_V1_BPK_PRS3_STREAM_ERR_NULL: return "null";
    case NEXUS_V1_BPK_PRS3_STREAM_ERR_ARCHIVE: return "archive";
    case NEXUS_V1_BPK_PRS3_STREAM_ERR_NOT_PRS3: return "not-prs3";
    case NEXUS_V1_BPK_PRS3_STREAM_ERR_UNSUPPORTED_MODE:
        return "unsupported-mode";
    case NEXUS_V1_BPK_PRS3_STREAM_ERR_TRUNCATED: return "truncated";
    default: return "unknown";
    }
}

int nexus_v1_bpk_archive_runtime_upload_plan(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkRuntimeUploadRow *out_rows,
    uint32_t row_capacity,
    Nexus_V1_BpkRuntimeUploadReceipt *out_receipt) {
    Nexus_V1_BpkRuntimeSurfaceHandoffSummary handoff;
    uint32_t count;
    uint32_t planned = 0U;
    int truncated = 0;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->route = NEXUS_V1_BPK_UPLOAD_ROUTE_INVALID;
    out_receipt->capacity = row_capacity;
    out_receipt->fallback_visuals_permitted = 0;
    if (out_rows && row_capacity > 0U) {
        memset(out_rows, 0, (size_t)row_capacity * sizeof(out_rows[0]));
    }

    if (nexus_v1_bpk_archive_runtime_surface_handoff(
            data, data_size, NULL, 0U, &handoff) != 0 ||
        read_header(data, data_size, &count) != 0) {
        return -1;
    }

    out_receipt->archive_entries = handoff.archive_entries;
    out_receipt->surface_entries = handoff.surface_entries;

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntry entry;
        Nexus_V1_BpkEntryPrefix prefix;
        Nexus_V1_BpkRuntimeUploadRow row;
        uint32_t bpp;
        uint64_t expected;

        if (nexus_v1_bpk_archive_get_entry(data, data_size, i, &entry) != 0 ||
            nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            out_receipt->route = NEXUS_V1_BPK_UPLOAD_ROUTE_INVALID;
            return -1;
        }
        if (!prefix.prefix_complete ||
            prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
            continue;
        }
        bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
        if (bpp == 0U) continue;
        expected = (uint64_t)prefix.width * (uint64_t)prefix.height * bpp;

        memset(&row, 0, sizeof(row));
        row.entry_index = i;
        row.surface_class = nexus_v1_bpk_mode_to_surface_class(prefix.mode);
        row.mode = prefix.mode;
        row.width = prefix.width;
        row.height = prefix.height;
        row.bpp = bpp;
        row.payload_offset = entry.payload_offset;
        row.payload_size = entry.payload_size;

        if (entry.has_prs3) {
            Nexus_V1_BpkPrs3StreamPlan plan;
            Nexus_V1_BpkSurfaceEntry decoded_surface;
            uint8_t *pixels;
            size_t written = 0U;
            int rc;
            rc = nexus_v1_bpk_archive_prs3_stream_plan(
                data, data_size, i, &plan);
            if (rc == NEXUS_V1_BPK_PRS3_STREAM_OK) {
                row.stream_offset = plan.stream_offset;
                row.stream_size = plan.stream_size;
                row.body_offset = plan.body_offset;
                row.body_size = plan.body_size;
                row.header_first_u32 = plan.header_first_u32;
                row.header_minus_payload = plan.header_minus_payload;
            } else {
                row.header_minus_payload = UINT32_MAX;
            }
            expected = (uint64_t)prefix.width * (uint64_t)prefix.height;
            if (expected > UINT32_MAX) {
                out_receipt->route = NEXUS_V1_BPK_UPLOAD_ROUTE_INVALID;
                return -1;
            }
            pixels = (uint8_t *)malloc((size_t)expected);
            if (pixels) {
                rc = nexus_v1_bpk_archive_decode_surface(
                    data, data_size, i, pixels, (size_t)expected,
                    &decoded_surface, &written);
            } else {
                rc = NEXUS_V1_BPK_DECODE_ERR_OUTPUT_TOO_SMALL;
            }
            if (rc == NEXUS_V1_BPK_DECODE_OK && written == (size_t)expected) {
                row.status = NEXUS_V1_BPK_SURFACE_HANDOFF_READY_STORED;
                row.surface_class = decoded_surface.layout.surface_class;
                row.bpp = decoded_surface.layout.bpp;
                row.expected_output_bytes = (uint32_t)written;
                row.upload_ready = 1;
                ++out_receipt->ready_uploads;
                out_receipt->expected_upload_bytes += written;
                out_receipt->extractable_upload_bytes += written;
            } else {
                row.status = NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_PRS3;
                row.decode_blocked = 1;
                row.expected_output_bytes = (uint32_t)expected;
                ++out_receipt->blocked_prs3_uploads;
                out_receipt->expected_upload_bytes += expected;
            }
            free(pixels);
        } else if ((uint64_t)entry.payload_size < expected) {
            row.status = NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_TRUNCATED;
            row.expected_output_bytes = (uint32_t)expected;
            ++out_receipt->blocked_truncated_uploads;
            out_receipt->expected_upload_bytes += expected;
        } else {
            row.status = NEXUS_V1_BPK_SURFACE_HANDOFF_READY_STORED;
            row.expected_output_bytes = (uint32_t)expected;
            row.upload_ready = 1;
            ++out_receipt->ready_uploads;
            out_receipt->expected_upload_bytes += expected;
            out_receipt->extractable_upload_bytes += expected;
        }

        if (out_rows && planned < row_capacity) {
            out_rows[planned] = row;
        } else if (out_rows && planned >= row_capacity) {
            truncated = 1;
        }
        ++planned;
    }

    out_receipt->planned_rows = planned;
    out_receipt->truncated = truncated;
    if (out_receipt->surface_entries == 0U) {
        out_receipt->route = NEXUS_V1_BPK_UPLOAD_ROUTE_NO_SURFACES;
    } else if (out_receipt->blocked_prs3_uploads > 0U) {
        out_receipt->route = NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3;
    } else if (out_receipt->blocked_truncated_uploads > 0U) {
        out_receipt->route = NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_TRUNCATED;
    } else if (out_receipt->ready_uploads > handoff.ready_stored_surfaces) {
        out_receipt->route = NEXUS_V1_BPK_UPLOAD_ROUTE_READY_DECODED;
    } else {
        out_receipt->route = NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED;
    }
    out_receipt->blocks_real_menu_surface_render =
        (out_receipt->route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3 ||
         out_receipt->route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_TRUNCATED)
            ? 1 : 0;
    out_receipt->fallback_visuals_permitted =
        out_receipt->blocks_real_menu_surface_render ? 0 : 1;
    return 0;
}

const char *nexus_v1_bpk_runtime_upload_route_name(
    Nexus_V1_BpkRuntimeUploadRoute route) {
    switch (route) {
    case NEXUS_V1_BPK_UPLOAD_ROUTE_INVALID: return "invalid";
    case NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED: return "ready-stored";
    case NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3: return "blocked-prs3";
    case NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_TRUNCATED:
        return "blocked-truncated";
    case NEXUS_V1_BPK_UPLOAD_ROUTE_NO_SURFACES: return "no-surfaces";
    case NEXUS_V1_BPK_UPLOAD_ROUTE_READY_DECODED: return "ready-decoded";
    default: return "unknown";
    }
}

int nexus_v1_bpk_archive_runtime_decode_receipt(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkRuntimeDecodeReceipt *out_receipt) {
    Nexus_V1_BpkRuntimeSurfaceHandoffSummary summary;
    uint32_t count;
    int have_first_blocked = 0;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->first_blocked_entry = UINT32_MAX;

    if (nexus_v1_bpk_archive_runtime_surface_handoff(
            data, data_size, NULL, 0U, &summary) != 0) {
        out_receipt->route = NEXUS_V1_BPK_DECODE_ROUTE_INVALID;
        return -1;
    }
    if (read_header(data, data_size, &count) != 0) {
        out_receipt->route = NEXUS_V1_BPK_DECODE_ROUTE_INVALID;
        return -1;
    }

    out_receipt->archive_entries = summary.archive_entries;
    out_receipt->surface_entries = summary.surface_entries;
    out_receipt->ready_stored_surfaces = summary.ready_stored_surfaces;
    out_receipt->blocked_prs3_surfaces = summary.blocked_prs3_surfaces;
    out_receipt->blocked_truncated_surfaces =
        summary.blocked_truncated_surfaces;
    out_receipt->requires_prs3_decoder = summary.requires_prs3_decoder;

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntry entry;
        Nexus_V1_BpkEntryPrefix prefix;
        uint32_t bpp;
        if (nexus_v1_bpk_archive_get_entry(data, data_size, i,
                                           &entry) != 0 ||
            nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            out_receipt->route = NEXUS_V1_BPK_DECODE_ROUTE_INVALID;
            return -1;
        }
        if (!entry.has_prs3 || !prefix.prefix_complete) continue;
        bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
        if (bpp == 0U) continue;

        {
            Nexus_V1_BpkPrs3StreamPlan plan;
            int rc = nexus_v1_bpk_archive_prs3_stream_plan(
                data, data_size, i, &plan);
            if (rc != NEXUS_V1_BPK_PRS3_STREAM_OK) {
                ++out_receipt->prs3_stream_plan_failures;
                if (!have_first_blocked) {
                    out_receipt->first_blocked_entry = i;
                    out_receipt->first_blocked_stream_offset = 0U;
                    out_receipt->first_blocked_stream_size = 0U;
                    out_receipt->first_blocked_expected_output_bytes =
                        (uint32_t)prefix.width * (uint32_t)prefix.height *
                        bpp;
                    out_receipt->first_blocked_header_first_u32 = 0U;
                    out_receipt->first_blocked_header_minus_payload =
                        UINT32_MAX;
                    have_first_blocked = 1;
                }
                continue;
            }
            ++out_receipt->prs3_stream_plans;
            if (plan.bounded_header_candidate) {
                ++out_receipt->prs3_bounded_header_candidates;
            }
            if (plan.header_underflow) {
                ++out_receipt->prs3_header_underflows;
            }
        }
        {
            Nexus_V1_BpkSurfaceEntry surface;
            uint8_t *pixels;
            size_t written = 0U;
            size_t expected =
                (size_t)prefix.width * (size_t)prefix.height;
            int rc;

            ++out_receipt->prs3_decode_attempts;
            pixels = (uint8_t *)malloc(expected);
            if (!pixels) {
                rc = NEXUS_V1_BPK_DECODE_ERR_OUTPUT_TOO_SMALL;
            } else {
                rc = nexus_v1_bpk_archive_decode_surface(
                    data, data_size, i, pixels, expected, &surface, &written);
            }
            if (rc == NEXUS_V1_BPK_DECODE_OK && written == expected) {
                ++out_receipt->prs3_decode_successes;
                out_receipt->prs3_decoded_surface_bytes += written;
            } else {
                ++out_receipt->prs3_decode_failures;
                if (!have_first_blocked) {
                    out_receipt->first_blocked_entry = i;
                    out_receipt->first_blocked_stream_offset =
                        entry.payload_offset + NEXUS_V1_BPK_PRS3_HEADER_BYTES;
                    out_receipt->first_blocked_stream_size =
                        entry.payload_size > NEXUS_V1_BPK_PRS3_HEADER_BYTES
                            ? entry.payload_size -
                                  NEXUS_V1_BPK_PRS3_HEADER_BYTES
                            : 0U;
                    out_receipt->first_blocked_expected_output_bytes =
                        (uint32_t)expected;
                    have_first_blocked = 1;
                }
                if (out_receipt->first_blocked_entry == i &&
                    out_receipt->first_blocked_decode_status == 0) {
                    out_receipt->first_blocked_decode_status = rc;
                }
            }
            free(pixels);
        }
    }

    if (out_receipt->surface_entries == 0U) {
        out_receipt->route = NEXUS_V1_BPK_DECODE_ROUTE_NO_SURFACES;
    } else if (out_receipt->prs3_decode_failures > 0U ||
               (out_receipt->blocked_prs3_surfaces > 0U &&
                out_receipt->prs3_decode_successes <
                    out_receipt->blocked_prs3_surfaces)) {
        out_receipt->route = NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3;
    } else if (out_receipt->blocked_truncated_surfaces > 0U) {
        out_receipt->route = NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED;
    } else if (out_receipt->prs3_decode_successes > 0U) {
        out_receipt->route = NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED;
    } else {
        out_receipt->route = NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED;
    }
    out_receipt->decode_blocked =
        (out_receipt->route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3 ||
         out_receipt->route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED)
            ? 1 : 0;
    return 0;
}

const char *nexus_v1_bpk_runtime_decode_route_name(
    Nexus_V1_BpkRuntimeDecodeRoute route) {
    switch (route) {
    case NEXUS_V1_BPK_DECODE_ROUTE_INVALID: return "invalid";
    case NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED: return "ready-stored";
    case NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3: return "blocked-prs3";
    case NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED:
        return "blocked-truncated";
    case NEXUS_V1_BPK_DECODE_ROUTE_NO_SURFACES: return "no-surfaces";
    case NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED: return "ready-decoded";
    default: return "unknown";
    }
}

/* pass1084 — bounded PRS3 compression evidence walker. Surfaces per-entry
 * structural receipts for the (still unknown) PRS3 stream format so we can
 * narrow down the algorithm family in subsequent passes. The walker is
 * intentionally incapable of "running" the compression: it reads the first
 * 4 bytes as a BE u32, copies the first 8 bytes verbatim, and tallies a
 * bounded frequency table. It never advances any conceptual read cursor
 * past the 8-byte receipt and never emits a decoded surface.
 *
 * pass1084b (later in the same walk) adds a bounded 4-quadrant byte-class
 * tally (bk >> 6) over the same sample. This is the cheapest structural
 * fingerprint that distinguishes "random / uniformly distributed" bytes
 * from "stream with length-class hints and repeated literal zeros", and
 * the real MENU.BPK distribution shows q0 (0x00..0x3F) dominating, which
 * is consistent with small header_minus_payload values (4..7) and the
 * short back-reference runs we hypothesise.
 *
 * Source-lock: this pass builds on the pass1082/pass1083 evidence
 *   ledger (the BPPK outer wrapper, BMPD directory, 20-byte entry prefix
 *   anchor, and the entry[0] directory-trailer contract). The algorithm
 *   itself is still cited as
 *     docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md:291-306
 *     (MENU.BPK packed, game-specific, no formal analysis documented)
 *   and
 *     docs/VERIFIED_HASHES.md:103
 *     (MENU.BPK size 89060 / sha256 ...da886636).
 */
int nexus_v1_bpk_archive_prs3_payload_evidence(
    const uint8_t *data,
    size_t data_size,
    uint32_t sample_size,
    Nexus_V1_BpkPrs3PayloadEvidence *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkPrs3PayloadEvidenceSummary *out_summary) {
    uint32_t count;
    uint32_t trailer_skip = 0U;
    uint32_t unknown_skip = 0U;
    uint32_t trailer_partial = 0U;
    uint32_t used = 0U;
    uint32_t truncated = 0U;
    uint64_t total_uncompressed = 0U;
    uint64_t total_payload = 0U;
    double sum_ratio = 0.0;
    double min_ratio = 0.0;
    double max_ratio = 0.0;
    int have_ratio = 0;
    uint32_t smallest_payload = UINT32_MAX;
    uint32_t largest_payload = 0U;

    if (!out_summary) return -1;
    memset(out_summary, 0, sizeof(*out_summary));
    out_summary->capacity = entry_capacity;
    out_summary->smallest_payload = UINT32_MAX;
    out_summary->largest_payload = 0U;
    out_summary->min_compression_ratio = 0.0;
    out_summary->max_compression_ratio = 0.0;

    if (sample_size > NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_SAMPLE_BYTES) {
        sample_size = NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_SAMPLE_BYTES;
    }

    if (read_header(data, data_size, &count) != 0) return -1;

    /* Pass 1: per-entry structural receipts.
     *
     * For every entry whose 20-byte prefix is complete AND whose prefix
     * mode is one of the four PRS3 pixel-mode tags (6/14/22/30), we
     * record (entry_index, dimensions, header_first_u32, first 8 bytes
     * verbatim, compression_ratio). The directory trailer (mode 10) and
     * unknown mode tags are skipped and counted so callers can verify
     * the 162 + 1 = 163 entry sum.
     *
     * Note on payload boundary: the lib's Nexus_V1_BpkEntry.payload_offset
     * starts immediately after the 4-byte PRS3 magic (entry.offset + 24),
     * which still includes the 8-byte PRS3 sub-header (version +
     * pixel_count). For the PRS3 algorithm-evidence receipt we want to
     * examine the bytes AFTER the full 12-byte PRS3 sub-header (i.e.
     * entry.offset + 32, matching inspect_prs3's `compressed_size`
     * measurement). All receipts below use that offset consistently. */
    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntryPrefix prefix;
        Nexus_V1_BpkEntry entry;
        uint32_t bpp;
        uint32_t uncompressed_size;
        uint32_t payload_size;
        uint32_t stream_offset;
        uint32_t copy;
        int has_first;
        uint32_t header_first = 0U;
        uint32_t header_minus = 0U;
        double r = 0.0;
        Nexus_V1_BpkPrs3PayloadEvidence *row = NULL;

        if (nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            return -1;
        }
        if (!prefix.prefix_complete) {
            /* Skip entries without the full 20-byte prefix; they have no
             * recognisable mode tag and could not produce evidence. */
            continue;
        }
        if (prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
            ++trailer_skip;
            continue;
        }
        bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
        if (bpp == 0U) {
            ++unknown_skip;
            continue;
        }
        if (nexus_v1_bpk_archive_get_entry(data, data_size, i, &entry) != 0) {
            return -1;
        }

        /* Pass1084 evidence uses the inspect_prs3 `compressed_size`
         * boundary: payload is bytes from (entry.offset + 32) until the
         * next entry's start. If the entry doesn't span past the full
         * 12-byte PRS3 sub-header the stream is "partial" and we still
         * surface payload_size == 0 without producing a negative count. */
        stream_offset = entry.offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
                        NEXUS_V1_BPK_PRS3_HEADER_BYTES;
        if (stream_offset <= entry.next_offset) {
            payload_size = entry.next_offset - stream_offset;
        } else {
            payload_size = 0U;
        }
        uncompressed_size = (uint32_t)prefix.width *
                            (uint32_t)prefix.height *
                            bpp;

        has_first = (payload_size >= 4U) ? 1 : 0;
        if (has_first) {
            header_first = rd32_be(data + stream_offset);
            header_minus = (header_first >= payload_size)
                               ? (header_first - payload_size)
                               : UINT32_MAX;
        }

        if (out_entries && used < entry_capacity) {
            row = &out_entries[used];
            memset(row, 0, sizeof(*row));
            row->entry_index = i;
            row->mode = prefix.mode;
            row->width = prefix.width;
            row->height = prefix.height;
            row->pixel_count = (uint32_t)prefix.width *
                               (uint32_t)prefix.height;
            row->bpp = bpp;
            row->uncompressed_size = uncompressed_size;
            row->payload_size = payload_size;
            row->payload_available = (payload_size > 0U) ? 1 : 0;
            row->header_first_u32 = header_first;
            row->header_first_readable = has_first;
            row->header_minus_payload = header_minus;
            if (uncompressed_size > 0U) {
                row->compression_ratio =
                    (double)payload_size / (double)uncompressed_size;
                r = row->compression_ratio;
            }
            if (payload_size > 0U) {
                copy = payload_size;
                if (copy > NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_FIRST_BYTES) {
                    copy = NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_FIRST_BYTES;
                }
                memcpy(row->first_payload,
                       data + stream_offset,
                       copy);
            }
        } else if (out_entries && used >= entry_capacity) {
            truncated = 1U;
        }

        if (uncompressed_size > 0U) {
            sum_ratio += r;
            if (!have_ratio) {
                min_ratio = r;
                max_ratio = r;
                have_ratio = 1;
            } else {
                if (r < min_ratio) min_ratio = r;
                if (r > max_ratio) max_ratio = r;
            }
        }

        ++used;
        ++out_summary->mode_count[prefix.mode];
        total_uncompressed += uncompressed_size;
        total_payload += payload_size;
        if (payload_size < smallest_payload) smallest_payload = payload_size;
        if (payload_size > largest_payload) largest_payload = payload_size;
        if (payload_size == 0U) ++trailer_partial;
    }

    out_summary->entries_seen = count;
    out_summary->trailer_skipped = trailer_skip;
    out_summary->unknown_skipped = unknown_skip;
    out_summary->trailer_partial = trailer_partial;
    out_summary->used = used;
    out_summary->smallest_payload = smallest_payload;
    out_summary->largest_payload = largest_payload;
    out_summary->total_uncompressed = total_uncompressed;
    out_summary->total_payload = total_payload;
    out_summary->mean_compression_ratio =
        (used > 0U) ? (sum_ratio / (double)used) : 0.0;
    out_summary->min_compression_ratio = have_ratio ? min_ratio : 0.0;
    out_summary->max_compression_ratio = have_ratio ? max_ratio : 0.0;
    out_summary->truncated = (int)truncated;

    /* Pass 2: bounded byte-frequency receipts for every row the caller
     * actually got back. Each sample is bounded to min(sample_size,
     * payload_size, MAX_SAMPLE_BYTES) so the walker is provably
     * O(sample_size * entry_capacity) and never reads past
     * MAX_SAMPLE_BYTES of any payload. We re-derive the archive
     * absolute offset of the payload via nexus_v1_bpk_archive_get_entry
     * so the inner loop is bounds-checked against data_size on every
     * byte. The byte window starts at entry.offset + 32 (pass1084
     * stream offset = after the 12-byte PRS3 sub-header), NOT at
     * entry.payload_offset (which is +24 in the lib's accounting). */
    if (out_entries && sample_size > 0U) {
        for (uint32_t i = 0; i < used && i < entry_capacity; ++i) {
            Nexus_V1_BpkPrs3PayloadEvidence *row = &out_entries[i];
            Nexus_V1_BpkEntry entry;
            uint32_t sample_n;
            uint32_t sample_offset;
            uint32_t histogram[256];
            uint32_t best_count = 0U;
            uint8_t best_byte = 0U;
            uint32_t distinct = 0U;
            uint32_t b;

            if (row->payload_size == 0U) continue;
            if (nexus_v1_bpk_archive_get_entry(data, data_size,
                                               row->entry_index,
                                               &entry) != 0) {
                continue;
            }
            sample_offset = entry.offset +
                           NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
                           NEXUS_V1_BPK_PRS3_HEADER_BYTES;
            sample_n = row->payload_size;
            if (sample_n > sample_size) sample_n = sample_size;

            memset(histogram, 0, sizeof(histogram));
            {
                /* pass1084b — bounded 4-quadrant byte-class tally over
                 * the same sample. Index = bk >> 6 maps each byte into
                 * [0..3] covering [0x00..0x3F], [0x40..0x7F],
                 * [0x80..0xBF], [0xC0..0xFF]. */
                uint32_t bc[4] = {0U, 0U, 0U, 0U};
                for (uint32_t k = 0; k < sample_n; ++k) {
                    uint8_t bk = data[sample_offset + k];
                    histogram[bk] += 1U;
                    bc[bk >> 6] += 1U;
                    if (histogram[bk] > best_count) {
                        best_count = histogram[bk];
                        best_byte = bk;
                    }
                }
                row->byte_class_count[0] = bc[0];
                row->byte_class_count[1] = bc[1];
                row->byte_class_count[2] = bc[2];
                row->byte_class_count[3] = bc[3];
            }
            for (b = 0; b < 256U; ++b) {
                if (histogram[b] != 0U) ++distinct;
            }
            row->sample_size_used = sample_n;
            row->most_common_byte = best_byte;
            row->most_common_byte_count = best_count;
            row->distinct_byte_values = distinct;
        }
    }

    return 0;
}
