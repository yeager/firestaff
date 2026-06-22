
#include "nexus_v1_dmdf_model.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Big-endian readers (Saturn SH2 is big-endian) */
static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint16_t rb16(const uint8_t *p) { return ((uint16_t)p[0]<<8)|p[1]; }
static int16_t rbs16(const uint8_t *p) { return (int16_t)rb16(p); }

int nexus_v1_dmdf_is_valid(const uint8_t *data, int size) {
    if (!data || size < 32) return 0;
    return rb32(data) == NEXUS_DMDF_MAGIC;
}

int nexus_v1_dmdf_load(Nexus_V1_Model *model, const uint8_t *data, int size, const char *name) {
    if (!model || !data || size < 32) return -1;
    memset(model, 0, sizeof(*model));

    if (!nexus_v1_dmdf_is_valid(data, size)) return -1;

    model->header.magic = rb32(data);
    model->header.file_size = rb32(data + 4);
    model->header.section_count = rb32(data + 8);
    model->header.flags = rb32(data + 12);
    model->header.data_offset = rb32(data + 28);
    model->name = name;

    printf("DMDF %s: size=%u sections=%u data_offset=%u\n",
        name ? name : "?",
        model->header.file_size,
        model->header.section_count,
        model->header.data_offset);

    /* Parse vertex/face data from sections */
    if (model->header.data_offset > 0 && (int)model->header.data_offset < size) {
        int off = (int)model->header.data_offset;
        /* Read vertex count and face count from data section */
        if (off + 8 <= size) {
            int vc = rb32(data + off);
            int fc = rb32(data + off + 4);
            if (vc > 0 && vc < 10000 && fc > 0 && fc < 30000) {
                model->vertex_count = vc;
                model->header.vertex_count = vc;
                model->header.face_count = fc;
                /* Allocate and read vertices */
                int vert_size = vc * 10; /* 5 int16 per vertex */
                if (off + 8 + vert_size <= size) {
                    model->vertices = (Nexus_DMDFVertex *)calloc(vc, sizeof(Nexus_DMDFVertex));
                    if (model->vertices) {
                        for (int i = 0; i < vc; i++) {
                            int vo = off + 8 + i * 10;
                            model->vertices[i].x = rbs16(data + vo);
                            model->vertices[i].y = rbs16(data + vo + 2);
                            model->vertices[i].z = rbs16(data + vo + 4);
                            model->vertices[i].u = rb16(data + vo + 6);
                            model->vertices[i].v = rb16(data + vo + 8);
                        }
                    }
                }
                /* Read face indices */
                int face_off = off + 8 + vert_size;
                int face_bytes = fc * 6; /* 3 uint16 per triangle */
                if (face_off + face_bytes <= size) {
                    model->faces = (uint16_t *)calloc(fc * 3, sizeof(uint16_t));
                    if (model->faces) {
                        model->face_count = fc;
                        for (int i = 0; i < fc * 3; i++)
                            model->faces[i] = rb16(data + face_off + i * 2);
                    }
                }
                printf("  vertices=%d faces=%d\n", vc, fc);
            }
        }
    }

    return 0;
}

void nexus_v1_dmdf_free(Nexus_V1_Model *model) {
    if (!model) return;
    free(model->vertices); model->vertices = NULL;
    free(model->faces); model->faces = NULL;
    free(model->texture_data); model->texture_data = NULL;
}

/* ── DMDF embedded palette / string block bounds gates ─────────────
 * Source-lock: docs/nexus_v1_phase2_data_formats_H2321.md §6.5,
 *   §8.2 VDP1 BITMAP notes ("8bpp (palette) or 16bpp (direct color)
 *   may use 4-bit or 8-bit clut (color look-up table)").
 *
 * These helpers exist so the parser can never read past the end of a
 * DMDF buffer just because a corrupt / fuzz-crafted block lied about
 * its size. Every offset, count and length is re-validated against the
 * caller-supplied (data, size) pair before any read happens.
 *
 * Intentionally parser-only: we expose raw values plus a `valid` flag
 * and do NOT allocate or decode. V1 runtime handoff for palette/string
 * blocks remains gated behind real-data evidence (see
 * docs/nexus_v1_phase2_data_formats_H2321.md status §6.10/§8.6). */

static void init_palette_block(Nexus_DMDFPaletteBlock *out) {
    if (!out) return;
    out->entry_count = 0;
    out->entry_size = 0;
    out->bytes_used = 0;
    out->payload_offset = 0;
    out->bpp = 0;
    out->valid = 0;
}

static void init_string_block(Nexus_DMDFStringBlock *out) {
    if (!out) return;
    out->string_count = 0;
    out->bytes_used = 0;
    out->payload_offset = 0;
    out->valid = 0;
}

