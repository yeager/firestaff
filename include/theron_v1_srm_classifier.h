#ifndef THERON_V1_SRM_CLASSIFIER_H
#define THERON_V1_SRM_CLASSIFIER_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_champions.h"
#include "theron_v1_dungeon_progression.h"

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 SRM (Save RAM) classifier — bounded real-artifact boundary.
 *
 * Source/evidence:
 *   - DMWeb reference, section "Theron's Quest savegame format":
 *       "greatstone has a section on Theron's Quest save games. TQ's
 *        save format is *completely different* from DM's (it uses
 *        gzipped custom format with a header). We should look at this
 *        when implementing src/theron/ V1 save/load."
 *   - dmweb community docs `community/documentation/` page credits
 *     Sphenx with several custom Theron's Quest save games documented
 *     at greatstone; Sphenx is also credited alongside kentaro.k-21
 *     on the SKWIN DM2 skproject.
 *   - The original PC Engine CD "Save Disk" cartridge stores 5 named
 *     disk slots (TQR uses a 5-slot disk save model per Sphenx's
 *     TQ-RTC tool chain). Firestaff keeps a 5-slot disk manifest
 *     here, not a slot-N.tqsv manifest (which is the synthetic native
 *     in-game save format already covered by theron_v1_save_load.c).
 *
 * What this module does:
 *   - Enumerates up to THERON_V1_SRM_DISK_SLOT_COUNT disk slots under
 *     a save-disk root (default `~/.firestaff/data/theron/save/`,
 *     override `FIRESTAFF_THERON_SRM_DIR`).
 *   - For each slot path `slotN.srm`, reads a small prefix and reports:
 *       * present and gzipped → PRESENT_AND_RECOGNIZED
 *       * present, not gzipped → UNRECOGNIZED (could be Sphenx's raw
 *         format, must stay non-launchable until decoded)
 *       * present, gzipped prefix but truncated → MALFORMED
 *       * missing → ABSENT
 *   - Computes a 32-bit rolling checksum over the first 1 KiB of each
 *     file (or whole file if smaller) for receipt/manifest purposes.
 *   - Inflates gzip-wrapped DEFLATE payloads when zlib is present.
 *   - Imports Firestaff-only readiness envelopes into progression and
 *     bounded champion body state for tests/probes.
 *   - Probes the recognized-slot path with bounded full-file reads so a
 *     gzipped `.srm` body can reach one decoded champion block when the
 *     body uses the Firestaff-only readiness envelope.
 *
 * What this module does NOT do (kept honest):
 *   - It does not decode the real Sphenx/Greatstone custom save body.
 *     Unknown payloads stay UNSUPPORTED_BODY and non-launchable.
 *   - It does not import real inventory/equipment bytes; the party-body
 *     readiness gate restores stats, condition fields, food/water, and
 *     shared gold only.
 *   - It does not promote any public screenshot or claim playability.
 *   - It does not claim full Sphenx-format coverage. Unknown `.srm`
 *     files that lack the gzip magic stay UNRECOGNIZED.
 *
 * Track 02 + Track 02 SRM interplay:
 *   - Theron's Quest has a strict "no in-dungeon saves" design rule
 *     (THQUEST.ASM T080). The real save model is the Save Disk
 *     cartridge, written only at dungeon entrance. This module is the
 *     bounded real-artifact counterpart to the synthetic in-game
 *     theron_v1_save_load.c slot model.
 * ══════════════════════════════════════════════════════════════════════ */

#define THERON_V1_SRM_DISK_SLOT_COUNT    5
#define THERON_V1_SRM_SLOT_NAME_MAX      64
#define THERON_V1_SRM_PATH_MAX           512
#define THERON_V1_SRM_PREFIX_BYTES       1024
#define THERON_V1_SRM_GZIP_MAGIC_0       0x1F
#define THERON_V1_SRM_GZIP_MAGIC_1       0x8B
#define THERON_V1_SRM_GZIP_MAGIC_2_DEFLATE 0x08

/* Maximum inflated Theron save body to materialize in a single bounded
 * inflate.  This is the work budget for the body-decode fixture; it is
 * deliberately modest so probe/process memory stays bounded, and it is
 * independent of the per-champion record table inside
 * theron_v1_srm_decode_progression_party_payload.  Real bodies that
 * exceed this budget return THERON_V1_SRM_PAYLOAD_PROBE_OUTPUT_TRUNCATED
 * rather than silently growing the heap. */
