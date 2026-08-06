#include "dm1_v1_fmtowns_dyna_buttons.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_all_44_labels_byte_verified(void) {
    /* Exact byte-verified strings from EDM.EXP's DYNA_BUTTONS at
     * vaddr 0x24194. Order and case match the executable exactly. */
    static const char *const expected[44] = {
        "N", "BLOCK", "CHOP", "X", "BLOW HORN", "FLIP", "PUNCH",
        "KICK", "WAR CRY", "STAB", "CLIMB DOWN", "FREEZE LIFE",
        "HIT", "SWING", "STAB", "THRUST", "JAB", "PARRY", "HACK",
        "BERZERK", "FIREBALL", "DISPELL", "CONFUSE", "LIGHTNING",
        "DISRUPT", "MELEE", "X", "INVOKE", "SLASH", "CLEAVE",
        "BASH", "STUN", "SHOOT", "SPELLSHIELD", "FIRESHIELD",
        "FLUXCAGE", "HEAL", "CALM", "LIGHT", "WINDOW", "SPIT",
        "BRANDISH", "THROW", "FUSE",
    };
    unsigned int i;
    for (i = 0; i < 44; ++i) {
        const char *actual = dm1_v1_fmtowns_dyna_button_label_pc34(i);
        assert(actual != NULL);
        assert(strcmp(actual, expected[i]) == 0);
    }
}

static void test_count_constant(void) {
    assert(DM1_V1_FMTOWNS_DYNA_BUTTONS_COUNT == 44u);
}

static void test_out_of_range_returns_null(void) {
    assert(dm1_v1_fmtowns_dyna_button_label_pc34(44) == NULL);
    assert(dm1_v1_fmtowns_dyna_button_label_pc34(100) == NULL);
    /* 0xFE is not the 0xFF sentinel; must reject. */
    assert(dm1_v1_fmtowns_dyna_button_label_pc34(0xFEu) == NULL);
}

static void test_sentinel_returns_empty_string(void) {
    /* GET_LABEL (EDM.EXP 0x43e4) treats 0xFF as sentinel and
     * returns a fixed empty-string pointer. */
    const char *s = dm1_v1_fmtowns_dyna_button_label_pc34(
            DM1_V1_FMTOWNS_DYNA_LABEL_SENTINEL);
    assert(s != NULL);
    assert(s[0] == '\0');
}

static void test_source_span_bytes(void) {
    /* Sum of ASCII lengths of the 44 labels + 44 NULs = 289 bytes.
     * The remaining 11 bytes up to symbol SPELLS at 0x242c0 are the
     * 11 empty padding entries the linker emitted. Total 289 + 11 =
     * 300, matching (0x242c0 - 0x24194). */
    assert(dm1_v1_fmtowns_dyna_buttons_source_span_bytes_pc34() == 289u);
}

static void test_get_label_semantic_signposts(void) {
    /* Spot-check a few well-known verbs to catch typos. */
    assert(strcmp(dm1_v1_fmtowns_dyna_button_label_pc34(1), "BLOCK") == 0);
    assert(strcmp(dm1_v1_fmtowns_dyna_button_label_pc34(20), "FIREBALL") == 0);
    assert(strcmp(dm1_v1_fmtowns_dyna_button_label_pc34(33),
                  "SPELLSHIELD") == 0);
    assert(strcmp(dm1_v1_fmtowns_dyna_button_label_pc34(43), "FUSE") == 0);
}

int main(void) {
    test_all_44_labels_byte_verified();
    test_count_constant();
    test_out_of_range_returns_null();
    test_sentinel_returns_empty_string();
    test_source_span_bytes();
    test_get_label_semantic_signposts();
    printf("All dm1_v1_fmtowns_dyna_buttons tests passed.\n");
    return 0;
}
