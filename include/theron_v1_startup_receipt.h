#ifndef THERON_V1_STARTUP_RECEIPT_H
#define THERON_V1_STARTUP_RECEIPT_H

/*
 * theron_v1_startup_receipt.h — Theron's Quest V1 startup real-asset receipt.
 *
 * A bounded, hashes-only receipt surface that proves a Theron Track 02
 * launch reached the existing boot/M11 handoff without claiming playability
 * and without committing any game data to the repository.  The receipt is
 * split into a deterministic no-data placeholder and an optional real-asset
 * path that the upstream M12 asset catalog (or a focused probe) can supply.
 *
 * Source/lock:
 *   - src/theron/theron_v1_track02.c       — Track 02 bank-signal decoder
 *   - src/theron/theron_v1_boot.c           — boot profile + direct launch
 *   - include/asset_status_m12.h m12_file_md5_hex — file-MD5 helper
 *   - src/shared/asset_status_m12.c g_theronVersions — known Track 02 MD5s
 *   - THQUEST.ASM T000 (startup), T080 (save ns), T400 (data track load),
 *     T520 (party placement), T560 (dungeon load), T800 (champion persist)
 *   - ReDMCSB has no Theron code; the Theron-side evidence is local byte
 *     inspection of the four cataloged Track 02 MD5s
 *   - docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *   - docs/source-lock/tqr_v1_phase1_boot_H2338.md
 *   - docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md
 *
 * This receipt deliberately does NOT:
 *   - copy Track 02 bytes anywhere tracked (only MD5 hashes live here)
 *   - claim a real Track 02 window is a decoded dungeon record
 *   - tick the gameplay loop beyond the M11 init step
 *   - claim screenshot or playability promotion
 */

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_boot.h"
#include "theron_v1_track02.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum MD5 hex length including the trailing NUL. */
#define THERON_V1_STARTUP_RECEIPT_MD5_HEX_LEN 33u

/* Maximum human-readable variant label length (incl. NUL). */
#define THERON_V1_STARTUP_RECEIPT_VARIANT_LABEL_LEN 32u

/* Maximum path length stored in the receipt (incl. NUL). */
#define THERON_V1_STARTUP_RECEIPT_PATH_LEN         512u

/* Maximum source-evidence citation length (incl. NUL). */
#define THERON_V1_STARTUP_RECEIPT_EVIDENCE_LEN     512u

/* Receipt verdict.
 *
 *   NO_DATA_PLACEHOLDER — deterministic shape produced when no Track 02
 *      file was supplied.  Used by CI / no-data hosts as the green path.
 *
 *   REAL_ASSET_RECEIPT  — a hash-verified Track 02 file matched one of the
 *      four known MD5s, the boot-profile direct launch succeeded, and the
 *      Track 02 bank-signal decoder either confirmed the known
 *      descriptor/span anchors (raw BIN variants) or the JP Rev 1 zero-fill
 *      contract (zero-filled ISO variant).  Never set without an actual
 *      file/MD5 round-trip.
 *
 *   SKIPPED             — caller asked to probe a file that does not exist,
 *      does not match a known MD5, or is too small to decode.  Always
 *      accompanied by a human-readable note so the receipt is honest.
 *
 *   REJECTED            — the file/MD5 pair failed the direct-launch contract
 *      (unknown MD5, empty path, NULL inputs).  The probe still emits a
 *      receipt so the audit trail records the rejection.
 *
 * This enum is intentionally small and stable; new verdicts must append,
 * not reorder, so external tools can switch on the integer value. */
typedef enum {
    THERON_V1_STARTUP_RECEIPT_NO_DATA_PLACEHOLDER = 1,
    THERON_V1_STARTUP_RECEIPT_REAL_ASSET_RECEIPT  = 2,
    THERON_V1_STARTUP_RECEIPT_SKIPPED             = 3,
    THERON_V1_STARTUP_RECEIPT_REJECTED            = 4
} Theron_V1_StartupReceiptVerdict;

