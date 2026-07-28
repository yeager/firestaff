/* Nexus V1 UI / Title Surface Renderer — implementation
 * =====================================================
 * Loads and blits DM Nexus Saturn UI surfaces and title screens.
 *
 * Source-lock references:
 *   ReDMCSB BLIT.C    — F0132 blit rect (screen blit pipeline)
 *   ReDMCSB PANEL.C   — F0120-F0125 panel element drawing
 *   ReDMCSB CEDTINCK.C — CEDT font/text rendering
 *   Saturn SDK        — VDP1 BITMAP command: pixel format Celdat format
 *   docs/NEXUS_FILE_CLASSIFICATION.md — TITLE.CG 164 KB, FACE.BIN 44 KB,
 *     STABG.BIN 52 KB, WARNING.BIN 99 KB, GAMEOVER.BIN 101 KB
 *
 * Saturn VDP1 BITMAP surface format:
 *   - Pixel format: 8-bit indexed (CLUT) = palette index per pixel
 *   - Stored as row-major byte array, left-to-right, top-to-bottom
 *   - Row stride = w (no padding, Saturn row stride is w * 1 byte)
 *
 * TITLE.CG on the verified Saturn disc is a 32-byte zero prefix followed by
 * 0x29000 bytes. That payload is exactly 328x1024 packed 4bpp pixels.
 *
 * Failed real-media decodes leave the surface unavailable. */

#include "nexus_v1_ui_surfaces.h"
#include "nexus_v1_prs3_decode.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t nexus_ui_read_be16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static uint32_t nexus_ui_read_be32_u(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) | data[3];
}

static uint32_t nexus_ui_fnv1a32(const uint8_t *data, size_t size)
{
    uint32_t hash = 2166136261U;
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619U;
    }
    return hash;
}

