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

#include "theron_v1_boot.h"
#include "theron_v1_startup_runtime_entry.h"

#include <stdio.h>
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

static Theron_V1StartupSkipSafeVerdict compute_verdict(
    const Theron_V1StartupSaveResume *snap);
static Theron_V1StartupResumeClaim compute_claim(
    const Theron_V1StartupSaveResume *snap);
static int theron_v1_startup_continue_srm_envelope_apply(
    Theron_V1_World *world,
    const Theron_V1SrmEnvelopeReceipt *envelope,
    Theron_V1SrmEnvelopeKind kind,
    const uint8_t *payload,
    const char *source_label,
    int slot_index,
    char *receipt,
    size_t receipt_cap);

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

static const char *path_basename(const char *path) {
    const char *slash;
    const char *backslash;
    if (!path || !path[0]) {
        return NULL;
    }
    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    if (!slash || (backslash && backslash > slash)) {
        slash = backslash;
    }
    return slash ? slash + 1 : path;
}

static int path_dirname(char out[THERON_V1_SRM_PATH_MAX],
                        const char *path) {
    const char *slash;
    const char *backslash;
    size_t len;
    if (!out) {
        return 0;
    }
    out[0] = '\0';
    if (!path || !path[0]) {
        return 0;
    }
    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    if (!slash || (backslash && backslash > slash)) {
        slash = backslash;
    }
    if (!slash || slash <= path) {
        copy_name(out, THERON_V1_SRM_PATH_MAX, ".");
        return 1;
    }
    len = (size_t)(slash - path);
    if (len >= THERON_V1_SRM_PATH_MAX) {
        len = THERON_V1_SRM_PATH_MAX - 1u;
    }
    memcpy(out, path, len);
    out[len] = '\0';
    return 1;
}

static int parse_slot_file(const char *path,
                           const char *suffix,
                           int max_slots) {
    const char *base = path_basename(path);
    if (!base || !suffix || max_slots <= 0) {
        return -1;
    }
    if (base[0] != 's' || base[1] != 'l' || base[2] != 'o' ||
        base[3] != 't' || base[4] < '0' ||
        base[4] >= (char)('0' + max_slots) ||
        strcmp(base + 5, suffix) != 0) {
        return -1;
    }
    return base[4] - '0';
}

static int path_has_suffix(const char *path, const char *suffix) {
    size_t path_len;
    size_t suffix_len;
    if (!path || !suffix) {
        return 0;
    }
    path_len = strlen(path);
    suffix_len = strlen(suffix);
    return path_len >= suffix_len &&
           strcmp(path + path_len - suffix_len, suffix) == 0;
}

static void recompute_verdict_and_claim(Theron_V1StartupSaveResume *snap) {
    if (!snap) {
        return;
    }
    snap->verdict = compute_verdict(snap);
    snap->resume_claim = compute_claim(snap);
    copy_name(snap->verdict_name,
              sizeof(snap->verdict_name),
              theron_v1_startup_save_resume_skip_safe_name(snap->verdict));
    copy_name(snap->resume_claim_name,
              sizeof(snap->resume_claim_name),
              theron_v1_startup_save_resume_claim_name(snap->resume_claim));
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
    /* An original SRM has no source-locked body decoder yet.  Its only
     * supported startup surface is a verified opaque transfer, never a
     * runtime resume claim. */
    int srm = 0;

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
    Theron_V1SrmEnvelopeReceipt envelopes[THERON_V1_SRM_DISK_SLOT_COUNT];
    snap->srm_total_slots = THERON_V1_SRM_DISK_SLOT_COUNT;
    snap->srm_present_slots = 0;
    snap->srm_recognized_slots = 0;
    snap->srm_first_recognized_slot = -1;
    snap->srm_first_recognized_checksum32 = 0u;
    snap->srm_first_decoded_slot = -1;
    snap->srm_first_opaque_transfer_slot = -1;
    snap->srm_opaque_transfer_slots = 0;
    snap->srm_payload_probe_status = THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT;
    snap->srm_payload_probe_ran = 0;
    snap->srm_payload_size = 0u;
    snap->srm_payload_hits_fstq_magic = 0;
    snap->srm_envelope_kind = THERON_V1_SRM_ENVELOPE_KIND_NONE;
    snap->srm_progress_import_status = THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    snap->srm_progress_import_ran = 0;
    snap->srm_progress_current_dungeon = -1;
    snap->srm_progress_current_level = -1;
    snap->srm_progress_quest_mask = -1;
    snap->srm_party_import_ran = 0;
    snap->srm_party_restored = 0;
    snap->srm_party_champion_count = 0;
    snap->srm_party_gold = 0u;
    memset(envelopes, 0, sizeof(envelopes));
    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; ++i) {
        envelopes[i].slot_index = i;
    }
    (void)theron_v1_srm_catalog_body_evidence(
        envelopes, THERON_V1_SRM_DISK_SLOT_COUNT,
        &snap->srm_body_evidence_catalog);

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

    /* Decode only far enough to authenticate the complete gzip member and
     * classify its body as opaque.  FSTQ envelopes are Firestaff fixtures,
     * not original save bodies, and are deliberately not surfaced here. */
    if (snap->srm_first_recognized_slot < 0) {
        return;
    }

#if FIRESTAFF_HAS_ZLIB
    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; ++i) {
        char slot_path[THERON_V1_SRM_PATH_MAX];
        uint8_t payload[TSR_INFLATE_BUFFER_BYTES];
        Theron_V1SrmEnvelopeReceipt *envelope = &envelopes[i];
        Theron_V1SrmEnvelopeKind kind;

        if (manifest.slots[i].status !=
            THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED ||
            !theron_v1_srm_slot_path(snap->srm_root, i, slot_path)) {
            continue;
        }
        memset(payload, 0, sizeof(payload));
        memset(envelope, 0, sizeof(*envelope));
        kind = theron_v1_srm_decode_path(slot_path, i, payload,
                                         sizeof(payload), envelope);
        if (i == snap->srm_first_recognized_slot) {
            snap->srm_envelope_kind = kind;
            snap->srm_payload_probe_status = envelope->inflate_status;
            snap->srm_payload_probe_ran = 1;
            snap->srm_payload_size = envelope->inflate_payload_size;
            snap->srm_payload_hits_fstq_magic = 0;
            snap->srm_progress_import_status =
                THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
        }
        if (kind == THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED &&
            envelope->inflate_status == THERON_V1_SRM_PAYLOAD_PROBE_OK &&
            envelope->body_evidence.captured &&
            envelope->body_evidence.gzip_trailer_verified) {
            ++snap->srm_opaque_transfer_slots;
            if (snap->srm_first_opaque_transfer_slot < 0) {
                snap->srm_first_opaque_transfer_slot = i;
            }
        }
    }
    (void)theron_v1_srm_catalog_body_evidence(
        envelopes, THERON_V1_SRM_DISK_SLOT_COUNT,
        &snap->srm_body_evidence_catalog);
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
    out_snapshot->srm_first_decoded_slot = -1;
    out_snapshot->srm_first_opaque_transfer_slot = -1;
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

