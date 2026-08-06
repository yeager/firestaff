#include "dm1_v1_fmtowns_jdm_bss.h"
#include <assert.h>
#include <stdio.h>

static void test_triangulated_scalars(void) {
    /* Scalars recovered by XREF triangulation (multi-instruction
     * fingerprint match). Vote counts recorded in evidence:
     *   MENU_OWNER    - 8 votes (block shift +0x228)
     *   MENU_ICONS    - 1 vote  (block shift +0x228)
     *   PARTY_SIZE    - 24 votes (block shift +0x26c)
     *   SCR_X_SIZE    - 3 votes  (block shift +0x264)
     *   CHAR_Y_SIZE   - 2 votes  (block shift +0x276)
     *   CHAR_Y_SPC    - 2 votes  (block shift +0x276)
     *   CHAR_Y_HYT    - 2 votes  (block shift +0x276)
     *   CHAR_X_WID    - 5 votes  (block shift +0x264)
     *   ICON_X_SIZE   - 2 votes  (block shift +0x264)
     *   ICON_Y_SIZE   - 1 vote   (block shift +0x264)
     */
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("MENU_OWNER")   == 0x2437eu);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("MENU_ICONS")   == 0x24384u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("PARTY_SIZE")   == 0x29690u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("SCR_X_SIZE")   == 0x26eccu);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_Y_SIZE")  == 0x26f02u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_Y_SPC")   == 0x26f06u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_Y_HYT")   == 0x26f0cu);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_X_WID")   == 0x26f0au);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("ICON_X_SIZE")  == 0x26edcu);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("ICON_Y_SIZE")  == 0x26edeu);
}

static void test_neighbor_derived_scalars(void) {
    /* Derived from a recovered peer by identical BSS-block stride. */
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("NUM_DYNABTNS")   == 0x24380u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("REDRAW_MENU")    == 0x24382u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("DYNAMENU")       == 0x243b4u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("DYNA_BUTTONS")   == 0x243bcu);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("ICON_SIZE")      == 0x26edau);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_X_SIZE")    == 0x26f00u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_X_SPC")     == 0x26f04u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_DESCENDER") == 0x26f08u);
}

static void test_block_stride_consistency(void) {
    /* Verify JDM preserves EDM's within-block byte stride exactly. */
    /* Menu-owner block (EDM MENU_OWNER..MENU_ICONS = 0x24156..0x2415c) */
    uint32_t owner = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("MENU_OWNER");
    uint32_t nbtns = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("NUM_DYNABTNS");
    uint32_t rdrw  = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("REDRAW_MENU");
    uint32_t ico   = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("MENU_ICONS");
    assert(nbtns - owner == 2u);
    assert(rdrw  - nbtns == 2u);
    assert(ico   - rdrw  == 2u);
    /* DYNAMENU..DYNA_BUTTONS diff = 8 (matches EDM 0x2418c..0x24194). */
    uint32_t dmen  = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("DYNAMENU");
    uint32_t dbtns = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("DYNA_BUTTONS");
    assert(dbtns - dmen == 8u);
    /* Character-metrics block strides. */
    uint32_t cxs = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_X_SIZE");
    uint32_t cys = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_Y_SIZE");
    uint32_t cxp = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_X_SPC");
    uint32_t cyp = dm1_v1_fmtowns_jdm_bss_vaddr_pc34("CHAR_Y_SPC");
    assert(cys - cxs == 2u);
    assert(cxp - cys == 2u);
    assert(cyp - cxp == 2u);
}

static void test_lookup_edge_cases(void) {
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("unknown_symbol") == 0u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34(NULL) == 0u);
    assert(dm1_v1_fmtowns_jdm_bss_vaddr_pc34("") == 0u);
}

static void test_count(void) {
    /* 18 scalars: 4 menu-owner block + 2 DYNA block + 4 icon/screen
     * block + 7 char-metrics + 1 party state = 18. */
    assert(dm1_v1_fmtowns_jdm_bss_count_pc34() == 18u);
}

int main(void) {
    test_triangulated_scalars();
    test_neighbor_derived_scalars();
    test_block_stride_consistency();
    test_lookup_edge_cases();
    test_count();
    printf("All dm1_v1_fmtowns_jdm_bss tests passed.\n");
    return 0;
}
