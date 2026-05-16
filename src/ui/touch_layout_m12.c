#include "touch_layout_m12.h"
#include "fs_portable_compat.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

/* ── Internal helpers ───────────────────────────────────────────────── */

static void m12_tl_copy_string(char* out, size_t outSize, const char* value) {
    if (!out || outSize == 0U) return;
    if (!value) value = "";
    snprintf(out, outSize, "%s", value);
}

static void m12_tl_trim(char* text) {
    char* start;
    size_t len;
    if (!text) return;
    start = text;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')
        ++start;
    if (start != text)
        memmove(text, start, strlen(start) + 1U);
    len = strlen(text);
    while (len > 0U) {
        char* end = &text[len - 1U];
        if (*end != ' ' && *end != '\t' && *end != '\r' && *end != '\n')
            break;
        *end = '\0';
        --len;
    }
}

static int m12_tl_parse_int(const char* value, int fallback) {
    char* end = NULL;
    long parsed;
    if (!value || value[0] == '\0') return fallback;
    parsed = strtol(value, &end, 10);
    if (!end || *end != '\0') return fallback;
    return (int)parsed;
}

static float m12_tl_parse_float(const char* value, float fallback) {
    char* end = NULL;
    float parsed;
    if (!value || value[0] == '\0') return fallback;
    parsed = (float)strtod(value, &end);
    if (!end || *end != '\0') return fallback;
    return parsed;
}

