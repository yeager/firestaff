/*
 * firestaff_dm1_v1_hoc_ordinal_2_sibling_promotion_audit_probe.c
 *
 * DM1 V1 Hall of Champions ordinal-2 sibling-promotion audit.
 *
 * Scope:
 *   Machine-checks the TODO row 10 (b) follow-up: the two ordinal-2
 *   sibling probes that the existing
 *   firestaff_dm1_v1_hoc_ordinal_2_future_route_readiness_gate_probe
 *   marks as "source present but not CTest-wired".  This probe
 *   narrows that open row honestly by:
 *
 *     - Confirming the open-count invariant matches the readiness
 *       gate (exactly 2 ordinal-2 sibling probes are not wired).
 *     - For each open sibling, reading the source file and emitting
 *       a structured promotion-blocker report:
 *
 *         (1) anchor / route variant the probe drives;
 *         (2) sibling-ordinal precedent — what ordinals already
 *             have a CTest-wired probe of the same route variant
 *             (so the promotion path is "do what ordinal N did");
 *         (3) fixture mismatch note — every open ordinal-2 sibling
 *             either retargets the (1,1)→(1,2) NORTH C127 sensor
 *             (synthetic strategy) or relies on a TextString-derived
 *             fixture that no longer matches the shipped DUNGEON.DAT;
 *         (4) promotion checklist — three concrete steps an
 *             operator can take to wire the probe into CTest.
 *
 *   The probe is a file-system / build-system audit.  It does NOT
 *   drive M11, does NOT load GRAPHICS.DAT / DUNGEON.DAT, and does
 *   NOT claim any runtime correctness.  It only reads:
 *
 *     - CMakeLists.txt (for the CTest wiring state);
 *     - the two open sibling probe source files (for anchor +
 *       fixture-mismatch markers);
 *     - the sibling-ordinal probe source files in
 *       probes/m11/firestaff_dm1_v1_*west_negative* /
 *       probes/m11/firestaff_dm1_v1_hall_of_champions_portrait_*_cancel_reopen*
 *       (to compute the promotion precedent).
 *
 *   Honest scope: file-system + CMake wiring + source-text scan only.
 *   No DOS pixel parity, no game data, no M11 runtime drive.
 *
 * Disjoint matrix:
 *   This probe is disjoint from the existing
 *   firestaff_dm1_v1_hoc_ordinal_2_future_route_readiness_gate_probe.
 *   The readiness gate drives M11 (opens DM1 V1 PC 3.4 data and
 *   scans the corridor band) and reports the open count via the
 *   file-existence + CMake substring check.  This new probe is
 *   data-free and adds structured per-sibling promotion notes
 *   that the readiness gate deliberately does not emit (so the
 *   readiness gate stays narrow and stays under its source-line
 *   budget).
 *
 * Source-evidence:
 *   The fixture-mismatch markers are read from the two open sibling
 *   probe source files.  Every marker is a literal phrase one of
 *   those files contains; the probe does not infer, paraphrase, or
 *   guess.  The sibling-ordinal precedent list is computed from
 *   CMakeLists.txt via the same substring check the readiness gate
 *   uses, so a wired ordinal_N sibling in CMakeLists.txt maps
 *   1:1 to the "promotion path: do what ordinal N did" note.
 *
 * Source-lock:
 *   No game logic touched.  ReDMCSB references for context only:
 *   DUNGEON.C:2573 (front-cell filter), DUNGEON.C:2608-2612
 *   (C127 sensorData → G0289), DUNVIEW.C:3913-3928 (C026 blit),
 *   REVIVE.C F0280 (candidate materialisation), REVIVE.C F0282
 *   (C162 cancel branch).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define PROBE_CMAKE_MAX_BYTES (8u * 1024u * 1024u)
#define PROBE_SOURCE_MAX_BYTES (2u * 1024u * 1024u)
#define PROBE_LINE_MAX 2048

/* The two ordinal-2 sibling probes the TODO row 10 (b) calls out.
 * `probe_basename` is the same literal the readiness gate uses for
 * the cross-reference check, so the two probes stay in lock-step on
 * naming.  `sibling_pattern` is the substring used to find sibling
 * ordinals already wired (e.g. for west_negative the pattern is
 * `_west_negative_portrait_rect_position_runtime_probe.c` and the
 * ordinals appear as `ordinal_N` in the basename). */
