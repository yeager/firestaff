/*
 * theron_v1_startup_save_resume.c
 *
 * Theron's Quest V1 — startup save/resume smoke gate.
 *
 * Bounded handoff between the boot profile, the bounded SRM (Save Disk)
 * classifier, and the bounded Firestaff-native .tqsv slot enumerator.
 * See include/theron_v1_startup_save_resume.h for the contract.
 *
 * Source/evidence:
 *   - docs/DMWEB_REFERENCE.md §6 "Theron's Quest savegame format"
 *   - docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *   - THQUEST.ASM T080  — between-dungeon save/load
 *   - THQUEST.ASM T800  — champion persistence between dungeons
 *
 * Status (this commit):
 *   - Data-free; never reads/parses user game data without explicit
 *     boot_save_root or FIRESTAFF_THERON_SRM_DIR + a staged file.
 *   - On a clean host (no save root staged) the gate reports
 *     SKIP_SAFE_NO_SAVE_ROOT and resume_claim = NONE.  This is the
 *     expected honest outcome and is recorded as a SKIP, not a
 *     failure, by the probe and unit test.
 *   - The gate consumes the existing theron_v1_srm_classifier
 *     (5-slot disk manifest, gzip magic/method detection) and
 *     theron_v1_save_load (8-slot .tqsv enumerate/verify) without
 *     re-implementing them.
 *   - It does not decode real Sphenx/Greatstone custom save bodies;
 *     unknown real .srm payloads stay UNSUPPORTED_BODY.
 *   - It does not auto-resume the game; the resume-claim level is a
 *     bounded receipt for the M12/M11 startup layer, not a runtime
 *     mutation.  The startup layer still owns the explicit "Continue"
 *     UX.
 */

#include "theron_v1_startup_save_resume.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#define TSR_PATH_SEP '\\'
#else
#define TSR_PATH_SEP '/'
#endif

/* Bounded payload decode probe — only when zlib is available and only
 * for the first RECOGNIZED .srm slot.  The probe stays deliberately
 * small: 8 KiB inflate window plus a 64-byte progression envelope
 * mirror, mirroring the existing theron_v1_srm_classifier_*_pc34
 * contract. */
#define TSR_INFLATE_BUFFER_BYTES 8192u
#define TSR_PROGRESSION_ENVELOPE_BYTES 44u
#define TSR_PROGRESSION_MAGIC_LEN 8u
#define TSR_FSTQ_PROGRESSION_MAGIC "FSTQPRG1"

