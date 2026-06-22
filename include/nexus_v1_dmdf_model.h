
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

/* ── DMDF embedded palette / string blocks ────────────────────────
 * Source-lock: docs/nexus_v1_phase2_data_formats_H2321.md §6.5,
 *   §8.2 VDP1 BITMAP notes ("8bpp (palette) or 16bpp (direct color)
 *   may use 4-bit or 8-bit clut (color look-up table)").
 * Format proposed from the observed Saturn layout, big-endian:
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
 * the offsets/sizes/counts and refuses to read past the block boundary.
 * Decoding into real RGBA / final string storage is intentionally out of
 * scope for the V1 parser-level lane. */

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

/* Parser-level bounds gates. Return 1 on success, 0 on any bounds
 * failure (offset past end, count overflow, payload past end, etc).
 * Output struct is always written, with valid=0 on failure so callers
 * can do `valid && bytes_used > 0` style checks safely. */
int nexus_v1_dmdf_parse_palette_block(const uint8_t *data, int size,
                                      int offset,
                                      Nexus_DMDFPaletteBlock *out);

int nexus_v1_dmdf_parse_string_block(const uint8_t *data, int size,
                                     int offset,
                                     Nexus_DMDFStringBlock *out);

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

