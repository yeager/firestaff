/*
 * menu_startup_a11y_m12.c — Launcher screen-reader / state manifest emitter
 *
 * Companion to firestaff_accessibility.c. Converts the public
 * M12_StartupMenuState into a stable list of fs_ax_* elements so the
 * launcher is reachable by external automation and screen readers.
 *
 * Scope (matches the open gap text in TODO.md and
 * docs/FIRESTAFF_GAP_LIST.md for "Screen reader launcher state
 * manifest"):
 *   - Main view (game cards)             → FS_AX_LAUNCHER_CARD per entry
 *                                           + a pinned data-availability value
 *                                             ("ready" | "data missing" | "selected | …")
 *   - Settings tabs                      → FS_AX_LAUNCHER_TAB per tab
 *   - Settings rows                      → FS_AX_LAUNCHER_ROW per visible row
 *   - Missing-data popup                 → FS_AX_POPUP + FS_AX_POPUP_OK
 *   - General message view               → FS_AX_POPUP + FS_AX_POPUP_OK
 *   - Bestiary view                      → FS_AX_CATEGORY_TAB per category
 *                                           + FS_AX_BESTIARY_ROW per visible creature
 *   - Item Encyclopedia view             → FS_AX_CATEGORY_TAB per category
 *                                           + FS_AX_ITEM_ENCYCLOPEDIA_ROW per visible item
 *   - Screenshot Gallery view            → FS_AX_SCREENSHOT_THUMB per visible entry
 *   - Museum of Lore view                → FS_AX_MUSEUM_CATEGORY per section
 *                                           + FS_AX_MUSEUM_BULLET per lore bullet on the active page
 *   - Manual / Docs view                 → FS_AX_LAUNCHER_ROW per public docs entry
 *   - Changelog view                     → FS_AX_LAUNCHER_ROW per visible changelog line
 *
 * Out of scope (kept for follow-up passes):
 *   - Quick-resume "CONTINUE" virtual entry on the main view
 *     (rendered inline in m12_draw_main_view, not in state->entries).
 *   - Data-validator / theme / save-browser / input-remap /
 *     custom-dungeon / campaign / spell-reference / map-viewer /
 *     touch-layout / presentation-preview views.
 *     Those still fall through to the main-view emission as a
 *     navigation anchor.
 *
 * Determinism contract:
 *   - Element order is pinned: cards before tabs, tabs before rows,
 *     popups last.
 *   - Element IDs are pinned ASCII ("GAME_CARD_DM1", "POPUP_OK", ...).
 *   - Bounding rects are integer framebuffer pixels; the values are
 *     the canonical render target used by M12_StartupMenu_Draw
 *     (480x270 legacy or 1920x1080 modern).
 *
 * Privacy contract:
 *   - The popup's data-dir text is suppressed unless the caller
 *     passes includePaths=1. The default keeps the manifest safe to
 *     commit under artifacts/ or attach to bug reports.
 *   - No filenames, MD5 hashes, or save-game paths leak. Game IDs are
 *     short stable tokens ("dm1", "csb", ...) already public in
 *     M12_AssetStatus output.
 */

#include "menu_startup_a11y_m12.h"
#include "firestaff_accessibility.h"
#include "bestiary_m12.h"
#include "changelog_m12.h"
#include "firestaff_item_encyclopedia.h"
#include "manual_docs_m12.h"
#include "screenshot_gallery_m12.h"

#include <stdio.h>
#include <string.h>

/* ── Element-order budget ───────────────────────────────────────────
 * The launcher manifest is small: 5 game cards + 1 museum + 1 settings
 * on the main view; up to 5 settings tabs + ~10 settings rows; one
 * popup. We stay well under the 128-element hard cap from
 * firestaff_accessibility.h so any future addition still fits. */

/* ── Game-id → stable card id mapping ─────────────────────────────── */

static const struct {
    const char* gameId;       /* value from state->entries[i].gameId */
    const char* elementId;    /* pinned screen-reader target */
    const char* fallbackLabel;/* used when entry->title is NULL */
} kGameCardTable[] = {
    { "dm1",    "GAME_CARD_DM1",    "Dungeon Master" },
    { "csb",    "GAME_CARD_CSB",    "Chaos Strikes Back" },
    { "dm2",    "GAME_CARD_DM2",    "Dungeon Master II" },
    { "nexus",  "GAME_CARD_NEXUS",  "Dungeon Master Nexus" },
    { "theron", "GAME_CARD_THERON", "Theron's Quest" },
    { NULL, NULL, NULL }
};

static const char* card_element_id_for(const char* gameId)
{
    int i;
    if (!gameId) {
        return "GAME_CARD_UNKNOWN";
    }
    for (i = 0; kGameCardTable[i].gameId != NULL; ++i) {
        if (strcmp(kGameCardTable[i].gameId, gameId) == 0) {
            return kGameCardTable[i].elementId;
        }
    }
    return "GAME_CARD_UNKNOWN";
}

static const char* card_fallback_label_for(const char* gameId)
{
    int i;
    if (!gameId) {
        return "Game";
    }
    for (i = 0; kGameCardTable[i].gameId != NULL; ++i) {
        if (strcmp(kGameCardTable[i].gameId, gameId) == 0) {
            return kGameCardTable[i].fallbackLabel;
        }
    }
    return "Game";
}

/* ── Settings tab → element id ────────────────────────────────────── */

typedef struct {
    const char* id;
    const char* label;
} TabSpec;

static const TabSpec kLegacyTabs[] = {
    { "TAB_GAME",         "Game" },
    { "TAB_GRAPHICS",     "Graphics" },
    { "TAB_CONTROLS",     "Controls" },
    { "TAB_AUDIO",        "Audio" },
    { "TAB_ACCESSIBILITY","Accessibility" },
    { NULL, NULL }
};

static const TabSpec kModernTabs[] = {
    { "TAB_DISPLAY",      "Display" },
    { "TAB_VIDEO",        "Video" },
    { "TAB_AUDIO",        "Audio" },
    { "TAB_CONTROLS",     "Controls" },
    { "TAB_ACCESSIBILITY","Accessibility" },
    { NULL, NULL }
};