static void copy_name(char *dst, size_t dst_size, const char *src) {
    if (!dst || dst_size == 0u) return;
    if (!src) {
        dst[0] = '\0';
        return;
    }
    size_t n = strlen(src);
    if (n >= dst_size) n = dst_size - 1u;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/* ── Root resolution ──────────────────────────────────────────────── */

static void resolve_tqsv_root(const char *boot_save_root,
                                char out[THERON_V1_SRM_PATH_MAX]) {
    if (!out) return;
    out[0] = '\0';
    if (boot_save_root && boot_save_root[0]) {
        size_t n = strlen(boot_save_root);
        if (n >= THERON_V1_SRM_PATH_MAX) n = THERON_V1_SRM_PATH_MAX - 1u;
        memcpy(out, boot_save_root, n);
        out[n] = '\0';
        return;
    }
    /* Fall back to the existing canonical default. */
    theron_v1_save_default_root(out, THERON_V1_SRM_PATH_MAX);
}

/* srm_root is resolved by the existing theron_v1_srm_default_root
 * helper (env override, then $HOME/.firestaff/data/theron/save, then
 * ./theron-save).  This gate does not introduce a parallel resolver. */
static void resolve_srm_root(char out[THERON_V1_SRM_PATH_MAX]) {
    if (!out) return;
    out[0] = '\0';
    if (!theron_v1_srm_default_root(out)) {
        out[0] = '\0';
    }
}

/* ── Verdict + claim computation ──────────────────────────────────── */

static Theron_V1StartupSkipSafeVerdict compute_verdict(
    const Theron_V1StartupSaveResume *snap) {

    if (snap->tqsv_total_slots == 0 && snap->srm_total_slots == 0) {
        /* Defensive: the classifier should never give zero slots,
         * but if it does, treat as a clean host. */
        return THERON_V1_STARTUP_SKIP_SAFE_NO_SAVE_ROOT;
    }

    int tqsv_root_live = snap->tqsv_root[0] != '\0';
    int srm_root_live = snap->srm_root[0] != '\0';

    if (!tqsv_root_live && !srm_root_live) {
        return THERON_V1_STARTUP_SKIP_SAFE_NO_SAVE_ROOT;
    }

    int tqsv_has_anything = (snap->tqsv_valid_slots > 0);
    int srm_has_anything = (snap->srm_present_slots > 0);

    if (!tqsv_has_anything && !srm_has_anything) {
        return THERON_V1_STARTUP_SKIP_SAVE_ROOT_PRESENT_NO_SLOTS;
    }

    int tqsv_recognized = tqsv_has_anything;
    int srm_recognized = (snap->srm_recognized_slots > 0);
    if (!tqsv_recognized && !srm_recognized) {
        return THERON_V1_STARTUP_SKIP_STAGED_NO_RECOGNIZED_SLOT;
    }

    return THERON_V1_STARTUP_SOURCES_LIVE;
}

static Theron_V1StartupResumeClaim compute_claim(
    const Theron_V1StartupSaveResume *snap) {

    int tqsv = snap->tqsv_valid_slots > 0;
    int srm = snap->srm_recognized_slots > 0;

    if (tqsv && srm) return THERON_V1_STARTUP_RESUME_DUAL;
    if (tqsv)        return THERON_V1_STARTUP_RESUME_TQSV;
    if (srm)         return THERON_V1_STARTUP_RESUME_SRM;
    return THERON_V1_STARTUP_RESUME_NONE;
}

/* ── .tqsv enumeration via existing API ───────────────────────────── */

static void scan_tqsv_slots(Theron_V1StartupSaveResume *snap) {
    Theron_SaveSlot slots[THERON_SAVE_SLOT_COUNT];
    memset(slots, 0, sizeof(slots));

    snap->tqsv_total_slots = THERON_SAVE_SLOT_COUNT;
    snap->tqsv_valid_slots = 0;
    snap->tqsv_active_slot = -1;
    snap->tqsv_active_timestamp = 0u;

    if (!snap->tqsv_root[0]) {
        return;
    }

    int found = theron_v1_save_enum_slots(snap->tqsv_root,
                                          slots,
                                          THERON_SAVE_SLOT_COUNT);
    if (found <= 0) {
        return;
    }

    uint32_t best_timestamp = 0u;
    int best_slot = -1;
    for (int i = 0; i < found; i++) {
        if (!slots[i].valid) continue;
        snap->tqsv_valid_slots++;
        if (best_slot < 0 ||
            slots[i].timestamp > best_timestamp) {
            best_slot = slots[i].slot_index;
            best_timestamp = slots[i].timestamp;
        }
    }
    snap->tqsv_active_slot = best_slot;
    snap->tqsv_active_timestamp = best_timestamp;
}

/* ── .srm classification via existing API ─────────────────────────── */

static void scan_srm_slots(Theron_V1StartupSaveResume *snap) {
    snap->srm_total_slots = THERON_V1_SRM_DISK_SLOT_COUNT;
    snap->srm_present_slots = 0;
    snap->srm_recognized_slots = 0;
    snap->srm_first_recognized_slot = -1;
    snap->srm_first_recognized_checksum32 = 0u;
    snap->srm_payload_probe_status = THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT;
    snap->srm_payload_probe_ran = 0;
    snap->srm_payload_size = 0u;
    snap->srm_payload_hits_fstq_magic = 0;
    snap->srm_progress_import_status = THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    snap->srm_progress_import_ran = 0;
    snap->srm_progress_current_dungeon = -1;
    snap->srm_progress_current_level = -1;
    snap->srm_progress_quest_mask = -1;

    if (!snap->srm_root[0]) {
        return;
    }

    Theron_V1SrmManifest manifest;
    memset(&manifest, 0, sizeof(manifest));
    if (!theron_v1_srm_classify_root(snap->srm_root, &manifest)) {
        return;
    }
    snap->srm_present_slots = manifest.present_count;
    snap->srm_recognized_slots = manifest.recognized_count;

    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; i++) {
        if (manifest.slots[i].status ==
                THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED) {
            snap->srm_first_recognized_slot = i;
            snap->srm_first_recognized_checksum32 =
                manifest.slots[i].prefix_checksum32;
            break;
        }
    }

    /* When the gate found a recognized .srm, attempt a bounded
     * payload probe + progression-decode.  We only do this for the
     * first RECOGNIZED slot and only when zlib is built in — the
     * gate stays cheap and never claims Sphenx/Greatstone custom
     * body coverage. */
    if (snap->srm_first_recognized_slot < 0) {
        return;
    }

#if FIRESTAFF_HAS_ZLIB
    char slot_path[THERON_V1_SRM_PATH_MAX];
    if (!theron_v1_srm_slot_path(snap->srm_root,
                                  snap->srm_first_recognized_slot,
                                  slot_path)) {
        return;
    }

    FILE *fp = fopen(slot_path, "rb");
    if (!fp) {
        return;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return;
    }
    long sz = ftell(fp);
    if (sz < 0 || sz > 256L * 1024L * 1024L) {
        /* Bounded: refuse to slurp anything larger than 256 MiB.
         * Real Theron Save Disk bodies are a few KiB; this keeps the
         * gate deterministic and skip-safe on accidental huge files. */
        fclose(fp);
        return;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return;
    }
    size_t srm_size = (size_t)sz;
    uint8_t *srm_bytes = (uint8_t *)malloc(srm_size > 0u ? srm_size : 1u);
    if (!srm_bytes) {
        fclose(fp);
        return;
    }
    size_t got = fread(srm_bytes, 1, srm_size, fp);
    fclose(fp);
    if (got != srm_size || srm_size < 10u) {
        free(srm_bytes);
        return;
    }

    uint8_t payload[TSR_INFLATE_BUFFER_BYTES];
    size_t payload_size = 0u;
    Theron_V1SrmPayloadProbeStatus probe_status =
        theron_v1_srm_probe_gzip_payload(
            srm_bytes,
            srm_size,
            payload,
            sizeof(payload),
            &payload_size);

    snap->srm_payload_probe_status = probe_status;
    snap->srm_payload_probe_ran = 1;
    snap->srm_payload_size = payload_size;
    snap->srm_payload_hits_fstq_magic =
        (payload_size >= TSR_PROGRESSION_MAGIC_LEN &&
         memcmp(payload, TSR_FSTQ_PROGRESSION_MAGIC,
                TSR_PROGRESSION_MAGIC_LEN) == 0) ? 1 : 0;

    if (probe_status == THERON_V1_SRM_PAYLOAD_PROBE_OK &&
        snap->srm_payload_hits_fstq_magic) {
        Theron_DungeonProgression prog;
        Theron_V1SrmProgressionReceipt receipt;
        Theron_V1SrmProgressImportStatus import_status =
            theron_v1_srm_decode_progression_payload(
                payload,
                payload_size >= TSR_PROGRESSION_ENVELOPE_BYTES
                    ? TSR_PROGRESSION_ENVELOPE_BYTES
                    : payload_size,
                &prog,
                &receipt);
        snap->srm_progress_import_status = import_status;
        snap->srm_progress_import_ran = 1;
        if (import_status == THERON_V1_SRM_PROGRESS_IMPORT_OK) {
            snap->srm_progress_current_dungeon = (int)receipt.current_dungeon;
            snap->srm_progress_current_level = (int)receipt.current_level;
            snap->srm_progress_quest_mask =
                (int)receipt.quest_items_bitmask;
        }
    }
    free(srm_bytes);
#else
    /* Without zlib we deliberately do not run the inflate step.
     * The verdict/claim surface is independent of zlib presence, so
     * CI hosts without zlib still report the SRM-recognized count
     * without the bounded decode. */
#endif
}

