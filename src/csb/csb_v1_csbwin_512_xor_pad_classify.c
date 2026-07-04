/*
 * csb_v1_csbwin_512_xor_pad_classify.c
 *
 * Read-only classifier for the CSBWin 512-byte XOR-pad
 * obfuscated save header. See
 * include/csb_v1_csbwin_512_xor_pad_classify.h for scope and
 * source references.
 *
 * Implementation stays close to CSBWin/Chaos.cpp:
 *
 *   CSBWin/Chaos.cpp:1341 UnscrambleBlock1(P1, P2):
 *     1. First half: 32 iterations of 4-word rolling checksum
 *        D6W = D6W + w[0]; D6W ^= w[1]; D6W = D6W - w[2]; D6W ^= w[3];
 *     2. Unscramble(bytes+256, LE16(word(P1 + 2*P2)), 128) on
 *        the second 256-byte half.
 *     3. D5W = sum of 128 uint16 LE words from bytes 256..511.
 *     4. Block validates iff D5W == D6W.
 *
 *   CSBWin/Chaos.cpp:2357 ReadSaves fallback:
 *     - First tries CSB key (29); on fail tries DM key (10).
 *     - Both fail -> reject.
 *
 *   CSBWin/CSBCode.cpp:9038 Unscramble(buf, initialHash, numword):
 *     A3 = buf
 *     D7W = initialHash (uint16)
 *     D6W = numword (uint16)
 *     D5W = initialHash (uint16)
 *     loop:
 *       D5W = D5W + LE16(wordGear(A3))
 *       wordGear(A3) ^= LE16(D7W)
 *       D5W = D5W + LE16(wordGear(A3))
 *       A3 += 2
 *       D7W = D7W + D6W
 *       D6W--
 *     until D6W == 0
 *     return D5W
 *
 * This module never modifies the caller's buffer; it always
 * copies 512 bytes into a local scratch buffer before
 * attempting the in-place unscramble. The classifier decodes
 * GAMEBLOCK1 plus bounded verified save-body summaries needed by
 * startup/resume handoff. It does not bind into M11/M12, does not promise
 * end-to-end CSBWin save import, and remains tracked under
 * docs/FIRESTAFF_GAP_LIST.md row C3 / A3 (CSBWin custom
 * resource handling) as a bounded advance.
 */

#include "csb_v1_csbwin_512_xor_pad_classify.h"

#include <stdlib.h>
#include <string.h>

/* ── Helpers ────────────────────────────────────────────────────────── */

static uint16_t read_le16(const uint8_t *bytes, size_t offset)
{
    return (uint16_t)(((uint16_t)bytes[offset]) |
                      ((uint16_t)bytes[offset + 1u] << 8));
}

static uint32_t read_le32(const uint8_t *bytes, size_t offset)
{
    return ((uint32_t)bytes[offset]) |
           ((uint32_t)bytes[offset + 1u] << 8) |
           ((uint32_t)bytes[offset + 2u] << 16) |
           ((uint32_t)bytes[offset + 3u] << 24);
}

static int checked_add_size(size_t a, size_t b, size_t *out)
{
    if (!out || a > ((size_t)-1) - b) {
        return 0;
    }
    *out = a + b;
    return 1;
}

static int checked_mul_size(size_t a, size_t b, size_t *out)
{
    if (!out || (a != 0u && b > ((size_t)-1) / a)) {
        return 0;
    }
    *out = a * b;
    return 1;
}

enum {
    CSBWIN_CHARDESC_STRIDE = 800,
    CSBWIN_CHARDESC_COUNT = 4,
    CSBWIN_CHARDESC_OFF_NAME = 0,
    CSBWIN_CHARDESC_OFF_TITLE = 8,
    CSBWIN_CHARDESC_OFF_FACING = 28,
    CSBWIN_CHARDESC_OFF_POSITION = 29,
    CSBWIN_CHARDESC_OFF_ATTACK_TYPE = 32,
    CSBWIN_CHARDESC_OFF_FACING3 = 40,
    CSBWIN_CHARDESC_OFF_MAX_RECENT_DAMAGE = 41,
    CSBWIN_CHARDESC_OFF_POISON_COUNT = 42,
    CSBWIN_CHARDESC_OFF_BUSY_TIMER = 44,
    CSBWIN_CHARDESC_OFF_TIMER_INDEX = 46,
    CSBWIN_CHARDESC_OFF_CHAR_FLAGS = 48,
    CSBWIN_CHARDESC_OFF_WOUNDS = 50,
    CSBWIN_CHARDESC_OFF_HP = 52,
    CSBWIN_CHARDESC_OFF_WORD64 = 64,
    CSBWIN_CHARDESC_OFF_FOOD = 66,
    CSBWIN_CHARDESC_OFF_WATER = 68,
    CSBWIN_CHARDESC_OFF_ATTRIBUTES = 70,
    CSBWIN_CHARDESC_OFF_SKILLS = 92,
    CSBWIN_CHARDESC_OFF_POSSESSIONS = 212,
    CSBWIN_CHARDESC_OFF_LOAD = 272,
    CSBWIN_CHARDESC_OFF_SHIELD = 274,
    CSBWIN_CHARDESC_OFF_TALENTS = 276,
    CSBWIN_CHARDESC_OFF_FINGERPRINT = 280,
    CSBWIN_CHARDESC_OFF_CAUSE_OF_DAMAGE = 282,
    CSBWIN_CHARDESC_OFF_MONSTER_CAUSING_DAMAGE = 284
};

