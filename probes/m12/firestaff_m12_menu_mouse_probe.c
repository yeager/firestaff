/*
 * firestaff_m12_menu_mouse_probe.c
 *
 * Bounded M12 slice probe for startup-menu mouse/hover/keyboard
 * interaction on the modern high-resolution renderer.
 *
 *   INV_MOUSE_01   hit-test on the modern main view maps the three
 *                  visible game cards to entry indices 0..2.
 *   INV_MOUSE_02   clicking the non-selected card moves selection and
 *                  activates it.
 *   INV_MOUSE_03   in settings view, clicking the right half of a row
 *                  cycles its value; cycling the language row
 *                  immediately changes the rendered output (no
 *                  restart required).
 *   INV_MOUSE_04   in settings view, clicking the back button returns
 *                  to the main view.
 *   INV_MOUSE_05   in game-options view, clicking the launch button
 *                  triggers the launch path (message view or
 *                  ready-to-launch / coming-soon).
 *   INV_MOUSE_06   hover coordinates are stored on the state when the
 *                  pointer moves without clicking.
 *   INV_MOUSE_07   keyboard UP/DOWN arrow navigation on the main view
 *                  cycles through all 5 cards deterministically.
 *   INV_MOUSE_08   immediate language switch: mutating
 *                  settings.languageIndex changes the very next
 *                  rendered frame (no reinit required).
 *   INV_MOUSE_09   frameTick animation tick affects the selected-card
 *                  rendered signature (visible pulse).
 *   INV_MOUSE_10   redesigned Extras rows can be selected and opened
 *                  through the same pointer hit path as keyboard input.
 */
#include "asset_status_m12.h"
#include "menu_startup_m12.h"
#include "menu_startup_render_modern_m12.h"
#include "menu_hit_m12.h"
#include "card_art_m12.h"
#include "creature_art_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>

static int probe_setenv(const char* name, const char* value) {
    return _putenv_s(name, value);
}

static void probe_mkdir(const char* path) {
    (void)_mkdir(path);
}
#else
static int probe_setenv(const char* name, const char* value) {
    return setenv(name, value, 1);
}

static void probe_mkdir(const char* path) {
    (void)mkdir(path, 0777);
}
#endif

typedef struct {
    int total;
    int passed;
} Tally;

static void record(Tally* t, const char* id, int ok, const char* msg) {
    t->total += 1;
    if (ok) {
        t->passed += 1;
        printf("PASS %s %s\n", id, msg);
    } else {
        printf("FAIL %s %s\n", id, msg);
    }
}

static unsigned long checksum(const unsigned char* buf, size_t n) {
    unsigned long h = 2166136261UL;
    for (size_t i = 0; i < n; i += 4) {
        h ^= buf[i + 0]; h *= 16777619UL;
        h ^= buf[i + 1]; h *= 16777619UL;
        h ^= buf[i + 2]; h *= 16777619UL;
    }
    return h;
}

static void force_dm1_ready(M12_StartupMenuState* state) {
    if (!state) return;
    state->assetStatus.dm1Available = 1;
    state->entries[0].available = 1;
    M12_AssetVersionStatus* v = &state->assetStatus.versions[0][0];
    v->matched = 1;
    snprintf(v->matchedPath, sizeof(v->matchedPath), "%s", "probe://forced-dm1");
    snprintf(v->matchedMd5, sizeof(v->matchedMd5), "%s", "forced");
    state->gameOptions[0].versionIndex = 0;
}

static void init_probe_menu_state(M12_StartupMenuState* state) {
    M12_StartupMenu_Init(state);
    if (state && state->view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_BACK);
    }
}

static void main_card_center(int entry, int* cx, int* cy) {
    const int railX = 42;
    const int railW = 390;
    const int gridLeft = railX + railW + 44;
    const int gridTop = 40;
    const int gridBottom = 1080 - 130;
    const int gap = 22;
    const int cols = 3;
    const int cardW = (1920 - gridLeft - 48 - gap * (cols - 1)) / cols;
    const int cardH = (gridBottom - gridTop - gap) / 2;
    int col = entry % cols;
    int row = entry / cols;
    if (cx) *cx = gridLeft + col * (cardW + gap) + cardW / 2;
    if (cy) *cy = gridTop + row * (cardH + gap) + cardH / 2;
}