int theron_v1_startup_save_resume_apply_explicit_path(
    Theron_V1StartupSaveResume *snap,
    const char *save_path,
    const char *tqsv_root)
{
    int slot;
    char root[THERON_V1_SRM_PATH_MAX];

    if (!snap || !save_path || !save_path[0]) {
        return 0;
    }

    slot = parse_slot_file(save_path, ".tqsv", THERON_SAVE_SLOT_COUNT);
    if (slot >= 0) {
        const char *root_to_use = tqsv_root;
        if (!root_to_use || !root_to_use[0]) {
            if (!path_dirname(root, save_path)) {
                return 0;
            }
            root_to_use = root;
        }
        if (!theron_v1_save_verify_slot(root_to_use, slot)) {
            return 0;
        }
        copy_name(snap->tqsv_root, sizeof(snap->tqsv_root), root_to_use);
        snap->tqsv_total_slots = THERON_SAVE_SLOT_COUNT;
        if (snap->tqsv_valid_slots <= 0) {
            snap->tqsv_valid_slots = 1;
        }
        snap->tqsv_active_slot = slot;
        recompute_verdict_and_claim(snap);
        return 1;
    }

    slot = parse_slot_file(save_path,
                           ".srm",
                           THERON_V1_SRM_DISK_SLOT_COUNT);
    if (slot >= 0 || path_has_suffix(save_path, ".srm")) {
        char srm_path[THERON_V1_SRM_PATH_MAX];
        uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
        Theron_V1SrmEnvelopeReceipt envelope;
        Theron_V1SrmEnvelopeKind kind;

        if (!path_dirname(root, save_path)) {
            return 0;
        }
        if (slot >= 0) {
            if (!theron_v1_srm_slot_path(root, slot, srm_path)) {
                return 0;
            }
        } else {
            copy_name(srm_path, sizeof(srm_path), save_path);
        }
        memset(scratch, 0, sizeof(scratch));
        memset(&envelope, 0, sizeof(envelope));
        kind = theron_v1_srm_decode_path(srm_path,
                                          slot,
                                          scratch,
                                          sizeof(scratch),
                                          &envelope);
        if (kind != THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED ||
            envelope.inflate_status != THERON_V1_SRM_PAYLOAD_PROBE_OK ||
            !envelope.body_evidence.captured ||
            !envelope.body_evidence.gzip_trailer_verified) {
            return 0;
        }
        copy_name(snap->srm_root, sizeof(snap->srm_root), root);
        snap->srm_total_slots = THERON_V1_SRM_DISK_SLOT_COUNT;
        if (snap->srm_present_slots <= 0) {
            snap->srm_present_slots = 1;
        }
        if (snap->srm_recognized_slots <= 0) {
            snap->srm_recognized_slots = 1;
        }
        snap->srm_first_recognized_slot = slot;
        snap->srm_first_decoded_slot = -1;
        snap->srm_first_opaque_transfer_slot = slot;
        snap->srm_opaque_transfer_slots = 1;
        snap->srm_envelope_kind = kind;
        snap->srm_payload_probe_status = envelope.inflate_status;
        snap->srm_payload_probe_ran =
            envelope.inflate_status != THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT;
        snap->srm_payload_size = envelope.inflate_payload_size;
        snap->srm_payload_hits_fstq_magic = 0;
        snap->srm_progress_import_status =
            THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
        recompute_verdict_and_claim(snap);
        return 1;
    }

    return 0;
}

int theron_v1_startup_save_resume_state_receipt(
    const Theron_V1StartupSaveResume *snapshot,
    int snapshot_ready,
    Theron_StartupStateReceipt *out_receipt) {

    if (!out_receipt) {
        return 0;
    }
    theron_v1_startup_state_receipt_init(out_receipt);
    out_receipt->set_save_resume = 1;
    out_receipt->save_resume_verdict =
        (snapshot_ready && snapshot) ? (int)snapshot->verdict : -1;
    out_receipt->save_resume_claim =
        (snapshot_ready && snapshot) ? (int)snapshot->resume_claim : -1;
    out_receipt->save_resume_active_slot =
        (snapshot_ready && snapshot) ? snapshot->tqsv_active_slot : -1;
    out_receipt->save_resume_srm_active_slot =
        (snapshot_ready && snapshot) ? snapshot->srm_first_opaque_transfer_slot : -1;
    out_receipt->save_resume_srm_import_status =
        (snapshot_ready && snapshot)
            ? (int)snapshot->srm_progress_import_status
            : (int)THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    out_receipt->save_resume_srm_current_dungeon =
        (snapshot_ready && snapshot) ? snapshot->srm_progress_current_dungeon : -1;
    out_receipt->save_resume_srm_current_level =
        (snapshot_ready && snapshot) ? snapshot->srm_progress_current_level : -1;
    out_receipt->save_resume_srm_quest_mask =
        (snapshot_ready && snapshot) ? snapshot->srm_progress_quest_mask : -1;
    out_receipt->save_resume_srm_party_restored =
        (snapshot_ready && snapshot) ? snapshot->srm_party_restored : 0;
    out_receipt->save_resume_srm_party_champion_count =
        (snapshot_ready && snapshot) ? snapshot->srm_party_champion_count : 0;
    out_receipt->save_resume_srm_party_gold =
        (snapshot_ready && snapshot) ? snapshot->srm_party_gold : 0u;
    out_receipt->set_continue_focus = 1;
    out_receipt->continue_focus = 0;
    out_receipt->save_resume_tqsv_slots =
        (snapshot_ready && snapshot) ? snapshot->tqsv_valid_slots : 0;
    out_receipt->save_resume_srm_slots =
        (snapshot_ready && snapshot) ? snapshot->srm_recognized_slots : 0;
    if (snapshot_ready &&
        snapshot &&
        snapshot->srm_first_opaque_transfer_slot >= 0 &&
        snapshot->srm_root[0] != '\0') {
        snprintf(out_receipt->save_resume_srm_root,
                 sizeof(out_receipt->save_resume_srm_root),
                 "%s",
                 snapshot->srm_root);
    } else {
        out_receipt->save_resume_srm_root[0] = '\0';
    }
    return 1;
}

static void theron_v1_startup_continue_reset_world_runtime(
    Theron_V1_World *world) {

    if (!world) {
        return;
    }
    world->current_dungeon = (int)world->progression.current_dungeon;
    world->current_level = world->progression.current_level > 0
        ? (int)world->progression.current_level - 1
        : 0;
    world->object_count = 0;
    world->timer_count = 0;
    world->transition_pending = 0;
    world->quest_items_in_dungeon =
        world->progression.quest_items_in_current_dungeon;
    world->dungeon_complete = 0;
    memset(world->level_loaded, 0, sizeof(world->level_loaded));
}

int theron_v1_startup_continue_tqsv_apply(
    Theron_V1_World *world,
    const char *save_root,
    int slot_index,
    char *receipt,
    size_t receipt_cap) {

    Theron_SaveSlot slot_info;
    Theron_DungeonProgression loaded_progression;
    unsigned char champion_data[
        THERON_MAX_CHAMPIONS * sizeof(Theron_V1_Champion)];

    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }
    if (!world || !save_root || save_root[0] == '\0') {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "Continue requires a .tqsv root");
        }
        return 0;
    }
    if (slot_index < 0 || slot_index >= THERON_SAVE_SLOT_COUNT) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "No .tqsv slot selected");
        }
        return 0;
    }

    memset(&slot_info, 0, sizeof(slot_info));
    memset(&loaded_progression, 0, sizeof(loaded_progression));
    memset(champion_data, 0, sizeof(champion_data));
    if (theron_v1_save_load_from_slot(save_root,
                                      slot_index,
                                      champion_data,
                                      sizeof(champion_data),
                                      &loaded_progression,
                                      sizeof(loaded_progression),
                                      &slot_info) != 0 ||
        !slot_info.valid) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt,
                     receipt_cap,
                     "Continue slot %d failed",
                     slot_index);
        }
        return 0;
    }

    Theron_V1_Party loaded_party;
    if (theron_v1_party_unpack(&loaded_party,
                               champion_data,
                               sizeof(champion_data)) != 0) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "Continue party decode failed");
        }
        return 0;
    }
    loaded_party.champion_count = 1;
    loaded_party.active_slot = THERON_CHAMPION_SLOT_THERON;
    world->progression = loaded_progression;
    world->party = loaded_party;
    theron_v1_startup_continue_reset_world_runtime(world);

    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 "continued slot=%d dungeon=%d label=%s",
                 slot_index,
                 (int)loaded_progression.current_dungeon,
                 slot_info.label[0] ? slot_info.label : "TQSV");
    }
    return 1;
}