static void copy_fixed_text(char *dst,
                            size_t dst_size,
                            const uint8_t *src,
                            size_t src_size)
{
    size_t i;
    if (!dst || dst_size == 0u) return;
    memset(dst, 0, dst_size);
    if (!src) return;
    for (i = 0u; i + 1u < dst_size && i < src_size; ++i) {
        dst[i] = (char)src[i];
        if (src[i] == 0u) break;
    }
}

static void parse_champion_summary(const uint8_t *record,
                                   CSB_V1_CSBWin512ChampionSummary *out)
{
    size_t i;
    if (!record || !out) return;
    memset(out, 0, sizeof(*out));
    out->valid = 1;
    copy_fixed_text(out->name, sizeof(out->name),
                    record + CSBWIN_CHARDESC_OFF_NAME, 8u);
    copy_fixed_text(out->title, sizeof(out->title),
                    record + CSBWIN_CHARDESC_OFF_TITLE, 16u);
    out->facing = record[CSBWIN_CHARDESC_OFF_FACING];
    out->char_position = record[CSBWIN_CHARDESC_OFF_POSITION];
    out->attack_type = (int8_t)record[CSBWIN_CHARDESC_OFF_ATTACK_TYPE];
    out->facing3 = record[CSBWIN_CHARDESC_OFF_FACING3];
    out->max_recent_damage = record[CSBWIN_CHARDESC_OFF_MAX_RECENT_DAMAGE];
    out->poison_count = record[CSBWIN_CHARDESC_OFF_POISON_COUNT];
    out->busy_timer =
        (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_BUSY_TIMER);
    out->timer_index =
        (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_TIMER_INDEX);
    out->char_flags =
        (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_CHAR_FLAGS);
    out->wounds = (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_WOUNDS);
    out->hp = (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_HP + 0u);
    out->max_hp = (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_HP + 2u);
    out->stamina = (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_HP + 4u);
    out->max_stamina =
        (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_HP + 6u);
    out->mana = (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_HP + 8u);
    out->max_mana =
        (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_HP + 10u);
    out->word64 = (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_WORD64);
    out->food = (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_FOOD);
    out->water = (int16_t)read_le16(record, CSBWIN_CHARDESC_OFF_WATER);
    for (i = 0u; i < 7u; ++i) {
        const size_t base = CSBWIN_CHARDESC_OFF_ATTRIBUTES + i * 3u;
        out->attributes[i][0] = record[base + 0u];
        out->attributes[i][1] = record[base + 1u];
        out->attributes[i][2] = record[base + 2u];
    }
    for (i = 0u; i < 20u; ++i) {
        const size_t base = CSBWIN_CHARDESC_OFF_SKILLS + i * 6u;
        out->skill_temp_adjust[i] = (int16_t)read_le16(record, base);
        out->skill_experience[i] = read_le32(record, base + 2u);
    }
    for (i = 0u; i < 30u; ++i) {
        out->possessions[i] =
            read_le16(record, CSBWIN_CHARDESC_OFF_POSSESSIONS + i * 2u);
    }
    out->load = read_le16(record, CSBWIN_CHARDESC_OFF_LOAD);
    out->shield_strength = read_le16(record, CSBWIN_CHARDESC_OFF_SHIELD);
    out->talents = read_le32(record, CSBWIN_CHARDESC_OFF_TALENTS);
    out->fingerprint = read_le16(record, CSBWIN_CHARDESC_OFF_FINGERPRINT);
    out->cause_of_damage =
        read_le16(record, CSBWIN_CHARDESC_OFF_CAUSE_OF_DAMAGE);
    out->monster_causing_damage =
        read_le16(record, CSBWIN_CHARDESC_OFF_MONSTER_CAUSING_DAMAGE);
}

