/*
 * m11_high_contrast_overlay_pc34_compat.c
 *
 * M11 in-game high-contrast overlay gate (see header for the full
 * contract). Pure C11, no ReDMCSB equivalent — this is Firestaff
 * accessibility glue on top of the existing M12 launcher remap
 * (m12_presented_color in src/ui/menu_startup_m12.c).
 *
 * Manifest (returned by M11_HighContrast_GetManifest):
 *
 *   GATE COVERS:
 *     - HUD text styles (g_text_small, g_text_shadow, g_text_title
 *       in m11_game_view.c): foreground color index is passed
 *       through M11_HighContrast_RemapPresentedColor().
 *     - Dialog choice text (m11_draw_dialog_choices): color
 *       remapped at draw time.
 *     - Action-area / rune-strip text: color remapped at draw time.
 *     - Combat log lines (m11_log_event): the color passed to
 *       m11_log_event is remapped before being baked into the
 *       log row, so chrome text in the log uses the high-contrast
 *       palette.
 *     - Hit-flash damage numbers (M11_GameView_DrawHitFlash):
 *       the foreground color used for the number is remapped.
 *     - Death overlay ("YOU HAVE PERISHED"), winner overlay,
 *       and any other M11-color-driven text that is NOT a raw
 *       dungeon-viewport pixel.
 *
 *   GATE DELIBERATELY DOES NOT COVER:
 *     - The 320x200 indexed dungeon-viewport pixel data sourced
 *       from GRAPHICS.DAT. These pixels are owned by the M11 V1
 *       draw path (src/engine/m11_game_view.c::m11_draw_viewport)
 *       and must stay bit-identical with the original DM1 PC 3.4
 *       presentation so capture/parity probes stay valid.
 *     - The wall / floor / door / creature ornament pixels blitted
 *       from the dungeon cell table. Same reason.
 *     - The HUD inventory/champion mirror panels: those use their
 *       own panel-C040/C017 indexed bitmap blits that come from
 *       GRAPHICS.DAT and are part of the original presentation.
 *
 *   PLANNED FUTURE COVERAGE (intentionally not in this gate):
 *     - A bounded in-place color remap for M11 chrome surfaces
 *       (inventory panel text, action strip glyph tint) when a
 *       future pass exposes them; for now the gate is a pure
 *       text-style remap so it is byte-stable and CTest-able
 *       without game data.
 */

#include "m11_high_contrast_overlay_pc34_compat.h"

#include <string.h>

/* ── Gate state ────────────────────────────────────────────────────── */

static int g_m11_high_contrast_active = 0;

void M11_HighContrast_SetActive(int active) {
    g_m11_high_contrast_active = (active != 0) ? 1 : 0;
}

int M11_HighContrast_IsActive(void) {
    return g_m11_high_contrast_active;
}

/* ── Manifest string ───────────────────────────────────────────────── */

static const char kM11HighContrastManifest[] =
    "M11_HIGH_CONTRAST_OVERLAY_GATE_v1\n"
    "covers: hud_text,dialog_text,action_area_text,rune_strip_text,"
    "combat_log_text,hit_flash_text,death_overlay_text,winner_overlay_text\n"
    "preserves: dungeon_viewport_320x200_pixels,wall_floor_door_creature_ornament_pixels,"
    "hud_panel_c040_blit_pixels,hud_panel_c017_backdrop_pixels\n"
    "default_state: off\n"
    "v1_fidelity_contract: bit_identical_when_off\n"
    "source_lock: none_accessibility_glue_mirrors_m12_presented_color";

const char* M11_HighContrast_GetManifest(void) {
    return kM11HighContrastManifest;
}

/* ── Color remap ───────────────────────────────────────────────────── */

