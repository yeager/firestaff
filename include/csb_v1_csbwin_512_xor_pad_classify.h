/*
 * csb_v1_csbwin_512_xor_pad_classify.h
 *
 * CSB V1 CSBWin 512-byte XOR-pad save-header classifier/writer.
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
 *     classifier/writer primitive that recognises the CSBWin 512-byte
 *     XOR-pad-obfuscated save header without committing to a
 *     full CSBWin importer. It probes the two documented
 *     scramble keys (CSB Noise[29] / DM Noise[10]), verifies
 *     the two-checksum invariant of UnscrambleBlock1, and
 *     surfaces the public fields of the unscrambled second-half
 *     block when a key matches. It also exposes bounded write helpers for
 *     GAMEBLOCK1, GAMEBLOCK2, CHARDESC, ITEM16, TIMER, and timer-queue
 *     sections. It never modifies classifier input buffers and never binds
 *     into M11/M12.
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
 *   - It summarizes CHARDESC champion records for runtime handoff; callers
 *     own the final import policy.
 *   - It does not bind to csb_v1_import_csb_save_buffer() (that
 *     loader rejects 512-byte XOR-pad headers).
 *   - It does not produce an M11/M12 wiring.
 *   - It does not promise end-to-end CSBWin save import/export. That
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
 *   - No file I/O. Callers feed bytes; the module reports or emits bounded
 *     in-memory CSBWin blocks.
 *   - Body-section decoding/writing is bounded verification plus summaries:
 *     GAMEBLOCK2, CHARDESC, ITEM16, timers, and timer queue are surfaced for
 *     startup/runtime handoff, but this module still does not assemble or
 *     write a complete save file.
 *   - No M11/M12 wiring. Callers decide what to do with the verdict/blocks.
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
#define CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES 64u
#define CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES 64u
#define CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES 64u
/* CSBWin SaveGame.cpp writes EDT_Palette as 24 complete 256-byte DB11
 * EXPOOL blocks (6144 bytes). Keep room for that source bundle plus the
 * adjacent DSA/database records. (Restored 2026-07-18 after the a192cb2b0
 * worktree-merge clobber reverted this to 4096, which overflowed the
 * 24-record EDT_Palette bundle.) */
#define CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES 8192u
#define CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES 256u
#define CSB_V1_CSBWIN_DSA_TRACING_WORDS 8u
/* CSBWin CSB.h: EDT_Database=5, EDBT_DSAtraces=7. */
#define CSB_V1_CSBWIN_DSA_TRACING_RECORD_ID 0x05070000u
#define CSB_V1_CSBWIN_EXTENDED_FEATURES_BYTES 512u
#define CSB_V1_CSBWIN_MAX_EXTENDED_DATA_MAP_BYTES 4096u
#define CSB_V1_CSBWIN_MAX_EXTENDED_DSA_COUNT 256u
#define CSB_V1_CSBWIN_MAX_EXTENDED_DSA_STATES 65536u
/* Keep authenticated DSA payloads within the CSB runtime importer's
 * aggregate bytecode allocation ceiling. This is an admission bound, not an
 * opcode-semantic claim. */
#define CSB_V1_CSBWIN_MAX_EXTENDED_DSA_PROGRAM_WORDS 32768u

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
    CSB_V1_CSBWIN_512_ERR_BAD_CHECKSUM = -4,
    CSB_V1_CSBWIN_512_ERR_BAD_EXPOOL = -5
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
    /* First 32 bytes of the legacy CSB_SAVE_HEADER.AdditionalData view.
     * These overlap CSBWin's GAMEBLOCK1 body-section fields at absolute
     * offsets 336..367 and are kept for the older header-readback tests. */
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
    uint8_t  csbwin_byte22808[132];  /* GAMEBLOCK1 offset 380 */
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
    int16_t word24;
    uint8_t facing;
    uint8_t char_position;
    uint8_t byte30;
    uint8_t byte31;
    int8_t attack_type;
    int8_t byte33;
    int8_t incantation[4];
    uint8_t facing3;
    uint8_t max_recent_damage;
    uint8_t poison_count;
    uint8_t ubyte43;
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
    uint8_t portrait[464];          /* CSBWin CHARDESC portrait bytes 336..799 */
} CSB_V1_CSBWin512ChampionSummary;

typedef struct {
    int valid;
    int truncated;
    uint16_t monster_index;
    uint8_t facings;
    uint8_t positions;
    uint8_t ubyte4;
    uint8_t ubyte5;
    uint8_t target_x;
    uint8_t target_y;
    uint8_t previous_x;
    uint8_t previous_y;
    uint8_t current_x;
    uint8_t current_y;
    uint8_t single_monster_status[4];
} CSB_V1_CSBWin512Item16Summary;