static int m12_tl_clamp(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float m12_tl_clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Action name table (matches M12_InputAction order) */
static const char* s_action_names[] = {
    "move_forward",    "move_backward",
    "turn_left",       "turn_right",
    "strafe_left",     "strafe_right",
    "accept",          "back",
    "action",          "cycle_champion",
    "rest_toggle",     "use_stairs",
    "pickup_item",     "drop_item",
    "spell_rune_1",    "spell_rune_2",
    "spell_rune_3",    "spell_rune_4",
    "spell_rune_5",    "spell_rune_6",
    "spell_cast",      "spell_clear",
    "use_item",        "map_toggle",
    "inventory_toggle","quick_save",
    "quick_load"
};

static M12_InputAction m12_tl_action_from_name(const char* name) {
    int i;
    for (i = 0; i < M12_ACTION_COUNT; ++i) {
        if (strcmp(name, s_action_names[i]) == 0)
            return (M12_InputAction)i;
    }
    return M12_ACTION_MOVE_FORWARD; /* fallback */
}

static const char* m12_tl_action_name(M12_InputAction a) {
    if (a >= 0 && a < M12_ACTION_COUNT)
        return s_action_names[a];
    return "unknown";
}

/* ── Preset definitions ─────────────────────────────────────────────── */

/*
 * Default zone helper.  All coordinates are in 1280x720 canvas space.
 * The layout mirrors a typical mobile RPG: D-pad on the left, action
 * buttons on the right, utility buttons at the top.
 */
static void m12_tl_add_zone(M12_TouchLayout* layout,
                            int x, int y, int w, int h,
                            M12_InputAction action, float opacity,
                            const char* label) {
    M12_TouchZone* z;
    if (layout->zoneCount >= M12_TOUCH_MAX_ZONES) return;
    z = &layout->zones[layout->zoneCount];
    z->x = x;
    z->y = y;
    z->w = w;
    z->h = h;
    z->action = action;
    z->opacity = opacity;
    z->visible = 1;
    m12_tl_copy_string(z->label, sizeof(z->label), label);
    layout->zoneCount++;
}

static void m12_tl_load_classic(M12_TouchLayout* layout) {
    memset(layout, 0, sizeof(*layout));
    m12_tl_copy_string(layout->presetName, sizeof(layout->presetName), "Classic");

    /* D-pad: left side */
    m12_tl_add_zone(layout,  60, 420, 100, 100, M12_ACTION_MOVE_FORWARD,  0.5f, "Forward");
    m12_tl_add_zone(layout,  60, 580, 100, 100, M12_ACTION_MOVE_BACKWARD, 0.5f, "Back");
    m12_tl_add_zone(layout,   0, 500, 100, 100, M12_ACTION_TURN_LEFT,     0.5f, "Turn L");
    m12_tl_add_zone(layout, 120, 500, 100, 100, M12_ACTION_TURN_RIGHT,    0.5f, "Turn R");
    m12_tl_add_zone(layout,   0, 400, 80,  80,  M12_ACTION_STRAFE_LEFT,   0.4f, "Strafe L");
    m12_tl_add_zone(layout, 140, 400, 80,  80,  M12_ACTION_STRAFE_RIGHT,  0.4f, "Strafe R");

    /* A/B buttons: right side */
    m12_tl_add_zone(layout, 1100, 480, 100, 100, M12_ACTION_ACTION,  0.5f, "Action");
    m12_tl_add_zone(layout, 1100, 600, 100, 100, M12_ACTION_BACK,    0.5f, "Cancel");
    m12_tl_add_zone(layout, 1000, 540, 80,  80,  M12_ACTION_ACCEPT,  0.5f, "Accept");

    /* Inventory button: top-right */
    m12_tl_add_zone(layout, 1160,  20,  90,  60, M12_ACTION_INVENTORY_TOGGLE, 0.4f, "Inv");

    /* Spell cast: right-center */
    m12_tl_add_zone(layout, 1060, 360,  90,  60, M12_ACTION_SPELL_CAST, 0.4f, "Cast");

    /* Map toggle: top-left */
    m12_tl_add_zone(layout,   20,  20,  90,  60, M12_ACTION_MAP_TOGGLE, 0.4f, "Map");
}

static void m12_tl_load_compact(M12_TouchLayout* layout) {
    memset(layout, 0, sizeof(*layout));
    m12_tl_copy_string(layout->presetName, sizeof(layout->presetName), "Compact");

    /* Smaller D-pad, tighter grouping */
    m12_tl_add_zone(layout,  40, 460,  80,  80, M12_ACTION_MOVE_FORWARD,  0.6f, "Fwd");
    m12_tl_add_zone(layout,  40, 600,  80,  80, M12_ACTION_MOVE_BACKWARD, 0.6f, "Bck");
    m12_tl_add_zone(layout,   0, 530,  80,  80, M12_ACTION_TURN_LEFT,     0.6f, "TL");
    m12_tl_add_zone(layout,  80, 530,  80,  80, M12_ACTION_TURN_RIGHT,    0.6f, "TR");

    /* Two buttons only */
    m12_tl_add_zone(layout, 1140, 520,  80,  80, M12_ACTION_ACTION, 0.6f, "Act");
    m12_tl_add_zone(layout, 1140, 620,  80,  80, M12_ACTION_BACK,   0.6f, "Back");

    /* Top bar */
    m12_tl_add_zone(layout,   10,  10,  70,  50, M12_ACTION_MAP_TOGGLE,       0.35f, "Map");
    m12_tl_add_zone(layout, 1200,  10,  70,  50, M12_ACTION_INVENTORY_TOGGLE, 0.35f, "Inv");
    m12_tl_add_zone(layout, 1120,  10,  70,  50, M12_ACTION_SPELL_CAST,       0.35f, "Cast");
}

static void m12_tl_load_one_handed(M12_TouchLayout* layout) {
    memset(layout, 0, sizeof(*layout));
    m12_tl_copy_string(layout->presetName, sizeof(layout->presetName), "One-handed");

    /* Everything on the right side, within thumb reach */
    m12_tl_add_zone(layout, 1000, 380, 100, 100, M12_ACTION_MOVE_FORWARD,  0.5f, "Fwd");
    m12_tl_add_zone(layout, 1000, 560, 100, 100, M12_ACTION_MOVE_BACKWARD, 0.5f, "Bck");
    m12_tl_add_zone(layout,  900, 470, 100, 100, M12_ACTION_TURN_LEFT,     0.5f, "TL");
    m12_tl_add_zone(layout, 1100, 470, 100, 100, M12_ACTION_TURN_RIGHT,    0.5f, "TR");

    m12_tl_add_zone(layout, 1160, 360,  80,  80, M12_ACTION_ACTION,  0.5f, "Act");
    m12_tl_add_zone(layout, 1160, 560,  80,  80, M12_ACTION_BACK,    0.5f, "Back");

    m12_tl_add_zone(layout, 1200,  10,  70,  50, M12_ACTION_INVENTORY_TOGGLE, 0.4f, "Inv");
    m12_tl_add_zone(layout, 1120,  10,  70,  50, M12_ACTION_SPELL_CAST,       0.4f, "Cast");
    m12_tl_add_zone(layout, 1040,  10,  70,  50, M12_ACTION_MAP_TOGGLE,       0.4f, "Map");
}

/* ── Preset management ──────────────────────────────────────────────── */

void M12_TouchLayout_LoadPreset(M12_TouchLayout* layout, M12_TouchPreset preset) {
    if (!layout) return;
    switch (preset) {
        case M12_TOUCH_PRESET_CLASSIC:     m12_tl_load_classic(layout);     break;
        case M12_TOUCH_PRESET_COMPACT:     m12_tl_load_compact(layout);     break;
        case M12_TOUCH_PRESET_ONE_HANDED:  m12_tl_load_one_handed(layout);  break;
        default:                           m12_tl_load_classic(layout);     break;
    }
}

const char* M12_TouchLayout_PresetName(M12_TouchPreset preset) {
    switch (preset) {
        case M12_TOUCH_PRESET_CLASSIC:    return "Classic";
        case M12_TOUCH_PRESET_COMPACT:    return "Compact";
        case M12_TOUCH_PRESET_ONE_HANDED: return "One-handed";
        default: return "Unknown";
    }
}

/* ── Layout API ─────────────────────────────────────────────────────── */

void M12_TouchLayout_SetDefaults(M12_TouchLayout* layout) {
    if (!layout) return;
    M12_TouchLayout_LoadPreset(layout, M12_TOUCH_PRESET_CLASSIC);
}

void M12_TouchLayout_ClampZone(M12_TouchZone* zone) {
    if (!zone) return;
    if (zone->w < M12_TOUCH_MIN_ZONE_SIZE) zone->w = M12_TOUCH_MIN_ZONE_SIZE;
    if (zone->h < M12_TOUCH_MIN_ZONE_SIZE) zone->h = M12_TOUCH_MIN_ZONE_SIZE;
    zone->x = m12_tl_clamp(zone->x, 0, M12_TOUCH_CANVAS_W - zone->w);
    zone->y = m12_tl_clamp(zone->y, 0, M12_TOUCH_CANVAS_H - zone->h);
    zone->opacity = m12_tl_clampf(zone->opacity, 0.0f, 1.0f);
}

int M12_TouchLayout_HitTest(const M12_TouchLayout* layout,
                            int canvasX, int canvasY) {
    int i;
    if (!layout) return -1;
    /* Reverse iteration: topmost zone wins */
    for (i = layout->zoneCount - 1; i >= 0; --i) {
        const M12_TouchZone* z = &layout->zones[i];
        if (!z->visible) continue;
        if (canvasX >= z->x && canvasX < z->x + z->w &&
            canvasY >= z->y && canvasY < z->y + z->h) {
            return i;
        }
    }
    return -1;
}

M12_ResizeHandle M12_TouchLayout_HitTestHandle(const M12_TouchLayout* layout,
                                               int zoneIndex,
                                               int canvasX, int canvasY) {
    const M12_TouchZone* z;
    int handleSize = 16;
    if (!layout || zoneIndex < 0 || zoneIndex >= layout->zoneCount)
        return M12_RESIZE_NONE;
    z = &layout->zones[zoneIndex];

    /* Top-left corner */
    if (canvasX >= z->x && canvasX < z->x + handleSize &&
        canvasY >= z->y && canvasY < z->y + handleSize)
        return M12_RESIZE_TOP_LEFT;

    /* Top-right corner */
    if (canvasX >= z->x + z->w - handleSize && canvasX < z->x + z->w &&
        canvasY >= z->y && canvasY < z->y + handleSize)
        return M12_RESIZE_TOP_RIGHT;

    /* Bottom-left corner */
    if (canvasX >= z->x && canvasX < z->x + handleSize &&
        canvasY >= z->y + z->h - handleSize && canvasY < z->y + z->h)
        return M12_RESIZE_BOTTOM_LEFT;

    /* Bottom-right corner */
    if (canvasX >= z->x + z->w - handleSize && canvasX < z->x + z->w &&
        canvasY >= z->y + z->h - handleSize && canvasY < z->y + z->h)
        return M12_RESIZE_BOTTOM_RIGHT;

    return M12_RESIZE_NONE;
}

/* ── Editor API ─────────────────────────────────────────────────────── */

void M12_TouchLayoutEditor_Init(M12_TouchLayoutEditor* ed) {
    if (!ed) return;
    memset(ed, 0, sizeof(*ed));
    ed->selectedZone = -1;
    ed->presetIndex = M12_TOUCH_PRESET_CLASSIC;
    M12_TouchLayout_LoadPreset(&ed->layout, M12_TOUCH_PRESET_CLASSIC);
}

void M12_TouchLayoutEditor_SelectPreset(M12_TouchLayoutEditor* ed,
                                        M12_TouchPreset preset) {
    if (!ed) return;
    if (preset < 0 || preset >= M12_TOUCH_PRESET_COUNT)
        preset = M12_TOUCH_PRESET_CLASSIC;
    ed->presetIndex = preset;
    M12_TouchLayout_LoadPreset(&ed->layout, preset);
    ed->selectedZone = -1;
    ed->editMode = M12_TOUCH_EDIT_IDLE;
    ed->dirty = 1;
}

void M12_TouchLayoutEditor_PointerDown(M12_TouchLayoutEditor* ed,
                                       int canvasX, int canvasY) {
    int hit;
    M12_ResizeHandle handle;
    if (!ed) return;

    hit = M12_TouchLayout_HitTest(&ed->layout, canvasX, canvasY);
    if (hit < 0) {
        ed->selectedZone = -1;
        ed->editMode = M12_TOUCH_EDIT_IDLE;
        return;
    }

    ed->selectedZone = hit;

    /* Check resize handles on selected zone */
    handle = M12_TouchLayout_HitTestHandle(&ed->layout, hit, canvasX, canvasY);
    if (handle != M12_RESIZE_NONE) {
        ed->editMode = M12_TOUCH_EDIT_RESIZING;
        ed->resizeHandle = handle;
        ed->dragOffsetX = canvasX;
        ed->dragOffsetY = canvasY;
    } else {
        ed->editMode = M12_TOUCH_EDIT_DRAGGING;
        ed->dragOffsetX = canvasX - ed->layout.zones[hit].x;
        ed->dragOffsetY = canvasY - ed->layout.zones[hit].y;
    }
}

void M12_TouchLayoutEditor_PointerMove(M12_TouchLayoutEditor* ed,
                                       int canvasX, int canvasY) {
    M12_TouchZone* z;
    int dx, dy;
    if (!ed || ed->selectedZone < 0) return;
    z = &ed->layout.zones[ed->selectedZone];

    if (ed->editMode == M12_TOUCH_EDIT_DRAGGING) {
        z->x = canvasX - ed->dragOffsetX;
        z->y = canvasY - ed->dragOffsetY;
        M12_TouchLayout_ClampZone(z);
        ed->dirty = 1;
    } else if (ed->editMode == M12_TOUCH_EDIT_RESIZING) {
        dx = canvasX - ed->dragOffsetX;
        dy = canvasY - ed->dragOffsetY;
        ed->dragOffsetX = canvasX;
        ed->dragOffsetY = canvasY;

        switch (ed->resizeHandle) {
            case M12_RESIZE_TOP_LEFT:
                z->x += dx; z->y += dy;
                z->w -= dx; z->h -= dy;
                break;
            case M12_RESIZE_TOP_RIGHT:
                z->y += dy;
                z->w += dx; z->h -= dy;
                break;
            case M12_RESIZE_BOTTOM_LEFT:
                z->x += dx;
                z->w -= dx; z->h += dy;
                break;
            case M12_RESIZE_BOTTOM_RIGHT:
                z->w += dx; z->h += dy;
                break;
            default:
                break;
        }
        M12_TouchLayout_ClampZone(z);
        ed->dirty = 1;
    }
}

void M12_TouchLayoutEditor_PointerUp(M12_TouchLayoutEditor* ed) {
    if (!ed) return;
    ed->editMode = M12_TOUCH_EDIT_IDLE;
    ed->resizeHandle = M12_RESIZE_NONE;
}

void M12_TouchLayoutEditor_SetZoneOpacity(M12_TouchLayoutEditor* ed,
                                          int zoneIndex, float opacity) {
    if (!ed || zoneIndex < 0 || zoneIndex >= ed->layout.zoneCount) return;
    ed->layout.zones[zoneIndex].opacity = m12_tl_clampf(opacity, 0.0f, 1.0f);
    ed->dirty = 1;
}

void M12_TouchLayoutEditor_SetZoneVisible(M12_TouchLayoutEditor* ed,
                                          int zoneIndex, int visible) {
    if (!ed || zoneIndex < 0 || zoneIndex >= ed->layout.zoneCount) return;
    ed->layout.zones[zoneIndex].visible = visible ? 1 : 0;
    ed->dirty = 1;
}

void M12_TouchLayoutEditor_DeleteZone(M12_TouchLayoutEditor* ed,
                                      int zoneIndex) {
    int i;
    if (!ed || zoneIndex < 0 || zoneIndex >= ed->layout.zoneCount) return;
    for (i = zoneIndex; i < ed->layout.zoneCount - 1; ++i)
        ed->layout.zones[i] = ed->layout.zones[i + 1];
    ed->layout.zoneCount--;
    if (ed->selectedZone == zoneIndex)
        ed->selectedZone = -1;
    else if (ed->selectedZone > zoneIndex)
        ed->selectedZone--;
    ed->dirty = 1;
}

/* ── Persistence ────────────────────────────────────────────────────── */

static int m12_tl_get_config_path(char* out, size_t outSize) {
    char configDir[FSP_PATH_MAX];
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        if (FSP_JoinPath(out, outSize, configDir, "touch-layout.toml"))
            return 1;
    }
    snprintf(out, outSize, "touch-layout.toml");
    return 1;
}

