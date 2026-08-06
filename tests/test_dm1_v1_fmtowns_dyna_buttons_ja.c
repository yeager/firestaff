#include "dm1_v1_fmtowns_dyna_buttons_ja.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int bytes_equal(const char *s, const unsigned char *want, size_t n) {
    if (!s) return 0;
    if (strlen(s) != n) return 0;
    return memcmp(s, want, n) == 0;
}

static void test_count_constant(void) {
    assert(DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT_JA == 44u);
    assert(DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL_JA == 0xFFu);
}

static void test_ascii_placeholder_slots(void) {
    /* Slots 0, 3, 26 are single-char ASCII placeholders in both
     * English and Japanese pools (JDM 1:1 with EDM). */
    assert(strcmp(dm1_v1_fmtowns_dyna_button_label_ja_pc34(0), "N") == 0);
    assert(strcmp(dm1_v1_fmtowns_dyna_button_label_ja_pc34(3), "X") == 0);
    assert(strcmp(dm1_v1_fmtowns_dyna_button_label_ja_pc34(26), "X") == 0);
}

static void test_key_kanji_labels_byte_exact(void) {
    /* Byte-verified Shift-JIS sequences from JDM.EXP at vaddr
     * 0x243bc. Every byte must match the executable exactly. */
    static const unsigned char block[]     = {0x82,0xb3,0x82,0xa6,0x82,0xac,0x82,0xe9}; /* さえぎる */
    static const unsigned char chop[]      = {0x92,0x40,0x82,0xab,0x90,0xd8,0x82,0xe9}; /* 叩き切る */
    static const unsigned char blow_horn[] = {0x8a,0x70,0x93,0x4a,0x82,0xf0,0x90,0x81,0x82,0xad}; /* 角笛を吹く */
    static const unsigned char war_cry[]   = {0x82,0xc6,0x82,0xab,0x82,0xcc,0x90,0xba}; /* ときの声 */
    static const unsigned char fireball[]  = {0x89,0xce,0x89,0x8a,0x92,0x65};           /* 火炎弾 */
    static const unsigned char spellshld[] = {0x8e,0xf4,0x95,0xb6,0x96,0x68,0x8c,0xe4}; /* 呪文防御 */
    static const unsigned char fuse[]      = {0x97,0x5a,0x8d,0x87,0x82,0xb7,0x82,0xe9}; /* 融合する */

    assert(bytes_equal(dm1_v1_fmtowns_dyna_button_label_ja_pc34(1),
                       block, sizeof(block)));
    assert(bytes_equal(dm1_v1_fmtowns_dyna_button_label_ja_pc34(2),
                       chop, sizeof(chop)));
    assert(bytes_equal(dm1_v1_fmtowns_dyna_button_label_ja_pc34(4),
                       blow_horn, sizeof(blow_horn)));
    assert(bytes_equal(dm1_v1_fmtowns_dyna_button_label_ja_pc34(8),
                       war_cry, sizeof(war_cry)));
    assert(bytes_equal(dm1_v1_fmtowns_dyna_button_label_ja_pc34(20),
                       fireball, sizeof(fireball)));
    assert(bytes_equal(dm1_v1_fmtowns_dyna_button_label_ja_pc34(33),
                       spellshld, sizeof(spellshld)));
    assert(bytes_equal(dm1_v1_fmtowns_dyna_button_label_ja_pc34(43),
                       fuse, sizeof(fuse)));
}

static void test_half_width_kana_flip(void) {
    /* Index 5 in the Japanese pool is the half-width kana form
     * ｺｲﾝﾄｽ (transliteration of "COIN TOSS", mirroring English
     * "FLIP"). Verify the exact 5 raw bytes. */
    static const unsigned char kana[] = {0xba, 0xb2, 0xdd, 0xc4, 0xbd};
    assert(bytes_equal(dm1_v1_fmtowns_dyna_button_label_ja_pc34(5),
                       kana, sizeof(kana)));
}

static void test_out_of_range(void) {
    assert(dm1_v1_fmtowns_dyna_button_label_ja_pc34(44) == NULL);
    assert(dm1_v1_fmtowns_dyna_button_label_ja_pc34(100) == NULL);
}

static void test_sentinel_returns_empty(void) {
    const char *s = dm1_v1_fmtowns_dyna_button_label_ja_pc34(
            DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL_JA);
    assert(s != NULL);
    assert(s[0] == '\0');
}

static void test_byte_len_helpers(void) {
    /* Single-ASCII entries are 1 byte long. */
    assert(dm1_v1_fmtowns_dyna_button_label_byte_len_ja_pc34(0) == 1);
    assert(dm1_v1_fmtowns_dyna_button_label_byte_len_ja_pc34(3) == 1);
    /* 4-char kanji is 8 raw bytes. */
    assert(dm1_v1_fmtowns_dyna_button_label_byte_len_ja_pc34(1) == 8);
    /* 5-char kanji is 10 raw bytes. */
    assert(dm1_v1_fmtowns_dyna_button_label_byte_len_ja_pc34(4) == 10);
    /* Half-width kana pool at index 5 is 5 raw bytes. */
    assert(dm1_v1_fmtowns_dyna_button_label_byte_len_ja_pc34(5) == 5);
    /* Sentinel and out-of-range return 0. */
    assert(dm1_v1_fmtowns_dyna_button_label_byte_len_ja_pc34(
                DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL_JA) == 0);
    assert(dm1_v1_fmtowns_dyna_button_label_byte_len_ja_pc34(200) == 0);
}

static void test_source_span_bytes(void) {
    /* Byte-verified from JDM.EXP: sum of Shift-JIS byte lengths
     * of the 44 real labels + 44 NUL separators = 336 bytes.
     * (Contrast the English pool at 289 bytes.) */
    assert(dm1_v1_fmtowns_dyna_buttons_source_span_bytes_ja_pc34() == 336u);
}

int main(void) {
    test_count_constant();
    test_ascii_placeholder_slots();
    test_key_kanji_labels_byte_exact();
    test_half_width_kana_flip();
    test_out_of_range();
    test_sentinel_returns_empty();
    test_byte_len_helpers();
    test_source_span_bytes();
    printf("All dm1_v1_fmtowns_dyna_buttons_ja tests passed.\n");
    return 0;
}