typedef struct {
    int valid;
    int truncated;
    uint32_t time;
    uint8_t function;
    uint8_t ubyte5;
    uint8_t ubyte6;
    uint8_t ubyte7;
    uint8_t ubyte8;
    uint8_t ubyte9;
    uint16_t sequence;
    uint8_t level;
    uint16_t source_index;
} CSB_V1_CSBWin512TimerSummary;

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
    int16_t character_tail_brightness;
    uint8_t character_tail_see_thru_walls;
    uint8_t character_tail_magic_footprints_active;
    int16_t character_tail_party_shield;
    int16_t character_tail_fire_shield;
    int16_t character_tail_spell_shield;
    uint8_t character_tail_num_footprint_entries;
    uint8_t character_tail_freeze_life_timer;
    uint8_t character_tail_first_magic_footprint;
    uint8_t character_tail_last_magic_footprint;
    uint16_t character_tail_party_footprints[24];
    uint8_t character_tail_byte13220[24];
    uint8_t character_tail_invisible;
    uint16_t item16_summary_count;
    uint16_t item16_summary_total;
    CSB_V1_CSBWin512Item16Summary
        item16[CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES];
    uint16_t timer_summary_count;
    uint16_t timer_summary_total;
    /* Verified, unscrambled TIMER/heap bytes retained for a future source
     * GameTimers owner; never inferred from the summary. */
    size_t timer_raw_size;
    uint32_t timer_raw_fnv1a;
    uint8_t timer_raw[CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES * 16u];
    CSB_V1_CSBWin512TimerSummary
        timers[CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES];
    uint16_t timer_queue_summary_count;
    uint16_t timer_queue_summary_total;
    size_t timer_queue_raw_size;
    uint32_t timer_queue_raw_fnv1a;
    uint8_t timer_queue_raw[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES * 2u];
    uint16_t timer_queue[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES];
    size_t appended_offset;
    size_t appended_size;
    size_t appended_preserved_size;
    uint32_t appended_fnv1a;
    int appended_truncated;
    int appended_expool_candidate;
    uint16_t appended_expool_block_count;
    uint8_t appended_preserved[CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES];
} CSB_V1_CSBWin512BodyReport;

/* Read-only receipt for the DSA trace bitmap stored in EXPOOL. CSBWin
 * DSAINDEX::WriteTracing writes eight uint32 words under
 * EDT_Database|EDBT_DSAtraces, and ReadTracing accepts only that exact
 * word count. The bitmap is reported as save evidence only; it does not
 * configure Firestaff tracing or reach runtime state. */
typedef struct {
    int valid;
    int present;
    uint32_t record_id;
    size_t payload_bytes;
    uint16_t enabled_dsa_count;
    uint32_t payload_fnv1a;
    uint32_t words[CSB_V1_CSBWIN_DSA_TRACING_WORDS];
} CSB_V1_CSBWinDSATracingReport;

typedef struct {
    uint16_t block2_hash;
    uint16_t block2_checksum;
    uint16_t character_hash;
    uint16_t character_checksum;
    size_t block2_size;
    size_t character_size;
    uint8_t block2_scrambled[128];
    uint8_t characters_scrambled[3328];
} CSB_V1_CSBWin512WritableChampionSections;

typedef struct {
    uint16_t item16_hash;
    uint16_t item16_checksum;
    uint16_t timers_hash;
    uint16_t timers_checksum;
    uint16_t timer_queue_hash;
    uint16_t timer_queue_checksum;
    size_t item16_size;
    size_t timers_size;
    size_t timer_queue_size;
    uint8_t item16_scrambled[CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES * 16u];
    uint8_t timers_scrambled[CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES * 16u];
    uint8_t timer_queue_scrambled[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES * 2u];
} CSB_V1_CSBWin512WritableRuntimeSections;