int M12_TouchLayout_Save(const M12_TouchLayout* layout) {
    char path[FSP_PATH_MAX];
    FILE* fp;
    int i;

    if (!layout) return 0;
    m12_tl_get_config_path(path, sizeof(path));

    fp = fopen(path, "w");
    if (!fp) return 0;

    fprintf(fp, "# Firestaff touch layout configuration\n");
    fprintf(fp, "# Delete this file to reset to defaults.\n\n");

    fprintf(fp, "[layout]\n");
    fprintf(fp, "preset = \"%s\"\n", layout->presetName);
    fprintf(fp, "zone_count = %d\n\n", layout->zoneCount);

    for (i = 0; i < layout->zoneCount; ++i) {
        const M12_TouchZone* z = &layout->zones[i];
        fprintf(fp, "[[zone]]\n");
        fprintf(fp, "label = \"%s\"\n", z->label);
        fprintf(fp, "action = \"%s\"\n", m12_tl_action_name(z->action));
        fprintf(fp, "x = %d\n", z->x);
        fprintf(fp, "y = %d\n", z->y);
        fprintf(fp, "w = %d\n", z->w);
        fprintf(fp, "h = %d\n", z->h);
        fprintf(fp, "opacity = %.2f\n", (double)z->opacity);
        fprintf(fp, "visible = %d\n\n", z->visible);
    }

    fclose(fp);
    return 1;
}