/* First-half rolling checksum from CSBWin/Chaos.cpp:1341
 * UnscrambleBlock1 first loop. Walks 32 tuples of 4 uint16
 * LE words starting at `bytes`, accumulating D6W = +/^/-/^.
 * Returns the resulting D6W as uint16 (the loop runs in
 * uint16 arithmetic per the CSBWin source). */
static uint16_t first_half_d6w(const uint8_t *bytes)
{
    uint16_t d6 = 0u;
    size_t i;
    /* 32 iterations of 4 words = 128 words = 256 bytes. */
    for (i = 0u; i < 32u; ++i) {
        size_t off = i * 8u;
        uint16_t w0 = read_le16(bytes, off + 0u);
        uint16_t w1 = read_le16(bytes, off + 2u);
        uint16_t w2 = read_le16(bytes, off + 4u);
        uint16_t w3 = read_le16(bytes, off + 6u);
        d6 = (uint16_t)(d6 + w0);
        d6 = (uint16_t)(d6 ^ w1);
        d6 = (uint16_t)(d6 - w2);
        d6 = (uint16_t)(d6 ^ w3);
    }
    return d6;
}

/* In-place Unscramble on `numword` uint16 LE words starting at
 * `buf`. Mirrors CSBWin/CSBCode.cpp:9038 byte-for-byte:
 *   D7 = initial_hash, D6 = numword, D5 = initial_hash.
 *   per word:
 *     D5 = D5 + LE16(word)
 *     word ^= LE16(D7)
 *     D5 = D5 + LE16(word)
 *     D7 = D7 + D6   (post-increment uses the *current* D6)
 *     D6--
 *   until D6 == 0
 * Returns the resulting D5 (uint16 wrap).
 */
static uint16_t unscramble_block(uint8_t *buf,
                                 uint16_t initial_hash,
                                 uint16_t numword)
{
    uint16_t d7 = initial_hash;
    uint16_t d6 = numword;
    uint16_t d5 = initial_hash;
    size_t i;
    /* numword is the loop count (numwordM1 in older CSB source
     * variants). When numword == 0 the loop is skipped, which
     * matches the documented "empty second half" behaviour. */
    for (i = 0u; i < numword; ++i) {
        size_t off = i * 2u;
        uint16_t w = read_le16(buf, off);
        d5 = (uint16_t)(d5 + w);
        w = (uint16_t)(w ^ d7);
        buf[off + 0u] = (uint8_t)(w & 0xFFu);
        buf[off + 1u] = (uint8_t)((w >> 8) & 0xFFu);
        d5 = (uint16_t)(d5 + w);
        /* CSB source updates D7 using the *current* D6 (the
         * loop count) before decrementing. Order matters: the
         * first iteration uses D6 == numword, the last iteration
         * uses D6 == 1. */
        d7 = (uint16_t)(d7 + d6);
        d6 = (uint16_t)(d6 - 1u);
    }
    return d5;
}

/* Second-half rolling checksum from CSBWin/Chaos.cpp Unscramble
 * Block1 second loop. Sums 128 uint16 LE words from bytes 256
 * ..511 in the unscrambled second half. Returns uint16. */
static uint16_t second_half_d5w(const uint8_t *bytes_256)
{
    uint16_t d5 = 0u;
    size_t i;
    for (i = 0u; i < 128u; ++i) {
        d5 = (uint16_t)(d5 + read_le16(bytes_256, i * 2u));
    }
    return d5;
}

/* Pull the documented public fields out of the unscrambled
 * second half. Caller guarantees the buffer has been
 * successfully unscrambled. */