typedef struct {
    int key_index;                  /* CSB_V1_CSBWIN_512_KEY_CSB or _DM */
    uint8_t byte22598;              /* GAMEBLOCK1 offset 300 */
    uint8_t byte22596;              /* GAMEBLOCK1 offset 301 */
    int16_t save_option;            /* GAMEBLOCK1 offset 306 */
    uint32_t random_game_id;        /* GAMEBLOCK1 offset 308 */
    uint16_t block2_hash;           /* GAMEBLOCK1 offset 312 */
    uint16_t item16_hash;           /* GAMEBLOCK1 offset 314 */
    uint16_t character_hash;        /* GAMEBLOCK1 offset 316 */
    uint16_t timers_hash;           /* GAMEBLOCK1 offset 318 */
    uint16_t timer_queue_hash;      /* GAMEBLOCK1 offset 320 */
    uint32_t total_move_count;      /* GAMEBLOCK1 offset 322 */
    uint16_t block2_checksum;       /* GAMEBLOCK1 offset 344 */
    uint16_t item16_checksum;       /* GAMEBLOCK1 offset 346 */
    uint16_t character_checksum;    /* GAMEBLOCK1 offset 348 */
    uint16_t timers_checksum;       /* GAMEBLOCK1 offset 350 */
    uint16_t timer_queue_checksum;  /* GAMEBLOCK1 offset 352 */
    int16_t word22594;              /* GAMEBLOCK1 offset 376 */
    int16_t word22592;              /* GAMEBLOCK1 offset 378 */
    uint8_t byte22808[132];         /* GAMEBLOCK1 offset 380 */
} CSB_V1_CSBWin512WritableHeader;

/* Read-only summary of CSBWin's optional Extended Features preamble.
 * SaveGame.cpp ReadExtendedFeatures reads this before GAMEBLOCK1, verifies
 * the header and its two maps, then hands the variable-length DSA payload to
 * the DSA interpreter. Firestaff records only the independently verifiable
 * container here; no DSA bytes reach the runtime through this boundary. */
typedef struct {
    int valid;
    int simple_encryption;
    uint8_t version;
    uint8_t flags;
    uint32_t extended_flags;
    uint16_t dsa_count;
    uint32_t data_map_length;
    uint32_t editing_options;
    uint32_t game_info_size;
    uint32_t cell_flag_array_size;
    uint32_t graphics_signature1;
    uint32_t graphics_signature2;
    uint32_t spell_filter_location;
    int32_t overlay_ordinal;
    int32_t overlay_p1;
    int32_t overlay_p2;
    int32_t overlay_p3;
    int32_t overlay_p4;
    uint32_t csbgraphics_signature1;
    uint32_t csbgraphics_signature2;
    uint8_t hint_key[8];
    size_t extension_payload_offset;
} CSB_V1_CSBWinExtendedFeaturesReport;

/* One read-only CSBWin Extended Features data-map record. SaveGame.cpp
 * loads the byte type map and the big-endian uint16 index map, then
 * SwapDataIndexMap() normalizes each index for use by the engine. This
 * representation exposes that verified map without exposing any DSA or
 * trailing extension bytes to Firestaff runtime state. */
typedef struct {
    uint8_t raw_type;
    uint8_t database_type;
    uint8_t position;
    uint16_t database_index;
} CSB_V1_CSBWinExtendedDataMapEntry;

/* Read-only evidence for the variable DSA records immediately after the
 * verified Extended Features maps. This is deliberately aggregate-only:
 * script descriptions and program words stay in the input buffer and no
 * record is promoted into the Firestaff DSA runtime. */
typedef struct {
    int valid;
    uint16_t dsa_count;
    uint32_t state_count;
    uint32_t action_count;
    uint32_t program_word_count;
    uint32_t computed_checksum;
    uint32_t stored_checksum;
    size_t dsa_payload_offset;
    size_t dsa_payload_size;
    size_t next_payload_offset;
} CSB_V1_CSBWinExtendedDSAReport;

/* Read-only decoding receipt for the bytes immediately following a verified
 * Extended Features DSA section. CSBWin SaveGame.cpp ReadGameInfo() consumes
 * exactly game_info_size bytes, then ReadDSALevelIndex() optionally consumes
 * three-byte (level, slot, DSA) rows through the 0xff,0xff,0xff terminator.
 * DSA tracing is stored later in EXPOOL data and is deliberately not inferred
 * from this immediate tail. */
typedef struct {
    int valid;
    size_t game_info_offset;
    uint32_t game_info_size;
    uint32_t game_info_fnv1a;
    int level_index_present;
    uint16_t level_index_entry_count;
    size_t next_payload_offset;
    uint16_t level_dsa_index[64][32];
} CSB_V1_CSBWinExtendedTailReport;

