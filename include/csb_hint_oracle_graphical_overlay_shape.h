/*
 * csb_hint_oracle_graphical_overlay_shape.h
 *
 * Minimal graphical overlay SHAPE / LAYOUT gate for the CSB Utility
 * Disk HCSB.HTC Hint Oracle. Sits one layer above the text-only
 * `csb_hint_oracle_ui_runtime_binding` surface: where the binding
 * module proves "decoded HCSB.HTC page text reaches a printable
 * buffer", this module proves "that buffer can be placed into a
 * bounded variant-aware layout shape that fits the panel" —
 * without ever opening a window, blitting a framebuffer, or
 * claiming pixel parity with the original Utility Disk screen.
 *
 * Scope:
 *   - A `CSB_HintOracleOverlayShape` struct that pins the panel
 *     rectangle (x, y, w, h in framebuffer pixels), the title row
 *     rectangle (the "hint N NAME" header line), and the body
 *     rectangle (the multi-line first-page text region), all in
 *     320x200 8-bit framebuffer coordinates.
 *   - A glyph-cell size: the bounding cell that each hint-text
 *     glyph occupies. Two variants:
 *       - "narrow" (8 px wide × 8 px tall) — Atari ST 2.x + Amiga
 *         R1 EN, where the Hint Oracle masks characters above
 *         0x7F as spaces (see the `uses_high_glyphs` flag in
 *         `csb_hint_oracle_htc_variant.h`).
 *       - "wide" (8 px wide × 16 px tall) — Amiga R3 EN + Amiga
 *         FR + Amiga GE, where the Hint Oracle renders 8-bit
 *         accented/multilingual glyphs above 0x7F.
 *     The wide variant is taller because the original Amiga
 *     Hint Oracle uses an 8×16 font to keep the multilingual
 *     glyph set readable. The narrow variant is the 8×8 font
 *     the Atari ST 2.x + Amiga R1 EN screen uses.
 *   - A `CSB_HintOracleOverlay_ShapeClass` enum that classifies
 *     each derived shape as COMPACT (small panel, narrow cell,
 *     short text), STANDARD (default panel + cell), or LARGE
 *     (panel grew to accommodate the text + wide cell). The
 *     class is derived, not caller-supplied, so the gate can
 *     assert "the same hint renders COMPACT on R1 EN and LARGE
 *     on R3 EN" without the caller having to remember which
 *     variant the loaded file belongs to.
 *   - A `csb_hint_oracle_overlay_shape_compute()` entry that takes
 *     a hint name + decoded first-page text length + the
 *     variant tag, and produces a fully-populated shape whose
 *     title row + body rows + class never overlap and never
 *     escape the 320x200 panel. The function is deterministic:
 *     the same inputs always produce the same shape.
 *   - A `csb_hint_oracle_overlay_shape_ascii_sketch()` helper that
 *     renders an ASCII layout sketch of the panel into a
 *     caller-owned buffer. The sketch uses '#' for border
 *     pixels, '.' for background, 'T' for the title row, 'B'
 *     for body rows, and 'X' for the corner pixels. The sketch
 *     is downsampled (every Nth framebuffer pixel becomes one
 *     ASCII column) so a 320x200 panel fits in roughly 64x40
 *     ASCII characters — small enough to log or paste into a
 *     git-tracked evidence file without exceeding common
 *     4 KiB line caps.
 *   - A `csb_hint_oracle_overlay_shape_fits_framebuffer()`
 *     verdict that asserts the shape's right + bottom edges
 *     fall inside the supplied framebuffer rect. The verdict
 *     is what an M11/M12 overlay caller would consult before
 *     committing to render the panel.
 *
 * Non-goals (deliberate, source-cited):
 *   - No M11/M12 event-loop integration. This is a shape gate
 *     only — no SDL_Render* call, no texture upload, no
 *     framebuffer pixel write.
 *   - No pixel-parity claim against the original Utility Disk
 *     Hint Oracle screen. The shape derives from the decoded
 *     text length + the variant's cell size + the panel
 *     defaults, not from a real captured screen.
 *   - No game-data discovery. The shape module consumes a
 *     caller-supplied variant tag + a caller-supplied decoded
 *     text length + a caller-supplied hint name; it does not
 *     own any file I/O or asset scanning.
 *   - No dependency on the existing overlay renderer. The
 *     shape module is a separate gate; if/when an M11/M12
 *     overlay renderer lands, it can consume the shape struct
 *     directly without going through this header.
 *
 * Source references:
 *   - ReDMCSB HINTHTC.C:177-358 — the format-2 / dungeon-13
 *     table whose hint count + page count the variant catalog
 *     uses to pick the cell class.
 *   - ReDMCSB HINTLZW.C:122-212 — the on-demand LZW page
 *     decode that produces the first-page text length the
 *     shape module reads.
 *   - dmweb Hint Oracle Files page — the canonical per-variant
 *     location/hint/page counts + the high-glyph flag the
 *     shape module reads.
 *   - dmweb Hint Oracle page — notes that the Amiga Hint
 *     Oracle uses an 8×16 font for multilingual glyphs while
 *     the Atari ST + Amiga R1 EN Hint Oracle uses an 8×8 font
 *     (the cell-size split the shape module bakes in).
 */