static const TabSpec* tabs_for_view(M12_MenuView view, int* outCount)
{
    /* The legacy (M12_SETTINGS_TAB_*) enum and the modern
     * (M12_SETTINGS_TAB2_*) enum both have 5 tabs in the same
     * order they appear in menu_startup_m12.c. Distinguish by the
     * broader view the user is in: SETTINGS for legacy tab drawing,
     * GAME_OPTIONS / SETTINGS for the modern tab drawing. */
    if (view == M12_MENU_VIEW_SETTINGS) {
        if (outCount) *outCount = (int)(sizeof(kLegacyTabs) / sizeof(kLegacyTabs[0])) - 1;
        return kLegacyTabs;
    }
    if (outCount) *outCount = (int)(sizeof(kModernTabs) / sizeof(kModernTabs[0])) - 1;
    return kModernTabs;
}

/* ── Framebuffer-relative rectangle helpers ───────────────────────── */

typedef struct {
    int x, y, w, h;
} AxRect;

static AxRect scale_rect(AxRect base, int fbW, int fbH)
{
    /* The legacy launcher renders into 480x270 and the modern one
     * into 1920x1080. The card / tab / row positions in
     * menu_startup_m12.c are written against the 480x270 framebuffer,
     * so scale them up proportionally for the modern path. */
    AxRect out;
    out.x = (base.x * fbW) / 480;
    out.y = (base.y * fbH) / 270;
    out.w = (base.w * fbW) / 480;
    out.h = (base.h * fbH) / 270;
    if (out.w < 1) out.w = 1;
    if (out.h < 1) out.h = 1;
    return out;
}

static void add_element_rect(int fbW, int fbH,
                             const char* id, const char* label,
                             FS_AX_ElementType type,
                             AxRect base, int enabled, const char* value)
{
    FS_AX_Element e;
    AxRect r = scale_rect(base, fbW, fbH);
    memset(&e, 0, sizeof(e));
    e.id = id;
    e.label = label ? label : "";
    e.type = type;
    e.x = r.x;
    e.y = r.y;
    e.w = r.w;
    e.h = r.h;
    e.enabled = enabled ? 1 : 0;
    e.value = value;
    fs_ax_add_element(&e);
}

static void add_element_bounds(const char* id, const char* label,
                               FS_AX_ElementType type,
                               int x, int y, int w, int h,
                               int enabled, const char* value)
{
    FS_AX_Element e;
    memset(&e, 0, sizeof(e));
    e.id = id;
    e.label = label ? label : "";
    e.type = type;
    e.x = x;
    e.y = y;
    e.w = w > 0 ? w : 1;
    e.h = h > 0 ? h : 1;
    e.enabled = enabled ? 1 : 0;
    e.value = value;
    fs_ax_add_element(&e);
}

/* Keep this table ordinal-aligned with the private
 * M12_SETTINGS_ROW_* enum in menu_startup_m12.c. The enum is local
 * to that translation unit, so the manifest keeps a small mirrored
 * table here and the probe locks the rows most likely to drift. */
typedef struct {
    const char* id;
    const char* label;
} SettingsRowSpec;

static const SettingsRowSpec kSettingsRows[] = {
    { "ROW_LANGUAGE",          "Language" },
    { "ROW_PRESENTATION_MODE", "Presentation Mode" },
    { "ROW_RENDERER_BACKEND",  "Renderer Backend" },
    { "ROW_WINDOW_MODE",       "Window Mode" },
    { "ROW_SCALE_MODE",        "Scale" },
    { "ROW_DISPLAY_ASPECT",    "Display Format" },
    { "ROW_INTEGER_SCALING",   "Pixel Snap" },
    { "ROW_SCALING_FILTER",    "Filter" },
    { "ROW_VSYNC",             "VSync" },
    { "ROW_VIEWPORT_STYLE",    "Viewport Style" },
    { "ROW_INPUT_MODE",        "Input Mode" },
    { NULL,                    NULL }, /* reserved WASD row */
    { "ROW_TOUCH_CONTROLS",    "Touch Controls" },
    { "ROW_MOVEMENT_MODE",     "Movement Mode" },
    { "ROW_SMOOTH_TURN_PAN",   "Smooth Turn Pan" },
    { "ROW_DATA_DIR",          "Data Directory" },
    { "ROW_DATA_STATUS",       "Original Data" },
    { "ROW_DEBUG_OVERLAY",     "Debug Overlay" },
    { "ROW_DEVELOPER_GATES",   "Developer Gates" },
    { "ROW_AUDIO_MASTER",      "Master Volume" },
    { "ROW_AUDIO_MUSIC",       "Music Volume" },
    { "ROW_AUDIO_SFX",         "SFX Volume" },
    { "ROW_AUDIO_MUTED",       "Mute Audio" },
    { "ROW_FONT_SCALE",        "Font Scale" },
    { "ROW_HIGH_CONTRAST",     "High Contrast" },
    { "ROW_COLORBLIND_MODE",   "Colorblind Mode" },
    { "ROW_AUTO_PAUSE",        "Auto Pause" },
    { "ROW_THEME",             "Theme" },
    { "ROW_BACKGROUND",        "Background" },
    { "ROW_QUICK_RESUME",      "Quick Resume" },
    { "ROW_SESSION_TIMER",     "Session Timer" },
    { "ROW_MINIMAP",           "Minimap" },
    { "ROW_AUTOMAP",           "Automap" },
    { "ROW_COMBAT_LOG",        "Combat Log" },
    { "ROW_SOUNDTRACK",        "Soundtrack" },
    { "ROW_AMBIENT",           "Ambient Sound" },
    { "ROW_AMBIENT_VOLUME",    "Ambient Volume" },
    { "ROW_UI_SCALE",          "UI Scale" },
    { "ROW_CUSTOM_MUSIC",      "Custom Music" },
    { "ROW_CUSTOM_DUNGEON",    "Custom Dungeons" },
    { "ROW_SCREENSHOTS",       "Screenshots" },
    { "ROW_STREAMER_MODE",     "Streamer Mode" },
    { "ROW_EXPORT_SAVE",       "Export Save Manifest" },
    { "ROW_IMPORT_SAVE",       "Import Save Manifest" },
    { "ROW_SYNC_NOW",          "Sync Now" },
    { "ROW_SYNC_STATUS",       "Sync Status" }
};

