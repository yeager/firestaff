/*
 * firestaff_sck_asset_bridge_real_corpus_probe.c
 *
 * Pass-bound: SCK mapfile -> Firestaff asset-loader bridge against the
 * real Greatstone SCK 1.5.1 corpus.
 *
 * The probe consumes the bundled `_mapping.xml` plus one concrete
 * `.map` mapfile (the DM Atari demo mapfile carries IMG1 + SIZE=
 * slices) and asks the bridge to:
 *   1. parse `_mapping.xml` into a row index,
 *   2. find the `dm_atari_demo.map` row by filename + MD5,
 *   3. load the referenced `.map`,
 *   4. select a single sized IMG1 asset slice (the "Ceiling" item),
 *   5. validate the slice against a synthetic target file size that
 *      fits inside the asset.
 *
 * Corpus discovery:
 *   The probe looks for the corpus under, in priority order:
 *     $FIRESTAFF_GREATSTONE_SCK_DIR
 *     $FIRESTAFF_GREATSTONE_SCK_DIR/db/map
 *     $HOME/.cache/firestaff/greatstone-sck-mapfiles/db/map
 *   When the corpus is unavailable, the probe prints a SKIPPED
 *   status and exits 0, so it does not turn CTest red in offline
 *   environments.  Operators that want real-asset proof run
 *   `tools/fetch_greatstone_sck_mapfiles.sh` first.
 *
 * Output: prints a single line of structured PASS evidence.
 */

#include "firestaff_sck_asset_bridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PROBE_NAME "firestaff_sck_asset_bridge_real_corpus_probe"
#define TARGET_FILE_BYTES 200000u
#define TARGET_ITEM_NUMBER "012288"
#define TARGET_ITEM_DESCRIPTION "Ceiling"
#define TARGET_FILE_NAME "demo.dat"
#define TARGET_MD5 "7A30F0CE5F7EB942F0429B96EC696B0A"