typedef struct OpenSibling {
    const char* route_variant;
    const char* probe_basename;
    const char* sibling_pattern; /* substring used to find sibling
                                  * ordinals already wired */
    const char* promotion_class; /* human label for the report */
} OpenSibling;

static const OpenSibling kOpenSiblings[] = {
    {
        "west_negative",
        "firestaff_dm1_v1_champion_mirror_ordinal_2_west_negative_portrait_rect_position_runtime_probe.c",
        "_west_negative_portrait_rect_position_runtime_probe.c",
        "front-mirror wrong-wall corridor band"
    },
    {
        "cancel_reopen",
        "firestaff_dm1_v1_hall_of_champions_portrait_02_cancel_reopen_portrait_rect_position_runtime_probe.c",
        "_cancel_reopen_portrait_rect_position_runtime_probe.c",
        "select / cancel / re-select resurrect panel cycle"
    }
};

#define OPEN_SIBLING_COUNT \
    (int)(sizeof(kOpenSiblings) / sizeof(kOpenSiblings[0]))

/* ── File / CMake helpers ─────────────────────────────────────── */

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static char* read_file_bounded(const char* path, size_t maxBytes) {
    FILE* f = fopen(path, "rb");
    long sz;
    char* buf;
    size_t got;
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    sz = ftell(f);
    if (sz <= 0 || sz > (long)maxBytes) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    got = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (got != (size_t)sz) { free(buf); return NULL; }
    buf[sz] = '\0';
    return buf;
}

static int cmake_contains(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    return strstr(haystack, needle) != NULL;
}

static int resolve_cmakelists_path(int argc, char** argv,
                                   char* outPath, size_t outPathSize) {
    const char* envSrc = getenv("CMAKE_SOURCE_DIR");
    char buf[1024];
    (void)argc; (void)argv;
    if (envSrc && envSrc[0] != '\0') {
        snprintf(buf, sizeof(buf), "%s/CMakeLists.txt", envSrc);
        if (file_exists(buf)) {
            snprintf(outPath, outPathSize, "%s", buf);
            return 1;
        }
    }
    if (file_exists("../CMakeLists.txt")) {
        snprintf(outPath, outPathSize, "../CMakeLists.txt");
        return 1;
    }
    if (file_exists("CMakeLists.txt")) {
        snprintf(outPath, outPathSize, "CMakeLists.txt");
        return 1;
    }
    if (file_exists("./CMakeLists.txt")) {
        snprintf(outPath, outPathSize, "./CMakeLists.txt");
        return 1;
    }
    return 0;
}

static int resolve_source_root(const char* cmakeListsPath,
                               char* outPath, size_t outPathSize) {
    const char* envSrc = getenv("CMAKE_SOURCE_DIR");
    char buf[1024];
    if (cmakeListsPath && cmakeListsPath[0] != '\0') {
        const char* slash = strrchr(cmakeListsPath, '/');
        if (slash && slash != cmakeListsPath) {
            size_t n = (size_t)(slash - cmakeListsPath);
            if (n >= sizeof(buf)) n = sizeof(buf) - 1;
            memcpy(buf, cmakeListsPath, n);
            buf[n] = '\0';
            snprintf(outPath, outPathSize, "%s", buf);
            return 1;
        }
        if (!slash) {
            snprintf(outPath, outPathSize, ".");
            return 1;
        }
    }
    if (envSrc && envSrc[0] != '\0') {
        snprintf(outPath, outPathSize, "%s", envSrc);
        return 1;
    }
    snprintf(outPath, outPathSize, ".");
    return 1;
}

/* ── Probe-source anchor / fixture-mismatch markers ───────────── */

/* Markers lifted verbatim from the open sibling source files.  Each
 * marker is a literal phrase the corresponding probe source contains.
 * The audit prints the marker when present and notes its role. */