#ifndef FIRESTAFF_CSB_HINT_ORACLE_GRAPHICAL_OVERLAY_SHAPE_H
#define FIRESTAFF_CSB_HINT_ORACLE_GRAPHICAL_OVERLAY_SHAPE_H

#include <stddef.h>
#include <stdint.h>

#include "csb_hint_oracle_htc_variant.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Framebuffer contract ───────────────────────────────────────── */

#define CSB_HINT_ORACLE_OVERLAY_SHAPE_PANEL_W 320
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_PANEL_H 200

/* Title row height in framebuffer pixels: one 8xN font row
 * (the title row uses 8 px even on the wide variant; only the
 * body rows grow with the wide variant). */
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_TITLE_ROW_PX 8u

/* Body cell sizes per variant class. The narrow cell is the
 * 8x8 font the Atari ST 2.x + Amiga R1 EN Hint Oracle uses;
 * the wide cell is the 8x16 font the Amiga R3 EN + FR + GE
 * Hint Oracle uses. The cell width is shared (8 px); only
 * the row height changes between narrow / wide. */
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_W_NARROW 8u
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_H_NARROW 8u
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_W_WIDE   8u
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_CELL_H_WIDE   16u

/* Border thickness in pixels. The original Hint Oracle uses a
 * 1-pixel-wide border around the panel; the shape module
 * follows the same convention. */
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_BORDER_PX 1u

/* Panel padding in pixels: 2 px on the top + bottom, 4 px on
 * the left + right, so the title row + body rows always start
 * at least 4 px from the border. The padding is fixed and not
 * caller-overridable (changing it would invalidate every
 * COMPACT/STANDARD/LARGE class invariant). */
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_PAD_TOP    2u
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_PAD_BOTTOM 2u
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_PAD_LEFT   4u
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_PAD_RIGHT  4u

/* Maximum hint name length (incl. trailing NUL) the shape
 * derives a title row from. Hint names are bounded to
 * CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES (22) at the format
 * level; this constant is the +1 for the trailing NUL. */
#define CSB_HINT_ORACLE_OVERLAY_SHAPE_NAME_CAP \
    (CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u)

/* ── Shape class ─────────────────────────────────────────────────── */

typedef enum {
    /* Unknown / unset. The classify function never returns this
     * for a fully-populated shape; it is reserved for the
     * "shape not yet computed" sentinel so callers can zero-
     * initialize a shape struct without ending up classified
     * as COMPACT by accident. */
    CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_UNKNOWN = 0,

    /* Compact shape: panel fits in < 240 px wide AND < 80 px
     * tall, with the narrow 8x8 cell. Used for very short
     * hints ("ANYWHERE", "BOMB", etc.) on R1 EN. */
    CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_COMPACT = 1,

    /* Standard shape: panel fills the 320x200 canvas
     * horizontally with one or two body rows of the narrow
     * 8x8 cell. The default for most R1 EN hints. */
    CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_STANDARD = 2,

    /* Large shape: panel grew vertically to accommodate the
     * wide 8x16 cell, or grew horizontally because the decoded
     * text length forces a multi-line body. The default for
     * R3 EN / FR / GE variants + long hints on R1 EN. */
    CSB_HINT_ORACLE_OVERLAY_SHAPE_CLASS_LARGE = 3
} CSB_HintOracleOverlay_ShapeClass;

const char *csb_hint_oracle_overlay_shape_class_name(
    CSB_HintOracleOverlay_ShapeClass cls);

/* ── Shape struct ───────────────────────────────────────────────── */