static void read_public_fields(const uint8_t *bytes_256,
                               CSB_V1_CSBWin512Public *out)
{
    size_t i;
    memset(out, 0, sizeof(*out));
    out->useluss_byte = bytes_256[CSB_V1_CSBWIN_512_OFF_USELESS];
    out->format_id = bytes_256[CSB_V1_CSBWIN_512_OFF_FORMAT_ID];
    out->save_and_play_choice =
        bytes_256[CSB_V1_CSBWIN_512_OFF_SAVE_AND_PLAY];
    out->game_id = read_le32(bytes_256, CSB_V1_CSBWIN_512_OFF_GAME_ID);
    for (i = 0u; i < 16u; ++i) {
        out->keys[i] =
            read_le16(bytes_256, (size_t)CSB_V1_CSBWIN_512_OFF_KEYS
                                  + i * 2u);
        out->checksums[i] =
            read_le16(bytes_256, (size_t)CSB_V1_CSBWIN_512_OFF_CHECKSUMS
                                  + i * 2u);
    }
    out->platform =
        (int16_t)read_le16(bytes_256, CSB_V1_CSBWIN_512_OFF_PLATFORM);
    out->dungeon_id =
        read_le16(bytes_256, CSB_V1_CSBWIN_512_OFF_DUNGEON_ID);
    /* First 32 bytes of AdditionalData — bounded to keep the
     * public-field struct at a fixed size. */
    for (i = 0u; i < 32u; ++i) {
        out->additional_data[i] =
            bytes_256[(size_t)CSB_V1_CSBWIN_512_OFF_ADDITIONAL + i];
    }

    /* CSBWin SaveGame.cpp GAMEBLOCK1 lines 59-104 stores the
     * body-section hashes/checksums in the first block; after
     * UnscrambleBlock1 those bytes live in this second-half buffer
     * at absolute-offset-minus-256. The load path consumes them at
     * SaveGame.cpp lines 1768-1855 through UnscrambleStream. */
    out->csbwin_byte22598 = bytes_256[300u - 256u];
    out->csbwin_byte22596 = bytes_256[301u - 256u];
    out->csbwin_save_option =
        (int16_t)read_le16(bytes_256, 306u - 256u);
    out->csbwin_random_game_id =
        read_le32(bytes_256, 308u - 256u);
    out->csbwin_block2_hash =
        read_le16(bytes_256, 312u - 256u);
    out->csbwin_item16_hash =
        read_le16(bytes_256, 314u - 256u);
    out->csbwin_character_hash =
        read_le16(bytes_256, 316u - 256u);
    out->csbwin_timers_hash =
        read_le16(bytes_256, 318u - 256u);
    out->csbwin_timer_queue_hash =
        read_le16(bytes_256, 320u - 256u);
    out->csbwin_total_move_count =
        read_le32(bytes_256, 322u - 256u);
    out->csbwin_block2_checksum =
        read_le16(bytes_256, 344u - 256u);
    out->csbwin_item16_checksum =
        read_le16(bytes_256, 346u - 256u);
    out->csbwin_character_checksum =
        read_le16(bytes_256, 348u - 256u);
    out->csbwin_timers_checksum =
        read_le16(bytes_256, 350u - 256u);
    out->csbwin_timer_queue_checksum =
        read_le16(bytes_256, 352u - 256u);
    out->csbwin_word22594 =
        (int16_t)read_le16(bytes_256, 376u - 256u);
    out->csbwin_word22592 =
        (int16_t)read_le16(bytes_256, 378u - 256u);
}

/* ── Internal: try one key on a fresh scratch copy ─────────────────── */

/* Try `key_index` on a fresh scratch copy of the 512 bytes.
 * Returns 1 on success (out_report filled), 0 on failure (no
 * out_report mutation). The caller does not see a partially
 * unscrambled scratch on failure because we copy fresh each
 * call. */
