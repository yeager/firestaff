#include "dm1_v1_fmtowns_pic_library.h"

#include <string.h>

/* Every byte offset, mask, and formula in this file is byte-verified
 * against the hash-verified HMA-240 English EDM.EXP. See
 * parity-evidence/dm1_fmtowns_pic_library_format.md for the full
 * disassembly trail. */

static uint16_t read_u16_le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_open_pc34(
    const uint8_t *data,
    size_t         data_size,
    dm1_v1_fmtowns_pic_library_view_t *out_view)
{
    if (data == NULL || out_view == NULL) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_NULL;
    }
    memset(out_view, 0, sizeof(*out_view));

    if (data_size < DM1_V1_FMTOWNS_PIC_LIB_COUNT_BYTES) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
    }
    uint16_t count = read_u16_le(data);
    /* Header = 2 + count*4 (INDEX_TO_FILE_OFFSET base at 0x8d13). */
    size_t header_bytes = (size_t)DM1_V1_FMTOWNS_PIC_LIB_COUNT_BYTES
                        + (size_t)count
                          * (size_t)DM1_V1_FMTOWNS_PIC_LIB_SIZE_ENTRY_BYTES
                          * (size_t)DM1_V1_FMTOWNS_PIC_LIB_TABLE_COUNT;
    if (data_size < header_bytes) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
    }

    out_view->data                 = data;
    out_view->data_size            = data_size;
    out_view->asset_count          = count;
    out_view->size_table_primary   = data + DM1_V1_FMTOWNS_PIC_LIB_COUNT_BYTES;
    out_view->size_table_secondary = out_view->size_table_primary
        + (size_t)count * DM1_V1_FMTOWNS_PIC_LIB_SIZE_ENTRY_BYTES;
    out_view->payload_offset       = header_bytes;

    /* Verify payload fits: sum(sizes) + header == data_size. */
    size_t sum = 0;
    for (uint16_t i = 0; i < count; ++i) {
        sum += read_u16_le(out_view->size_table_primary
                           + (size_t)i * DM1_V1_FMTOWNS_PIC_LIB_SIZE_ENTRY_BYTES);
    }
    if (header_bytes + sum > data_size) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
    }
    return DM1_V1_FMTOWNS_PIC_LIB_OK;
}

dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_asset_size_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view,
    uint16_t  index,
    uint16_t *out_size_bytes)
{
    if (view == NULL || out_size_bytes == NULL) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_NULL;
    }
    if (index >= view->asset_count) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_INDEX;
    }
    *out_size_bytes = read_u16_le(view->size_table_primary
        + (size_t)index * DM1_V1_FMTOWNS_PIC_LIB_SIZE_ENTRY_BYTES);
    return DM1_V1_FMTOWNS_PIC_LIB_OK;
}

dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_asset_offset_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view,
    uint16_t  index,
    size_t   *out_file_offset)
{
    if (view == NULL || out_file_offset == NULL) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_NULL;
    }
    if (index >= view->asset_count) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_INDEX;
    }
    /* Exact formula from INDEX_TO_FILE_OFFSET (EDM.EXP 0x8d04):
     *   result = 2 + count*4 + sum(size_table_primary[0..index-1]) */
    size_t off = view->payload_offset;
    for (uint16_t i = 0; i < index; ++i) {
        off += read_u16_le(view->size_table_primary
            + (size_t)i * DM1_V1_FMTOWNS_PIC_LIB_SIZE_ENTRY_BYTES);
    }
    *out_file_offset = off;
    return DM1_V1_FMTOWNS_PIC_LIB_OK;
}

dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_asset_bytes_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view,
    uint16_t         index,
    const uint8_t **out_ptr,
    uint16_t        *out_size_bytes)
{
    if (view == NULL || out_ptr == NULL || out_size_bytes == NULL) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_NULL;
    }
    uint16_t size;
    size_t   off;
    dm1_v1_fmtowns_pic_library_status_t s;
    s = dm1_v1_fmtowns_pic_library_asset_size_pc34(view, index, &size);
    if (s != DM1_V1_FMTOWNS_PIC_LIB_OK) return s;
    s = dm1_v1_fmtowns_pic_library_asset_offset_pc34(view, index, &off);
    if (s != DM1_V1_FMTOWNS_PIC_LIB_OK) return s;
    if (off + size > view->data_size) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
    }
    *out_ptr        = view->data + off;
    *out_size_bytes = size;
    return DM1_V1_FMTOWNS_PIC_LIB_OK;
}

