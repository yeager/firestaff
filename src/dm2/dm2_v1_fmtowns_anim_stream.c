#include "dm2_v1_fmtowns_anim_stream.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8u) | p[1]);
}

static int add_chunk(DM2_V1_FmtownsAnimStreamReceipt *receipt,
                     uint16_t tag)
{
    switch (tag) {
    case 0x414eu: ++receipt->an_count; break; /* AN */
    case 0x504cu: ++receipt->pl_count; break; /* PL */
    case 0x454eu: ++receipt->en_count; break; /* EN */
    case 0x444cu: ++receipt->dl_count; break; /* DL */
    case 0x5344u: ++receipt->sd_count; break; /* SD */
    case 0x4252u: ++receipt->br_count; break; /* BR */
    case 0x534fu: ++receipt->so_count; break; /* SO */
    case 0x444fu: ++receipt->do_count; break; /* DO */
    case 0x464fu: ++receipt->fo_count; break; /* FO */
    case 0x4e45u: ++receipt->ne_count; break; /* NE */
    case 0x424eu: ++receipt->bn_count; break; /* BN */
    default: return 0;
    }
    ++receipt->chunk_count;
    return 1;
}

int dm2_v1_fmtowns_anim_stream_parse(
    const uint8_t *data, size_t data_size,
    DM2_V1_FmtownsAnimStreamReceipt *out)
{
    DM2_V1_FmtownsAnimStreamReceipt receipt;
    size_t offset = 0u;

    memset(&receipt, 0, sizeof(receipt));
    if (!data || data_size < 6u || data_size > UINT32_MAX) {
        if (out) *out = receipt;
        return 0;
    }
    while (offset + 6u <= data_size) {
        const uint16_t tag = read_be16(data + offset);
        const size_t payload_size = read_be16(data + offset + 2u);
        const size_t record_size = 6u + payload_size;

        if (record_size > data_size - offset || !add_chunk(&receipt, tag)) {
            if (out) *out = receipt;
            return 0;
        }
        if (tag == 0x414eu) {
            /* AN: two reserved bytes, width, height, bit depth, trailer. */
            if (payload_size != 8u) {
                if (out) *out = receipt;
                return 0;
            }
            /* END has two identical AN phases.  Preserve the first header
             * as the stream canvas identity, but reject a later phase that
             * would silently change it. */
            if (receipt.an_count == 1u) {
                receipt.width = read_be16(data + offset + 6u);
                receipt.height = read_be16(data + offset + 8u);
                receipt.bit_depth = read_be16(data + offset + 10u);
                receipt.an_trailer = read_be16(data + offset + 12u);
            } else if (receipt.width != read_be16(data + offset + 6u) ||
                       receipt.height != read_be16(data + offset + 8u) ||
                       receipt.bit_depth != read_be16(data + offset + 10u) ||
                       receipt.an_trailer != read_be16(data + offset + 12u)) {
                if (out) *out = receipt;
                return 0;
            }
        }
        offset += record_size;
    }
    if (offset != data_size || receipt.an_count == 0u || receipt.do_count != 1u) {
        if (out) *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.byte_count = (uint32_t)data_size;
    if (out) *out = receipt;
    return 1;
}

int dm2_v1_fmtowns_anim_stream_is_hme242_swoosh(
    const DM2_V1_FmtownsAnimStreamReceipt *r)
{
    return r && r->valid && r->width == 0u && r->height == 0u &&
           r->bit_depth == 4u && r->an_trailer == 2u &&
           r->chunk_count == 22u && r->an_count == 1u && r->pl_count == 1u &&
           r->en_count == 1u && r->dl_count == 18u && r->do_count == 1u &&
           r->sd_count == 0u && r->br_count == 0u && r->so_count == 0u &&
           r->fo_count == 0u && r->ne_count == 0u && r->bn_count == 0u;
}

int dm2_v1_fmtowns_anim_stream_is_hme242_title(
    const DM2_V1_FmtownsAnimStreamReceipt *r)
{
    return r && r->valid && r->width == 320u && r->height == 200u &&
           r->bit_depth == 4u && r->an_trailer == 3u &&
           r->chunk_count == 235u && r->an_count == 1u && r->pl_count == 1u &&
           r->en_count == 1u && r->dl_count == 224u && r->sd_count == 1u &&
           r->br_count == 1u && r->so_count == 5u && r->do_count == 1u &&
           r->fo_count == 0u && r->ne_count == 0u && r->bn_count == 0u;
}

int dm2_v1_fmtowns_anim_stream_is_hme242_end(
    const DM2_V1_FmtownsAnimStreamReceipt *r)
{
    return r && r->valid && r->width == 320u && r->height == 200u &&
           r->bit_depth == 4u && r->an_trailer == 3u &&
           r->chunk_count == 401u && r->an_count == 2u && r->pl_count == 5u &&
           r->en_count == 3u && r->dl_count == 382u && r->br_count == 2u &&
           r->do_count == 1u && r->fo_count == 2u && r->ne_count == 2u &&
           r->bn_count == 2u && r->sd_count == 0u && r->so_count == 0u;
}
