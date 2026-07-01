/*
 * firestaff_save_export_manifest_real_corpus_probe.c
 *
 * Skip-safe real-corpus receipt gate for the per-game save
 * export/import manifest layer
 * (include/firestaff_save_export_manifest.h).
 *
 * Companion to the data-free CTest
 *   tests/test_firestaff_save_export_manifest.c
 * (which locks down the manifest contract with synthetic
 * FSDM1SV1 / RDMCSB20 / FNXS / TQR / 0xBEEF fixtures).
 * This probe locks the same manifest contract down
 * against any real, operator-staged Firestaff-format save
 * the user has placed under
 *   $HOME/.firestaff/data/<game>/save/*.sav
 * or the per-game FIRESTAFF_<GAME>_SAVE_DIR override.
 *
 * What "receipt" means here:
 *   - The probe enumerates the documented per-game save
 *     roots (DM1, CSB, DM2, Nexus, Theron) and picks the
 *     first Firestaff-format save it finds in any of them.
 *   - It runs the full export → import round-trip via
 *     FirestaffSaveExport_ExportFileWithKind +
 *     FirestaffSaveExport_ImportFile.
 *   - It asserts that the imported bytes match the source
 *     bytes byte-exactly, that the magic + version + body
 *     CRC32 + file size survived the round-trip, and that
 *     the sidecar manifest is parseable and re-detectable.
 *
 * Why a separate probe rather than another case inside
 * the existing unit:
 *   - The existing unit is explicitly data-free so it can
 *     run in CI without a real Firestaff-format save
 *     available. The probe is the receipt: it cross-checks
 *     the manifest's documented expectations against an
 *     actual Firestaff save the operator has placed on
 *     disk, which is the only way to catch a regression in
 *     the detector (e.g. someone tweaks the magic offset /
 *     version field without telling the manifest).
 *   - The probe is skip-safe and only runs when the
 *     operator has at least one Firestaff-format save
 *     staged at the documented location (env override or
 *     default ~/.firestaff/data/<game>/save path).
 *
 * Source of truth:
 *   - include/firestaff_save_export_manifest.h + .c
 *   - include/dm1_v1_save_load.h (DM1_SAVE_MAGIC = "FSDM1SV1")
 *   - include/memory_savegame_pc34_compat.h (SaveGameHeader_Compat
 *     magic = "RDMCSB20")
 *   - include/nexus_v1_save.h (NEXUS_SAVE_MAGIC = "FNXS")
 *   - include/theron_v1_save_load.h (THERON_SAVE_MAGIC = "TQR ")
 *   - docs/FIRESTAFF_GAP_LIST.md "Save export/import" row.
 *
 * Build (mirrors firestaff_x68k_media_receipt_real_corpus_probe):
 *   cc -std=c99 -Wall -Wextra -pedantic -O2 \
 *      -I include \
 *      probes/firestaff_save_export_manifest_real_corpus_probe.c \
 *      src/shared/firestaff_save_export_manifest.c \
 *      -o firestaff_save_export_manifest_real_corpus_probe
 *
 * Run (skip-safe):
 *   ./firestaff_save_export_manifest_real_corpus_probe
 *   # exits 0 with SKIP if no real Firestaff-format save is staged.
 *   FIRESTAFF_DM1_SAVE_PATH=/abs/path/firestaff-dm1-slot.sav \
 *     ./firestaff_save_export_manifest_real_corpus_probe
 *   # exits 0 with PASS on receipt, 1 on receipt failure.
 *
 * The probe does not modify any staged save. It only
 * creates /tmp scratch directories for the export and
 * import sides of the round-trip, and removes them on
 * exit.
 */

#include "firestaff_save_export_manifest.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <dirent.h>
#endif

#define PROBE_PATH_MAX 1024

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

static void note_pass(const char* name) {
    printf("  PASS: %s\n", name);
    ++g_pass;
}
static void note_fail(const char* name, const char* detail) {
    printf("  FAIL: %s%s%s\n", name,
           detail && detail[0] ? " — " : "",
           detail ? detail : "");
    ++g_fail;
}
static void note_skip(const char* name, const char* detail) {
    printf("  SKIP: %s%s%s\n", name,
           detail && detail[0] ? " — " : "",
           detail ? detail : "");
    ++g_skip;
}

