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

/* ------------------------------------------------------------------
 * DECODEGRAPHIC (EDM.EXP 0x1f63c) — full RLE inner-loop port
 * ------------------------------------------------------------------
 * All state that the original routine kept in the globals at
 * [0x2958c] (source stream pointer) and [0x29588] (destination pixel
 * buffer) is kept here in `decode_state`. Every branch below is
 * annotated with the byte-for-byte EDM.EXP vaddr it implements. */

typedef struct {
    const uint8_t *src;       /* asset span base */
    size_t         src_pos;   /* == [0x2958c] - asset_base (bytes consumed) */
    size_t         src_end;   /* asset_size (for bounds checks) */
    uint8_t       *dst;       /* == [0x29588] (destination pixel buffer) */
    size_t         dst_bytes; /* row_bytes * height (for bounds checks) */
    uint16_t       width;     /* [ebp-2] pixels per row (unpadded) */
    uint16_t       padded_w;  /* [ebp-6] pixels per row (padded) */
    uint16_t       delta;     /* [ebp-0xe] padded_w - width */
    uint32_t       total_px;  /* [ebp-8] padded_w * height */
    uint32_t       esi_px;    /* current destination pixel index (esi) */
    int32_t        di_row;    /* pixels remaining in current row (di) */
} decode_state;

static inline int decode_read_byte(decode_state *st, uint8_t *out) {
    if (st->src_pos >= st->src_end) return 0;
    *out = st->src[st->src_pos++];
    return 1;
}

/* 0x1f4c4: put one pixel — EVEN pixel_index → LOW nibble,
 * ODD pixel_index → HIGH nibble. */
static inline void decode_put_pixel(decode_state *st, uint32_t pi, uint8_t nib) {
    size_t bo = (size_t)(pi >> 1);
    if (bo >= st->dst_bytes) return;
    uint8_t b = st->dst[bo];
    if (pi & 1u) {
        b = (uint8_t)((b & 0x0fu) | ((nib & 0x0fu) << 4));
    } else {
        b = (uint8_t)((b & 0xf0u) | (nib & 0x0fu));
    }
    st->dst[bo] = b;
}

/* 0x1f518: fill `count` pixels starting at `start_pi` with nibble `nib`.
 * Handles odd-pixel prologue then advances two pixels per byte-write
 * using a duplicated-nibble byte. */
static void decode_fill_pixels(decode_state *st,
                               uint32_t start_pi, uint8_t nib, int32_t count) {
    if (count <= 0) return;
    uint32_t idx = start_pi;
    int32_t cnt = count;
    if (idx & 1u) {
        decode_put_pixel(st, idx, nib);
        idx++;
        cnt--;
    }
    uint8_t dup = (uint8_t)(((nib & 0x0fu) << 4) | (nib & 0x0fu));
    while (cnt > 0) {
        size_t bo = (size_t)(idx >> 1);
        if (bo < st->dst_bytes) st->dst[bo] = dup;
        idx += 2u;
        cnt -= 2;
    }
}

/* 0x1f578: copy `count` pixels from the source stream (packed
 * big-nibble-first from `base_src_pos`) to destination pixel index
 * `dst_pi`. */
static void decode_copy_from_stream(decode_state *st,
                                    size_t base_src_pos,
                                    uint32_t src_ppos,
                                    uint32_t dst_pi,
                                    int32_t count) {
    while (count > 0) {
        size_t bo = base_src_pos + (size_t)(src_ppos >> 1);
        if (bo >= st->src_end) return;
        uint8_t b = st->src[bo];
        uint8_t nib = (src_ppos & 1u) ? (uint8_t)(b & 0x0fu)
                                       : (uint8_t)((b >> 4) & 0x0fu);
        decode_put_pixel(st, dst_pi, nib);
        dst_pi++;
        src_ppos++;
        count--;
    }
}

/* 0x1f5d8: row copy — read whole bytes from `src_pi` and write them
 * to `dst_pi` inside the destination buffer. Used to duplicate the
 * previous scanline. Odd-aligned dst prologue handles a single-pixel
 * high-nibble copy first. */
