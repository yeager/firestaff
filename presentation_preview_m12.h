#ifndef FIRESTAFF_PRESENTATION_PREVIEW_M12_H
#define FIRESTAFF_PRESENTATION_PREVIEW_M12_H

/*
 * Presentation Mode Preview — M12 launcher feature.
 *
 * Side-by-side comparison view showing how the game will look in
 * each presentation mode: V1 (authentic), V2 (enhanced 2D), and
 * V3 (modern 3D).  Provides textual descriptions of feature sets,
 * interactive toggling between modes, and a visual-diff split
 * position for comparing two modes simultaneously.
 *
 * Actual pixel rendering is deferred to the modern renderer; this
 * module produces structured preview state and text descriptions
 * that the renderer consumes.
 *
 * Data flow:
 *   1. M12_PresentationPreview_Init()         — reset state
 *   2. M12_PresentationPreview_SetGame()      — configure for a game
 *   3. M12_PresentationPreview_HandleInput()  — navigate/toggle
 *   4. M12_PresentationPreview_Draw()         — render text preview
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Comparison side ─────────────────────────────────────────────── */
typedef enum {
    M12_PREVIEW_SIDE_LEFT = 0,
    M12_PREVIEW_SIDE_RIGHT
} M12_PreviewSide;

/* ── Feature highlight entry ─────────────────────────────────────── */
typedef struct {
    const char* name;         /* Short feature name              */
    const char* description;  /* One-line explanation             */
    int         available;    /* 1 = available in this mode       */
} M12_PreviewFeature;

/* Maximum features listed per mode */
#define M12_PREVIEW_MAX_FEATURES  12

/* ── Mode description block ──────────────────────────────────────── */
typedef struct {
    const char*       modeName;       /* e.g. "V1 ORIGINAL"         */
    const char*       subtitle;       /* e.g. "AUTHENTIC EXPERIENCE"*/
    const char*       summary;        /* Multi-line summary text     */
    M12_PreviewFeature features[M12_PREVIEW_MAX_FEATURES];
    int               featureCount;
} M12_PreviewModeDesc;

/* ── Split comparison state ──────────────────────────────────────── */
typedef struct {
    /* Which presentation modes are shown on each side */
    int  leftMode;            /* M12_PRESENTATION_V1_ORIGINAL etc. */
    int  rightMode;

    /* Split divider position (0–100, percentage from left) */
    int  splitPosition;

    /* Currently focused side for highlight purposes */
    M12_PreviewSide focusSide;

    /* Currently selected feature row on focused side */
    int  selectedFeature;

    /* Game context */
    char gameId[32];
    char gameTitle[64];

    /* Cached mode descriptions (one per presentation mode) */
    M12_PreviewModeDesc modeDescs[3];

    /* Whether the preview has been initialised with a game */
    int  loaded;
} M12_PresentationPreviewState;

/* ── API ─────────────────────────────────────────────────────────── */

/* Initialise preview state with defaults (V1 left, V2 right). */
void M12_PresentationPreview_Init(M12_PresentationPreviewState* state);

/* Set the game context for the preview.  gameId and title are
 * copied into the state struct.  Rebuilds feature availability. */
void M12_PresentationPreview_SetGame(M12_PresentationPreviewState* state,
                                     const char* gameId,
                                     const char* gameTitle);

/* Handle navigation input.  UP/DOWN scroll features, LEFT/RIGHT
 * switch focus side, ACCEPT cycles the focused side's mode,
 * ACTION adjusts split position. */
void M12_PresentationPreview_HandleInput(
    M12_PresentationPreviewState* state, int input);

/* Render the text-based comparison into a framebuffer.
 * Draws a split-screen layout with mode names, feature lists,
 * and a divider line at the current split position. */
void M12_PresentationPreview_Draw(
    const M12_PresentationPreviewState* state,
    unsigned char* framebuffer,
    int fbWidth, int fbHeight);

/* Get the mode description for a given presentation mode index. */
const M12_PreviewModeDesc* M12_PresentationPreview_GetModeDesc(
    const M12_PresentationPreviewState* state, int mode);

/* Get the current split position (0–100). */
int M12_PresentationPreview_GetSplitPosition(
    const M12_PresentationPreviewState* state);

/* Get the label for the left side's current mode. */
const char* M12_PresentationPreview_GetLeftLabel(
    const M12_PresentationPreviewState* state);

/* Get the label for the right side's current mode. */
const char* M12_PresentationPreview_GetRightLabel(
    const M12_PresentationPreviewState* state);

/* Return 1 if the given feature index is highlighted (selected). */
int M12_PresentationPreview_IsFeatureHighlighted(
    const M12_PresentationPreviewState* state,
    M12_PreviewSide side, int featureIndex);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_PRESENTATION_PREVIEW_M12_H */