#define THERON_V1_SRM_BODY_DECODE_MAX_BYTES 4096u

/* Maximum compressed Save Disk member accepted by the opaque transfer path.
 * This is a file-I/O work bound only; it does not claim a real body size or
 * field layout.  The member must still inflate and trailer-authenticate
 * within THERON_V1_SRM_BODY_DECODE_MAX_BYTES before it can be copied. */
#define THERON_V1_SRM_CONTAINER_TRANSFER_MAX_BYTES 65536u

/* Retained change runs in an authenticated two-corpus body comparison.
 * The receipt is deliberately bounded: it profiles byte topology without
 * retaining or assigning semantics to a whole unknown Save Disk body. */
#define THERON_V1_SRM_BODY_DELTA_MAX_RUNS 32u

/* Per-slot classification.  Order is stable; integer values are part of
 * the public manifest contract for downstream receipt tooling. */
typedef enum {
    THERON_V1_SRM_SLOT_ABSENT = 0,
    THERON_V1_SRM_SLOT_UNRECOGNIZED = 1,
    THERON_V1_SRM_SLOT_MALFORMED = 2,
    THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED = 3
} Theron_V1SrmSlotStatus;

typedef struct {
    int slot_index;                                /* 0..4 */
    Theron_V1SrmSlotStatus status;
    char path[THERON_V1_SRM_PATH_MAX];             /* resolved path */
    uint64_t size_bytes;                           /* 0 if ABSENT */
    uint32_t prefix_checksum32;                    /* rolling sum over the first
                                                      THERON_V1_SRM_PREFIX_BYTES
                                                      bytes (or whole file if
                                                      smaller); 0 if ABSENT */
    int gzip_magic_seen;                           /* 1 if first 2 bytes were 0x1F 0x8B */
    int gzip_deflate_method_seen;                  /* 1 if byte 2 is DEFLATE (0x08) */
} Theron_V1SrmSlotInfo;

typedef struct {
    char root[THERON_V1_SRM_PATH_MAX];
    int slot_count;                                /* always THERON_V1_SRM_DISK_SLOT_COUNT */
    int present_count;                             /* 0..5 */
    int recognized_count;                          /* 0..5 */
    Theron_V1SrmSlotInfo slots[THERON_V1_SRM_DISK_SLOT_COUNT];
    int root_resolved;                             /* 1 if root was non-NULL/non-empty */
} Theron_V1SrmManifest;

typedef enum {
    THERON_V1_SRM_PAYLOAD_PROBE_OK = 1,
    THERON_V1_SRM_PAYLOAD_PROBE_ZLIB_UNAVAILABLE = 0,
    THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT = -1,
    THERON_V1_SRM_PAYLOAD_PROBE_NOT_GZIP = -2,
    THERON_V1_SRM_PAYLOAD_PROBE_UNSUPPORTED_METHOD = -3,
    THERON_V1_SRM_PAYLOAD_PROBE_INFLATE_FAILED = -4,
    THERON_V1_SRM_PAYLOAD_PROBE_OUTPUT_TRUNCATED = -5,
    /* The raw DEFLATE stream inflated, but the gzip trailer did not
     * authenticate the resulting body.  Do not treat this as evidence for
     * any body layout or permit it to reach an envelope decoder. */
    THERON_V1_SRM_PAYLOAD_PROBE_TRAILER_MISMATCH = -6,
    /* Save Disk slots are one complete gzip member.  A valid first member
     * followed by bytes or another member is not an authenticated SRM body. */
    THERON_V1_SRM_PAYLOAD_PROBE_TRAILING_DATA = -7
} Theron_V1SrmPayloadProbeStatus;

typedef enum {
    THERON_V1_SRM_PROGRESS_IMPORT_OK = 1,
    THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY = 0,
    THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT = -1,
    THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_VERSION = -2,
    THERON_V1_SRM_PROGRESS_IMPORT_OUT_OF_RANGE = -3,
    THERON_V1_SRM_PROGRESS_IMPORT_NON_MONOTONIC_QUEST_STATE = -4
} Theron_V1SrmProgressImportStatus;