int theron_v1_startup_continue_tqsv_path_apply(
    Theron_V1_World *world,
    const char *save_path,
    char *receipt,
    size_t receipt_cap) {

    Theron_SaveSlot slot_info;
    Theron_DungeonProgression loaded_progression;
    unsigned char champion_data[
        THERON_MAX_CHAMPIONS * sizeof(Theron_V1_Champion)];

    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }
    if (!world || !save_path || save_path[0] == '\0') {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "Continue requires a .tqsv path");
        }
        return 0;
    }

    memset(&slot_info, 0, sizeof(slot_info));
    memset(&loaded_progression, 0, sizeof(loaded_progression));
    memset(champion_data, 0, sizeof(champion_data));
    if (theron_v1_save_load_from_path(save_path,
                                      champion_data,
                                      sizeof(champion_data),
                                      &loaded_progression,
                                      sizeof(loaded_progression),
                                      &slot_info) != 0 ||
        !slot_info.valid) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "Continue TQSV path failed");
        }
        return 0;
    }

    world->progression = loaded_progression;
    theron_v1_startup_continue_reset_world_runtime(world);
    if (theron_v1_party_unpack(&world->party,
                               champion_data,
                               sizeof(champion_data)) != 0) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "Continue party decode failed");
        }
        return 0;
    }
    world->party.champion_count = 1;
    world->party.active_slot = THERON_CHAMPION_SLOT_THERON;

    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 "continued tqsv path dungeon=%d label=%s",
                 (int)loaded_progression.current_dungeon,
                 slot_info.label[0] ? slot_info.label : "TQSV");
    }
    return 1;
}

static int theron_v1_startup_continue_srm_envelope_apply(
    Theron_V1_World *world,
    const Theron_V1SrmEnvelopeReceipt *envelope,
    Theron_V1SrmEnvelopeKind kind,
    const uint8_t *payload,
    const char *source_label,
    int slot_index,
    char *receipt,
    size_t receipt_cap) {

    int i;

    if (!world || !envelope) {
        return 0;
    }
    if (kind != THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION &&
        kind != THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt,
                     receipt_cap,
                     "SRM decode unsupported: %s",
                     theron_v1_srm_envelope_kind_name(kind));
        }
        return 0;
    }
    if (!envelope->progression.restored) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "SRM progression missing");
        }
        return 0;
    }

    theron_v1_dungeon_progression_init(&world->progression);
    world->progression.current_dungeon = envelope->progression.current_dungeon;
    world->progression.current_level = envelope->progression.current_level;
    world->progression.quest_items_collected =
        envelope->progression.quest_items_bitmask;
    world->progression.dungeon_playtime_seconds =
        envelope->progression.dungeon_playtime_seconds;
    for (i = 0; i < THERON_DUNGEON_COUNT; ++i) {
        world->progression.dungeon_seeds[i] =
            envelope->progression.dungeon_seeds[i];
    }
    if (kind == THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY &&
        envelope->party.restored && payload) {
        Theron_DungeonProgression decoded_progression;
        Theron_V1_Party decoded_party;
        Theron_V1SrmPartyImportReceipt party_receipt;
        if (theron_v1_srm_decode_progression_party_payload(
                payload,
                envelope->inflate_payload_size,
                &decoded_progression,
                &decoded_party,
                &party_receipt) == THERON_V1_SRM_PROGRESS_IMPORT_OK) {
            world->progression = decoded_progression;
            world->party = decoded_party;
        }
    } else {
        world->party.champion_count = 1;
        world->party.active_slot = THERON_CHAMPION_SLOT_THERON;
    }
    theron_v1_startup_continue_reset_world_runtime(world);

    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 "continued srm %s=%d kind=%s dungeon=%d",
                 source_label && source_label[0] ? source_label : "slot",
                 slot_index,
                 theron_v1_srm_envelope_kind_name(kind),
                 (int)world->progression.current_dungeon);
    }
    return 1;
}

int theron_v1_startup_continue_srm_apply(
    Theron_V1_World *world,
    const char *srm_root,
    int slot_index,
    char *receipt,
    size_t receipt_cap) {

    char root[THERON_V1_SRM_PATH_MAX];
    char path[THERON_V1_SRM_PATH_MAX];
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;

    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }
    if (!world) {
        return 0;
    }
    if (slot_index < 0 || slot_index >= THERON_V1_SRM_DISK_SLOT_COUNT) return 0;
    if (srm_root && srm_root[0] != '\0') snprintf(root, sizeof(root), "%s", srm_root);
    else if (!theron_v1_srm_default_root(root)) return 0;
    if (!theron_v1_srm_slot_path(root, slot_index, path)) return 0;
    memset(scratch, 0, sizeof(scratch));
    memset(&envelope, 0, sizeof(envelope));
    kind = theron_v1_srm_decode_path(path, slot_index, scratch, sizeof(scratch), &envelope);
    return theron_v1_startup_continue_srm_envelope_apply(
        world, &envelope, kind, scratch, "slot", slot_index, receipt, receipt_cap);
}

int theron_v1_startup_continue_srm_path_apply(
    Theron_V1_World *world,
    const char *srm_path,
    char *receipt,
    size_t receipt_cap) {

    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;

    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }
    if (!world || !srm_path || srm_path[0] == '\0') {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "Continue requires an SRM path");
        }
        return 0;
    }

    memset(scratch, 0, sizeof(scratch));
    memset(&envelope, 0, sizeof(envelope));
    kind = theron_v1_srm_decode_path(srm_path, -1, scratch, sizeof(scratch), &envelope);
    return theron_v1_startup_continue_srm_envelope_apply(
        world, &envelope, kind, scratch, "path", -1, receipt, receipt_cap);
}

void theron_v1_startup_continue_request_init(
    Theron_V1StartupContinueRequest *request) {

    if (!request) {
        return;
    }
    memset(request, 0, sizeof(*request));
    request->resume_claim = THERON_V1_STARTUP_RESUME_NONE;
    request->tqsv_slot_index = -1;
    request->srm_slot_index = -1;
    request->srm_import_status = THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
}

void theron_v1_startup_continue_result_init(
    Theron_V1StartupContinueResult *result) {

    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->source = THERON_V1_STARTUP_CONTINUE_SOURCE_NONE;
    result->source_slot_index = -1;
    result->srm_import_status = THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    result->selected_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    result->startup_cursor = 0;
    result->continue_focus = 0;
    result->srm_current_dungeon = -1;
    result->srm_current_level = -1;
    result->srm_quest_mask = -1;
}

void theron_v1_startup_continue_apply_receipt_init(
    Theron_V1StartupContinueApplyReceipt *receipt) {

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->input_result = THERON_STARTUP_INPUT_RESULT_IGNORED;
    receipt->source = THERON_V1_STARTUP_CONTINUE_SOURCE_NONE;
    receipt->source_slot_index = -1;
    receipt->srm_import_status = THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    receipt->srm_current_dungeon = -1;
    receipt->srm_current_level = -1;
    receipt->srm_quest_mask = -1;
}

static void theron_v1_startup_continue_apply_receipt_copy_media_spans(
    Theron_V1StartupContinueApplyReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *media_receipt) {

    if (!out_receipt || !media_receipt) {
        return;
    }
    out_receipt->track02_media_title_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_title_first_raw_offset;
    out_receipt->track02_media_title_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_title_last_raw_offset;
    out_receipt->track02_media_title_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_title_first_user_data_offset;
    out_receipt->track02_media_title_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_title_last_user_data_offset;
    out_receipt->track02_media_stage_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_first_raw_offset;
    out_receipt->track02_media_stage_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_last_raw_offset;
    out_receipt->track02_media_stage_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_first_user_data_offset;
    out_receipt->track02_media_stage_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_last_user_data_offset;
    out_receipt->track02_media_soul_room_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_first_raw_offset;
    out_receipt->track02_media_soul_room_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_last_raw_offset;
    out_receipt->track02_media_soul_room_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_first_user_data_offset;
    out_receipt->track02_media_soul_room_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_last_user_data_offset;
    out_receipt->track02_media_forcefield_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_first_raw_offset;
    out_receipt->track02_media_forcefield_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_last_raw_offset;
    out_receipt->track02_media_forcefield_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_first_user_data_offset;
    out_receipt->track02_media_forcefield_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_last_user_data_offset;
}