int nexus_v1_dmdf_parse_palette_block(const uint8_t *data, int size,
                                      int offset,
                                      Nexus_DMDFPaletteBlock *out)
{
    /* Header is 16 bytes: magic(4) + size(4) + count(4) + esize(4). */
    const int kHeader = 16;
    uint32_t magic = 0, blk_size = 0, count = 0, esize = 0;

    if (!out) return 0;
    init_palette_block(out);

    if (!data || size <= 0) return 0;
    if (offset < 0 || offset > size) return 0;
    if (size - offset < kHeader) return 0;

    magic  = rb32(data + offset);
    blk_size = rb32(data + offset + 4);
    count  = rb32(data + offset + 8);
    esize  = rb32(data + offset + 12);

    if (magic != NEXUS_DMDF_PALETTE_BLOCK_MAGIC) return 0;
    if (blk_size < (uint32_t)kHeader) return 0;
    /* block must fit entirely inside the buffer */
    if ((uint32_t)offset + blk_size > (uint32_t)size) return 0;

    /* entry_size must be one of {1, 2, 4} to map onto
     *   4bpp CLUT (1 byte), BGR555/BGR565 (2 bytes), 32-bit XRGB (4 bytes) */
    if (esize != 1 && esize != 2 && esize != 4) return 0;
    if (esize > NEXUS_DMDF_MAX_PALETTE_ENTRY_SZ) return 0;

    /* count must fit inside block payload and respect the hard ceiling */
    if (count > NEXUS_DMDF_MAX_PALETTE_ENTRIES) return 0;
    {
        uint64_t payload = (uint64_t)count * (uint64_t)esize;
        if ((uint32_t)kHeader + (uint32_t)payload != blk_size) return 0;
        if (payload > (uint64_t)(size - offset - kHeader)) return 0;
    }

    out->entry_count   = count;
    out->entry_size    = esize;
    out->bytes_used    = blk_size;
    out->payload_offset = (uint32_t)(offset + kHeader);
    /* 4bpp CLUT when entries are single bytes, otherwise 8bpp. */
    out->bpp           = (esize == 1) ? 4 : 8;
    out->valid         = 1;
    return 1;
}

int nexus_v1_dmdf_parse_string_block(const uint8_t *data, int size,
                                     int offset,
                                     Nexus_DMDFStringBlock *out)
{
    /* Header is 12 bytes: magic(4) + size(4) + count(4). */
    const int kHeader = 12;
    uint32_t magic = 0, blk_size = 0, count = 0;
    uint64_t tbl_bytes = 0;
    uint32_t body_bytes = 0;

    if (!out) return 0;
    init_string_block(out);

    if (!data || size <= 0) return 0;
    if (offset < 0 || offset > size) return 0;
    if (size - offset < kHeader) return 0;

    magic    = rb32(data + offset);
    blk_size = rb32(data + offset + 4);
    count    = rb32(data + offset + 8);

    if (magic != NEXUS_DMDF_STRING_BLOCK_MAGIC) return 0;
    if (blk_size < (uint32_t)kHeader) return 0;
    if ((uint32_t)offset + blk_size > (uint32_t)size) return 0;
    if (count > NEXUS_DMDF_MAX_STRING_RECORDS) return 0;

    /* Two parallel uint32 tables (offsets, lengths) plus body. */
    tbl_bytes   = (uint64_t)count * 8u;
    if (kHeader + (uint64_t)tbl_bytes > (uint64_t)blk_size) return 0;
    body_bytes = blk_size - (uint32_t)kHeader - (uint32_t)tbl_bytes;
    if (body_bytes > NEXUS_DMDF_MAX_STRING_BYTES) return 0;

    /* Spot-check every record: reject if offset/length would extend
     * past the block boundary. Records may legitimately point to
     * overlapping ranges (e.g. shared suffixes) but never past the
     * declared block size. */
    {
        uint32_t i;
        for (i = 0; i < count; i++) {
            uint32_t ro = rb32(data + offset + kHeader + i * 4);
            uint32_t rl = rb32(data + offset + kHeader + count * 4 + i * 4);
            if ((uint64_t)ro + (uint64_t)rl > (uint64_t)blk_size) return 0;
            /* offset must point inside the block, not the header area */
            if (ro < (uint32_t)kHeader + (uint32_t)tbl_bytes) return 0;
            if (rl > NEXUS_DMDF_MAX_STRING_BYTES) return 0;
        }
    }

    out->string_count    = count;
    out->bytes_used      = blk_size;
    out->payload_offset  = (uint32_t)(offset + kHeader);
    out->valid           = 1;
    return 1;
}

int nexus_v1_dmdf_palette_entry(const uint8_t *data, int size,
                                const Nexus_DMDFPaletteBlock *blk,
                                uint32_t idx, uint32_t *out_value)
{
    uint64_t base = 0;
    uint32_t esz = 0;
    uint32_t val = 0;
    uint32_t k;

    if (!data || size <= 0 || !blk || !out_value) return 0;
    if (!blk->valid) return 0;
    if (idx >= blk->entry_count) return 0;

    base = (uint64_t)blk->payload_offset + (uint64_t)idx * (uint64_t)blk->entry_size;
    esz  = blk->entry_size;
    if (base + esz > (uint64_t)size) return 0;

    for (k = 0; k < esz; k++) {
        val = (val << 8) | data[base + k];
    }
    *out_value = val;
    return 1;
}

int nexus_v1_dmdf_string_record(const uint8_t *data, int size,
                                const Nexus_DMDFStringBlock *blk,
                                uint32_t idx,
                                uint32_t *out_offset, uint32_t *out_length)
{
    uint64_t ofs_base = 0, len_base = 0;

    if (!data || size <= 0 || !blk || !out_offset || !out_length) return 0;
    if (!blk->valid) return 0;
    if (idx >= blk->string_count) return 0;
    /* Each table is uint32 × count. */
    if ((uint64_t)blk->payload_offset + (uint64_t)idx * 4u + 4u > (uint64_t)size) return 0;
    if ((uint64_t)blk->payload_offset + (uint64_t)blk->string_count * 4u
        + (uint64_t)idx * 4u + 4u > (uint64_t)size) return 0;

    ofs_base = (uint64_t)blk->payload_offset + (uint64_t)idx * 4u;
    len_base = (uint64_t)blk->payload_offset
             + (uint64_t)blk->string_count * 4u
             + (uint64_t)idx * 4u;

    *out_offset = rb32(data + (size_t)ofs_base);
    *out_length = rb32(data + (size_t)len_base);
    return 1;
}