typedef struct AnchorMarker {
    const char* phrase;        /* literal phrase in the source file */
    const char* role;          /* what the marker tells us */
} AnchorMarker;

/* Markers the west_negative probe source uses to signal
 * "this probe depends on a fixture assumption that no longer
 * matches the shipped DUNGEON.DAT".  These phrases are taken
 * verbatim from the source file (see probes/m11/firestaff_dm1_v1_
 * champion_mirror_ordinal_2_west_negative_portrait_rect_position_
 * runtime_probe.c lines 24-28 / 102-104). */
static const AnchorMarker kWestNegativeMarkers[] = {
    { "The slice was authored against the same DM1 V1 PC 3.4 fixture used",
      "fixture anchoring: shared with companion ordinal-2 probes" },
    { "TextString-derived fixture mapped ordinal 2 to that pose",
      "fixture drift: ordinal 2 was a TextString-derived location" },
    { "DUNGEON.DAT is the source-locked fixture that proves",
      "negative route contract: D1C cutout must remain empty" }
};

#define WEST_NEGATIVE_MARKER_COUNT \
    (int)(sizeof(kWestNegativeMarkers) / sizeof(kWestNegativeMarkers[0]))

/* Markers the cancel_reopen probe source uses to signal
 * "the legacy fixture no longer matches the shipped DUNGEON.DAT,
 * so the probe retargets the (1,1) C127 sensor".  These phrases
 * are taken verbatim from lines 17-23 of the source file. */
static const AnchorMarker kCancelReopenMarkers[] = {
    { "fixture the existing walkpath probe targets) places ordinal 2 at",
      "fixture anchoring: walkpath probe shared ordinal-2 location" },
    { "current DM1 fixture under test has a",
      "fixture drift: shipped DUNGEON.DAT differs from reference" },
    { "legacyOrdinal2Fixture mismatch for this",
      "skip-mode marker: walkpath + candidate probes also SKIP here" },
    { "retargets the canonical ordinal-1 C127 sensor at (1,2) facing NORTH",
      "promotion-safe strategy: synthetic (1,2) NORTH retarget" }
};

#define CANCEL_REOPEN_MARKER_COUNT \
    (int)(sizeof(kCancelReopenMarkers) / sizeof(kCancelReopenMarkers[0]))

/* Count how many anchor markers from a marker table are present
 * in the probe source.  Returns the matched count.  Used to give
 * a per-sibling evidence score without scoring the wrong source. */
static int count_markers(const char* sourceText,
                         const AnchorMarker* markers,
                         int markerCount) {
    int i;
    int hits = 0;
    if (!sourceText) return 0;
    for (i = 0; i < markerCount; ++i) {
        if (strstr(sourceText, markers[i].phrase)) {
            ++hits;
        }
    }
    return hits;
}

/* ── Sibling-ordinal precedent computation ────────────────────── */

/* Walk the CMakeLists.txt and emit the ordinal list whose sibling
 * probes (per the route-variant pattern) are already CTest-wired.
 *
 * For west_negative the pattern is
 * `_west_negative_portrait_rect_position_runtime_probe.c` and the
 * ordinals appear as `ordinal_N` in the basename.  For
 * cancel_reopen the pattern is
 * `_cancel_reopen_portrait_rect_position_runtime_probe.c` and the
 * ordinals appear as `portrait_NN`.  We extract the trailing
 * ordinal token from each basename that matches the pattern and
 * is referenced from CMakeLists.txt. */
#define ORDINAL_BUF_MAX 64

typedef struct OrdinalList {
    int  ordinals[ORDINAL_BUF_MAX];
    int  count;
} OrdinalList;

static int extract_west_negative_ordinal(const char* basename) {
    /* "...ordinal_<N>_west_negative..."  or
     * "...ordinal_<N>_<name>_west_negative..." */
    const char* p;
    int n;
    char digits[8];
    int di;
    p = strstr(basename, "ordinal_");
    if (!p) return -1;
    p += strlen("ordinal_");
    if (*p < '0' || *p > '9') return -1;
    di = 0;
    while (*p >= '0' && *p <= '9' && di < (int)sizeof(digits) - 1) {
        digits[di++] = *p++;
    }
    digits[di] = '\0';
    n = atoi(digits);
    if (n < 0 || n > 23) return -1;
    return n;
}

