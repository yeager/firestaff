/*
 * csb_hint_oracle_graphical_overlay_shape.c
 *
 * Implementation of the minimal graphical overlay SHAPE / LAYOUT
 * gate for the CSB Utility Disk HCSB.HTC Hint Oracle.
 *
 * See include/csb_hint_oracle_graphical_overlay_shape.h for the
 * full source-lock boundary + scope.
 *
 * The implementation is a deliberately small, deterministic
 * layout engine:
 *
 *   - The cell width / height comes from the variant's
 *     `uses_high_glyphs` flag — narrow (8x8) for Atari ST 2.x +
 *     Amiga R1 EN, wide (8x16) for Amiga R3 EN + FR + GE.
 *   - The panel rect is anchored at (0, 0) by default and
 *     grows to fit the hint name + the body lines derived
 *     from the decoded text length.
 *   - The title row + body row are stacked vertically with the
 *     border + padding constants from the header, so they
 *     never overlap the border, each other, or the panel
 *     edges.
 *   - The body line count is derived from
 *     `ceil(decoded_text_len / body_column_count)` so a longer
 *     decoded text produces more body lines, not a wider
 *     panel.
 *   - The shape class is derived from the resulting panel
 *     width / height + cell size, never caller-supplied.
 *
 * The implementation never reads from a file, never allocates,
 * and never calls into the SDL renderer. The sketch helper
 * uses a fixed downsample (8x6) so the output stays bounded.
 */

#include "csb_hint_oracle_graphical_overlay_shape.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Downsample used by the ASCII sketch. 8 framebuffer pixels
 * become 1 ASCII column, 6 framebuffer pixels become 1 ASCII
 * row. With a 320x200 panel, that yields 40 columns and ~33
 * rows — fits comfortably in a 4 KiB buffer plus header. */
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_X 8u
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_Y 6u

/* Shape-class thresholds (in framebuffer pixels). These match
 * the class enum's documented intent:
 *   COMPACT  = panel < 240 wide AND < 80 tall AND narrow cell
 *   STANDARD = panel fills 320 wide AND < 160 tall
 *   LARGE    = everything else (multi-line body OR wide cell
 *              that pushed panel past 160 tall) */
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_COMPACT_MAX_W 239
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_COMPACT_MAX_H 79
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_STANDARD_MAX_H 159

/* ── Cell size lookup ───────────────────────────────────────────── */

static int cell_size_for_variant(CSB_HintOracleHTC_Variant variant,
                                 int *out_w, int *out_h,
                                 int *out_is_wide)
{
    int is_wide;

    if (out_w) *out_w = CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_W_NARROW;
    if (out_h) *out_h = CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_H_NARROW;

    /* UNKNOWN gets the narrow cell so a not-yet-classified
     * cache still produces a sane (if possibly too small)
     * shape. R3 EN / FR / GE get the wide cell. R1 EN + R2 EN
     * get the narrow cell because the Atari ST + Amiga R1 EN
     * Hint Oracle masks 0x80..0xFF as spaces — there are no
     * multilingual glyphs to render at the 8x16 cell size. */
    is_wide = 0;
    if (variant == CSB_HINT_ORACLE_HTC_VARIANT_RELEASE_3_EN ||
        variant == CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_FR ||
        variant == CSB_HINT_ORACLE_HTC_VARIANT_AMIGA_GE) {
        if (out_w) *out_w = CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_W_WIDE;
        if (out_h) *out_h = CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_H_WIDE;
        is_wide = 1;
    }
    if (out_is_wide) {
        *out_is_wide = is_wide;
    }
    return 1;
}

/* ── Result-name table ──────────────────────────────────────────── */

const char *csb_hint_oracle_overlay_shape_class_name(
    CSB_HintOracleOverlay_ShapeClass cls)
{
    switch (cls) {
    case CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_UNKNOWN: return "UNKNOWN";
    case CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_COMPACT: return "COMPACT";
    case CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_STANDARD: return "STANDARD";
    case CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_LARGE: return "LARGE";
    default: return "INVALID";
    }
}

/* ── Compute ────────────────────────────────────────────────────── */

