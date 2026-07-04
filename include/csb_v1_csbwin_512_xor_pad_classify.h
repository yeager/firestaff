/*
 * csb_v1_csbwin_512_xor_pad_classify.h
 *
 * CSB V1 CSBWin 512-byte XOR-pad save-header classifier.
 *
 * Closes a bounded slice of the "CSBWin custom resource handling
 * (csbgraphics.dat + dmsave + csbgame)" gap
 * (docs/FIRESTAFF_GAP_LIST.md row C3 / A3).
 *
 * Where the existing modules live:
 *   - csb_v1_csbgraphics_dat_classify reads a CSBgraphics.dat
 *     override file (graphics index + LZW payload spans).
 *   - csb_v1_csbwin_save_loader_boundary_pc34_compat exercises
 *     csb_v1_import_csb_save_buffer() against the documented
 *     CSBWin / DM1 save shapes.
 *   - csb_v1_csbwin_512_xor_pad_classify (this module) is the
 *     read-only classifier that recognises the CSBWin 512-byte
 *     XOR-pad-obfuscated save header without committing to a
 *     full CSBWin importer. It probes the two documented
 *     scramble keys (CSB Noise[29] / DM Noise[10]), verifies
 *     the two-checksum invariant of UnscrambleBlock1, and
 *     surfaces the public fields of the unscrambled second-half
 *     block when a key matches. It never modifies the input
 *     buffer and never binds into M11/M12.
 *
 * The two-checksum invariant (CSBWin/Chaos.cpp UnscrambleBlock1
 * lines 1341-1368):
 *   1. The first 256 bytes are organised as 32 tuples of 4
 *      uint16 words. A rolling checksum D6W is computed:
 *         D6W = D6W + w[0]; D6W ^= w[1]; D6W = D6W - w[2]; D6W ^= w[3];
 *   2. The second 256 bytes are RC4-like-XOR-scrambled in place
 *      using Unscramble() with an initial hash drawn from
 *      word[P2] of the *unscrambled* second half. After
 *      unscrambling, D5W = sum of 128 uint16 LE words from
 *      bytes 256..511.
 *   3. The block validates iff D5W == D6W.
 *
 * For a CSBWin save P2=29 (CSB_SAVE_HEADER Noise[29]) and the
 * save is a CSB save. For a DM1Win save P2=10 (DM_SAVE_HEADER
 * Noise[10]) and the save is a DM1 save. CSBWin/Chaos.cpp:2357
 * tries CSB first, falls back to DM on UnscrambleBlock1
 * returning 0.
 *
 * What this module proves (data-free, synthetic fixtures):
 *   - A 512-byte CSBWin header built with the CSB key produces
 *     a UnscrambleBlock1 accept verdict.
 *   - The same applies for the DM key.
 *   - Garbage 512 bytes produce both-keys-reject verdicts.
 *   - After a successful unscramble, the public second-half
 *     fields (FormatID, Useless, SaveOption, RandomGameID,
 *     GameID, Keys[0..15], Checksums[0..15], Platform,
 *     DungeonID, AdditionalData[0..N]) read back as documented
 *     ReDMCSB values.
 *
 * What this module does NOT do:
 *   - It does not parse/import decoded CSBWin items, timers, DSAs,
 *     or DSA-level indexes into runtime state.
 *   - It only summarizes CHARDESC champion records; it does not import
 *     portrait bytes or full skill-XP model state into runtime by itself.
 *   - It does not bind to csb_v1_import_csb_save_buffer() (that
 *     loader rejects 512-byte XOR-pad headers).
 *   - It does not produce an M11/M12 wiring.
 *   - It does not promise end-to-end CSBWin save import. That
 *     remains tracked under docs/FIRESTAFF_GAP_LIST.md row C3 /
 *     A3 as OPEN-LARGE.
 *
 * Source references:
 *   - CSBWin/SaveGame.cpp:880 GAMEBLOCK1 (512 bytes, first block).
 *   - CSBWin/SaveGame.cpp:1145 WriteFirstBlock / ScrambleAndWrite
 *     (write side: trashes first 256 bytes, writes
 *     LE16(D5W ^ D6W) as last word, then Unscramble second half).
 *   - CSBWin/Chaos.cpp:1326 ReadGameBlock1 (read 512 bytes).
 *   - CSBWin/Chaos.cpp:1341 UnscrambleBlock1 (validate + unscramble).
 *   - CSBWin/Chaos.cpp:2357 ReadSaves path: try CSB key (29), then
 *     DM key (10); both fail -> reject.
 *   - CSBWin/CSBCode.cpp:9038 Unscramble (RC4-like XOR stream).
 *   - CSBWin/Hint.cpp:601 Unscramble (CSB Hint Oracle uses the same
 *     scramble on the HCSB.HTC hint text; this gate deliberately
 *     mirrors that pattern because the algorithm is identical).
 *   - ReDMCSB DEFS.H:480 CSB_SAVE_HEADER Noise[150] + FormatID at
 *     0x12C (300); C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX = 29.
 *   - ReDMCSB DEFS.H:469 DM_SAVE_HEADER Noise[149] + FormatID at
 *     0x12A (298); C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX = 10.
 *   - ReDMCSB DEFS.H:483 CSB_SAVE_HEADER SaveHeader, 512-byte comment.
 *   - ReDMCSB DEFS.H:500/501 C10/C29 key index macros.
 *   - docs/FIRESTAFF_GAP_LIST.md row C3 / A3 "CSBWin custom
 *     resource handling (csbgraphics.dat + dmsave + csbgame)".
 *
 * Non-claims:
 *   - No file I/O. Callers feed 512 bytes; the module reports.
 *   - Body-section decoding is bounded verification plus summaries:
 *     GAMEBLOCK2 and CHARDESC fields are surfaced for startup/runtime
 *     handoff, while ITEM16, timers, and timer queue remain checksum-only.
 *   - No M11/M12 wiring. Callers (a future launcher import
 *     button) decide what to do with the verdict.
 */