int theron_v1_startup_host_receipt_from_continue_apply(
    const Theron_V1StartupContinueApplyReceipt *apply_receipt,
    Theron_StartupHostReceipt *out_receipt) {

    if (!apply_receipt || !out_receipt) {
        return 0;
    }
    theron_v1_startup_host_receipt_init(out_receipt);
    out_receipt->input_result = apply_receipt->input_result;
    out_receipt->status_scope = apply_receipt->status_scope;
    out_receipt->status = apply_receipt->status;
    out_receipt->inspect_scope = apply_receipt->inspect_scope;
    snprintf(out_receipt->inspect_detail,
             sizeof(out_receipt->inspect_detail),
             "%s",
             apply_receipt->inspect_detail);
    out_receipt->track02_media_route = apply_receipt->track02_media_route;
    out_receipt->track02_media_route_mask =
        apply_receipt->track02_media_route_mask;
    out_receipt->track02_media_checksum =
        apply_receipt->track02_media_checksum;
    out_receipt->track02_media_title_first_raw_offset =
        apply_receipt->track02_media_title_first_raw_offset;
    out_receipt->track02_media_title_last_raw_offset =
        apply_receipt->track02_media_title_last_raw_offset;
    out_receipt->track02_media_title_first_user_data_offset =
        apply_receipt->track02_media_title_first_user_data_offset;
    out_receipt->track02_media_title_last_user_data_offset =
        apply_receipt->track02_media_title_last_user_data_offset;
    out_receipt->track02_media_stage_first_raw_offset =
        apply_receipt->track02_media_stage_first_raw_offset;
    out_receipt->track02_media_stage_last_raw_offset =
        apply_receipt->track02_media_stage_last_raw_offset;
    out_receipt->track02_media_stage_first_user_data_offset =
        apply_receipt->track02_media_stage_first_user_data_offset;
    out_receipt->track02_media_stage_last_user_data_offset =
        apply_receipt->track02_media_stage_last_user_data_offset;
    out_receipt->track02_media_soul_room_first_raw_offset =
        apply_receipt->track02_media_soul_room_first_raw_offset;
    out_receipt->track02_media_soul_room_last_raw_offset =
        apply_receipt->track02_media_soul_room_last_raw_offset;
    out_receipt->track02_media_soul_room_first_user_data_offset =
        apply_receipt->track02_media_soul_room_first_user_data_offset;
    out_receipt->track02_media_soul_room_last_user_data_offset =
        apply_receipt->track02_media_soul_room_last_user_data_offset;
    out_receipt->track02_media_forcefield_first_raw_offset =
        apply_receipt->track02_media_forcefield_first_raw_offset;
    out_receipt->track02_media_forcefield_last_raw_offset =
        apply_receipt->track02_media_forcefield_last_raw_offset;
    out_receipt->track02_media_forcefield_first_user_data_offset =
        apply_receipt->track02_media_forcefield_first_user_data_offset;
    out_receipt->track02_media_forcefield_last_user_data_offset =
        apply_receipt->track02_media_forcefield_last_user_data_offset;
    return 1;
}

static const char *theron_v1_startup_continue_source_name(
    Theron_V1StartupContinueSource source) {

    switch (source) {
    case THERON_V1_STARTUP_CONTINUE_SOURCE_TQSV:
        return "TQSV";
    case THERON_V1_STARTUP_CONTINUE_SOURCE_SRM:
        return "SRM";
    case THERON_V1_STARTUP_CONTINUE_SOURCE_NONE:
    default:
        return "NONE";
    }
}

void theron_v1_startup_continue_availability_init(
    Theron_V1StartupContinueAvailability *availability) {

    if (!availability) {
        return;
    }
    memset(availability, 0, sizeof(*availability));
    availability->tqsv_slot = -1;
    availability->srm_slot = -1;
    availability->srm_opaque_transfer_slot = -1;
}

int theron_v1_startup_continue_availability_from_state(
    Theron_V1StartupResumeClaim resume_claim,
    int tqsv_slot_index,
    int srm_slot_index,
    Theron_V1SrmProgressImportStatus srm_import_status,
    Theron_V1StartupContinueAvailability *out_availability) {

    if (!out_availability) {
        return 0;
    }
    theron_v1_startup_continue_availability_init(out_availability);
    out_availability->tqsv_slot = tqsv_slot_index;
    out_availability->srm_slot = srm_slot_index;
    out_availability->has_tqsv_continue =
        tqsv_slot_index >= 0 &&
        (resume_claim == THERON_V1_STARTUP_RESUME_TQSV ||
         resume_claim == THERON_V1_STARTUP_RESUME_DUAL);
    /* This adapter also serves historical explicit, already-decoded test
     * receipts. Startup profile selection never emits OK for an SRM body;
     * it emits UNSUPPORTED_BODY plus the opaque transfer candidate instead. */
    out_availability->has_srm_continue =
        srm_slot_index >= 0 &&
        srm_import_status == THERON_V1_SRM_PROGRESS_IMPORT_OK;
    out_availability->has_srm_opaque_transfer =
        srm_slot_index >= 0 &&
        srm_import_status == THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
    out_availability->srm_opaque_transfer_slot =
        out_availability->has_srm_opaque_transfer ? srm_slot_index : -1;
    out_availability->has_any_continue =
        out_availability->has_tqsv_continue ||
        out_availability->has_srm_continue;
    return 1;
}

static void theron_v1_startup_continue_capture_result(
    const Theron_V1_World *world,
    Theron_V1StartupContinueSource source,
    int source_slot_index,
    Theron_V1SrmProgressImportStatus srm_import_status,
    Theron_V1StartupContinueResult *out_result) {

    if (!world || !out_result) {
        return;
    }
    theron_v1_startup_continue_result_init(out_result);
    out_result->source = source;
    out_result->source_slot_index = source_slot_index;
    out_result->srm_import_status = srm_import_status;
    out_result->selected_dungeon = world->progression.current_dungeon;
    out_result->level_loaded = 0;
    out_result->startup_cursor = 0;
    out_result->continue_focus = 0;
    out_result->party_x = world->party.leader_x;
    out_result->party_y = world->party.leader_y;
    out_result->party_dir = world->party.leader_dir;
    if (source == THERON_V1_STARTUP_CONTINUE_SOURCE_SRM) {
        out_result->srm_current_dungeon =
            (int)world->progression.current_dungeon;
        out_result->srm_current_level =
            (int)world->progression.current_level;
        out_result->srm_quest_mask =
            (int)world->progression.quest_items_collected;
        out_result->srm_party_restored = world->party.champion_count > 0;
        out_result->srm_party_champion_count =
            (int)world->party.champion_count;
        out_result->srm_party_gold = world->party.gold;
    }
    out_result->tick_count = (int)world->world_tick;
}