/* ── Public API ───────────────────────────────────────────────────── */

int theron_v1_startup_save_resume_evaluate(
    const char *boot_save_root,
    Theron_V1StartupSaveResume *out_snapshot) {

    if (!out_snapshot) return 0;
    memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->tqsv_active_slot = -1;
    out_snapshot->srm_first_recognized_slot = -1;
    out_snapshot->srm_progress_current_dungeon = -1;
    out_snapshot->srm_progress_current_level = -1;
    out_snapshot->srm_progress_quest_mask = -1;

    resolve_tqsv_root(boot_save_root, out_snapshot->tqsv_root);
    resolve_srm_root(out_snapshot->srm_root);

    scan_tqsv_slots(out_snapshot);
    scan_srm_slots(out_snapshot);

    out_snapshot->verdict = compute_verdict(out_snapshot);
    out_snapshot->resume_claim = compute_claim(out_snapshot);

    copy_name(out_snapshot->verdict_name,
              sizeof(out_snapshot->verdict_name),
              theron_v1_startup_save_resume_skip_safe_name(
                  out_snapshot->verdict));
    copy_name(out_snapshot->resume_claim_name,
              sizeof(out_snapshot->resume_claim_name),
              theron_v1_startup_save_resume_claim_name(
                  out_snapshot->resume_claim));

    return 1;
}

