#ifndef FIRESTAFF_ACCESSIBILITY_H
#define FIRESTAFF_ACCESSIBILITY_H
/*
 * firestaff_accessibility.h — Accessibility manifest for UI automation tools
 *
 * Writes a JSON file describing all interactive UI zones in the current frame.
 * External tools (Peekaboo, macOS Accessibility, test harnesses) can read this
 * to identify clickable elements without parsing the framebuffer.
 *
 * Protocol: game writes ~/.firestaff/accessibility.json atomically each frame.
 * Format follows Peekaboo DetectedElement schema for direct integration.
 */

#include <stdint.h>

#define FS_AX_MAX_ELEMENTS 128

#ifdef __cplusplus
extern "C" {
#endif

/* Element types matching Peekaboo ElementType */
typedef enum {
    FS_AX_BUTTON,        /* Clickable button (menu items, HUD buttons) */
    FS_AX_REGION,        /* Named region (viewport, HUD panel, spell area) */
    FS_AX_TEXT,          /* Text display (status, dialog text) */
    FS_AX_SLOT,          /* Inventory/equipment slot */
    FS_AX_PORTRAIT,      /* Champion portrait */
    FS_AX_MOVEMENT,      /* Movement arrow */
    FS_AX_DIALOG_CHOICE, /* Dialog YES/NO choice */
    FS_AX_CHAMPION_MIRROR, /* Hall of Champions mirror */
    /* ── Launcher / state-manifest additions (pass gap-launcher-state-a11y) ── */
    FS_AX_LAUNCHER_CARD,    /* Game card / destination entry on launcher main view */
    FS_AX_LAUNCHER_TAB,     /* Settings tab strip entry */
    FS_AX_LAUNCHER_ROW,     /* Settings / option row */
    FS_AX_POPUP,            /* Modal popup panel (missing-data, errors) */
    FS_AX_POPUP_OK,         /* Confirmation button inside a popup */
    /* ── Extras-view cell-by-cell manifest additions (gap bestiary/encyclopedia/museum) ──
     * The launcher has four navigable cell-by-cell views: Bestiary,
     * Item Encyclopedia, Screenshot Gallery, and Museum of Lore.
     * Each uses the same per-cell row model so screen readers can
     * announce a "row N of M, NAME, selected" pattern. Categories
     * (bestiary filter, item-encyclopedia category strip) are surfaced
     * as their own element type so a screen reader can announce
     * "Category tab BEASTS selected" before the row content. */
    FS_AX_CATEGORY_TAB,         /* Bestiary / item-encyclopedia category tab */
    FS_AX_BESTIARY_ROW,         /* One creature row in the bestiary list */
    FS_AX_ITEM_ENCYCLOPEDIA_ROW,/* One item row in the item encyclopedia */
    FS_AX_SCREENSHOT_THUMB,     /* One screenshot entry in the gallery */
    FS_AX_MUSEUM_CATEGORY,      /* One archive section in the Museum of Lore */
    FS_AX_MUSEUM_BULLET         /* One lore bullet on the active museum page */
} FS_AX_ElementType;

typedef struct {
    const char* id;       /* Unique ID, e.g. "B1", "MOVE_FWD", "SLOT_HAND_L" */
    const char* label;    /* Display label, e.g. "Forward", "Left Hand" */
    FS_AX_ElementType type;
    int x, y, w, h;      /* Bounding rect in framebuffer coords */
    int enabled;          /* 1=clickable, 0=disabled/hidden */
    const char* value;    /* Optional current value (text content, item name) */
} FS_AX_Element;

/* Begin a new frame manifest. Call once per render frame. */
void fs_ax_begin_frame(int framebuffer_width, int framebuffer_height,
                       const char* game_state);

/* Add an element to the current frame manifest. */
void fs_ax_add_element(const FS_AX_Element* element);

/* Write the manifest to disk. Call after all elements are added.
 * Path: ~/.firestaff/accessibility.json (atomic write via rename). */
void fs_ax_flush(void);

/* Enable/disable manifest writing. Disabled by default for performance.
 * Enable with --accessibility flag or FS_ACCESSIBILITY=1 env var. */
void fs_ax_set_enabled(int enabled);
int  fs_ax_is_enabled(void);

/* Override the output directory. Default is $HOME/.firestaff. Tests
 * redirect HOME for isolation; this lets the launcher a11y emitter
 * (or future code) drop into a chosen sandbox without mutating env.
 * Pass NULL or "" to revert to the default $HOME/.firestaff location. */
void fs_ax_set_output_dir(const char* dir);

/* Return the absolute path to the last (or current) manifest file.
 * Buffer is statically allocated inside firestaff_accessibility.c and
 * remains valid until fs_ax_set_enabled / fs_ax_set_output_dir is
 * called again. */
const char* fs_ax_get_output_path(void);

/* Cleanup */
void fs_ax_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ACCESSIBILITY_H */
