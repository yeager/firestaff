#include "csb_v1_fmtowns_tmenu_sym1.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_count(void) {
    assert(CSB_V1_FMTOWNS_TMENU_SYM1_COUNT == 1724U);
}

static void test_sorted(void) {
    for (unsigned int i = 1; i < CSB_V1_FMTOWNS_TMENU_SYM1_COUNT; ++i) {
        assert(csb_v1_fmtowns_tmenu_sym1_entries[i-1].vaddr <=
               csb_v1_fmtowns_tmenu_sym1_entries[i].vaddr);
    }
}

static void test_known_lookups(void) {
    /* Spot checks from the SYM1 blob. */
    const char *n;
    uint32_t v;
    v = csb_v1_fmtowns_tmenu_sym1_vaddr_for_name_pc34("ACT_HANDLE");
    assert(v == 0x0000d2f3u);
    v = csb_v1_fmtowns_tmenu_sym1_vaddr_for_name_pc34("CD_Drive_Reset");
    assert(v == 0x00000520u);
    /* Reverse lookup */
    n = csb_v1_fmtowns_tmenu_sym1_name_for_vaddr_pc34(0x0000d2f3u);
    assert(n && strcmp(n, "ACT_HANDLE") == 0);
    /* Missing */
    assert(csb_v1_fmtowns_tmenu_sym1_vaddr_for_name_pc34("NOT_REAL") == 0u);
    assert(csb_v1_fmtowns_tmenu_sym1_vaddr_for_name_pc34(NULL) == 0u);
    assert(csb_v1_fmtowns_tmenu_sym1_name_for_vaddr_pc34(0xdeadbeefu) == NULL);
}

static void test_shared_rtl_with_dm1(void) {
    /* Both binaries should have identical Watcom C RTL symbols
     * (they were built with the same compiler and same runtime
     * versions). */
    uint32_t v = csb_v1_fmtowns_tmenu_sym1_vaddr_for_name_pc34("_mwfcmp");
    assert(v != 0u);
    v = csb_v1_fmtowns_tmenu_sym1_vaddr_for_name_pc34("_mwxmpy");
    assert(v != 0u);
}

int main(void) {
    test_count();
    test_sorted();
    test_known_lookups();
    test_shared_rtl_with_dm1();
    puts("All csb_v1_fmtowns_tmenu_sym1 tests passed.");
    return 0;
}
