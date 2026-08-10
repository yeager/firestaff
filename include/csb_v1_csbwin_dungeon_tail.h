/*
 * Read-only framing for the CSBWin saved-dungeon tail.
 *
 * Source: CSBWin SaveGame.cpp ReadDatabases()/WriteAndChecksum(), CSB.h
 * DUNGEONDATINDEX/LEVELDESC/DB0..DB15, and data.cpp dbEntrySizes. Database
 * bytes remain read-only; this module exposes only their verified spans.
 */
#ifndef FIRESTAFF_CSB_V1_CSBWIN_DUNGEON_TAIL_H
#define FIRESTAFF_CSB_V1_CSBWIN_DUNGEON_TAIL_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_csbwin_512_xor_pad_classify.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES = 44u,
    CSB_V1_CSBWIN_LEVEL_DESC_BYTES = 16u,
    CSB_V1_CSBWIN_MAX_SAVE_LEVELS = 64u,
    CSB_V1_CSBWIN_DATABASE_COUNT = 16u,
    CSB_V1_CSBWIN_MAX_COMPRESSED_TEXT_WORDS = 1000000u,
    CSB_V1_CSBWIN_EXTENDED_FLAG_INDIRECT_TEXT = 0x08u,
    CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS = 0x80u,
    CSB_V1_CSBWIN_LEGACY_FEATURE_VERSION = '@'
};

typedef enum {
    CSB_V1_CSBWIN_DUNGEON_TAIL_OK = 0,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT = -1,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED = -2,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LEVEL_COUNT = -3,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_OVERFLOW = -4,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_PREFIX = -5,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT = -6,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_CHECKSUM = -7,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TEXT_SIZE = -8,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_IO = -9
} CSB_V1_CSBWinDungeonTailResult;

typedef struct {
    int valid;
    uint16_t sentinel;
    uint16_t legacy_cell_flag_bytes;
    uint8_t level_count;
    uint16_t text_word_count;
    uint16_t object_list_length;
    uint16_t database_entries[CSB_V1_CSBWIN_DATABASE_COUNT];
    uint16_t level_last_column[CSB_V1_CSBWIN_MAX_SAVE_LEVELS];
    uint16_t column_pointer_count;
    int indirect_text;
    uint32_t compressed_text_word_count;
    size_t dungeon_index_offset;
    size_t level_descriptors_offset;
    size_t object_list_index_offset;
    size_t object_list_offset;
    size_t text_offset;
    size_t compressed_text_size_offset;
    size_t compressed_text_offset;
    size_t next_database_offset;
} CSB_V1_CSBWinDungeonTailPrefix;

typedef struct {
    uint8_t database_number;
    uint16_t entry_count;
    uint16_t source_entry_bytes;
    size_t offset;
    size_t byte_count;
} CSB_V1_CSBWinDungeonTailDatabaseSpan;

typedef struct {
    int valid;
    uint8_t extended_features_version;
    uint8_t extended_flags;
    int big_actuators;
    int legacy_scroll_records;
    int cell_flag_size_from_extended_features;
    size_t database_offset;
    size_t database_bytes;
    CSB_V1_CSBWinDungeonTailDatabaseSpan
        database[CSB_V1_CSBWIN_DATABASE_COUNT];
    size_t cell_flags_offset;
    uint32_t cell_flag_bytes;
    size_t checksum_offset;
    uint16_t computed_checksum;
    uint16_t stored_checksum;
} CSB_V1_CSBWinDungeonTailDatabaseLayout;

/* A private, fully decoded legacy CSBWin dungeon candidate.  Preparing one
 * never publishes it through csb_v1_dungeon_set_current() and never reaches
 * RuntimeProfile.  This is deliberately an opaque ownership boundary: a
 * later resume transaction may adopt the candidate only after it has proved
 * that the associated GAMEBLOCK2, champions, ITEM16 owners and timer heap
 * can all move together.
 *
 * Source: CSBWin SaveGame.cpp ReadDatabases() (2512-2896), which completes
 * database construction before ReadGame() makes the restored world live. */
typedef struct CSB_V1_CSBWinLegacyDungeonCandidate
    CSB_V1_CSBWinLegacyDungeonCandidate;

/* The last non-publishing ownership boundary before a CSBWin legacy resume.
 * It owns the private dungeon candidate and its scalar GAMEBLOCK2/timer
 * prepare receipt together.  There is deliberately no take, publish or
 * mutable accessor: construction and destruction are the only ownership
 * operations exposed here.  A future live transaction must add the complete
 * RuntimeProfile/champion/ITEM16/timer adoption step before it can relax the
 * resume gate.
 *
 * Source: CSBWin SaveGame.cpp ReadGame():1707-1906 completes GAMEBLOCK2,
 * champions, ITEM16 and timers before ReadDatabases():2512-2896 finishes
 * the dungeon. */
