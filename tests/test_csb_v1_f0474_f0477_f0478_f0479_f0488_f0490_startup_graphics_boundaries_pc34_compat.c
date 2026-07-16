#include "csb_v1_f0474_f0477_f0478_f0479_f0488_f0490_startup_graphics_boundaries_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static void test_no_synthetic_startup_graphics_boundaries(void)
{
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 receipt;
    csb_v1_f047x_startup_graphics_boundary_receipt_init_pc34(&receipt);

    CHECK(F0477_MEMORY_OpenGraphicsDat_CPSDF(&receipt) == 0);
    CHECK(F0479_MEMORY_ReadGraphicsDatHeader(&receipt) == 0);
    CHECK(F0490_MEMORY_LoadDecompressAndExpandGraphic(&receipt) == 0);
    CHECK(F0474_MEMORY_LoadGraphic_CPSDF(&receipt) == 0);
    CHECK(F0488_MEMORY_ExpandGraphicToBitmap(&receipt) == 0);
    F0478_MEMORY_CloseGraphicsDat_CPSDF(&receipt);

    CHECK((receipt.attempted_stage_mask &
           CSB_V1_STARTUP_GRAPHICS_ALL_BOUNDARY_STAGES_PC34) ==
          CSB_V1_STARTUP_GRAPHICS_ALL_BOUNDARY_STAGES_PC34);
    CHECK((receipt.no_synthetic_fallback_mask &
           CSB_V1_STARTUP_GRAPHICS_ALL_BOUNDARY_STAGES_PC34) ==
          CSB_V1_STARTUP_GRAPHICS_ALL_BOUNDARY_STAGES_PC34);
    CHECK((receipt.completed_stage_mask &
           CSB_V1_STARTUP_GRAPHICS_F0478_CLOSE_GRAPHICS_DAT_PC34) != 0u);
    CHECK((receipt.completed_stage_mask &
           ~CSB_V1_STARTUP_GRAPHICS_F0478_CLOSE_GRAPHICS_DAT_PC34) == 0u);
    CHECK((receipt.blocked_stage_mask &
           ~CSB_V1_STARTUP_GRAPHICS_F0478_CLOSE_GRAPHICS_DAT_PC34) ==
          (CSB_V1_STARTUP_GRAPHICS_ALL_BOUNDARY_STAGES_PC34 &
           ~CSB_V1_STARTUP_GRAPHICS_F0478_CLOSE_GRAPHICS_DAT_PC34));
}

static void test_evidence_strings(void)
{
    check_contains(csb_v1_f0474_load_graphic_source_evidence_pc34(),
                   "MEMORY.C:707-731");
    check_contains(csb_v1_f0474_load_graphic_source_evidence_pc34(),
                   "102-105");

    check_contains(csb_v1_f0477_open_graphics_dat_source_evidence_pc34(),
                   "MEMORY.C:1212-1285");
    check_contains(csb_v1_f0477_open_graphics_dat_source_evidence_pc34(),
                   "82-84");

    check_contains(csb_v1_f0478_close_graphics_dat_source_evidence_pc34(),
                   "MEMORY.C:1287-1295");
    check_contains(csb_v1_f0478_close_graphics_dat_source_evidence_pc34(),
                   "133");

    check_contains(csb_v1_f0479_read_graphics_dat_header_source_evidence_pc34(),
                   "MEMORY.C:1330-1351");
    check_contains(csb_v1_f0479_read_graphics_dat_header_source_evidence_pc34(),
                   "82-91");

    check_contains(csb_v1_f0488_expand_graphic_source_evidence_pc34(),
                   "MEMORY.C:2474-2503");
    check_contains(csb_v1_f0488_expand_graphic_source_evidence_pc34(),
                   "IMG3 four-plane decode");

    check_contains(csb_v1_f0490_load_decompress_expand_source_evidence_pc34(),
                   "MEMORY.C:2583-2654");
    check_contains(csb_v1_f0490_load_decompress_expand_source_evidence_pc34(),
                   "82-133");
}

int main(void)
{
    test_no_synthetic_startup_graphics_boundaries();
    test_evidence_strings();
    return 0;
}