typedef enum {
    CSB_V1_CSBWIN_EXTENDED_OK = 0,
    CSB_V1_CSBWIN_EXTENDED_ABSENT = 1,
    CSB_V1_CSBWIN_EXTENDED_UNSUPPORTED_ENCRYPTION = 2,
    CSB_V1_CSBWIN_EXTENDED_UNSUPPORTED_DSA = 3,
    CSB_V1_CSBWIN_EXTENDED_ERR_ARGUMENT = -1,
    CSB_V1_CSBWIN_EXTENDED_ERR_TRUNCATED = -2,
    CSB_V1_CSBWIN_EXTENDED_ERR_SENTINEL = -3,
    CSB_V1_CSBWIN_EXTENDED_ERR_CHECKSUM = -4,
    CSB_V1_CSBWIN_EXTENDED_ERR_BOUNDS = -5,
    CSB_V1_CSBWIN_EXTENDED_ERR_DSA = -6,
    CSB_V1_CSBWIN_EXTENDED_ERR_LEVEL_INDEX = -7
} CSB_V1_CSBWinExtendedFeaturesResult;

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

/* Validate the unencrypted Extended Features preamble that may precede a
 * CSBWin GAMEBLOCK1. Mirrors SaveGame.cpp ReadExtendedFeatures lines 298-385
 * through the header/map checks. A valid preamble carrying DSA records is
 * returned as UNSUPPORTED_DSA rather than partially decoded; the output still
 * identifies the proven boundary and the first extension-payload offset. */
int csb_v1_csbwin_512_inspect_extended_features(
    const uint8_t *bytes,
    size_t size,
    CSB_V1_CSBWinExtendedFeaturesReport *out_report);

/* Copy the checksum-verified Extended Features type/index maps into caller
 * storage. This is inspection only: it neither reads the variable DSA/game
 * info/level-index tail nor mutates runtime state. A valid DSA-bearing
 * preamble may be reported because its maps precede the DSA payload, but the
 * return value remains UNSUPPORTED_DSA and callers must not promote it.
 * Encrypted preambles remain unsupported because their maps cannot be
 * independently inspected until a fully validated decrypt path exists.
 *
 * `out_count` is always set to the validated entry count on a valid plain
 * preamble, even when `entries` is NULL or too small. Pass entries=NULL and
 * capacity=0 to query that count. Returns ERR_BOUNDS when caller storage is
 * too small; no partial entry list is written. */
int csb_v1_csbwin_512_inspect_extended_data_map(
    const uint8_t *bytes,
    size_t size,
    CSB_V1_CSBWinExtendedDataMapEntry *entries,
    size_t entries_capacity,
    size_t *out_count,
    CSB_V1_CSBWinExtendedFeaturesReport *out_report);

/* Decode and authenticate CSBWin's variable DSA section without exposing
 * DSA bytecode or mutating game state. The section starts after the validated
 * type/index maps and consists of `dsa_count` numbered DSA records followed
 * by a little-endian checksum. Mirrors SaveGame.cpp ReadDSAs lines 211-241,
 * DSA.cpp DSA::Read lines 5637-5669, DSAState::Read lines 5742-5762, and
 * DSAAction::Read lines 5790-5798. The checksum begins at 0xffff, as in
 * data.cpp RCS lines 1798-1827. On success, `next_payload_offset` is the
 * exact boundary for the separately-gated game-info/level-index tail.
 *
 * Simple-encrypted preambles remain unsupported. All DSA records, including
 * duplicate IDs/states, invalid counts, aggregate bytecode exceeding
 * CSB_V1_CSBWIN_MAX_EXTENDED_DSA_PROGRAM_WORDS, truncated programs, and a
 * bad final checksum are rejected before the output report becomes valid. */
int csb_v1_csbwin_512_inspect_extended_dsa_section(
    const uint8_t *bytes,
    size_t size,
    CSB_V1_CSBWinExtendedDSAReport *out_report,
    CSB_V1_CSBWinExtendedFeaturesReport *out_features);

/* Decode the non-script tail after inspect_extended_dsa_section() has
 * authenticated its variable DSA payload. The game-info bytes are reported
 * only as a size and FNV-1a receipt. When LevelDSAInfoPresent is set, every
 * level-index row must fit the source 64x32 table and the sequence must end
 * in the CSBWin 0xff,0xff,0xff sentinel. No tail field reaches runtime. */
int csb_v1_csbwin_512_inspect_extended_tail(
    const uint8_t *bytes,
    size_t size,
    CSB_V1_CSBWinExtendedTailReport *out_report,
    CSB_V1_CSBWinExtendedDSAReport *out_dsa,
    CSB_V1_CSBWinExtendedFeaturesReport *out_features);

/* Build the first CSBWin writeback sections from a bounded runtime summary.
 * The output contains scrambled GAMEBLOCK2 and CHARDESC/character bytes plus
 * the section hashes/checksums that a future GAMEBLOCK1 writer must store.
 * It intentionally does not emit the 512-byte header, ITEM16, timers, timer
 * queue, DSA data, or appended dungeon/global blocks. */