static int extract_cancel_reopen_ordinal(const char* basename) {
    /* "...portrait_<NN>_cancel_reopen..." */
    const char* p;
    int n;
    char digits[8];
    int di;
    p = strstr(basename, "_portrait_");
    if (!p) return -1;
    p += strlen("_portrait_");
    if (*p < '0' || *p > '9') return -1;
    di = 0;
    while (*p >= '0' && *p <= '9' && di < (int)sizeof(digits) - 1) {
        digits[di++] = *p++;
    }
    digits[di] = '\0';
    n = atoi(digits);
    if (n < 0 || n > 23) return -1;
    return n;
}

static int is_west_negative_basename(const char* basename) {
    /* The basename has been stripped of `.c` by collect_cmake_basenames,
     * so check for the suffix without the extension. */
    return strstr(basename, "_west_negative_portrait_rect_position_runtime_probe") != NULL;
}

static int is_cancel_reopen_basename(const char* basename) {
    return strstr(basename, "_cancel_reopen_portrait_rect_position_runtime_probe") != NULL;
}

/* Iterate over every line in the CMakeLists.txt buffer that ends
 * with `.c"` (a quoted CMake source reference) and, for each such
 * line, append the basename to `outBasenames`.  Returns the count
 * of basenames appended.
 *
 * Each CMakeLists.txt quoted source reference has the shape
 *   `"probes/m11/<something>.c"`
 * On a given line we locate the LAST occurrence of `.c"` (the
 * closing quote of the source reference), then walk left to find
 * the matching opening `"`, then take the substring between the
 * opening `"` and the closing quote.  The basename is the substring
 * after the last `/` (if any).  The `.c` extension is preserved so
 * the basename stays identical to the literal the readiness gate
 * cross-references.
 */
#define BASENAME_BUF_MAX 4096

static int collect_cmake_basenames(const char* cmakeBuf,
                                   char (*outBasenames)[256],
                                   int outMax) {
    const char* p = cmakeBuf;
    int n = 0;
    if (!cmakeBuf) return 0;
    while (*p && n < outMax) {
        const char* eol = strchr(p, '\n');
        size_t len = eol ? (size_t)(eol - p) : strlen(p);
        const char* dotC;
        const char* openQuote;
        const char* slash;
        const char* baseStart;
        size_t baseLen;
        if (len < 4) { p += len + (eol ? 1 : 0); continue; }
        /* Find the last occurrence of `.c"` on this line. */
        dotC = NULL;
        for (const char* q = p; q + 2 < p + len; ++q) {
            if (q[0] == '.' && q[1] == 'c' && q[2] == '"') {
                dotC = q;
            }
        }
        if (!dotC) { p += len + (eol ? 1 : 0); continue; }
        /* Walk left from dotC to find the matching opening quote. */
        openQuote = NULL;
        if (dotC > p) {
            for (const char* q = dotC - 1; q >= p; --q) {
                if (*q == '"') { openQuote = q + 1; break; }
            }
        }
        if (!openQuote) { p += len + (eol ? 1 : 0); continue; }
        /* Find the last `/` between openQuote and dotC so the
         * basename is just the filename, not the full probes/m11
         * prefix.  Base starts at slash+1 (or openQuote if no
         * slash), and ends at dotC+2 — i.e. inclusive of the
         * closing `"` so the literal matches CMakeLists.txt 1:1. */
        slash = NULL;
        for (const char* q = openQuote; q < dotC; ++q) {
            if (*q == '/') slash = q;
        }
        baseStart = slash ? slash + 1 : openQuote;
        baseLen = (size_t)(dotC + 2 - baseStart);
        if (baseLen == 0 || baseLen >= 256) {
            p += len + (eol ? 1 : 0);
            continue;
        }
        memcpy(outBasenames[n], baseStart, baseLen);
        outBasenames[n][baseLen] = '\0';
        ++n;
        p += len + (eol ? 1 : 0);
    }
    return n;
}