#ifndef FIRESTAFF_CSB_V1_CSBWIN_512_XOR_PAD_CLASSIFY_H
#define FIRESTAFF_CSB_V1_CSBWIN_512_XOR_PAD_CLASSIFY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The 512-byte CSBWin / DM1 save header. GAMEBLOCK1 in CSBWin
 * source is exactly 512 bytes (CSBWin/SaveGame.cpp:947 ASSERT).
 * We refuse anything shorter so the 32-iteration first-half
 * loop and the 128-word second-half unscramble can run without
 * bounds traps. */
#define CSB_V1_CSBWIN_BLOCK1_BYTES  512u

/* The two documented scramble keys. CSBWin/Chaos.cpp:2357 tries
 * CSB_KEY first, then DM_KEY on UnscrambleBlock1 returning 0.
 * ReDMCSB DEFS.H:500/501 names the same constants. */
#define CSB_V1_CSBWIN_512_KEY_CSB   29   /* C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX */
#define CSB_V1_CSBWIN_512_KEY_DM    10   /* C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX */

/* ReDMCSB DEFS.H FormatID values (DEFS.H:485-489 + 503-510). */
#define CSB_V1_FORMAT_DM_ATARI_ST                1
#define CSB_V1_FORMAT_DM_AMIGA_2X_PC98_X68K_FM   2
#define CSB_V1_FORMAT_DM_APPLE_IIGS              3
#define CSB_V1_FORMAT_DM_AMIGA_36_PC_CSB         5

