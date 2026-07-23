#include "csb_v1_f2166_f2205_cpsx_platform_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F2166F2205SourceAuditPc34 k_audit[] = {
    NONE(2166),
    BLOCK(2167, "IIGS.H; IO.C; DEFS.H F2167_BuildAppleIIGSMouseCursor", "fail_closed: no authenticated CSB PC34 Apple IIGS cursor owner"),
    NONE(2168), NONE(2169), NONE(2170), NONE(2171), NONE(2172), NONE(2173), NONE(2174), NONE(2175), NONE(2176), NONE(2177), NONE(2178),
    BLOCK(2179, "GSTHINGS.C; STARTUP1.C F2179_InitializeGraphicData", "fail_closed: no authenticated CSB PC34 graphics initializer"),
    NONE(2180), NONE(2181), NONE(2182), NONE(2183), NONE(2184), NONE(2185), NONE(2186), NONE(2187), NONE(2188), NONE(2189), NONE(2190),
    BLOCK(2191, "LOADSAVE.C; COMMAND.C; MOVESENS.C F2191_CPSX", "fail_closed: no authenticated CSB PC34 CPSX owner"),
    BLOCK(2192, "LOADSAVE.C; COMMAND.C; MOVESENS.C F2192_CPSX", "fail_closed: no authenticated CSB PC34 CPSX owner"),
    BLOCK(2193, "LOADSAVE.C; COMMAND.C; MOVESENS.C F2193_CPSX", "fail_closed: no authenticated CSB PC34 CPSX owner"),
    BLOCK(2194, "LOADSAVE.C; COMMAND.C; MOVESENS.C F2194_CPSX", "fail_closed: no authenticated CSB PC34 CPSX owner"),
    BLOCK(2195, "LOADSAVE.C; COMMAND.C; MOVESENS.C F2195_CPSX", "fail_closed: no authenticated CSB PC34 CPSX owner"),
    BLOCK(2196, "COMMAND.C; MOVESENS.C F2196_CPSX", "fail_closed: no authenticated CSB PC34 CPSX owner"),
    BLOCK(2197, "LOADSAVE.C; COMMAND.C; MOVESENS.C F2197_CPSX", "fail_closed: no authenticated CSB PC34 CPSX owner"),
    BLOCK(2198, "SOUND.C F2198_CPSX", "fail_closed: no authenticated CSB PC34 sound owner"),
    BLOCK(2199, "SOUND.C F2199_CPSX", "fail_closed: no authenticated CSB PC34 sound owner"),
    BLOCK(2200, "SOUND.C F2200_CPSX", "fail_closed: no authenticated CSB PC34 sound owner"),
    BLOCK(2201, "SOUND.C F2201_CPSX", "fail_closed: no authenticated CSB PC34 sound owner"),
    BLOCK(2202, "SOUND.C F2202_CPSX", "fail_closed: no authenticated CSB PC34 sound owner"),
    BLOCK(2203, "FLOPPY.C F2203_CPSX", "fail_closed: no authenticated CSB PC34 floppy owner"),
    BLOCK(2204, "FLOPPY.C F2204_CPSX", "fail_closed: no authenticated CSB PC34 floppy owner"),
    BLOCK(2205, "FLOPPY.C F2205_CPSX", "fail_closed: no authenticated CSB PC34 floppy owner")
};

#undef BLOCK
#undef NONE

const CSB_V1_F2166F2205SourceAuditPc34 *
csb_v1_f2166_f2205_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F2166F2205SourceAuditPc34 *
csb_v1_f2166_f2205_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f2166_f2205_source_audit_evidence_pc34(void)
{
    return "ReDMCSB IIGS.H, IO.C, DEFS.H, GSTHINGS.C, STARTUP1.C, LOADSAVE.C, "
           "COMMAND.C, MOVESENS.C, SOUND.C, and FLOPPY.C own the identified "
           "F2166-F2205 routes. No CSB PC34 owner is present, so all cursor, "
           "graphics, CPSX, sound, and floppy routes fail closed without authenticated "
           "material. This audit does not render or synthesize UI, graphics, timing, "
           "input, save, audio, or file behavior.";
}
