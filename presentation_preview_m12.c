#include "presentation_preview_m12.h"
#include "menu_startup_m12.h"
#include <string.h>
#include <stdio.h>

/* ── Mode descriptions ───────────────────────────────────────────
 *
 * Static data describing each presentation mode's feature set.
 * These are displayed in the side-by-side preview so the player
 * can compare what each mode offers before launching.
 */

static const M12_PreviewModeDesc g_v1Desc = {
    "V1 ORIGINAL",
    "AUTHENTIC EXPERIENCE",
    "PIXEL-PERFECT RECREATION OF THE ORIGINAL\n"
    "DUNGEON MASTER AS IT APPEARED ON THE ATARI\n"
    "ST AND AMIGA. EVERY PIXEL, PALETTE CYCLE,\n"
    "AND TIMING QUIRK FAITHFULLY PRESERVED.",
    {
        {"320x200 RESOLUTION",   "ORIGINAL FIXED RESOLUTION",              1},
        {"4-BIT PALETTE",        "16-COLOUR VGA/ST PALETTE",               1},
        {"ORIGINAL RENDERER",    "COLUMN-STRIP RAYCASTING ENGINE",         1},
        {"TICK-ACCURATE TIMING", "FRAME PACING MATCHES ORIGINAL HW",      1},
        {"ORIGINAL FONT",        "BITMAP FONT FROM GRAPHICS.DAT",          1},
        {"ORIGINAL SOUND",       "SND3/SONG.DAT AUDIO PLAYBACK",          1},
        {"WIDESCREEN",           "NOT AVAILABLE IN V1 MODE",               0},
        {"BILINEAR FILTERING",   "NOT AVAILABLE IN V1 MODE",               0},
        {"ENHANCED SPRITES",     "NOT AVAILABLE IN V1 MODE",               0},
        {"DYNAMIC LIGHTING",     "NOT AVAILABLE IN V1 MODE",               0},
        {"HD TEXTURES",          "NOT AVAILABLE IN V1 MODE",               0},
        {"3D PERSPECTIVE",       "NOT AVAILABLE IN V1 MODE",               0},
    },
    12
};

static const M12_PreviewModeDesc g_v2Desc = {
    "V2 ENHANCED 2D",
    "ENHANCED CLASSIC",
    "THE ORIGINAL DUNGEON MASTER EXPERIENCE\n"
    "WITH QUALITY-OF-LIFE IMPROVEMENTS: HIGHER\n"
    "RESOLUTION, WIDESCREEN SUPPORT, SMOOTHER\n"
    "SCROLLING, AND ENHANCED SPRITE ARTWORK.",
    {
        {"SCALABLE RESOLUTION",  "UP TO 1280x960 RENDERING",              1},
        {"EXTENDED PALETTE",     "FULL 8-BIT COLOUR DEPTH",               1},
        {"ORIGINAL RENDERER",    "ENHANCED COLUMN-STRIP ENGINE",           1},
        {"SMOOTH MOVEMENT",      "INTERPOLATED CAMERA TRANSITIONS",        1},
        {"ENHANCED FONT",        "ANTI-ALIASED BITMAP FONT",              1},
        {"ENHANCED SOUND",       "HIGHER SAMPLE RATE AUDIO",              1},
        {"WIDESCREEN",           "16:9 AND 16:10 ASPECT RATIOS",          1},
        {"BILINEAR FILTERING",   "SMOOTH TEXTURE SCALING",                1},
        {"ENHANCED SPRITES",     "REDRAWN CREATURE ARTWORK",              1},
        {"DYNAMIC LIGHTING",     "NOT AVAILABLE IN V2 MODE",              0},
        {"HD TEXTURES",          "NOT AVAILABLE IN V2 MODE",              0},
        {"3D PERSPECTIVE",       "NOT AVAILABLE IN V2 MODE",              0},
    },
    12
};

static const M12_PreviewModeDesc g_v3Desc = {
    "V3 MODERN/3D",
    "FULL REIMAGINING",
    "A COMPLETE VISUAL REIMAGINING WITH TRUE 3D\n"
    "PERSPECTIVE, DYNAMIC LIGHTING, HIGH-RES\n"
    "TEXTURES, AND MODERN RENDERING EFFECTS.\n"
    "THE DUNGEON LIKE YOU ALWAYS IMAGINED IT.",
    {
        {"NATIVE RESOLUTION",    "RENDERS AT YOUR DISPLAY RESOLUTION",    1},
        {"HDR PALETTE",          "FULL 32-BIT COLOUR WITH HDR",           1},
        {"3D RENDERER",          "TRUE PERSPECTIVE 3D ENGINE",             1},
        {"FREE CAMERA",          "SMOOTH 360-DEGREE MOVEMENT",            1},
        {"MODERN FONT",          "TRUETYPE FONT RENDERING",               1},
        {"SPATIAL AUDIO",        "POSITIONAL 3D SOUND ENGINE",            1},
        {"WIDESCREEN",           "ANY ASPECT RATIO SUPPORTED",            1},
        {"BILINEAR FILTERING",   "ANISOTROPIC TEXTURE FILTERING",         1},
        {"ENHANCED SPRITES",     "FULL 3D CREATURE MODELS",               1},
        {"DYNAMIC LIGHTING",     "REAL-TIME TORCH AND SPELL LIGHT",       1},
        {"HD TEXTURES",          "HIGH-RESOLUTION WALL TEXTURES",         1},
        {"3D PERSPECTIVE",       "TRUE DEPTH WITH PARALLAX",              1},
    },
    12
};