/* Public fields of the unscrambled second-half CSB_SAVE_HEADER /
 * DM_SAVE_HEADER. The CSB header is 512 bytes total (300 bytes
 * Noise[150] uint16 + 1 byte Useless + 1 byte FormatID + 4 bytes
 * aUnreferenced + 1 byte SaveAndPlayChoice + padding + 4 bytes
 * GameID + 16 uint16 Keys + 16 uint16 Checksums + 2 byte
 * Platform + 2 byte DungeonID + 132 bytes AdditionalData = 632
 * bytes for the full CSB_SAVE_HEADER). CSBWin only reads the
 * first 512 bytes of that overlay (the rest is reserved); the
 * offset below are the public bytes we surface from the 512-byte
 * block after a successful unscramble.
 *
 * Offset values are byte offsets into the *unscrambled* second
 * half of the CSBWin GAMEBLOCK1 (i.e., bytes 256..511 of the
 * 512-byte block).
 */
#define CSB_V1_CSBWIN_512_OFF_USELESS           0x000u  /* 1 byte */
#define CSB_V1_CSBWIN_512_OFF_FORMAT_ID         0x001u  /* 1 byte */
#define CSB_V1_CSBWIN_512_OFF_AUNREFERENCED     0x002u  /* 4 bytes (long, BE on Atari) */
#define CSB_V1_CSBWIN_512_OFF_SAVE_AND_PLAY     0x006u  /* 1 byte */
#define CSB_V1_CSBWIN_512_OFF_GAME_ID           0x008u  /* 4 bytes (long, BE on Atari) */
#define CSB_V1_CSBWIN_512_OFF_KEYS              0x00Cu  /* 16 uint16 LE */
#define CSB_V1_CSBWIN_512_OFF_CHECKSUMS         0x02Cu  /* 16 uint16 LE */
#define CSB_V1_CSBWIN_512_OFF_PLATFORM          0x04Cu  /* 2 bytes (int16) */
#define CSB_V1_CSBWIN_512_OFF_DUNGEON_ID        0x04Eu  /* 2 bytes uint16 */
#define CSB_V1_CSBWIN_512_OFF_ADDITIONAL        0x050u  /* additional data */

/* Result codes. 0 = OK with the documented verdict, negatives
 * are bounded errors. The classifier never returns a positive
 * count — it returns a verdict in CSB_V1_CSBWin512KeyVerdict. */
typedef enum {
    CSB_V1_CSBWIN_512_OK = 0,
    CSB_V1_CSBWIN_512_ERR_ARGUMENT = -1,
    CSB_V1_CSBWIN_512_ERR_TOO_SMALL = -2,
    CSB_V1_CSBWIN_512_ERR_BAD_KEYS = -3,
    CSB_V1_CSBWIN_512_ERR_BAD_CHECKSUM = -4
} CSB_V1_CSBWin512Result;

/* The verdict: which documented CSBWin scramble key (if any)
 * produced a valid UnscrambleBlock1 read on the supplied
 * 512-byte block.
 *   - CSB: CSB key (29) decoded; save is a CSBWin CSB save.
 *   - DM:  DM key (10) decoded; save is a CSBWin DM1 save.
 *   - NEITHER: no key validated (corrupt, foreign, or
 *     non-512-byte XOR-pad layout). Caller should reject.
 */
typedef enum {
    CSB_V1_CSBWIN_512_VERDICT_NEITHER = 0,
    CSB_V1_CSBWIN_512_VERDICT_CSB     = 1,
    CSB_V1_CSBWIN_512_VERDICT_DM      = 2
} CSB_V1_CSBWin512KeyVerdict;

typedef enum {
    CSB_V1_CSBWIN_512_SECTION_BLOCK2 = 0,
    CSB_V1_CSBWIN_512_SECTION_ITEM16 = 1,
    CSB_V1_CSBWIN_512_SECTION_CHARACTERS = 2,
    CSB_V1_CSBWIN_512_SECTION_TIMERS = 3,
    CSB_V1_CSBWIN_512_SECTION_TIMER_QUEUE = 4,
    CSB_V1_CSBWIN_512_SECTION_COUNT = 5
} CSB_V1_CSBWin512BodySectionKind;