static int try_key(const uint8_t *bytes, int key_index,
                   CSB_V1_CSBWin512Report *out_report)
{
    uint8_t scratch[CSB_V1_CSBWIN_BLOCK1_BYTES];
    uint16_t d6;
    uint16_t initial_hash;
    uint16_t d5;
    size_t p2_off;

    if (key_index < 0 || key_index > 65535) {
        return 0;
    }
    /* CSBWin reads the initial hash from word[P2] of the *raw*
     * bytes — i.e. bytes [2*P2 .. 2*P2+1] of the 512-byte block
     * (the field sits inside the unscrambled second-half layout,
     * but the value is read off the scrambled bytes first so
     * the unscramble sees a consistent seed). For CSB key=29 the
     * word is at byte 58; for DM key=10 the word is at byte 20.
     * Both fall within the first 256 bytes of the layout that
     * CSBWin stores on disk (the "scrambled zero-junk with the
     * hash word at the end of the first half"). */
    p2_off = (size_t)key_index * 2u;
    if (p2_off + 1u >= CSB_V1_CSBWIN_BLOCK1_BYTES) {
        return 0;
    }
    initial_hash = read_le16(bytes, p2_off);

    /* Copy the 512 bytes into scratch. The first 256 bytes are
     * left intact (the first-half checksum is read-only). The
     * second 256 bytes are unscrambled in place. */
    memcpy(scratch, bytes, CSB_V1_CSBWIN_BLOCK1_BYTES);

    d6 = first_half_d6w(scratch);

    /* Unscramble the second half. CSBWin/Chaos.cpp calls
     * Unscramble(buf+256, hash, 128) — 128 uint16 words. The
     * return value is the Unscramble-loop rolling checksum
     * (sum of pre-XOR + post-XOR word values); it is NOT
     * the same quantity as the second-half sum the read side
     * checks against (CSBWin/Chaos.cpp:1341 UnscrambleBlock1
     * compares D5W_outer = sum(post-unscramble words) against
     * D6W = first-half rolling checksum). We discard the
     * Unscramble return value here because the read-side
     * invariant uses the second-half sum directly. */
    (void)unscramble_block(scratch + 256u, initial_hash, 128u);

    /* Compute the post-unscramble second-half sum. This is the
     * D5W that CSBWin/Chaos.cpp:1341 UnscrambleBlock1 compares
     * against D6W (= first_half_d6w). */
    {
        uint16_t d5_post = second_half_d5w(scratch + 256u);
        if (d5_post != d6) {
            return 0;
        }
        d5 = d5_post;
    }

    if (out_report) {
        out_report->verdict = (key_index == CSB_V1_CSBWIN_512_KEY_CSB)
                                  ? CSB_V1_CSBWIN_512_VERDICT_CSB
                                  : CSB_V1_CSBWIN_512_VERDICT_DM;
        out_report->key_index = key_index;
        out_report->first_half_d6w = d6;
        out_report->second_half_d5w = d5;
        read_public_fields(scratch + 256u, &out_report->public_fields);
    }
    return 1;
}

/* ── Public API ────────────────────────────────────────────────────── */

int csb_v1_csbwin_512_xor_pad_classify(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBWin512Report *out_report)
{
    if (!bytes || !out_report) {
        return CSB_V1_CSBWIN_512_ERR_ARGUMENT;
    }
    if (size < CSB_V1_CSBWIN_BLOCK1_BYTES) {
        return CSB_V1_CSBWIN_512_ERR_TOO_SMALL;
    }
    memset(out_report, 0, sizeof(*out_report));

    /* CSBWin/Chaos.cpp:2357 tries CSB first, then DM. We mirror
     * that ordering so the verdict lines up with what CSBWin
     * itself would have accepted. */
    if (try_key(bytes, CSB_V1_CSBWIN_512_KEY_CSB, out_report)) {
        return CSB_V1_CSBWIN_512_OK;
    }
    if (try_key(bytes, CSB_V1_CSBWIN_512_KEY_DM, out_report)) {
        return CSB_V1_CSBWIN_512_OK;
    }
    out_report->verdict = CSB_V1_CSBWIN_512_VERDICT_NEITHER;
    out_report->key_index = 0;
    /* D6W is still informative even on a reject verdict: report
     * the first-half checksum so callers can sanity-check the
     * block (e.g. all-zero blocks produce D6W == 0 and a clean
     * "both keys failed" verdict). */
    out_report->first_half_d6w = first_half_d6w(bytes);
    out_report->second_half_d5w = 0u;
    return CSB_V1_CSBWIN_512_OK;
}

int csb_v1_csbwin_512_xor_pad_classify_csb_key(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBWin512Report *out_report)
{
    if (!bytes || !out_report) {
        return CSB_V1_CSBWIN_512_ERR_ARGUMENT;
    }
    if (size < CSB_V1_CSBWIN_BLOCK1_BYTES) {
        return CSB_V1_CSBWIN_512_ERR_TOO_SMALL;
    }
    memset(out_report, 0, sizeof(*out_report));
    if (try_key(bytes, CSB_V1_CSBWIN_512_KEY_CSB, out_report)) {
        return CSB_V1_CSBWIN_512_OK;
    }
    out_report->verdict = CSB_V1_CSBWIN_512_VERDICT_NEITHER;
    out_report->key_index = 0;
    out_report->first_half_d6w = first_half_d6w(bytes);
    out_report->second_half_d5w = 0u;
    return CSB_V1_CSBWIN_512_OK;
}

