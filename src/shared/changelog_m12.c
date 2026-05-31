#include "changelog_m12.h"
#include <stddef.h>

/* ── Embedded changelog ───────────────────────────────────────────
 * Each entry is a single string displayed as one line in the viewer.
 * Empty strings produce blank separator lines.
 */
static const char* const g_changelogLines[] = {
    "FIRESTAFF CHANGELOG",
    "====================",
    "",
    "V2.7.0  (2026-05-31)",
    "  - CSB V2: Phase 0-6 complete — V1 compat lock, launch/profile separation, enhanced asset pipeline, stairs animation, touch controller affordances",
    "  - DM2 V2: Phase 1-6 complete — launch/profile gates, smooth movement runtime, enhanced lighting, outdoor FX, torch flicker, fog animation, HUD overlay hardening",
    "  - Nexus V2: Phase 1-6 complete — touch/controller affordance ergonomics, atmosphere, lighting, particles, upscaler fixes",
    "  - Theron V1: Phase 1-4 — rendering pipeline, tile renderer, asset loader, UI chrome, creature instance lifecycle",
    "  - DM1 V1: Phase 8 complete — door/special-square interaction, wall rendering integrity, blurry inscription probes, champion portrait Z-order fix",
    "  - DM1 V2: Phase 8 complete — door-frame type override, message log pixel font atlas, champion panel renderer, modern asset pipeline",
    "  - Accessibility: high-contrast game view toggle, configurable in-game font scaling (M11 fontScale from M12)",
    "  - Probes: nexus_v1_mechanics_parity (Phase 7), CSB V1 Phase 2 DSA script section, DM1 V1 parity-evidence manifests, source-lock evidence docs",
    "  - M12: JSON settings export/import feature",
    "",
    "V0.11.0  (2026-05-04)",
    "  - ADD CHANGELOG/VERSION VIEWER IN LAUNCHER",
    "  - MUSEUM OF LORE ARCHIVE SECTIONS",
    "  - CREATURE ART GALLERY WITH PALETTE LEVELS",
    "  - AUDIO SETTINGS VIEW",
    "",
    "V0.10.0  (2026-04-15)",
    "  - GAME OPTIONS PER-TITLE (VERSION/PATCH/SPEED)",
    "  - MODERN RENDERER LAYOUT WITH HERO BANNER",
    "  - CARD ART DISPLAY FOR GAME ENTRIES",
    "  - BRANDING LOGO RENDERING",
    "",
    "V0.9.0  (2026-03-20)",
    "  - ASSET STATUS AND HASH VERIFICATION",
    "  - MULTI-VERSION SUPPORT (DM1/CSB/DM2)",
    "  - CONFIGURABLE PRESENTATION MODES",
    "  - SDL3 RENDERER BACKEND SELECTION",
    "",
    "V0.8.0  (2026-02-28)",
    "  - INITIAL M12 LAUNCHER MENU SYSTEM",
    "  - SETTINGS VIEW (LANGUAGE/GRAPHICS/WINDOW)",
    "  - KEYBOARD AND MOUSE INPUT HANDLING",
    "  - SPARSE AND MODERN DRAW PATHS",
};

#define G_CHANGELOG_LINE_COUNT \
    ((int)(sizeof(g_changelogLines) / sizeof(g_changelogLines[0])))

void M12_Changelog_Init(M12_ChangelogState* cl) {
    if (!cl) {
        return;
    }
    cl->scrollOffset = 0;
    cl->totalLines = G_CHANGELOG_LINE_COUNT;
}

void M12_Changelog_Scroll(M12_ChangelogState* cl, int delta) {
    int maxOffset;
    if (!cl) {
        return;
    }
    cl->scrollOffset += delta;
    if (cl->scrollOffset < 0) {
        cl->scrollOffset = 0;
    }
    maxOffset = cl->totalLines - M12_CHANGELOG_VISIBLE_LINES;
    if (maxOffset < 0) {
        maxOffset = 0;
    }
    if (cl->scrollOffset > maxOffset) {
        cl->scrollOffset = maxOffset;
    }
}

int M12_Changelog_LineCount(void) {
    return G_CHANGELOG_LINE_COUNT;
}

const char* M12_Changelog_GetLine(int index) {
    if (index < 0 || index >= G_CHANGELOG_LINE_COUNT) {
        return NULL;
    }
    return g_changelogLines[index];
}

const char* M12_Changelog_VersionString(void) {
    return "2.6.0";
}