/*
 * Mirror of m12_presented_color() in src/ui/menu_startup_m12.c.
 * The launcher collapses muted slots to BLACK and lets YELLOW /
 * LIGHT_CYAN / WHITE survive; everything else collapses to WHITE.
 * We use the same restricted palette so the launcher and the
 * in-game chrome surface agree when the toggle is on.
 *
 * The M11 palette has 16 indexed slots (DM PC VGA 16-color mode).
 * Slot 0..15 are the only legal input. Out-of-range inputs are
 * returned unchanged (defensive, never used by the M11 call sites).
 */
unsigned char M11_HighContrast_RemapPresentedColor(unsigned char color) {
    if (!g_m11_high_contrast_active) {
        return color;
    }
    switch (color) {
        case 0:   /* BLACK */
            return 0;
        case 1:   /* GRAY (muted) */
        case 3:   /* BROWN (muted) */
        case 5:   /* DARK_BROWN (muted) */
        case 6:   /* GREEN (muted) */
        case 8:   /* RED (muted) */
        case 12:  /* DARK_GRAY (muted) */
        case 14:  /* NAVY / LIGHT_BLUE (muted) */
            return 0;
        case 2:   /* LIGHT_GRAY (still readable) */
            return 2;
        case 4:   /* CYAN / LIGHT_CYAN (invariant) */
        case 7:   /* LIGHT_GREEN (still readable) */
        case 9:   /* LIGHT_RED / ORANGE (still readable) */
        case 13:  /* SILVER (still readable) */
            return color;
        case 10:  /* MAGENTA (skin tone) — keep readable */
            return 10;
        case 11:  /* YELLOW (anchor) */
            return 11;
        case 15:  /* WHITE (anchor) */
            return 15;
        default:
            return 15;
    }
}

/* ── Region apply ──────────────────────────────────────────────────── */

/*
 * Apply the chrome remap to a sub-rect of an indexed framebuffer,
 * preserving any pixel whose slot has its bit set in `excludeMask`.
 *
 * The typical caller is the M11 HUD chrome layer that wants to
 * remap the dialog text style but explicitly fence off the
 * dungeon viewport sub-rect (320x136, off-screen bottom for the
 * 320x200 logical buffer) so the original pixel data is left
 * untouched. Bits 0..15 of `excludeMask` map to palette indices
 * 0..15; bit n+1 corresponds to palette index n.
 */
int M11_HighContrast_ApplyActiveRGBA(unsigned char* framebuffer,
                                     int framebufferWidth,
                                     int framebufferHeight,
                                     int x, int y, int width, int height,
                                     unsigned int excludeMask) {
    int startX;
    int startY;
    int endX;
    int endY;
    int row;
    int col;
    int remappedCount = 0;

    if (!framebuffer || framebufferWidth <= 0 || framebufferHeight <= 0) {
        return 0;
    }
    if (width <= 0 || height <= 0) {
        return 0;
    }
    if (!g_m11_high_contrast_active) {
        /* Off → identity, but report 0 so callers don't claim a
         * remap actually happened. */
        return 0;
    }

    /* Clip to framebuffer bounds. */
    if (x < 0) {
        startX = 0;
    } else {
        startX = x;
    }
    if (y < 0) {
        startY = 0;
    } else {
        startY = y;
    }
    endX = x + width;
    endY = y + height;
    if (endX > framebufferWidth) endX = framebufferWidth;
    if (endY > framebufferHeight) endY = framebufferHeight;
    if (startX >= endX || startY >= endY) {
        return 0;
    }

    for (row = startY; row < endY; ++row) {
        unsigned char* line = framebuffer + (size_t)row * (size_t)framebufferWidth;
        for (col = startX; col < endX; ++col) {
            unsigned char idx = line[col];
            unsigned int bit = (1u << (idx & 0x0Fu));
            if (excludeMask & bit) {
                continue;
            }
            unsigned char remapped = M11_HighContrast_RemapPresentedColor(idx);
            if (remapped != idx) {
                line[col] = remapped;
                ++remappedCount;
            }
        }
    }
    return remappedCount > 0 ? 1 : 0;
}