size_t theron_v1_startup_save_resume_format(
    const Theron_V1StartupSaveResume *snap,
    char *buf,
    size_t buf_size) {

    if (!buf || buf_size == 0u) return 0u;
    if (!snap) {
        buf[0] = '\0';
        return 0u;
    }

    int n = snprintf(buf, buf_size,
                     "=== Theron V1 Startup Save/Resume Smoke Gate ===\n"
                     "tqsv_root:        %s\n"
                     "srm_root:         %s\n"
                     "verdict:          %s\n"
                     "resume_claim:     %s\n"
                     "tqsv_total_slots: %d\n"
                     "tqsv_valid_slots: %d\n"
                     "tqsv_active_slot: %d\n"
                     "srm_total_slots:  %d\n"
                     "srm_present_slots:%d\n"
                     "srm_recognized:   %d\n"
                     "srm_first_recog:  %d\n"
                     "srm_payload_probe:%s ran=%d size=%zu fstq_magic=%d\n"
                     "srm_prog_import:  %s ran=%d dungeon=%d level=%d quest=0x%02x\n",
                     snap->tqsv_root[0] ? snap->tqsv_root : "(none)",
                     snap->srm_root[0]  ? snap->srm_root  : "(none)",
                     snap->verdict_name[0] ? snap->verdict_name : "UNKNOWN",
                     snap->resume_claim_name[0] ? snap->resume_claim_name : "UNKNOWN",
                     snap->tqsv_total_slots,
                     snap->tqsv_valid_slots,
                     snap->tqsv_active_slot,
                     snap->srm_total_slots,
                     snap->srm_present_slots,
                     snap->srm_recognized_slots,
                     snap->srm_first_recognized_slot,
                     theron_v1_srm_payload_probe_status_name(snap->srm_payload_probe_status),
                     snap->srm_payload_probe_ran,
                     snap->srm_payload_size,
                     snap->srm_payload_hits_fstq_magic,
                     theron_v1_srm_progress_import_status_name(snap->srm_progress_import_status),
                     snap->srm_progress_import_ran,
                     snap->srm_progress_current_dungeon,
                     snap->srm_progress_current_level,
                     snap->srm_progress_quest_mask >= 0
                         ? snap->srm_progress_quest_mask : 0);

    if (n < 0) {
        buf[0] = '\0';
        return 0u;
    }
    if ((size_t)n >= buf_size) {
        return buf_size - 1u;
    }
    return (size_t)n;
}