static int theron_v1_startup_continue_attach_track02_media(
    Theron_V1_World *world,
    Theron_V1StartupContinueResult *result,
    const Theron_StartupMediaStateReceipt *media_receipt,
    char *receipt,
    size_t receipt_cap) {

    if (!world || !result || !media_receipt) {
        return 1;
    }
    if (!theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            media_receipt)) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap,
                     "Track 02 Continue media receipt incomplete; fallback visuals blocked");
        }
        return 0;
    }
    if (!theron_v1_startup_media_bind_runtime_receipt(world, media_receipt)) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap,
                     "Track 02 Continue media binding failed; fallback visuals blocked");
        }
        return 0;
    }
    result->track02_media_route = 1;
    result->track02_media = *media_receipt;
    if (!theron_v1_world_runtime_media_select_level_bank(
            world,
            THERON_RUNTIME_LEVEL_BANK_SAVE_RESUME,
            world->progression.current_dungeon,
            world->current_level)) {
        theron_v1_world_runtime_media_clear(world);
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap,
                     "Track 02 Continue level bank binding failed; fallback visuals blocked");
        }
        return 0;
    }
    result->track02_level_bank = world->runtime_media.level_bank;
    if (receipt && receipt_cap > 0u) {
        size_t used = strlen(receipt);
        if (used < receipt_cap - 1u) {
            snprintf(receipt + used, receipt_cap - used,
                     "%sTrack 02 media atlas=0x%x checksum=%08x level_bank=%d:%d route=0x%x",
                     used ? "; " : "",
                     media_receipt->startup_bitmap_atlas_route_mask,
                     (unsigned)media_receipt->startup_bitmap_atlas_checksum,
                     (int)result->track02_level_bank.dungeon_id,
                     result->track02_level_bank.level_index,
                     result->track02_level_bank.route_bit);
        }
    }
    return 1;
}

static int theron_v1_startup_continue_media_preflight(
    const Theron_StartupMediaStateReceipt *media_receipt,
    char *receipt,
    size_t receipt_cap) {

    if (!media_receipt) {
        return 1;
    }
    if (theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            media_receipt)) {
        return 1;
    }
    if (receipt && receipt_cap > 0u) {
        snprintf(receipt, receipt_cap,
                 "Track 02 Continue media receipt incomplete; fallback visuals blocked");
    }
    return 0;
}

int theron_v1_startup_continue_apply_request(
    Theron_V1_World *world,
    const Theron_V1StartupContinueRequest *request,
    Theron_V1StartupContinueResult *out_result,
    char *receipt,
    size_t receipt_cap) {

    int tqsv_available;
    int srm_available;
    Theron_V1_World staged_world;

    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }
    if (out_result) {
        theron_v1_startup_continue_result_init(out_result);
    }
    if (!world || !request) {
        return 0;
    }

    /* A supplied Track 02 receipt is launch evidence, not optional
     * decoration.  Check it before either restore path mutates the world so
     * Continue cannot leave a restored level that falls back to synthetic
     * startup graphics. */
    if (!theron_v1_startup_continue_media_preflight(
            request->track02_media_receipt, receipt, receipt_cap)) {
        return 0;
    }

    tqsv_available =
        request->tqsv_slot_index >= 0 &&
        request->tqsv_slot_index < THERON_SAVE_SLOT_COUNT &&
        (request->resume_claim == THERON_V1_STARTUP_RESUME_TQSV ||
         request->resume_claim == THERON_V1_STARTUP_RESUME_DUAL);
    srm_available =
        request->srm_slot_index >= 0 &&
        request->srm_slot_index < THERON_V1_SRM_DISK_SLOT_COUNT &&
        request->srm_import_status == THERON_V1_SRM_PROGRESS_IMPORT_OK;

    staged_world = *world;
    if (tqsv_available) {
        if (!theron_v1_startup_continue_tqsv_apply(
                &staged_world,
                request->tqsv_root,
                request->tqsv_slot_index,
                receipt,
                receipt_cap)) {
            return 0;
        }
        theron_v1_startup_continue_capture_result(
            &staged_world,
            THERON_V1_STARTUP_CONTINUE_SOURCE_TQSV,
            request->tqsv_slot_index,
            THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT,
            out_result);
        return theron_v1_startup_continue_attach_track02_media(
            &staged_world, out_result, request->track02_media_receipt, receipt, receipt_cap)
            ? (*world = staged_world, 1) : 0;
    }
    if (srm_available) {
        if (!theron_v1_startup_continue_srm_apply(
                &staged_world,
                request->srm_root,
                request->srm_slot_index,
                receipt,
                receipt_cap)) {
            return 0;
        }
        theron_v1_startup_continue_capture_result(
            &staged_world,
            THERON_V1_STARTUP_CONTINUE_SOURCE_SRM,
            request->srm_slot_index,
            request->srm_import_status,
            out_result);
        return theron_v1_startup_continue_attach_track02_media(
            &staged_world, out_result, request->track02_media_receipt, receipt, receipt_cap)
            ? (*world = staged_world, 1) : 0;
    }

    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 srm_available
                     ? "Continue requires a .tqsv slot"
                     : "Continue requires decoded SRM progress");
    }
    return 0;
}

int theron_v1_startup_continue_apply_receipt(
    const Theron_StartupActionPlan *plan,
    const Theron_V1StartupContinueResult *result,
    const char *continue_receipt,
    const char *chapter_marker_line,
    Theron_V1StartupContinueApplyReceipt *out_receipt) {

    if (!out_receipt) {
        return 0;
    }
    theron_v1_startup_continue_apply_receipt_init(out_receipt);
    if (!plan || !result ||
        result->source == THERON_V1_STARTUP_CONTINUE_SOURCE_NONE) {
        return 0;
    }

    out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
    out_receipt->source = result->source;
    out_receipt->source_slot_index = result->source_slot_index;
    out_receipt->srm_import_status = result->srm_import_status;
    out_receipt->srm_current_dungeon = result->srm_current_dungeon;
    out_receipt->srm_current_level = result->srm_current_level;
    out_receipt->srm_quest_mask = result->srm_quest_mask;
    out_receipt->srm_party_restored = result->srm_party_restored;
    out_receipt->srm_party_champion_count =
        result->srm_party_champion_count;
    out_receipt->srm_party_gold = result->srm_party_gold;
    out_receipt->track02_media_route = result->track02_media_route;
    out_receipt->track02_media_route_mask =
        result->track02_media.startup_bitmap_atlas_route_mask;
    out_receipt->track02_media_checksum =
        result->track02_media.startup_bitmap_atlas_checksum;
    theron_v1_startup_continue_apply_receipt_copy_media_spans(
        out_receipt,
        &result->track02_media);
    out_receipt->track02_level_bank = result->track02_level_bank;
    out_receipt->status_scope = plan->status_scope
        ? plan->status_scope
        : "STARTUP";
    out_receipt->status = plan->status ? plan->status : "CONTINUE LOADED";
    out_receipt->inspect_scope = "STARTUP";

    if (continue_receipt && continue_receipt[0] &&
        chapter_marker_line && chapter_marker_line[0]) {
        snprintf(out_receipt->inspect_detail,
                 sizeof(out_receipt->inspect_detail),
                 "%s; %s",
                 continue_receipt,
                 chapter_marker_line);
    } else if (continue_receipt && continue_receipt[0]) {
        snprintf(out_receipt->inspect_detail,
                 sizeof(out_receipt->inspect_detail),
                 "%s",
                 continue_receipt);
    } else {
        snprintf(out_receipt->inspect_detail,
                 sizeof(out_receipt->inspect_detail),
                 "%s",
                 chapter_marker_line ? chapter_marker_line : "");
    }
    return 1;
}

static int theron_v1_startup_continue_failure_apply_receipt(
    const Theron_StartupActionPlan *plan,
    const Theron_V1StartupContinueResult *result,
    const char *continue_receipt,
    Theron_V1StartupContinueApplyReceipt *out_receipt) {

    const char *status;

    if (!out_receipt) {
        return 0;
    }
    theron_v1_startup_continue_apply_receipt_init(out_receipt);
    status = (continue_receipt && continue_receipt[0])
        ? continue_receipt
        : (plan && plan->failure_status ? plan->failure_status
                                        : "CONTINUE FAILED");
    out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
    if (result) {
        out_receipt->source = result->source;
        out_receipt->source_slot_index = result->source_slot_index;
        out_receipt->srm_import_status = result->srm_import_status;
        out_receipt->srm_current_dungeon = result->srm_current_dungeon;
        out_receipt->srm_current_level = result->srm_current_level;
        out_receipt->srm_quest_mask = result->srm_quest_mask;
        out_receipt->srm_party_restored = result->srm_party_restored;
        out_receipt->srm_party_champion_count =
            result->srm_party_champion_count;
        out_receipt->srm_party_gold = result->srm_party_gold;
    }
    out_receipt->status_scope = (plan && plan->status_scope)
        ? plan->status_scope
        : "STARTUP";
    out_receipt->status = status;
    out_receipt->inspect_scope = "STARTUP";
    snprintf(out_receipt->inspect_detail,
             sizeof(out_receipt->inspect_detail),
             "%s%s%s",
             status,
             result ? " source=" : "",
             result ? theron_v1_startup_continue_source_name(result->source)
                    : "");
    return 1;
}