int csb_v1_csbwin_512_xor_pad_classify_dm_key(
    const uint8_t *bytes, size_t size,
    CSB_V1_CSBWin512Report *out_report)
{
    if (!bytes || !out_report) {
        return CSB_V1_CSBWIN_512_ERR_ARGUMENT;
    }
    if (size < CSB_V1_CSBWIN_BLOCK1_BYTES) {
        return CSB_V1_CSBWIN_512_ERR_TOO_SMALL;
    }
    memset(out_report, 0, sizeof(*out_report));
    if (try_key(bytes, CSB_V1_CSBWIN_512_KEY_DM, out_report)) {
        return CSB_V1_CSBWIN_512_OK;
    }
    out_report->verdict = CSB_V1_CSBWIN_512_VERDICT_NEITHER;
    out_report->key_index = 0;
    out_report->first_half_d6w = first_half_d6w(bytes);
    out_report->second_half_d5w = 0u;
    return CSB_V1_CSBWIN_512_OK;
}

int csb_v1_csbwin_512_decode_stream_section(
    const uint8_t *src,
    size_t size,
    uint16_t initial_hash,
    uint16_t expected_checksum,
    uint8_t *out,
    size_t out_capacity)
{
    uint16_t actual_checksum;

    if (!src || !out) {
        return CSB_V1_CSBWIN_512_ERR_ARGUMENT;
    }
    if (size == 0u || (size & 1u) != 0u || out_capacity < size) {
        return CSB_V1_CSBWIN_512_ERR_TOO_SMALL;
    }
    memcpy(out, src, size);
    /* CSBWin/CSBCode.cpp UnscrambleStream lines 9061-9069 reads the
     * encrypted bytes, calls Unscramble(dest, initialHash, size/2),
     * and accepts the section only when the returned checksum equals
     * the GAMEBLOCK1 section checksum. */
    actual_checksum = unscramble_block(out, initial_hash,
                                       (uint16_t)(size / 2u));
    if (actual_checksum != expected_checksum) {
        memset(out, 0, size);
        return CSB_V1_CSBWIN_512_ERR_BAD_CHECKSUM;
    }
    return CSB_V1_CSBWIN_512_OK;
}

static int verify_body_section(
    const uint8_t *bytes,
    size_t size,
    CSB_V1_CSBWin512BodySectionKind kind,
    size_t offset,
    size_t section_size,
    uint16_t initial_hash,
    uint16_t expected_checksum,
    CSB_V1_CSBWin512BodySectionReport *section,
    uint8_t **out_decoded)
{
    uint8_t *decoded = NULL;
    int rc;
    size_t end;

    if (!bytes || !section || !out_decoded) {
        return CSB_V1_CSBWIN_512_ERR_ARGUMENT;
    }
    memset(section, 0, sizeof(*section));
    section->kind = kind;
    section->encrypted_offset = offset;
    section->encrypted_size = section_size;
    section->initial_hash = initial_hash;
    section->expected_checksum = expected_checksum;

    if (section_size == 0u) {
        section->present = 0;
        section->checksum_ok = 1;
        *out_decoded = NULL;
        return CSB_V1_CSBWIN_512_OK;
    }
    if ((section_size & 1u) != 0u ||
        !checked_add_size(offset, section_size, &end) ||
        end > size) {
        return CSB_V1_CSBWIN_512_ERR_TOO_SMALL;
    }
    decoded = (uint8_t *)malloc(section_size);
    if (!decoded) {
        return CSB_V1_CSBWIN_512_ERR_ARGUMENT;
    }
    rc = csb_v1_csbwin_512_decode_stream_section(
        bytes + offset, section_size, initial_hash, expected_checksum,
        decoded, section_size);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        free(decoded);
        return rc;
    }
    section->present = 1;
    section->checksum_ok = 1;
    *out_decoded = decoded;
    return CSB_V1_CSBWIN_512_OK;
}