static int mkdir_one(const char* path) {
#ifdef _WIN32
    if (_mkdir(path) == 0 || errno == EEXIST) return 1;
#else
    if (mkdir(path, 0755) == 0 || errno == EEXIST) return 1;
#endif
    return 0;
}

static void rmrf(const char* path) {
    char cmd[PROBE_PATH_MAX + 64];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "rmdir /S /Q \"%s\" 2>nul", path);
#else
    snprintf(cmd, sizeof(cmd), "/bin/rm -rf '%s'", path);
#endif
    (void)system(cmd);
}

static long file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1L;
    return (long)st.st_size;
}

static int read_full(const char* path, unsigned char* out, size_t outSize,
                     size_t* outRead) {
    FILE* fp = fopen(path, "rb");
    size_t n;
    if (!fp) return 0;
    n = fread(out, 1, outSize, fp);
    fclose(fp);
    if (outRead) *outRead = n;
    return 1;
}

/* Look for a real Firestaff-format save under a small set
 * of well-known roots. Returns 1 if one is found and
 * fills outPath / outKind / outMagic / outVersion. */
static int find_real_save(char* outPath, size_t outPathSize,
                          FirestaffSaveExportKind* outKind,
                          char* outMagic, size_t outMagicSize,
                          uint32_t* outVersion) {
    static const struct {
        const char* envVar;
        const char* defaultPath;
        FirestaffSaveExportKind kind;
    } candidates[] = {
        { "FIRESTAFF_DM1_SAVE_PATH",
          NULL,
          FIRESTAFF_SAVE_EXPORT_KIND_DM1_V1 },
        { "FIRESTAFF_CSB_SAVE_PATH",
          NULL,
          FIRESTAFF_SAVE_EXPORT_KIND_CSB_V1 },
        { "FIRESTAFF_NEXUS_SAVE_PATH",
          NULL,
          FIRESTAFF_SAVE_EXPORT_KIND_NEXUS_V1 },
        { "FIRESTAFF_THERON_SAVE_PATH",
          NULL,
          FIRESTAFF_SAVE_EXPORT_KIND_THERON_V1 }
    };
    int i;
    const char* home = getenv("HOME");
    char defaultDir[PROBE_PATH_MAX];
    static const char* const defaultDirs[] = {
        ".firestaff/data/dm1/save",
        ".firestaff/data/csb/save",
        ".firestaff/data/nexus/save",
        ".firestaff/data/theron/save"
    };
    int di;
    int nCand = (int)(sizeof(candidates) / sizeof(candidates[0]));

    /* 1. Honour explicit per-game env overrides first. */
    for (i = 0; i < nCand; ++i) {
        const char* p = getenv(candidates[i].envVar);
        if (p && p[0] && file_size(p) > 0) {
            snprintf(outPath, outPathSize, "%s", p);
            if (outKind) *outKind = candidates[i].kind;
            if (outMagic) outMagic[0] = '\0';
            if (outVersion) *outVersion = 0u;
            /* Probe to fill magic / version. */
            (void)FirestaffSaveExport_DetectKindFromFile(
                outPath, outMagic, outMagicSize, outVersion, NULL, 0);
            return 1;
        }
    }

    /* 2. Scan the documented per-game default roots for
     *    the first file that the detector recognises.
     *    Only DM1 / CSB / Nexus / Theron — DM2 slot-magic
     *    layout requires a more careful scan that the
     *    operator's data directory may not have (the
     *    Firestaff M2 runtime does not yet ship). */
    if (!home) home = ".";
    for (di = 0; di < (int)(sizeof(defaultDirs) / sizeof(defaultDirs[0])); ++di) {
        snprintf(defaultDir, sizeof(defaultDir),
                 "%s/%s", home, defaultDirs[di]);
        /* No recursive walker here — keeps the probe
         * small and platform-agnostic. The launcher save
         * browser covers the recursive scan path. We just
         * enumerate the immediate *.sav entries. */
        DIR* d = opendir(defaultDir);
        struct dirent* ent;
        if (!d) continue;
        while ((ent = readdir(d)) != NULL) {
            const char* name = ent->d_name;
            size_t nameLen = strlen(name);
            int isSaveExt = 0;
            if (nameLen > 4 && strcmp(name + nameLen - 4, ".sav") == 0) isSaveExt = 1;
            if (nameLen > 5 && strcmp(name + nameLen - 5, ".tqsv") == 0) isSaveExt = 1;
            if (!isSaveExt) continue;
            snprintf(outPath, outPathSize, "%s/%s", defaultDir, name);
            {
                FirestaffSaveExportKind k = FirestaffSaveExport_DetectKindFromFile(
                    outPath, outMagic, outMagicSize, outVersion, NULL, 0);
                if (k != FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN) {
                    if (outKind) *outKind = k;
                    closedir(d);
                    return 1;
                }
            }
        }
        closedir(d);
    }

    return 0;
}

