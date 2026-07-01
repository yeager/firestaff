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
 * attempting the in-place unscramble. The classifier never
 * decodes anything beyond the documented GAMEBLOCK1 public
 * fields. It does not bind into M11/M12, does not promise
 * end-to-end CSBWin save import, and remains tracked under
 * docs/FIRESTAFF_GAP_LIST.md row C3 / A3 (CSBWin custom
 * resource handling) as a bounded advance.
 */

#include "csb_v1_csbwin_512_xor_pad_classify.h"

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

    if (key_index < 0 || key_index > 0xFFFFu) {
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

const char *csb_v1_csbwin_512_xor_pad_result_name(int result)
{
    switch (result) {
    case CSB_V1_CSBWIN_512_OK: return "OK";
    case CSB_V1_CSBWIN_512_ERR_ARGUMENT: return "argument";
    case CSB_V1_CSBWIN_512_ERR_TOO_SMALL: return "too-small";
    case CSB_V1_CSBWIN_512_ERR_BAD_KEYS: return "bad-keys";
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
        "CSBWin/Chaos.cpp:1326 ReadGameBlock1 (read 512 bytes)\n"
        "CSBWin/Chaos.cpp:1341 UnscrambleBlock1 (first-half D6W + Unscramble + D5W)\n"
        "CSBWin/Chaos.cpp:2357 ReadSaves fallback: try CSB key (29), then DM key (10)\n"
        "CSBWin/CSBCode.cpp:9038 Unscramble (RC4-like XOR stream)\n"
        "CSBWin/Hint.cpp:601 Unscramble (same algorithm on HCSB.HTC hint text)\n"
        "ReDMCSB DEFS.H:469 DM_SAVE_HEADER Noise[149] (C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX = 10)\n"
        "ReDMCSB DEFS.H:480 CSB_SAVE_HEADER Noise[150] (C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX = 29)\n"
        "ReDMCSB DEFS.H:483 CSB_SAVE_HEADER SaveHeader /* 512 bytes */\n"
        "ReDMCSB DEFS.H:500 C10_DM_SAVE_HEADER_DECRYPTION_KEY_INDEX\n"
        "ReDMCSB DEFS.H:501 C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX\n"
        "docs/FIRESTAFF_GAP_LIST.md row C3 / A3 CSBWin custom resource handling";
}