static uint64_t nexus_ui_fnv1a64_append(uint64_t hash,
                                        const uint8_t *data,
                                        size_t size)
{
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= (uint64_t)data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint64_t nexus_ui_fnv1a64_append_u32(uint64_t hash, uint32_t value)
{
    uint8_t bytes[4];
    int index;

    for (index = 0; index < 4; ++index)
        bytes[index] = (uint8_t)(value >> (index * 8));
    return nexus_ui_fnv1a64_append(hash, bytes, sizeof(bytes));
}

static uint64_t nexus_ui_fnv1a64_append_u64(uint64_t hash, uint64_t value)
{
    uint8_t bytes[8];
    int index;

    for (index = 0; index < 8; ++index)
        bytes[index] = (uint8_t)(value >> (index * 8));
    return nexus_ui_fnv1a64_append(hash, bytes, sizeof(bytes));
}

static uint8_t nexus_ui_expand_5bit(uint16_t value)
{
    return (uint8_t)((value << 3) | (value >> 2));
}

/* ── Manager lifecycle ──────────────────────────────────────────── */
void nexus_ui_manager_init(Nexus_UI_Manager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

void nexus_ui_manager_free(Nexus_UI_Manager *mgr) {
    int i;
    if (!mgr) return;
    for (i = 0; i < NEXUS_SURFACE_COUNT; i++) {
        if (mgr->surfaces[i].owns_data && mgr->surfaces[i].data) {
            free(mgr->surfaces[i].data);
            mgr->surfaces[i].data = NULL;
            mgr->surfaces[i].owns_data = 0;
        }
    }
}

/* ── Surface load helper ────────────────────────────────────────── */
int nexus_ui_surface_load(Nexus_UI_Manager *mgr,
    Nexus_UISurfaceType which,
    const uint8_t *data, int data_size,
    int w, int h,
    uint8_t pal_start, uint8_t pal_count,
    const char *source)
{
    Nexus_UI_Surface *surf;

    if (!mgr || which >= NEXUS_SURFACE_COUNT) return -1;
    surf = &mgr->surfaces[which];

    /* Free previous if owned */
    if (surf->owns_data && surf->data) {
        free(surf->data);
    }
    memset(surf, 0, sizeof(*surf));

    if (!data || data_size < w * h) {
        /* Saturn startup media is all-or-nothing. A missing or short source
         * must reach the launch gate instead of becoming a plausible UI. */
        printf("Nexus UI: rejecting incomplete surface %d [%s] (%d < %d)\n",
               which, source ? source : "?", data_size, w * h);
        return -1;
    }

    surf->w = w; surf->h = h;
    surf->pal_start = pal_start;
    surf->pal_count = pal_count;
    surf->source = source;

    surf->data = (uint8_t *)malloc(w * h);
    if (surf->data) {
        surf->owns_data = 1;
        memcpy(surf->data, data, w * h);
    }

    printf("Nexus UI: surface %d [%s] loaded %dx%d "
           "(palette %d-%d) own=%d size=%d\n",
           which, source ? source : "?",
           surf->w, surf->h, surf->pal_start, surf->pal_start + surf->pal_count - 1,
           surf->owns_data, data_size);
    return (int)surf->owns_data;
}

int nexus_ui_load_bpk_runtime_surface(Nexus_UI_Manager *mgr,
    Nexus_UISurfaceType which,
    const uint8_t *archive_data, size_t archive_size,
    const Nexus_V1_BpkRuntimeSurfaceHandoff *handoff,
    uint8_t pal_start, uint8_t pal_count,
    const char *source,
    Nexus_UI_BpkImportReceipt *out_receipt)
{
    Nexus_UI_BpkImportReceipt receipt;
    Nexus_V1_BpkSurfaceEntry extracted_surface;
    uint8_t *pixels = NULL;
    size_t written = 0U;
    int extract_status;
    int load_status;

    memset(&receipt, 0, sizeof(receipt));
    if (!mgr || !archive_data || !handoff) {
        if (out_receipt) *out_receipt = receipt;
        return NEXUS_UI_BPK_IMPORT_ERR_NULL;
    }

    receipt.entry_index = handoff->entry_index;
    receipt.payload_offset = handoff->payload_offset;
    receipt.width = (int)handoff->surface.width;
    receipt.height = (int)handoff->surface.height;
    receipt.surface_class = handoff->surface.layout.surface_class;

    if (handoff->status == NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_PRS3) {
        receipt.blocked_prs3 = 1;
        if (out_receipt) *out_receipt = receipt;
        return NEXUS_UI_BPK_IMPORT_ERR_NOT_READY;
    }
    if (handoff->status == NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_TRUNCATED) {
        receipt.blocked_truncated = 1;
        if (out_receipt) *out_receipt = receipt;
        return NEXUS_UI_BPK_IMPORT_ERR_NOT_READY;
    }
    if (handoff->status != NEXUS_V1_BPK_SURFACE_HANDOFF_READY_STORED ||
        !handoff->extractable ||
        handoff->surface.layout.surface_bytes == 0U) {
        receipt.blocked_not_ready = 1;
        if (out_receipt) *out_receipt = receipt;
        return NEXUS_UI_BPK_IMPORT_ERR_NOT_READY;
    }

    pixels = (uint8_t *)malloc(handoff->surface.layout.surface_bytes);
    if (!pixels) {
        if (out_receipt) *out_receipt = receipt;
        return NEXUS_UI_BPK_IMPORT_ERR_LOAD;
    }
    memset(&extracted_surface, 0, sizeof(extracted_surface));
    extract_status = nexus_v1_bpk_archive_extract_stored_surface(
        archive_data,
        archive_size,
        handoff->entry_index,
        pixels,
        handoff->surface.layout.surface_bytes,
        &extracted_surface,
        &written);
    if (extract_status != NEXUS_V1_BPK_EXTRACT_OK ||
        written != handoff->surface.layout.surface_bytes) {
        free(pixels);
        if (out_receipt) *out_receipt = receipt;
        return NEXUS_UI_BPK_IMPORT_ERR_EXTRACT;
    }

    load_status = nexus_ui_surface_load(mgr,
                                        which,
                                        pixels,
                                        (int)written,
                                        (int)extracted_surface.width,
                                        (int)extracted_surface.height,
                                        pal_start,
                                        pal_count,
                                        source ? source : "MENU.BPK");
    free(pixels);
    if (load_status <= 0) {
        if (out_receipt) *out_receipt = receipt;
        return NEXUS_UI_BPK_IMPORT_ERR_LOAD;
    }

    receipt.loaded = 1;
    receipt.bytes_loaded = (uint32_t)written;
    receipt.width = (int)extracted_surface.width;
    receipt.height = (int)extracted_surface.height;
    receipt.surface_class = extracted_surface.layout.surface_class;
    if (out_receipt) *out_receipt = receipt;
    return NEXUS_UI_BPK_IMPORT_OK;
}

const char *nexus_ui_bpk_import_status_name(int status) {
    switch (status) {
    case NEXUS_UI_BPK_IMPORT_OK: return "ok";
    case NEXUS_UI_BPK_IMPORT_ERR_NULL: return "null";
    case NEXUS_UI_BPK_IMPORT_ERR_NOT_READY: return "not-ready";
    case NEXUS_UI_BPK_IMPORT_ERR_EXTRACT: return "extract";
    case NEXUS_UI_BPK_IMPORT_ERR_LOAD: return "load";
    default: return "unknown";
    }
}

int nexus_ui_dgt2_pp_view(const uint8_t *data,
                          size_t data_size,
                          Nexus_UI_Dgt2PpView *out_view)
{
    size_t pixels;
    int width;
    int height;

    if (!data || !out_view || data_size < 6U + 512U ||
        memcmp(data, "PP", 2) != 0) {
        return -1;
    }
    width = (int)nexus_ui_read_be16(data + 2);
    height = (int)nexus_ui_read_be16(data + 4);
    if (width <= 0 || height <= 0 ||
        (size_t)width > SIZE_MAX / (size_t)height) {
        return -1;
    }
    pixels = (size_t)width * (size_t)height;
    if (pixels > data_size - (6U + 512U)) {
        return -1;
    }
    memset(out_view, 0, sizeof(*out_view));
    out_view->clut_bgr555_be = data + 6;
    out_view->pixels = data + 6 + 512;
    out_view->pixel_bytes = pixels;
    out_view->width = width;
    out_view->height = height;
    return 0;
}

int nexus_ui_res_dgt2_pp_view(const uint8_t *data,
                              size_t data_size,
                              uint32_t resource_id,
                              Nexus_UI_Dgt2PpView *out_view)
{
    uint32_t declared_size;
    uint32_t entry_count;
    uint32_t offset = 0U;
    uint32_t next_offset = 0U;
    size_t table_size;
    uint32_t i;

    if (!data || !out_view || data_size < 12U ||
        memcmp(data, "RES*", 4) != 0) {
        return -1;
    }
    declared_size = nexus_ui_read_be32_u(data + 4);
    entry_count = nexus_ui_read_be16(data + 8);
    if (declared_size != data_size || entry_count == 0U ||
        entry_count > (data_size - 12U) / 12U) {
        return -1;
    }
    table_size = 12U + (size_t)entry_count * 12U;
    if (table_size > data_size) {
        return -1;
    }
    for (i = 0U; i < entry_count; ++i) {
        const uint8_t *entry = data + 12U + (size_t)i * 12U;
        uint32_t entry_id;
        uint32_t entry_offset;

        if (memcmp(entry, "DGT2", 4) != 0) {
            return -1;
        }
        entry_id = nexus_ui_read_be32_u(entry + 4);
        entry_offset = nexus_ui_read_be32_u(entry + 8);
        if (entry_offset < table_size || entry_offset >= data_size) {
            return -1;
        }
        if (entry_id == resource_id) {
            offset = entry_offset;
            if (i + 1U < entry_count) {
                next_offset = nexus_ui_read_be32_u(entry + 20);
            } else {
                next_offset = (uint32_t)data_size;
            }
            break;
        }
    }
    if (offset == 0U || next_offset <= offset || next_offset > data_size ||
        next_offset - offset < 8U || memcmp(data + offset, "DGT2", 4) != 0 ||
        nexus_ui_read_be32_u(data + offset + 4) != resource_id) {
        return -1;
    }
    return nexus_ui_dgt2_pp_view(data + offset + 8U,
                                 (size_t)(next_offset - offset - 8U),
                                 out_view);
}

int nexus_ui_dgt2_pp_palette_rgba(const Nexus_UI_Dgt2PpView *view,
                                  uint32_t out_palette[256])
{
    size_t i;
    if (!view || !view->clut_bgr555_be || !out_palette) {
        return -1;
    }
    for (i = 0U; i < 256U; ++i) {
        uint16_t bgr555 = nexus_ui_read_be16(view->clut_bgr555_be + i * 2U);
        uint8_t red = nexus_ui_expand_5bit((uint16_t)((bgr555 >> 10) & 0x1fU));
        uint8_t green = nexus_ui_expand_5bit((uint16_t)((bgr555 >> 5) & 0x1fU));
        uint8_t blue = nexus_ui_expand_5bit((uint16_t)(bgr555 & 0x1fU));
        out_palette[i] = 0xff000000U | ((uint32_t)red << 16) |
                         ((uint32_t)green << 8) | (uint32_t)blue;
    }
    return 0;
}

static int nexus_ui_load_dgt2_pp_surface(Nexus_UI_Manager *mgr,
                                          Nexus_UISurfaceType which,
                                          const Nexus_UI_Dgt2PpView *view,
                                          const char *source)
{
    Nexus_UI_Surface *surface;
    int result;

    if (!mgr || !view || !view->pixels || !view->clut_bgr555_be ||
        view->pixel_bytes > (size_t)INT_MAX) {
        return -1;
    }
    result = nexus_ui_surface_load(mgr, which, view->pixels,
                                   (int)view->pixel_bytes,
                                   view->width, view->height, 0, 0, source);
    if (result <= 0) {
        return result;
    }
    surface = &mgr->surfaces[which];
    if (nexus_ui_dgt2_pp_palette_rgba(view, surface->dgt2_palette_rgba) != 0) {
        nexus_ui_surface_free(mgr, which);
        return -1;
    }
    surface->dgt2_palette_fnv1a32 =
        nexus_ui_fnv1a32(view->clut_bgr555_be, 512U);
    surface->dgt2_palette_loaded = 1;
    return result;
}

/* ── Surface-specific loaders ──────────────────────────────────── */

int nexus_ui_load_title(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    uint8_t *pixels;
    size_t i;
    (void)palette;
    if (!mgr || !data || data_size != (int)NEXUS_UI_TITLE_CG_BYTES) {
        printf("Nexus UI: TITLE.CG requires the verified 328x1024 4bpp layout\n");
        return -1;
    }
    for (i = 0U; i < NEXUS_UI_TITLE_CG_HEADER_BYTES; ++i) {
        if (data[i] != 0U) {
            printf("Nexus UI: TITLE.CG requires the verified 328x1024 4bpp layout\n");
            return -1;
        }
    }
    pixels = (uint8_t *)malloc((size_t)NEXUS_UI_TITLE_CG_WIDTH *
                               (size_t)NEXUS_UI_TITLE_CG_HEIGHT);
    if (!pixels) return -1;
    for (i = 0U; i < NEXUS_UI_TITLE_CG_PACKED_BYTES; ++i) {
        uint8_t packed = data[NEXUS_UI_TITLE_CG_HEADER_BYTES + i];
        pixels[i * 2U] = (uint8_t)(packed >> 4);
        pixels[i * 2U + 1U] = (uint8_t)(packed & 0x0fU);
    }
    i = (size_t)nexus_ui_surface_load(mgr, NEXUS_SURFACE_TITLE,
                                      pixels,
                                      NEXUS_UI_TITLE_CG_WIDTH *
                                          NEXUS_UI_TITLE_CG_HEIGHT,
                                      NEXUS_UI_TITLE_CG_WIDTH,
                                      NEXUS_UI_TITLE_CG_HEIGHT,
                                      0, 16, "TITLE.CG/4bpp-atlas");
    free(pixels);
    return (int)i;
}

/* WARNING.BIN is a RES* directory of DGT2 PP images. Sega's Saturn/32X
 * Graphic References, section 6, defines PP as a 256-entry BGR555 CLUT
 * followed by one byte per pixel. Resource 0 is the initial warning plane. */
int nexus_ui_load_warning(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    Nexus_UI_Dgt2PpView view;
    (void)palette;
    if (!mgr) return -1;
    if (!data || data_size <= 0 ||
        nexus_ui_res_dgt2_pp_view(data, (size_t)data_size, 0U, &view) != 0) {
        printf("Nexus UI: WARNING.BIN requires a valid RES* container DGT2 PP image\n");
        return -1;
    }
    return nexus_ui_load_dgt2_pp_surface(mgr, NEXUS_SURFACE_WARNING,
                                         &view, "WARNING.BIN/DGT2#0");
}

/* GAMEOVER.BIN is a verified RES* DGT2 PP container, not a raw 320x200
 * bitmap. Consume only its documented pixel plane. */
int nexus_ui_load_gameover(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    Nexus_UI_Dgt2PpView view;
    (void)palette;
    if (!mgr) return -1;
    if (!data || data_size <= 0 ||
        nexus_ui_res_dgt2_pp_view(data, (size_t)data_size, 0U, &view) != 0) {
        printf("Nexus UI: GAMEOVER.BIN requires a valid RES* container DGT2 PP image\n");
        return -1;
    }
    return nexus_ui_load_dgt2_pp_surface(mgr, NEXUS_SURFACE_GAMEOVER,
                                         &view, "GAMEOVER.BIN/DGT2#0");
}

static int nexus_ui_read_be32(const uint8_t *p);

int nexus_ui_stabg_stmp_framing_receipt(const uint8_t *data,
                                        int data_size,
                                        Nexus_UI_StabgStmpFraming *out_framing)
{
    uint32_t declared;
    uint32_t dir_off, dir_len, pal_off, pal_len, pix_off, pix_len;
    uint32_t rel[NEXUS_UI_STABG_STMP_MAX_MAPS];
    int map_count = 0;
    uint32_t table_bytes;
    uint32_t max_cell = 0;
    uint32_t expected_next = 0;
    int bounded = 1;
    int i;

    if (!out_framing) return -1;
    memset(out_framing, 0, sizeof(*out_framing));
    if (!data || data_size < 0x20 || memcmp(data, "STMP", 4) != 0) return -1;
    declared = nexus_ui_read_be32(data + 4);
    if (declared != (uint32_t)data_size) return -1;

    dir_off = nexus_ui_read_be32(data + 0x08);
    dir_len = nexus_ui_read_be32(data + 0x0c);
    pal_off = nexus_ui_read_be32(data + 0x10);
    pal_len = nexus_ui_read_be32(data + 0x14);
    pix_off = nexus_ui_read_be32(data + 0x18);
    pix_len = nexus_ui_read_be32(data + 0x1c);
    /* The three region pairs must tile [0x20, EOF) exactly, with a
     * 512-byte (256 x u16) CLUT region. */
    if (dir_off != 0x20U || dir_len < 8U ||
        pal_off != dir_off + dir_len || pal_len != 512U ||
        pix_off != pal_off + pal_len ||
        pix_off + pix_len != (uint32_t)data_size) return -1;

    /* Zero-terminated u32 table of map offsets relative to dir_off. */
    for (;;) {
        uint32_t entry;
        if ((uint64_t)map_count * 4U + 4U > (uint64_t)dir_len) return -1;
        entry = nexus_ui_read_be32(data + dir_off + (uint32_t)map_count * 4U);
        if (entry == 0U) break;
        if (map_count >= NEXUS_UI_STABG_STMP_MAX_MAPS) return -1;
        rel[map_count] = entry;
        ++map_count;
    }
    if (map_count <= 0) return -1;
    table_bytes = (uint32_t)(map_count + 1) * 4U;

    /* The maps form a contiguous run right after the table and fill the
     * directory region exactly. */
    if (rel[0] != table_bytes) return -1;
    for (i = 0; i < map_count; ++i) {
        uint32_t abs_off = dir_off + rel[i];
        int w, h;
        uint32_t cells, j;
        uint32_t struct_bytes;
        if (i > 0 && rel[i] != expected_next) return -1;
        if ((uint64_t)rel[i] + 4U > (uint64_t)dir_len) return -1;
        w = (data[abs_off] << 8) | data[abs_off + 1];
        h = (data[abs_off + 2] << 8) | data[abs_off + 3];
        if (w <= 0 || h <= 0 || w > 1024 || h > 1024) return -1;
        cells = (uint32_t)(w * h);
        struct_bytes = 4U + 2U * cells;
        if ((uint64_t)rel[i] + struct_bytes > (uint64_t)dir_len) return -1;
        for (j = 0; j < cells; ++j) {
            uint32_t cell = (uint32_t)((data[abs_off + 4 + 2 * j] << 8) |
                                       data[abs_off + 5 + 2 * j]);
            if (cell > max_cell) max_cell = cell;
            if ((uint64_t)cell * 2U + 4U > (uint64_t)pix_len) bounded = 0;
        }
        if (i == 0) {
            out_framing->background_cell_w = w;
            out_framing->background_cell_h = h;
        }
        expected_next = rel[i] + struct_bytes;
    }
    if (expected_next != dir_len) return -1;
    if (!bounded) return -1;

    out_framing->valid = 1;
    out_framing->declared_size = declared;
    out_framing->directory_offset = dir_off;
    out_framing->directory_size = dir_len;
    out_framing->palette_offset = pal_off;
    out_framing->palette_size = pal_len;
    out_framing->pixels_offset = pix_off;
    out_framing->pixels_size = pix_len;
    out_framing->map_count = map_count;
    out_framing->max_cell_word_offset = max_cell;
    out_framing->cell_offsets_bounded = 1;
    return 0;
}

/* STABG.BIN has verified source identity and, since the STMP framing
 * receipt above, a proven container layout. The pixel-unit decode
 * semantics of the tile-map cells remain unproven, so no surface may be
 * materialized yet: file-size-derived dimensions were synthetic and are
 * forbidden. */
int nexus_ui_load_stabg(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    (void)mgr; (void)data; (void)data_size; (void)palette;
    return -1;
}

int nexus_ui_face_full_entry_count(int data_size, int portrait_w, int portrait_h) {
    /* FACE.BIN is a variable-length PRS3 container, not a raw portrait
     * table. Keep this legacy query inert so arbitrary bytes cannot admit a
     * 48x48 surface through an old caller. */
    (void)data_size;
    (void)portrait_w;
    (void)portrait_h;
    return 0;
}

static int nexus_ui_read_be32(const uint8_t *p) {
    if (!p) return 0;
    return ((int)p[0] << 24) |
           ((int)p[1] << 16) |
           ((int)p[2] << 8) |
           (int)p[3];
}

enum {
    NEXUS_UI_FACE_HEADER_BYTES = 56,
    NEXUS_UI_FACE_COMPACT_PREFIX_BYTES = 128,
    NEXUS_UI_FACE_PRS3_HEADER_BYTES = 16,
    NEXUS_UI_FACE_CANONICAL_FRAME_COUNT = 20,
    NEXUS_UI_FACE_CANONICAL_PIXEL_COUNT = 56 * 56
};

static int nexus_ui_face_compact_walk(const uint8_t *data, int data_size,
                                      int wanted_index,
                                      Nexus_UI_FaceCompactRecordDescriptor *out)
{
    size_t cursor = NEXUS_UI_FACE_HEADER_BYTES;
    int index;

    if (out) memset(out, 0, sizeof(*out));
    if (!data || data_size < NEXUS_UI_FACE_HEADER_BYTES ||
        memcmp(data, "FACE", 4) != 0 ||
        nexus_ui_read_be32(data + 4) != data_size ||
        data[8] != 0 || data[9] != NEXUS_UI_FACE_CANONICAL_FRAME_COUNT) {
        return 0;
    }
    for (index = 0; index < NEXUS_UI_FACE_CANONICAL_FRAME_COUNT; ++index) {
        size_t prs3_offset;
        size_t stream_size;
        size_t frame_end;
        if (cursor > (size_t)data_size ||
            (size_t)data_size - cursor < NEXUS_UI_FACE_COMPACT_PREFIX_BYTES +
                                      NEXUS_UI_FACE_PRS3_HEADER_BYTES) {
            return 0;
        }
        prs3_offset = cursor + NEXUS_UI_FACE_COMPACT_PREFIX_BYTES;
        prs3_offset = (prs3_offset + 3U) & ~(size_t)3U;
        if (memcmp(data + prs3_offset, "PRS3", 4) != 0 ||
            nexus_ui_read_be32(data + prs3_offset + 4) != 1 ||
            nexus_ui_read_be32(data + prs3_offset + 8) !=
                NEXUS_UI_FACE_CANONICAL_PIXEL_COUNT) {
            return 0;
        }
        stream_size = (size_t)nexus_ui_read_be32(data + prs3_offset + 12);
        if (stream_size == 0 || stream_size > (size_t)data_size - prs3_offset -
                                               NEXUS_UI_FACE_PRS3_HEADER_BYTES) {
            return 0;
        }
        frame_end = prs3_offset + NEXUS_UI_FACE_PRS3_HEADER_BYTES + stream_size;
        if (index == wanted_index && out) {
            out->valid = 1;
            out->face_index = index;
            out->prefix_offset = cursor;
            out->prefix_size = prs3_offset - cursor;
            out->prs3_offset = prs3_offset;
            out->prs3_size = NEXUS_UI_FACE_PRS3_HEADER_BYTES + stream_size;
            out->stream_offset = prs3_offset + NEXUS_UI_FACE_PRS3_HEADER_BYTES;
            out->stream_size = stream_size;
            out->prs3_version = (uint32_t)nexus_ui_read_be32(data + prs3_offset + 4);
            out->declared_pixel_count = NEXUS_UI_FACE_CANONICAL_PIXEL_COUNT;
        }
        cursor = frame_end;
    }
    /* The canonical file has a two-byte zero tail after the final stream.
     * It is container alignment, never PRS3 input or portrait output. */
    return cursor + 2U == (size_t)data_size &&
           data[cursor] == 0U && data[cursor + 1U] == 0U;
}

int nexus_ui_face_layout_detect(const uint8_t *data,
    int data_size,
    Nexus_UI_FaceLayout *out_layout)
{
    Nexus_UI_FaceLayout layout;
    memset(&layout, 0, sizeof(layout));
    layout.portrait_w = 48;
    layout.portrait_h = 48;
    if (!data || data_size <= 0) {
        if (out_layout) *out_layout = layout;
        return 0;
    }
    if (nexus_ui_face_compact_walk(data, data_size, -1, NULL)) {
        layout.valid = 1;
        layout.header_size = NEXUS_UI_FACE_HEADER_BYTES;
        layout.entry_count = NEXUS_UI_FACE_CANONICAL_FRAME_COUNT;
        layout.entry_size = 0; /* Frames are variable-length PRS3 spans. */
        layout.portrait_w = 56;
        layout.portrait_h = 56;
        if (out_layout) *out_layout = layout;
        return 1;
    }
    /* No alternate raw-table form is evidenced for retail FACE.BIN. */
    if (out_layout) *out_layout = layout;
    return 0;
}

int nexus_ui_face_compact_record_descriptor(const uint8_t *data,
    int data_size,
    int face_index,
    Nexus_UI_FaceCompactRecordDescriptor *out_descriptor)
{
    if (face_index < 0 || face_index >= NEXUS_UI_FACE_CANONICAL_FRAME_COUNT) {
        if (out_descriptor) memset(out_descriptor, 0, sizeof(*out_descriptor));
        return 0;
    }
    return nexus_ui_face_compact_walk(data, data_size, face_index, out_descriptor);
}

int nexus_ui_face_prs3_corpus_receipt(const uint8_t *data,
                                      int data_size,
                                      Nexus_UI_FacePrs3CorpusReceipt *out_receipt)
{
    Nexus_UI_FacePrs3CorpusReceipt receipt;
    int index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!data || data_size <= 0 ||
        !nexus_ui_face_compact_walk(data, data_size, -1, NULL)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_byte_count = (size_t)data_size;
    receipt.source_bytes_fnv1a64 = nexus_ui_fnv1a64_append(
        UINT64_C(14695981039346656037), data, receipt.source_byte_count);
    receipt.prs3_headers_fnv1a64 = UINT64_C(14695981039346656037);
    receipt.stream_bytes_fnv1a64 = UINT64_C(14695981039346656037);
    for (index = 0; index < NEXUS_UI_FACE_CANONICAL_FRAME_COUNT; ++index) {
        Nexus_UI_FaceCompactRecordDescriptor descriptor;
        if (!nexus_ui_face_compact_record_descriptor(data, data_size, index,
                                                     &descriptor) ||
            !descriptor.valid || descriptor.prs3_offset > (size_t)data_size ||
            descriptor.prs3_size > (size_t)data_size - descriptor.prs3_offset ||
            descriptor.stream_offset > (size_t)data_size ||
            descriptor.stream_size > (size_t)data_size - descriptor.stream_offset) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.no_draw_only = 1;
            *out_receipt = receipt;
            return 0;
        }
        receipt.prs3_headers_fnv1a64 = nexus_ui_fnv1a64_append(
            receipt.prs3_headers_fnv1a64, data + descriptor.prs3_offset,
            NEXUS_UI_FACE_PRS3_HEADER_BYTES);
        receipt.stream_bytes_fnv1a64 = nexus_ui_fnv1a64_append(
            receipt.stream_bytes_fnv1a64, data + descriptor.stream_offset,
            descriptor.stream_size);
        receipt.total_stream_byte_count += descriptor.stream_size;
        receipt.declared_total_pixel_count += descriptor.declared_pixel_count;
        ++receipt.frame_count;
    }
    receipt.valid = receipt.frame_count == NEXUS_UI_FACE_CANONICAL_FRAME_COUNT;
    *out_receipt = receipt;
    return receipt.valid ? 1 : 0;
}

int nexus_ui_face_prs3_capture_target(const uint8_t *data,
                                      int data_size,
                                      int face_index,
                                      int source_hash_verified,
                                      Nexus_UI_FacePrs3CaptureTarget *out_target)
{
    Nexus_UI_FacePrs3CaptureTarget target;

    if (!out_target) return -1;
    memset(&target, 0, sizeof(target));
    target.face_index = -1;
    target.no_draw_only = 1;
    if (!data || data_size <= 0 || !source_hash_verified ||
        !nexus_ui_face_compact_record_descriptor(data, data_size, face_index,
                                                 &target.descriptor) ||
        !target.descriptor.valid ||
        target.descriptor.prefix_offset > (size_t)data_size ||
        target.descriptor.prefix_size >
            (size_t)data_size - target.descriptor.prefix_offset ||
        target.descriptor.prs3_offset > (size_t)data_size ||
        target.descriptor.prs3_size >
            (size_t)data_size - target.descriptor.prs3_offset ||
        target.descriptor.stream_offset > (size_t)data_size ||
        target.descriptor.stream_size >
            (size_t)data_size - target.descriptor.stream_offset) {
        *out_target = target;
        return 0;
    }
    target.face_index = face_index;
    target.source_byte_count = (size_t)data_size;
    target.source_bytes_fnv1a64 = nexus_ui_fnv1a64_append(
        UINT64_C(14695981039346656037), data, target.source_byte_count);
    target.prefix_bytes_fnv1a64 = nexus_ui_fnv1a64_append(
        UINT64_C(14695981039346656037),
        data + target.descriptor.prefix_offset, target.descriptor.prefix_size);
    target.prs3_header_fnv1a64 = nexus_ui_fnv1a64_append(
        UINT64_C(14695981039346656037),
        data + target.descriptor.prs3_offset, NEXUS_UI_FACE_PRS3_HEADER_BYTES);
    target.stream_bytes_fnv1a64 = nexus_ui_fnv1a64_append(
        UINT64_C(14695981039346656037),
        data + target.descriptor.stream_offset, target.descriptor.stream_size);
    target.capture_producer_required = 1;
    target.original_saturn_capture_required = 1;
    target.valid = target.source_bytes_fnv1a64 != 0U &&
        target.prefix_bytes_fnv1a64 != 0U &&
        target.prs3_header_fnv1a64 != 0U &&
        target.stream_bytes_fnv1a64 != 0U;
    *out_target = target;
    return target.valid ? 1 : 0;
}

int nexus_ui_face_prs3_capture_campaign(
    const uint8_t *data,
    int data_size,
    int source_hash_verified,
    Nexus_UI_FacePrs3CaptureCampaignReceipt *out_receipt)
{
    Nexus_UI_FacePrs3CaptureCampaignReceipt receipt;
    int index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!data || data_size <= 0 || !source_hash_verified) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_byte_count = (size_t)data_size;
    receipt.source_bytes_fnv1a64 = nexus_ui_fnv1a64_append(
        UINT64_C(14695981039346656037), data, receipt.source_byte_count);
    receipt.ordered_target_fnv1a64 = UINT64_C(14695981039346656037);
    receipt.source_lanes_fnv1a64 = UINT64_C(14695981039346656037);
    for (index = 0; index < NEXUS_UI_FACE_CANONICAL_FRAME_COUNT; ++index) {
        Nexus_UI_FacePrs3CaptureTarget target;

        if (nexus_ui_face_prs3_capture_target(data, data_size, index, 1,
                                               &target) != 1 ||
            !target.valid || !target.capture_producer_required ||
            !target.original_saturn_capture_required ||
            target.decoder_permitted || !target.no_draw_only ||
            target.fallback_visuals_permitted) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.no_draw_only = 1;
            *out_receipt = receipt;
            return 0;
        }
        receipt.ordered_target_fnv1a64 = nexus_ui_fnv1a64_append_u32(
            receipt.ordered_target_fnv1a64, (uint32_t)target.face_index);
        receipt.ordered_target_fnv1a64 = nexus_ui_fnv1a64_append_u64(
            receipt.ordered_target_fnv1a64, target.descriptor.prs3_offset);
        receipt.ordered_target_fnv1a64 = nexus_ui_fnv1a64_append_u64(
            receipt.ordered_target_fnv1a64, target.descriptor.stream_size);
        receipt.ordered_target_fnv1a64 = nexus_ui_fnv1a64_append_u64(
            receipt.ordered_target_fnv1a64, target.prs3_header_fnv1a64);
        receipt.source_lanes_fnv1a64 = nexus_ui_fnv1a64_append_u64(
            receipt.source_lanes_fnv1a64, target.prefix_bytes_fnv1a64);
        receipt.source_lanes_fnv1a64 = nexus_ui_fnv1a64_append_u64(
            receipt.source_lanes_fnv1a64, target.stream_bytes_fnv1a64);
        if (receipt.total_stream_byte_count > SIZE_MAX -
            target.descriptor.stream_size) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.no_draw_only = 1;
            *out_receipt = receipt;
            return 0;
        }
        receipt.total_stream_byte_count += target.descriptor.stream_size;
        ++receipt.frame_count;
    }
    receipt.capture_producer_required = 1;
    receipt.original_saturn_capture_required = 1;
    receipt.valid = receipt.frame_count == NEXUS_UI_FACE_CANONICAL_FRAME_COUNT &&
        receipt.source_bytes_fnv1a64 != 0U &&
        receipt.ordered_target_fnv1a64 != UINT64_C(14695981039346656037) &&
        receipt.source_lanes_fnv1a64 != UINT64_C(14695981039346656037) &&
        receipt.total_stream_byte_count != 0U;
    *out_receipt = receipt;
    return receipt.valid ? 1 : 0;
}