/* Receipt payload.
 *
 * All strings are NUL-terminated.  All sizes are bytes.  The struct is
 * deterministic across runs when given the same inputs (no clock, no
 * random state).  The `session_tick_token` is a 32-bit FNV-1a hash of the
 * other fields; it lets log scrapers spot duplicates without parsing
 * every byte.
 *
 * Field semantics:
 *   - verdict / verdict_name           : enum + stable string
 *   - variant / variant_name           : Theron Track 02 variant enum
 *   - track02_path                     : verified Track 02 path or "" (placeholder)
 *   - track02_md5_hex                  : 32-char hex MD5 or "" (placeholder)
 *   - track02_byte_count               : Track 02 file size in bytes, 0 for placeholder
 *   - descriptor_offset                : Track 02 bank-signal descriptor offset
 *   - descriptor_size                  : Track 02 bank-signal descriptor size
 *   - descriptor_value_count           : number of LE words in the descriptor table
 *   - descriptor_stride                : documented 0x0400 stride
 *   - anchor_count                     : number of bank-signal anchors
 *   - post_boundary_span_offset        : first post-boundary span offset (0 if N/A)
 *   - post_boundary_span_size          : 44 for raw BIN / ISO when matched
 *   - next_nonzero_offset              : first nonzero byte after descriptor
 *   - initial_candidate_*              : hash/anchor-gated startup level
 *                                        candidate receipt when raw Track 02
 *                                        exposes the bounded 32x27 payload
 *   - boot_profile_platform            : detected platform enum
 *   - boot_profile_version_id          : copy of profile->version_id
 *   - boot_profile_assets_verified     : 1 when the direct launch marked assets_verified
 *   - boot_profile_tick_rate_hz        : deterministic tick rate (integer Hz)
 *   - boot_profile_max_champions       : TQ party size from the boot profile
 *   - boot_profile_dungeon_count       : 7 (TQ mini-dungeon count)
 *   - boot_profile_dungeon_seed        : dungeon_seed word from the boot profile
 *   - m11_dispatch_source_kind         : M11_GameSourceKind value used by M11_Start
 *                                        (or -1 if M11 was not dispatched)
 *   - m11_view_active                  : 1 after M11_GameView_Start returns true
 *   - m11_world_present                : 1 when the boot profile + Theron world both exist
 *   - skip_reason_note                 : human-readable skip reason ("no data on host")
 *   - source_evidence                  : short citation blob (module name + MD5s)
 *   - session_tick_token               : FNV-1a over the rest of the receipt
 */
typedef struct {
    /* ── verdict / variant ───────────────────────────────────── */
    Theron_V1_StartupReceiptVerdict verdict;
    char     verdict_name[THERON_V1_STARTUP_RECEIPT_VARIANT_LABEL_LEN];
    Theron_Track02Variant variant;
    char     variant_name[THERON_V1_STARTUP_RECEIPT_VARIANT_LABEL_LEN];

    /* ── track02 file receipt (hashes only, never bytes) ──────── */
    char     track02_path[THERON_V1_STARTUP_RECEIPT_PATH_LEN];
    char     track02_md5_hex[THERON_V1_STARTUP_RECEIPT_MD5_HEX_LEN];
    uint64_t track02_byte_count;

    /* ── Track 02 bank-signal summary ─────────────────────────── */
    uint64_t descriptor_offset;
    uint64_t descriptor_size;
    uint64_t descriptor_value_count;
    uint32_t descriptor_stride;
    uint64_t anchor_count;
    uint64_t post_boundary_span_offset;
    uint64_t post_boundary_span_size;
    uint64_t next_nonzero_offset;

    /* ── Track 02 startup-level candidate summary ─────────────── */
    int      initial_candidate_found;
    uint64_t initial_candidate_offset;
    uint64_t initial_candidate_size;
    uint32_t initial_candidate_width;
    uint32_t initial_candidate_height;
    uint32_t initial_candidate_seed;
    uint32_t initial_candidate_level_index;
    int      initial_candidate_start_x;
    int      initial_candidate_start_y;
    int      initial_candidate_start_dir;
    uint64_t initial_candidate_descriptor_delta;
    int      initial_candidate_anchor_match;

    /* ── boot profile summary (after direct launch) ───────────── */
    Theron_Platform boot_profile_platform;
    char            boot_profile_version_id[16];
    int             boot_profile_assets_verified;
    uint32_t        boot_profile_tick_rate_hz;
    uint32_t        boot_profile_max_champions;
    uint32_t        boot_profile_dungeon_count;
    uint32_t        boot_profile_dungeon_seed;

    /* ── M11 dispatch summary (only populated for real-asset path) ── */
    int32_t         m11_dispatch_source_kind;
    int             m11_view_active;
    int             m11_world_present;

    /* ── human-readable notes ─────────────────────────────────── */
    char     skip_reason_note[THERON_V1_STARTUP_RECEIPT_EVIDENCE_LEN];
    char     source_evidence[THERON_V1_STARTUP_RECEIPT_EVIDENCE_LEN];

    /* ── 32-bit FNV-1a hash of all the fields above; recomputed on emit ── */
    uint32_t session_tick_token;
} Theron_V1_StartupReceipt;