/* Public fields the classifier surfaces from a successful
 * unscramble. `format_id` is the documented ReDMCSB FormatID
 * (CSB_V1_FORMAT_*). `save_and_play_choice` is 0 for "Save and
 * Quit" or 1 for "Save and Play". `game_id` is the random
 * per-game id; two saves with the same `game_id` belong to the
 * same playthrough. `keys[16]` / `checksums[16]` are the
 * documented 16-slot tables (only 5 slots are documented as in
 * use by the original engine; we surface all 16 to keep the
 * classifier lossless). `platform` / `dungeon_id` are the
 * CSB_SAVE_HEADER / DM_SAVE_HEADER tail fields. */
typedef struct {
    uint8_t  useluss_byte;          /* CSB_SAVE_HEADER.Useless */
    uint8_t  format_id;             /* CSB_SAVE_HEADER.FormatID */
    uint8_t  save_and_play_choice;  /* CSB_SAVE_HEADER.SaveAndPlayChoice */
    uint32_t game_id;               /* CSB_SAVE_HEADER.GameID (LE) */
    uint16_t keys[16];              /* CSB_SAVE_HEADER.Keys[16] */
    uint16_t checksums[16];         /* CSB_SAVE_HEADER.Checksums[16] */
    int16_t  platform;              /* CSB_SAVE_HEADER.Platform */
    uint16_t dungeon_id;            /* CSB_SAVE_HEADER.DungeonID */
    /* First 32 bytes of CSB_SAVE_HEADER.AdditionalData. The full
     * field is 132 bytes; the 512-byte CSBWin block only carries
     * the first 96 of those (offsets 0x050..0x0B0 relative to
     * start of CSB_SAVE_HEADER, i.e. bytes 0x050..0x0B0 of the
     * second half). We surface the first 32 to keep the struct
     * bounded. */
    uint8_t  additional_data[32];

    /* CSBWin SaveGame.cpp GAMEBLOCK1 fields needed to decode the
     * following save-body sections. These byte offsets are the
     * CSBWin GAMEBLOCK1 layout at absolute offsets 300..378,
     * read from the already-unscrambled second half (offset
     * absolute - 256). Source: CSBWin/SaveGame.cpp lines 59-104
     * and load path lines 1768-1855. */
    uint8_t  csbwin_byte22598;       /* GAMEBLOCK1 offset 300 */
    uint8_t  csbwin_byte22596;       /* GAMEBLOCK1 offset 301 */
    int16_t  csbwin_save_option;     /* GAMEBLOCK1 offset 306 */
    uint32_t csbwin_random_game_id;  /* GAMEBLOCK1 offset 308 */
    uint16_t csbwin_block2_hash;     /* GAMEBLOCK1 offset 312 */
    uint16_t csbwin_item16_hash;     /* GAMEBLOCK1 offset 314 */
    uint16_t csbwin_character_hash;  /* GAMEBLOCK1 offset 316 */
    uint16_t csbwin_timers_hash;     /* GAMEBLOCK1 offset 318 */
    uint16_t csbwin_timer_queue_hash;/* GAMEBLOCK1 offset 320 */
    uint32_t csbwin_total_move_count;/* GAMEBLOCK1 offset 322 */
    uint16_t csbwin_block2_checksum; /* GAMEBLOCK1 offset 344 */
    uint16_t csbwin_item16_checksum; /* GAMEBLOCK1 offset 346 */
    uint16_t csbwin_character_checksum; /* GAMEBLOCK1 offset 348 */
    uint16_t csbwin_timers_checksum; /* GAMEBLOCK1 offset 350 */
    uint16_t csbwin_timer_queue_checksum; /* GAMEBLOCK1 offset 352 */
    int16_t  csbwin_word22594;       /* GAMEBLOCK1 offset 376 */
    int16_t  csbwin_word22592;       /* GAMEBLOCK1 offset 378 */
} CSB_V1_CSBWin512Public;

