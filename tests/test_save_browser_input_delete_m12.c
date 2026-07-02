/*
 * test_save_browser_input_delete_m12.c — Data-free regression for the
 * M12 save-browser launcher handoff boundary.
 *
 * Pins the M12 launcher-side input dispatch contract that decides which
 * save the M12 → M11 game view handoff will read. Without this gate the
 * navigation, scroll-window, delete-confirmation, and on-disk delete
 * path of `src/shared/save_browser_m12.c` only had one focused test
 * (`test_save_browser_export_import_m12`) which exercises export/import
 * plus ACCEPT against a pre-classified manifest entry, leaving the rest
 * of the HandleInput / DeleteSelected / Scan-sort behaviour uncovered.
 *
 * Source-locked against:
 *   - `M12_SaveBrowser_Scan` (src/shared/save_browser_m12.c:308)
 *   - `compare_entries` qsort key (newest-first mtime, filename
 *     tie-break) at src/shared/save_browser_m12.c:295-302
 *   - `M12_SaveBrowser_HandleInput` UP/DOWN/ACCEPT/ACTION branch
 *     (input codes 1=UP, 2=DOWN, 5=ACCEPT, 7=ACTION) at
 *     src/shared/save_browser_m12.c:364-409
 *   - `M12_SaveBrowser_DeleteSelected` file-remove + entry-shift +
 *     selection-clamp at src/shared/save_browser_m12.c:411-435
 *   - `M12_SaveBrowser_DeleteSelected` selection-clamp behaviour for
 *     last-entry and empty-after-delete states.
 *   - `M12_SaveBrowser_GetSelected` out-of-bounds / NULL / empty
 *     rejection at src/shared/save_browser_m12.c:488-494.
 *
 * Disjoint from:
 *   - `test_save_browser_no_data.c` (missing root / empty dir /
 *     filename filter — also currently not wired into CMakeLists,
 *     and intentionally not promoted here).
 *   - `test_save_browser_export_import_m12.c` (export/import copy +
 *     PC34 manifest MATCH/WRONG_GAME classification).
 *
 * Honest scope: data-free deterministic contract for the launcher's
 * save-browser input → file handoff boundary only. No real Firestaff
 * save bytes, no SDL window, no M11 game-view tick, no original-game
 * pixel parity claim.
 */

#include "save_browser_m12.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <process.h>
#include <sys/utime.h>
#define unlink(path) _unlink(path)
#define rmdir(path) _rmdir(path)
typedef struct _utimbuf firestaff_utimbuf;
static int portable_utime(const char* path, firestaff_utimbuf* t) {
    return _utime(path, t);
}
#else
#include <unistd.h>
#include <utime.h>
typedef struct utimbuf firestaff_utimbuf;
static int portable_utime(const char* path, firestaff_utimbuf* t) {
    return utime(path, t);
}
#endif

static int g_failures = 0;
static int g_checks = 0;

static void check(int ok, const char* name) {
    ++g_checks;
    if (!ok) {
        ++g_failures;
        printf("FAIL %s\n", name);
    }
}

static int portable_mkdtemp(char* templ) {
#ifdef _WIN32
    char* marker = strstr(templ, "XXXXXX");
    int i;
    if (!marker) return 0;
    for (i = 0; i < 1000; ++i) {
        snprintf(marker, 7, "%06ld", ((long)_getpid() + i) % 1000000L);
        if (_mkdir(templ) == 0) return 1;
    }
    return 0;
#else
    return mkdtemp(templ) != NULL;
#endif
}

static int write_fixture_file(const char* dir, const char* name) {
    char path[512];
    FILE* fp;

    snprintf(path, sizeof(path), "%s/%s", dir, name);
    fp = fopen(path, "wb");
    if (!fp) return 0;
    fputs("not a valid Firestaff save\n", fp);
    return fclose(fp) == 0;
}

static int set_fixture_mtime(const char* dir, const char* name, time_t when) {
    char path[512];
    firestaff_utimbuf ub;

    snprintf(path, sizeof(path), "%s/%s", dir, name);
    ub.actime = when;
    ub.modtime = when;
    return portable_utime(path, &ub) == 0;
}

