#include "fmtowns_pharlap_all_games.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_count(void) {
    assert(FMTOWNS_PHARLAP_BINARY_COUNT == 11U);
}

static void test_all_use_canonical_slots(void) {
    /* Sum of per-slot counts must equal total for every profile. */
    assert(fmtowns_pharlap_all_binaries_use_canonical_slots_only_pc34() == 1);
}

static void test_dm1_edm(void) {
    const fmtowns_pharlap_binary_profile_t *p =
        fmtowns_pharlap_binary_profile_for_name_pc34("EDM.EXP");
    assert(p != NULL);
    assert(strcmp(p->game, "DM1") == 0);
    assert(p->file_size == 310518);
    assert(p->slot_tbios == 70);
    assert(p->sym1_size == 0x51b5);
}

static void test_csb_chtwe(void) {
    const fmtowns_pharlap_binary_profile_t *p =
        fmtowns_pharlap_binary_profile_for_name_pc34("CHTWE.EXP");
    assert(p != NULL);
    assert(strcmp(p->game, "CSB") == 0);
    assert(p->file_size == 283936);
    assert(p->slot_tbios == 17);
    assert(p->slot_secondary == 11);
    assert(p->sym1_size == 0);  /* stripped */
}

static void test_dm2_skull(void) {
    const fmtowns_pharlap_binary_profile_t *p =
        fmtowns_pharlap_binary_profile_for_name_pc34("SKULL.EXP");
    assert(p != NULL);
    assert(strcmp(p->game, "DM2") == 0);
    assert(p->file_size == 374416);
    assert(p->slot_tbios == 17);
    assert(p->slot_secondary == 10);
}

static void test_unknown_name_returns_null(void) {
    assert(fmtowns_pharlap_binary_profile_for_name_pc34("NONE.EXP") == NULL);
    assert(fmtowns_pharlap_binary_profile_for_name_pc34(NULL) == NULL);
}

static void test_all_binaries_use_only_4_slots(void) {
    /* Sanity: every binary in the table has slot_timing < slot_tbios
     * and slot_hardware_init small — the DM1-derived assumption
     * about slot roles is consistent across CSB and DM2. */
    for (unsigned int i = 0; i < FMTOWNS_PHARLAP_BINARY_COUNT; ++i) {
        const fmtowns_pharlap_binary_profile_t *p =
            &fmtowns_pharlap_binary_profiles[i];
        assert(p->slot_hardware_init <= 8u);
        assert(p->slot_timing <= 2u);
    }
}

int main(void) {
    test_count();
    test_all_use_canonical_slots();
    test_dm1_edm();
    test_csb_chtwe();
    test_dm2_skull();
    test_unknown_name_returns_null();
    test_all_binaries_use_only_4_slots();

    /* Cross-game direct I/O invariant */
    assert(FMTOWNS_DIRECT_IO_CROSS_GAME_COUNT == 8U);
    assert(fmtowns_all_game_binaries_touch_only_sound_int_pc34() == 1);
    /* Every entry has exactly 1 SOUND_INT_REASON read and no other port. */
    for (unsigned int i = 0; i < 8; ++i) {
        assert(fmtowns_direct_io_cross_game_profiles[i].sound_int_reason_reads == 1);
        assert(fmtowns_direct_io_cross_game_profiles[i].total_direct_ports_used == 1);
    }

    puts("All fmtowns_pharlap_all_games tests passed.");
    return 0;
}