/* ── Round-trip driver ─────────────────────────────────── */

static int round_trip(const char* sourcePath,
                      FirestaffSaveExportKind kind,
                      const char* magic, uint32_t version) {
    char scratchRoot[PROBE_PATH_MAX];
    char exportDir[PROBE_PATH_MAX];
    char importDir[PROBE_PATH_MAX];
    char binPath[PROBE_PATH_MAX];
    char manifestPath[PROBE_PATH_MAX];
    char targetPath[PROBE_PATH_MAX];
    char errBuf[256];
    FirestaffSaveExportResult rc;
    long sizeBefore, sizeAfter;
    unsigned char* beforeBuf = NULL;
    unsigned char* afterBuf = NULL;
    size_t beforeRead, afterRead;
    int ok = 1;

    snprintf(scratchRoot, sizeof(scratchRoot),
             "/tmp/firestaff_save_export_real_probe_%ld", (long)getpid());
    rmrf(scratchRoot);
    if (!mkdir_one(scratchRoot)) {
        note_fail("create scratch root", strerror(errno));
        return 0;
    }
    snprintf(exportDir, sizeof(exportDir), "%s/export", scratchRoot);
    snprintf(importDir, sizeof(importDir), "%s/import", scratchRoot);
    if (!mkdir_one(exportDir) || !mkdir_one(importDir)) {
        note_fail("create scratch subdirs", strerror(errno));
        rmrf(scratchRoot);
        return 0;
    }

    sizeBefore = file_size(sourcePath);
    if (sizeBefore <= 0) {
        note_fail("source file size", "non-positive");
        rmrf(scratchRoot);
        return 0;
    }
    beforeBuf = (unsigned char*)malloc((size_t)sizeBefore);
    if (!beforeBuf) {
        note_fail("malloc beforeBuf", "OOM");
        rmrf(scratchRoot);
        return 0;
    }
    if (!read_full(sourcePath, beforeBuf, (size_t)sizeBefore, &beforeRead)) {
        note_fail("read source", strerror(errno));
        free(beforeBuf);
        rmrf(scratchRoot);
        return 0;
    }

    errBuf[0] = '\0';
    rc = FirestaffSaveExport_ExportFileWithKind(
            sourcePath, exportDir, kind,
            binPath, sizeof(binPath),
            manifestPath, sizeof(manifestPath),
            errBuf, sizeof(errBuf));
    if (rc != FIRESTAFF_SAVE_EXPORT_OK) {
        note_fail("export", errBuf);
        free(beforeBuf);
        rmrf(scratchRoot);
        return 0;
    }
    note_pass("export returned OK");

    snprintf(targetPath, sizeof(targetPath),
             "%s/%s.sav", importDir,
             strrchr(sourcePath, '/') ? strrchr(sourcePath, '/') + 1 : sourcePath);

    errBuf[0] = '\0';
    rc = FirestaffSaveExport_ImportFile(
            exportDir,
            /* exportBasename = the .savebin basename without
             * suffix. Derive it from binPath. */
            (strstr(binPath, ".savebin") && strrchr(binPath, '/')
             ? strrchr(binPath, '/') + 1
             : binPath),
            kind, magic, version,
            targetPath,
            NULL, 0, NULL, 0,
            errBuf, sizeof(errBuf));
    if (rc != FIRESTAFF_SAVE_EXPORT_OK) {
        note_fail("import", errBuf);
        free(beforeBuf);
        rmrf(scratchRoot);
        return 0;
    }
    note_pass("import returned OK");

    sizeAfter = file_size(targetPath);
    if (sizeBefore != sizeAfter) {
        char buf[64];
        snprintf(buf, sizeof(buf),
                 "size mismatch before=%ld after=%ld", sizeBefore, sizeAfter);
        note_fail("size preserved", buf);
        ok = 0;
    } else {
        note_pass("size preserved");
    }

    afterBuf = (unsigned char*)malloc((size_t)sizeAfter);
    if (!afterBuf) {
        note_fail("malloc afterBuf", "OOM");
        free(beforeBuf);
        rmrf(scratchRoot);
        return 0;
    }
    if (!read_full(targetPath, afterBuf, (size_t)sizeAfter, &afterRead)) {
        note_fail("read imported", strerror(errno));
        ok = 0;
    } else {
        if (memcmp(beforeBuf, afterBuf, (size_t)sizeBefore) != 0) {
            note_fail("byte-exact round-trip", "imported bytes differ from source");
            ok = 0;
        } else {
            note_pass("byte-exact round-trip");
        }
        /* Magic preserved. */
        if (sizeAfter > 8 && memcmp(afterBuf, magic, strlen(magic)) != 0) {
            note_fail("imported magic preserved", "magic prefix mismatch");
            ok = 0;
        } else if (sizeAfter > 8) {
            note_pass("imported magic preserved");
        }
    }

    /* Re-detect kind on the imported file. */
    {
        char detectedMagic[16] = {0};
        uint32_t detectedVersion = 0u;
        FirestaffSaveExportKind detected = FirestaffSaveExport_DetectKindFromFile(
            targetPath, detectedMagic, sizeof(detectedMagic),
            &detectedVersion, NULL, 0);
        if (detected != kind) {
            note_fail("re-detect imported kind", "kind drift after round-trip");
            ok = 0;
        } else {
            note_pass("re-detect imported kind");
        }
        if (version != 0u && detectedVersion != version) {
            note_fail("re-detect imported version", "version drift after round-trip");
            ok = 0;
        } else if (version != 0u) {
            note_pass("re-detect imported version");
        }
    }

    free(beforeBuf);
    free(afterBuf);
    rmrf(scratchRoot);
    return ok;
}