int csb_v1_csbwin_512_verify_save_body(
    const uint8_t *bytes,
    size_t size,
    uint16_t timer_record_size,
    CSB_V1_CSBWin512BodyReport *out)
{
    CSB_V1_CSBWin512BodySectionReport section;
    uint8_t *block2 = NULL;
    uint8_t *decoded = NULL;
    size_t offset = CSB_V1_CSBWIN_BLOCK1_BYTES;
    size_t item16_size = 0u;
    size_t timer_size = 0u;
    size_t timer_queue_size = 0u;
    int rc;

    if (!bytes || !out) {
        return CSB_V1_CSBWIN_512_ERR_ARGUMENT;
    }
    memset(out, 0, sizeof(*out));
    if (timer_record_size == 0u) {
        timer_record_size = 16u;
    }
    if (timer_record_size != 10u &&
        timer_record_size != 12u &&
        timer_record_size != 16u) {
        return CSB_V1_CSBWIN_512_ERR_ARGUMENT;
    }
    out->timer_record_size = timer_record_size;

    rc = csb_v1_csbwin_512_xor_pad_classify(bytes, size, &out->header);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        return rc;
    }
    if (out->header.verdict == CSB_V1_CSBWIN_512_VERDICT_NEITHER) {
        return CSB_V1_CSBWIN_512_ERR_BAD_KEYS;
    }
    out->header_valid = 1;

    /* CSBWin/SaveGame.cpp lines 1768-1775: GAMEBLOCK2 is the
     * first body section after GAMEBLOCK1 and is always 128 bytes. */
    rc = verify_body_section(
        bytes, size, CSB_V1_CSBWIN_512_SECTION_BLOCK2, offset, 128u,
        out->header.public_fields.csbwin_block2_hash,
        out->header.public_fields.csbwin_block2_checksum,
        &section, &block2);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        return rc;
    }
    out->sections[CSB_V1_CSBWIN_512_SECTION_BLOCK2] = section;
    ++out->sections_verified;
    offset += 128u;

    /* CSBWin SaveGame.cpp GAMEBLOCK2 lines 106-132, consumed by
     * the load path at lines 1775-1811 after swapBlock2(). These
     * fields are the bounded startup/resume handoff summary; the
     * raw decoded sections are still not imported here. */
    out->game_time = read_le32(block2, 0u);
    out->random_seed = read_le32(block2, 4u);
    out->object_in_hand = read_le16(block2, 8u);
    out->num_character = read_le16(block2, 10u);
    out->party_x = read_le16(block2, 12u);
    out->party_y = read_le16(block2, 14u);
    out->party_facing = read_le16(block2, 16u);
    out->party_level = read_le16(block2, 18u);
    out->hand_char = read_le16(block2, 20u);
    out->magic_caster = read_le16(block2, 22u);
    out->num_timer = read_le16(block2, 24u);
    out->first_avail_timer = read_le16(block2, 26u);
    out->max_timers = read_le16(block2, 28u);
    out->item16_queue_len = read_le16(block2, 30u);
    out->last_monster_attack_time = read_le32(block2, 32u);
    out->last_party_move_time = read_le32(block2, 36u);
    out->party_move_disable_timer = read_le16(block2, 40u);
    out->word11712 = read_le16(block2, 42u);
    out->word11714 = read_le16(block2, 44u);
    out->max_item16 = read_le16(block2, 46u);
    out->timer_sequence = read_le16(block2, 48u);
    free(block2);
    block2 = NULL;

    if (!checked_mul_size((size_t)out->max_item16, 16u, &item16_size) ||
        !checked_mul_size((size_t)out->max_timers,
                          (size_t)timer_record_size, &timer_size) ||
        !checked_mul_size((size_t)out->max_timers, 2u,
                          &timer_queue_size)) {
        return CSB_V1_CSBWIN_512_ERR_TOO_SMALL;
    }

    /* CSBWin/SaveGame.cpp lines 1822-1831 skips ITEM16 when
     * MaxITEM16 is zero; mirror that by reporting a zero-size
     * absent-but-ok section. */
    rc = verify_body_section(
        bytes, size, CSB_V1_CSBWIN_512_SECTION_ITEM16, offset, item16_size,
        out->header.public_fields.csbwin_item16_hash,
        out->header.public_fields.csbwin_item16_checksum,
        &section, &decoded);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        return rc;
    }
    out->sections[CSB_V1_CSBWIN_512_SECTION_ITEM16] = section;
    ++out->sections_verified;
    free(decoded);
    decoded = NULL;
    offset += item16_size;

    rc = verify_body_section(
        bytes, size, CSB_V1_CSBWIN_512_SECTION_CHARACTERS, offset, 3328u,
        out->header.public_fields.csbwin_character_hash,
        out->header.public_fields.csbwin_character_checksum,
        &section, &decoded);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        return rc;
    }
    out->sections[CSB_V1_CSBWIN_512_SECTION_CHARACTERS] = section;
    ++out->sections_verified;
    {
        size_t champion_index;
        /* CSBWin/SaveGame.cpp:1838 calls swapCharacterData() after reading
         * the 3328-byte character body. CSBWin/CSB.h CHARDESC lines
         * 2486-2597 defines four 800-byte CHARDESC records at the start of
         * that body; SaveGame.cpp:489-520 lists every endian-swapped field.
         * We surface a lossless fixed-offset summary here and leave portrait
         * bytes plus trailing PartyFootprints/Brightness state for later. */
        for (champion_index = 0u;
             champion_index < CSBWIN_CHARDESC_COUNT;
             ++champion_index) {
            parse_champion_summary(
                decoded + champion_index * CSBWIN_CHARDESC_STRIDE,
                &out->champions[champion_index]);
        }
    }
    free(decoded);
    decoded = NULL;
    offset += 3328u;

    rc = verify_body_section(
        bytes, size, CSB_V1_CSBWIN_512_SECTION_TIMERS, offset, timer_size,
        out->header.public_fields.csbwin_timers_hash,
        out->header.public_fields.csbwin_timers_checksum,
        &section, &decoded);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        return rc;
    }
    out->sections[CSB_V1_CSBWIN_512_SECTION_TIMERS] = section;
    ++out->sections_verified;
    free(decoded);
    decoded = NULL;
    offset += timer_size;

    rc = verify_body_section(
        bytes, size, CSB_V1_CSBWIN_512_SECTION_TIMER_QUEUE, offset,
        timer_queue_size,
        out->header.public_fields.csbwin_timer_queue_hash,
        out->header.public_fields.csbwin_timer_queue_checksum,
        &section, &decoded);
    if (rc != CSB_V1_CSBWIN_512_OK) {
        return rc;
    }
    out->sections[CSB_V1_CSBWIN_512_SECTION_TIMER_QUEUE] = section;
    ++out->sections_verified;
    free(decoded);
    decoded = NULL;
    offset += timer_queue_size;

    out->required_size = offset;
    return CSB_V1_CSBWIN_512_OK;
}

