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
    FS_AX_CHAMPION_MIRROR /* Hall of Champions mirror */
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

/* Cleanup */
void fs_ax_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ACCESSIBILITY_H */
