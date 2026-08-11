#ifndef DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_WALKER_H
#define DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_WALKER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Raw-payload walker for the DM2 extended GRAPHICS.DAT formats
 * (ext_v4 = FM Towns, ext_v5 = DOS). Both share the same raw table
 * layout; only the signature word differs.
 *
 * Empirical layout (byte-verified 2026-08-07 against real discs):
 *   Header: {u16 sig, u16 count, u32 raw0_size,
 *            u16 raw_size[count - 1]}                  (6 + 2*count bytes)
 *   Payload: consecutive raw payloads, raw 0 first
 *
 * Raw 0 is the source-owned ENT1 directory (0x8001 in FM Towns); all
 * following raw payloads are addressed by that directory. `aux` is retained
 * in the receipt for ABI compatibility and is always zero because the real
 * format has no second per-record word. Fully-decoded pixel extraction still
 * belongs to the image codec layer.
 */

typedef enum {
    DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OK             = 0,
    DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_BAD_ARGS       = -1,
    DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_TOO_SMALL      = -2,
    DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_BAD_SIGNATURE  = -3,
    DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_OVERRUN        = -4
} dm2_v1_fmtowns_graphics_dat_ext_status_t;

typedef struct {
    uint16_t signature;       /* 0x8004 or 0x8005 */
    uint16_t asset_count;
    uint32_t header_size;     /* 6 + 2 * asset_count */
    uint32_t payload_offset;  /* == header_size */
    uint32_t payload_size;    /* file_size - header_size */
} dm2_v1_fmtowns_graphics_dat_ext_header_t;

typedef struct {
    uint16_t index;
    uint32_t stored_size;
    uint16_t aux;             /* compatibility field; always zero */
    uint32_t payload_offset;  /* absolute file offset */
    int      is_directory;    /* 1 for asset 0 (stored_size > 32 KiB and aux==0) */
} dm2_v1_fmtowns_graphics_dat_ext_record_t;

/* Parse the header. `blob_size` must include the payload; the
 * function only reads the leading `6 + 2*count` bytes but reports
 * `payload_size` derived from `blob_size - header_size`. */
dm2_v1_fmtowns_graphics_dat_ext_status_t
dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(
    const uint8_t *blob, size_t blob_size,
    dm2_v1_fmtowns_graphics_dat_ext_header_t *out);

/* Extract one raw payload by index. Advances `payload_offset` internally
 * by walking the raw-size sum through record `index`; O(index).
 * Safe against overrun — returns OVERRUN if any prior record's
 * declared size runs past the file end. */
dm2_v1_fmtowns_graphics_dat_ext_status_t
dm2_v1_fmtowns_graphics_dat_ext_get_record_pc34(
    const uint8_t *blob, size_t blob_size,
    uint16_t index,
    dm2_v1_fmtowns_graphics_dat_ext_record_t *out);

/* Callback signature for a linear iteration. Return 0 to continue,
 * non-zero to stop early (the returned value is propagated back). */
typedef int (*dm2_v1_fmtowns_graphics_dat_ext_visitor)(
    void *user, const dm2_v1_fmtowns_graphics_dat_ext_record_t *rec);

/* Walk every record once, O(count). Stops on OVERRUN or when the
 * visitor returns non-zero. */
dm2_v1_fmtowns_graphics_dat_ext_status_t
dm2_v1_fmtowns_graphics_dat_ext_walk_pc34(
    const uint8_t *blob, size_t blob_size,
    dm2_v1_fmtowns_graphics_dat_ext_visitor visitor, void *user);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_WALKER_H */