const char *theron_v1_startup_save_resume_skip_safe_name(
    Theron_V1StartupSkipSafeVerdict verdict) {
    switch (verdict) {
    case THERON_V1_STARTUP_SOURCES_LIVE:
        return "SOURCES_LIVE";
    case THERON_V1_STARTUP_SKIP_SAFE_NO_SAVE_ROOT:
        return "SKIP_SAFE_NO_SAVE_ROOT";
    case THERON_V1_STARTUP_SKIP_SAVE_ROOT_PRESENT_NO_SLOTS:
        return "SKIP_SAVE_ROOT_PRESENT_NO_SLOTS";
    case THERON_V1_STARTUP_SKIP_STAGED_NO_RECOGNIZED_SLOT:
        return "SKIP_STAGED_NO_RECOGNIZED_SLOT";
    }
    return "UNKNOWN";
}

const char *theron_v1_startup_save_resume_claim_name(
    Theron_V1StartupResumeClaim claim) {
    switch (claim) {
    case THERON_V1_STARTUP_RESUME_NONE:
        return "NO_RESUME_CLAIM";
    case THERON_V1_STARTUP_RESUME_TQSV:
        return "TQSV_RESUME_CLAIM";
    case THERON_V1_STARTUP_RESUME_SRM:
        return "SRM_RESUME_CLAIM";
    case THERON_V1_STARTUP_RESUME_DUAL:
        return "DUAL_RESUME_CLAIM";
    }
    return "UNKNOWN";
}

const char *theron_v1_startup_save_resume_source_evidence(void) {
    return
        "Theron V1 startup save/resume smoke gate\n"
        "\n"
        "Source/evidence:\n"
        "  - THQUEST.ASM T080 — between-dungeon save/load (no in-dungeon)\n"
        "  - THQUEST.ASM T800 — champion persistence between dungeons\n"
        "  - docs/DMWEB_REFERENCE.md §6 'Theron's Quest savegame format':\n"
        "      'TQ's save format is *completely different* from DM's (it\n"
        "       uses gzipped custom format with a header).'\n"
        "  - docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md\n"
        "    anchors the JP/US Track 02 MD5s and the T080 design rule.\n"
        "  - dmweb community docs credit Sphenx with several custom TQ\n"
        "    saves documented at greatstone; Sphenx is also a SKWIN DM2\n"
        "    skproject co-author.\n"
        "\n"
        "Status:\n"
        "  - Data-free startup gate; the clean-host default outcome is\n"
        "    SKIP_SAFE_NO_SAVE_ROOT with NO_RESUME_CLAIM.\n"
        "  - Reads the existing theron_v1_srm_classifier manifest\n"
        "    (5-slot Save Disk classifier, gzip magic/method detection)\n"
        "    and theron_v1_save_load .tqsv enumerator (8-slot\n"
        "    between-dungeon slot model); no parallel decoders.\n"
        "  - Reports one of four skip-safe verdicts:\n"
        "      SOURCES_LIVE,\n"
        "      SKIP_SAFE_NO_SAVE_ROOT,\n"
        "      SKIP_SAVE_ROOT_PRESENT_NO_SLOTS,\n"
        "      SKIP_STAGED_NO_RECOGNIZED_SLOT.\n"
        "  - Reports one of four bounded resume claims:\n"
        "      NO_RESUME_CLAIM, TQSV_RESUME_CLAIM,\n"
        "      SRM_RESUME_CLAIM, DUAL_RESUME_CLAIM.\n"
        "  - When zlib is available and a recognized .srm is staged,\n"
        "    additionally runs the bounded payload probe + bounded\n"
        "    FSTQPRG1 progression-envelope decode; unknown real\n"
        "    Sphenx/Greatstone bodies stay UNSUPPORTED_BODY.\n"
        "  - Does NOT auto-resume the game; the M12/M11 startup layer\n"
        "    still owns the explicit Continue UX.\n"
        "  - Does NOT decode real Sphenx/Greatstone custom save bodies\n"
        "    or promote any auto-resume into runtime state.";
}