enum {
    M12_A11Y_SETTINGS_ROW_RESERVED_WAS = 11,
    M12_A11Y_SETTINGS_ROW_DATA_DIR = 15,
    M12_A11Y_SETTINGS_ROW_DATA_STATUS = 16,
    M12_A11Y_SETTINGS_ROW_FONT_SCALE = 23,
    M12_A11Y_SETTINGS_ROW_COUNT =
        (int)(sizeof(kSettingsRows) / sizeof(kSettingsRows[0]))
};

static const char* on_off(int enabled)
{
    return enabled ? "ON" : "OFF";
}

static const char* set_default(const char* path)
{
    return (path && path[0] != '\0') ? "SET" : "DEFAULT";
}

static const char* setting_row_value(const M12_StartupMenuState* state,
                                     int row,
                                     int includePaths,
                                     char* out,
                                     size_t outSize)
{
    const M12_MenuSettingsState* s = state ? &state->settings : NULL;
    if (!out || outSize == 0) {
        return NULL;
    }
    out[0] = '\0';
    switch (row) {
    case 0:
        snprintf(out, outSize, "INDEX %d", s ? s->languageIndex : 0);
        break;
    case 1:
        snprintf(out, outSize, "%s",
                 M12_StartupMenu_GetPresentationModeLabel(state));
        break;
    case 2:
        snprintf(out, outSize, "%s | %s",
                 M12_StartupMenu_GetRendererBackendLabel(state),
                 M12_StartupMenu_GetRendererBackendStatusLabel(state));
        break;
    case 3:
        snprintf(out, outSize, "INDEX %d", s ? s->windowModeIndex : 0);
        break;
    case 4:
        snprintf(out, outSize, "INDEX %d", s ? s->scaleModeIndex : 0);
        break;
    case 5:
        snprintf(out, outSize, "INDEX %d", s ? s->displayAspectMode : 0);
        break;
    case 6:
        snprintf(out, outSize, "%s", on_off(s ? s->integerScaling : 0));
        break;
    case 7:
        snprintf(out, outSize, "INDEX %d", s ? s->scalingFilterIndex : 0);
        break;
    case 8:
        snprintf(out, outSize, "%s", on_off(s ? s->vsyncIndex : 0));
        break;
    case 9:
        snprintf(out, outSize, "INDEX %d", s ? s->viewportStyleIndex : 0);
        break;
    case 10:
        snprintf(out, outSize, "INDEX %d", s ? s->inputModeIndex : 0);
        break;
    case 12:
        snprintf(out, outSize, "%s", on_off(s ? s->touchControlsIndex : 0));
        break;
    case 13:
        snprintf(out, outSize, "INDEX %d", s ? s->movementModeIndex : 0);
        break;
    case 14:
        snprintf(out, outSize, "%s", on_off(s ? s->dm1V2SmoothTurnPanEnabled : 0));
        break;
    case M12_A11Y_SETTINGS_ROW_DATA_DIR:
        snprintf(out, outSize, "%s",
                 includePaths ? M12_StartupMenu_GetVisibleDataDir(state) : "hidden");
        break;
    case M12_A11Y_SETTINGS_ROW_DATA_STATUS:
        snprintf(out, outSize, "%s", M12_StartupMenu_GetDataStatusValue(state));
        break;
    case 17:
        snprintf(out, outSize, "INDEX %d", s ? s->debugOverlayIndex : 0);
        break;
    case 18:
        snprintf(out, outSize, "INDEX %d", s ? s->developerGatesIndex : 0);
        break;
    case 19:
        snprintf(out, outSize, "%d%%", s ? s->audioMasterVolume : 0);
        break;
    case 20:
        snprintf(out, outSize, "%d%%", s ? s->audioMusicVolume : 0);
        break;
    case 21:
        snprintf(out, outSize, "%d%%", s ? s->audioSfxVolume : 0);
        break;
    case 22:
        snprintf(out, outSize, "%s", on_off(s ? s->audioMuted : 0));
        break;
    case M12_A11Y_SETTINGS_ROW_FONT_SCALE:
        snprintf(out, outSize, "%dx", s ? s->fontScale : 1);
        break;
    case 24:
        snprintf(out, outSize, "%s", on_off(s ? s->highContrast : 0));
        break;
    case 25:
        snprintf(out, outSize, "INDEX %d", s ? s->colorblindMode : 0);
        break;
    case 26:
        snprintf(out, outSize, "%s", on_off(s ? s->autoPause : 0));
        break;
    case 27:
        snprintf(out, outSize, "INDEX %d", s ? s->themeIndex : 0);
        break;
    case 28:
        snprintf(out, outSize, "INDEX %d", s ? s->bgAnimationPreset : 0);
        break;
    case 29:
        snprintf(out, outSize, "%s", on_off(s ? s->quickResumeEnabled : 0));
        break;
    case 30:
        snprintf(out, outSize, "INDEX %d", s ? s->sessionTimerIndex : 0);
        break;
    case 31:
        snprintf(out, outSize, "%s", on_off(s ? s->minimapEnabled : 0));
        break;
    case 32:
        snprintf(out, outSize, "%s", on_off(s ? s->autoMapEnabled : 0));
        break;
    case 33:
        snprintf(out, outSize, "%s", on_off(s ? s->combatLogEnabled : 0));
        break;
    case 34:
        snprintf(out, outSize, "INDEX %d", s ? s->soundtrackMode : 0);
        break;
    case 35:
        snprintf(out, outSize, "%s", on_off(s ? s->ambientEnabled : 0));
        break;
    case 36:
        snprintf(out, outSize, "%d%%", s ? s->ambientVolume : 0);
        break;
    case 37:
        snprintf(out, outSize, "%d%%", s ? s->uiScale : 100);
        break;
    case 38:
        snprintf(out, outSize, "%s", set_default(s ? s->customMusicPath : NULL));
        break;
    case 39:
        snprintf(out, outSize, "%s", set_default(s ? s->customDungeonPath : NULL));
        break;
    case 40:
        snprintf(out, outSize, "%s", set_default(s ? s->screenshotPath : NULL));
        break;
    case 41:
        snprintf(out, outSize, "%s", on_off(s ? s->streamerMode : 0));
        break;
    case 42:
        snprintf(out, outSize, "WRITE");
        break;
    case 43:
        snprintf(out, outSize, "READ");
        break;
    case 44:
        snprintf(out, outSize, "NOT CONFIGURED");
        break;
    case 45:
        snprintf(out, outSize, "LOCAL ONLY");
        break;
    default:
        break;
    }
    return out[0] ? out : NULL;
}