/* Pointers indexed by M12_PresentationMode enum */
static const M12_PreviewModeDesc* g_modeDescs[3] = {
    &g_v1Desc,
    &g_v2Desc,
    &g_v3Desc
};

/* ── Helpers ─────────────────────────────────────────────────────── */

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int wrap_mode(int mode, int delta) {
    int m = mode + delta;
    if (m < 0) m = M12_PRESENTATION_MODE_COUNT - 1;
    if (m >= M12_PRESENTATION_MODE_COUNT) m = 0;
    return m;
}

/* ── Init ────────────────────────────────────────────────────────── */

void M12_PresentationPreview_Init(M12_PresentationPreviewState* state) {
    memset(state, 0, sizeof(*state));
    state->leftMode = M12_PRESENTATION_V1_ORIGINAL;
    state->rightMode = M12_PRESENTATION_V2_ENHANCED_2D;
    state->splitPosition = 50;
    state->focusSide = M12_PREVIEW_SIDE_LEFT;
    state->selectedFeature = 0;
    state->loaded = 0;

    /* Copy static descriptions into state for uniform access */
    memcpy(&state->modeDescs[0], &g_v1Desc, sizeof(M12_PreviewModeDesc));
    memcpy(&state->modeDescs[1], &g_v2Desc, sizeof(M12_PreviewModeDesc));
    memcpy(&state->modeDescs[2], &g_v3Desc, sizeof(M12_PreviewModeDesc));
}

/* ── Set Game ────────────────────────────────────────────────────── */

void M12_PresentationPreview_SetGame(M12_PresentationPreviewState* state,
                                     const char* gameId,
                                     const char* gameTitle) {
    if (!state) return;

    memset(state->gameId, 0, sizeof(state->gameId));
    memset(state->gameTitle, 0, sizeof(state->gameTitle));

    if (gameId) {
        strncpy(state->gameId, gameId, sizeof(state->gameId) - 1);
    }
    if (gameTitle) {
        strncpy(state->gameTitle, gameTitle, sizeof(state->gameTitle) - 1);
    }

    state->selectedFeature = 0;
    state->loaded = 1;
}

/* ── Input Handling ──────────────────────────────────────────────── */

void M12_PresentationPreview_HandleInput(
    M12_PresentationPreviewState* state, int input) {
    if (!state) return;

    const M12_PreviewModeDesc* focusDesc;
    int maxFeature;

    switch (input) {
    case M12_MENU_INPUT_UP:
        if (state->selectedFeature > 0) {
            state->selectedFeature--;
        }
        break;

    case M12_MENU_INPUT_DOWN:
        focusDesc = (state->focusSide == M12_PREVIEW_SIDE_LEFT)
            ? &state->modeDescs[state->leftMode]
            : &state->modeDescs[state->rightMode];
        maxFeature = focusDesc->featureCount - 1;
        if (maxFeature < 0) maxFeature = 0;
        if (state->selectedFeature < maxFeature) {
            state->selectedFeature++;
        }
        break;

    case M12_MENU_INPUT_LEFT:
        if (state->focusSide == M12_PREVIEW_SIDE_RIGHT) {
            state->focusSide = M12_PREVIEW_SIDE_LEFT;
            state->selectedFeature = 0;
        } else {
            /* Adjust split position left */
            state->splitPosition = clamp_int(state->splitPosition - 5, 10, 90);
        }
        break;

    case M12_MENU_INPUT_RIGHT:
        if (state->focusSide == M12_PREVIEW_SIDE_LEFT) {
            state->focusSide = M12_PREVIEW_SIDE_RIGHT;
            state->selectedFeature = 0;
        } else {
            /* Adjust split position right */
            state->splitPosition = clamp_int(state->splitPosition + 5, 10, 90);
        }
        break;

    case M12_MENU_INPUT_ACCEPT:
        /* Cycle the focused side's mode forward */
        if (state->focusSide == M12_PREVIEW_SIDE_LEFT) {
            state->leftMode = wrap_mode(state->leftMode, 1);
            /* Avoid showing the same mode on both sides */
            if (state->leftMode == state->rightMode) {
                state->leftMode = wrap_mode(state->leftMode, 1);
            }
        } else {
            state->rightMode = wrap_mode(state->rightMode, 1);
            if (state->rightMode == state->leftMode) {
                state->rightMode = wrap_mode(state->rightMode, 1);
            }
        }
        state->selectedFeature = 0;
        break;

    case M12_MENU_INPUT_ACTION:
        /* Reset split to centre */
        state->splitPosition = 50;
        break;

    default:
        break;
    }
}