/* ── API ──────────────────────────────────────────────────────────── */

/* Reset the receipt to its deterministic no-data placeholder state.
 * Always succeeds; safe to call with NULL (returns immediately). */
void theron_v1_startup_receipt_reset(Theron_V1_StartupReceipt *receipt);

/* Set the receipt to its deterministic no-data placeholder.  This is the
 * CI / no-data host path: no Track 02 file is consulted, no M11
 * dispatch happens, and the verdict is NO_DATA_PLACEHOLDER.  The
 * source_evidence field is filled with the locked module + MD5 citations
 * so the placeholder is auditable.
 *
 * Always succeeds; safe to call with NULL. */
void theron_v1_startup_receipt_set_placeholder(Theron_V1_StartupReceipt *receipt);

/* Compute the deterministic FNV-1a session_tick_token for a receipt.
 * The token is stable across runs given identical inputs.
 * Returns the token (also stored into receipt->session_tick_token). */
uint32_t theron_v1_startup_receipt_session_tick(const Theron_V1_StartupReceipt *receipt);

/* Look up a path/MD5 pair without reading the file.
 * Returns 1 if expected_md5 matches one of the four known TQ Track 02
 * MD5s (asset_status_m12.c::g_theronVersions), 0 otherwise.  Pure logic;
 * no filesystem access. */
int theron_v1_startup_receipt_md5_is_known(const char *expected_md5);

/* Produce a real-asset receipt against a Track 02 file on disk.
 *
 *   track02_path : absolute or relative path to the Track 02 file.
 *                  When NULL/empty or missing on disk, the receipt is
 *                  downgraded to NO_DATA_PLACEHOLDER with a clear note.
 *   expected_md5 : 32-char hex MD5 the caller already trusts.
 *                  When NULL/empty/unknown, the receipt is REJECTED.
 *   receipt      : output.  Caller-initialized; all fields overwritten.
 *
 * Returns 1 when the verdict is REAL_ASSET_RECEIPT (file existed, MD5
 * matched, direct-launch succeeded, bank-signal decoder consumed the
 * bytes and emitted anchors or a JP Rev 1 zero-fill verdict).
 *
 * Returns 0 in every other case; the receipt is still filled with a
 * deterministic verdict so the audit trail is consistent. */
int theron_v1_startup_receipt_from_file(const char *track02_path,
                                         const char *expected_md5,
                                         Theron_V1_StartupReceipt *receipt);

/* Render a receipt as a single-line JSON-shaped ASCII string.  Suitable
 * for `tee`-friendly logs and CI verdict lines.  Caller-owned buffer.
 * Returns the number of bytes written (excluding NUL), or 0 on bad input. */
size_t theron_v1_startup_receipt_to_line(const Theron_V1_StartupReceipt *receipt,
                                          char *buf, size_t buf_size);

/* ── Source evidence ─────────────────────────────────────────────── */
const char *theron_v1_startup_receipt_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_STARTUP_RECEIPT_H */