typedef struct {
    /* Panel rect in framebuffer pixel coordinates. Always
     * anchored at x=0, y=0 by default (a future caller could
     * offset the panel, but the shape module keeps it simple
     * and predictable). */
    int panel_x;
    int panel_y;
    int panel_w;
    int panel_h;

    /* Title row rect, in framebuffer pixel coordinates. Always
     * pinned to panel_y + border + pad_top so it never
     * overlaps the border. */
    int title_x;
    int title_y;
    int title_w;
    int title_h;

    /* Body rect, in framebuffer pixel coordinates. Always
     * pinned to title_y + title_h so it never overlaps the
     * title row. */
    int body_x;
    int body_y;
    int body_w;
    int body_h;

    /* Glyph cell size in framebuffer pixels. Mirrors the
     * variant's documented font size: 8x8 for narrow
     * (Atari ST + Amiga R1 EN), 8x16 for wide (Amiga R3 EN
     * + FR + GE). */
    int cell_w;
    int cell_h;

    /* Number of body lines the panel can fit. Derived from
     * body_h / cell_h, rounded down so a half-row never
     * renders. */
    int body_line_count;

    /* Number of columns of glyph cells the body can fit.
     * Derived from body_w / cell_w, rounded down. */
    int body_column_count;

    /* Variant tag the shape was computed against. */
    CSB_HintOracleHTC_Variant variant;

    /* Glyph-cell class derived from the variant's
     * `uses_high_glyphs` flag. The shape module exposes this
     * so callers can verify the cell size matches the variant
     * without having to re-query the catalog. */
    int cell_is_wide;

    /* Derived class. Always non-UNKNOWN after a successful
     * csb_hint_oracle_overlay_shape_compute() call. */
    CSB_HintOracleOverlay_ShapeClass shape_class;

    /* Hint name the shape was computed against (NUL-terminated,
     * truncated to CSB_HINT_ORACLE_OVERLAY_SHAPE_NAME_CAP - 1u
     * bytes if the caller passed a longer string). */
    char hint_name[CSB_HINT_ORACLE_OVERLAY_SHAPE_NAME_CAP];
} CSB_HintOracleOverlayShape;

/* ── Compute / derive ───────────────────────────────────────────── */

/* Compute a shape from the supplied inputs. The shape's
 * panel_x / panel_y are always set to 0 (the shape module does
 * not choose an anchor); the panel_w / panel_h grow to fit the
 * hint name + the body lines derived from decoded_text_len.
 *
 * `hint_name` may be NULL or empty — the shape still derives
 * a valid title row + body rect, but the title row contents
 * will be a placeholder.
 *
 * `decoded_text_len` is the byte length of the decoded
 * first-page text. The body line count is derived from
 * `decoded_text_len` + the variant's cell width: every
 * `body_column_count - 1` characters of decoded text force a
 * line break, so a 200-byte decoded string on a 40-column
 * narrow cell body produces 5 body lines.
 *
 * The function is deterministic: the same inputs always
 * produce the same shape. The function never allocates, never
 * reads from a file, and never claims pixel parity. */
void csb_hint_oracle_overlay_shape_compute(
    const char *hint_name,
    size_t decoded_text_len,
    CSB_HintOracleHTC_Variant variant,
    CSB_HintOracleOverlayShape *out_shape);

/* ── Framebuffer fit verdict ────────────────────────────────────── */

/* Returns 1 when `shape` fits inside a framebuffer whose width
 * is `framebuffer_w` and height is `framebuffer_h`, 0 when it
 * does not, and -1 on argument error. The verdict does not
 * check pixel content — only that the shape's panel rect +
 * the body rect fall inside the framebuffer rect entirely.
 * A negative framebuffer_w or framebuffer_h returns 0 (the
 * shape trivially cannot fit a non-positive area). */
int csb_hint_oracle_overlay_shape_fits_framebuffer(
    const CSB_HintOracleOverlayShape *shape,
    int framebuffer_w,
    int framebuffer_h);

/* ── ASCII sketch ──────────────────────────────────────────────── */

/* Sketch the shape into `buf` as a multi-line ASCII layout.
 * Each ASCII character maps to a (downsample_w × downsample_h)
 * rectangle of framebuffer pixels; default downsample is 8x6
 * so a 320x200 panel renders as 40 columns × ~33 lines + a
 * few header lines. The sketch uses:
 *   - '#' for any pixel that touches the border
 *   - 'T' for any pixel that falls inside the title row
 *   - 'B' for any pixel that falls inside the body row
 *   - '.' for any pixel that falls inside the panel but is
 *         outside the title + body rows
 *   - ' ' for any pixel outside the panel
 *
 * Returns the number of bytes written (excl. the trailing
 * NUL), or -1 on argument error. The buffer is always
 * NUL-terminated when buf_size > 0. The sketch is downsampled
 * — it is not a pixel-perfect representation of the panel;
 * it is a layout fingerprint suitable for logs and
 * regression evidence. */
int csb_hint_oracle_overlay_shape_ascii_sketch(
    const CSB_HintOracleOverlayShape *shape,
    char *buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_GRAPHICAL_OVERLAY_SHAPE_H */
