#include "dm1_v1_m201_m250_source_audit_pc34_compat.h"

#define NO_ROUTE(number, name, anchor) {number, name, anchor, DM1_V1_M201_M250_UNVERIFIED_NO_ROUTE_PC34}
#define ABSENT(number) {number, "M" #number "_absent", "ReDMCSB label inventory", DM1_V1_M201_M250_ABSENT_PC34}

static const Dm1V1M201M250SourceAuditPc34 kSymbols[] = {
    NO_ROUTE(201, "M201_PREVIOUS_BLOCK_SIZE", "DEFS.H:5245; UTMEMORY.C"),
    NO_ROUTE(202, "M202_BLOCK_SIZE", "DEFS.H:5246; UTMEMORY.C"),
    NO_ROUTE(203, "M203_USED_BLOCK_USAGE_COUNT", "DEFS.H:5247"),
    NO_ROUTE(204, "M204_USED_BLOCK_PREVIOUS_INDEX", "DEFS.H:5248"),
    NO_ROUTE(205, "M205_USED_BLOCK_NEXT_INDEX", "DEFS.H:5249"),
    NO_ROUTE(206, "M206_USED_BLOCK_BITMAP_INDEX", "DEFS.H:5250"),
    NO_ROUTE(207, "M207_USED_BLOCK_BITMAP", "DEFS.H:5251"),
    ABSENT(208), ABSENT(209), ABSENT(210), ABSENT(211), ABSENT(212),
    ABSENT(213), ABSENT(214), ABSENT(215), ABSENT(216), ABSENT(217),
    ABSENT(218), ABSENT(219), ABSENT(220), ABSENT(221), ABSENT(222),
    ABSENT(223), ABSENT(224), ABSENT(225), ABSENT(226), ABSENT(227),
    ABSENT(228), ABSENT(229), ABSENT(230), ABSENT(231), ABSENT(232),
    ABSENT(233), ABSENT(234), ABSENT(235), ABSENT(236), ABSENT(237),
    ABSENT(238), ABSENT(239), ABSENT(240), ABSENT(241), ABSENT(242),
    ABSENT(243), ABSENT(244), ABSENT(245), ABSENT(246), ABSENT(247),
    ABSENT(248), ABSENT(249), ABSENT(250),
};

const Dm1V1M201M250SourceAuditPc34 *
dm1_v1_m201_m250_source_audit_pc34(unsigned int number)
{
    if (number < 201U || number > 250U) return 0;
    return &kSymbols[number - 201U];
}

int dm1_v1_m201_m250_has_verified_owner_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_m201_m250_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m201_m250_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv M201-M207; "
           "DEFS.H:5245-5251 and UTMEMORY.C. M208-M250 are absent. No generated "
           "macro, graphics, UI, input, timing, memory, or platform route.";
}