static int file_exists_on_disk(const char* dir, const char* name) {
    char path[512];
    struct stat st;
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static void remove_fixture(const char* dir, const char* name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    unlink(path);
}

/* Build a 3-entry fixture directory with distinct mtimes so the
 * Scan sort places newest-first. Returns 1 on success, 0 on failure. */
static int build_three_entry_fixture(const char* root) {
    time_t base;

    if (!write_fixture_file(root, "firestaff-dm1-old.sav")) return 0;
    if (!write_fixture_file(root, "firestaff-dm1-mid.sav")) return 0;
    if (!write_fixture_file(root, "firestaff-dm1-new.sav")) return 0;
    base = 1700000000; /* 2023-11-14 */
    if (!set_fixture_mtime(root, "firestaff-dm1-old.sav", base)) return 0;
    if (!set_fixture_mtime(root, "firestaff-dm1-mid.sav", base + 60)) return 0;
    if (!set_fixture_mtime(root, "firestaff-dm1-new.sav", base + 120)) return 0;
    return 1;
}

static void teardown_three_entry_fixture(const char* root) {
    remove_fixture(root, "firestaff-dm1-old.sav");
    remove_fixture(root, "firestaff-dm1-mid.sav");
    remove_fixture(root, "firestaff-dm1-new.sav");
}

int main(void) {
    char tmpTemplate[] = "/tmp/firestaff-save-browser-input-delete-XXXXXX";
    M12_SaveBrowserState state;
    const M12_SaveBrowserEntry* selected;

    /* -------------------------------------------------------------- */
    /*  Group A — NULL / empty-state guards                            */
    /* -------------------------------------------------------------- */
    memset(&state, 0x5a, sizeof(state));
    check(M12_SaveBrowser_HandleInput(NULL, 1) == 0,
          "Group A: NULL state returns 0");
    check(M12_SaveBrowser_HandleInput(&state, 1) == 0,
          "Group A: empty state (entryCount=0) returns 0 for UP");
    check(M12_SaveBrowser_HandleInput(&state, 7) == 0,
          "Group A: empty state (entryCount=0) returns 0 for ACTION");

    /* -------------------------------------------------------------- */
    /*  Group B — UP / DOWN scroll bounds (assume ~8 visible rows)     */
    /* -------------------------------------------------------------- */
    if (!portable_mkdtemp(tmpTemplate)) {
        perror("portable_mkdtemp");
        return 1;
    }
    if (!build_three_entry_fixture(tmpTemplate)) {
        fprintf(stderr, "FAIL: could not build three-entry fixture\n");
        return 1;
    }
    check(M12_SaveBrowser_Scan(&state, tmpTemplate) == 3,
          "Group B: Scan finds three entries");

    /* UP at top is no-op (selectedIndex stays 0, scrollOffset stays 0). */
    state.selectedIndex = 0;
    state.scrollOffset = 0;
    check(M12_SaveBrowser_HandleInput(&state, 1) == 0,
          "Group B: UP at top returns 0 (no navigation)");
    check(state.selectedIndex == 0 && state.scrollOffset == 0,
          "Group B: UP at top preserves selectedIndex=0 and scrollOffset=0");

    /* DOWN at bottom is no-op (selectedIndex stays at last). */
    state.selectedIndex = state.entryCount - 1; /* 2 */
    state.scrollOffset = 0;
    check(M12_SaveBrowser_HandleInput(&state, 2) == 0,
          "Group B: DOWN at bottom returns 0 (no navigation)");
    check(state.selectedIndex == 2,
          "Group B: DOWN at bottom preserves selectedIndex at last");

    /* Unknown input code is a no-op (no scroll, no confirm). */
    state.selectedIndex = 1;
    state.scrollOffset = 0;
    state.confirmDelete = 0;
    check(M12_SaveBrowser_HandleInput(&state, 99) == 0,
          "Group B: unknown input code returns 0");
    check(state.selectedIndex == 1 && state.scrollOffset == 0 &&
          state.confirmDelete == 0,
          "Group B: unknown input code preserves selection + scroll + delete state");

    /* -------------------------------------------------------------- */
    /*  Group C — Sort order with distinct mtimes (newest first)      */
    /* -------------------------------------------------------------- */
    /* After Scan, the entries should be ordered newest-first by mtime:
     *   firestaff-dm1-new.sav (mtime base+120) first
     *   firestaff-dm1-mid.sav (mtime base+60)   second
     *   firestaff-dm1-old.sav (mtime base)      third
     * Even though the on-disk directory iteration order is unspecified,
     * the qsort inside Scan must canonicalise to mtime desc. */
    check(strcmp(state.entries[0].filename, "firestaff-dm1-new.sav") == 0,
          "Group C: newest-mtime entry sorts to position 0");
    check(strcmp(state.entries[1].filename, "firestaff-dm1-mid.sav") == 0,
          "Group C: middle-mtime entry sorts to position 1");
    check(strcmp(state.entries[2].filename, "firestaff-dm1-old.sav") == 0,
          "Group C: oldest-mtime entry sorts to position 2");

    /* -------------------------------------------------------------- */
    /*  Group D — DELETE flow (ACTION + ACCEPT cancellation)          */
    /* -------------------------------------------------------------- */
    /* The "old" entry is at selectedIndex 2 (per the sort above). */
    state.selectedIndex = 2;
    state.scrollOffset = 0;
    state.confirmDelete = 0;
    check(M12_SaveBrowser_HandleInput(&state, 7) == 0,
          "Group D: ACTION sets confirmDelete and returns 0");
    check(state.confirmDelete == 1,
          "Group D: ACTION leaves confirmDelete=1");

    /* Non-ACCEPT input after ACTION cancels confirmDelete (no delete).
     * Per the source contract this is a hard short-circuit BEFORE the
     * UP/DOWN switch — the cancellation path does NOT also apply the
     * navigation handler. selectedIndex stays put. */
    check(M12_SaveBrowser_HandleInput(&state, 1) == 0,
          "Group D: non-ACCEPT cancels confirmDelete and returns 0");
    check(state.confirmDelete == 0,
          "Group D: non-ACCEPT clears confirmDelete to 0");
    check(file_exists_on_disk(tmpTemplate, "firestaff-dm1-old.sav"),
          "Group D: cancelled confirmDelete leaves file on disk");
    check(state.selectedIndex == 2,
          "Group D: cancellation is a hard short-circuit, selectedIndex stays at 2");

    /* ACTION again, then ACCEPT → actual delete on disk.
     * selectedIndex is 2 (firestaff-dm1-old.sav). */
    check(M12_SaveBrowser_HandleInput(&state, 7) == 0,
          "Group D: ACTION re-arms confirmDelete");
    check(state.confirmDelete == 1,
          "Group D: confirmDelete back to 1 after second ACTION");
    check(M12_SaveBrowser_HandleInput(&state, 5) == 0,
          "Group D: ACCEPT on confirmDelete performs delete and returns 0 (not load)");
    check(state.confirmDelete == 0,
          "Group D: ACCEPT clears confirmDelete after performing delete");
    check(!file_exists_on_disk(tmpTemplate, "firestaff-dm1-old.sav"),
          "Group D: ACCEPT deletes the tail (oldest) file from disk");
    check(state.entryCount == 2,
          "Group D: entry list shrinks by one after delete");
    /* After deleting the TAIL entry, the tail slot is overwritten with
     * the zero-initialised trailing array member (the shift loop has
     * no successor). entries[1] (firestaff-dm1-mid.sav) is unchanged
     * because it sits before the deleted slot. */
    check(strcmp(state.entries[1].filename, "firestaff-dm1-mid.sav") == 0,
          "Group D: mid slot is unchanged after tail delete");
    /* selectedIndex (was 2) >= entryCount (now 2) clamps to entryCount-1=1. */
    check(state.selectedIndex == 1,
          "Group D: selectedIndex clamps to new last entry after tail delete");

    /* -------------------------------------------------------------- */
    /*  Group E — ACCEPT on a valid entry returns 1 (load handoff)    */
    /* -------------------------------------------------------------- */
    /* selectedIndex is 1 (firestaff-dm1-old.sav, corrupt → invalid).
     * Navigate DOWN once is a no-op (last), so the load handoff on the
     * current selection will refuse (valid=0). */
    check(M12_SaveBrowser_HandleInput(&state, 5) == 0,
          "Group E: ACCEPT on invalid entry returns 0 (no load handoff)");

    /* Navigate UP to firestaff-dm1-new.sav at selectedIndex 0 and
     * accept — the corrupt fixture makes valid=0 but the contract is
     * still "ACCEPT returns entries[i].valid ? 1 : 0", so 0 is correct
     * for the corrupt fixture regardless of which entry we pick. We
     * re-arm confirmDelete to verify that Accept WITHOUT confirmDelete
     * returns the valid-bit (0 here, but the path is load-handoff, not
     * delete). */
    state.selectedIndex = 0;
    state.confirmDelete = 0;
    check(M12_SaveBrowser_HandleInput(&state, 5) == 0,
          "Group E: ACCEPT on corrupt top entry returns 0 (load-handoff path, not delete)");
    check(state.entryCount == 2,
          "Group E: ACCEPT without confirmDelete does NOT delete");
    check(file_exists_on_disk(tmpTemplate, "firestaff-dm1-new.sav"),
          "Group E: ACCEPT without confirmDelete leaves top file on disk");

    /* -------------------------------------------------------------- */
    /*  Group F — DeleteSelected direct API                           */
    /* -------------------------------------------------------------- */
    /* Delete the currently selected (top) entry through the direct
     * API rather than through HandleInput. selectedIndex is 0
     * (firestaff-dm1-new.sav, the newest) at this point — after Group E
     * set it explicitly to 0 and no navigation happened in between. */
    state.confirmDelete = 0;
    check(M12_SaveBrowser_DeleteSelected(&state) == 0,
          "Group F: DeleteSelected on top entry returns 0");
    check(!file_exists_on_disk(tmpTemplate, "firestaff-dm1-new.sav"),
          "Group F: DeleteSelected removes top file from disk");
    check(state.entryCount == 1,
          "Group F: DeleteSelected decrements entryCount");
    check(strcmp(state.entries[0].filename, "firestaff-dm1-mid.sav") == 0,
          "Group F: remaining entry is mid (the only non-deleted slot)");

    /* Delete the last remaining entry — selection should reset to 0. */
    check(M12_SaveBrowser_DeleteSelected(&state) == 0,
          "Group F: DeleteSelected on last entry returns 0");
    check(state.entryCount == 0,
          "Group F: entryCount reaches 0 after deleting the last entry");
    check(state.selectedIndex == 0,
          "Group F: selectedIndex resets to 0 when entryCount is 0");
    check(!file_exists_on_disk(tmpTemplate, "firestaff-dm1-old.sav"),
          "Group F: last file is removed from disk");

    /* DeleteSelected on empty state returns -1, NULL state returns -1. */
    check(M12_SaveBrowser_DeleteSelected(&state) == -1,
          "Group F: DeleteSelected on empty state returns -1");
    check(M12_SaveBrowser_DeleteSelected(NULL) == -1,
          "Group F: DeleteSelected with NULL state returns -1");

    /* -------------------------------------------------------------- */
    /*  Group G — GetSelected out-of-bounds / empty                  */
    /* -------------------------------------------------------------- */
    state.selectedIndex = -1;
    check(M12_SaveBrowser_GetSelected(&state) == NULL,
          "Group G: GetSelected returns NULL for selectedIndex=-1");
    state.selectedIndex = 5;
    check(M12_SaveBrowser_GetSelected(&state) == NULL,
          "Group G: GetSelected returns NULL for selectedIndex>=entryCount");
    state.selectedIndex = 0;
    check(M12_SaveBrowser_GetSelected(&state) == NULL,
          "Group G: GetSelected returns NULL when entryCount=0");
    check(M12_SaveBrowser_GetSelected(NULL) == NULL,
          "Group G: GetSelected returns NULL for NULL state");

    /* -------------------------------------------------------------- */
    /*  Group H — Scroll-window boundary: scrollOffset follows UP     */
    /* -------------------------------------------------------------- */
    /* Build a 10-entry fixture so DOWN can cross the +8 visible-row
     * boundary and UP can drag scrollOffset back. */
    {
        int i;
        char name[64];
        time_t base = 1700000000;
        for (i = 0; i < 10; ++i) {
            snprintf(name, sizeof(name),
                     "firestaff-dm1-%02d.sav", i);
            if (!write_fixture_file(tmpTemplate, name)) {
                fprintf(stderr, "FAIL: could not write %s\n", name);
                return 1;
            }
            if (!set_fixture_mtime(tmpTemplate, name,
                                   base + (time_t)(i * 10))) {
                fprintf(stderr, "FAIL: could not set mtime for %s\n", name);
                return 1;
            }
        }
        memset(&state, 0, sizeof(state));
        check(M12_SaveBrowser_Scan(&state, tmpTemplate) == 10,
              "Group H: Scan finds ten entries");

        /* Walk DOWN nine times — by step 9 selectedIndex reaches 9 (last)
         * and the DOWN handler should clamp so the last scroll call
         * keeps selectedIndex at 9 (DOWN at bottom is a no-op). */
        for (i = 0; i < 12; ++i) {
            (void)M12_SaveBrowser_HandleInput(&state, 2);
        }
        check(state.selectedIndex == 9,
              "Group H: DOWN past bottom clamps selectedIndex to last");
        check(state.scrollOffset == 2,
              "Group H: DOWN window scrolls to keep selected visible (offset 9-7)");

        /* Walk UP from index 9. scrollOffset (currently 2) only drops
         * when selectedIndex itself drops below 2. Five UPs land at
         * selectedIndex=4 (still above scrollOffset=2), so the scroll
         * window must stay put. */
        for (i = 0; i < 5; ++i) {
            (void)M12_SaveBrowser_HandleInput(&state, 1);
        }
        check(state.selectedIndex == 4,
              "Group H: 5 UP moves selectedIndex from 9 to 4");
        check(state.scrollOffset == 2,
              "Group H: scrollOffset stays at 2 while selectedIndex stays above it");

        /* Three more UPs land at selectedIndex=1 (which IS below
         * scrollOffset=2), so the scroll window drags down to 1. */
        for (i = 0; i < 3; ++i) {
            (void)M12_SaveBrowser_HandleInput(&state, 1);
        }
        check(state.selectedIndex == 1,
              "Group H: 3 more UPs move selectedIndex from 4 to 1");
        check(state.scrollOffset == 1,
              "Group H: scrollOffset drags down to 1 once selectedIndex < scrollOffset");

        /* Walk UP all the way back to 0 — scrollOffset must reach 0 too
         * (no negative scroll). */
        for (i = 0; i < 10; ++i) {
            (void)M12_SaveBrowser_HandleInput(&state, 1);
        }
        check(state.selectedIndex == 0,
              "Group H: UP past top clamps selectedIndex to 0");
        check(state.scrollOffset == 0,
              "Group H: scrollOffset clamps to 0 (never negative)");

        /* Selected should now point at the FIRST entry in the fixture
         * (which has the LOWEST mtime after the qsort newest-first).
         * Group H fixture is built with mtime = base + i*10, so the
         * fixture-name order is ascending mtime, and the newest-first
         * sort places firestaff-dm1-09.sav at index 0. */
        selected = M12_SaveBrowser_GetSelected(&state);
        check(selected != NULL,
              "Group H: GetSelected returns non-NULL after navigation");
        if (selected) {
            check(strcmp(selected->filename, "firestaff-dm1-09.sav") == 0,
                  "Group H: top-of-list after scroll-up is the newest fixture");
        }

        /* Cleanup the 10-fixture lot. */
        for (i = 0; i < 10; ++i) {
            snprintf(name, sizeof(name),
                     "firestaff-dm1-%02d.sav", i);
            remove_fixture(tmpTemplate, name);
        }
    }

    /* Cleanup Group B / C / D / E / F fixture. */
    teardown_three_entry_fixture(tmpTemplate);
    rmdir(tmpTemplate);

    if (g_failures) {
        printf("test_save_browser_input_delete_m12: FAIL %d/%d\n",
               g_failures, g_checks);
        return 1;
    }
    printf("test_save_browser_input_delete_m12: PASS %d/%d\n",
           g_checks, g_checks);
    return 0;
}