static int settings_visible_rows(int fbW, int fbH)
{
    if (fbW <= 480 && fbH <= 270) {
        return 6;
    }
    {
        int margin = fbW / 30;
        int heroH = fbH / 3;
        int contentY;
        int availableH;
        int visibleRows;
        if (margin < 12) margin = 12;
        if (heroH < 82) heroH = 82;
        contentY = margin + heroH + 10;
        availableH = fbH - contentY - 92;
        visibleRows = availableH / 34;
        if (visibleRows < 4) visibleRows = 4;
        if (visibleRows > M12_A11Y_SETTINGS_ROW_COUNT) {
            visibleRows = M12_A11Y_SETTINGS_ROW_COUNT;
        }
        return visibleRows;
    }
}

static int settings_first_visible_row(const M12_StartupMenuState* state,
                                      int visibleRows)
{
    int selected = state ? state->settingsSelectedIndex : 0;
    int firstRow = selected - (visibleRows / 2);
    if (firstRow < 0) firstRow = 0;
    if (firstRow > M12_A11Y_SETTINGS_ROW_COUNT - visibleRows) {
        firstRow = M12_A11Y_SETTINGS_ROW_COUNT - visibleRows;
    }
    if (firstRow < 0) firstRow = 0;
    return firstRow;
}

/* ── Per-view emitters ────────────────────────────────────────────── */

/* Main view: emit one card element per state->entries[i]. For game
 * cards we also surface a data-availability reason on the element's
 * value so a screen reader can announce why a card is disabled
 * (e.g. "data missing" vs "ready") without the user having to look
 * at the launcher chrome. The labels below are pinned ASCII
 * (`data missing`, `ready`) so the manifest is safe to grep /
 * assert in tests and the screen-reader output stays deterministic
 * across runs.
 *
 * Privacy: the value text never embeds filesystem paths, MD5
 * hashes, or other fingerprints. The data-dir row in the settings
 * view is the only place that ever surfaces the absolute data
 * directory, and only when the caller passes includePaths=1.
 *
 * The "not supported" branch is intentionally not surfaced: every
 * entry in the static g_entryTemplate catalog is supported, so
 * `available == 0` on a catalogued game always means the data is
 * missing from the configured data dir. If a future pass adds an
 * unsupported-but-catalogued entry, the natural extension here is
 * to add a static m12_is_catalog_supported() lookup or expose the
 * existing m12_game_supported() helper. */
static const char* main_view_game_value(const M12_MenuEntry* entry, int selected)
{
    if (!entry || entry->kind != M12_MENU_ENTRY_GAME) {
        return selected ? "selected" : NULL;
    }
    if (selected) {
        return entry->available ? "selected | ready"
                                : "selected | data missing";
    }
    if (!entry->available) {
        return "data missing";
    }
    return NULL;
}

static void emit_main_view(const M12_StartupMenuState* state,
                           int fbW, int fbH)
{
    int i;
    int count = M12_StartupMenu_GetEntryCount();
    for (i = 0; i < count; ++i) {
        const M12_MenuEntry* entry = M12_StartupMenu_GetEntry(state, i);
        const char* id;
        const char* label;
        const char* gameId;
        int enabled;
        const char* value;
        int selected;
        /* Card row Y at 76, +24 per row. See m12_draw_main_view. */
        AxRect base = { 130, 76 + (i * 24), 168, 24 };

        if (!entry) {
            continue;
        }
        gameId = entry->gameId;
        if (entry->kind == M12_MENU_ENTRY_GAME && gameId) {
            id = card_element_id_for(gameId);
            label = entry->title ? entry->title : card_fallback_label_for(gameId);
            enabled = entry->available ? 1 : 0;
        } else if (entry->kind == M12_MENU_ENTRY_SETTINGS) {
            id = "MENU_SETTINGS";
            label = entry->title ? entry->title : "Settings";
            enabled = 1;
        } else if (entry->kind == M12_MENU_ENTRY_MUSEUM) {
            id = "MENU_MUSEUM";
            label = entry->title ? entry->title : "Museum of Lore";
            enabled = 1;
        } else {
            id = "MENU_UNKNOWN";
            label = entry->title ? entry->title : "Item";
            enabled = entry->available ? 1 : 0;
        }

        selected = (state->selectedIndex == i);
        value = main_view_game_value(entry, selected);
        add_element_rect(fbW, fbH, id, label,
                         FS_AX_LAUNCHER_CARD, base, enabled,
                         value);
    }
}

/* Settings view: tabs first, then rows. Row labels mirror the strings
 * used by m12_draw_settings_view / m12_draw_settings_view_modern. */