int nexus_ui_expand_face_record_48x48(const uint8_t *record_data,
    int record_size,
    uint8_t *out_pixels,
    int out_size,
    Nexus_UI_FaceRecordDecodeInfo *out_info)
{
    Nexus_UI_FaceRecordDecodeInfo info;
    memset(&info, 0, sizeof(info));
    info.kind = NEXUS_UI_FACE_RECORD_NONE;
    info.source_size = record_size;
    info.portrait_w = 48;
    info.portrait_h = 48;
    if (!out_pixels || out_size <= 0) {
        if (out_info) *out_info = info;
        return -1;
    }
    if (!record_data || record_size <= 0) {
        if (out_info) *out_info = info;
        return 0;
    }
    if (record_size >= NEXUS_UI_FACE_PRS3_HEADER_BYTES &&
        memcmp(record_data, "PRS3", 4) == 0) {
        Nexus_V1_Prs3Header hdr;
        if (nexus_v1_prs3_parse_header(record_data, record_size, &hdr) &&
            (int)hdr.uncompressed_size <= out_size) {
            Nexus_V1_Prs3DecodeResult dr = nexus_v1_prs3_decompress(
                hdr.stream, (int)hdr.compressed_size,
                out_pixels, out_size, (int)hdr.uncompressed_size);
            if (dr.success) {
                info.kind = NEXUS_UI_FACE_RECORD_PRS3_DECODED;
                if (out_info) *out_info = info;
                return 1;
            }
        }
        info.kind = NEXUS_UI_FACE_RECORD_PRS3_UNPROVEN;
        if (out_info) *out_info = info;
        return 0;
    }
    if (out_info) *out_info = info;
    return 0;
}