int
dm1_v1_fmtowns_pic_library_tables_are_mirror_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view)
{
    if (view == NULL) return 0;
    size_t bytes = (size_t)view->asset_count
                 * DM1_V1_FMTOWNS_PIC_LIB_SIZE_ENTRY_BYTES;
    return memcmp(view->size_table_primary,
                  view->size_table_secondary, bytes) == 0;
}

uint16_t
dm1_v1_fmtowns_pic_library_padded_width_pc34(uint16_t width_pixels)
{
    /* EDM.EXP 0x1f665..0x1f66e:
     *   movzx edx, dx   ; width
     *   add   edx, 0x1f
     *   and   dl, 0xe0  ; low byte &= ~0x1f
     * The `and dl, 0xe0` only masks the low byte, but since width
     * plus 0x1f stays inside a single word for every shipped asset
     * (max width observed is 320), the arithmetic is equivalent to
     * a 16-bit `& ~0x1f`. */
    return (uint16_t)((width_pixels + 0x1fu) & (uint16_t)0xffe0u);
}

uint16_t
dm1_v1_fmtowns_pic_library_row_bytes_pc34(uint16_t width_pixels)
{
    /* EDM.EXP 0x1f66e..0x1f672:
     *   sar edx, 1
     *   add edx, edx
     * i.e. drop the low bit then double — because `padded_width` is
     * a multiple of 32, `padded_width/2` is already even, so the
     * net effect is exactly `padded_width / 2` (bytes per row at
     * 4 bpp). */
    return (uint16_t)(dm1_v1_fmtowns_pic_library_padded_width_pc34(width_pixels) >> 1);
}

dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_parse_gfx_header_pc34(
    const uint8_t *asset_bytes,
    size_t         asset_size,
    dm1_v1_fmtowns_pic_library_gfx_header_t *out_header)
{
    if (asset_bytes == NULL || out_header == NULL) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_NULL;
    }
    if (asset_size < DM1_V1_FMTOWNS_PIC_LIB_ASSET_HEADER_BYTES) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_ASSET_HEADER;
    }
    uint16_t w = read_u16_le(asset_bytes + 0);
    uint16_t h = read_u16_le(asset_bytes + 2);
    uint16_t pw = dm1_v1_fmtowns_pic_library_padded_width_pc34(w);
    uint16_t rb = (uint16_t)(pw >> 1);

    out_header->width_pixels        = w;
    out_header->height_pixels       = h;
    out_header->padded_width_pixels = pw;
    out_header->row_bytes           = rb;
    out_header->decoded_total_bytes = (uint32_t)rb * (uint32_t)h;
    /* DECODEGRAPHIC 0x1f689: `cmp dx, ax` then `je 0x1f85f` — if the
     * on-disk width equals the padded width, the RLE loop is
     * skipped and the pixels are copied verbatim. */
    out_header->is_uncompressed     = (pw == w) ? 1 : 0;
    return DM1_V1_FMTOWNS_PIC_LIB_OK;
}

dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_load_raw_asset_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view,
    uint16_t  index,
    uint8_t  *dst_buffer,
    size_t    dst_capacity,
    size_t   *out_bytes_written)
{
    if (view == NULL || dst_buffer == NULL || out_bytes_written == NULL) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_NULL;
    }
    *out_bytes_written = 0;

    const uint8_t *src;
    uint16_t       size;
    dm1_v1_fmtowns_pic_library_status_t s =
        dm1_v1_fmtowns_pic_library_asset_bytes_pc34(view, index, &src, &size);
    if (s != DM1_V1_FMTOWNS_PIC_LIB_OK) return s;
    if (dst_capacity < (size_t)size) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
    }
    /* This mirrors the DIRECT+NO_HDR path in GET_MY_DECODED
     * (EDM.EXP 0x9f4a): the caller's buffer receives the raw asset
     * bytes and `_DECODE` is never invoked. */
    memcpy(dst_buffer, src, size);
    *out_bytes_written = size;
    return DM1_V1_FMTOWNS_PIC_LIB_OK;
}