/* Top-level verdict + public-field record. `verdict` is the
 * documented key verdict. When `verdict != NEITHER`, `key_index`
 * is the documented key (CSB_V1_CSBWIN_512_KEY_CSB / _DM), and
 * `public_fields` carries the unscrambled second-half readback.
 * `first_half_d6w` / `second_half_d5w` are the two documented
 * checksums from CSBWin UnscrambleBlock1 (D6W = first-half
 * rolling checksum over the trashed/junk bytes; D5W = sum of
 * 128 uint16 words from bytes 256..511 after Unscramble). The
 * verdict passes iff they are equal after the selected key's
 * unscramble. */
typedef struct {
    CSB_V1_CSBWin512KeyVerdict verdict;
    int                        key_index;       /* CSB_V1_CSBWIN_512_KEY_CSB / _DM, or 0 when verdict == NEITHER */
    uint16_t                   first_half_d6w;  /* CSBWin UnscrambleBlock1 D6W */
    uint16_t                   second_half_d5w; /* CSBWin UnscrambleBlock1 D5W */
    CSB_V1_CSBWin512Public     public_fields;
} CSB_V1_CSBWin512Report;

typedef struct {
    CSB_V1_CSBWin512BodySectionKind kind;
    size_t encrypted_offset;
    size_t encrypted_size;
    uint16_t initial_hash;
    uint16_t expected_checksum;
    int present;
    int checksum_ok;
} CSB_V1_CSBWin512BodySectionReport;

typedef struct {
    int valid;
    char name[9];
    char title[17];
    uint8_t facing;
    uint8_t char_position;
    int8_t attack_type;
    uint8_t facing3;
    uint8_t max_recent_damage;
    uint8_t poison_count;
    int16_t busy_timer;
    int16_t timer_index;
    int16_t char_flags;
    int16_t wounds;
    int16_t hp;
    int16_t max_hp;
    int16_t stamina;
    int16_t max_stamina;
    int16_t mana;
    int16_t max_mana;
    int16_t word64;
    int16_t food;
    int16_t water;
    uint8_t attributes[7][3];       /* CSBWin ATTRIBUTE: max,current,min */
    int16_t skill_temp_adjust[20];  /* CSBWin SKILL.tempAdjust */
    uint32_t skill_experience[20];  /* CSBWin SKILL.experience */
    uint16_t possessions[30];       /* CSBWin RN values */
    uint16_t load;
    uint16_t shield_strength;
    uint32_t talents;
    uint16_t fingerprint;
    uint16_t cause_of_damage;
    uint16_t monster_causing_damage;
} CSB_V1_CSBWin512ChampionSummary;

typedef struct {
    CSB_V1_CSBWin512Report header;
    int header_valid;
    uint16_t timer_record_size;
    uint32_t game_time;
    uint32_t random_seed;
    uint16_t object_in_hand;
    uint16_t max_item16;
    uint16_t max_timers;
    uint16_t num_timer;
    uint16_t first_avail_timer;
    uint16_t item16_queue_len;
    uint16_t timer_sequence;
    uint16_t num_character;
    uint16_t party_x;
    uint16_t party_y;
    uint16_t party_facing;
    uint16_t party_level;
    uint16_t hand_char;
    uint16_t magic_caster;
    uint32_t last_monster_attack_time;
    uint32_t last_party_move_time;
    uint16_t party_move_disable_timer;
    uint16_t word11712;
    uint16_t word11714;
    size_t required_size;
    int sections_verified;
    CSB_V1_CSBWin512BodySectionReport
        sections[CSB_V1_CSBWIN_512_SECTION_COUNT];
    CSB_V1_CSBWin512ChampionSummary champions[4];
} CSB_V1_CSBWin512BodyReport;

/* ── Public API ──────────────────────────────────────────────────────── */