typedef struct CSB_V1_CSBWinLegacyResumeTransaction
    CSB_V1_CSBWinLegacyResumeTransaction;

/* Identity of the exact read-only tail a private candidate owns.  This is
 * deliberately provenance only: it cannot publish or transfer the decoded
 * dungeon.  `source_tail_signature` is a compact diagnostic fingerprint;
 * candidate_matches_source_tail() performs the authoritative byte-for-byte
 * comparison before a future atomic resume transaction can bind GAMEBLOCK2
 * to this candidate.
 *
 * Source: CSBWin SaveGame.cpp ReadDatabases()/ReadGame(): saved database
 * bytes remain the ownership source until the complete restored world is
 * made live. */
typedef struct {
    int valid;
    size_t source_tail_size;
    uint64_t source_tail_signature;
    uint16_t source_tail_checksum;
    uint8_t level_count;
    uint32_t database_entry_count;
} CSB_V1_CSBWinLegacyDungeonCandidateIdentity;

/* A non-owning, non-publishing receipt for the complete set of facts which
 * a later atomic legacy resume must move together.  It intentionally carries
 * scalar evidence only: the candidate and the authenticated body remain
 * owned by their callers until a future transaction can adopt both at once.
 * No pointer in this receipt can make a private dungeon live.
 *
 * Source: CSBWin SaveGame.cpp ReadGame():1768-1906 reads GAMEBLOCK2,
 * ITEM16, champions and timer storage before ReadDatabases() completes the
 * restored dungeon; none of those partial reads is a valid public world. */
typedef struct {
    int valid;
    CSB_V1_CSBWinLegacyDungeonCandidateIdentity candidate_identity;
    uint32_t source_body_appended_fnv1a;
    uint32_t game_time;
    uint32_t random_seed;
    uint16_t party_level;
    uint16_t party_x;
    uint16_t party_y;
    uint16_t party_facing;
    uint16_t object_in_hand;
    uint16_t hand_char;
    uint16_t magic_caster;
    uint16_t num_character;
    uint16_t item16_count;
    uint16_t max_timers;
    uint16_t num_timer;
    uint16_t first_avail_timer;
    uint16_t timer_sequence;
    uint16_t timer_record_size;
    size_t timer_raw_size;
    uint32_t timer_raw_fnv1a;
    size_t timer_queue_raw_size;
    uint32_t timer_queue_raw_fnv1a;
} CSB_V1_CSBWinLegacyResumePrepare;

/* Parse the unencrypted tail immediately following the authenticated
 * GAMEBLOCK1/2 streams. `extended_flags` is from Extended Features. */
int csb_v1_csbwin_dungeon_tail_parse_prefix(
    const uint8_t *tail,
    size_t tail_size,
    uint8_t extended_flags,
    CSB_V1_CSBWinDungeonTailPrefix *out);

/* Verify the DB0..DB15 spans after a parsed prefix without decoding or
 * publishing any record. SaveGame.cpp selects an 8-byte DB3 when
 * BigActuators is clear and a 4-byte DB7 before Extended Features version B;
 * every other source entry size is fixed by data.cpp dbEntrySizes.
 *
 * `extended_cell_flag_bytes` is EXTENDEDFEATURESBLOCK::cellFlagArraySize.
 * Zero selects DUNGEONDATINDEX::LegacyCellFlagArraySize, matching
 * ReadDatabases. The sixteen spans, cell flags, and terminal checksum must
 * consume the tail exactly. On any error `out` is unchanged. */
int csb_v1_csbwin_dungeon_tail_parse_databases(
    const uint8_t *tail,
    size_t tail_size,
    const CSB_V1_CSBWinDungeonTailPrefix *prefix,
    uint8_t extended_features_version,
    uint8_t extended_flags,
    uint32_t extended_cell_flag_bytes,
    CSB_V1_CSBWinDungeonTailDatabaseLayout *out);

/* CSBWin SaveGame.cpp WriteAndChecksum()/ReadDatabases() carries a running
 * unsigned-byte checksum over the tail and stores its final u16 in big-endian
 * order. Returns 1 for a verified tail, 0 for a checksum mismatch, and -1
 * for an invalid argument or a tail without the terminal checksum word. */
int csb_v1_csbwin_dungeon_tail_validate_checksum(
    const uint8_t *tail,
    size_t tail_size,
    uint16_t *out_computed,
    uint16_t *out_stored);