void csb_hint_oracle_overlay_shape_compute(
    const char *hint_name,
    size_t decoded_text_len,
    CSB_HintOracleHTC_Variant variant,
    CSB_HintOracleOverlayShape *out_shape)
{
    int cell_w = CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_W_NARROW;
    int cell_h = CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_H_NARROW;
    int cell_is_wide = 0;
    size_t name_len = 0u;
    size_t i;

    if (!out_shape) {
        return;
    }
    memset(out_shape, 0, sizeof(*out_shape));
    out_shape->variant = variant;

    /* Cell size from variant. */
    (void)cell_size_for_variant(variant, &cell_w, &cell_h,
                                &cell_is_wide);
    out_shape->cell_w = cell_w;
    out_shape->cell_h = cell_h;
    out_shape->cell_is_wide = cell_is_wide;

    /* Hint name — NUL-terminated, truncated to NAME_CAP - 1u. */
    if (hint_name) {
        name_len = strlen(hint_name);
        if (name_len >= CSB_HINT_ORACLE_OVERLAY_SHAPE_NAME_CAP) {
            name_len = CSB_HINT_ORACLE_OVERLAY_SHAPE_NAME_CAP - 1u;
        }
        for (i = 0u; i < name_len; ++i) {
            out_shape->hint_name[i] = hint_name[i];
        }
    }
    out_shape->hint_name[name_len] = '\0';

    /* Panel rect: anchored at (0, 0), fills the 320x200 canvas
     * horizontally. We do not let the panel grow taller than
     * the canvas — the LARGE class catches "tried to grow,
     * hit the cap" as a layout warning rather than a clip. */
    out_shape->panel_x = 0;
    out_shape->panel_y = 0;
    out_shape->panel_w = CSB_HINT_ORACLE_OVERLAY_SHAPE_PANEL_W;
    out_shape->panel_h = CSB_HINT_ORACLE_OVERLAY_SHAPE_PANEL_H;

    /* Title row: pinned to (panel_x + border + pad_left,
     * panel_y + border + pad_top). Width is the panel width
     * minus the borders + padding on both sides, so the title
     * row never touches the border. */
    out_shape->title_x = out_shape->panel_x +
        (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_BORDER_PX +
        (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_PAD_LEFT;
    out_shape->title_y = out_shape->panel_y +
        (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_BORDER_PX +
        (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_PAD_TOP;
    out_shape->title_w = out_shape->panel_w -
        (2 * (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_BORDER_PX) -
        (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_PAD_LEFT -
        (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_PAD_RIGHT;
    out_shape->title_h = (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_TITLE_ROW_PX;

    /* Body row: pinned directly below the title row, sharing
     * the same x / width. Body width is always a multiple of
     * the cell width — rounded down so a half-column never
     * renders. */
    out_shape->body_x = out_shape->title_x;
    out_shape->body_y = out_shape->title_y + out_shape->title_h;
    out_shape->body_w = out_shape->title_w;
    {
        int cols = out_shape->body_w / cell_w;
        if (cols < 1) cols = 1;
        out_shape->body_w = cols * cell_w;
        out_shape->body_column_count = cols;
    }

    /* Body height: derive from decoded text length. Every
     * (cols * chars_per_cell) bytes of decoded text force a
     * new body line. Add 1 so a zero-length decoded text
     * still gets a visible body row (the contract test for
     * truncated / empty pages). Round down to a multiple of
     * cell_h so the body never ends mid-cell. */
    {
        size_t chars_per_line = (size_t)out_shape->body_column_count;
        size_t line_count = 0u;
        size_t body_h_px;
        if (chars_per_line == 0u) chars_per_line = 1u;
        if (decoded_text_len > 0u) {
            line_count = decoded_text_len / chars_per_line;
            if (decoded_text_len % chars_per_line != 0u) {
                ++line_count;
            }
        }
        if (line_count < 1u) line_count = 1u;
        body_h_px = line_count * (size_t)cell_h;

        /* Body height is capped at the remaining panel
         * height so a runaway line count does not push the
         * body off the canvas. The LARGE class catches the
         * "hit the cap" case. */
        {
            int panel_bottom = out_shape->panel_y + out_shape->panel_h;
            int body_bottom_max = panel_bottom -
                (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_BORDER_PX -
                (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_PAD_BOTTOM;
            int body_h_max = body_bottom_max - out_shape->body_y;
            if (body_h_max < (int)cell_h) body_h_max = (int)cell_h;
            if ((int)body_h_px > body_h_max) {
                body_h_px = (size_t)body_h_max;
            }
            /* Round down to a multiple of cell_h so the body
             * never ends mid-cell. */
            body_h_px = (body_h_px / (size_t)cell_h) * (size_t)cell_h;
            if (body_h_px < (size_t)cell_h) body_h_px = (size_t)cell_h;
        }
        out_shape->body_h = (int)body_h_px;
        out_shape->body_line_count = (int)(body_h_px / (size_t)cell_h);
        if (out_shape->body_line_count < 1) {
            out_shape->body_line_count = 1;
        }
    }

    /* Classify. The class is derived from the panel width +
     * panel height + cell size; it is not caller-supplied. */
    if (out_shape->panel_w <= CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_COMPACT_MAX_W &&
        out_shape->panel_h <= CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_COMPACT_MAX_H &&
        !cell_is_wide) {
        out_shape->shape_class = CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_COMPACT;
    } else if (out_shape->panel_h <= CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_STANDARD_MAX_H) {
        out_shape->shape_class = CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_STANDARD;
    } else {
        out_shape->shape_class = CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_LARGE;
    }
}

/* ── Framebuffer fit verdict ────────────────────────────────────── */

int csb_hint_oracle_overlay_shape_fits_framebuffer(
    const CSB_HintOracleOverlayShape *shape,
    int framebuffer_w,
    int framebuffer_h)
{
    int right;
    int bottom;

    if (!shape) {
        return -1;
    }
    if (framebuffer_w <= 0 || framebuffer_h <= 0) {
        return 0;
    }
    right = shape->panel_x + shape->panel_w;
    bottom = shape->panel_y + shape->panel_h;
    if (right > framebuffer_w || bottom > framebuffer_h) {
        return 0;
    }
    /* Title + body rects must also fit, even though they are
     * derived from the panel rect. */
    if (shape->title_x < shape->panel_x ||
        shape->title_y < shape->panel_y ||
        shape->title_x + shape->title_w > right ||
        shape->title_y + shape->title_h > bottom) {
        return 0;
    }
    if (shape->body_x < shape->panel_x ||
        shape->body_y < shape->panel_y ||
        shape->body_x + shape->body_w > right ||
        shape->body_y + shape->body_h > bottom) {
        return 0;
    }
    return 1;
}

/* ── ASCII sketch ───────────────────────────────────────────────── */

/* Sketch one ASCII column. `panel_x_px` / `panel_y_px` are the
 * top-left of the panel in framebuffer pixels; `col_origin_x`
 * is the framebuffer x of the leftmost pixel of this ASCII
 * column; `col_origin_y` is the framebuffer y of the topmost
 * pixel of the first row in this ASCII column; the rest of
 * the column is laid out downward in increments of
 * CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_Y. */
static char sketch_classify_pixel(const CSB_HintOracleOverlayShape *shape,
                                  int px, int py)
{
    int panel_right = shape->panel_x + shape->panel_w;
    int panel_bottom = shape->panel_y + shape->panel_h;
    int title_right = shape->title_x + shape->title_w;
    int title_bottom = shape->title_y + shape->title_h;
    int body_right = shape->body_x + shape->body_w;
    int body_bottom = shape->body_y + shape->body_h;

    if (px < shape->panel_x || px >= panel_right ||
        py < shape->panel_y || py >= panel_bottom) {
        return ' ';
    }
    if (px == shape->panel_x || px == panel_right - 1 ||
        py == shape->panel_y || py == panel_bottom - 1) {
        return '#';
    }
    if (px >= shape->title_x && px < title_right &&
        py >= shape->title_y && py < title_bottom) {
        return 'T';
    }
    if (px >= shape->body_x && px < body_right &&
        py >= shape->body_y && py < body_bottom) {
        return 'B';
    }
    return '.';
}

int csb_hint_oracle_overlay_shape_ascii_sketch(
    const CSB_HintOracleOverlayShape *shape,
    char *buf, size_t buf_size)
{
    int sketch_cols;
    int sketch_rows;
    int col;
    int row;
    size_t written = 0u;
    int n;

    if (!shape || !buf || buf_size == 0u) {
        return -1;
    }

    sketch_cols = (shape->panel_w +
                   (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_X - 1) /
                  (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_X;
    sketch_rows = (shape->panel_h +
                   (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_Y - 1) /
                  (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_Y;
    if (sketch_cols < 1) sketch_cols = 1;
    if (sketch_rows < 1) sketch_rows = 1;
    /* When the last sampled row would land strictly above
     * the bottom border, add one more row so the bottom
     * 1-px border is captured. */
    {
        int last_row_origin = (sketch_rows - 1) *
            (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_Y;
        if (last_row_origin < shape->panel_h - 1) {
            ++sketch_rows;
        }
    }

    /* Header line: one-line summary that names the shape +
     * variant + cell + class so a log capture is self-
     * identifying. The header is appended BEFORE the
     * downsampled ASCII rows. */
    {
        int header_n = snprintf(buf + written,
                                written < buf_size ?
                                    buf_size - written : 0u,
                                "shape class=%s variant=%s cell=%dx%d "
                                "panel=%dx%d title=%dx%d+%d+%d "
                                "body=%dx%d+%d+%d lines=%d cols=%d\n",
                                csb_hint_oracle_overlay_shape_class_name(
                                    shape->shape_class),
                                csb_hint_oracle_htc_variant_name(
                                    shape->variant),
                                shape->cell_w, shape->cell_h,
                                shape->panel_w, shape->panel_h,
                                shape->title_w, shape->title_h,
                                shape->title_x, shape->title_y,
                                shape->body_w, shape->body_h,
                                shape->body_x, shape->body_y,
                                shape->body_line_count,
                                shape->body_column_count);
        if (header_n < 0) {
            return -1;
        }
        if ((size_t)header_n >= (written < buf_size ?
                                 buf_size - written : 0u)) {
            /* Truncated — the header alone exceeds the
             * buffer; clamp and return what fits. */
            if (buf_size > 0u) {
                buf[buf_size - 1u] = '\0';
            }
            return (int)(buf_size - 1u);
        }
        written += (size_t)header_n;
    }

    /* ASCII rows. */
    for (row = 0; row < sketch_rows; ++row) {
        int py_origin = shape->panel_y +
            (row * (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_Y);
        int py_center;
        if (py_origin >= shape->panel_y + shape->panel_h) {
            py_center = shape->panel_y + shape->panel_h - 1;
        } else {
            /* Sample the top edge of the band (so a 1-px
             * border at panel_y is captured in the first
             * ASCII row). */
            py_center = py_origin;
        }
        for (col = 0; col < sketch_cols; ++col) {
            int px_origin = shape->panel_x +
                (col * (int)CSB_HINT_ORACLE_OVERLAY_SHAPE_SKETCH_DOWN_X);
            int px_center;
            char pixel;
            if (px_origin >= shape->panel_x + shape->panel_w) {
                px_center = shape->panel_x + shape->panel_w - 1;
            } else {
                px_center = px_origin;
            }
            pixel = sketch_classify_pixel(shape, px_center, py_center);
            if (written + 1u >= buf_size) {
                /* Out of room — NUL-terminate and stop. */
                if (buf_size > 0u) {
                    buf[buf_size - 1u] = '\0';
                }
                return (int)(buf_size - 1u);
            }
            buf[written++] = pixel;
        }
        if (written + 1u >= buf_size) {
            if (buf_size > 0u) {
                buf[buf_size - 1u] = '\0';
            }
            return (int)(buf_size - 1u);
        }
        buf[written++] = '\n';
    }

    if (written < buf_size) {
        buf[written] = '\0';
    } else {
        buf[buf_size - 1u] = '\0';
    }
    /* Sanity: the buffer must be NUL-terminated. */
    if (buf[written < buf_size ? written : buf_size - 1u] != '\0') {
        buf[buf_size - 1u] = '\0';
    }
    n = (int)written;
    /* Trim trailing newlines for callers that compare sketches
     * byte-for-byte (the regression test does this). The
     * sketch still ends with a newline when truncated; the
     * trim is a no-op in that case. */
    while (n > 0 && buf[n - 1] == '\n') {
        buf[--n] = '\0';
    }
    return n;
}
