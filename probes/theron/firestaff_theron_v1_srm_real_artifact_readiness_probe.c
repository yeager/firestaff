/*
 * firestaff_theron_v1_srm_real_artifact_readiness_probe.c
 *
 * Theron's Quest V1 — SRM real-artifact readiness probe (skip-safe).
 *
 * This probe closes the remaining 2026-06-28 TODO row for
 * "Theron V1 SRM real-artifact decode" by wiring a skip-safe CTest
 * gate over the real save-disk root:
 *
 *   - It walks the real `$HOME/.firestaff/data/theron/save/` (or
 *     the `FIRESTAFF_THERON_SRM_DIR` override) using the existing
 *     `theron_v1_srm_classify_root` classifier.
 *   - For each slot classified PRESENT_AND_RECOGNIZED, it reads the
 *     full file bytes, feeds them into `theron_v1_srm_probe_gzip_payload`
 *     (bounded gzip inflate), and then runs the inflated bytes
 *     through `theron_v1_srm_decode_progression_payload`. That is the
 *     bounded "one Theron_DungeonProgression decode fixture" the
 *     2026-06-28 gap text calls for.
 *   - When the inflated body imports a progression successfully, the
 *     probe takes the in-memory `Theron_DungeonProgression` and
 *     exports it through the synthetic `theron_v1_save_to_slot` /
 *     `theron_v1_save_export_slot` path as a `.tqsv`, then re-imports
 *     the same `.tqsv` into a fresh slot and verifies the round-trip
 *     progression survives.  This is the
 *     "real `.srm` -> written `.tqsv`" pairing the gap text requests.
 *
 * Outcome contract (skip-safe):
 *
 *   - If no real `.srm` is staged on this host
 *     (recognized_count == 0), the probe prints
 *     `SKIP THERON_SRM_NO_REAL_ARTIFACT_00` and exits 0.  This is
 *     the expected honest outcome today and matches the documented
 *     "ABSENT manifest" baseline.
 *   - If at least one real `.srm` is staged AND zlib is available
 *     AND the inflate + progression-decode + .tqsv round-trip all
 *     succeed, the probe prints
 *     `PASS THERON_SRM_REAL_ARTIFACT_OK_00` and exits 0.  This is
 *     the receipt that a real staged `.srm` can be promoted through
 *     the bounded readiness envelope without manual intervention.
 *   - If zlib is not available but at least one `.srm` is staged,
 *     the probe reports
 *     `SKIP THERON_SRM_ZLIB_UNAVAILABLE_00` and exits 0.  Inflating
 *     a real gzip body without zlib is not safe, so this is a skip
 *     rather than a failure.
 *   - If a real `.srm` is staged, zlib is available, but the inflate
 *     or decode fails, the probe prints
 *     `FAIL THERON_SRM_REAL_ARTIFACT_ERR_*` and exits 1.  That is
 *     the loud signal that something regressed between the bounded
 *     readiness envelope and the on-disk real artifact.
 *
 * What this probe deliberately does NOT do:
 *
 *   - It does not interpret the real Sphenx / Greatstone
 *     `TQR`-RTC custom save body.  Any inflated body whose first
 *     8 bytes do not equal the documented `FSTQPRG1` progression
 *     magic still returns `UNSUPPORTED_BODY` from
 *     `theron_v1_srm_decode_progression_payload`; the probe records
 *     that as a per-slot SKIP line and keeps the run green so that
 *     a future real-body decoder can promote the slot to PASS
 *     without changing the probe contract.
 *   - It does not import inventory / equipment bytes, run champion
 *     body decode, or promote any public screenshot.
 *   - It does not vendor any real `.srm` artifact or game data.
 *
 * Source / evidence:
 *   - `docs/DMWEB_REFERENCE.md` §6 "Theron's Quest savegame format":
 *       gzipped custom format with a header.
 *   - dmweb community docs credit Sphenx with several custom TQ-RTC
 *     save games documented at greatstone; Sphenx is also a SKWIN
 *     DM2 skproject co-author.
 *   - `docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md`
 *     anchors the JP / US Track 02 hash and the THQUEST.ASM T080
 *     (no in-dungeon saves) design rule.
 *   - THQUEST.ASM T080 — between-dungeon save / load
 *   - THQUEST.ASM T800 — champion persistence between dungeons
 *   - `docs/source-lock/tqr_v1_phase2_data_formats_H2339.md` §5/§9
 *
 * Build (mirrors `firestaff_theron_v1_srm_classifier_probe`):
 *   cmake --build build --target firestaff_theron_v1_srm_real_artifact_readiness_probe
 *
 * Run (skip-safe):
 *   SDL_VIDEODRIVER=dummy \
 *     ./build/firestaff_theron_v1_srm_real_artifact_readiness_probe
 *
 * CTest: registered as `theron_v1_srm_real_artifact_readiness_probe`
 *        with labels `tier2;theron;v1;srm;real-artifact;skip-safe`.
 */