/* Materialize a legacy CSBWin saved-dungeon tail into the normal source
 * dungeon reader without publishing it to the global dungeon context.
 *
 * CSBWin SaveGame.cpp writes DUNGEONDATINDEX, LEVELDESC, pointer tables and
 * DB0..DB15 in big-endian source order; ReadDatabases() applies the narrowly
 * defined swaps in data.cpp before ownership is published.  This function
 * reproduces precisely those field swaps into a private temporary buffer and
 * invokes Firestaff's source-byte dungeon loader.  The terminal checksum is
 * not part of DUNGEON.DAT and is never passed through.  The caller must have
 * validated `prefix` and `databases` against the same immutable tail.
 *
 * Only the legacy (non-Extended-Features) tail is admitted.  In particular,
 * indirect text and big actuators have no byte-for-byte equivalence with the
 * legacy DUNGEON.DAT consumer and remain fail-closed.  On failure `out` is
 * zeroed; `tail` is never modified and no global dungeon is changed.
 *
 * Source: CSBWin SaveGame.cpp:1236-1337,2512-2896;
 * data.cpp:DUNGEONDATINDEX::Swap/DATABASES::swap; CSB.h DB0..DB15. */
int csb_v1_csbwin_dungeon_tail_load_legacy_source_dungeon(
    const uint8_t *tail,
    size_t tail_size,
    const CSB_V1_CSBWinDungeonTailPrefix *prefix,
    const CSB_V1_CSBWinDungeonTailDatabaseLayout *databases,
    CSB_V1_DungeonData *out);

/* Validate, normalize and privately own an unextended CSBWin saved-dungeon
 * tail.  This is a convenience transaction over parse_prefix(),
 * parse_databases() and load_legacy_source_dungeon(): output is assigned
 * only after every fallible stage succeeds.  The caller's tail is read-only.
 *
 * Extended Features, indirect text and big actuators remain intentionally
 * rejected because their database representation is not equivalent to the
 * legacy source-dungeon consumer. */
int csb_v1_csbwin_dungeon_tail_prepare_legacy_candidate(
    const uint8_t *tail, size_t tail_size,
    CSB_V1_CSBWinLegacyDungeonCandidate **out_candidate);

/* Read-only access to a prepared candidate.  It remains valid until discard.
 * There is intentionally no publish/take API here: ownership transfer is a
 * future atomic runtime operation, not a parser side effect. */
const CSB_V1_DungeonData *csb_v1_csbwin_dungeon_tail_candidate_dungeon(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate);
const CSB_V1_CSBWinDungeonTailPrefix
    *csb_v1_csbwin_dungeon_tail_candidate_prefix(
        const CSB_V1_CSBWinLegacyDungeonCandidate *candidate);
const CSB_V1_CSBWinDungeonTailDatabaseLayout
    *csb_v1_csbwin_dungeon_tail_candidate_databases(
        const CSB_V1_CSBWinLegacyDungeonCandidate *candidate);

/* Return immutable source-tail provenance owned by `candidate`.  On an
 * invalid argument `out` is unchanged. */
int csb_v1_csbwin_dungeon_tail_candidate_identity(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    CSB_V1_CSBWinLegacyDungeonCandidateIdentity *out);

/* Prove that a later authenticated save body still names exactly the tail
 * that constructed this private candidate.  This is a byte-for-byte check,
 * not a checksum or fingerprint comparison.  It remains candidate-only and
 * never changes global dungeon ownership. */
int csb_v1_csbwin_dungeon_tail_candidate_matches_source_tail(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    const uint8_t *tail, size_t tail_size);

/* Check the source-owned coordinates that a later atomic CSBWin resume
 * transaction would bind to this private dungeon.  GAMEBLOCK2's pose must
 * name a real square.  ITEM16::word0 is a signed DB4 index (not an RN
 * handle); inactive -1 slots are ignored and every active index must name
 * an existing DB4 record in the same saved dungeon.
 *
 * This remains candidate-only validation: it neither publishes the dungeon
 * nor consumes the supplied GAMEBLOCK2/ITEM16 bytes.  Callers pass the
 * already authenticated decoded values from CSBWin's body verifier.
 *
 * Source: CSBWin CSB.h ITEM16:2193-2230; CSBCode.cpp
 * AttachItem16ToMonster():6040-6105; SaveGame.cpp ReadGame():1768-1855. */
int csb_v1_csbwin_dungeon_tail_candidate_validate_resume_shape(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    uint16_t party_level, uint16_t party_x, uint16_t party_y,
    uint16_t party_facing, const uint16_t *item16_monster_indices,
    size_t item16_count);

