#include "dm1_v1_m101_m150_source_audit_pc34_compat.h"

#define NO_ROUTE(number, name, anchor) {number, name, anchor, DM1_V1_M101_M150_UNVERIFIED_NO_ROUTE_PC34}
#define VERIFIED(number, name, anchor) {number, name, anchor, DM1_V1_M101_M150_VERIFIED_SHARED_OWNER_PC34}
#define ABSENT(number) {number, "M" #number "_absent", "ReDMCSB label inventory", DM1_V1_M101_M150_ABSENT_PC34}

static const Dm1V1M101M150SourceAuditPc34 kSymbols[] = {
    NO_ROUTE(101, "M101_PIXEL_HEIGHT", "DEFS.H:3445"),
    NO_ROUTE(102, "M102_XXX_RANGE", "DEFS.H:1652"),
    VERIFIED(103, "M103_BITMAP_BYTE_COUNT", "DEFS.H:3466; redmcsb_f0751_get_bitmap_byte_count_pc34_compat.c"),
    VERIFIED(104, "M104_EVEN_INTEGER", "DEFS.H:3419; redmcsb_f0684_blit_c25_pc34_compat.c"),
    NO_ROUTE(105, "M105_GRAPHIC_PIXEL_WIDTH", "DEFS.H:3478"),
    NO_ROUTE(106, "M106_USED_BLOCK_BITMAP_WIDTH", "DEFS.H:3479"),
    NO_ROUTE(107, "M107_USED_BLOCK_BITMAP_HEIGHT", "DEFS.H:3480"),
    NO_ROUTE(108, "M108_PREVIOUS", "DEFS.H:3481"),
    NO_ROUTE(109, "M109_PIXEL_WIDTH_2", "DEFS.H:3482"),
    NO_ROUTE(110, "M110_PIXEL_HEIGHT_2", "DEFS.H:3483"),
    ABSENT(111), ABSENT(112), ABSENT(113), ABSENT(114), ABSENT(115),
    ABSENT(116), ABSENT(117), ABSENT(118), ABSENT(119), ABSENT(120),
    ABSENT(121), ABSENT(122), ABSENT(123), ABSENT(124), ABSENT(125),
    ABSENT(126), ABSENT(127), ABSENT(128), ABSENT(129), ABSENT(130),
    ABSENT(131), ABSENT(132), ABSENT(133), ABSENT(134), ABSENT(135),
    ABSENT(136), ABSENT(137), ABSENT(138), ABSENT(139), ABSENT(140),
    ABSENT(141), ABSENT(142), ABSENT(143), ABSENT(144), ABSENT(145),
    ABSENT(146), ABSENT(147), ABSENT(148), ABSENT(149), ABSENT(150),
};

const Dm1V1M101M150SourceAuditPc34 *
dm1_v1_m101_m150_source_audit_pc34(unsigned int number)
{
    if (number < 101U || number > 150U) return 0;
    return &kSymbols[number - 101U];
}

int dm1_v1_m101_m150_has_verified_owner_pc34(unsigned int number)
{
    const Dm1V1M101M150SourceAuditPc34 *entry =
        dm1_v1_m101_m150_source_audit_pc34(number);
    return entry != 0 && entry->disposition == DM1_V1_M101_M150_VERIFIED_SHARED_OWNER_PC34;
}

int dm1_v1_m101_m150_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m101_m150_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv M101-M110; "
           "DEFS.H:1652,3419,3445-3483. M103 is verified by the F0751 original "
           "PC34 byte-count helper and M104 by the F0684 C25 blit helper. M111-M150 "
           "are absent. No generated macro, graphics, UI, input, timing, memory, or "
           "platform route.";
}