int csb_v1_csbwin_512_build_writable_champion_sections(
    const CSB_V1_CSBWin512BodyReport *summary,
    CSB_V1_CSBWin512WritableChampionSections *out);

/* Build CSBWin ITEM16, TIMER, and timer-queue body sections from a bounded
 * verified/runtime summary. Totals must fit the bounded summary arrays; this
 * function refuses truncated summaries instead of emitting partial save data. */
int csb_v1_csbwin_512_build_writable_runtime_sections(
    const CSB_V1_CSBWin512BodyReport *summary,
    CSB_V1_CSBWin512WritableRuntimeSections *out);

/* Build a CSBWin GAMEBLOCK1 header from bounded section metadata.
 *
 * Mirrors CSBWin SaveGame.cpp ScrambleAndWrite / Chaos.cpp WriteFirstBlock:
 * the first 256 bytes are trashed to zero except the final checksum word,
 * and the second 256 bytes carry the GAMEBLOCK1 public fields before being
 * scrambled with the selected key word. This emits only the first 512-byte
 * header. Callers still own the body sections and any appended dungeon/global
 * data. */
int csb_v1_csbwin_512_build_writable_header(
    const CSB_V1_CSBWin512WritableHeader *header,
    uint8_t out_header[CSB_V1_CSBWIN_BLOCK1_BYTES]);

/* Assemble the bounded CSBWin core save prefix in file order:
 * GAMEBLOCK1, GAMEBLOCK2, ITEM16, CHARDESC/character data, TIMER, and timer
 * queue. The caller owns the output buffer and receives the exact byte count.
 * This intentionally stops at the timer queue; appended DSA/global/expool data
 * remains outside this bounded core writer. */
int csb_v1_csbwin_512_build_writable_core_save(
    const CSB_V1_CSBWin512BodyReport *summary,
    uint8_t *out,
    size_t out_capacity,
    size_t *out_size);

/* Export one already-authenticated CSBWin CSB save without reconstructing
 * opaque game data. The input must be a complete CSB-key GAMEBLOCK1/body
 * whose encrypted sections verify; every byte, including the appended
 * DSA/DB11/EXPOOL tail, is copied exactly to `out`. DM-key saves, truncated
 * tails, invalid checksums, and insufficient output space fail closed.
 *
 * This is the original-save preservation route. Callers that need to change
 * live state must first gain a source-backed owner for every opaque section;
 * they must not use the bounded summary writer as a substitute. */
int csb_v1_csbwin_512_export_verified_csb_save(
    const uint8_t *source,
    size_t source_size,
    uint16_t timer_record_size,
    uint8_t *out,
    size_t out_capacity,
    size_t *out_size);

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

/* Locate a CSBWin DB11/Expool-style record inside a preserved appended
 * save tail. This mirrors CSBWin data.cpp EXPOOL::Locate and the dungeon
 * loader's DB11 adapter: hash = key * 0xbb40e62d, bucket table at word 32,
 * optional secondary bucket table, and linked nodes whose p+1 word is the
 * record id and p+2 starts the payload. Returns 1 when found, 0 when absent
 * or when the appended tail is truncated / not DB11-shaped. */
int csb_v1_csbwin_512_appended_expool_locate_record(
    const CSB_V1_CSBWin512BodyReport *report,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size);

/* Validate the complete preserved DB11/EXPOOL tail before a caller promotes
 * it into a live save owner.  Every bucket and linked record reachable from
 * the source hash table must stay in bounds, refer to its owning 64-word
 * block, and have a non-zero, bounded payload extent.  An empty tail is
 * valid; a non-empty tail must be a whole EXPOOL block sequence.  This is a
 * structural admission gate only: it neither selects records nor changes
 * runtime state. */
int csb_v1_csbwin_512_validate_appended_expool_tail(
    const CSB_V1_CSBWin512BodyReport *report);

/* Inspect the source-locked CSBWin DSA tracing record after save-body
 * verification has preserved a complete DB11/EXPOOL tail. A missing record
 * is valid evidence with `present == 0`; a present record must be exactly
 * eight uint32 words. Truncated, non-EXPOOL, or malformed tails reject.
 * This helper is intentionally read-only and has no runtime handoff. */
int csb_v1_csbwin_512_inspect_appended_dsa_tracing(
    const CSB_V1_CSBWin512BodyReport *report,
    CSB_V1_CSBWinDSATracingReport *out_report);

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