int nexus_ui_load_face_record(Nexus_UI_Manager *mgr,
    const uint8_t *record_data,
    int record_size,
    int face_index,
    int portrait_w,
    int portrait_h,
    const uint32_t *palette)
{
    int entry_size;
    int expand_result;
    Nexus_UI_Surface *surf;
    (void)palette;
    if (!mgr) return -1;
    if (face_index < 0 || face_index >= 24) return -1;
    if (portrait_w <= 0 || portrait_h <= 0) {
        portrait_w = 48; portrait_h = 48;
    }
    entry_size = portrait_w * portrait_h;
    if (entry_size <= 0 || !record_data || record_size <= 0) {
        return -1;
    }
    surf = &mgr->surfaces[NEXUS_SURFACE_FACE0 + face_index];
    if (surf->owns_data && surf->data) {
        free(surf->data);
    }
    memset(surf, 0, sizeof(*surf));
    surf->data = (uint8_t *)calloc(entry_size, 1);
    if (!surf->data) {
        return -1;
    }
    surf->owns_data = 1;
    expand_result = nexus_ui_expand_face_record_48x48(record_data,
                                                      record_size,
                                                      surf->data,
                                                      entry_size,
                                                      NULL);
    if (expand_result <= 0) {
        free(surf->data);
        memset(surf, 0, sizeof(*surf));
        return -1;
    }
    surf->w = portrait_w;
    surf->h = portrait_h;
    surf->pal_start = 192;
    surf->pal_count = 16;
    surf->source = "FACE.BIN";
    surf->owns_data = 1;
    return 1;
}