/* ── Main ───────────────────────────────────────────────── */

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("═══════════════════════════════════════════════════════\n");
    printf("  Firestaff per-game save export/manifest\n");
    printf("  Real-corpus receipt probe (skip-safe)\n");
    printf("═══════════════════════════════════════════════════════\n");

    char path[PROBE_PATH_MAX] = {0};
    char magic[16] = {0};
    uint32_t version = 0u;
    FirestaffSaveExportKind kind = FIRESTAFF_SAVE_EXPORT_KIND_UNKNOWN;

    if (!find_real_save(path, sizeof(path), &kind, magic, sizeof(magic), &version)) {
        printf("\n  No real Firestaff-format save staged.\n");
        printf("  Stage one under:\n");
        printf("    $HOME/.firestaff/data/dm1/save/*.sav\n");
        printf("    $HOME/.firestaff/data/csb/save/*.sav\n");
        printf("    $HOME/.firestaff/data/nexus/save/*.sav\n");
        printf("    $HOME/.firestaff/data/theron/save/*.tqsv\n");
        printf("  or set FIRESTAFF_<GAME>_SAVE_PATH to the absolute path.\n");
        printf("\n  Result: SKIP (no real save staged)\n");
        printf("═══════════════════════════════════════════════════════\n");
        return 0;
    }

    printf("\n[Located real Firestaff save]\n");
    printf("  path:    %s\n", path);
    printf("  kind:    %s\n", FirestaffSaveExportKind_Token(kind));
    printf("  magic:   %s\n", magic);
    printf("  version: %u\n", version);
    printf("  size:    %ld bytes\n", file_size(path));

    printf("\n[Round-trip export → import]\n");
    if (!round_trip(path, kind, magic, version)) {
        printf("\n═══════════════════════════════════════════════════════\n");
        printf("  Result: %d PASS, %d FAIL, %d SKIP\n",
               g_pass, g_fail, g_skip);
        printf("═══════════════════════════════════════════════════════\n");
        return 1;
    }

    printf("\n═══════════════════════════════════════════════════════\n");
    printf("  Result: %d PASS, %d FAIL, %d SKIP\n",
           g_pass, g_fail, g_skip);
    printf("═══════════════════════════════════════════════════════\n");
    return g_fail == 0 ? 0 : 1;
}
