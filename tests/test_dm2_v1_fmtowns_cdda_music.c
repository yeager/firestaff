/* FM Towns HMP-to-CDDA source-boundary tests.  The mapping is available
 * only through a parsed original SKULL.EXP receipt. */

#include "dm2_v1_fmtowns_cdda_music.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_rejects_missing_source(void)
{
    DM2_V1_FmtownsCddaMusicReceipt receipt;
    assert(dm2_v1_fmtowns_cdda_music_parse(NULL, 0u, &receipt) == 0);
    assert(!receipt.valid);
    assert(dm2_v1_fmtowns_hmp_to_cdda(NULL, 4) == 0);
}

static void test_out_of_range(void)
{
    DM2_V1_FmtownsCddaMusicReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    assert(dm2_v1_fmtowns_hmp_to_cdda(&receipt, -1) == 0);
    assert(dm2_v1_fmtowns_hmp_to_cdda(&receipt, 29) == 0);
    assert(dm2_v1_fmtowns_hmp_to_cdda(&receipt, 100) == 0);
}

static void test_table_is_unavailable(void)
{
    assert(dm2_v1_fmtowns_cdda_map_table(NULL) == NULL);
}

int main(void)
{
    printf("FM Towns CDDA source-boundary tests\n\n");
    test_rejects_missing_source(); printf("  rejects_missing_source PASS\n");
    test_out_of_range();           printf("  out_of_range           PASS\n");
    test_table_is_unavailable();   printf("  table_unavailable      PASS\n");
    printf("\n3/3 tests passed\n");
    return 0;
}