static void decode_row_copy(decode_state *st,
                            uint32_t src_pi, uint32_t dst_pi, int32_t count) {
    if (count <= 0) return;
    if (dst_pi & 1u) {
        size_t bs = (size_t)(src_pi >> 1);
        size_t bd = (size_t)(dst_pi >> 1);
        if (bd < st->dst_bytes && bs < st->dst_bytes) {
            uint8_t sb = st->dst[bs];
            st->dst[bd] = (uint8_t)((st->dst[bd] & 0x0fu) | (sb & 0xf0u));
        }
        dst_pi++;
        src_pi++;
        count--;
    }
    while (count > 0) {
        size_t bs = (size_t)(src_pi >> 1);
        size_t bd = (size_t)(dst_pi >> 1);
        if (bd < st->dst_bytes && bs < st->dst_bytes) {
            st->dst[bd] = st->dst[bs];
        }
        dst_pi += 2u;
        src_pi += 2u;
        count -= 2;
    }
}

dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_decode_asset_pc34(
    const uint8_t *asset_bytes,
    size_t         asset_size,
    uint8_t       *dst_buffer,
    size_t         dst_capacity,
    size_t        *out_bytes_written,
    size_t        *out_source_bytes_consumed,
    dm1_v1_fmtowns_pic_library_gfx_header_t *out_header)
{
    if (asset_bytes == NULL || dst_buffer == NULL
        || out_bytes_written == NULL || out_source_bytes_consumed == NULL) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_NULL;
    }
    *out_bytes_written = 0;
    *out_source_bytes_consumed = 0;

    dm1_v1_fmtowns_pic_library_gfx_header_t hdr;
    dm1_v1_fmtowns_pic_library_status_t s =
        dm1_v1_fmtowns_pic_library_parse_gfx_header_pc34(asset_bytes, asset_size, &hdr);
    if (s != DM1_V1_FMTOWNS_PIC_LIB_OK) return s;
    if (out_header) *out_header = hdr;

    if ((size_t)hdr.decoded_total_bytes > dst_capacity) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
    }
    memset(dst_buffer, 0, hdr.decoded_total_bytes);

    decode_state st;
    st.src        = asset_bytes;
    st.src_pos    = DM1_V1_FMTOWNS_PIC_LIB_ASSET_HEADER_BYTES;
    st.src_end    = asset_size;
    st.dst        = dst_buffer;
    st.dst_bytes  = hdr.decoded_total_bytes;
    st.width      = hdr.width_pixels;
    st.padded_w   = hdr.padded_width_pixels;
    st.delta      = (uint16_t)(hdr.padded_width_pixels - hdr.width_pixels);
    st.total_px   = (uint32_t)hdr.padded_width_pixels * (uint32_t)hdr.height_pixels;
    st.esi_px     = 0;
    st.di_row     = hdr.width_pixels;

    /* Fast path (0x1f85f): width == padded_width. Not shipped here —
     * every FM Towns asset that hits the fast path is already
     * reachable via the raw-asset path; this decoder targets the RLE
     * branch that the format-evidence pass deferred. */
    if (hdr.is_uncompressed) {
        return DM1_V1_FMTOWNS_PIC_LIB_ERR_ASSET_HEADER;
    }

    /* Main loop (0x1f69b .. 0x1f854). */
    while (st.esi_px < st.total_px) {
        uint8_t ctrl;
        if (!decode_read_byte(&st, &ctrl)) {
            return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
        }
        uint8_t ctrl_low = (uint8_t)(ctrl & 0x0fu);

        if ((ctrl & 0x80u) == 0u) {
            /* 0x1f6b8: short-form fill. count = (ctrl >> 4) + 1. */
            int32_t count = (int32_t)((ctrl >> 4) + 1u);
            /* 0x1f6bd loop: while di <= count, fill row-remaining. */
            while (st.di_row <= count) {
                decode_fill_pixels(&st, st.esi_px, ctrl_low, st.di_row);
                st.esi_px += (uint32_t)st.di_row + (uint32_t)st.delta;
                count -= st.di_row;
                st.di_row = st.width;
            }
            if (count > 0) {
                decode_fill_pixels(&st, st.esi_px, ctrl_low, count);
                st.esi_px += (uint32_t)count;
                st.di_row -= count;
            }
        } else {
            int32_t count;
            if ((ctrl & 0x40u) == 0u) {
                /* 0x1f6e9: 1-byte count. */
                uint8_t b; if (!decode_read_byte(&st, &b)) return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
                count = (int32_t)((uint32_t)b + 1u);
            } else {
                /* 0x1f6fa: 2-byte big-endian count. */
                uint8_t hi, lo;
                if (!decode_read_byte(&st, &hi)) return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
                if (!decode_read_byte(&st, &lo)) return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
                count = (int32_t)(((uint32_t)hi << 8) | (uint32_t)lo) + 1;
            }
            uint8_t mode = (uint8_t)(ctrl & 0x30u);
            if (mode == 0x00u) {
                /* 0x1f734: mode 0 — extended fill with ctrl_low. */
                while (st.di_row <= count) {
                    decode_fill_pixels(&st, st.esi_px, ctrl_low, st.di_row);
                    st.esi_px += (uint32_t)st.di_row + (uint32_t)st.delta;
                    count -= st.di_row;
                    st.di_row = st.width;
                }
                if (count > 0) {
                    decode_fill_pixels(&st, st.esi_px, ctrl_low, count);
                    st.esi_px += (uint32_t)count;
                    st.di_row -= count;
                }
            } else if (mode == 0x10u) {
                /* 0x1f775: mode 1 — raw stream copy. */
                size_t base = st.src_pos;
                int32_t stored;   /* [ebp-0xc] tracks post-prologue count */
                if (count & 1) {
                    /* Odd count: single pixel prologue from ctrl_low. */
                    decode_put_pixel(&st, st.esi_px, ctrl_low);
                    st.esi_px++;
                    st.di_row--;
                    if (st.di_row == 0) {
                        st.esi_px += (uint32_t)st.delta;
                        st.di_row = st.width;
                    }
                    count--;
                }
                stored = count;
                uint32_t src_ppos = 0;
                while (st.di_row <= count) {
                    decode_copy_from_stream(&st, base, src_ppos, st.esi_px, st.di_row);
                    src_ppos += (uint32_t)st.di_row;
                    st.esi_px += (uint32_t)st.di_row + (uint32_t)st.delta;
                    count -= st.di_row;
                    st.di_row = st.width;
                }
                if (count > 0) {
                    decode_copy_from_stream(&st, base, src_ppos, st.esi_px, count);
                    src_ppos += (uint32_t)count;
                    st.esi_px += (uint32_t)count;
                    st.di_row -= count;
                }
                /* 0x1f7eb: sp += stored/2 (i.e. bytes consumed from stream). */
                st.src_pos += (size_t)(stored >> 1);
                if (st.src_pos > st.src_end) {
                    return DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED;
                }
            } else if (mode == 0x30u) {
                /* 0x1f7f3: mode 3 — row copy from previous scanline.
                 * NB: the partial branch (di > count > 0) passes `di`
                 * to the row-copy helper, not `count` — this matches
                 * the byte-exact disassembly and any pixels written
                 * beyond `count` are either off the row's active area
                 * or overwritten by subsequent runs. */
                while (st.di_row <= count) {
                    decode_row_copy(&st,
                                    st.esi_px - (uint32_t)st.padded_w,
                                    st.esi_px, st.di_row);
                    st.esi_px += (uint32_t)st.di_row + (uint32_t)st.delta;
                    count -= st.di_row;
                    st.di_row = st.width;
                }
                if (count > 0) {
                    decode_row_copy(&st,
                                    st.esi_px - (uint32_t)st.padded_w,
                                    st.esi_px, st.di_row);
                    st.esi_px += (uint32_t)count;
                    st.di_row -= count;
                }
                /* 0x1f833: always write one extra pixel with ctrl_low. */
                decode_put_pixel(&st, st.esi_px, ctrl_low);
                st.esi_px++;
                st.di_row--;
                if (st.di_row == 0) {
                    st.esi_px += (uint32_t)st.delta;
                    st.di_row = st.width;
                }
            }
            /* mode 0x20: falls through the DECODEGRAPHIC switch and
             * jumps to the end-of-iteration test at 0x1f850 — no
             * pixels written, no source bytes beyond the count. */
        }
    }

    *out_bytes_written = hdr.decoded_total_bytes;
    *out_source_bytes_consumed = st.src_pos;
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
