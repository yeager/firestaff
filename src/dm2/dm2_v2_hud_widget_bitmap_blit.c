/*
 * dm2_v2_hud_widget_bitmap_blit.c — DM2 V2 HUD Widget bounded bitmap blit
 *
 * Implementation. Companion to include/dm2_v2_hud_widget_bitmap_blit.h.
 *
 * The blit path is intentionally tiny: it only handles the synthetic-
 * test PNG envelope that examples/dm2_hud_widget_synthetic/ already
 * ships (1x1, 8-bit, color type 6 RGBA, single IDAT, zlib deflate).
 * Anything outside that envelope returns 0 so the caller can fall
 * back to the legacy 1-pixel anchor stamp and the V1 chrome
 * fallback stays byte-identical to the no-gate baseline.
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - PNG specification (W3C / ISO 15948):
 *       8-byte signature → IHDR (13 bytes data) → optional ancillary
 *       chunks → one or more IDAT (zlib-wrapped deflate) → IEND.
 *     For 1x1 RGBA, the raw IDAT payload is exactly 5 bytes:
 *     one filter byte (always 0 = None for a 1x1 image) + 4 RGBA
 *     bytes.
 *   - examples/dm2_hud_widget_synthetic/ (synthetic 1x1 RGBA fixtures)
 *   - include/dm2_v2_hud_widget_assets.h (slot gate this module reads)
 *   - include/dm2_v2_hud_runtime.h (runtime hook this module extends)
 */

#include "dm2_v2_hud_widget_bitmap_blit.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef FIRESTAFF_HAS_ZLIB
#include <zlib.h>
#endif

/* ── PNG constants (W3C / ISO 15948) ────────────────────────────── */
static const unsigned char k_png_signature[8] = {
    0x89U, 0x50U, 0x4EU, 0x47U, 0x0DU, 0x0AU, 0x1AU, 0x0AU
};

/* ── Internal helpers ───────────────────────────────────────────── */

static void dm2_v2_hwb_zero_pixel(DM2_V2_HudWidgetBlitPixel* out) {
    if (!out) return;
    memset(out, 0, sizeof(*out));
}

/* Read big-endian uint32 from a buffer (no alignment requirements). */
static uint32_t dm2_v2_hwb_read_be32(const unsigned char* p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |
            (uint32_t)p[3];
}

/* ── PNG signature check ────────────────────────────────────────── */
static int dm2_v2_hwb_check_signature(const unsigned char* hdr) {
    if (!hdr) return 0;
    return memcmp(hdr, k_png_signature, 8) == 0 ? 1 : 0;
}

/* ── Minimal PNG chunk walker ─────────────────────────────────────
 *
 * Scans the bytes from `start`..`end` for IHDR / IDAT / IEND chunks.
 * All other chunks (tEXt, gAMA, pHYs, etc.) are skipped — the
 * synthetic fixtures only carry a tEXt "synthetic-test-fixture"
 * chunk between IHDR and IDAT, and the bounded-blit envelope does
 * not need to decode those. We track a sum of every IDAT chunk
 * length so the caller can inflate a single contiguous buffer.
 *
 * The walker is intentionally strict:
 *   - chunk length must be >= 0
 *   - chunk type must be exactly 4 ASCII bytes (no leading CRCs, no
 *     length-in-length confusion)
 *   - next chunk offset = current + 8 (length + type) + length + 4 (CRC)
 *   - stops cleanly at IEND
 *
 * Returns 1 on a clean walk that found IHDR, IDAT, and IEND; 0 on
 * any structural error (truncated file, malformed length, missing
 * chunk, IHDR not first). */
typedef struct {
    int      found_ihdr;
    int      found_idat;
    int      found_iend;

    /* IHDR fields */
    uint32_t ihdr_width;
    uint32_t ihdr_height;
    uint8_t  ihdr_bit_depth;
    uint8_t  ihdr_color_type;

    /* IDAT region: offset and total concatenated length inside the
     * raw file buffer. The walker collects every IDAT chunk in
     * order; the caller inflates from this region. */
    size_t   idat_offset;
    size_t   idat_total_len;
} DM2_V2_HwbChunkWalk;

