#ifndef THERON_V1_STARTUP_SAVE_RESUME_H
#define THERON_V1_STARTUP_SAVE_RESUME_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_save_load.h"
#include "theron_v1_srm_classifier.h"
#include "theron_v1_startup_flow.h"
#include "theron_v1_world.h"

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 startup save/resume smoke gate
 *
 * Bounded handoff between the boot profile, the bounded SRM (Save Disk)
 * classifier, and the bounded Firestaff-native .tqsv slot enumerator.
 *
 * What this module does:
 *   - Resolves both save roots (in-game .tqsv + Save Disk .srm) from
 *     the boot profile and/or env overrides so the launch path can
 *     read a single, deterministic save/resume state at startup.
 *   - Classifies staged .srm (Save Disk) and .tqsv (in-game) slots
 *     into a single Theron_V1StartupSaveResume snapshot:
 *       * SOURCES_LIVE             — at least one save slot usable.
 *       * SKIP_SAFE_NO_SAVE_ROOT   — neither root exists; the
 *                                    startup path must keep the game
 *                                    paused at the dungeon title, not
 *                                    silently auto-resume.
 *       * SKIP_SAVE_ROOT_PRESENT_NO_SLOTS — root exists, all slots
 *                                    are empty; treat as fresh boot.
 *       * SKIP_STAGED_NO_RECOGNIZED_SLOT — files exist but neither
 *                                    the .srm gzip-deflate path nor a
 *                                    .tqsv with valid TQR header/
 *                                    footer recognized them.
 *   - Computes a bounded resume claim (the maximum a launchable
 *     gate can prove without a real .srm body decode):
 *       * 0   = NO_RESUME_CLAIM   (clean fresh boot, no auto-resume)
 *       * 1   = TQSV_RESUME_CLAIM (a valid .tqsv slot is present;
 *                                 the gate can offer "Continue" but
 *                                 it does not auto-resume mid-run)
 *       * 2   = SRM_RESUME_CLAIM  (a recognized .srm Save Disk
 *                                 slot is present; the gate can
 *                                 enumerate it but still does not
 *                                 auto-resume without an explicit
 *                                 THQUEST.ASM T080 load routine)
 *       * 3   = DUAL_RESUME_CLAIM (both sources have at least one
 *                                 usable slot; the gate keeps both
 *                                 lists visible)
 *     The resume-claim level is intentionally narrow: this gate does
 *     not pick a slot, does not decode a real Sphenx/Greatstone
 *     custom save body, and does not promote any auto-resume into
 *     runtime state.
 *   - When zlib is built in, additionally attempts to inflate a
 *     recognized .srm payload and applies the bounded
 *     Theron_V1SrmProgressImportStatus decode.  Unknown real
 *     payloads stay UNSUPPORTED_BODY (no claim).
 *
 * What this module does NOT do (kept honest):
 *   - It does not decode a real Sphenx/Greatstone custom save body.
 *     Unknown real .srm payloads stay UNSUPPORTED_BODY and are not
 *     promoted into runtime state.
 *   - It does not auto-resume the game. The gate only reports the
 *     highest-bounded resume claim; the M12/M11 startup layer still
 *     owns the explicit "Continue" UX.
 *   - It does not claim screenshot or playability parity.
 *
 * Source/evidence:
 *   - THQUEST.ASM T080  — between-dungeon save/load
 *   - THQUEST.ASM T800  — champion persistence between dungeons
 *   - docs/DMWEB_REFERENCE.md §6 'Theron's Quest savegame format'
 *     (greatstone: gzipped custom format with a header; "completely
 *     different" from DM).
 *   - dmweb community docs credit Sphenx with several custom
 *     Theron's Quest save games documented at greatstone; Sphenx is
 *     also credited alongside kentaro.k-21 on the SKWIN DM2 skproject.
 * ══════════════════════════════════════════════════════════════════════ */

/* Resume-claim levels (stable integer contract for receipts). */
typedef enum {
    THERON_V1_STARTUP_RESUME_NONE = 0,
    THERON_V1_STARTUP_RESUME_TQSV = 1,
    THERON_V1_STARTUP_RESUME_SRM = 2,
    THERON_V1_STARTUP_RESUME_DUAL = 3
} Theron_V1StartupResumeClaim;

/* Skip-safe verdict (stable integer contract). */
typedef enum {
    THERON_V1_STARTUP_SOURCES_LIVE = 0,
    THERON_V1_STARTUP_SKIP_SAFE_NO_SAVE_ROOT = 1,
    THERON_V1_STARTUP_SKIP_SAVE_ROOT_PRESENT_NO_SLOTS = 2,
    THERON_V1_STARTUP_SKIP_STAGED_NO_RECOGNIZED_SLOT = 3
} Theron_V1StartupSkipSafeVerdict;

/* Bounded snapshot. Stable contract: integer fields + NUL-terminated
 * string fields only.  No pointers, no arena ownership. */
typedef struct {
    /* ── Resolved roots (NUL-terminated, max length 512 incl. NUL) ─ */
    char tqsv_root[THERON_V1_SRM_PATH_MAX];
    char srm_root[THERON_V1_SRM_PATH_MAX];

    /* ── Verdict ── */
    Theron_V1StartupSkipSafeVerdict verdict;
    Theron_V1StartupResumeClaim    resume_claim;

    /* ── .tqsv (Firestaff-native in-game saves) rollup ── */
    int tqsv_total_slots;          /* 0..THERON_SAVE_SLOT_COUNT */
    int tqsv_valid_slots;          /* 0..total */
    int tqsv_active_slot;          /* first valid slot, or -1 */
    uint32_t tqsv_active_timestamp;/* 0 if no valid slot */

    /* ── .srm (Save Disk cartridge) rollup ── */
    int srm_total_slots;           /* 0..THERON_V1_SRM_DISK_SLOT_COUNT */
    int srm_present_slots;         /* 0..total */
    int srm_recognized_slots;      /* 0..total */
    int srm_first_recognized_slot; /* 0..4, or -1 */
    uint32_t srm_first_recognized_checksum32; /* 0 if no recognized slot */

    /* ── Bounded payload probe (zlib only) ── */
    Theron_V1SrmPayloadProbeStatus  srm_payload_probe_status;
    int                             srm_payload_probe_ran; /* 1 if zlib ran */
    size_t                          srm_payload_size;     /* 0 if none */
    int                             srm_payload_hits_fstq_magic;
    Theron_V1SrmEnvelopeKind         srm_envelope_kind;

    /* ── Bounded progression decode (FSTQPRG1 envelope) ── */
    Theron_V1SrmProgressImportStatus srm_progress_import_status;
    int                              srm_progress_import_ran;
    int                              srm_progress_current_dungeon; /* -1 if none */
    int                              srm_progress_current_level;   /* -1 if none */
    int                              srm_progress_quest_mask;      /* -1 if none */
    int                              srm_party_import_ran;
    int                              srm_party_restored;
    int                              srm_party_champion_count;
    uint32_t                         srm_party_gold;

    /* ── Receipt marker ── */
    char verdict_name[40];
    char resume_claim_name[40];
} Theron_V1StartupSaveResume;

typedef enum {
    THERON_V1_STARTUP_CONTINUE_SOURCE_NONE = 0,
    THERON_V1_STARTUP_CONTINUE_SOURCE_TQSV = 1,
    THERON_V1_STARTUP_CONTINUE_SOURCE_SRM = 2
} Theron_V1StartupContinueSource;

typedef struct {
    Theron_V1StartupResumeClaim resume_claim;
    int tqsv_slot_index;
    const char *tqsv_root;
    int srm_slot_index;
    Theron_V1SrmProgressImportStatus srm_import_status;
    const char *srm_root;
} Theron_V1StartupContinueRequest;

typedef struct {
    Theron_V1StartupContinueSource source;
    int source_slot_index;
    Theron_V1SrmProgressImportStatus srm_import_status;
    Theron_DungeonID selected_dungeon;
    int level_loaded;
    int startup_cursor;
    int continue_focus;
    int party_x;
    int party_y;
    int party_dir;
    int srm_current_dungeon;
    int srm_current_level;
    int srm_quest_mask;
    int srm_party_restored;
    int srm_party_champion_count;
    uint32_t srm_party_gold;
    int tick_count;
} Theron_V1StartupContinueResult;

typedef struct {
    Theron_StartupInputResult input_result;
    Theron_V1StartupContinueSource source;
    int source_slot_index;
    Theron_V1SrmProgressImportStatus srm_import_status;
    int srm_current_dungeon;
    int srm_current_level;
    int srm_quest_mask;
    int srm_party_restored;
    int srm_party_champion_count;
    uint32_t srm_party_gold;
    const char *status_scope;
    const char *status;
    const char *inspect_scope;
    char inspect_detail[320];
} Theron_V1StartupContinueApplyReceipt;

typedef struct {
    int has_tqsv_continue;
    int tqsv_slot;
    int has_srm_continue;
    int srm_slot;
    int has_any_continue;
} Theron_V1StartupContinueAvailability;

/* Resolve both save roots deterministically:
 *   tqsv_root: profile->save_root when non-empty, otherwise
 *              theron_v1_save_default_root() result.
 *   srm_root:  env override `FIRESTAFF_THERON_SRM_DIR` if non-empty,
 *              otherwise `$HOME/.firestaff/data/theron/save`, then
 *              `./theron-save`.
 *
 * `boot_save_root` may be NULL or empty; it is consulted only when
 * non-empty.  `out_snapshot` is always populated (zeroed first) so
 * the call is total over its inputs. */
int theron_v1_startup_save_resume_evaluate(
    const char *boot_save_root,
    Theron_V1StartupSaveResume *out_snapshot);
int theron_v1_startup_save_resume_apply_explicit_path(
    Theron_V1StartupSaveResume *snapshot,
    const char *save_path,
    const char *tqsv_root);
int theron_v1_startup_save_resume_state_receipt(
    const Theron_V1StartupSaveResume *snapshot,
    int snapshot_ready,
    Theron_StartupStateReceipt *out_receipt);

/* Apply an explicit startup Continue request into a Theron world.
 * These helpers own the save/SRM decode and between-dungeon world reset;
 * the caller still owns UI state, status text, and subsequent level load. */
int theron_v1_startup_continue_tqsv_apply(
    Theron_V1_World *world,
    const char *save_root,
    int slot_index,
    char *receipt,
    size_t receipt_cap);
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
    size_t receipt_cap);
