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
        surf->data = NULL;
        surf->owns_data = 0;
    }

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
    return nexus_ui_surface_load(mgr, NEXUS_SURFACE_WARNING,
        view.pixels, (int)view.pixel_bytes,
        view.width, view.height, 0, 0, "WARNING.BIN/DGT2#0");
}

/* GAMEOVER.BIN (101 KB) — simple indexed 320×200 */
int nexus_ui_load_gameover(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    (void)palette;
    if (!mgr) return -1;
    return nexus_ui_surface_load(mgr, NEXUS_SURFACE_GAMEOVER,
        data, data_size, 320, 200, 128, 64, "GAMEOVER.BIN");
}

/* STABG.BIN (52 KB) — status area background.
 * May be 320×200 or a narrower horizontal strip. */
int nexus_ui_load_stabg(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette)
{
    (void)palette;
    if (!mgr) return -1;
    /* 52 KB / 320 ≈ 170 rows; treat as 320×170 or fall back to 320×200 */
    if (data_size >= 320 * 200) {
        return nexus_ui_surface_load(mgr, NEXUS_SURFACE_STABG,
            data, data_size, 320, 200, 0, 16, "STABG.BIN");
    } else if (data_size >= 320 * 50) {
        return nexus_ui_surface_load(mgr, NEXUS_SURFACE_STABG,
            data, data_size, 320, 52, 0, 16, "STABG.BIN");
    }
    return nexus_ui_surface_load(mgr, NEXUS_SURFACE_STABG,
        data, data_size, 320, 200, 0, 16, "STABG.BIN");
}

int nexus_ui_face_full_entry_count(int data_size, int portrait_w, int portrait_h) {
    int entry_size;
    int count;
    if (portrait_w <= 0 || portrait_h <= 0 || data_size <= 0) {
        return 0;
    }
    entry_size = portrait_w * portrait_h;
    if (entry_size <= 0) {
        return 0;
    }
    count = data_size / entry_size;
    if (count < 0) return 0;
    if (count > 24) return 24;
    return count;
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
    layout.valid = 1;
    layout.header_size = 0;
    layout.entry_count = nexus_ui_face_full_entry_count(data_size, 48, 48);
    layout.entry_size = 48 * 48;
    if (out_layout) *out_layout = layout;
    return layout.entry_count > 0;
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

int nexus_ui_expand_face_record_48x48(const uint8_t *record_data,
    int record_size,
    uint8_t *out_pixels,
    int out_size,
    Nexus_UI_FaceRecordDecodeInfo *out_info)
{
    Nexus_UI_FaceRecordDecodeInfo info;
    const int entry_size = 48 * 48;
    memset(&info, 0, sizeof(info));
    info.kind = NEXUS_UI_FACE_RECORD_NONE;
    info.source_size = record_size;
    info.portrait_w = 48;
    info.portrait_h = 48;
    if (!out_pixels || out_size < entry_size) {
        if (out_info) *out_info = info;
        return -1;
    }
    if (!record_data || record_size <= 0) {
        if (out_info) *out_info = info;
        return 0;
    }
    if (record_size >= NEXUS_UI_FACE_PRS3_HEADER_BYTES &&
        memcmp(record_data, "PRS3", 4) == 0) {
        /* The descriptor has proven a bounded PRS3 frame, not its opcode
         * grammar. Keeping this blocked protects the real portrait route. */
        info.kind = NEXUS_UI_FACE_RECORD_PRS3_UNPROVEN;
        if (out_info) *out_info = info;
        return 0;
    }
    if (record_size != entry_size) {
        if (out_info) *out_info = info;
        return 0;
    }
    memcpy(out_pixels, record_data, (size_t)entry_size);
    info.copied_pixels = entry_size;
    info.kind = NEXUS_UI_FACE_RECORD_RAW_48X48;
    if (out_info) *out_info = info;
    return 1;
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
