/*
 * firestaff_m12_language_cycle_layout_probe.c
 *
 * Focused M12 localization/layout gate: render the modern message popup
 * using the 19-language startup-menu cycle's validator-message strings and
 * prove the translated text remains inside the popup panel bounds.
 */

#include "menu_startup_m12.h"
#include "menu_startup_render_modern_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int total;
    int passed;
} Tally;

typedef struct {
    const char* lang;
    const char* line1;
    const char* line2;
    const char* line3;
} LayoutCase;

typedef struct {
    int found;
    int minX;
    int maxX;
} Bounds;

enum {
    PANEL_W = 840,
    PANEL_H = 320
};

/* Layout fixtures for the M12 launcher 19-language UI cycle.
 *
 * The cycle count and locale codes are now derived from the
 * production source of truth via M12_StartupMenu_GetLanguageCount()
 * + M12_StartupMenu_GetLanguageCode() at runtime, so this fixture
 * table does not hardcode 19 (or any specific locale code).
 *
 * Per-language strings are sourced from the real
 * po/startup-menu.<lang>.po catalogs today: only EN, SV, FR, and
 * DE have non-English msgstr; the remaining 15 locales fall back
 * to the English msgid verbatim, which is exactly what the
 * launcher surfaces for those indices in production.
 *
 * MAX_FIXTURES is a compile-time upper bound sized to the current
 * 19-language cycle.  If a future cycle grows past this bound the
 * fixture initializer must grow with it; the runtime loop only
 * walks up to M12_StartupMenu_GetLanguageCount() so the cycle
 * itself stays source-of-truth-driven. */
#define MAX_FIXTURES 19