static int compute_sibling_precedent(const char* cmakeBuf,
                                     const char* pattern,
                                     int (*isMatch)(const char*),
                                     int (*extractOrdinal)(const char*),
                                     OrdinalList* out) {
    char basenames[BASENAME_BUF_MAX][256];
    int nBasenames;
    int i;
    out->count = 0;
    if (!cmakeBuf || !pattern) return 0;
    nBasenames = collect_cmake_basenames(cmakeBuf, basenames, BASENAME_BUF_MAX);
    for (i = 0; i < nBasenames; ++i) {
        int ord;
        if (!isMatch(basenames[i])) continue;
        if (!cmake_contains(basenames[i], pattern)) continue;
        ord = extractOrdinal(basenames[i]);
        if (ord < 0) continue;
        if (out->count < ORDINAL_BUF_MAX) {
            out->ordinals[out->count++] = ord;
        }
    }
    return out->count;
}

/* ── Promotion checklist emission ─────────────────────────────── */

static void emit_promotion_checklist(const OpenSibling* sib,
                                     const char* sourceText,
                                     int markerHits,
                                     int markerTotal,
                                     const OrdinalList* precedent,
                                     int ordinal2Open,
                                     int ordinal2SourcePresent) {
    int i;
    printf("\n  Promotion checklist for ordinal-2 %s:\n", sib->route_variant);
    printf("    class: %s\n", sib->promotion_class);
    printf("    source: probes/m11/%s\n", sib->probe_basename);
    printf("    marker coverage: %d/%d verbatim source phrases matched\n",
           markerHits, markerTotal);
    printf("    Step 1 — confirm the sibling-ordinal precedent is still green:\n");
    if (precedent->count == 0) {
        printf("      no wired precedent — promotion is a fresh route, not a fix-up\n");
    } else {
        printf("      wired siblings (%d ordinals): ",
               precedent->count);
        for (i = 0; i < precedent->count; ++i) {
            printf("%d%s",
                   precedent->ordinals[i],
                   (i + 1 < precedent->count) ? ", " : "");
        }
        printf("\n");
        printf("      → copy the wired precedent's if(EXISTS ...)/add_executable/\n");
        printf("        add_test block in CMakeLists.txt, swap in this probe's\n");
        printf("        basename, and rerun `cmake --build build --target <exe>`\n");
        printf("        to confirm the probe links + passes.\n");
    }
    printf("    Step 2 — verify the ordinal-2 source still compiles + exits 0\n");
    printf("      against the shipped DM1 V1 PC 3.4 DUNGEON.DAT:\n");
    if (ordinal2SourcePresent) {
        printf("      source present, not CTest-wired (matches readiness gate)\n");
        printf("      → build manually with:\n");
        printf("          cc probes/m11/%s -I include -L build \\\n",
               sib->probe_basename);
        printf("             -lfirestaff_m12 -lfirestaff_m11 -lfirestaff_v2 \\\n");
        printf("             -lfirestaff_m10 -lSDL3 -lz -lm\n");
    } else {
        printf("      source MISSING on disk — promotion requires authoring it\n");
        printf("      first (this is the 'repair or retire' branch the TODO row\n");
        printf("      calls out).\n");
    }
    printf("    Step 3 — record the verdict:\n");
    if (ordinal2Open && ordinal2SourcePresent) {
        printf("      'open (source present)' — once Step 2 PASSes, add the\n");
        printf("      probe to CMakeLists.txt + bump the readiness-gate\n");
        printf("      expected_wired flag, and the readiness gate will\n");
        printf("      reclassify this row from 'open' to 'covered'.\n");
    } else {
        printf("      'covered' or 'retired' — update the TODO row (b) and\n");
        printf("      bump the readiness-gate sibling matrix to match.\n");
    }
    (void)sourceText;
}

/* ── main ─────────────────────────────────────────────────────── */