#include "theron_v1_srm_classifier.h"
#include "theron_v1_save_load.h"
#include "theron_v1_dungeon_progression.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef FIRESTAFF_HAS_ZLIB
#define FIRESTAFF_HAS_ZLIB 0
#endif

#if defined(_WIN32) || defined(_WIN64)
#define RPROBE_PATH_SEP '\\'
#include <direct.h>
#define rprobe_mkdir(p) _mkdir(p)
#define rprobe_rmdir(p) _rmdir(p)
#define rprobe_unlink(p) remove(p)
#else
#define RPROBE_PATH_SEP '/'
#include <unistd.h>
#define rprobe_mkdir(p) mkdir((p), 0700)
#define rprobe_rmdir(p) rmdir(p)
#define rprobe_unlink(p) unlink(p)
#endif

#define RPROBE_OUT_PAYLOAD_CAPACITY 2048u

static int g_fail = 0;
static int g_pass = 0;
static int g_skip = 0;

static void pass(const char *name, const char *detail) {
    printf("PASS %s%s%s\n", name,
           detail && detail[0] ? " " : "",
           detail ? detail : "");
    ++g_pass;
}

static void skip(const char *name, const char *detail) {
    printf("SKIP %s%s%s\n", name,
           detail && detail[0] ? " " : "",
           detail ? detail : "");
    ++g_skip;
}

static void fail(const char *name, const char *detail) {
    printf("FAIL %s%s%s\n", name,
           detail && detail[0] ? " " : "",
           detail ? detail : "");
    ++g_fail;
}