/* Generic raw-face helper. Real FACE.BIN records must be complete: a short
 * record is a decode failure, never a gray/prefix-filled portrait. */
int nexus_ui_load_faces(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_of_face,
    int data_size, int face_index,
    int portrait_w, int portrait_h,
    const uint32_t *palette)
{
    int entry_size;
    int copied_real = 0;
    Nexus_UI_Surface *surf;
    (void)palette;

    if (!mgr) return -1;
    if (face_index < 0 || face_index >= 24) return -1;
    /* Canonical FACE.BIN is a compact PRS3 container, never a raw portrait
     * atlas. Its undecoded bytes must not enter this legacy raw helper. */
    if (data && data_size >= 4 && memcmp(data, "FACE", 4) == 0) return -1;
    if (portrait_w <= 0 || portrait_h <= 0) {
        portrait_w = 48; portrait_h = 48;  /* default assumed size */
    }
    entry_size = portrait_w * portrait_h;

    surf = &mgr->surfaces[NEXUS_SURFACE_FACE0 + face_index];
    if (surf->owns_data && surf->data) free(surf->data);
    memset(surf, 0, sizeof(*surf));

    surf->w = portrait_w;
    surf->h = portrait_h;
    surf->pal_start = 192;  /* UI palette block for faces */
    surf->pal_count = 16;
    surf->source = "FACE.BIN";
    surf->owns_data = 1;

    if (data && data_size >= data_of_face + entry_size) {
        surf->data = (uint8_t *)malloc(entry_size);
        if (surf->data) {
            memcpy(surf->data, data + data_of_face, entry_size);
            copied_real = 1;
        } else {
            surf->data = (uint8_t *)calloc(entry_size, 1);
        }
    } else {
        return -1;
    }
    /* Face surfaces are stored individually in the manager */
    return copied_real ? 1 : -1;
}