typedef struct {
    uint8_t version;
    uint8_t quest_items_bitmask;
    Theron_DungeonID current_dungeon;
    uint8_t current_level;
    uint32_t dungeon_playtime_seconds;
    uint32_t dungeon_seeds[THERON_DUNGEON_COUNT];
    int restored;
} Theron_V1SrmProgressionReceipt;

typedef struct {
    Theron_V1SrmProgressionReceipt progression;
    uint32_t party_gold;
    uint8_t champion_count;
    uint8_t active_slot;
    uint8_t imported_body_count;
    int restored;
} Theron_V1SrmPartyImportReceipt;

/* Neutral evidence for one successfully authenticated inflated SRM body.
 * This intentionally records container and body fingerprints only.  It does
 * not map any payload bytes to Sphenx/Greatstone save fields, so unknown real
 * bodies remain non-launchable while staged artifacts can be compared safely.
 */
typedef struct {
    size_t payload_size;
    uint32_t payload_checksum32;
    uint32_t payload_prefix_checksum32;
    uint32_t payload_suffix_checksum32;
    uint32_t gzip_trailer_crc32;
    uint32_t computed_crc32;
    uint32_t gzip_trailer_isize;
    uint8_t gzip_flags;
    uint8_t gzip_extra_flags;
    uint8_t gzip_os;
    int gzip_trailer_verified;
    int captured;
} Theron_V1SrmBodyEvidence;

/* Opaque comparison receipt for authenticated Save Disk bodies.
 *
 * This is corpus evidence, not a save-body decoder.  A fingerprint group is
 * formed only from a verified gzip trailer plus the existing whole/prefix/
 * suffix payload fingerprints.  Matching groups are useful when comparing
 * staged Sphenx/Greatstone artifacts, but they do not assert that any byte
 * has a known gameplay meaning or that two payloads are cryptographically
 * identical.  Unauthenticated, truncated, and unsupported inputs do not
 * enter a group.
 */
typedef struct {
    size_t authenticated_slot_count;
    size_t fingerprint_group_count;
    int first_authenticated_slot; /* 0..4, or -1 */
    int fingerprint_group_for_slot[THERON_V1_SRM_DISK_SLOT_COUNT]; /* -1 if absent */
    int first_slot_for_group[THERON_V1_SRM_DISK_SLOT_COUNT]; /* -1 if unused */
    uint32_t catalog_checksum;
} Theron_V1SrmBodyEvidenceCatalog;

/* Opaque slot-for-slot comparison of two authenticated Save Disk corpora.
 * A slot is comparable only when both sides supplied a captured,
 * trailer-verified body for that same slot number.  A match uses exactly the
 * catalog fingerprint fields; it assigns no payload byte a save-field meaning
 * and is never a Continue-selection input. */
typedef struct {
    unsigned int left_authenticated_slot_mask;
    unsigned int right_authenticated_slot_mask;
    unsigned int comparable_slot_mask;
    unsigned int matching_slot_mask;
    unsigned int mismatch_slot_mask;
    uint32_t comparison_checksum;
} Theron_V1SrmBodyEvidenceComparison;

/* Position-bound opaque delta receipt for two already authenticated SRM
 * bodies. This describes byte layout only: it deliberately assigns no
 * payload byte a save-field meaning and is never a Continue-selection input.
 * The shared prefix and suffix do not overlap. */
typedef struct {
    size_t left_payload_size;
    size_t right_payload_size;
    size_t shared_prefix_bytes;
    size_t shared_suffix_bytes;
    size_t differing_byte_count;
    size_t differing_run_count;
    size_t retained_run_count;
    size_t longest_differing_run_bytes;
    int differing_runs_truncated;
    struct {
        size_t offset;
        size_t byte_count;
    } differing_runs[THERON_V1_SRM_BODY_DELTA_MAX_RUNS];
    uint32_t delta_checksum;
} Theron_V1SrmBodyEvidenceDelta;

/* Resolve the default save-disk root: env override
 * `FIRESTAFF_THERON_SRM_DIR` first, then
 * `$HOME/.firestaff/data/theron/save`, then `./theron-save/`.
 * Returns a stable, NUL-terminated absolute path in `out_root` (always
 * non-empty on success).  Returns 1 on success, 0 on input failure. */
int theron_v1_srm_default_root(char out_root[THERON_V1_SRM_PATH_MAX]);

