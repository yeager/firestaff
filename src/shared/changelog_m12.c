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
    return "1.7.6";
}
