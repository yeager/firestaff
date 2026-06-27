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
 *   - Settings tabs                      → FS_AX_LAUNCHER_TAB per tab
 *   - Settings rows                      → FS_AX_LAUNCHER_ROW per row
 *   - Missing-data popup                 → FS_AX_POPUP + FS_AX_POPUP_OK
 *   - General message view               → FS_AX_POPUP + FS_AX_POPUP_OK
 *   - Bestiary view                      → FS_AX_CATEGORY_TAB per category
 *                                           + FS_AX_BESTIARY_ROW per visible creature
 *   - Item Encyclopedia view             → FS_AX_CATEGORY_TAB per category
 *                                           + FS_AX_ITEM_ENCYCLOPEDIA_ROW per visible item
 *   - Screenshot Gallery view            → FS_AX_SCREENSHOT_THUMB per visible entry
 *   - Museum of Lore view                → FS_AX_MUSEUM_CATEGORY per section
 *                                           + FS_AX_MUSEUM_BULLET per lore bullet on the active page
 *
 * Out of scope (kept for follow-up passes):
 *   - Quick-resume "CONTINUE" virtual entry on the main view
 *     (rendered inline in m12_draw_main_view, not in state->entries).
 *   - Manual / docs / changelog / data-validator / theme / save-browser
 *     / input-remap / custom-dungeon / campaign / spell-reference /
 *     map-viewer / touch-layout / presentation-preview views.
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
#include "firestaff_item_encyclopedia.h"
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

/* ── Per-view emitters ────────────────────────────────────────────── */

/* Main view: emit one card element per state->entries[i]. */
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

        /* selectedIndex marks the focused card; surface it via the
         * element's value so the screen reader can announce "selected". */
        if (state->selectedIndex == i) {
            add_element_rect(fbW, fbH, id, label,
                             FS_AX_LAUNCHER_CARD, base, enabled,
                             "selected");
        } else {
            add_element_rect(fbW, fbH, id, label,
                             FS_AX_LAUNCHER_CARD, base, enabled, NULL);
        }
    }
}

/* Settings view: tabs first, then rows. Row labels mirror the strings
 * used by m12_draw_settings_view / m12_draw_settings_view_modern. */
static void emit_settings_view(const M12_StartupMenuState* state,
                               int fbW, int fbH)
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

    /* Settings rows. For simplicity emit only a generic row locator;
     * the precise row list lives in m12_accessibility_settings[] in
     * menu_startup_m12.c and we mirror the most-frequent settings. */
    {
        static const struct {
            const char* id;
            const char* label;
        } rows[] = {
            { "ROW_FONT_SCALE",       "Font Scale" },
            { "ROW_HIGH_CONTRAST",    "High Contrast" },
            { "ROW_COLORBLIND_MODE",  "Colorblind Mode" },
            { "ROW_SCREEN_READER",    "Screen Reader" },
            { "ROW_LARGE_CURSOR",     "Large Cursor" },
            { "ROW_REDUCED_MOTION",   "Reduced Motion" },
            { "ROW_BUTTON_HOLD_TIME", "Button Hold Time" },
            { "ROW_AUDIO_CUES",       "Audio Cues" },
            { NULL, NULL }
        };
        int rowY = 76;
        for (i = 0; rows[i].id != NULL; ++i) {
            AxRect base = { 122, rowY, 180, 24 };
            add_element_rect(fbW, fbH, rows[i].id, rows[i].label,
                             FS_AX_LAUNCHER_ROW, base, 1, NULL);
            rowY += 24;
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
        emit_settings_view(state, framebufferWidth, framebufferHeight);
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
    case M12_MENU_VIEW_MAIN:
    case M12_MENU_VIEW_MANUAL_DOCS:
    case M12_MENU_VIEW_CHANGELOG:
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
