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
    "V2.7.13  (2026-06-13)",
    "  - DM1 V1 combat fidelity audit: full systematic review of the DM1 V1 runtime against the ReDMCSB decompilation, documented in docs/DM1_V1_BUG_AUDIT.md",
    "  - Armor defense overhaul: replaced skill-level approximation with the F0321 wound defense calculation that iterates worn armor slots and scales attack by (130 - avgDefense) / 64",
    "  - Fire/Spell Shield defense: Fire Shield and Spell Shield spells now reduce incoming damage per CHAMPION.C F0321:1842-1857",
    "  - Creature poison: melee poison application now respects creature profile poison value, 50% chance per hit, vitality-adjusted via F0307",
    "  - Luck and stamina adjustments: F0308-style luck bias and F0306 stamina-adjusted value compiler order hazard fixed",
    "  - Psychic damage: C6_PSYCHIC damage type now applies from the spell descriptor",
    "  - Thieves Eye, light table, stat gain, magic map, dynamics table: source-locked to the exact ReDMCSB tables",
    "  - Projectile sub-cell hit mask: narrows from 0xFF to the actually-targeted sub-cell",
    "  - Creature AI: 7 creature types promoted from STUB to FULL tier (Giant Scorpion, Giggler, Screamer, Vexirk, Magenta Worm, Animated Armour, Red Dragon)",
    "  - Savegame field mask: bit layout now matches LOADSAVE.C",
    "  - Test infrastructure: FIRESTAFF_BUILD_DIR env var for out-of-tree builds in Python verification scripts",
    "  - Viewport crop readiness gate (pass434) wired to pass610 wall-collision runtime capture",
    "",
    "V2.7.12  (2026-06-13)",
    "  - DM1 V1 original capture: added a DOSBox in-dungeon movement route using VGA mode and source-locked keyboard-simulation movement",
    "  - DM1 V1 viewport: expanded source-lock coverage across additional F0108, F0111, and F0115 wall, door, side-wall, ornament, stairs/pit, and thing-pass slices",
    "  - DM1 V1 runtime: hardened chest, mirror-candidate, champion-panel, door-bash, sleep/wakeup, projectile, creature, and inventory regressions",
    "  - Release wiring: fixed the newest D1L/D1R viewport-gate packaging path before tagging",
    "",
    "V2.7.11  (2026-06-12)",
    "  - DM1 V1 Hall of Champions: fixed mirror-candidate survival and party movement timing so champions no longer drain or die while walking the Hall",
    "  - DM1 V1 mirrors: restored candidate slot ownership during confirm/cancel and preserved mirror runtime state across quick-resume sidecars",
    "  - DM1 V1 intro: corrected the FTL/SWSH PC palette rows and presented each source palette mutation immediately so the swoosh no longer races or uses Atari colors",
    "  - DM1 V1 title/runtime: restored the PC TITLE palette base and made the accessibility manifest opt-in to avoid per-frame disk writes during normal play",
    "",
    "V2.7.10  (2026-06-12)",
    "  - DM1 V1 viewport: expanded source-lock and pixel coverage across additional front, side, door, wall-ornament, floor, ceiling, pit, teleporter, and thing-pass paths",
    "  - DM1 V1 runtime: hardened chest, mirror-candidate, champion-panel, projectile, creature, poison-cloud, fake-wall, teleporter, keyhole, pit, fountain, skill, food/water, and Vi altar edge cases",
    "  - DM1 original-capture workflow: tightened DOSBox rawshot fallback, freshness checks, transcript rows, and 320x200/viewport crop validation for source comparison",
    "  - Regression coverage: broadened no-game-data and real-data gates while keeping release packaging on the green GitHub Actions verify matrix",
    "",
    "V2.7.9  (2026-06-12)",
    "  - DM1 V1 launch: fixed Retina/HiDPI window pixel-size events so entrance door buttons keep using SDL's logical mouse coordinate space on MacBook displays",
    "  - DM1 V1 FTL/SWSH: restored the ReDMCSB palette cadence by batching adjacent Setcolor commands and applying DBF wait counts as N+1 VBlanks",
    "  - Regression gates: added high-DPI resize mapping coverage and tightened the SWSH source-animation timing invariant",
    "",
    "V2.7.8  (2026-06-12)",
    "  - DM1 V1 viewport: added source-lock coverage for D1L2/D1R2, D3L2/D3R2, D2L2/D2R2, D0L2/D0R2, and D0C floor, ceiling, ornament, door-front, and thing-pass paths",
    "  - DM1 V1 inventory and mirror-candidate runtime: hardened chest occupied-slot swaps, scroll pickup/drop, C040 panel-live handoff, reshuffle, cancel, and candidate-close routes",
    "  - CSB V1 viewport/runtime: expanded D1L2/D1R2 and D2C/D0L2/D0R2 door/floor/ceiling evidence plus movement-command, command-chain, and CustomBackgrounds gates",
    "  - Verification: latest strict warnings, M10 verify, CMake build matrix, Phase A, audio probe, and cross-platform determinism run green on GitHub Actions before release",
    "",
    "V2.7.7  (2026-06-08)",
    "  - DM1 V1 viewport: expanded source-lock and pixel gates for side walls, floor/ceiling fallback, stairs/pit dispatch, door fronts, wall ornaments, and projectile side-cell behavior",
    "  - DM1 V1 inventory and champion panels: hardened chest routing, mirror-candidate handoff, hand-slot priority, status-hand, held-item, portrait, wound, and stale-pixel regressions",
    "  - Runtime coverage: added focused gates for chest pickup/swap/close/reopen edges, spell-rune preservation, poison/cloud timing, room-transition pickup ordering, delayed timeline saves, and keyhole no-op behavior",
    "  - Cross-game regressions: added CSB viewport/import/chaos/optional-asset gates plus DM2, Nexus, and Theron save/load, bounds, launch-marker, and transition guards",
    "",
    "V2.7.6  (2026-06-07)",
    "  - DM1 V1 inventory panel: added a source-locked status-row hand-slot routing regression gate",
    "  - DM1 V1 input safety: proves status hand boxes resolve to the correct champion/source slot without crossing the inventory swap path",
    "  - Regression gates: covers dead, candidate, open-inventory, out-of-party, null-health, and per-champion mouse-item routing edges",
    "",
    "V2.7.5  (2026-06-05)",
    "  - DM1 V1 launch: restored FTL/SWSH intro discovery for structured data directories",
    "  - DM1 V1 TITLE: restored ReDMCSB step-specific PRESENTS/DUNGEON/MASTER palette mapping",
    "  - DM1 V1 entrance: fixed button clicks after live macOS window-size changes",
    "  - Regression gates: added SWSH pathfinder, TITLE palette step, and entrance button click coverage",
    "",
    "V2.7.4  (2026-06-05)",
    "  - DM1 V1 viewport: fixed small-scale window layout and side-wall drawing regressions",
    "  - DM1 V1 runtime: added champion mirror visibility, mirror Z-order, chest compact-close, D2L side-wall, capture-route, and champion panel pixel probes",
    "  - DM1 V1 presentation: corrected wall inscription source-font rendering and slowed title cadence to the V1 tick path",
    "  - CI and worker hygiene: cleared stale Firestaff queue failures and made the CSB DSA probe mkdir path portable",
    "",
    "V2.7.3  (2026-06-03)",
    "  - Regression coverage: added launch/profile gates for DM1, CSB, DM2, Nexus, Theron, M11 overlay input, accessibility manifest, save browser, and M12 data-directory cancel paths",
    "  - Asset scanner: added no-data, irrelevant-root, partial-data, archive-backed, and required-file accounting guards",
    "  - Cross-platform build: fixed Windows stat/tempdir portability and static-library link order for the expanded test harnesses",
    "  - Verification: restored green GitHub Actions across Ubuntu, macOS, Windows, strict warnings, Phase A, audio smoke, and cross-platform determinism",
    "",
    "V2.7.2  (2026-06-03)",
    "  - Game-data scanner: recursive hash discovery, ZIP/ISO entry scanning, and archive-backed DM1/CSB/DM2 cache handoff",
    "  - Start menu: data-directory status wiring, missing-data popups, and safer launch gating for required hashes",
    "  - Theron V1: JP Rev 1 and US Track 02 ISO recognition plus direct runtime handoff into the native viewport path",
    "  - DM2 and CSB: real-asset loader/save/dungeon probe regressions fixed and reverified",
    "  - DM1 V1: viewport, inventory, movement-legality, and source-lock regression gates restored",
    "  - CSB V2/DM2 V2: smooth-movement verification gates restored",
    "",
    "V2.7.1  (2026-06-02)",
    "  - DM1 PC-34 boot: restore ReDMCSB SWSH/FTL logo playback before TITLE",
    "  - DM1 PC-34 TITLE: keep GRAPHICS.DAT C001 title zoom on source-locked final guard timing",
    "  - DM1 PC-34 entrance: source-locked vblank cadence for pre-open delay and door animation",
    "  - SWSH palette: drive FTL logo from ReDMCSB Setcolor commands instead of TITLE palette",
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
    return "2.7.17";
}