int theron_v1_startup_continue_state_receipt_from_result(
    const Theron_V1StartupContinueResult *result,
    Theron_StartupStateReceipt *out_receipt) {

    Theron_StartupFlow flow;

    if (!result || !out_receipt ||
        result->source == THERON_V1_STARTUP_CONTINUE_SOURCE_NONE) {
        return 0;
    }
    if (theron_v1_startup_show_stage_select(&flow,
                                            result->selected_dungeon) !=
        THERON_STARTUP_OK) {
        return 0;
    }
    if (!theron_v1_startup_state_receipt_from_flow(&flow, out_receipt)) {
        return 0;
    }
    out_receipt->set_level_loaded = 1;
    out_receipt->level_loaded = result->level_loaded;
    out_receipt->set_startup_cursor = 1;
    out_receipt->startup_cursor = result->startup_cursor;
    out_receipt->set_continue_focus = 1;
    out_receipt->continue_focus = result->continue_focus;
    out_receipt->set_party_pose = 1;
    out_receipt->party_x = result->party_x;
    out_receipt->party_y = result->party_y;
    out_receipt->party_dir = result->party_dir;
    out_receipt->set_tick_count = 1;
    out_receipt->tick_count = result->tick_count;
    out_receipt->set_runtime_level_route = 1;
    out_receipt->runtime_level_source =
        result->track02_media_route
            ? THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_MEDIA
            : THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME;
    out_receipt->runtime_track02_semantic_handoff = 0;
    out_receipt->runtime_fallback_visuals_blocked =
        result->track02_media_route;
    out_receipt->runtime_structured_route = 1;
    out_receipt->runtime_receipt_text_route = 0;
    if (result->source == THERON_V1_STARTUP_CONTINUE_SOURCE_SRM) {
        out_receipt->set_save_resume = 1;
        out_receipt->save_resume_claim = THERON_V1_STARTUP_RESUME_SRM;
        out_receipt->save_resume_srm_active_slot =
            result->source_slot_index;
        out_receipt->save_resume_srm_import_status =
            (int)result->srm_import_status;
        out_receipt->save_resume_srm_current_dungeon =
            result->srm_current_dungeon;
        out_receipt->save_resume_srm_current_level =
            result->srm_current_level;
        out_receipt->save_resume_srm_quest_mask =
            result->srm_quest_mask;
        out_receipt->save_resume_srm_party_restored =
            result->srm_party_restored;
        out_receipt->save_resume_srm_party_champion_count =
            result->srm_party_champion_count;
        out_receipt->save_resume_srm_party_gold =
            result->srm_party_gold;
    }
    return 1;
}

int theron_v1_startup_continue_apply_request_with_receipts(
    Theron_V1_World *world,
    const Theron_V1StartupContinueRequest *request,
    const Theron_StartupActionPlan *plan,
    const char *chapter_marker_line,
    Theron_V1StartupContinueResult *out_result,
    Theron_V1StartupContinueApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueResult local_result;
    Theron_V1StartupContinueResult *result =
        out_result ? out_result : &local_result;

    theron_v1_startup_continue_result_init(result);
    if (out_apply_receipt) {
        theron_v1_startup_continue_apply_receipt_init(out_apply_receipt);
    }
    if (out_state_receipt) {
        theron_v1_startup_state_receipt_init(out_state_receipt);
    }
    if (!theron_v1_startup_continue_apply_request(world,
                                                  request,
                                                  result,
                                                  receipt,
                                                  receipt_cap)) {
        if (out_apply_receipt) {
            theron_v1_startup_continue_failure_apply_receipt(
                plan,
                result,
                receipt,
                out_apply_receipt);
        }
        return 0;
    }
    if (out_state_receipt &&
        !theron_v1_startup_continue_state_receipt_from_result(
            result,
            out_state_receipt)) {
        return 0;
    }
    if (out_apply_receipt &&
        !theron_v1_startup_continue_apply_receipt(plan,
                                                  result,
                                                  receipt,
                                                  chapter_marker_line,
                                                  out_apply_receipt)) {
        return 0;
    }
    return 1;
}

int theron_v1_startup_continue_apply_request_with_host_receipts(
    Theron_V1_World *world,
    const Theron_V1StartupContinueRequest *request,
    const Theron_StartupActionPlan *plan,
    const char *chapter_marker_line,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueApplyReceipt apply_receipt;

    theron_v1_startup_continue_apply_receipt_init(&apply_receipt);
    if (out_host_receipt) {
        theron_v1_startup_host_receipt_init(out_host_receipt);
    }
    if (!theron_v1_startup_continue_apply_request_with_receipts(
            world,
            request,
            plan,
            chapter_marker_line,
            out_result,
            &apply_receipt,
            out_state_receipt,
            receipt,
            receipt_cap)) {
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_continue_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }
    if (out_host_receipt &&
        !theron_v1_startup_host_receipt_from_continue_apply(
            &apply_receipt,
            out_host_receipt)) {
        return 0;
    }
    return 1;
}

int theron_v1_startup_continue_apply_request_with_inspect_receipts(
    Theron_V1_World *world,
    const Theron_V1StartupContinueRequest *request,
    const Theron_StartupActionPlan *plan,
    const Theron_StartupChapterInspectRequest *inspect_request,
    Theron_V1StartupContinueResult *out_result,
    Theron_V1StartupContinueApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_StartupChapterInspectReceipt inspect_receipt;
    const char *chapter_marker_line = NULL;

    theron_v1_startup_chapter_inspect_receipt_init(&inspect_receipt);
    if (inspect_request &&
        theron_v1_startup_chapter_inspect_receipt_from_request(
            inspect_request,
            &inspect_receipt) &&
        inspect_receipt.marker_line[0] != '\0') {
        chapter_marker_line = inspect_receipt.marker_line;
    }
    return theron_v1_startup_continue_apply_request_with_receipts(
        world,
        request,
        plan,
        chapter_marker_line,
        out_result,
        out_apply_receipt,
        out_state_receipt,
        receipt,
        receipt_cap);
}

int theron_v1_startup_continue_apply_facts_with_inspect_receipts(
    Theron_V1_World *world,
    Theron_V1StartupResumeClaim resume_claim,
    int tqsv_slot_index,
    const char *tqsv_root,
    int srm_slot_index,
    Theron_V1SrmProgressImportStatus srm_import_status,
    const char *srm_root,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_V1StartupContinueApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueRequest request;
    Theron_StartupChapterInspectRequest inspect_request;

    theron_v1_startup_continue_request_init(&request);
    request.resume_claim = resume_claim;
    request.tqsv_slot_index = tqsv_slot_index;
    request.tqsv_root = tqsv_root;
    request.srm_slot_index = srm_slot_index;
    request.srm_import_status = srm_import_status;
    request.srm_root = srm_root;

    memset(&inspect_request, 0, sizeof(inspect_request));
    inspect_request.boot_profile = boot_profile;
    inspect_request.world = world;
    inspect_request.prefix = receipt;

    return theron_v1_startup_continue_apply_request_with_inspect_receipts(
        world,
        &request,
        plan,
        &inspect_request,
        out_result,
        out_apply_receipt,
        out_state_receipt,
        receipt,
        receipt_cap);
}