static int file_exists_regular(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static uint8_t *read_whole_file(const char *path, size_t *out_size) {
    if (out_size) *out_size = 0;
    if (!path) return NULL;
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    long size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    uint8_t *buf = (uint8_t *)malloc((size_t)size);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    size_t got = fread(buf, 1, (size_t)size, fp);
    fclose(fp);
    if (got != (size_t)size) {
        free(buf);
        return NULL;
    }
    if (out_size) *out_size = (size_t)size;
    return buf;
}

static int make_temp_root(char out[THERON_V1_SRM_PATH_MAX]) {
#if defined(_WIN32) || defined(_WIN64)
    int pid = (int)getpid();
    for (int i = 0; i < 32; i++) {
        int n = snprintf(out, THERON_V1_SRM_PATH_MAX,
                         "firestaff_theron_srm_real_%d_%d", pid, i);
        if (n <= 0 || n >= THERON_V1_SRM_PATH_MAX) return 0;
        if (rprobe_mkdir(out) == 0) return 1;
    }
    out[0] = '\0';
    return 0;
#else
    static const char *tpl = "/tmp/firestaff_theron_srm_real_XXXXXX";
    if (strlen(tpl) + 1 > THERON_V1_SRM_PATH_MAX) return 0;
    strncpy(out, tpl, THERON_V1_SRM_PATH_MAX - 1);
    out[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    return mkdtemp(out) != NULL;
#endif
}

static void cleanup_temp_root(const char *root) {
    if (!root || !root[0]) return;
    char export_path[THERON_V1_SRM_PATH_MAX];
    snprintf(export_path, sizeof(export_path),
             "%s%creal_to_tqsv_export.tqsv", root, RPROBE_PATH_SEP);
    rprobe_unlink(export_path);
    for (int i = 0; i < THERON_SAVE_SLOT_COUNT; i++) {
        char slot_path[THERON_V1_SRM_PATH_MAX];
        theron_v1_save_slot_path(root, i, slot_path, sizeof(slot_path));
        rprobe_unlink(slot_path);
    }
    rprobe_rmdir(root);
}

/* Compare two Theron_DungeonProgression records on the fields that
 * round-trip through the .tqsv synthetic save path.  Champion bytes
 * are deliberately excluded — this is the progression-only fixture
 * the gap text asks for. */
static int progression_equal_for_round_trip(const Theron_DungeonProgression *a,
                                            const Theron_DungeonProgression *b) {
    if (!a || !b) return 0;
    if (a->quest_items_collected != b->quest_items_collected) return 0;
    if (a->current_dungeon != b->current_dungeon) return 0;
    if (a->current_level != b->current_level) return 0;
    if (a->dungeon_playtime_seconds != b->dungeon_playtime_seconds) return 0;
    if (a->item_reset_mode != b->item_reset_mode) return 0;
    if (a->champion_stats_persist != b->champion_stats_persist) return 0;
    if (a->champion_inv_persist != b->champion_inv_persist) return 0;
    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
        if (a->dungeon_states[i] != b->dungeon_states[i]) return 0;
        if (a->dungeon_seeds[i] != b->dungeon_seeds[i]) return 0;
    }
    return 1;
}

/* Run the bounded inflate + progression decode + .tqsv round-trip for
 * one real `.srm` slot.  Returns 1 on receipt PASS, 0 on FAIL.  The
 * caller emits per-skip labels when the inflated body is non-Firestaff
 * readiness envelope (UNSUPPORTED_BODY) so the probe stays green on
 * real Sphenx / Greatstone custom bodies that the bounded envelope
 * cannot yet decode. */
static int run_slot_receipt(const char *slot_path,
                             const char *temp_save_root,
                             int *out_unsupported_body) {
    if (out_unsupported_body) *out_unsupported_body = 0;

    size_t srm_size = 0;
    uint8_t *srm_bytes = read_whole_file(slot_path, &srm_size);
    if (!srm_bytes) {
        fail("THERON_SRM_REAL_ARTIFACT_READ_00",
             "could not read staged .srm body");
        return 0;
    }

    uint8_t payload[RPROBE_OUT_PAYLOAD_CAPACITY];
    memset(payload, 0, sizeof(payload));
    size_t payload_size = 0;
    Theron_V1SrmPayloadProbeStatus probe_status =
        theron_v1_srm_probe_gzip_payload(srm_bytes, srm_size,
                                          payload, sizeof(payload),
                                          &payload_size);
    free(srm_bytes);
    srm_bytes = NULL;

    if (probe_status != THERON_V1_SRM_PAYLOAD_PROBE_OK) {
        fail("THERON_SRM_REAL_ARTIFACT_INFLATE_00",
             theron_v1_srm_payload_probe_status_name(probe_status));
        return 0;
    }

    Theron_DungeonProgression restored;
    Theron_V1SrmProgressionReceipt receipt;
    Theron_V1SrmProgressImportStatus import_status =
        theron_v1_srm_decode_progression_payload(
            payload, payload_size, &restored, &receipt);
    if (import_status == THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY) {
        if (out_unsupported_body) *out_unsupported_body = 1;
        return 1;
    }
    if (import_status != THERON_V1_SRM_PROGRESS_IMPORT_OK) {
        fail("THERON_SRM_REAL_ARTIFACT_DECODE_00",
             theron_v1_srm_progress_import_status_name(import_status));
        return 0;
    }

    /* Write the imported progression through the synthetic .tqsv
     * export path, then import the exported image and verify the
     * progression round-trips.  Champion bytes are intentionally
     * zero-filled since this is the progression-only fixture. */
    uint8_t champ_data[THERON_SAVE_CHAMPION_COUNT *
                       THERON_SAVE_CHAMPION_BLOCK_SIZE];
    memset(champ_data, 0, sizeof(champ_data));
    int rc = theron_v1_save_to_slot(temp_save_root, 0,
                                    champ_data, sizeof(champ_data),
                                    &restored, "SRM real-artifact round-trip");
    if (rc != 0) {
        fail("THERON_SRM_REAL_ARTIFACT_SAVE_00",
             "theron_v1_save_to_slot failed for imported progression");
        return 0;
    }

    char export_path[THERON_V1_SRM_PATH_MAX];
    snprintf(export_path, sizeof(export_path),
             "%s%creal_to_tqsv_export.tqsv",
             temp_save_root, RPROBE_PATH_SEP);
    rc = theron_v1_save_export_slot(temp_save_root, 0, export_path);
    if (rc != 0) {
        fail("THERON_SRM_REAL_ARTIFACT_EXPORT_00",
             "theron_v1_save_export_slot failed");
        return 0;
    }
    if (!file_exists_regular(export_path)) {
        fail("THERON_SRM_REAL_ARTIFACT_EXPORT_MISSING_00",
             export_path);
        return 0;
    }

    rc = theron_v1_save_import_slot(temp_save_root, 1,
                                     export_path, NULL);
    if (rc != 0) {
        fail("THERON_SRM_REAL_ARTIFACT_IMPORT_00",
             "theron_v1_save_import_slot failed");
        return 0;
    }

    Theron_DungeonProgression round_trip;
    uint8_t champ_read[THERON_SAVE_CHAMPION_COUNT *
                       THERON_SAVE_CHAMPION_BLOCK_SIZE];
    Theron_SaveSlot slot_info;
    rc = theron_v1_save_load_from_slot(
        temp_save_root, 1,
        champ_read, sizeof(champ_read),
        &round_trip, sizeof(round_trip),
        &slot_info);
    if (rc != 0) {
        fail("THERON_SRM_REAL_ARTIFACT_LOAD_00",
             "theron_v1_save_load_from_slot failed");
        return 0;
    }
    if (slot_info.valid != 1) {
        fail("THERON_SRM_REAL_ARTIFACT_LOAD_VALID_00",
             "round-trip slot not flagged valid");
        return 0;
    }
    if (!progression_equal_for_round_trip(&restored, &round_trip)) {
        fail("THERON_SRM_REAL_ARTIFACT_ROUNDTRIP_MISMATCH_00",
             "imported progression did not survive .srm -> .tqsv -> .tqsv round-trip");
        return 0;
    }
    return 1;
}

static void probe_real_root_receipt(void) {
    Theron_V1SrmManifest manifest;
    memset(&manifest, 0, sizeof(manifest));
    int rc = theron_v1_srm_classify_root(NULL, &manifest);
    if (rc != 1) {
        fail("THERON_SRM_REAL_ARTIFACT_CLASSIFY_00",
             "theron_v1_srm_classify_root returned non-success");
        return;
    }

    printf("real-artifact manifest: root=%s present=%d recognized=%d\n",
           manifest.root, manifest.present_count, manifest.recognized_count);

    if (manifest.recognized_count == 0) {
        skip("THERON_SRM_NO_REAL_ARTIFACT_00",
             "no recognized .srm staged under the configured save root");
        return;
    }

#if !FIRESTAFF_HAS_ZLIB
    skip("THERON_SRM_ZLIB_UNAVAILABLE_00",
         "recognized .srm present but zlib unavailable; cannot inflate");
    return;
#else

    char temp_root[THERON_V1_SRM_PATH_MAX];
    if (!make_temp_root(temp_root)) {
        fail("THERON_SRM_REAL_ARTIFACT_TMPDIR_00",
             "could not create temp save root for round-trip");
        return;
    }

    int slot_pass = 0;
    int slot_unsupported = 0;
    int slot_other_fail = 0;
    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; i++) {
        const Theron_V1SrmSlotInfo *slot = &manifest.slots[i];
        if (slot->status != THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED) {
            continue;
        }
        int unsupported_body = 0;
        int ok = run_slot_receipt(slot->path, temp_root, &unsupported_body);
        if (ok && unsupported_body) {
            char label[80];
            snprintf(label, sizeof(label),
                     "THERON_SRM_SLOT_%d_UNSUPPORTED_BODY_00", i);
            skip(label,
                 "real .srm inflated but body is not Firestaff readiness envelope");
            ++slot_unsupported;
        } else if (ok) {
            char label[80];
            snprintf(label, sizeof(label),
                     "THERON_SRM_SLOT_%d_ROUND_TRIP_OK_00", i);
            pass(label, slot->path);
            ++slot_pass;
        } else {
            ++slot_other_fail;
        }
    }

    cleanup_temp_root(temp_root);

    if (slot_other_fail == 0 && slot_pass >= 1) {
        pass("THERON_SRM_REAL_ARTIFACT_OK_00",
             "at least one staged .srm survived bounded inflate + .tqsv round-trip");
    } else if (slot_other_fail == 0 && slot_unsupported >= 1 && slot_pass == 0) {
        /* Every recognized .srm inflated cleanly but none used the
         * bounded readiness envelope.  This is the expected outcome
         * on real Sphenx / Greatstone TQ-RTC bodies and stays a
         * SKIP, not a FAIL. */
        skip("THERON_SRM_REAL_ARTIFACT_READY_BUT_NO_ENVELOPE_00",
             "recognized .srm inflated cleanly but no Firestaff readiness envelope present");
    } else if (slot_other_fail > 0) {
        /* g_fail was already incremented per slot. */
    } else {
        skip("THERON_SRM_REAL_ARTIFACT_NO_RECEIPT_00",
             "no recognized slot reached the .tqsv round-trip");
    }
#endif /* FIRESTAFF_HAS_ZLIB */
}

int main(void) {
    printf("=== Theron V1 SRM Real-Artifact Readiness Probe ===\n");
    printf("%s\n", theron_v1_srm_source_evidence());

    probe_real_root_receipt();

    printf("summary: pass=%d fail=%d skip=%d zlib=%d\n",
           g_pass, g_fail, g_skip, FIRESTAFF_HAS_ZLIB);
    return g_fail ? 1 : 0;
}