/* Build the canonical slot path: `<root>/slot<index>.srm`.
 * Returns 1 on success, 0 on bad input.  The output is a clean,
 * deterministic, platform-correct path (no double separators). */
int theron_v1_srm_slot_path(const char *root,
                             int slot_index,
                             char out_path[THERON_V1_SRM_PATH_MAX]);

/* Run the classifier over a save-disk root.  Populates `out_manifest`
 * with one Theron_V1SrmSlotInfo per slot, the resolved root, and the
 * present/recognized rollup.  The function tolerates a missing root
 * (slot status = ABSENT for all slots) and never reports an error for
 * a clean host with no real .srm data.  Returns 1 on success. */
int theron_v1_srm_classify_root(const char *root,
                                Theron_V1SrmManifest *out_manifest);

/* Bounded gzip-payload probe for recognized .srm bytes.
 *
 * When Firestaff is built with zlib, this inflates one complete gzip-wrapped
 * DEFLATE member into `out_payload` up to `out_payload_capacity` bytes and
 * records the number of bytes written in `out_payload_size`.  Reserved gzip
 * flags, invalid optional-header CRCs, trailing bytes, and concatenated
 * members are rejected before a body can become import evidence.  It is
 * deliberately a payload receipt gate, not a Theron's Quest save decoder:
 * it does not interpret the custom Sphenx/TQ-RTC save body or synthesize
 * runtime state.
 *
 * When zlib is unavailable, the function returns
 * THERON_V1_SRM_PAYLOAD_PROBE_ZLIB_UNAVAILABLE after the cheap gzip/method
 * checks. */
Theron_V1SrmPayloadProbeStatus theron_v1_srm_probe_gzip_payload(
    const uint8_t *srm_bytes,
    size_t srm_size,
    uint8_t *out_payload,
    size_t out_payload_capacity,
    size_t *out_payload_size);

/* ── Body-decode envelope surface ──────────────────────────────────
 *
 * The body-decode envelope is the single-call convenience for "take a
 * raw .srm byte stream and produce a typed envelope receipt".  It is
 * the natural completion of the (inflate → decode) chain so that
 * tests and probes can exercise one full end-to-end happy path
 * instead of stitching the inflate, decode-progression, and decode-
 * party payloads together by hand.
 *
 * The envelope deliberately recognizes the same Firestaff readiness
 * envelopes (FSTQPRG1 for progression-only, FSTQPTY1 for progression
 * + party body) that the lower-level decoders do, and returns
 * UNSUPPORTED_BODY for unknown real Sphenx/Greatstone custom bodies.
 */

typedef enum {
    THERON_V1_SRM_ENVELOPE_KIND_NONE = 0,
    THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION = 1,
    THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY = 2,
    THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED = 3
} Theron_V1SrmEnvelopeKind;

typedef struct {
    Theron_V1SrmEnvelopeKind kind;
    Theron_V1SrmPayloadProbeStatus inflate_status;
    Theron_V1SrmProgressImportStatus decode_status;
    size_t inflate_payload_size;
    int slot_index;                /* -1 when envelope is built from a raw
                                    *   buffer (no slot file was read) */
    uint64_t file_size;             /* 0 when envelope is built from a raw
                                    *    buffer */
    char source_path[THERON_V1_SRM_PATH_MAX]; /* "" when envelope is
                                               * built from a raw buffer */
    Theron_V1SrmProgressionReceipt progression; /* restored==0 when kind
                                                * is UNSUPPORTED/NONE */
    Theron_V1SrmPartyImportReceipt party;       /* restored==0 when kind
                                                * is not PROG_PARTY */
    Theron_V1SrmBodyEvidence body_evidence;     /* captured only after a
                                                 * verified gzip body inflate */
} Theron_V1SrmEnvelopeReceipt;

/* Receipt for a lossless Save Disk container transfer.  This is the only
 * original-SRM write path currently supported: it preserves an authenticated
 * gzip member byte-for-byte and deliberately does not write a generated
 * body.  `body_kind` may be UNSUPPORTED, which is the expected state for a
 * real Sphenx/Greatstone body whose field layout is still unknown. */