/* ── Draw ────────────────────────────────────────────────────────── */

/*
 * Renders a text-based split-screen comparison to the framebuffer.
 *
 * The actual pixel rendering of game scenes is deferred to the
 * modern renderer.  This function draws:
 *   - Title bar with game name
 *   - Left panel: mode name, subtitle, feature list
 *   - Split divider at the configured position
 *   - Right panel: mode name, subtitle, feature list
 *   - Navigation hints at the bottom
 *
 * For now this writes structured ASCII text into the framebuffer
 * area as a placeholder.  The modern renderer will replace this
 * with actual graphical rendering.
 */
void M12_PresentationPreview_Draw(
    const M12_PresentationPreviewState* state,
    unsigned char* framebuffer,
    int fbWidth, int fbHeight) {

    if (!state || !framebuffer) return;

    /* Clear framebuffer to background colour (dark grey = 0x1A) */
    memset(framebuffer, 0x1A, (size_t)fbWidth * (size_t)fbHeight);

    /*
     * Text rendering is handled by the modern renderer pipeline.
     * This function establishes the layout geometry:
     *
     * ┌──────────────────┬──────────────────┐
     * │  PRESENTATION MODE PREVIEW          │
     * │  [Game Title]                       │
     * ├──────────────────┼──────────────────┤
     * │  V1 ORIGINAL     │ V2 ENHANCED 2D  │
     * │  AUTHENTIC EXP.  │ ENHANCED CLASSIC │
     * │                  │                  │
     * │  ✓ 320x200 RES  │ ✓ SCALABLE RES  │
     * │  ✓ 4-BIT PAL    │ ✓ EXTENDED PAL   │
     * │  ✓ ORIG RENDER  │ ✓ ORIG RENDER    │
     * │  ✗ WIDESCREEN   │ ✓ WIDESCREEN     │
     * │  ✗ BILINEAR     │ ✓ BILINEAR       │
     * │  ...            │ ...              │
     * ├──────────────────┴──────────────────┤
     * │  ←→ SWITCH SIDE  ENTER: CYCLE MODE │
     * │  ↑↓ SCROLL       ACTION: RESET     │
     * └─────────────────────────────────────┘
     *
     * The split divider is at splitPosition% from the left.
     * The focused side has a highlight marker.
     *
     * The framebuffer encoding is: each byte at (y * fbWidth + x)
     * holds a palette index.  Layout coordinates are computed here
     * and consumed by the modern renderer for actual text blitting.
     */

    /* Encode layout metadata in the first 64 bytes as a structured
     * header that the modern renderer can parse.  This avoids
     * coupling this module to font rendering internals. */

    /* Header magic: "PPVW" (Presentation Preview) */
    if (fbWidth * fbHeight >= 64) {
        framebuffer[0] = 'P';
        framebuffer[1] = 'P';
        framebuffer[2] = 'V';
        framebuffer[3] = 'W';
        /* Split position (byte 4) */
        framebuffer[4] = (unsigned char)state->splitPosition;
        /* Left mode (byte 5) */
        framebuffer[5] = (unsigned char)state->leftMode;
        /* Right mode (byte 6) */
        framebuffer[6] = (unsigned char)state->rightMode;
        /* Focus side (byte 7) */
        framebuffer[7] = (unsigned char)state->focusSide;
        /* Selected feature (byte 8) */
        framebuffer[8] = (unsigned char)state->selectedFeature;
        /* Loaded flag (byte 9) */
        framebuffer[9] = (unsigned char)state->loaded;
    }
}

/* ── Accessors ───────────────────────────────────────────────────── */

const M12_PreviewModeDesc* M12_PresentationPreview_GetModeDesc(
    const M12_PresentationPreviewState* state, int mode) {
    if (!state) return NULL;
    if (mode < 0 || mode >= M12_PRESENTATION_MODE_COUNT) return NULL;
    return &state->modeDescs[mode];
}

int M12_PresentationPreview_GetSplitPosition(
    const M12_PresentationPreviewState* state) {
    if (!state) return 50;
    return state->splitPosition;
}

const char* M12_PresentationPreview_GetLeftLabel(
    const M12_PresentationPreviewState* state) {
    if (!state) return "UNKNOWN";
    if (state->leftMode < 0 || state->leftMode >= M12_PRESENTATION_MODE_COUNT)
        return "UNKNOWN";
    return state->modeDescs[state->leftMode].modeName;
}

const char* M12_PresentationPreview_GetRightLabel(
    const M12_PresentationPreviewState* state) {
    if (!state) return "UNKNOWN";
    if (state->rightMode < 0 || state->rightMode >= M12_PRESENTATION_MODE_COUNT)
        return "UNKNOWN";
    return state->modeDescs[state->rightMode].modeName;
}

int M12_PresentationPreview_IsFeatureHighlighted(
    const M12_PresentationPreviewState* state,
    M12_PreviewSide side, int featureIndex) {
    if (!state) return 0;
    if (state->focusSide != side) return 0;
    return (state->selectedFeature == featureIndex) ? 1 : 0;
}