static void emit_settings_view(const M12_StartupMenuState* state,
                               int fbW, int fbH,
                               int includePaths)
{
    const TabSpec* tabs;
    int tabCount = 0;
    int i;

    tabs = tabs_for_view(state->view, &tabCount);

    /* Tab strip rect: from the legacy draw at y=42, height=18, full width */
    for (i = 0; i < tabCount; ++i) {
        AxRect base = { 122 + (i * 60), 42, 60, 18 };
        int isSelected = (state->settingsTabIndex == i);
        add_element_rect(fbW, fbH,
                         tabs[i].id ? tabs[i].id : "TAB_UNKNOWN",
                         tabs[i].label ? tabs[i].label : "",
                         FS_AX_LAUNCHER_TAB, base,
                         1,
                         isSelected ? "selected" : NULL);
    }

    /* Settings rows: mirror the visible window from
     * m12_draw_settings_view / m12_draw_settings_view_modern using
     * the same selected-row centering and reserved-row skip. */
    {
        int visibleRows = settings_visible_rows(fbW, fbH);
        int firstRow = settings_first_visible_row(state, visibleRows);
        int rowSlot = 0;
        int row;
        int x;
        int y;
        int w;
        int h;
        int rowStep;
        static char s_setting_values[M12_A11Y_SETTINGS_ROW_COUNT][128];

        if (fbW <= 480 && fbH <= 270) {
            x = 122;
            y = 70;
            w = 228;
            h = 18;
            rowStep = 18;
        } else {
            int margin = fbW / 30;
            int heroH = fbH / 3;
            int contentY;
            int leftW;
            int panelX;
            if (margin < 12) margin = 12;
            if (heroH < 82) heroH = 82;
            contentY = margin + heroH + 10;
            leftW = (fbW * 38) / 100;
            panelX = margin + leftW + 12;
            x = panelX + 10;
            y = contentY + 36;
            w = fbW - margin - panelX - 20;
            h = 34;
            rowStep = 34;
        }

        for (row = firstRow;
             row < M12_A11Y_SETTINGS_ROW_COUNT && row < firstRow + visibleRows;
             ++row) {
            const SettingsRowSpec* spec = &kSettingsRows[row];
            const char* value;
            if (row == M12_A11Y_SETTINGS_ROW_RESERVED_WAS || !spec->id) {
                continue;
            }
            value = setting_row_value(state, row, includePaths,
                                      s_setting_values[row],
                                      sizeof(s_setting_values[row]));
            if (state && state->settingsSelectedIndex == row) {
                if (value && value[0]) {
                    char existing[128];
                    snprintf(existing, sizeof(existing), "%s", value);
                    snprintf(s_setting_values[row],
                             sizeof(s_setting_values[row]),
                             "selected | %s", existing);
                } else {
                    snprintf(s_setting_values[row],
                             sizeof(s_setting_values[row]),
                             "selected");
                }
                value = s_setting_values[row];
            }
            add_element_bounds(spec->id, spec->label,
                               FS_AX_LAUNCHER_ROW,
                               x, y + (rowSlot * rowStep), w, h,
                               1, value);
            ++rowSlot;
        }
    }
}

/* Game-options view: just expose the row labels so a screen reader
 * can announce them as the user tabs. */
static void emit_game_options_view(const M12_StartupMenuState* state,
                                   int fbW, int fbH)
{
    static const char* rowIds[] = {
        "GAME_OPT_PRESENTATION",
        "GAME_OPT_VERSION",
        "GAME_OPT_PATCH",
        "GAME_OPT_LANGUAGE",
        "GAME_OPT_CHEATS",
        "GAME_OPT_SPEED",
        "GAME_OPT_ASPECT",
        "GAME_OPT_RESOLUTION"
    };
    static const char* rowLabels[] = {
        "Presentation", "Version", "Patch", "Language",
        "Cheats", "Speed", "Aspect Ratio", "Resolution"
    };
    int i;
    int rowY = 76;
    int count = (int)(sizeof(rowIds) / sizeof(rowIds[0]));
    (void)state;
    for (i = 0; i < count; ++i) {
        AxRect base = { 122, rowY, 180, 24 };
        add_element_rect(fbW, fbH, rowIds[i], rowLabels[i],
                         FS_AX_LAUNCHER_ROW, base, 1, NULL);
        rowY += 24;
    }
}

/* ── Extras view emitters ────────────────────────────────────────────
 *
 * The four cell-by-cell views (bestiary, item encyclopedia,
 * screenshot gallery, museum of lore) all use the same per-cell row
 * model so a screen reader can announce "row N of M, NAME, selected"
 * without the caller having to learn a new pattern per view.
 *
 * Rect math mirrors menu_startup_m12.c m12_draw_*_view_modern:
 *   margin = framebufferWidth / 30
 *   heroH   = framebufferHeight / 4 (clamped to >= 64)
 *   contentY = margin + heroH + 10
 *   panelH = framebufferHeight - contentY - 28
 *   rowY  = contentY + 14 + i * 22  (i is visible row index)
 * The base rects below are in the 480x270 legacy framebuffer; the
 * scale_rect helper upscales them for 1920x1080. */

/* Bestiary view: one category tab per known category, then one
 * row per visible creature. The category tab strip is not
 * separately drawn in the modern bestiary view (the filter name
 * lives in the title), but the screen reader benefits from
 * surfacing each category tab so a user can navigate to "beasts"
 * or "dragons" by tab id. */
static void emit_bestiary_view(const M12_StartupMenuState* state,
                                int fbW, int fbH)
{
    int margin = 480 / 30;
    int heroH = 270 / 4;
    int contentY;
    int i;
    int filteredCount;
    int scrollOff;
    int visible;
    int categoryCount = (int)M12_BESTIARY_CAT_COUNT;
    int catTabY = 42; /* top tab strip; rowY starts below hero */

    /* FS_AX_Element stores a `const char*` pointer that lives until
     * fs_ax_flush() consumes it. Stack-allocated snprintf buffers
     * would dangle, so we pre-format the dynamic IDs into a static
     * scratch array below. */
    static char s_bestiary_cat_id[8][32];
    static char s_bestiary_cat_value[8][16];
    static char s_bestiary_row_id[12][32];
    static char s_bestiary_row_value[12][96];

    if (margin < 12) margin = 12;
    if (heroH < 64) heroH = 64;
    contentY = margin + heroH + 10;

    /* Emit category tabs. The legacy tab rect is 60w x 18h starting
     * at x=122. With 7 categories, lay them out in a single row. */
    for (i = 0; i < categoryCount; ++i) {
        AxRect base = { 122 + (i % 5) * 60, catTabY + (i / 5) * 22, 60, 18 };
        const char* name = M12_Bestiary_CategoryName((M12_BestiaryCategory)i);
        int isSelected = ((int)state->bestiary.categoryFilter == i);
        const char* selValue = isSelected ? "selected" : NULL;
        if ((int)(sizeof(s_bestiary_cat_id) / sizeof(s_bestiary_cat_id[0])) <= i) {
            continue;
        }
        snprintf(s_bestiary_cat_id[i], sizeof(s_bestiary_cat_id[i]),
                 "BESTIARY_CAT_%d", i);
        s_bestiary_cat_value[i][0] = '\0';
        if (isSelected) {
            snprintf(s_bestiary_cat_value[i], sizeof(s_bestiary_cat_value[i]),
                     "%s", "selected");
            selValue = s_bestiary_cat_value[i];
        }
        add_element_rect(fbW, fbH, s_bestiary_cat_id[i], name ? name : "",
                         FS_AX_CATEGORY_TAB, base, 1, selValue);
    }

    /* Emit creature rows. Row N is the Nth visible entry
     * (scrollOff..scrollOff+visible-1) in the filtered list. */
    filteredCount = M12_Bestiary_FilteredCount(&state->bestiary);
    scrollOff = state->bestiary.scrollOffset;
    visible = M12_BESTIARY_VISIBLE_LINES;
    if (visible > filteredCount - scrollOff) {
        visible = filteredCount - scrollOff;
    }
    if (visible < 0) visible = 0;
    if (visible > 12) visible = 12; /* hard cap for element budget */
    if (visible > (int)(sizeof(s_bestiary_row_id) / sizeof(s_bestiary_row_id[0]))) {
        visible = (int)(sizeof(s_bestiary_row_id) / sizeof(s_bestiary_row_id[0]));
    }

    for (i = 0; i < visible; ++i) {
        const M12_BestiaryEntry* e =
            M12_Bestiary_GetFiltered(&state->bestiary, scrollOff + i);
        if (!e) break;
        {
            AxRect base = { 130, contentY + 14 + (i * 22), 240, 22 };
            int isSelected = (state->bestiary.selectedIndex == scrollOff + i);
            const char* value;
            snprintf(s_bestiary_row_id[i], sizeof(s_bestiary_row_id[i]),
                     "BESTIARY_ROW_%d", scrollOff + i);
            if (isSelected) {
                value = "selected";
            } else {
                snprintf(s_bestiary_row_value[i], sizeof(s_bestiary_row_value[i]),
                         "%s | HP %d-%d | L%d",
                         e->name ? e->name : "",
                         e->hpMin, e->hpMax, e->dungeonLevel);
                value = s_bestiary_row_value[i];
            }
            add_element_rect(fbW, fbH, s_bestiary_row_id[i],
                             e->name ? e->name : "",
                             FS_AX_BESTIARY_ROW, base, 1, value);
        }
    }
}