typedef struct {
    Theron_V1SrmPayloadProbeStatus validation_status;
    Theron_V1SrmEnvelopeKind body_kind;
    uint64_t source_size;
    uint64_t destination_size;
    uint32_t source_checksum32;
    uint32_t destination_checksum32;
    int copied;
    int byte_exact;
    Theron_V1SrmBodyEvidence body_evidence;
} Theron_V1SrmOpaqueTransferReceipt;

/* Decode one raw .srm buffer (a gzip-wrapped deflate stream with the
 * Firestaff readiness envelope) and write a single envelope receipt.
 *
 * This is the Theron_DungeonProgression + champion-block decode
 * fixture's natural entry point: when synthetic bytes inflate and the
 * decoded envelope populates `out_envelope`, the receipt carries the
 * restored progression and (optionally) party body.  The function
 * tolerates and reports every failure mode of the (inflate, magic,
 * range, monotonic) chain through the receipt's `inflate_status` and
 * `decode_status` fields, so call-sites can branch on the most
 * informative error without re-running the chain. */
Theron_V1SrmEnvelopeKind theron_v1_srm_decode_envelope(
    const uint8_t *srm_bytes,
    size_t srm_size,
    uint8_t *scratch,
    size_t scratch_capacity,
    Theron_V1SrmEnvelopeReceipt *out_envelope);

/* Slot-file convenience: read `path`, inflate the gzip-wrapped body,
 * decode the envelope, and fill `out_envelope`.  Skips cleanly when
 * the path is NULL, empty, or the file does not exist (returns
 * ENVELOPE_KIND_NONE with inflate_status=BAD_INPUT, decode_status=
 * BAD_INPUT, and source_path="").  Honors the SAME bounds as
 * theron_v1_srm_decode_envelope, including the work budget of
 * THERON_V1_SRM_BODY_DECODE_MAX_BYTES.
 *
 * When `path` exists and is recognized, this is the path the gap uses
 * to advance a real operator-staged Sphenx-format or other real .srm
 * artifact without claiming Sphenx body semantics. */
Theron_V1SrmEnvelopeKind theron_v1_srm_decode_path(
    const char *path,
    int slot_index,
    uint8_t *scratch,
    size_t scratch_capacity,
    Theron_V1SrmEnvelopeReceipt *out_envelope);

/* Validate and atomically copy one original `.srm` gzip member without
 * interpreting or generating its custom body.  The source must be a regular
 * file no larger than THERON_V1_SRM_CONTAINER_TRANSFER_MAX_BYTES and must
 * pass the existing complete-member gzip/trailer validation.  The complete
 * container is verified in a private exclusive staging file before an atomic
 * no-replace publication, so corruption or destination conflicts leave an
 * existing destination untouched.  Destination must not already exist.  On
 * success it contains the exact original bytes;
 * this never changes Continue selection, runtime state, or Track 02's
 * no-generated-presentation gate. */
int theron_v1_srm_transfer_opaque_path(
    const char *source_path,
    const char *destination_path,
    uint8_t *scratch,
    size_t scratch_capacity,
    Theron_V1SrmOpaqueTransferReceipt *out_receipt);

/* Group authenticated body evidence from up to the five Save Disk slots.
 * `envelopes` may include NONE/UNSUPPORTED/decoded receipts; only receipts
 * with captured, trailer-verified body evidence are included.  Slot indexes
 * must be unique and in range.  The output has stable group ordinals in
 * ascending input slot order and is never used to select Continue. */
int theron_v1_srm_catalog_body_evidence(
    const Theron_V1SrmEnvelopeReceipt *envelopes,
    size_t envelope_count,
    Theron_V1SrmBodyEvidenceCatalog *out_catalog);

/* Compare two Save Disk receipt sets by matching slot number.  Only captured,
 * gzip-trailer-verified bodies participate.  Missing, malformed, unsupported,
 * or duplicated slot receipts are rejected or omitted rather than treated as
 * matching evidence.  This is observation-only and cannot promote a body,
 * route, palette, object, runtime, or Continue state. */
int theron_v1_srm_compare_body_evidence(
    const Theron_V1SrmEnvelopeReceipt *left_envelopes,
    size_t left_envelope_count,
    const Theron_V1SrmEnvelopeReceipt *right_envelopes,
    size_t right_envelope_count,
    Theron_V1SrmBodyEvidenceComparison *out_comparison);

