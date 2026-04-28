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

int main(void) {
    Tally t = {0, 0};
    const int W = M12_ModernMenu_NativeWidth();
    const int H = M12_ModernMenu_NativeHeight();
    const size_t rgbaBytes = (size_t)W * (size_t)H * 4U;
    unsigned char* a = (unsigned char*)malloc(rgbaBytes);
    unsigned char* b = (unsigned char*)malloc(rgbaBytes);
    if (!a || !b) { free(a); free(b); return 2; }

    mkdir("verification-m12", 0777);
    mkdir("verification-m12/menu-mouse", 0777);

    /* ---------- INV_MOUSE_01 ---------- */
    {
        M12_StartupMenuState s;
        M12_StartupMenu_Init(&s);
        /* Modern front-door layout: brand card at visual slot 0,
         * then DM1/CSB/DM2 game cards at visual slots 1..3. */
        int side = 48;
        int gap = 22;
        int visualCount = 4;
        int cardW = (W - 2 * side - gap * (visualCount - 1)) / visualCount;
        int allOk = 1;
        for (int entry = 0; entry < 3; ++entry) {
            int visualSlot = entry + 1;
            int cx = side + visualSlot * (cardW + gap) + cardW / 2;
            int cy = 400;
            M12_MouseHit h_ = M12_ModernMenu_HitTest(&s, cx, cy);
            if (!(h_.kind == M12_HIT_MAIN_CARD && h_.index == entry)) {
                allOk = 0;
                printf("  DEBUG visual card %d at (%d,%d) -> kind=%d index=%d\n",
                       visualSlot, cx, cy, h_.kind, h_.index);
            }
        }
        record(&t, "INV_MOUSE_01", allOk,
               "hit-test maps visible game card centres to entry indices 0..2");
    }

    /* ---------- INV_MOUSE_02 ---------- */
    {
        M12_StartupMenuState s;
        M12_StartupMenu_Init(&s);
        force_dm1_ready(&s);
        int side = 48;
        int gap = 22;
        int visualCount = 4;
        int cardW = (W - 2 * side - gap * (visualCount - 1)) / visualCount;
        /* Click visual card 3 / entry 2 (DM2): visible in the catalog,
         * but not launch-supported. */
        int cx = side + 3 * (cardW + gap) + cardW / 2;
        int cy = 400;
        int changed = M12_ModernMenu_HandlePointer(&s, cx, cy, 1, NULL);
        int ok = changed == 1 &&
                 s.selectedIndex == 2 &&
                 s.view == M12_MENU_VIEW_MESSAGE &&
                 s.launchRequested == 0 &&
                 s.messageLine1 && s.messageLine1[0] != '\0';
        record(&t, "INV_MOUSE_02", ok,
               "clicking an unsupported card selects it and shows coming-soon without launch");

        /* Card 0 is DM1 and it's forced-ready, click should open game-opts. */
        M12_StartupMenu_Init(&s);
        force_dm1_ready(&s);
        cx = side + 1 * (cardW + gap) + cardW / 2;
        cy = 400;
        M12_ModernMenu_HandlePointer(&s, cx, cy, 1, NULL);
        record(&t, "INV_MOUSE_02B",
               s.selectedIndex == 0 && s.view == M12_MENU_VIEW_GAME_OPTIONS,
               "clicking DM1 card opens game options when data is ready");
    }

    /* ---------- INV_MOUSE_03 ---------- */
    {
        M12_StartupMenuState s;
        M12_StartupMenu_Init(&s);
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
        M12_StartupMenu_Init(&s);
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
        M12_StartupMenu_Init(&s);
        force_dm1_ready(&s);
        s.selectedIndex = 0;
        s.view = M12_MENU_VIEW_GAME_OPTIONS;
        s.activatedIndex = 0;
        s.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;

        /* Launch button is at (panelX+panelW-w-36, panelY+panelH-h-20) */
        int panelX = 96;
        int panelY = 260;
        int panelW = W - 2 * panelX;
        int panelH = 400;
        int w = 240, h = 54;
        int lx = panelX + panelW - w - 36;
        int ly = panelY + panelH - h - 20;

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
        M12_StartupMenu_Init(&s);
        M12_ModernMenu_HandlePointer(&s, 640, 360, 0, NULL);
        record(&t, "INV_MOUSE_06",
               s.hoverX == 640 && s.hoverY == 360 && s.view == M12_MENU_VIEW_MAIN,
               "pointer motion without a click updates hoverX/hoverY and "
               "leaves the view untouched");
    }

    /* ---------- INV_MOUSE_07 ---------- */
    {
        M12_StartupMenuState s;
        M12_StartupMenu_Init(&s);
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
        M12_StartupMenu_Init(&s);
        s.settings.graphicsIndex = M12_PRESENTATION_V2_ENHANCED_2D;
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
        M12_StartupMenu_Init(&s);
        s.settings.graphicsIndex = M12_PRESENTATION_V2_ENHANCED_2D;
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

    /* Write a screenshot of the modern main view with hover for audit. */
    {
        M12_StartupMenuState s;
        M12_StartupMenu_Init(&s);
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
