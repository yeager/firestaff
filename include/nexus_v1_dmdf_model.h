
#ifndef NEXUS_V1_DMDF_MODEL_H
#define NEXUS_V1_DMDF_MODEL_H
#include <stdint.h>

/* DMDF — Dungeon Master Data Format
 * Magic: "DMDF" at offset 0
 * Used for creature models (.MNS files) in DM Nexus Saturn.
 * Big-endian (SH2 processor). */

#define NEXUS_DMDF_MAGIC 0x444D4446  /* "DMDF" */

typedef struct {
    uint32_t magic;
    uint32_t file_size;
    uint32_t section_count;
    uint32_t flags;
    uint32_t reserved[4];
    uint32_t data_offset;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t face_count;
} Nexus_DMDFHeader;

typedef struct {
    int16_t x, y, z;
    int16_t nx, ny, nz;  /* normal */
    uint16_t u, v;        /* texture coords */
} Nexus_DMDFVertex;

typedef struct {
    Nexus_DMDFHeader header;
    Nexus_DMDFVertex *vertices;
    int vertex_count;
    uint16_t *faces;      /* triangle/quad indices */
    int face_count;
    uint8_t *texture_data;
    int texture_size;
    const char *name;
} Nexus_V1_Model;

/* ── DMDF embedded BITMAP / palette / string blocks ───────────────
 * Source-lock: docs/nexus_v1_phase2_data_formats_H2321.md §6.5,
 *   §8.2 VDP1 BITMAP notes ("8bpp (palette) or 16bpp (direct color)
 *   may use 4-bit or 8-bit clut (color look-up table)").
 * Format proposed from the observed Saturn layout, big-endian:
 *
 *   BITM (bitmap block) — 32-byte header + packed pixels
 *     0x00  uint32 magic   = 0x4249544D ("BITM")
 *     0x04  uint32 size    = total block bytes including header
 *     0x08  uint32 width   = texture width (power of two, 8..256)
 *     0x0C  uint32 height  = texture height (power of two, 8..256)
 *     0x10  uint32 bpp     = 4, 8, or 16
 *     0x14  uint32 flags   = parser-visible metadata only
 *     0x18  uint32 palette = palette slot / CLUT hint, if any
 *     0x1C  uint32 reserved
 *     0x20..size-1         = packed pixel payload
 *
 *   PLTB (palette block) — 16-byte header + N entries × entry_size
 *     0x00  uint32 magic   = 0x504C5442 ("PLTB")
 *     0x04  uint32 size    = total block bytes including header
 *     0x08  uint32 count   = number of palette entries (0..256)
 *     0x0C  uint32 esize   = bytes per entry (1, 2 or 4)
 *     0x10..size-1         = entry data, big-endian
 *
 *   STRB (string block) — 12-byte header + N string records
 *     0x00  uint32 magic   = 0x53545242 ("STRB")
 *     0x04  uint32 size    = total block bytes including header
 *     0x08  uint32 count   = number of strings (0..256)
 *     then  uint32[N] ofs  = per-string offset from block start
 *     then  uint32[N] len  = per-string length in bytes
 *     then  char data concatenated
 *
 * These blocks are parser-only scaffolding: the bounds gate validates
 * offsets/sizes/counts/dimensions and refuses to read past the block
 * boundary. Decoding VDP1 pixels into RGBA or final string storage is
 * intentionally out of scope for the V1 parser-level lane. */

#define NEXUS_DMDF_BITMAP_BLOCK_MAGIC   0x4249544DU  /* "BITM" */
#define NEXUS_DMDF_PALETTE_BLOCK_MAGIC  0x504C5442U  /* "PLTB" */
#define NEXUS_DMDF_STRING_BLOCK_MAGIC   0x53545242U  /* "STRB" */

/* Hard ceilings — derived from Saturn VDP1 Color RAM (256 entries) and
 *  DM1-style name tables. Tuned so any genuine DMDF block fits and
 *  corrupt / fuzz inputs are rejected before they can read past the
 *  block boundary. */
#define NEXUS_DMDF_MAX_PALETTE_ENTRIES  256
#define NEXUS_DMDF_MAX_PALETTE_ENTRY_SZ 4
#define NEXUS_DMDF_MAX_STRING_RECORDS   256
#define NEXUS_DMDF_MAX_STRING_BYTES     4096
#define NEXUS_DMDF_MAX_BITMAP_DIM       256
#define NEXUS_DMDF_MAX_BITMAP_BYTES     (256U * 256U * 2U)