/* Classify the first 512 bytes of a CSBWin / DM1 save.
 *
 * Runs the documented CSBWin UnscrambleBlock1 validation in
 * read-only fashion:
 *   1. Computes the first-half D6W rolling checksum over bytes
 *      0..255 (CSBWin/Chaos.cpp UnscrambleBlock1 first loop).
 *   2. Tries Unscramble(bytes + 256, LE16(word(P1 + 2*29)), 128)
 *      using the CSB key. If the post-Unscramble D5W equals
 *      D6W, the verdict is CSB and `public_fields` is filled
 *      from the unscrambled second half.
 *   3. If the CSB key fails, tries Unscramble(bytes + 256,
 *      LE16(word(P1 + 2*10)), 128) using the DM key on a fresh
 *      in-place copy of the bytes (so the caller's buffer is
 *      never modified).
 *   4. Otherwise the verdict is NEITHER.
 *
 * Bounds:
 *   - Rejects bytes == NULL or size < CSB_V1_CSBWIN_BLOCK1_BYTES.
 *   - Never modifies the caller's input buffer (an internal
 *     scratch copy holds the unscrambled second half).
 *   - Reports the actual D6W / D5W / key_index so callers can
 *     audit the verdict.
 *
 * Returns CSB_V1_CSBWIN_512_OK when `out_report` is populated;
 * a negative CSB_V1_CSBWin512Result on argument / size failure.
 */
int csb_v1_csbwin_512_xor_pad_classify(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBWin512Report *out_report);

/* Pure-DM helper: try the DM key (10) only. Useful for tests
 * that want to lock the DM-key path independently of the
 * CSB-first fallback in csb_v1_csbwin_512_xor_pad_classify(). */
int csb_v1_csbwin_512_xor_pad_classify_dm_key(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBWin512Report *out_report);

/* Pure-CSB helper: try the CSB key (29) only. */
int csb_v1_csbwin_512_xor_pad_classify_csb_key(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBWin512Report *out_report);

/* Decode one CSBWin save-body stream section.
 *
 * Mirrors CSBWin/CSBCode.cpp UnscrambleStream lines 9061-9069:
 * the caller supplies the encrypted stream bytes, the section's
 * initial hash, and the expected checksum from GAMEBLOCK1. The
 * function copies the decoded bytes to `out`, verifies the
 * Unscramble return checksum, and never mutates `src`.
 *
 * This is the primitive needed after GAMEBLOCK1 classification for
 * block 2, ITEM16, characters, timers, and timer queue. It does
 * not parse those sections or import runtime state by itself. */
int csb_v1_csbwin_512_decode_stream_section(
    const uint8_t *src,
    size_t size,
    uint16_t initial_hash,
    uint16_t expected_checksum,
    uint8_t *out,
    size_t out_capacity);

/* Verify the CSBWin save-body layout that follows GAMEBLOCK1.
 *
 * Source: CSBWin/SaveGame.cpp lines 1768-1855 reads 128-byte
 * GAMEBLOCK2, then ITEM16 (`16 * MaxITEM16`), character data
 * (3328), timers (`MaxTimers * timerSize`), and timer queue
 * (`MaxTimers * 2`). `timer_record_size` accepts 10, 12, or 16;
 * pass 0 for the current CSBWin extended-timer default of 16.
 *
 * The function verifies each encrypted section with the hashes and
 * checksums stored in GAMEBLOCK1. It reports section boundaries,
 * decoded GAMEBLOCK2 sizing fields, and a bounded summary of the four
 * CSBWin CHARDESC records. It does not expose decoded section buffers. */
int csb_v1_csbwin_512_verify_save_body(
    const uint8_t *bytes,
    size_t size,
    uint16_t timer_record_size,
    CSB_V1_CSBWin512BodyReport *out);

/* ── Lookup helpers (used by tests + probe + docs) ──────────────────── */

const char *csb_v1_csbwin_512_xor_pad_result_name(int result);
const char *csb_v1_csbwin_512_xor_pad_verdict_name(
    CSB_V1_CSBWin512KeyVerdict verdict);

/* Source-evidence string for tests + docs. */
const char *csb_v1_csbwin_512_xor_pad_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_CSBWIN_512_XOR_PAD_CLASSIFY_H */