void nexus_ui_surface_free(Nexus_UI_Manager *mgr,
    Nexus_UISurfaceType which)
{
    if (!mgr || which >= NEXUS_SURFACE_COUNT) return;
    if (mgr->surfaces[which].owns_data
        && mgr->surfaces[which].data) {
        free(mgr->surfaces[which].data);
    }
    memset(&mgr->surfaces[which], 0, sizeof(Nexus_UI_Surface));
}

/* ── Blit primitives ─────────────────────────────────────────────── */

/* Simple 1:1 clip blit from surface → indexed framebuffer.
 * Source-lock: ReDMCSB BLIT.C F0132 (F0132 pixel copy).        */
void nexus_ui_blit_surface(const Nexus_UI_Surface *surf,
    uint8_t *fb, int fb_w, int fb_h, int dx, int dy)
{
    int row, col;
    if (!surf || !fb || !surf->data) return;
    /* Clip destination */
    if (dx < 0) { dx = 0; }
    if (dy < 0) { dy = 0; }
    if (dx + surf->w > fb_w)  dx = fb_w - surf->w;
    if (dy + surf->h > fb_h)  dy = fb_h - surf->h;
    if (dx < 0 || dy < 0 || dx + surf->w > fb_w
        || dy + surf->h > fb_h) return;

    for (row = 0; row < surf->h; row++) {
        int sy = dy + row;
        if (sy < 0 || sy >= fb_h) continue;
        for (col = 0; col < surf->w; col++) {
            int sx = dx + col;
            if (sx < 0 || sx >= fb_w) continue;
            fb[sy * fb_w + sx] = surf->data[row * surf->w + col];
        }
    }
}

