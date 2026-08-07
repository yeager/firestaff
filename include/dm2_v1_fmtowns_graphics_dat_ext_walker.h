#ifndef DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_WALKER_H
#define DM2_V1_FMTOWNS_GRAPHICS_DAT_EXT_WALKER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Per-record walker for the DM2 extended GRAPHICS.DAT formats
 * (ext_v4 = FM Towns, ext_v5 = DOS). Both share the same 4-byte
 * record stride and header shape; only the signature word differs.
 *
 * Empirical layout (byte-verified 2026-08-07 against real discs):
 *   Header: {u16 sig, u16 count}                       (4 bytes)
 *   Records: {u16 stored_size, u16 aux}[count]        (4 * count)
 *   Payload: consecutive asset payloads
 *
 * The `stored_size` field is the count of payload bytes owned by
 * this record. The `aux` field is a companion metadata word whose
 * exact semantics are not yet round-trip verified. Observation
 * across all 3407 DM2 FM Towns records:
 *
 *   * Record 0 is a directory/palette blob with stored_size ~65 KB
 *     and aux == 0. Downstream tools MUST treat record 0 specially.
 *   * For records 1..N, aux tracks stored_size closely — often
 *     within +/- a few bytes. The most likely reading is that
 *     stored_size is the compressed / stored length and aux is
 *     the decoded / uncompressed length or a decode hint.
 *   * sum(stored_size) balances the payload to within ~208 KB on
 *     the DM2 FM Towns disc. The residual gap is expected to be
 *     absorbed by per-asset sub-headers or by asset-0's directory
 *     entry; per-asset decode is required to close the gap.
 *
 * This module ships the STRIDE-LEVEL walker: it validates the
 * header, iterates records, and reports {stored_size, aux, payload
 * offset} per asset. Fully-decoded pixel extraction is the next
 * layer and remains open — it needs per-asset codec identification.
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
    uint32_t header_size;     /* 4 + 4 * asset_count */
    uint32_t payload_offset;  /* == header_size */
    uint32_t payload_size;    /* file_size - header_size */
} dm2_v1_fmtowns_graphics_dat_ext_header_t;

typedef struct {
    uint16_t index;
    uint16_t stored_size;
    uint16_t aux;
    uint32_t payload_offset;  /* absolute file offset */
    int      is_directory;    /* 1 for asset 0 (stored_size > 32 KiB and aux==0) */
} dm2_v1_fmtowns_graphics_dat_ext_record_t;

/* Parse the header. `blob_size` must include the payload; the
 * function only reads the leading `4 + 4*count` bytes but reports
 * `payload_size` derived from `blob_size - header_size`. */
dm2_v1_fmtowns_graphics_dat_ext_status_t
dm2_v1_fmtowns_graphics_dat_ext_parse_header_pc34(
    const uint8_t *blob, size_t blob_size,
    dm2_v1_fmtowns_graphics_dat_ext_header_t *out);

/* Extract one record by index. Advances `payload_offset` internally
 * by walking the stored_size sum through record `index`; O(index).
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