int theron_v1_startup_continue_apply_facts_with_host_receipts(
    Theron_V1_World *world,
    Theron_V1StartupResumeClaim resume_claim,
    int tqsv_slot_index,
    const char *tqsv_root,
    int srm_slot_index,
    Theron_V1SrmProgressImportStatus srm_import_status,
    const char *srm_root,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueApplyReceipt apply_receipt;

    theron_v1_startup_continue_apply_receipt_init(&apply_receipt);
    if (out_host_receipt) {
        theron_v1_startup_host_receipt_init(out_host_receipt);
    }
    if (!theron_v1_startup_continue_apply_facts_with_inspect_receipts(
            world,
            resume_claim,
            tqsv_slot_index,
            tqsv_root,
            srm_slot_index,
            srm_import_status,
            srm_root,
            plan,
            boot_profile,
            out_result,
            &apply_receipt,
            out_state_receipt,
            receipt,
            receipt_cap)) {
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_continue_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }
    if (out_host_receipt &&
        !theron_v1_startup_host_receipt_from_continue_apply(
            &apply_receipt,
            out_host_receipt)) {
        return 0;
    }
    return 1;
}

int theron_v1_startup_continue_apply_boot_profile_with_host_receipts(
    Theron_V1_World *world,
    Theron_V1StartupResumeClaim resume_claim,
    int tqsv_slot_index,
    const void *boot_profile,
    int srm_slot_index,
    Theron_V1SrmProgressImportStatus srm_import_status,
    const char *srm_root,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    const Theron_V1_BootProfile *profile =
        (const Theron_V1_BootProfile *)boot_profile;

    if (out_result) {
        theron_v1_startup_continue_result_init(out_result);
    }
    if (!profile) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "missing Theron boot profile");
        }
        if (out_host_receipt) {
            Theron_V1StartupContinueApplyReceipt apply_receipt;
            theron_v1_startup_continue_failure_apply_receipt(
                plan,
                out_result,
                receipt,
                &apply_receipt);
            theron_v1_startup_host_receipt_from_continue_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }
    return theron_v1_startup_continue_apply_facts_with_host_receipts(
        world,
        resume_claim,
        tqsv_slot_index,
        profile->save_root,
        srm_slot_index,
        srm_import_status,
        srm_root,
        plan,
        boot_profile,
        out_result,
        out_host_receipt,
        out_state_receipt,
        receipt,
        receipt_cap);
}

int theron_v1_startup_continue_tqsv_apply_with_host_receipts(
    Theron_V1_World *world,
    const char *save_root,
    int slot_index,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueResult local_result;
    Theron_V1StartupContinueResult *result =
        out_result ? out_result : &local_result;
    Theron_V1StartupContinueApplyReceipt apply_receipt;
    Theron_StartupChapterInspectReceipt inspect_receipt;
    Theron_StartupChapterInspectRequest inspect_request;
    const char *chapter_marker_line = NULL;

    theron_v1_startup_continue_result_init(result);
    theron_v1_startup_continue_apply_receipt_init(&apply_receipt);
    if (out_host_receipt) {
        theron_v1_startup_host_receipt_init(out_host_receipt);
    }
    if (out_state_receipt) {
        theron_v1_startup_state_receipt_init(out_state_receipt);
    }

    if (!theron_v1_startup_continue_tqsv_apply(world,
                                               save_root,
                                               slot_index,
                                               receipt,
                                               receipt_cap)) {
        theron_v1_startup_continue_failure_apply_receipt(
            plan,
            result,
            receipt,
            &apply_receipt);
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_continue_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }

    theron_v1_startup_continue_capture_result(
        world,
        THERON_V1_STARTUP_CONTINUE_SOURCE_TQSV,
        slot_index,
        THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT,
        result);
    if (out_state_receipt &&
        !theron_v1_startup_continue_state_receipt_from_result(
            result,
            out_state_receipt)) {
        return 0;
    }

    theron_v1_startup_chapter_inspect_receipt_init(&inspect_receipt);
    memset(&inspect_request, 0, sizeof(inspect_request));
    inspect_request.boot_profile = boot_profile;
    inspect_request.world = world;
    inspect_request.prefix = receipt;
    if (boot_profile &&
        theron_v1_startup_chapter_inspect_receipt_from_request(
            &inspect_request,
            &inspect_receipt) &&
        inspect_receipt.marker_line[0] != '\0') {
        chapter_marker_line = inspect_receipt.marker_line;
    }
    if (!theron_v1_startup_continue_apply_receipt(plan,
                                                  result,
                                                  receipt,
                                                  chapter_marker_line,
                                                  &apply_receipt)) {
        theron_v1_startup_continue_failure_apply_receipt(
            plan,
            result,
            receipt,
            &apply_receipt);
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_continue_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }
    if (out_host_receipt &&
        !theron_v1_startup_host_receipt_from_continue_apply(
            &apply_receipt,
            out_host_receipt)) {
        return 0;
    }
    return 1;
}

int theron_v1_startup_continue_tqsv_path_apply_with_host_receipts(
    Theron_V1_World *world,
    const char *save_path,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueResult local_result;
    Theron_V1StartupContinueResult *result =
        out_result ? out_result : &local_result;
    Theron_V1StartupContinueApplyReceipt apply_receipt;
    Theron_StartupChapterInspectReceipt inspect_receipt;
    Theron_StartupChapterInspectRequest inspect_request;
    const char *chapter_marker_line = NULL;

    theron_v1_startup_continue_result_init(result);
    theron_v1_startup_continue_apply_receipt_init(&apply_receipt);
    if (out_host_receipt) {
        theron_v1_startup_host_receipt_init(out_host_receipt);
    }
    if (out_state_receipt) {
        theron_v1_startup_state_receipt_init(out_state_receipt);
    }

    if (!theron_v1_startup_continue_tqsv_path_apply(world,
                                                    save_path,
                                                    receipt,
                                                    receipt_cap)) {
        theron_v1_startup_continue_failure_apply_receipt(
            plan,
            result,
            receipt,
            &apply_receipt);
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_continue_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }

    theron_v1_startup_continue_capture_result(
        world,
        THERON_V1_STARTUP_CONTINUE_SOURCE_TQSV,
        -1,
        THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT,
        result);
    if (out_state_receipt &&
        !theron_v1_startup_continue_state_receipt_from_result(
            result,
            out_state_receipt)) {
        return 0;
    }

    theron_v1_startup_chapter_inspect_receipt_init(&inspect_receipt);
    memset(&inspect_request, 0, sizeof(inspect_request));
    inspect_request.boot_profile = boot_profile;
    inspect_request.world = world;
    inspect_request.prefix = receipt;
    if (boot_profile &&
        theron_v1_startup_chapter_inspect_receipt_from_request(
            &inspect_request,
            &inspect_receipt) &&
        inspect_receipt.marker_line[0] != '\0') {
        chapter_marker_line = inspect_receipt.marker_line;
    }
    if (!theron_v1_startup_continue_apply_receipt(plan,
                                                  result,
                                                  receipt,
                                                  chapter_marker_line,
                                                  &apply_receipt)) {
        theron_v1_startup_continue_failure_apply_receipt(
            plan,
            result,
            receipt,
            &apply_receipt);
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_continue_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }
    if (out_host_receipt &&
        !theron_v1_startup_host_receipt_from_continue_apply(
            &apply_receipt,
            out_host_receipt)) {
        return 0;
    }
    return 1;
}