/* Item Encyclopedia view: one category tab per known category,
 * then one row per visible item in the active category. */
static void emit_item_encyclopedia_view(const M12_StartupMenuState* state,
                                         int fbW, int fbH)
{
    int margin = 480 / 30;
    int heroH = 270 / 4;
    int contentY;
    int i;
    int categoryCount = (int)FS_ITEM_CAT_COUNT;
    int total;
    int catTabY = 42;
    int sel = state->itemEncyclopediaSelectedIndex;
    int cat = state->itemEncyclopediaCategory;
    int scrollOff = state->itemEncyclopediaScrollOffset;
    int visible = 12;
    int rendered = 0;

    /* Pre-format dynamic IDs / values into static scratch space
     * so the FS_AX_Element pointers remain valid until flush. */
    static char s_item_cat_id[8][32];
    static char s_item_cat_value[8][16];
    static char s_item_row_id[12][32];
    static char s_item_row_value[12][96];

    if (margin < 12) margin = 12;
    if (heroH < 64) heroH = 64;
    contentY = margin + heroH + 10;

    /* Category tabs. */
    for (i = 0; i < categoryCount; ++i) {
        AxRect base = { 122 + (i % 5) * 60, catTabY + (i / 5) * 22, 60, 18 };
        const char* name = fs_item_category_name((FS_ItemCategory)i);
        int isSelected = (state->itemEncyclopediaCategory == i);
        const char* selValue = NULL;
        if ((int)(sizeof(s_item_cat_id) / sizeof(s_item_cat_id[0])) <= i) {
            continue;
        }
        snprintf(s_item_cat_id[i], sizeof(s_item_cat_id[i]), "ITEM_CAT_%d", i);
        if (isSelected) {
            snprintf(s_item_cat_value[i], sizeof(s_item_cat_value[i]),
                     "%s", "selected");
            selValue = s_item_cat_value[i];
        }
        add_element_rect(fbW, fbH, s_item_cat_id[i], name ? name : "",
                         FS_AX_CATEGORY_TAB, base, 1, selValue);
    }

    /* Item rows. Walk the global item list (like the renderer does)
     * and emit only items in the active category. */
    total = fs_item_encyclopedia_count();
    for (i = 0; i < total && rendered < visible; ++i) {
        const FS_ItemEntry* e = fs_item_encyclopedia_get(i);
        if (!e) break;
        if ((int)e->category != cat) continue;
        if (i < scrollOff) continue;
        if (rendered >= (int)(sizeof(s_item_row_id) / sizeof(s_item_row_id[0]))) {
            break;
        }
        {
            AxRect base = { 130, contentY + 14 + (rendered * 22), 240, 22 };
            int isSelected = (i == sel);
            const char* value;
            snprintf(s_item_row_id[rendered], sizeof(s_item_row_id[rendered]),
                     "ITEM_ROW_%d", i);
            if (isSelected) {
                value = "selected";
            } else {
                if (e->attack > 0) {
                    snprintf(s_item_row_value[rendered],
                             sizeof(s_item_row_value[rendered]),
                             "%s | ATK %d | WT %d",
                             e->name ? e->name : "", e->attack, e->weight);
                } else if (e->defense > 0) {
                    snprintf(s_item_row_value[rendered],
                             sizeof(s_item_row_value[rendered]),
                             "%s | DEF %d | WT %d",
                             e->name ? e->name : "", e->defense, e->weight);
                } else {
                    snprintf(s_item_row_value[rendered],
                             sizeof(s_item_row_value[rendered]),
                             "%s | WT %d",
                             e->name ? e->name : "", e->weight);
                }
                value = s_item_row_value[rendered];
            }
            add_element_rect(fbW, fbH, s_item_row_id[rendered],
                             e->name ? e->name : "",
                             FS_AX_ITEM_ENCYCLOPEDIA_ROW, base, 1, value);
        }
        ++rendered;
    }
}

/* Screenshot Gallery view: one row per visible entry. The gallery's
 * cell data is the public M12_ScreenshotGalleryState.entries[] list. */
