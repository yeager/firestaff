/*
 * test_m11_point_in_source_box_pc34_compat.c
 *
 * DM1 V1 click hit-test regression gate for the modern
 * m11_point_in_source_box() function in src/engine/m11_game_view.c.
 *
 * This gate pins the contract of ReDMCSB's
 * F0376_COMMAND_IsPointInBox (CLIKVIEW.C:290):
 *
 *   return ((x <= right) && (x >= left) && (y <= bottom) && (y >= top));
 *
 * The modern firestaff helper has the same contract, packaged as
 * a 4-element int box {left, right, top, bottom}.  This test
 * verifies:
 *
 *   1. Inside the box (including the four edges) -> 1.
 *   2. Outside the box on every side -> 0.
 *   3. Door-button D1C box {160, 175, 44, 52} accepts a click
 *      at (167, 48) (door-button centre) and rejects (100, 100)
 *      (empty viewport corner).
 *   4. Wall-ornament box {96, 127, 35, 63} accepts (110, 50)
 *      and rejects (50, 50).
 *   5. Source-evidence cites ReDMCSB CLIKVIEW.C:290 +
 *      m11_game_view.c:10833.
 *   6. Contract-only: no live game data, no SDL rendering.
 *
 * Exit codes:
 *   0  - all checks passed
 *   1  - one or more checks failed
 *
 * Source-lock:
 *  - ReDMCSB CLIKVIEW.C:285-310 F0376_COMMAND_IsPointInBox
 *  - include/m11_game_view.h m11_point_in_source_box declaration
 *  - src/engine/m11_game_view.c:10833 m11_point_in_source_box impl
 *  - src/engine/m11_game_view.c:10995, 11007, 11092, 11100
 *    call-sites for door-button / wall-ornament / object-pile
 *  - M11_GAME_INPUT_IGNORED=0 / M11_GAME_INPUT_REDRAW=1
 *    from m11_game_view.h (referenced by callers)
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

/* ctest sets CWD to builds/n2-build, so we resolve the source file
 * relative to the project root.  Probe source root via FIRESTAFF_ROOT_PATH. */
#ifndef FIRESTAFF_ROOT_PATH
#define FIRESTAFF_ROOT_PATH "/Users/bosse/.openclaw/workspace-main/"
#endif

static int g_assertions = 0;
static int g_passed = 0;

static void check_int(const char *id, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
    } else {
        ++g_passed;
    }
}

static void check_str_contains(const char *id, const char *hay, const char *needle)
{
    ++g_assertions;
    if (!hay || !needle || !strstr(hay, needle)) {
        printf("FAIL %s missing substring: %s\n", id, needle ? needle : "(null)");
    } else {
        ++g_passed;
    }
}

/* Generic 4-corner box geometry. */
static void check_inside_outside(void)
{
    /* Square box at (10, 30) x (50, 60). */
    int box[4] = { 10, 30, 30, 60 };
    /* Inside, centre. */
    check_int("generic.inside.centre", m11_point_in_source_box(20, 45, box), 1);
    /* On each edge, inclusive. */
    check_int("generic.edge.left",    m11_point_in_source_box(10, 45, box), 1);
    check_int("generic.edge.right",   m11_point_in_source_box(30, 45, box), 1);
    check_int("generic.edge.top",     m11_point_in_source_box(20, 30, box), 1);
    check_int("generic.edge.bottom",  m11_point_in_source_box(20, 60, box), 1);
    /* Outside, just past each edge. */
    check_int("generic.outside.left",   m11_point_in_source_box(9,  45, box), 0);
    check_int("generic.outside.right",  m11_point_in_source_box(31, 45, box), 0);
    check_int("generic.outside.top",    m11_point_in_source_box(20, 29, box), 0);
    check_int("generic.outside.bottom", m11_point_in_source_box(20, 61, box), 0);
    /* Corner outside. */
    check_int("generic.corner.tl", m11_point_in_source_box(0, 0, box), 0);
    check_int("generic.corner.br", m11_point_in_source_box(100, 100, box), 0);
    /* Far outside. */
    check_int("generic.far.negative", m11_point_in_source_box(-100, -100, box), 0);
    check_int("generic.far.large",    m11_point_in_source_box(10000, 10000, box), 0);
}

/* Door-button D1C box (kDoorButtonD1CBox in m11_game_view.c:10988). */
static void check_d1c_door_button(void)
{
    int box[4] = { 160, 175, 44, 52 };
    /* Centre of door-button zone. */
    check_int("door_d1c.centre", m11_point_in_source_box(167, 48, box), 1);
    /* Top-left corner. */
    check_int("door_d1c.tl", m11_point_in_source_box(160, 44, box), 1);
    /* Bottom-right corner. */
    check_int("door_d1c.br", m11_point_in_source_box(175, 52, box), 1);
    /* Just outside (typical off-button click). */
    check_int("door_d1c.outside.left",  m11_point_in_source_box(159, 48, box), 0);
    check_int("door_d1c.outside.right", m11_point_in_source_box(176, 48, box), 0);
    check_int("door_d1c.outside.top",   m11_point_in_source_box(167, 43, box), 0);
    check_int("door_d1c.outside.bot",   m11_point_in_source_box(167, 53, box), 0);
    /* Far outside. */
    check_int("door_d1c.empty.corner", m11_point_in_source_box(100, 100, box), 0);
}