int theron_v1_startup_continue_srm_apply_with_receipts(
    Theron_V1_World *world,
    const char *srm_root,
    int slot_index,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_V1StartupContinueApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueResult local_result;
    Theron_V1StartupContinueResult *result =
        out_result ? out_result : &local_result;
    Theron_V1StartupContinueApplyReceipt apply_receipt;
    Theron_StartupChapterInspectReceipt inspect_receipt;
    Theron_StartupChapterInspectRequest inspect_request;
    const char *chapter_marker_line = NULL;

    theron_v1_startup_continue_result_init(result);
    theron_v1_startup_continue_apply_receipt_init(&apply_receipt);
    if (out_apply_receipt) {
        theron_v1_startup_continue_apply_receipt_init(out_apply_receipt);
    }
    if (out_state_receipt) {
        theron_v1_startup_state_receipt_init(out_state_receipt);
    }

    if (!theron_v1_startup_continue_srm_apply(world,
                                              srm_root,
                                              slot_index,
                                              receipt,
                                              receipt_cap)) {
        theron_v1_startup_continue_failure_apply_receipt(
            plan,
            result,
            receipt,
            &apply_receipt);
        if (out_apply_receipt) {
            *out_apply_receipt = apply_receipt;
        }
        return 0;
    }

    theron_v1_startup_continue_capture_result(
        world,
        THERON_V1_STARTUP_CONTINUE_SOURCE_SRM,
        slot_index,
        THERON_V1_SRM_PROGRESS_IMPORT_OK,
        result);
    if (out_state_receipt &&
        !theron_v1_startup_continue_state_receipt_from_result(
            result,
            out_state_receipt)) {
        return 0;
    }

    theron_v1_startup_chapter_inspect_receipt_init(&inspect_receipt);
    memset(&inspect_request, 0, sizeof(inspect_request));
    inspect_request.boot_profile = boot_profile;
    inspect_request.world = world;
    inspect_request.prefix = receipt;
    if (boot_profile &&
        theron_v1_startup_chapter_inspect_receipt_from_request(
            &inspect_request,
            &inspect_receipt) &&
        inspect_receipt.marker_line[0] != '\0') {
        chapter_marker_line = inspect_receipt.marker_line;
    }
    if (!theron_v1_startup_continue_apply_receipt(plan,
                                                  result,
                                                  receipt,
                                                  chapter_marker_line,
                                                  &apply_receipt)) {
        theron_v1_startup_continue_failure_apply_receipt(
            plan,
            result,
            receipt,
            &apply_receipt);
        if (out_apply_receipt) {
            *out_apply_receipt = apply_receipt;
        }
        return 0;
    }
    if (out_apply_receipt) {
        *out_apply_receipt = apply_receipt;
    }
    return 1;
}

int theron_v1_startup_continue_srm_apply_with_host_receipts(
    Theron_V1_World *world,
    const char *srm_root,
    int slot_index,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueApplyReceipt apply_receipt;

    theron_v1_startup_continue_apply_receipt_init(&apply_receipt);
    if (out_host_receipt) {
        theron_v1_startup_host_receipt_init(out_host_receipt);
    }
    if (!theron_v1_startup_continue_srm_apply_with_receipts(
            world,
            srm_root,
            slot_index,
            plan,
            boot_profile,
            out_result,
            &apply_receipt,
            out_state_receipt,
            receipt,
            receipt_cap)) {
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_continue_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }
    if (out_host_receipt &&
        !theron_v1_startup_host_receipt_from_continue_apply(
            &apply_receipt,
            out_host_receipt)) {
        return 0;
    }
    return 1;
}

int theron_v1_startup_continue_srm_path_apply_with_receipts(
    Theron_V1_World *world,
    const char *srm_path,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_V1StartupContinueApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueResult local_result;
    Theron_V1StartupContinueResult *result =
        out_result ? out_result : &local_result;
    Theron_V1StartupContinueApplyReceipt apply_receipt;
    Theron_StartupChapterInspectReceipt inspect_receipt;
    Theron_StartupChapterInspectRequest inspect_request;
    const char *chapter_marker_line = NULL;

    theron_v1_startup_continue_result_init(result);
    theron_v1_startup_continue_apply_receipt_init(&apply_receipt);
    if (out_apply_receipt) {
        theron_v1_startup_continue_apply_receipt_init(out_apply_receipt);
    }
    if (out_state_receipt) {
        theron_v1_startup_state_receipt_init(out_state_receipt);
    }

    if (!theron_v1_startup_continue_srm_path_apply(world,
                                                   srm_path,
                                                   receipt,
                                                   receipt_cap)) {
        theron_v1_startup_continue_failure_apply_receipt(
            plan,
            result,
            receipt,
            &apply_receipt);
        if (out_apply_receipt) {
            *out_apply_receipt = apply_receipt;
        }
        return 0;
    }

    theron_v1_startup_continue_capture_result(
        world,
        THERON_V1_STARTUP_CONTINUE_SOURCE_SRM,
        -1,
        THERON_V1_SRM_PROGRESS_IMPORT_OK,
        result);
    if (out_state_receipt &&
        !theron_v1_startup_continue_state_receipt_from_result(
            result,
            out_state_receipt)) {
        return 0;
    }

    theron_v1_startup_chapter_inspect_receipt_init(&inspect_receipt);
    memset(&inspect_request, 0, sizeof(inspect_request));
    inspect_request.boot_profile = boot_profile;
    inspect_request.world = world;
    inspect_request.prefix = receipt;
    if (boot_profile &&
        theron_v1_startup_chapter_inspect_receipt_from_request(
            &inspect_request,
            &inspect_receipt) &&
        inspect_receipt.marker_line[0] != '\0') {
        chapter_marker_line = inspect_receipt.marker_line;
    }
    if (!theron_v1_startup_continue_apply_receipt(plan,
                                                  result,
                                                  receipt,
                                                  chapter_marker_line,
                                                  &apply_receipt)) {
        theron_v1_startup_continue_failure_apply_receipt(
            plan,
            result,
            receipt,
            &apply_receipt);
        if (out_apply_receipt) {
            *out_apply_receipt = apply_receipt;
        }
        return 0;
    }
    if (out_apply_receipt) {
        *out_apply_receipt = apply_receipt;
    }
    return 1;
}

int theron_v1_startup_continue_srm_path_apply_with_host_receipts(
    Theron_V1_World *world,
    const char *srm_path,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap) {

    Theron_V1StartupContinueApplyReceipt apply_receipt;

    theron_v1_startup_continue_apply_receipt_init(&apply_receipt);
    if (out_host_receipt) {
        theron_v1_startup_host_receipt_init(out_host_receipt);
    }
    if (!theron_v1_startup_continue_srm_path_apply_with_receipts(
            world,
            srm_path,
            plan,
            boot_profile,
            out_result,
            &apply_receipt,
            out_state_receipt,
            receipt,
            receipt_cap)) {
        if (out_host_receipt) {
            theron_v1_startup_host_receipt_from_continue_apply(
                &apply_receipt,
                out_host_receipt);
        }
        return 0;
    }
    if (out_host_receipt &&
        !theron_v1_startup_host_receipt_from_continue_apply(
            &apply_receipt,
            out_host_receipt)) {
        return 0;
    }
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
                     "srm_first_decoded:%d\n"
                     "srm_envelope:     %s\n"
                     "srm_payload_probe:%s ran=%d size=%zu fstq_magic=%d\n"
                     "srm_prog_import:  %s ran=%d dungeon=%d level=%d quest=0x%02x\n"
                     "srm_party_import: ran=%d restored=%d champions=%d gold=%u\n",
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
                     snap->srm_first_decoded_slot,
                     theron_v1_srm_envelope_kind_name(snap->srm_envelope_kind),
                     theron_v1_srm_payload_probe_status_name(snap->srm_payload_probe_status),
                     snap->srm_payload_probe_ran,
                     snap->srm_payload_size,
                     snap->srm_payload_hits_fstq_magic,
                     theron_v1_srm_progress_import_status_name(snap->srm_progress_import_status),
                     snap->srm_progress_import_ran,
                     snap->srm_progress_current_dungeon,
                     snap->srm_progress_current_level,
                     snap->srm_progress_quest_mask >= 0
                         ? snap->srm_progress_quest_mask : 0,
                     snap->srm_party_import_ran,
                     snap->srm_party_restored,
                     snap->srm_party_champion_count,
                     snap->srm_party_gold);

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
