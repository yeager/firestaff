#ifndef DM1_V1_DOS_PC34_AN_CONTAINER_H
#define DM1_V1_DOS_PC34_AN_CONTAINER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 DOS PC 3.4 "AN" animation container (TITLE, END).
 *
 * Byte-verified 2026-08-07 against the shipping disc's TITLE
 * (12002 bytes) and END (364196 bytes). Both files use a compact
 * chunk container inspired by but distinct from EA IFF ANIM
 * (2-byte tags, not 4-byte; no FORM wrapper; no ANHD/DLTA/BODY).
 *
 * Fixed 14-byte header (identical in both files):
 *   [0..1]   "AN"           2-byte signature
 *   [2..3]   00 08          version / flags
 *   [4..5]   00 00          reserved
 *   [6..7]   01 40 BE       width  = 320
 *   [8..9]   00 c8 BE       height = 200
 *   [10..11] 00 04 BE       bit planes = 4 (16 colour palette)
 *   [12..13] 00 03 BE       constant (subtype)
 *
 * After the header, a stream of 2-byte-tag chunks whose exact
 * framing rules do NOT match a plain "tag + BE u32 length + payload"
 * shape. Observed absolute tag offsets in TITLE (12002 bytes):
 *   14   "BR"   (short marker; declared length 1, payload 0x50)
 *   42   "PL"   (palette; ~72 bytes to next tag)
 *   114  "EN"   (encoded frame body; runs 6210 bytes to next PL)
 *   6324 "PL"   (second-frame palette; also 72 bytes)
 *   6396 "EN"   (second-frame body; runs to EOF, 5606 bytes)
 * Observed in END (364196 bytes):
 *   14+  seven "TD" catalogue entries (per-frame body sizes)
 *   ...  then "BR"/"PL"/"EN" as in TITLE but for 7 frames.
 *
 * The block from offset 21..41 in TITLE (21 raw bytes) between
 * BR's payload byte and the first PL does not carry a valid tag
 * header; the container-level framing needs one more RE pass to
 * be exact. This module therefore ships ONLY the fixed 14-byte
 * header parser today, plus a stub `find_next_tag` that scans
 * forward for the next 2-byte ASCII tag from a known set — good
 * enough for locating palettes and frame bodies inside the file
 * without pretending we know the interstitial byte layout.
 *
 * Full chunk iteration, RLE decompression, and palette conversion
 * remain open (next layer).
 */

#define DM1_V1_DOS_PC34_AN_WIDTH        320
#define DM1_V1_DOS_PC34_AN_HEIGHT       200
#define DM1_V1_DOS_PC34_AN_PLANES         4
#define DM1_V1_DOS_PC34_AN_HEADER_BYTES  14

typedef enum {
    DM1_V1_DOS_PC34_AN_OK             = 0,
    DM1_V1_DOS_PC34_AN_BAD_ARGS       = -1,
    DM1_V1_DOS_PC34_AN_TOO_SMALL      = -2,
    DM1_V1_DOS_PC34_AN_BAD_SIGNATURE  = -3,
    DM1_V1_DOS_PC34_AN_BAD_GEOMETRY   = -4,
    DM1_V1_DOS_PC34_AN_OVERRUN        = -5
} dm1_v1_dos_pc34_an_status_t;

typedef struct dm1_v1_dos_pc34_an_header {
    uint16_t width;
    uint16_t height;
    uint16_t planes;
    uint16_t subtype;
} dm1_v1_dos_pc34_an_header_t;

/* Parse and validate the fixed 14-byte header. Rejects blobs that
 * don't match the exact DM1 DOS 3.4 shape (320x200x4). */
dm1_v1_dos_pc34_an_status_t
dm1_v1_dos_pc34_an_parse_header_pc34(
    const uint8_t *blob, size_t blob_size,
    dm1_v1_dos_pc34_an_header_t *out);

/* Scan forward from `from_offset` and return the absolute offset of
 * the next known 2-byte tag in the recognised set: "BR", "P8", "PL",
 * "EN", "TD". Writes the found tag bytes into `tag_out[0..1]` (plus
 * NUL at [2]). Returns 1 on success, 0 if EOF is reached first. */
int dm1_v1_dos_pc34_an_find_next_tag_pc34(
    const uint8_t *blob, size_t blob_size,
    uint32_t from_offset, uint32_t *offset_out,
    char tag_out[3]);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_DOS_PC34_AN_CONTAINER_H */