int main(void) {
    Tally t = {0, 0};
    const int W = M12_ModernMenu_NativeWidth();
    const int H = M12_ModernMenu_NativeHeight();
    const size_t rgbaBytes = (size_t)W * (size_t)H * 4U;
    unsigned char* a = (unsigned char*)malloc(rgbaBytes);
    unsigned char* b = (unsigned char*)malloc(rgbaBytes);
    if (!a || !b) { free(a); free(b); return 2; }

    probe_mkdir("verification-m12");
    probe_mkdir("verification-m12/menu-mouse");
    probe_mkdir("verification-m12/menu-mouse/home");
    probe_mkdir("verification-m12/menu-mouse/empty-screenshots");
    probe_setenv("HOME", "verification-m12/menu-mouse/home");
    probe_setenv("FIRESTAFF_SCREENSHOTS_DIR",
                 "verification-m12/menu-mouse/empty-screenshots");

    /* ---------- INV_MOUSE_01 ---------- */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        int allOk = 1;
        for (int entry = 0; entry < 4; ++entry) {
            int cx, cy;
            main_card_center(entry, &cx, &cy);
            M12_MouseHit h_ = M12_ModernMenu_HitTest(&s, cx, cy);
            if (!(h_.kind == M12_HIT_MAIN_CARD && h_.index == entry)) {
                allOk = 0;
                printf("  DEBUG entry card %d at (%d,%d) -> kind=%d index=%d\n",
                       entry, cx, cy, h_.kind, h_.index);
            }
        }
        record(&t, "INV_MOUSE_01", allOk,
               "hit-test maps visible game card centres to entry indices 0..3");
    }

    /* ---------- INV_MOUSE_02 ---------- */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        force_dm1_ready(&s);
        /* Click entry 2 (DM2): visible in the catalog,
         * but not launch-supported. */
        int cx, cy;
        main_card_center(2, &cx, &cy);
        int changed = M12_ModernMenu_HandlePointer(&s, cx, cy, 1, NULL);
        int ok = changed == 1 &&
                 s.selectedIndex == 2 &&
                 s.view == M12_MENU_VIEW_MESSAGE &&
                 s.launchRequested == 0 &&
                 s.messageLine1 && s.messageLine1[0] != '\0';
        record(&t, "INV_MOUSE_02", ok,
               "clicking an unsupported card selects it and shows coming-soon without launch");

        /* Card 0 is DM1 and it's forced-ready, click should open game-opts. */
        init_probe_menu_state(&s);
        force_dm1_ready(&s);
        main_card_center(0, &cx, &cy);
        M12_ModernMenu_HandlePointer(&s, cx, cy, 1, NULL);
        record(&t, "INV_MOUSE_02B",
               s.selectedIndex == 0 && s.view == M12_MENU_VIEW_GAME_OPTIONS,
               "clicking DM1 card opens game options when data is ready");
    }

    /* ---------- INV_MOUSE_03 ---------- */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        s.view = M12_MENU_VIEW_SETTINGS;
        s.settingsSelectedIndex = 0;
        s.settings.languageIndex = 0;

        memset(a, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, a, W, H);
        unsigned long sigEN = checksum(a, rgbaBytes);

        /* Click the right half of the LANGUAGE row (row 0) so the
         * value cycles 0 -> 1. */
        int rowX = 96 + 36;
        int rowY = 260 + 36; /* panelY + 36 */
        int rowW = (W - 2 * 96) - 72;
        int clickX = rowX + (rowW * 85) / 100;
        int clickY = rowY + 20;
        int changed = M12_ModernMenu_HandlePointer(&s, clickX, clickY, 1, NULL);

        memset(b, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, b, W, H);
        unsigned long sigSV = checksum(b, rgbaBytes);

        int ok = changed == 1 &&
                 s.settings.languageIndex == 1 &&
                 sigEN != sigSV &&
                 s.languageExplicit == 1;
        record(&t, "INV_MOUSE_03", ok,
               "clicking the settings language row cycles immediately and "
               "changes the rendered output without restart");
    }

    /* ---------- INV_MOUSE_04 ---------- */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        s.view = M12_MENU_VIEW_SETTINGS;
        /* Back button at (24,120)-(134,164) */
        M12_ModernMenu_HandlePointer(&s, 60, 140, 1, NULL);
        record(&t, "INV_MOUSE_04",
               s.view == M12_MENU_VIEW_MAIN,
               "clicking the back button in settings returns to main view");
    }

    /* ---------- INV_MOUSE_05 ---------- */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        force_dm1_ready(&s);
        s.selectedIndex = 0;
        s.view = M12_MENU_VIEW_GAME_OPTIONS;
        s.activatedIndex = 0;
        s.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;

        /* Launch button is rendered centered at panel bottom. */
        int panelX = 96;
        int panelY = 190;
        int panelW = W - 2 * panelX;
        int panelH = 780;
        int w = 240, h = 54;
        int lx = panelX + (panelW - w) / 2;
        int ly = panelY + panelH - h - 24;

        int changed = M12_ModernMenu_HandlePointer(&s, lx + w / 2, ly + h / 2, 1, NULL);
        record(&t, "INV_MOUSE_05",
               changed == 1 &&
               s.view == M12_MENU_VIEW_MESSAGE &&
               s.gameOptSelectedRow >= M12_GAME_OPT_ROW_COUNT,
               "clicking the launch button jumps cursor to launch row and "
               "triggers the launch path (message view)");
    }

    /* ---------- INV_MOUSE_06 ---------- */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        M12_ModernMenu_HandlePointer(&s, 640, 360, 0, NULL);
        record(&t, "INV_MOUSE_06",
               s.hoverX == 640 && s.hoverY == 360 && s.view == M12_MENU_VIEW_MAIN,
               "pointer motion without a click updates hoverX/hoverY and "
               "leaves the view untouched");
    }

    /* ---------- INV_MOUSE_07 ---------- */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        int ok = s.selectedIndex == 0;
        int visits[5] = {0, 0, 0, 0, 0};
        for (int i = 0; i < 10; ++i) {
            visits[s.selectedIndex] = 1;
            M12_StartupMenu_HandleInput(&s, M12_MENU_INPUT_DOWN);
        }
        for (int i = 0; i < 5; ++i) ok = ok && visits[i];
        /* Now go back up through all */
        for (int i = 0; i < 5; ++i) visits[i] = 0;
        for (int i = 0; i < 10; ++i) {
            visits[s.selectedIndex] = 1;
            M12_StartupMenu_HandleInput(&s, M12_MENU_INPUT_UP);
        }
        for (int i = 0; i < 5; ++i) ok = ok && visits[i];
        record(&t, "INV_MOUSE_07", ok,
               "keyboard UP/DOWN cycles through all 5 cards deterministically");
    }

    /* ---------- INV_MOUSE_08 ---------- */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        s.settings.graphicsIndex = M12_PRESENTATION_V21_UPSCALED;
        s.settings.languageIndex = 0;
        memset(a, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, a, W, H);
        unsigned long sigEN = checksum(a, rgbaBytes);

        /* Mutate in-place, re-render same state object. No reinit. */
        s.settings.languageIndex = 1;
        memset(b, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, b, W, H);
        unsigned long sigSV = checksum(b, rgbaBytes);

        s.settings.languageIndex = 2;
        unsigned char* cbuf = (unsigned char*)malloc(rgbaBytes);
        memset(cbuf, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, cbuf, W, H);
        unsigned long sigFR = checksum(cbuf, rgbaBytes);
        free(cbuf);

        record(&t, "INV_MOUSE_08",
               sigEN != sigSV && sigSV != sigFR && sigEN != sigFR,
               "same-state language index change alters the very next "
               "rendered frame (no restart required)");
    }

    /* ---------- INV_MOUSE_09 ---------- */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        s.settings.graphicsIndex = M12_PRESENTATION_V21_UPSCALED;
        s.selectedIndex = 1;
        s.frameTick = 0;
        memset(a, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, a, W, H);
        unsigned long sig0 = checksum(a, rgbaBytes);
        s.frameTick = 15; /* peak of the 60-frame triangle wave */
        memset(b, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, b, W, H);
        unsigned long sig1 = checksum(b, rgbaBytes);
        record(&t, "INV_MOUSE_09",
               sig0 != sig1,
               "frameTick drives a visible pulse on the selected card");
    }

    /* ---------- INV_MOUSE_10 ---------- */
    {
        M12_StartupMenuState s;
        M12_MouseHit h_;
        init_probe_menu_state(&s);
        s.mainMenuSelected = M12_MAIN_MENU_EXTRAS;
        m12_redesigned_handle_input(&s, 0, 0, 0, 0, 1, 0);

        int rowX = W / 20 + 40;
        int rowY0 = H / 5;
        h_ = M12_ModernMenu_HitTest(&s,
                                    rowX,
                                    rowY0 + M12_EXTRAS_ITEMS * 26 + 13);
        record(&t, "INV_MOUSE_10A",
               m12_get_nav_level() == (int)M12_NAV_EXTRAS &&
               h_.kind == M12_HIT_EXTRAS_ROW &&
               h_.index == M12_EXTRAS_ITEMS,
               "hit-test maps redesigned Extras row centres to extras indices");

        M12_ModernMenu_HandlePointer(&s,
                                     rowX,
                                     rowY0 + M12_EXTRAS_ITEMS * 26 + 13,
                                     0,
                                     NULL);
        record(&t, "INV_MOUSE_10B",
               s.extrasSelected == M12_EXTRAS_ITEMS &&
               s.view == M12_MENU_VIEW_MAIN,
               "hovering an Extras row updates the same selection as keyboard navigation");

        M12_ModernMenu_HandlePointer(&s,
                                     rowX,
                                     rowY0 + M12_EXTRAS_SPELLS * 26 + 13,
                                     1,
                                     NULL);
        record(&t, "INV_MOUSE_10C",
               s.extrasSelected == M12_EXTRAS_SPELLS &&
               s.view == M12_MENU_VIEW_MESSAGE &&
               s.messageLine2 &&
               strstr(s.messageLine2, "NO DATA SOURCE") != NULL,
               "clicking a disabled Extras row opens the explanatory popup");

        M12_StartupMenu_HandleInput(&s, M12_MENU_INPUT_BACK);
        s.mainMenuSelected = M12_MAIN_MENU_EXTRAS;
        m12_redesigned_handle_input(&s, 0, 0, 0, 0, 1, 0);
        M12_ModernMenu_HandlePointer(&s,
                                     rowX,
                                     rowY0 + M12_EXTRAS_MANUAL * 26 + 13,
                                     1,
                                     NULL);
        record(&t, "INV_MOUSE_10D",
               s.extrasSelected == M12_EXTRAS_MANUAL &&
               s.view == M12_MENU_VIEW_MANUAL_DOCS,
               "clicking an available Extras row opens its real view");
    }

    /* Write a screenshot of the modern main view with hover for audit. */
    {
        M12_StartupMenuState s;
        init_probe_menu_state(&s);
        force_dm1_ready(&s);
        s.selectedIndex = 1;
        s.hoverX = 700;
        s.hoverY = 400;
        s.frameTick = 8;
        memset(a, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, a, W, H);
        FILE* fp = fopen("verification-m12/menu-mouse/main_with_hover.ppm", "wb");
        if (fp) {
            fprintf(fp, "P6\n%d %d\n255\n", W, H);
            for (int i = 0; i < W * H; ++i) {
                unsigned char px[3] = {a[i * 4], a[i * 4 + 1], a[i * 4 + 2]};
                fwrite(px, 1, 3, fp);
            }
            fclose(fp);
        }
        s.view = M12_MENU_VIEW_GAME_OPTIONS;
        s.activatedIndex = 0;
        s.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
        memset(a, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, a, W, H);
        fp = fopen("verification-m12/menu-mouse/gameopts_launch.ppm", "wb");
        if (fp) {
            fprintf(fp, "P6\n%d %d\n255\n", W, H);
            for (int i = 0; i < W * H; ++i) {
                unsigned char px[3] = {a[i * 4], a[i * 4 + 1], a[i * 4 + 2]};
                fwrite(px, 1, 3, fp);
            }
            fclose(fp);
        }
        s.view = M12_MENU_VIEW_SETTINGS;
        s.settings.languageIndex = 1;
        memset(a, 0, rgbaBytes);
        M12_ModernMenu_Render(&s, a, W, H);
        fp = fopen("verification-m12/menu-mouse/settings_sv.ppm", "wb");
        if (fp) {
            fprintf(fp, "P6\n%d %d\n255\n", W, H);
            for (int i = 0; i < W * H; ++i) {
                unsigned char px[3] = {a[i * 4], a[i * 4 + 1], a[i * 4 + 2]};
                fwrite(px, 1, 3, fp);
            }
            fclose(fp);
        }
    }

    free(a);
    free(b);
    printf("# summary: %d/%d invariants passed\n", t.passed, t.total);
    return (t.total > 0 && t.passed == t.total) ? 0 : 1;
}