const char *csb_v1_csbwin_512_xor_pad_result_name(int result)
{
    switch (result) {
    case CSB_V1_CSBWIN_512_OK: return "OK";
    case CSB_V1_CSBWIN_512_ERR_ARGUMENT: return "argument";
    case CSB_V1_CSBWIN_512_ERR_TOO_SMALL: return "too-small";
    case CSB_V1_CSBWIN_512_ERR_BAD_KEYS: return "bad-keys";
    case CSB_V1_CSBWIN_512_ERR_BAD_CHECKSUM: return "bad-checksum";
    default: return "unknown";
    }
}

const char *csb_v1_csbwin_512_xor_pad_verdict_name(
    CSB_V1_CSBWin512KeyVerdict verdict)
{
    switch (verdict) {
    case CSB_V1_CSBWIN_512_VERDICT_CSB: return "CSB";
    case CSB_V1_CSBWIN_512_VERDICT_DM:  return "DM";
    case CSB_V1_CSBWIN_512_VERDICT_NEITHER: return "neither";
    default: return "unknown";
    }
}

const char *csb_v1_csbwin_512_xor_pad_source_evidence(void)
{
    return
        "CSBWin/SaveGame.cpp:880 GAMEBLOCK1 (512 bytes, first block)\n"
        "CSBWin/SaveGame.cpp:1145 WriteFirstBlock / ScrambleAndWrite\n"
        "CSBWin/SaveGame.cpp:715 ScrambleAndWrite (trash first 256 bytes + write D5W^D6W)\n"
        "CSBWin/SaveGame.cpp:59-104 GAMEBLOCK1 body-section hash/checksum fields\n"
        "CSBWin/SaveGame.cpp:1768-1855 load path decodes block2/items/characters/timers\n"
        "CSBWin/SaveGame.cpp:1114-1204 write path stores body-section checksums/streams\n"
        "CSBWin/Chaos.cpp:1326 ReadGameBlock1 (read 512 bytes)\n"
        "CSBWin/Chaos.cpp:1341 UnscrambleBlock1 (first-half D6W + Unscramble + D5W)\n"
        "CSBWin/Chaos.cpp:2357 ReadSaves fallback: try CSB key (29), then DM key (10)\n"
        "CSBWin/CSBCode.cpp:9061 UnscrambleStream (section checksum gate)\n"
        "CSBWin/CSBCode.cpp:9038 Unscramble (RC4-like XOR stream)\n"
        "CSBWin/Hint.cpp:601 Unscramble (same algorithm on HCSB.HTC hint text)\n"
        "ReDMCSB DEFS.H:469 DM_SAVE_HEADER Noise[149] (C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX = 10)\n"
        "ReDMCSB DEFS.H:480 CSB_SAVE_HEADER Noise[150] (C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX = 29)\n"
        "ReDMCSB DEFS.H:483 CSB_SAVE_HEADER SaveHeader /* 512 bytes */\n"
        "ReDMCSB DEFS.H:500 C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX\n"
        "ReDMCSB DEFS.H:501 C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX\n"
        "docs/FIRESTAFF_GAP_LIST.md row C3 / A3 CSBWin custom resource handling";
}