typedef struct {
    uint32_t width;            /* parsed width (power of two, 8..256)    */
    uint32_t height;           /* parsed height (power of two, 8..256)   */
    uint32_t bpp;              /* parsed bits per pixel (4, 8, or 16)    */
    uint32_t flags;            /* raw flags word for future callers      */
    uint32_t palette_index;    /* raw palette / CLUT hint                */
    uint32_t bytes_used;       /* total block bytes consumed             */
    uint32_t payload_offset;   /* byte offset of first packed pixel      */
    uint32_t payload_bytes;    /* packed pixel payload bytes             */
    int      valid;            /* 1 if bounds checks all passed          */
} Nexus_DMDFBitmapBlock;

typedef struct {
    uint32_t entry_count;       /* parsed N (clamped to [0..256])        */
    uint32_t entry_size;        /* parsed entry_size (1, 2 or 4)         */
    uint32_t bytes_used;        /* total block bytes consumed            */
    uint32_t payload_offset;    /* byte offset of first entry inside buf */
    uint8_t  bpp;               /* 4 or 8 (clut-bits-per-pixel)          */
    int      valid;             /* 1 if bounds checks all passed         */
} Nexus_DMDFPaletteBlock;

typedef struct {
    uint32_t string_count;      /* parsed N (clamped to [0..256])        */
    uint32_t bytes_used;        /* total block bytes consumed            */
    uint32_t payload_offset;    /* byte offset of first offset/length tbl */
    int      valid;             /* 1 if bounds checks all passed         */
} Nexus_DMDFStringBlock;

typedef struct {
    uint32_t bitmap_block_count;
    uint32_t palette_block_count;
    uint32_t string_block_count;
    uint32_t invalid_candidate_count;
    uint32_t bytes_scanned;
    uint32_t first_bitmap_offset;
    uint32_t first_palette_offset;
    uint32_t first_string_offset;
    int      valid;
} Nexus_DMDFEmbeddedScan;

typedef struct {
    uint32_t offset;
    uint32_t bytes;
    int      valid;
} Nexus_DMDFRawTexturePayload;

/* Parser-level bounds gates. Return 1 on success, 0 on any bounds
 * failure (offset past end, count overflow, payload past end, etc).
 * Output struct is always written, with valid=0 on failure so callers
 * can do `valid && bytes_used > 0` style checks safely. */
int nexus_v1_dmdf_parse_palette_block(const uint8_t *data, int size,
                                      int offset,
                                      Nexus_DMDFPaletteBlock *out);

int nexus_v1_dmdf_parse_bitmap_block(const uint8_t *data, int size,
                                     int offset,
                                     Nexus_DMDFBitmapBlock *out);

int nexus_v1_dmdf_parse_string_block(const uint8_t *data, int size,
                                     int offset,
                                     Nexus_DMDFStringBlock *out);

/* Walk a bounded byte range and count parser-recognized embedded BITM,
 * PLTB, and STRB blocks. `offset` selects the first byte to inspect and
 * `max_bytes` caps the search window; pass 0 for max_bytes to scan from
 * offset to the end of the buffer. Recognized valid blocks advance by
 * their declared size; malformed candidates advance by one byte and are
 * counted in invalid_candidate_count. */
int nexus_v1_dmdf_scan_embedded_blocks(const uint8_t *data, int size,
                                       int offset, int max_bytes,
                                       Nexus_DMDFEmbeddedScan *out);

/* Estimate the raw texture payload after the currently implemented DMDF
 * vertex/triangle face stream: data_offset + 8 + vertex_count*10 +
 * face_count*6. This does not decode pixels; it only bounds the residual
 * payload so real .MNS probes can report whether an embedded texture-like
 * tail is present without dereferencing past EOF. */
int nexus_v1_dmdf_estimate_raw_texture_payload(
    const uint8_t *data, int size, Nexus_DMDFRawTexturePayload *out);

/* Helper: read a single palette entry out of a parsed palette block.
 * Returns 1 on success and stores the entry value (up to 32 bits) in
 * *out_value; returns 0 if idx is out of range or the entry would
 * straddle the block boundary. */
int nexus_v1_dmdf_palette_entry(const uint8_t *data, int size,
                                const Nexus_DMDFPaletteBlock *blk,
                                uint32_t idx, uint32_t *out_value);

/* Helper: read a single string offset/length record out of a parsed
 * string block. Returns 1 on success and stores the values in
 * *out_offset, *out_length (both relative to the start of the block);
 * returns 0 if idx is out of range. */
int nexus_v1_dmdf_string_record(const uint8_t *data, int size,
                                const Nexus_DMDFStringBlock *blk,
                                uint32_t idx,
                                uint32_t *out_offset, uint32_t *out_length);

int nexus_v1_dmdf_load(Nexus_V1_Model *model, const uint8_t *data, int size, const char *name);
void nexus_v1_dmdf_free(Nexus_V1_Model *model);
int nexus_v1_dmdf_is_valid(const uint8_t *data, int size);

#endif