static const LayoutCase g_cases[MAX_FIXTURES] = {
    /* EN baseline (matches po/startup-menu.en.po msgid) */
    {"en", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    /* SV real translation (po/startup-menu.sv.po) */
    {"sv", "ENDAST VALIDATOR-MALL", "LÄGG TILL VERIFIERADE DETALJHANDEL-HASHAR", "ESC ÅTERGÅR TILL MENYN"},
    /* FR real translation (po/startup-menu.fr.po) */
    {"fr", "ÉCHAFAUDAGE VALIDATEUR UNIQUEMENT", "AJOUTER HASHES COMMERCE VÉRIFIÉS", "ÉCHAP RETOURNE AU MENU"},
    /* DE real translation (po/startup-menu.de.po) */
    {"de", "NUR VALIDATOR-GERÜST", "VERIFIZIERTE EINZELHANDELS-HASHES HINZUFÜGEN", "ESC KEHRT ZUM MENÜ ZURÜCK"},
    /* JA/ZH and all other locales: today they fall back to the
     * English msgid (no .po override) and the launcher surfaces the
     * English string verbatim.  The fixture therefore uses the
     * English baseline; the test still pins the panel layout for
     * every language index in the cycle. */
    {"ja", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"zh", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"cs", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"da", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"es", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"fi", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"hu", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"it", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"ko", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"nl", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"no", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"pl", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"pt", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"ru", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"},
    {"tr", "VALIDATOR SCAFFOLD ONLY", "ADD VERIFIED RETAIL HASHES", "ESC RETURNS TO MENU"}
};

static const int g_fixtureCount =
    (int)(sizeof(g_cases) / sizeof(g_cases[0]));

static void record(Tally* t, const char* id, int ok, const char* msg) {
    t->total += 1;
    if (ok) {
        t->passed += 1;
        printf("PASS %s %s\n", id, msg);
    } else {
        printf("FAIL %s %s\n", id, msg);
    }
}

static int abs_i(int v) {
    return v < 0 ? -v : v;
}

static int pixel_changed(const unsigned char* a, const unsigned char* b) {
    return abs_i((int)a[0] - (int)b[0]) +
           abs_i((int)a[1] - (int)b[1]) +
           abs_i((int)a[2] - (int)b[2]) > 32;
}

static Bounds changed_bounds(const unsigned char* rendered,
                             const unsigned char* baseline,
                             int w,
                             int h,
                             int y,
                             int bandH) {
    Bounds b;
    int yy;
    int xx;
    b.found = 0;
    b.minX = w;
    b.maxX = -1;
    for (yy = y; yy < y + bandH && yy < h; ++yy) {
        if (yy < 0) {
            continue;
        }
        for (xx = 0; xx < w; ++xx) {
            const unsigned char* rp = rendered + (((size_t)yy * (size_t)w + (size_t)xx) * 4U);
            const unsigned char* bp = baseline + (((size_t)yy * (size_t)w + (size_t)xx) * 4U);
            if (!pixel_changed(rp, bp)) {
                continue;
            }
            if (!b.found || xx < b.minX) {
                b.minX = xx;
            }
            if (!b.found || xx > b.maxX) {
                b.maxX = xx;
            }
            b.found = 1;
        }
    }
    return b;
}

static int bounds_fit_panel(Bounds b, int panelX) {
    return b.found && b.minX >= panelX && b.maxX < panelX + PANEL_W;
}

static void render_message_case(const LayoutCase* lc,
                                int languageIndex,
                                unsigned char* out,
                                int w,
                                int h) {
    M12_StartupMenuState state;
    memset(&state, 0, sizeof(state));
    state.view = M12_MENU_VIEW_MESSAGE;
    state.settings.languageIndex = languageIndex;
    state.messageLine1 = lc ? lc->line1 : "";
    state.messageLine2 = lc ? lc->line2 : "";
    state.messageLine3 = lc ? lc->line3 : "";
    M12_ModernMenu_Render(&state, out, w, h);
}

int main(void) {
    const int w = M12_ModernMenu_NativeWidth();
    const int h = M12_ModernMenu_NativeHeight();
    const int panelX = (w - PANEL_W) / 2;
    const int panelY = (h - PANEL_H) / 2;
    const size_t bytes = (size_t)w * (size_t)h * 4U;
    unsigned char* baseline = (unsigned char*)malloc(bytes);
    unsigned char* rendered = (unsigned char*)malloc(bytes);
    Tally tally = {0, 0};
    int i;

    if (!baseline || !rendered) {
        fprintf(stderr, "allocation failed\n");
        free(baseline);
        free(rendered);
        return 2;
    }

    record(&tally, "M12_LANG_LAYOUT_01",
           w == 1920 && h == 1080 && panelX == 540 && panelY == 380,
           "modern message-popup layout constants match the native canvas");
    record(&tally, "M12_LANG_LAYOUT_02",
           g_fixtureCount == M12_StartupMenu_GetLanguageCount(),
           "validator popup table covers the M12 launcher language cycle (source-of-truth)");
    record(&tally, "M12_LANG_LAYOUT_02b",
           g_fixtureCount == MAX_FIXTURES,
           "validator popup table matches its compile-time MAX_FIXTURES bound");

    for (i = 0; i < M12_StartupMenu_GetLanguageCount(); ++i) {
        char msg[192];
        Bounds b1;
        Bounds b2;
        Bounds b3;
        const LayoutCase* lc = &g_cases[i];
        const char* langLabel = M12_StartupMenu_GetLanguageCode(i);
        render_message_case(NULL, i, baseline, w, h);
        render_message_case(lc, i, rendered, w, h);

        b1 = changed_bounds(rendered, baseline, w, h, panelY + 46, 46);
        b2 = changed_bounds(rendered, baseline, w, h, panelY + 138, 34);
        b3 = changed_bounds(rendered, baseline, w, h, panelY + 218, 34);

        /* Prefer the production locale code (uppercase: "EN", "SV",
         * ...) when available so the PASS/FAIL line stays in sync
         * with whatever the launcher actually surfaces.  Fall back
         * to the fixture's lang field if the getter returned NULL
         * (which only happens for out-of-range indices, guarded
         * against by the M12_LANG_LAYOUT_02 source-of-truth check
         * above). */
        snprintf(msg, sizeof(msg),
                 "%s validator title stays within x=%d..%d (got %d..%d)",
                 langLabel ? langLabel : lc->lang,
                 panelX, panelX + PANEL_W - 1, b1.minX, b1.maxX);
        record(&tally, "M12_LANG_LAYOUT_03", bounds_fit_panel(b1, panelX), msg);

        snprintf(msg, sizeof(msg),
                 "%s long validator detail stays within x=%d..%d (got %d..%d)",
                 langLabel ? langLabel : lc->lang,
                 panelX, panelX + PANEL_W - 1, b2.minX, b2.maxX);
        record(&tally, "M12_LANG_LAYOUT_04", bounds_fit_panel(b2, panelX), msg);

        snprintf(msg, sizeof(msg),
                 "%s message footer stays within x=%d..%d (got %d..%d)",
                 langLabel ? langLabel : lc->lang,
                 panelX, panelX + PANEL_W - 1, b3.minX, b3.maxX);
        record(&tally, "M12_LANG_LAYOUT_05", bounds_fit_panel(b3, panelX), msg);
    }

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    free(baseline);
    free(rendered);
    return (tally.total > 0 && tally.passed == tally.total) ? 0 : 1;
}