int M12_TouchLayout_Load(M12_TouchLayout* layout) {
    char path[FSP_PATH_MAX];
    FILE* fp;
    char line[256];
    int currentZone = -1;

    if (!layout) return 0;
    m12_tl_get_config_path(path, sizeof(path));

    fp = fopen(path, "r");
    if (!fp) return 0;

    memset(layout, 0, sizeof(*layout));

    while (fgets(line, (int)sizeof(line), fp)) {
        char* eq;
        char key[256];
        char value[192];

        m12_tl_trim(line);
        if (line[0] == '#' || line[0] == '\0') continue;

        /* Array-of-tables marker for a new zone */
        if (strcmp(line, "[[zone]]") == 0) {
            if (layout->zoneCount < M12_TOUCH_MAX_ZONES) {
                currentZone = layout->zoneCount;
                layout->zoneCount++;
                memset(&layout->zones[currentZone], 0, sizeof(M12_TouchZone));
                layout->zones[currentZone].visible = 1;
                layout->zones[currentZone].opacity = 0.5f;
            }
            continue;
        }

        /* Section headers (skip) */
        if (line[0] == '[') continue;

        eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        snprintf(key, sizeof(key), "%s", line);
        snprintf(value, sizeof(value), "%s", eq + 1);
        m12_tl_trim(key);
        m12_tl_trim(value);

        /* Strip quotes from string values */
        {
            size_t vlen = strlen(value);
            if (vlen >= 2U && value[0] == '"' && value[vlen - 1U] == '"') {
                memmove(value, value + 1, vlen - 2U);
                value[vlen - 2U] = '\0';
            }
        }

        /* Layout-level keys */
        if (currentZone < 0) {
            if (strcmp(key, "preset") == 0) {
                m12_tl_copy_string(layout->presetName,
                                   sizeof(layout->presetName), value);
            }
            /* zone_count is informational; we derive it from [[zone]] blocks */
            continue;
        }

        /* Zone-level keys */
        {
            M12_TouchZone* z = &layout->zones[currentZone];
            if (strcmp(key, "label") == 0)
                m12_tl_copy_string(z->label, sizeof(z->label), value);
            else if (strcmp(key, "action") == 0)
                z->action = m12_tl_action_from_name(value);
            else if (strcmp(key, "x") == 0)
                z->x = m12_tl_parse_int(value, 0);
            else if (strcmp(key, "y") == 0)
                z->y = m12_tl_parse_int(value, 0);
            else if (strcmp(key, "w") == 0)
                z->w = m12_tl_parse_int(value, 80);
            else if (strcmp(key, "h") == 0)
                z->h = m12_tl_parse_int(value, 80);
            else if (strcmp(key, "opacity") == 0)
                z->opacity = m12_tl_parse_float(value, 0.5f);
            else if (strcmp(key, "visible") == 0)
                z->visible = m12_tl_parse_int(value, 1);
        }
    }

    fclose(fp);
    return 1;
}

