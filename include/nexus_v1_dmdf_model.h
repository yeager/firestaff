
#ifndef NEXUS_V1_DMDF_MODEL_H
#define NEXUS_V1_DMDF_MODEL_H
#include <stdint.h>
#include <stddef.h>
#include "nexus_v1_bpk_archive.h"

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
#define NEXUS_DMDF_TEXTURE_SECTION_MAGIC 0x54455854U /* "TEXT" */

/* Retail MNS environment resources such as SN_FLOOR.MNS and SN_WALL.MNS
 * carry a top-level TEXT section.  It is not an embedded BITM block: the
 * DMDF header points to it through the big-endian word at 0x24 and the
 * section starts with TEXT + a bounded section byte count.  The remaining
 * descriptor grammar and texel codec are intentionally not guessed here. */
typedef struct {
    uint16_t material_id;
    uint16_t flags;
    uint16_t width;
    uint16_t height;
    uint32_t pixel_offset;
    uint32_t reserved;
    int valid;
} Nexus_DMDFTextureDescriptor;

#define NEXUS_DMDF_MAX_TEXTURE_DESCRIPTORS 32

typedef struct {
    uint32_t offset;
    uint32_t bytes;
    uint32_t declared_entry_count;
    uint32_t flags;
    uint32_t descriptor_offset;
    uint32_t pixel_data_offset;
    uint32_t descriptor_count;
    Nexus_DMDFTextureDescriptor descriptors[NEXUS_DMDF_MAX_TEXTURE_DESCRIPTORS];
    int valid;
} Nexus_DMDFTextureSection;

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

/* Runtime-ready indexed surface decoded from a bounded BITM/PLTB pair.
 * `pixels` is owned by the surface and already contains global palette
 * indices, so it can be passed directly to the V1 software rasterizer. */
typedef struct {
    uint8_t *pixels;
    int width;
    int height;
    uint32_t palette[256];
    /* Set only after the complete BPK upload/decode route has committed
     * this surface into the bank. This lets the DGN host prove it rasterized
     * real FLOORS/WALLS archive pixels rather than a DMDF-only substitute. */
    int from_bpk;
    int valid;
} Nexus_DMDFTextureSurface;

#define NEXUS_DMDF_MATERIAL_COUNT 256
typedef struct {
    Nexus_DMDFTextureSurface surfaces[NEXUS_DMDF_MATERIAL_COUNT];
    int surface_count;
    int bpk_imported_surface_count;
    int bpk_indexed_surface_count;
    int bpk_truecolor_surface_count;
    int bpk_prs3_surface_count;
    int valid;
} Nexus_DMDFMaterialBank;

typedef enum {
    NEXUS_V1_DGN_MATERIAL_CATEGORY_INVALID = 0,
    NEXUS_V1_DGN_MATERIAL_CATEGORY_FLOOR = 1,
    NEXUS_V1_DGN_MATERIAL_CATEGORY_CEILING = 2,
    NEXUS_V1_DGN_MATERIAL_CATEGORY_WALL = 3
} Nexus_V1_DgnMaterialCategory;

typedef struct {
    Nexus_V1_DgnMaterialCategory category;
    uint32_t command_count;
    uint32_t material_surface_count;
    uint32_t missing_material_count;
    uint32_t unique_material_id_count;
    uint32_t covered_unique_material_id_count;
    uint8_t first_missing_material_id;
    int covered;
    int fallback_visuals_permitted;
} Nexus_V1_DgnMaterialCategoryCoverageReceipt;

typedef struct {
    Nexus_V1_DgnMaterialCategory category;
    Nexus_V1_BpkRuntimeUploadRoute upload_route;
    uint32_t archive_entries;
    uint32_t surface_entries;
    uint32_t ready_uploads;
    uint32_t blocked_prs3_uploads;
    uint32_t blocked_truncated_uploads;
    uint64_t expected_upload_bytes;
    uint64_t extractable_upload_bytes;
    int blocks_real_surface_render;
    int fallback_visuals_permitted;
    int before_surface_count;
    int after_surface_count;
    int imported_surface_count;
    int imported_indexed_surface_count;
    int imported_truecolor_surface_count;
    int imported_prs3_surface_count;
    int host_consumed_surfaces;
} Nexus_V1_BpkMaterialHostRouteReceipt;

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

/* Read the authoritative top-level TEXT section boundary from a retail DMDF
 * MNS resource.  This only authenticates and bounds original bytes; it does
 * not claim that the opaque payload has been decoded into VDP1 texels. */
int nexus_v1_dmdf_parse_texture_section(const uint8_t *data, int size,
                                        Nexus_DMDFTextureSection *out);

/* Decode retail MNS TEXT descriptors carrying BGR555 texels into the
 * existing indexed material-bank format. A texture needing more than 256
 * distinct source colours is rejected instead of quantized or synthesized. */
int nexus_v1_dmdf_decode_text_material_bank(const uint8_t *data, int size,
                                            Nexus_DMDFMaterialBank *out);

/* Decode every valid BITM in a DMDF payload into the material slot matching
 * its ordinal. A matching PLTB supplies its CLUT; malformed, direct-colour,
 * or unpaletted blocks are rejected rather than replaced with a flat colour. */
int nexus_v1_dmdf_decode_material_bank(const uint8_t *data, int size,
                                       Nexus_DMDFMaterialBank *out);

/* Fill vacant DGN material slots from a BPK archive. Entry indices are
 * material IDs; existing DMDF BITM surfaces always win. Only decoded source
 * pixels are accepted, so an unknown PRS3 stream cannot become a flat or
 * generic runtime material. */
int nexus_v1_dmdf_import_bpk_material_bank(const uint8_t *data,
                                           size_t data_size,
                                           Nexus_DMDFMaterialBank *out);

int nexus_v1_dmdf_import_bpk_material_bank_host_route(
    const uint8_t *data,
    size_t data_size,
    Nexus_DMDFMaterialBank *out,
    Nexus_V1_DgnMaterialCategory category,
    Nexus_V1_BpkMaterialHostRouteReceipt *out_receipt);

int nexus_v1_dmdf_material_category_coverage_receipt(
    const Nexus_DMDFMaterialBank *bank,
    Nexus_V1_DgnMaterialCategory category,
    const uint8_t *material_ids,
    size_t material_id_count,
    Nexus_V1_DgnMaterialCategoryCoverageReceipt *out_receipt);

const char *nexus_v1_dgn_material_category_name(
    Nexus_V1_DgnMaterialCategory category);

void nexus_v1_dmdf_free_material_bank(Nexus_DMDFMaterialBank *bank);

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