static int dm2_v2_hwb_walk_chunks(const unsigned char* buf, size_t buf_len,
                                   DM2_V2_HwbChunkWalk* out) {
    if (!buf || !out || buf_len < 8U) return 0;
    memset(out, 0, sizeof(*out));

    /* After the 8-byte signature the very first chunk MUST be IHDR
     * per the PNG spec. We enforce that strictly. */
    size_t pos = 8U;
    if (pos + 8U + 13U > buf_len) return 0;

    uint32_t len0 = dm2_v2_hwb_read_be32(buf + pos);
    if (len0 != 13U) return 0;
    if (memcmp(buf + pos + 4, "IHDR", 4) != 0) return 0;
    /* Parse IHDR fields (always 13 data bytes). */
    out->ihdr_width      = dm2_v2_hwb_read_be32(buf + pos + 8);
    out->ihdr_height     = dm2_v2_hwb_read_be32(buf + pos + 12);
    out->ihdr_bit_depth  = buf[pos + 16];
    out->ihdr_color_type = buf[pos + 17];
    out->found_ihdr = 1;
    pos += 8U + 13U + 4U; /* len + type + data + crc */

    /* Walk subsequent chunks until IEND or error. */
    while (pos + 8U <= buf_len) {
        uint32_t len = dm2_v2_hwb_read_be32(buf + pos);
        const unsigned char* type = buf + pos + 4;
        if (memcmp(type, "IDAT", 4) == 0) {
            if (len > buf_len - pos - 8U) return 0; /* truncated */
            if (!out->found_idat) {
                out->idat_offset = pos + 8U; /* first IDAT data start */
                out->idat_total_len = len;
            } else {
                out->idat_total_len += len;
            }
            out->found_idat = 1;
            pos += 8U + len + 4U;
        } else if (memcmp(type, "IEND", 4) == 0) {
            out->found_iend = 1;
            pos += 8U + len + 4U;
            break;
        } else {
            /* Ancillary chunk (tEXt, gAMA, pHYs, ...): skip. */
            if (len > buf_len - pos - 8U) return 0;
            pos += 8U + len + 4U;
        }
    }

    return (out->found_ihdr && out->found_idat && out->found_iend) ? 1 : 0;
}