/* ── Rendering (editor overlay) ─────────────────────────────────────── */

/*
 * Draw a simple rectangle outline into an RGBA framebuffer.
 * Used for zone outlines and selection highlights.
 */
static void m12_tl_draw_rect(unsigned char* fb, int fbW, int fbH,
                             int rx, int ry, int rw, int rh,
                             unsigned char r, unsigned char g,
                             unsigned char b, unsigned char a) {
    int x, y;
    int x0 = rx, y0 = ry, x1 = rx + rw - 1, y1 = ry + rh - 1;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= fbW) x1 = fbW - 1;
    if (y1 >= fbH) y1 = fbH - 1;

    /* Top and bottom edges */
    for (x = x0; x <= x1; ++x) {
        if (y0 >= 0 && y0 < fbH) {
            unsigned char* p = fb + (y0 * fbW + x) * 4;
            p[0] = r; p[1] = g; p[2] = b; p[3] = a;
        }
        if (y1 >= 0 && y1 < fbH) {
            unsigned char* p = fb + (y1 * fbW + x) * 4;
            p[0] = r; p[1] = g; p[2] = b; p[3] = a;
        }
    }
    /* Left and right edges */
    for (y = y0; y <= y1; ++y) {
        if (x0 >= 0 && x0 < fbW) {
            unsigned char* p = fb + (y * fbW + x0) * 4;
            p[0] = r; p[1] = g; p[2] = b; p[3] = a;
        }
        if (x1 >= 0 && x1 < fbW) {
            unsigned char* p = fb + (y * fbW + x1) * 4;
            p[0] = r; p[1] = g; p[2] = b; p[3] = a;
        }
    }
}

