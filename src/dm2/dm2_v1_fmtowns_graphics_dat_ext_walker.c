#include "dm2_v1_fmtowns_graphics_dat_ext_walker.h"

static uint16_t rd_u16_le(const uint8_t *p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t rd_u32_le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

dm2_v1_fmtowns_graphics_dat_ext_status_t
dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(
        const uint8_t *blob, size_t blob_size,
        dm2_v1_fmtowns_graphics_dat_ext_header_t *out) {
    if (!blob || !out) return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_BAD_ARGS;
    if (blob_size < 4)  return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_TOO_SMALL;
    uint16_t sig   = rd_u16_le(blob + 0);
    uint16_t count = rd_u16_le(blob + 2);
    if (sig != 0x8004u && sig != 0x8005u)
        return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_BAD_SIGNATURE;
    if (count == 0u) return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_TOO_SMALL;
    uint32_t hdr = 6u + (uint32_t)count * 2u;
    if ((uint64_t)hdr > (uint64_t)blob_size)
        return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_TOO_SMALL;
    out->signature      = sig;
    out->asset_count    = count;
    out->header_size    = hdr;
    out->payload_offset = hdr;
    out->payload_size   = (uint32_t)(blob_size - hdr);
    return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK;
}

dm2_v1_fmtowns_graphics_dat_ext_status_t
dm2_v1_fmtowns_graphics_dat_ext_get_record_pc34(
        const uint8_t *blob, size_t blob_size,
        uint16_t index,
        dm2_v1_fmtowns_graphics_dat_ext_record_t *out) {
    dm2_v1_fmtowns_graphics_dat_ext_header_t h;
    dm2_v1_fmtowns_graphics_dat_ext_status_t rc =
        dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(blob, blob_size, &h);
    if (rc != DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK) return rc;
    if (!out) return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_BAD_ARGS;
    if (index >= h.asset_count) return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_BAD_ARGS;
    uint32_t cursor = h.header_size;
    for (uint32_t i = 0; i <= (uint32_t)index; ++i) {
        uint32_t stored = i == 0u
            ? rd_u32_le(blob + 4u)
            : (uint32_t)rd_u16_le(blob + 8u + (i - 1u) * 2u);
        if ((uint64_t)cursor + stored > blob_size)
            return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OVERRUN;
        if (i == (uint32_t)index) {
            out->index          = (uint16_t)i;
            out->stored_size    = stored;
            out->aux            = 0u;
            out->payload_offset = cursor;
            out->is_directory   = i == 0u ? 1 : 0;
            return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK;
        }
        cursor += stored;
    }
    return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OVERRUN;
}

dm2_v1_fmtowns_graphics_dat_ext_status_t
dm2_v1_fmtowns_graphics_dat_ext_walk_pc34(
        const uint8_t *blob, size_t blob_size,
        dm2_v1_fmtowns_graphics_dat_ext_visitor visitor, void *user) {
    dm2_v1_fmtowns_graphics_dat_ext_header_t h;
    dm2_v1_fmtowns_graphics_dat_ext_status_t rc =
        dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(blob, blob_size, &h);
    if (rc != DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK) return rc;
    if (!visitor) return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_BAD_ARGS;
    uint32_t cursor = h.header_size;
    for (uint32_t i = 0; i < (uint32_t)h.asset_count; ++i) {
        uint32_t stored = i == 0u
            ? rd_u32_le(blob + 4u)
            : (uint32_t)rd_u16_le(blob + 8u + (i - 1u) * 2u);
        if ((uint64_t)cursor + stored > blob_size)
            return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OVERRUN;
        dm2_v1_fmtowns_graphics_dat_ext_record_t rec;
        rec.index          = (uint16_t)i;
        rec.stored_size    = stored;
        rec.aux            = 0u;
        rec.payload_offset = cursor;
        rec.is_directory   = i == 0u ? 1 : 0;
        int vc = visitor(user, &rec);
        if (vc != 0) return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK;
        cursor += stored;
    }
    return DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK;
}