/* ── Read a 1x1 RGBA pixel from a PNG file ─────────────────────── */
int dm2_v2_hud_widget_bitmap_blit_read_pixel(
    const char* path,
    DM2_V2_HudWidgetBlitPixel* out_pixel)
{
    if (out_pixel) dm2_v2_hwb_zero_pixel(out_pixel);
    if (!path || path[0] == '\0' || !out_pixel) return 0;

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    /* Read the entire file. Bounded by DM2_V2_HUD_WIDGET_BLIT_MAX_FILE_BYTES
     * so a pathologically large file cannot blow up the runtime. */
    unsigned char* buf = (unsigned char*)malloc(
        DM2_V2_HUD_WIDGET_BLIT_MAX_FILE_BYTES);
    if (!buf) { fclose(fp); return 0; }

    size_t n = fread(buf, 1U, DM2_V2_HUD_WIDGET_BLIT_MAX_FILE_BYTES, fp);
    fclose(fp);
    if (n < 8U + 8U + 13U + 4U) { /* signature + IHDR + minimum trailing */
        free(buf);
        return 0;
    }

    /* Signature must match exactly. */
    if (!dm2_v2_hwb_check_signature(buf)) {
        free(buf);
        return 0;
    }

    /* Walk chunks. Strict — any structural error returns 0. */
    DM2_V2_HwbChunkWalk walk;
    if (!dm2_v2_hwb_walk_chunks(buf, n, &walk)) {
        free(buf);
        return 0;
    }

    /* Enforce the synthetic-blit envelope:
     *   width  ∈ [1, DM2_V2_HUD_WIDGET_BLIT_MAX_WIDTH]
     *   height ∈ [1, DM2_V2_HUD_WIDGET_BLIT_MAX_HEIGHT]
     *   bit depth == 8
     *   color type == 6 (RGBA)
     * Anything else is "operator-installed real art" territory and is
     * explicitly rejected here so the bounded-blit path cannot silently
     * fall into a multi-pixel decode. */
    if (walk.ihdr_width  == 0U ||
        walk.ihdr_width  > (uint32_t)DM2_V2_HUD_WIDGET_BLIT_MAX_WIDTH) {
        free(buf); return 0;
    }
    if (walk.ihdr_height == 0U ||
        walk.ihdr_height > (uint32_t)DM2_V2_HUD_WIDGET_BLIT_MAX_HEIGHT) {
        free(buf); return 0;
    }
    if (walk.ihdr_bit_depth  != 8U)  { free(buf); return 0; }
    if (walk.ihdr_color_type != 6U)  { free(buf); return 0; }

    /* For 1x1 RGBA, the raw IDAT payload is exactly 5 bytes:
     *   1 filter byte (always 0 = None for a 1x1 image)
     *   4 RGBA bytes
     *
     * We allocate an output buffer sized to the expected payload,
     * inflate the concatenated IDAT region, and validate the result.
     *
     * When FIRESTAFF_HAS_ZLIB is unavailable (release builds that
     * explicitly disable the bundled miniz provider), this path is
     * a no-op: the bounded blit becomes unavailable and the runtime
     * falls back to the 1-pixel anchor stamp, which is the same
     * behaviour as the pre-blit code path. */
#ifdef FIRESTAFF_HAS_ZLIB
    size_t expected = 5U; /* 1x1 RGBA raw payload */
    unsigned char raw[5];

    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if (inflateInit2(&zs, MAX_WBITS) != Z_OK) {
        free(buf);
        return 0;
    }
    zs.next_in  = (Bytef*)(buf + walk.idat_offset);
    zs.avail_in = (uInt)walk.idat_total_len;
    zs.next_out = raw;
    zs.avail_out = (uInt)sizeof(raw);

    int ret = inflate(&zs, Z_FINISH);
    inflateEnd(&zs);
    if (ret != Z_STREAM_END) {
        free(buf);
        return 0;
    }
    /* Confirm exactly 5 bytes were produced; any extra bytes mean the
     * PNG had multi-pixel data which the bounded envelope rejects. */
    size_t produced = sizeof(raw) - zs.avail_out;
    if (produced != expected) {
        free(buf);
        return 0;
    }
    /* Filter byte must be 0 (None) for the bounded envelope — a
     * non-zero filter would imply a larger image with a per-row
     * filter that this module does not implement. */
    if (raw[0] != 0U) {
        free(buf);
        return 0;
    }
#else
    /* No zlib available — bounded blit is disabled. The runtime's
     * fallback path takes over. */
    free(buf);
    (void)walk;
    return 0;
#endif

    /* Success — fill the pixel record. */
    out_pixel->width      = (int)walk.ihdr_width;
    out_pixel->height     = (int)walk.ihdr_height;
    out_pixel->bit_depth  = (int)walk.ihdr_bit_depth;
    out_pixel->color_type = (int)walk.ihdr_color_type;
#ifdef FIRESTAFF_HAS_ZLIB
    out_pixel->r = raw[1];
    out_pixel->g = raw[2];
    out_pixel->b = raw[3];
    out_pixel->a = raw[4];
#endif

    free(buf);
    return 1;
}