/* Fill a small square (resize handle indicator) */
static void m12_tl_fill_rect(unsigned char* fb, int fbW, int fbH,
                             int rx, int ry, int rw, int rh,
                             unsigned char r, unsigned char g,
                             unsigned char b, unsigned char a) {
    int x, y;
    for (y = ry; y < ry + rh; ++y) {
        if (y < 0 || y >= fbH) continue;
        for (x = rx; x < rx + rw; ++x) {
            if (x < 0 || x >= fbW) continue;
            unsigned char* p = fb + (y * fbW + x) * 4;
            p[0] = r; p[1] = g; p[2] = b; p[3] = a;
        }
    }
}

void M12_TouchLayoutEditor_Draw(const M12_TouchLayoutEditor* ed,
                                unsigned char* framebuffer,
                                int fbWidth, int fbHeight) {
    int i;
    if (!ed || !framebuffer) return;

    for (i = 0; i < ed->layout.zoneCount; ++i) {
        const M12_TouchZone* z = &ed->layout.zones[i];
        unsigned char alpha;
        int isSelected;

        if (!z->visible) continue;

        alpha = (unsigned char)(z->opacity * 200.0f);
        isSelected = (i == ed->selectedZone);

        /* Zone fill (semi-transparent) */
        {
            int x, y;
            for (y = z->y; y < z->y + z->h; ++y) {
                if (y < 0 || y >= fbHeight) continue;
                for (x = z->x; x < z->x + z->w; ++x) {
                    if (x < 0 || x >= fbWidth) continue;
                    unsigned char* p = framebuffer + (y * fbWidth + x) * 4;
                    /* Blend: darker tint for zone area */
                    unsigned char base = isSelected ? 80 : 40;
                    p[0] = base;
                    p[1] = isSelected ? 120 : 60;
                    p[2] = isSelected ? 200 : 120;
                    p[3] = alpha;
                }
            }
        }

        /* Zone outline */
        if (isSelected) {
            m12_tl_draw_rect(framebuffer, fbWidth, fbHeight,
                             z->x, z->y, z->w, z->h,
                             255, 200, 0, 255);
            /* Resize handles (8x8 filled squares at corners) */
            m12_tl_fill_rect(framebuffer, fbWidth, fbHeight,
                             z->x, z->y, 8, 8,
                             255, 255, 0, 255);
            m12_tl_fill_rect(framebuffer, fbWidth, fbHeight,
                             z->x + z->w - 8, z->y, 8, 8,
                             255, 255, 0, 255);
            m12_tl_fill_rect(framebuffer, fbWidth, fbHeight,
                             z->x, z->y + z->h - 8, 8, 8,
                             255, 255, 0, 255);
            m12_tl_fill_rect(framebuffer, fbWidth, fbHeight,
                             z->x + z->w - 8, z->y + z->h - 8, 8, 8,
                             255, 255, 0, 255);
        } else {
            m12_tl_draw_rect(framebuffer, fbWidth, fbHeight,
                             z->x, z->y, z->w, z->h,
                             100, 180, 255, 200);
        }
    }
}