static void emit_screenshot_gallery_view(const M12_StartupMenuState* state,
                                           int fbW, int fbH)
{
    int margin = 480 / 30;
    int heroH = 270 / 4;
    int contentY;
    int i;
    const M12_ScreenshotGalleryState* gal = &state->screenshotGallery;
    int total = gal->entryCount;
    int sel = gal->selectedIndex;
    int scrollOff = gal->scrollOffset;
    int visible = 12;

    static char s_screenshot_id[12][32];
    static char s_screenshot_value[12][80];

    if (margin < 12) margin = 12;
    if (heroH < 64) heroH = 64;
    contentY = margin + heroH + 10;

    for (i = scrollOff; i < total && i < scrollOff + visible; ++i) {
        const M12_ScreenshotGalleryEntry* e = &gal->entries[i];
        AxRect base = { 130, contentY + 14 + ((i - scrollOff) * 22), 240, 22 };
        int slot = i - scrollOff;
        int isSelected = (i == sel);
        const char* value;
        if (slot < 0
            || slot >= (int)(sizeof(s_screenshot_id) / sizeof(s_screenshot_id[0]))) {
            break;
        }
        snprintf(s_screenshot_id[slot], sizeof(s_screenshot_id[slot]),
                 "SCREENSHOT_ROW_%d", i);
        if (isSelected) {
            value = "selected";
        } else {
            snprintf(s_screenshot_value[slot], sizeof(s_screenshot_value[slot]),
                     "%s | %dx%d | %ld B",
                     e->filename, e->width, e->height, e->fileSize);
            value = s_screenshot_value[slot];
        }
        add_element_rect(fbW, fbH, s_screenshot_id[slot], e->filename,
                         FS_AX_SCREENSHOT_THUMB, base, 1, value);
    }
}

/* Museum of Lore view: one MUSEUM_CATEGORY element per archive
 * section, then one MUSEUM_BULLET element per lore bullet on the
 * active page. The category titles are private to menu_startup_m12.c
 * but exposed via M12_Museum_GetCategoryTitle(). */
static void emit_museum_view(const M12_StartupMenuState* state,
                              int fbW, int fbH)
{
    int margin = 480 / 30;
    int heroH = 270 / 4;
    int contentY;
    int leftW = (480 * 32) / 100;
    int panelX = margin + leftW + 12;
    int i;
    int categoryCount = 5; /* M12_MUSEUM_CATEGORY_COUNT */
    int catIndex = state->museumSelectedIndex;
    if (catIndex < 0) catIndex = 0;
    if (catIndex >= categoryCount) catIndex = categoryCount - 1;

    static char s_museum_cat_id[5][32];
    static char s_museum_cat_value[5][16];
    static char s_museum_bullet_id[5][32];

    if (margin < 12) margin = 12;
    if (heroH < 64) heroH = 64;
    contentY = margin + heroH + 10;

    /* Category list (left panel). */
    for (i = 0; i < categoryCount; ++i) {
        const char* title = M12_Museum_GetCategoryTitle(i);
        AxRect base = { margin + 10, contentY + 30 + (i * 28), leftW - 20, 24 };
        int isSelected = (i == catIndex);
        const char* selValue = NULL;
        snprintf(s_museum_cat_id[i], sizeof(s_museum_cat_id[i]),
                 "MUSEUM_CATEGORY_%d", i);
        if (isSelected) {
            snprintf(s_museum_cat_value[i], sizeof(s_museum_cat_value[i]),
                     "%s", "selected");
            selValue = s_museum_cat_value[i];
        }
        add_element_rect(fbW, fbH, s_museum_cat_id[i], title ? title : "",
                         FS_AX_MUSEUM_CATEGORY, base, 1, selValue);
    }

    /* Active page bullets (right panel). */
    {
        int pageIndex = state->museumPageIndex;
        if (pageIndex < 0) pageIndex = 0;
        /* Page-count is read from the helper but we just clamp
         * to bullet index 0..4 here since the helper signature
         * already clamps for us. */
        for (i = 0; i < 5; ++i) {
            const char* bullet = M12_Museum_GetBullet(catIndex, pageIndex, i);
            if (!bullet || !bullet[0]) continue;
            {
                AxRect base = { panelX + 18, contentY + 58 + (i * 20), 240, 18 };
                snprintf(s_museum_bullet_id[i], sizeof(s_museum_bullet_id[i]),
                         "MUSEUM_BULLET_%d", i);
                add_element_rect(fbW, fbH, s_museum_bullet_id[i], bullet,
                                 FS_AX_MUSEUM_BULLET, base, 1, NULL);
            }
        }
    }
}

/* Manual / Docs view: expose the public docs entry table instead of
 * duplicating renderer-local labels. The repoPath values are project
 * relative and safe for manifests; no local absolute path is emitted. */
static void emit_manual_docs_view(int fbW, int fbH)
{
    int margin = 480 / 30;
    int heroH = 270 / 4;
    int contentY;
    int i;
    int count = M12_ManualDocs_EntryCount();
    static char s_manual_doc_id[12][32];
    static char s_manual_doc_value[12][160];

    if (margin < 12) margin = 12;
    if (heroH < 64) heroH = 64;
    contentY = margin + heroH + 10;
    if (count > (int)(sizeof(s_manual_doc_id) / sizeof(s_manual_doc_id[0]))) {
        count = (int)(sizeof(s_manual_doc_id) / sizeof(s_manual_doc_id[0]));
    }

    for (i = 0; i < count; ++i) {
        const M12_ManualDocsEntry* entry = M12_ManualDocs_GetEntry(i);
        AxRect base = { 34, contentY + 34 + (i * 22), 400, 22 };
        if (!entry) {
            continue;
        }
        snprintf(s_manual_doc_id[i], sizeof(s_manual_doc_id[i]),
                 "MANUAL_DOC_%d", i);
        snprintf(s_manual_doc_value[i], sizeof(s_manual_doc_value[i]),
                 "%s | %s",
                 entry->repoPath ? entry->repoPath : "",
                 entry->summary ? entry->summary : "");
        add_element_rect(fbW, fbH, s_manual_doc_id[i],
                         entry->title ? entry->title : "",
                         FS_AX_LAUNCHER_ROW, base, 1,
                         s_manual_doc_value[i]);
    }
}

/* Changelog view: mirror the visible window from
 * m12_draw_changelog_view_modern, including scrollOffset. */