/* Compare two captured, gzip-trailer-authenticated inflated bodies without
 * interpreting their contents. The supplied payloads must match their
 * receipts' existing opaque whole/prefix/suffix fingerprints; otherwise the
 * call rejects them. It is intended for a real operator-staged corpus probe,
 * never for save-body decoding or runtime promotion. */
int theron_v1_srm_compare_authenticated_payloads(
    const Theron_V1SrmEnvelopeReceipt *left_envelope,
    const uint8_t *left_payload,
    size_t left_payload_size,
    const Theron_V1SrmEnvelopeReceipt *right_envelope,
    const uint8_t *right_payload,
    size_t right_payload_size,
    Theron_V1SrmBodyEvidenceDelta *out_delta);

/* Probe-side helper: scan the configured save-disk root for slot0,
 * attempt a Theron_DungeonProgression decode via
 * theron_v1_srm_decode_path, and return the resulting envelope kind.
 *
 * The function is skip-clean: when no save-disk root or slot0 file
 * exists, it returns ENVELOPE_KIND_NONE without touching the
 * caller's envelope pointer.  This is the bounded real-asset
 * counterpart to the synthetic fixture path: real artifacts advance
 * the gate, absences stay honest and non-fatal. */
Theron_V1SrmEnvelopeKind theron_v1_srm_probe_slot0_envelope(
    Theron_V1SrmEnvelopeReceipt *out_envelope);

/* Status name + kind label helpers, used by tests, probes, and
 * receipts.  The strings are part of the public surface; adding new
 * kinds requires updating both the enum and these accessors. */
const char *theron_v1_srm_envelope_kind_name(Theron_V1SrmEnvelopeKind kind);

/* Interpret one inflated SRM body as a between-dungeon progression
 * checkpoint, when the body carries Firestaff's bounded readiness
 * envelope:
 *
 *   bytes 0..7    "FSTQPRG1"
 *   byte  8       version (1)
 *   byte  9       current dungeon (1..7)
 *   byte 10       collected quest-item mask (0..0x7f)
 *   byte 11       current level (1..3)
 *   bytes 12..15  playtime seconds, little-endian
 *   bytes 16..43  seven dungeon seeds, little-endian uint32
 *
 * This deliberately does not claim Sphenx/Greatstone custom-body coverage:
 * real payloads without this envelope return UNSUPPORTED_BODY so they stay
 * non-launchable until the real body layout is decoded.  The accepted
 * checkpoint is constrained to TQ's documented between-dungeon sequence:
 * collected quest bits must form a completed prefix and current_dungeon
 * must be the next dungeon (or dungeon 7 for the all-complete state). */
Theron_V1SrmProgressImportStatus theron_v1_srm_decode_progression_payload(
    const uint8_t *payload,
    size_t payload_size,
    Theron_DungeonProgression *out_progression,
    Theron_V1SrmProgressionReceipt *out_receipt);

/* Interpret one inflated SRM body as a Firestaff-only readiness envelope
 * carrying both between-dungeon progression and bounded champion body state.
 *
 * Accepted magic: "FSTQPTY1".  This is not the Sphenx/Greatstone real-body
 * layout.  Unknown real bodies still return UNSUPPORTED_BODY.  The imported
 * champion body fields deliberately exclude inventory/equipment because the
 * real Save Disk body is not decoded yet and THQUEST.ASM T080/T800 only lets
 * this gate prove between-dungeon state restoration safely. */
Theron_V1SrmProgressImportStatus theron_v1_srm_decode_progression_party_payload(
    const uint8_t *payload,
    size_t payload_size,
    Theron_DungeonProgression *out_progression,
    Theron_V1_Party *out_party,
    Theron_V1SrmPartyImportReceipt *out_receipt);

/* Status-name helpers (stable string contract for receipts/manifests). */
const char *theron_v1_srm_slot_status_name(Theron_V1SrmSlotStatus status);
const char *theron_v1_srm_payload_probe_status_name(Theron_V1SrmPayloadProbeStatus status);
const char *theron_v1_srm_progress_import_status_name(Theron_V1SrmProgressImportStatus status);

/* Source/evidence citation.  Same shape as the other Theron probes. */
const char *theron_v1_srm_source_evidence(void);

#endif /* THERON_V1_SRM_CLASSIFIER_H */