/* Prove that GAMEBLOCK2's decoded TIMER pool can be carried with this
 * candidate.  This checks only source-owned timer-slot and active-queue
 * invariants: all active handles are unique and point at retained, valid
 * TIMER records, and every active summary still agrees byte-for-byte with
 * the authenticated raw TIMER and queue streams.  The saved active queue
 * order is kept as
 * source evidence; an older CSBWin producer's exact comparator variant is
 * not inferred from one corpus.  It does not materialize an M10 event,
 * publish the dungeon, or change a RuntimeProfile.
 *
 * `body` must be the already authenticated output of
 * csb_v1_csbwin_512_verify_save_body().  A later atomic resume transaction
 * must still transfer its raw timer pool, timer queue, GAMEBLOCK2, champions
 * and dungeon as one ownership operation.
 *
 * Source: CSBWin SaveGame.cpp:1851-1906; Timer.cpp:CheckTimers 1203-1239. */
int csb_v1_csbwin_dungeon_tail_candidate_validate_resume_timers(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    const CSB_V1_CSBWin512BodyReport *body);

/* Assemble a receipt only after the exact retained tail, GAMEBLOCK2 pose,
 * ITEM16 DB4 owners and raw TIMER/queue streams all validate against the
 * same private candidate.  This neither publishes nor consumes either
 * input; on failure `out` is unchanged. */
int csb_v1_csbwin_dungeon_tail_prepare_legacy_resume(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    const CSB_V1_CSBWin512BodyReport *body,
    const uint8_t *source_tail, size_t source_tail_size,
    CSB_V1_CSBWinLegacyResumePrepare *out);

/* Read one complete, legacy (no Extended Features preamble) CSBWin save and
 * prepare its private dungeon plus scalar resume receipt in one read-only
 * transaction.  This is deliberately not a runtime loader: neither the
 * global dungeon context nor a RuntimeProfile is inspected or changed.
 *
 * `max_size == 0` selects the conservative 4 MiB artifact limit used by the
 * CSBWin runtime reader.  On failure both outputs are unchanged.  On success
 * the caller owns `*out_candidate` and must discard it.  The adapter is the
 * last safe bridge before a future all-or-nothing runtime transaction; that
 * transaction must still adopt GAMEBLOCK2, champions, ITEM16, timers and
 * this candidate together.
 *
 * Source: CSBWin SaveGame.cpp ReadGame():1707-1906 and
 * ReadDatabases():2512-2896. */
int csb_v1_csbwin_dungeon_tail_prepare_legacy_resume_file(
    const char *path, size_t max_size,
    CSB_V1_CSBWinLegacyDungeonCandidate **out_candidate,
    CSB_V1_CSBWinLegacyResumePrepare *out_receipt);
void csb_v1_csbwin_dungeon_tail_discard_legacy_candidate(
    CSB_V1_CSBWinLegacyDungeonCandidate *candidate);

/* Build one private, owning legacy-resume transaction from an authentic
 * artifact.  This is a wrapper over the read-only file prepare path, but it
 * removes the split ownership between candidate and receipt from callers.
 * On failure `*out_transaction` is unchanged.  Success does not inspect or
 * mutate the global dungeon context or any RuntimeProfile. */
int csb_v1_csbwin_dungeon_tail_begin_legacy_resume_transaction_file(
    const char *path, size_t max_size,
    CSB_V1_CSBWinLegacyResumeTransaction **out_transaction);

/* Read-only views of an owning transaction.  The returned views are valid
 * only until discard; they cannot transfer the candidate or publish it. */
const CSB_V1_CSBWinLegacyResumePrepare
    *csb_v1_csbwin_dungeon_tail_legacy_resume_transaction_prepare(
        const CSB_V1_CSBWinLegacyResumeTransaction *transaction);
const CSB_V1_DungeonData
    *csb_v1_csbwin_dungeon_tail_legacy_resume_transaction_dungeon(
        const CSB_V1_CSBWinLegacyResumeTransaction *transaction);
int csb_v1_csbwin_dungeon_tail_legacy_resume_transaction_identity(
    const CSB_V1_CSBWinLegacyResumeTransaction *transaction,
    CSB_V1_CSBWinLegacyDungeonCandidateIdentity *out);
void csb_v1_csbwin_dungeon_tail_discard_legacy_resume_transaction(
    CSB_V1_CSBWinLegacyResumeTransaction *transaction);

const char *csb_v1_csbwin_dungeon_tail_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