int theron_v1_startup_continue_tqsv_path_apply(
    Theron_V1_World *world,
    const char *save_path,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_continue_tqsv_path_apply_with_host_receipts(
    Theron_V1_World *world,
    const char *save_path,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_continue_srm_apply(
    Theron_V1_World *world,
    const char *srm_root,
    int slot_index,
    char *receipt,
    size_t receipt_cap);
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
    size_t receipt_cap);
int theron_v1_startup_continue_srm_path_apply(
    Theron_V1_World *world,
    const char *srm_path,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_continue_srm_path_apply_with_receipts(
    Theron_V1_World *world,
    const char *srm_path,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_V1StartupContinueApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_continue_srm_path_apply_with_host_receipts(
    Theron_V1_World *world,
    const char *srm_path,
    const Theron_StartupActionPlan *plan,
    const void *boot_profile,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
void theron_v1_startup_continue_request_init(
    Theron_V1StartupContinueRequest *request);
void theron_v1_startup_continue_result_init(
    Theron_V1StartupContinueResult *result);
void theron_v1_startup_continue_apply_receipt_init(
    Theron_V1StartupContinueApplyReceipt *receipt);
int theron_v1_startup_host_receipt_from_continue_apply(
    const Theron_V1StartupContinueApplyReceipt *apply_receipt,
    Theron_StartupHostReceipt *out_receipt);
void theron_v1_startup_continue_availability_init(
    Theron_V1StartupContinueAvailability *availability);
int theron_v1_startup_continue_availability_from_state(
    Theron_V1StartupResumeClaim resume_claim,
    int tqsv_slot_index,
    int srm_slot_index,
    Theron_V1SrmProgressImportStatus srm_import_status,
    Theron_V1StartupContinueAvailability *out_availability);
int theron_v1_startup_continue_apply_request(
    Theron_V1_World *world,
    const Theron_V1StartupContinueRequest *request,
    Theron_V1StartupContinueResult *out_result,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_continue_apply_receipt(
    const Theron_StartupActionPlan *plan,
    const Theron_V1StartupContinueResult *result,
    const char *continue_receipt,
    const char *chapter_marker_line,
    Theron_V1StartupContinueApplyReceipt *out_receipt);
int theron_v1_startup_continue_state_receipt_from_result(
    const Theron_V1StartupContinueResult *result,
    Theron_StartupStateReceipt *out_receipt);
int theron_v1_startup_continue_apply_request_with_receipts(
    Theron_V1_World *world,
    const Theron_V1StartupContinueRequest *request,
    const Theron_StartupActionPlan *plan,
    const char *chapter_marker_line,
    Theron_V1StartupContinueResult *out_result,
    Theron_V1StartupContinueApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_continue_apply_request_with_host_receipts(
    Theron_V1_World *world,
    const Theron_V1StartupContinueRequest *request,
    const Theron_StartupActionPlan *plan,
    const char *chapter_marker_line,
    Theron_V1StartupContinueResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_continue_apply_request_with_inspect_receipts(
    Theron_V1_World *world,
    const Theron_V1StartupContinueRequest *request,
    const Theron_StartupActionPlan *plan,
    const Theron_StartupChapterInspectRequest *inspect_request,
    Theron_V1StartupContinueResult *out_result,
    Theron_V1StartupContinueApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
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
    size_t receipt_cap);
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
    size_t receipt_cap);
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
    size_t receipt_cap);

/* Pretty-printer. Writes a multi-line diagnostic to `buf` up to
 * `buf_size` bytes (always NUL-terminated).  Returns bytes written
 * excluding the NUL terminator. */
size_t theron_v1_startup_save_resume_format(
    const Theron_V1StartupSaveResume *snapshot,
    char *buf,
    size_t buf_size);

/* Stable string contracts. */
const char *theron_v1_startup_save_resume_skip_safe_name(
    Theron_V1StartupSkipSafeVerdict verdict);
const char *theron_v1_startup_save_resume_claim_name(
    Theron_V1StartupResumeClaim claim);

/* Source evidence citation. */
const char *theron_v1_startup_save_resume_source_evidence(void);

#endif /* THERON_V1_STARTUP_SAVE_RESUME_H */