/* Same but with optional horizontal flip (champion mirror) */
void nexus_ui_blit_surface_flip(const Nexus_UI_Surface *surf,
    uint8_t *fb, int fb_w, int fb_h,
    int dx, int dy, int flip_h)
{
    int row, col;
    if (!surf || !fb || !surf->data) return;
    if (dx < 0) dx = 0;
    if (dy < 0) dy = 0;
    if (dx + surf->w > fb_w) dx = fb_w - surf->w;
    if (dy + surf->h > fb_h) dy = fb_h - surf->h;
    if (dx < 0 || dy < 0) return;

    for (row = 0; row < surf->h; row++) {
        int sy = dy + row;
        if (sy < 0 || sy >= fb_h) continue;
        for (col = 0; col < surf->w; col++) {
            int sx = dx + col;
            int src_col = flip_h ? (surf->w - 1 - col) : col;
            if (sx < 0 || sx >= fb_w) continue;
            fb[sy * fb_w + sx] = surf->data[row * surf->w + src_col];
        }
    }
}

/* ── Convenience render wrappers ─────────────────────────────── */
void nexus_ui_render_title(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h)
{
    if (!mgr || !fb) return;
    nexus_ui_blit_surface(&mgr->surfaces[NEXUS_SURFACE_TITLE],
        fb, fb_w, fb_h, 0, 0);
}