/* Wall-ornament box (g_wallOrnamentBox in m11_game_view.c:10832). */
static void check_wall_ornament_box(void)
{
    int box[4] = { 96, 127, 35, 63 };
    /* Centre. */
    check_int("wall_orn.centre", m11_point_in_source_box(110, 50, box), 1);
    /* Edges. */
    check_int("wall_orn.left",  m11_point_in_source_box(96,  50, box), 1);
    check_int("wall_orn.right", m11_point_in_source_box(127, 50, box), 1);
    check_int("wall_orn.top",   m11_point_in_source_box(110, 35, box), 1);
    check_int("wall_orn.bot",   m11_point_in_source_box(110, 63, box), 1);
    /* Outside. */
    check_int("wall_orn.out.left",  m11_point_in_source_box(50, 50, box), 0);
    check_int("wall_orn.out.right", m11_point_in_source_box(200, 50, box), 0);
    check_int("wall_orn.out.top",   m11_point_in_source_box(110, 10, box), 0);
    check_int("wall_orn.out.bot",   m11_point_in_source_box(110, 100, box), 0);
}

/* Degenerate boxes. */
static void check_degenerate(void)
{
    int zero[4] = { 0, 0, 0, 0 };
    /* Single-point box. Only (0, 0) should hit. */
    check_int("degenerate.zero.centre", m11_point_in_source_box(0, 0, zero), 1);
    check_int("degenerate.zero.1_0",   m11_point_in_source_box(1, 0, zero), 0);
    check_int("degenerate.zero.0_1",   m11_point_in_source_box(0, 1, zero), 0);
    /* Inverted box (right < left, bottom < top): every point is outside
     * because the inclusive range is empty.  The contract uses <=/>=,
     * so an inverted box is simply "empty". */
    int inverted[4] = { 100, 50, 100, 50 };
    check_int("degenerate.inverted.centre", m11_point_in_source_box(75, 75, inverted), 0);
    check_int("degenerate.inverted.left",  m11_point_in_source_box(50, 50, inverted), 0);
    check_int("degenerate.inverted.right", m11_point_in_source_box(100, 100, inverted), 0);
}

/* The contract must match F0376_COMMAND_IsPointInBox exactly.  Both
 * use <= and >= on the same four edges.  Build a 16-point grid and
 * verify that the modern helper matches an explicit F0376 reference
 * implementation on every point. */
static int f0376_reference(int px, int py, const int box[4])
{
    return (px <= box[1]) && (px >= box[0]) && (py <= box[3]) && (py >= box[2]);
}

static void check_f0376_equivalence(void)
{
    int box[4] = { 17, 53, 88, 124 };
    int dx, dy;
    int mismatch = 0;
    /* Sweep a 100x100 grid around the box. */
    for (dy = 60; dy < 150; ++dy) {
        for (dx = 0; dx < 100; ++dx) {
            int modern = m11_point_in_source_box(dx, dy, box);
            int reference = f0376_reference(dx, dy, box);
            if (modern != reference) {
                printf("FAIL f0376.equiv(%d,%d) modern=%d reference=%d\n",
                       dx, dy, modern, reference);
                mismatch = 1;
            }
        }
    }
    ++g_assertions;
    if (mismatch) {
        printf("FAIL f0376.equivalence (mismatch above)\n");
    } else {
        ++g_passed;
    }
}

/* Confirm the contract is documented in the source. */
static void check_source_evidence(void)
{
    /* The implementation lives at m11_game_view.c around the
     * m11_point_in_source_box function.  We test by reading the
     * file and grepping for the canonical pattern.  The file
     * is 1.2 MB so we read it in chunks. */
    FILE *f = fopen(FIRESTAFF_ROOT_PATH "src/engine/m11_game_view.c", "r");
    char buf[4096];
    size_t n;
    int found_modern = 0;
    int found_f0376 = 0;
    if (!f) {
        printf("FAIL evidence: cannot open m11_game_view.c\n");
        ++g_assertions;
        return;
    }
    while ((n = fread(buf, 1, sizeof(buf) - 1, f)) > 0) {
        buf[n] = 0;
        if (!found_modern && strstr(buf, "m11_point_in_source_box")) {
            found_modern = 1;
        }
        if (!found_f0376 && strstr(buf, "F0376")) {
            found_f0376 = 1;
        }
        if (found_modern && found_f0376) break;
    }
    fclose(f);
    if (found_modern) ++g_passed; else printf("FAIL evidence.m11_game_view missing\n");
    ++g_assertions;
    if (found_f0376) ++g_passed; else printf("FAIL evidence.f0376 missing\n");
    ++g_assertions;
}

int main(void)
{
    printf("=== m11_point_in_source_box F0376 contract regression ===\n");
    check_inside_outside();
    check_d1c_door_button();
    check_wall_ornament_box();
    check_degenerate();
    check_f0376_equivalence();
    check_source_evidence();
    printf("--- %d / %d passed ---\n", g_passed, g_assertions);
    if (g_passed != g_assertions) {
        return 1;
    }
    return 0;
}