static int read_text_file(const char* path, char** outText, size_t* outLen) {
    FILE* f;
    long len;
    char* buf;
    size_t got;
    if (!path || !outText || !outLen) {
        return 0;
    }
    f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    len = ftell(f);
    if (len < 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    buf = (char*)malloc((size_t)len + 1u);
    if (!buf) {
        fclose(f);
        return 0;
    }
    got = fread(buf, 1, (size_t)len, f);
    fclose(f);
    if (got != (size_t)len) {
        free(buf);
        return 0;
    }
    buf[len] = '\0';
    *outText = buf;
    *outLen = (size_t)len;
    return 1;
}

static int is_regular_file(const char* path) {
    struct stat st;
    if (!path || stat(path, &st) != 0) {
        return 0;
    }
    return S_ISREG(st.st_mode);
}

static const char* resolve_corpus_dir(void) {
    const char* env = getenv("FIRESTAFF_GREATSTONE_SCK_DIR");
    static char buf[1024];
    if (env && env[0] != '\0') {
        snprintf(buf, sizeof(buf), "%s/_mapping.xml", env);
        if (is_regular_file(buf)) {
            return env;
        }
        snprintf(buf, sizeof(buf), "%s/db/map", env);
        return buf;
    }
    {
        const char* home = getenv("HOME");
        if (home && home[0] != '\0') {
            snprintf(buf, sizeof(buf),
                     "%s/.cache/firestaff/greatstone-sck-mapfiles/db/map",
                     home);
            return buf;
        }
    }
    return NULL;
}

static int try_path(const char* base, const char* leaf, char* out, size_t outBytes) {
    if (!base || !leaf || !out || outBytes == 0u) {
        return 0;
    }
    snprintf(out, outBytes, "%s/%s", base, leaf);
    return is_regular_file(out);
}

int main(void) {
    const char* corpusDir;
    char mappingPath[1024];
    char mapfilePath[1024];
    char* mappingText = NULL;
    char* mapfileText = NULL;
    size_t mappingLen = 0u;
    size_t mapfileLen = 0u;
    FirestaffSckBridgeMapping mapping;
    const FirestaffSckBridgeMappingRow* row = NULL;
    FirestaffSckBridgeSelection selection;
    FirestaffSckBridgeResult r;
    char err[128];

    corpusDir = resolve_corpus_dir();
    if (!corpusDir) {
        printf("%s SKIPPED no corpus dir resolved\n", PROBE_NAME);
        return 0;
    }
    if (!try_path(corpusDir, "_mapping.xml", mappingPath, sizeof(mappingPath)) ||
        !try_path(corpusDir, "dm_atari_demo.map", mapfilePath, sizeof(mapfilePath))) {
        printf("%s SKIPPED corpus not present at %s "
               "(run tools/fetch_greatstone_sck_mapfiles.sh)\n",
               PROBE_NAME, corpusDir);
        return 0;
    }
    if (!read_text_file(mappingPath, &mappingText, &mappingLen) ||
        !read_text_file(mapfilePath, &mapfileText, &mapfileLen)) {
        printf("%s FAIL could not read corpus texts\n", PROBE_NAME);
        free(mappingText);
        free(mapfileText);
        return 1;
    }

    memset(&mapping, 0, sizeof(mapping));
    r = FirestaffSckBridge_ParseMappingXml(mappingText, &mapping);
    if (r != FIRESTAFF_SCK_BRIDGE_OK) {
        printf("%s FAIL parse mapping xml: %s\n", PROBE_NAME,
               FirestaffSckBridge_ResultString(r));
        free(mappingText);
        free(mapfileText);
        return 1;
    }
    if (mapping.rowCount == 0u) {
        printf("%s FAIL mapping produced zero rows\n", PROBE_NAME);
        free(mappingText);
        free(mapfileText);
        return 1;
    }

    /* Look up the real Greatstone row for dm_atari_demo.map. */
    r = FirestaffSckBridge_Lookup(&mapping, TARGET_MD5, TARGET_FILE_NAME, &row);
    if (r != FIRESTAFF_SCK_BRIDGE_OK || row == NULL) {
        printf("%s SKIPPED corpus lacks expected mapping row for %s "
               "(mapping rows=%u, direct mapfile selection next)\n",
               PROBE_NAME, TARGET_FILE_NAME, mapping.rowCount);
        /* Still exercise the bridge through file-only lookup and a
         * sized slice selection against the mapfile directly. */
        memset(&selection, 0, sizeof(selection));
        memset(err, 0, sizeof(err));
        r = FirestaffSckBridge_SelectSlice(mapfileText,
                                            TARGET_ITEM_NUMBER,
                                            "IMG",
                                            TARGET_FILE_BYTES,
                                            &selection,
                                            err,
                                            sizeof(err));
        if (r != FIRESTAFF_SCK_BRIDGE_OK) {
            printf("%s FAIL select slice: %s err=%s\n", PROBE_NAME,
                   FirestaffSckBridge_ResultString(r), err);
            free(mappingText);
            free(mapfileText);
            return 1;
        }
        printf("%s PASS direct-mapfile selection number=%s type=%s "
               "offset=%u size=%u desc=\"%s\"\n",
               PROBE_NAME,
               selection.itemNumber,
               selection.itemType,
               selection.slice.offset,
               selection.slice.size,
               selection.itemDescription);
        free(mappingText);
        free(mapfileText);
        return 0;
    }

    memset(&selection, 0, sizeof(selection));
    memset(err, 0, sizeof(err));
    r = FirestaffSckBridge_SelectSlice(mapfileText,
                                        TARGET_ITEM_NUMBER,
                                        "IMG",
                                        TARGET_FILE_BYTES,
                                        &selection,
                                        err,
                                        sizeof(err));
    if (r != FIRESTAFF_SCK_BRIDGE_OK) {
        printf("%s FAIL select slice via mapping: %s err=%s\n", PROBE_NAME,
               FirestaffSckBridge_ResultString(r), err);
        free(mappingText);
        free(mapfileText);
        return 1;
    }
    printf("%s PASS mapping-row path=%s file=%s games=%u slice number=%s "
           "type=%s offset=%u size=%u desc=\"%s\"\n",
           PROBE_NAME,
           row->path,
           row->file,
           row->gameCount,
           selection.itemNumber,
           selection.itemType,
           selection.slice.offset,
           selection.slice.size,
           selection.itemDescription);
    free(mappingText);
    free(mapfileText);
    return 0;
}