void nexus_ui_render_warning(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h)
{
    if (!mgr || !fb) return;
    nexus_ui_blit_surface(&mgr->surfaces[NEXUS_SURFACE_WARNING],
        fb, fb_w, fb_h, 0, 0);
}

void nexus_ui_render_gameover(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h)
{
    if (!mgr || !fb) return;
    nexus_ui_blit_surface(&mgr->surfaces[NEXUS_SURFACE_GAMEOVER],
        fb, fb_w, fb_h, 0, 0);
}

void nexus_ui_render_stabg(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h, int dest_x, int dest_y)
{
    if (!mgr || !fb) return;
    nexus_ui_blit_surface(&mgr->surfaces[NEXUS_SURFACE_STABG],
        fb, fb_w, fb_h, dest_x, dest_y);
}

void nexus_ui_render_portrait(const Nexus_UI_Manager *mgr,
    int portrait_index, uint8_t *fb, int fb_w, int fb_h,
    int dest_x, int dest_y, int flip_h)
{
    Nexus_UISurfaceType which;
    if (!mgr || !fb) return;
    if (portrait_index < 0 || portrait_index >= 24) return;
    which = NEXUS_SURFACE_FACE0 + portrait_index;
    nexus_ui_blit_surface_flip(&mgr->surfaces[which],
        fb, fb_w, fb_h, dest_x, dest_y, flip_h);
}

/* ── Palette remap ─────────────────────────────────────────────── */
void nexus_ui_surface_remap_pal(Nexus_UI_Surface *surf,
    uint8_t new_pal_start)
{
    int i;
    if (!surf || !surf->data) return;
    /* Shift all pixel values by (new - old) palette start */
    int delta = (int)(new_pal_start - surf->pal_start);
    if (delta == 0) return;
    for (i = 0; i < surf->w * surf->h; i++) {
        int v = (int)surf->data[i] + delta;
        surf->data[i] = (v < 0) ? 0 : (v > 255) ? 255 : (uint8_t)v;
    }
    surf->pal_start = new_pal_start;
}

/* Darken surface in-place for blur/focus states */
void nexus_ui_surface_darken(Nexus_UI_Surface *surf, float factor) {
    /* For indexed surfaces, we darken by halving palette indices.
     * A real implementation would compose with the brightness in the
     * V1 framebuffer pipeline.  Here we simply darken each pixel's
     * index toward 0 (0 stays 0 = black).                         */
    int i;
    if (!surf || !surf->data) return;
    if (factor <= 0.0f) factor = 0.5f;
    for (i = 0; i < surf->w * surf->h; i++) {
        int v = (int)((uint8_t)surf->data[i] * factor);
        surf->data[i] = (v < 0) ? 0 : (uint8_t)v;
    }
}

#if defined(__GNUC__) && !defined(__clang__)
/* suppress unused parameter warnings */
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
