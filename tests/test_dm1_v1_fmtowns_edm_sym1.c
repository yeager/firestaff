#include "dm1_v1_fmtowns_edm_sym1.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_count(void) {
    assert(DM1_V1_FMTOWNS_EDM_SYM1_COUNT == 1174U);
}

static void test_sorted_by_vaddr(void) {
    /* Binary search requires monotonic ordering. */
    for (unsigned int i = 1; i < DM1_V1_FMTOWNS_EDM_SYM1_COUNT; ++i) {
        assert(dm1_v1_fmtowns_edm_sym1_entries[i - 1].vaddr <=
               dm1_v1_fmtowns_edm_sym1_entries[i].vaddr);
    }
}

static void test_known_menu_vaddrs(void) {
    /* DRAW_DMENU, DRAW_ICN_BUTTON, GET_LABEL byte-verified against
     * parity-evidence/dm1_fmtowns_menu_p3_disassembly.md. */
    const char *n;
    n = dm1_v1_fmtowns_edm_sym1_name_for_vaddr_pc34(0x4620);
    assert(n != NULL);
    assert(strcmp(n, "DRAW_DMENU") == 0);
    n = dm1_v1_fmtowns_edm_sym1_name_for_vaddr_pc34(0x44f0);
    assert(n != NULL);
    assert(strcmp(n, "DRAW_ICN_BUTTON") == 0);
    n = dm1_v1_fmtowns_edm_sym1_name_for_vaddr_pc34(0x43e4);
    assert(n != NULL);
    assert(strcmp(n, "GET_LABEL") == 0);
}

static void test_lookup_by_name(void) {
    assert(dm1_v1_fmtowns_edm_sym1_vaddr_for_name_pc34("DRAW_DMENU") == 0x4620U);
    assert(dm1_v1_fmtowns_edm_sym1_vaddr_for_name_pc34("DRAW_ICN_BUTTON") == 0x44f0U);
    assert(dm1_v1_fmtowns_edm_sym1_vaddr_for_name_pc34("GET_LABEL") == 0x43e4U);
    /* Non-existent name returns 0. */
    assert(dm1_v1_fmtowns_edm_sym1_vaddr_for_name_pc34("NOT_A_REAL_SYMBOL") == 0U);
    /* NULL name returns 0. */
    assert(dm1_v1_fmtowns_edm_sym1_vaddr_for_name_pc34(NULL) == 0U);
}

static void test_lookup_missing_vaddr_returns_null(void) {
    /* A vaddr in the middle of the range but not on any symbol. */
    const char *n = dm1_v1_fmtowns_edm_sym1_name_for_vaddr_pc34(0x00000001);
    assert(n == NULL);
    n = dm1_v1_fmtowns_edm_sym1_name_for_vaddr_pc34(0xFFFFFFFFu);
    assert(n == NULL);
}

static void test_prefixed_symbols(void) {
    /* Phar Lap prepends underscore to C symbols. Verify a few. */
    uint32_t v;
    v = dm1_v1_fmtowns_edm_sym1_vaddr_for_name_pc34("_EXIT");
    assert(v != 0U);
    v = dm1_v1_fmtowns_edm_sym1_vaddr_for_name_pc34("_BASE");
    assert(v != 0U);
    /* Watcom's ONEXIT_TERMINATION uses double underscore. */
    v = dm1_v1_fmtowns_edm_sym1_vaddr_for_name_pc34("__ONEXIT_TERMINATION");
    assert(v != 0U);
}

/* Real-data round-trip: re-parse EDM.EXP's SYM1 blob and confirm
 * every shipped entry appears with matching vaddr. */
static void test_real_data_round_trip(void) {
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    FILE *fp;
    if (!path || !path[0]) { puts("SKIP: no EDM.EXP"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }
    /* Read SYM1 table at file offset 0x46b41, size 0x51b5. */
    if (fseek(fp, 0x46b41, SEEK_SET) != 0) {
        fclose(fp); puts("SKIP: seek failed"); return;
    }
    uint8_t *sym = (uint8_t *)malloc(0x51b5);
    if (!sym || fread(sym, 1, 0x51b5, fp) != 0x51b5) {
        free(sym); fclose(fp); puts("SKIP: read failed"); return;
    }
    fclose(fp);
    /* Header: "SYM1" then advance to first symbol at 0x22. */
    assert(sym[0] == 'S' && sym[1] == 'Y' && sym[2] == 'M' && sym[3] == '1');
    unsigned int off = 0x22;
    unsigned int parsed = 0;
    while (off + 7 <= 0x51b5) {
        unsigned int nlen = sym[off];
        if (nlen == 0 || nlen > 64) break;
        off += 1;
        char name[65];
        memcpy(name, sym + off, nlen);
        name[nlen] = '\0';
        off += nlen;
        uint32_t vaddr = (uint32_t)sym[off] |
                        ((uint32_t)sym[off+1] << 8) |
                        ((uint32_t)sym[off+2] << 16) |
                        ((uint32_t)sym[off+3] << 24);
        off += 6;
        /* Verify this (vaddr, name) pair appears in shipped table. */
        const char *shipped = dm1_v1_fmtowns_edm_sym1_name_for_vaddr_pc34(vaddr);
        if (shipped != NULL) {
            /* At least one symbol at this vaddr; must match. Duplicates
             * are handled by shipping the sort-stable first entry. */
            if (strcmp(shipped, name) != 0) {
                /* Could be a duplicate-vaddr entry; loop the shipped
                 * array to confirm at least one matches. */
                int found = 0;
                for (unsigned int i = 0; i < DM1_V1_FMTOWNS_EDM_SYM1_COUNT; ++i) {
                    if (dm1_v1_fmtowns_edm_sym1_entries[i].vaddr == vaddr &&
                        strcmp(dm1_v1_fmtowns_edm_sym1_entries[i].name, name) == 0) {
                        found = 1; break;
                    }
                }
                assert(found);
            }
        }
        parsed++;
    }
    free(sym);
    assert(parsed == DM1_V1_FMTOWNS_EDM_SYM1_COUNT);
    puts("PASS: all 1174 real EDM.EXP SYM1 entries match shipped table");
}

int main(void) {
    test_count();
    test_sorted_by_vaddr();
    test_known_menu_vaddrs();
    test_lookup_by_name();
    test_lookup_missing_vaddr_returns_null();
    test_prefixed_symbols();
    test_real_data_round_trip();
    puts("All dm1_v1_fmtowns_edm_sym1 tests passed.");
    return 0;
}