/* ── Bounded single-pixel blit ──────────────────────────────────── */
int dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
    uint8_t* fb, int w, int h_res,
    int dst_x, int dst_y,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    (void)g; (void)b; /* DM2 HUD writes palette-indexed bytes; we use R */
    if (!fb || w <= 0 || h_res <= 0) return 0;
    if (dst_x < 0 || dst_x >= w) return 0;
    if (dst_y < 0 || dst_y >= h_res) return 0;

    uint8_t* cell = fb + (size_t)dst_y * (size_t)w + (size_t)dst_x;
    if (a >= 255) {
        *cell = r;
    } else {
        /* "Src over Dst" integer alpha blend, clamped to byte range.
         * The red channel is what we ultimately store (palette index),
         * but we blend on R to keep the math symmetric. */
        uint16_t dst = (uint16_t)*cell;
        uint16_t src = (uint16_t)r;
        uint16_t out = (uint16_t)(((uint32_t)src * a +
                                   (uint32_t)dst * (255U - a)) / 255U);
        if (out > 255U) out = 255U;
        *cell = (uint8_t)out;
    }
    return 1;
}

/* ── High-level: read PNG + bounded blit ────────────────────────── */
int dm2_v2_hud_widget_bitmap_blit_render_slot(
    const DM2_V2_HudWidgetSlotInfo* info,
    uint8_t* fb, int w, int h_res,
    int dst_x, int dst_y)
{
    if (!info) return 0;
    /* Guard against a REAL slot whose source_file did not actually
     * resolve on disk (i.e. PARTIAL-classified). The gate already
     * filters these, but the blit defends itself anyway. */
    if (info->resolved_path[0] == '\0') return 0;
    if (!info->file_exists) return 0;

    DM2_V2_HudWidgetBlitPixel px;
    if (!dm2_v2_hud_widget_bitmap_blit_read_pixel(info->resolved_path, &px)) {
        return 0;
    }
    return dm2_v2_hud_widget_bitmap_blit_pixel_rgba(
        fb, w, h_res, dst_x, dst_y, px.r, px.g, px.b, px.a);
}

/* ── Source evidence ────────────────────────────────────────────── */
const char* dm2_v2_hud_widget_bitmap_blit_source_evidence(void) {
    return
        "DM2 V2 HUD Widget Bounded Bitmap Blit — Phase 3 follow-up\n"
        "Source: SKULL.ASM T560              (DM2 HUD rendering pipeline)\n"
        "Source: skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)\n"
        "Source: ReDMCSB PANEL.C F0354       (champion status-box drawing)\n"
        "Source: PNG specification (W3C / ISO 15948): IHDR + IDAT + IEND\n"
        "Source: examples/dm2_hud_widget_synthetic/ (synthetic 1x1 RGBA fixtures)\n"
        "Source: include/dm2_v2_hud_widget_assets.h (slot gate this module reads)\n"
        "Source: include/dm2_v2_hud_runtime.h (runtime hook this module extends)\n"
        "Envelope: 1x1 8-bit RGBA color-type-6 single-IDAT PNGs only\n"
        "Bounded: dst_x clamped to [0,w), dst_y clamped to [0,h_res);\n"
        "         file size capped at DM2_V2_HUD_WIDGET_BLIT_MAX_FILE_BYTES (64KB)\n"
        "         multi-pixel PNGs explicitly rejected (gate stays synthetic-only)\n"
        "Fallback: 1-pixel anchor stamp via dm2_v2_hud_runtime_stamp_real_slot()\n"
        "          when this module returns 0 (unsupported format, decompression\n"
        "          failure, file missing, destination out of bounds)\n"
        "Honest boundary: the bounded blit ONLY substitutes the procedural\n"
        "                  fallback for synthetic 1x1 RGBA test fixtures. It is\n"
        "                  NOT a finished-PBR real-art decoder. Real-art promotion\n"
        "                  requires a multi-pixel decode path (OPEN-BOUNDED next\n"
        "                  step) plus a sibling gap-list update.\n"
        "V1 invariant: V1 framebuffer is only ever written inside [0,w) x [0,h_res)\n"
        "V2 rule: the blit only runs for DM2_V2_HUD_WIDGET_CLASS_REAL slots whose\n"
        "         manifest source_file resolves on disk; everything else falls back\n";
}
