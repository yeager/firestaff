#ifndef FIRESTAFF_TOUCH_LAYOUT_M12_H
#define FIRESTAFF_TOUCH_LAYOUT_M12_H

/*
 * touch_layout_m12 — Touch overlay layout editor for the Firestaff launcher.
 *
 * Defines on-screen touch zones mapped to game actions (M12_InputAction
 * from input_remap_m12.h).  Supports multiple preset layouts, per-zone
 * drag/resize editing, and persistence to touch-layout.toml.
 *
 * The editor is accessed from the launcher settings menu via
 * M12_MENU_VIEW_TOUCH_LAYOUT.
 */

#include "input_remap_m12.h"  /* M12_InputAction, M12_ACTION_COUNT */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Limits ─────────────────────────────────────────────────────────── */

enum {
    M12_TOUCH_MAX_ZONES       = 32,
    M12_TOUCH_LABEL_MAX       = 24,
    M12_TOUCH_PRESET_NAME_MAX = 32,
    M12_TOUCH_MAX_PRESETS     = 8,
    M12_TOUCH_MIN_ZONE_SIZE   = 24,
    M12_TOUCH_CANVAS_W        = 1280,
    M12_TOUCH_CANVAS_H        = 720
};

/* ── Touch zone ─────────────────────────────────────────────────────── */

typedef struct {
    int              x;           /* top-left X (canvas coords, 1280x720) */
    int              y;           /* top-left Y */
    int              w;           /* width  */
    int              h;           /* height */
    M12_InputAction  action;      /* bound game action */
    float            opacity;     /* 0.0–1.0 overlay opacity */
    char             label[M12_TOUCH_LABEL_MAX];
    int              visible;     /* 0 = hidden, 1 = visible */
} M12_TouchZone;

/* ── Preset identifier ──────────────────────────────────────────────── */

typedef enum {
    M12_TOUCH_PRESET_CLASSIC = 0,
    M12_TOUCH_PRESET_COMPACT,
    M12_TOUCH_PRESET_ONE_HANDED,
    M12_TOUCH_PRESET_COUNT
} M12_TouchPreset;

/* ── Touch layout (full zone set) ───────────────────────────────────── */

typedef struct {
    M12_TouchZone    zones[M12_TOUCH_MAX_ZONES];
    int              zoneCount;
    char             presetName[M12_TOUCH_PRESET_NAME_MAX];
} M12_TouchLayout;

/* ── Editor interaction mode ────────────────────────────────────────── */

typedef enum {
    M12_TOUCH_EDIT_IDLE = 0,
    M12_TOUCH_EDIT_DRAGGING,
    M12_TOUCH_EDIT_RESIZING
} M12_TouchEditMode;

/* ── Resize handle ──────────────────────────────────────────────────── */

typedef enum {
    M12_RESIZE_NONE = 0,
    M12_RESIZE_TOP_LEFT,
    M12_RESIZE_TOP_RIGHT,
    M12_RESIZE_BOTTOM_LEFT,
    M12_RESIZE_BOTTOM_RIGHT
} M12_ResizeHandle;

/* ── Editor state ───────────────────────────────────────────────────── */

typedef struct {
    M12_TouchLayout   layout;
    M12_TouchEditMode editMode;
    int               selectedZone;    /* -1 = none */
    M12_ResizeHandle  resizeHandle;
    int               dragOffsetX;     /* grab offset within zone */
    int               dragOffsetY;
    int               dirty;           /* unsaved changes flag */
    int               presetIndex;     /* current preset (M12_TouchPreset) */
    int               showGrid;        /* snap grid overlay */
    int               editorRow;       /* menu row selection */
} M12_TouchLayoutEditor;

/* ── Preset management ──────────────────────────────────────────────── */

void M12_TouchLayout_LoadPreset(M12_TouchLayout* layout,
                                M12_TouchPreset preset);
const char* M12_TouchLayout_PresetName(M12_TouchPreset preset);

/* ── Layout API ─────────────────────────────────────────────────────── */

void M12_TouchLayout_SetDefaults(M12_TouchLayout* layout);
int  M12_TouchLayout_HitTest(const M12_TouchLayout* layout,
                             int canvasX, int canvasY);
M12_ResizeHandle M12_TouchLayout_HitTestHandle(const M12_TouchLayout* layout,
                                               int zoneIndex,
                                               int canvasX, int canvasY);
void M12_TouchLayout_ClampZone(M12_TouchZone* zone);

/* ── Editor API ─────────────────────────────────────────────────────── */

void M12_TouchLayoutEditor_Init(M12_TouchLayoutEditor* ed);
void M12_TouchLayoutEditor_SelectPreset(M12_TouchLayoutEditor* ed,
                                        M12_TouchPreset preset);

/* Pointer events (canvas coordinates) */
void M12_TouchLayoutEditor_PointerDown(M12_TouchLayoutEditor* ed,
                                       int canvasX, int canvasY);
void M12_TouchLayoutEditor_PointerMove(M12_TouchLayoutEditor* ed,
                                       int canvasX, int canvasY);
void M12_TouchLayoutEditor_PointerUp(M12_TouchLayoutEditor* ed);

/* Zone property changes from the editor UI */
void M12_TouchLayoutEditor_SetZoneOpacity(M12_TouchLayoutEditor* ed,
                                          int zoneIndex, float opacity);
void M12_TouchLayoutEditor_SetZoneVisible(M12_TouchLayoutEditor* ed,
                                          int zoneIndex, int visible);
void M12_TouchLayoutEditor_DeleteZone(M12_TouchLayoutEditor* ed,
                                      int zoneIndex);

/* ── Persistence (touch-layout.toml) ────────────────────────────────── */

int M12_TouchLayout_Save(const M12_TouchLayout* layout);
int M12_TouchLayout_Load(M12_TouchLayout* layout);

/* ── Rendering ──────────────────────────────────────────────────────── */

void M12_TouchLayoutEditor_Draw(const M12_TouchLayoutEditor* ed,
                                unsigned char* framebuffer,
                                int fbWidth, int fbHeight);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_TOUCH_LAYOUT_M12_H */