static void emit_changelog_view(const M12_StartupMenuState* state,
                                int fbW, int fbH)
{
    int margin = 480 / 30;
    int heroH = 270 / 4;
    int contentY;
    int scrollOff = state ? state->changelog.scrollOffset : 0;
    int lineCount = M12_Changelog_LineCount();
    int visible = M12_CHANGELOG_VISIBLE_LINES;
    int i;
    static char s_changelog_id[M12_CHANGELOG_VISIBLE_LINES][32];

    if (margin < 12) margin = 12;
    if (heroH < 64) heroH = 64;
    contentY = margin + heroH + 10;
    if (scrollOff < 0) scrollOff = 0;
    if (scrollOff > lineCount) scrollOff = lineCount;
    if (visible > lineCount - scrollOff) {
        visible = lineCount - scrollOff;
    }
    if (visible < 0) visible = 0;

    for (i = 0; i < visible; ++i) {
        const char* line = M12_Changelog_GetLine(scrollOff + i);
        AxRect base = { margin + 14, contentY + 14 + (i * 20), 420, 20 };
        if (!line) {
            continue;
        }
        snprintf(s_changelog_id[i], sizeof(s_changelog_id[i]),
                 "CHANGELOG_LINE_%d", scrollOff + i);
        add_element_rect(fbW, fbH, s_changelog_id[i],
                         line,
                         FS_AX_LAUNCHER_ROW, base, 1, NULL);
    }
}

/* Message view (used for missing-data popup, data-dir result,
 * quick-resume missing, and the general "OK" confirmation). The popup
 * rect comes from m12_draw_message_view at x=118 y=74 w=188 h=88 in
 * the legacy 480x270 framebuffer; the OK button lives at x=180 y=146
 * w=64 h=16. We surface the three message lines so a screen reader
 * can announce them. */
static void emit_message_view(const M12_StartupMenuState* state,
                              int fbW, int fbH,
                              int includePaths)
{
    /* Suppress the absolute data-dir line by default to keep the
     * manifest safe to publish. */
    const char* line3 = includePaths
                        ? (state && state->messageLine3 ? state->messageLine3 : NULL)
                        : NULL;

    AxRect popup = { 118, 74, 188, 88 };
    AxRect line1 = { 124, 86,  176, 14 };
    AxRect line2 = { 124, 108, 176, 14 };
    AxRect line3Rect = { 124, 130, 176, 14 };
    AxRect okRect = { 180, 146, 64, 16 };

    add_element_rect(fbW, fbH,
                     "POPUP_MESSAGE",
                     "Status",
                     FS_AX_POPUP, popup, 1,
                     (state && state->messageLine1) ? state->messageLine1 : "");
    add_element_rect(fbW, fbH,
                     "POPUP_LINE1",
                     "Message Line 1",
                     FS_AX_TEXT, line1, 1,
                     (state && state->messageLine1) ? state->messageLine1 : "");
    add_element_rect(fbW, fbH,
                     "POPUP_LINE2",
                     "Message Line 2",
                     FS_AX_TEXT, line2, 1,
                     (state && state->messageLine2) ? state->messageLine2 : "");
    add_element_rect(fbW, fbH,
                     "POPUP_LINE3",
                     "Message Line 3",
                     FS_AX_TEXT, line3Rect, 1,
                     line3);
    add_element_rect(fbW, fbH,
                     "POPUP_OK",
                     "OK",
                     FS_AX_POPUP_OK, okRect, 1, NULL);
}

/* ── Public entry ─────────────────────────────────────────────────── */

void m12_launcher_a11y_emit(const M12_StartupMenuState* state,
                             int framebufferWidth,
                             int framebufferHeight,
                             int includePaths)
{
    /* The state must be valid and the writer enabled. The caller is
     * expected to call fs_ax_begin_frame(...) before invoking this
     * and fs_ax_flush() afterwards; we only emit elements. */
    if (!state || !fs_ax_is_enabled()) {
        return;
    }
    if (framebufferWidth <= 0) framebufferWidth = 480;
    if (framebufferHeight <= 0) framebufferHeight = 270;

    switch (state->view) {
    case M12_MENU_VIEW_MESSAGE:
        emit_message_view(state, framebufferWidth, framebufferHeight, includePaths);
        break;
    case M12_MENU_VIEW_SETTINGS:
        emit_settings_view(state, framebufferWidth, framebufferHeight, includePaths);
        break;
    case M12_MENU_VIEW_GAME_OPTIONS:
        emit_game_options_view(state, framebufferWidth, framebufferHeight);
        break;
    case M12_MENU_VIEW_BESTIARY:
        emit_bestiary_view(state, framebufferWidth, framebufferHeight);
        break;
    case M12_MENU_VIEW_ITEM_ENCYCLOPEDIA:
        emit_item_encyclopedia_view(state, framebufferWidth, framebufferHeight);
        break;
    case M12_MENU_VIEW_SCREENSHOT_GALLERY:
        emit_screenshot_gallery_view(state, framebufferWidth, framebufferHeight);
        break;
    case M12_MENU_VIEW_MUSEUM:
        emit_museum_view(state, framebufferWidth, framebufferHeight);
        break;
    case M12_MENU_VIEW_MANUAL_DOCS:
        emit_manual_docs_view(framebufferWidth, framebufferHeight);
        break;
    case M12_MENU_VIEW_CHANGELOG:
        emit_changelog_view(state, framebufferWidth, framebufferHeight);
        break;
    case M12_MENU_VIEW_MAIN:
    case M12_MENU_VIEW_DATA_VALIDATOR:
    case M12_MENU_VIEW_AUDIO_SETTINGS:
    case M12_MENU_VIEW_ACCESSIBILITY:
    case M12_MENU_VIEW_THEME:
    case M12_MENU_VIEW_SAVE_BROWSER:
    case M12_MENU_VIEW_INPUT_REMAP:
    case M12_MENU_VIEW_CUSTOM_DUNGEON:
    case M12_MENU_VIEW_CAMPAIGN:
    case M12_MENU_VIEW_SPELL_REFERENCE:
    case M12_MENU_VIEW_MAP_VIEWER:
    case M12_MENU_VIEW_TOUCH_LAYOUT:
    case M12_MENU_VIEW_PRESENTATION_PREVIEW:
    default:
        /* Default to main view emission; non-main views still get
         * the game-card list as a navigation anchor. */
        emit_main_view(state, framebufferWidth, framebufferHeight);
        break;
    }
}