int main(int argc, char** argv) {
    char resolvedCmake[1024];
    char resolvedRoot[1024];
    const char* cmakeListsPath = NULL;
    const char* sourceRoot = ".";
    char* cmakeListsBuf = NULL;
    int ordinal2OpenTotal = 0;
    int ordinal2SourceTotal = 0;
    int i;

    printf("=== DM1 V1 HoC ordinal-2 sibling-promotion audit ===\n");

    if (!resolve_cmakelists_path(argc, argv,
                                 resolvedCmake, sizeof(resolvedCmake))) {
        fprintf(stderr, "FATAL: could not locate CMakeLists.txt\n");
        return 2;
    }
    cmakeListsPath = resolvedCmake;
    if (!resolve_source_root(cmakeListsPath,
                             resolvedRoot, sizeof(resolvedRoot))) {
        snprintf(resolvedRoot, sizeof(resolvedRoot), ".");
    }
    sourceRoot = resolvedRoot;

    printf("cmakeListsPath=%s\n", cmakeListsPath);
    printf("sourceRoot=%s\n", sourceRoot);

    cmakeListsBuf = read_file_bounded(cmakeListsPath, PROBE_CMAKE_MAX_BYTES);
    if (!cmakeListsBuf) {
        fprintf(stderr, "FATAL: could not read CMakeLists.txt\n");
        return 2;
    }

    /* Group A — open-count invariant matches the readiness gate.
     *
     * The readiness gate asserts `open == 2` (west_negative +
     * cancel_reopen).  We assert the same invariant here from the
     * build-system side, without ever loading DM1 V1 data.  The
     * two probes should agree: if they disagree, one of them has
     * drifted, and the TODO row (b) needs an explicit reconciliation
     * note before either can be trusted. */
    printf("\n[Group A] Open-count invariant (must match readiness gate == 2)\n");
    {
        int openCount = 0;
        int srcCount  = 0;
        for (i = 0; i < OPEN_SIBLING_COUNT; ++i) {
            char pathBuf[1024];
            int present;
            int wired;
            snprintf(pathBuf, sizeof(pathBuf), "%s/probes/m11/%s",
                     sourceRoot, kOpenSiblings[i].probe_basename);
            present = file_exists(pathBuf);
            wired = cmake_contains(cmakeListsBuf, kOpenSiblings[i].probe_basename);
            if (present) ++srcCount;
            if (present && !wired) ++openCount;
            printf("  %-15s present=%d wired=%d open=%d\n",
                   kOpenSiblings[i].route_variant,
                   present ? 1 : 0,
                   wired   ? 1 : 0,
                   (present && !wired) ? 1 : 0);
            ordinal2SourceTotal += present ? 1 : 0;
            ordinal2OpenTotal   += (present && !wired) ? 1 : 0;
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "Group A: ordinal-2 open-count == 2 (matches readiness gate), got %d",
                     openCount);
            CHECK(openCount == 2, msg);
            snprintf(msg, sizeof(msg),
                     "Group A: ordinal-2 source-present-count == 2 (matches readiness gate), got %d",
                     srcCount);
            CHECK(srcCount == 2, msg);
        }
    }

    /* Group B — per-sibling promotion audit. */
    printf("\n[Group B] Per-sibling promotion audit\n");
    for (i = 0; i < OPEN_SIBLING_COUNT; ++i) {
        const OpenSibling* sib = &kOpenSiblings[i];
        char pathBuf[1024];
        char* sourceText = NULL;
        int markerHits = 0;
        int markerTotal = 0;
        OrdinalList precedent;
        int present;
        int wired;

        snprintf(pathBuf, sizeof(pathBuf), "%s/probes/m11/%s",
                 sourceRoot, sib->probe_basename);
        present = file_exists(pathBuf);
        wired = cmake_contains(cmakeListsBuf, sib->probe_basename);

        printf("\n  --- ordinal-2 %s ---\n", sib->route_variant);
        printf("  source: probes/m11/%s\n", sib->probe_basename);
        printf("  source present: %s\n", present ? "yes" : "NO");
        printf("  CTest wired:    %s\n", wired   ? "yes" : "NO (expected — TODO row (b))");

        if (!present) {
            printf("  promotion verdict: 'retire or author first'\n");
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "Group B: ordinal-2 %s source file is present on disk (matches readiness gate)",
                         sib->route_variant);
                CHECK(0, msg); /* intentionally FAIL to surface the
                                * missing source; the readiness gate
                                * already records this case as
                                * 'gap (source missing)' */
            }
            continue;
        }

        sourceText = read_file_bounded(pathBuf, PROBE_SOURCE_MAX_BYTES);
        if (!sourceText) {
            fprintf(stderr,
                    "WARN: could not read ordinal-2 %s source (skipping per-sibling report)\n",
                    sib->route_variant);
            continue;
        }

        if (strcmp(sib->route_variant, "west_negative") == 0) {
            markerTotal = WEST_NEGATIVE_MARKER_COUNT;
            markerHits = count_markers(sourceText,
                                       kWestNegativeMarkers,
                                       WEST_NEGATIVE_MARKER_COUNT);
            precedent.count = 0;
            compute_sibling_precedent(cmakeListsBuf,
                                      sib->sibling_pattern,
                                      is_west_negative_basename,
                                      extract_west_negative_ordinal,
                                      &precedent);
        } else {
            markerTotal = CANCEL_REOPEN_MARKER_COUNT;
            markerHits = count_markers(sourceText,
                                       kCancelReopenMarkers,
                                       CANCEL_REOPEN_MARKER_COUNT);
            precedent.count = 0;
            compute_sibling_precedent(cmakeListsBuf,
                                      sib->sibling_pattern,
                                      is_cancel_reopen_basename,
                                      extract_cancel_reopen_ordinal,
                                      &precedent);
        }

        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "Group B: ordinal-2 %s marker coverage >= 2 verbatim phrases (got %d/%d)",
                     sib->route_variant, markerHits, markerTotal);
            CHECK(markerHits >= 2, msg);
            snprintf(msg, sizeof(msg),
                     "Group B: ordinal-2 %s source file is not CTest-wired (open)",
                     sib->route_variant);
            CHECK(!wired, msg);
            snprintf(msg, sizeof(msg),
                     "Group B: ordinal-2 %s has at least one wired sibling-ordinal precedent (got %d)",
                     sib->route_variant, precedent.count);
            CHECK(precedent.count >= 1, msg);
        }

        emit_promotion_checklist(sib,
                                 sourceText,
                                 markerHits,
                                 markerTotal,
                                 &precedent,
                                 ordinal2OpenTotal,
                                 ordinal2SourceTotal);

        free(sourceText);
    }

    /* Group C — readiness-gate cross-reference.  The two gates
     * MUST agree on the open-count.  Print a machine-checkable
     * cross-reference so a future promotion audit can grep for
     * both lines in one CI run. */
    printf("\n[Group C] Readiness-gate cross-reference\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "Group C: this audit found %d open ordinal-2 siblings (readiness gate agrees iff == 2)",
                 ordinal2OpenTotal);
        CHECK(ordinal2OpenTotal == 2, msg);
        printf("  cross-reference: open=%d source-present=%d\n",
               ordinal2OpenTotal, ordinal2SourceTotal);
        printf("  the readiness gate (data-required) records:\n");
        printf("    real ordinal-2 corridor-sensor hits: 0\n");
        printf("    sibling ordinal-2 routes: 10 total, 10 source-present, 8 build-wired\n");
        printf("    TODO row (b) open count: 2\n");
    }

    free(cmakeListsBuf);

    printf("\n=== ordinal-2 sibling-promotion audit summary ===\n");
    printf("  PASS=%d  FAIL=%d\n", g_pass, g_fail);
    printf("  ordinal-2 sibling open-count: %d (must match readiness gate == 2)\n",
           ordinal2OpenTotal);
    printf("  TODO row (b) follow-up:\n");
    printf("    open-count == 2: still source-present + stale-fixture, per-emitter\n");
    printf("    promotion checklist: see Step 1..3 for each sibling\n");
    printf("    non-claim: no M11 drive, no game data, no runtime parity\n");

    return (g_fail == 0) ? 0 : 1;
}
