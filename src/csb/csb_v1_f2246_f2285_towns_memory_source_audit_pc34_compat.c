#include "csb_v1_f2246_f2285_towns_memory_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F2246F2285SourceAuditPc34 k_audit[] = {
    NONE(2246),
    BLOCK(2247, "FMTOWNS.H; STARTUP2.C; TOWNSIO.C F2247_RESTORE_TOWNS", "fail_closed: no authenticated CSB PC34 Towns owner"),
    BLOCK(2248, "ANIM.C; ANIMTOWN.C; STARTUP2.C F2248_PlayAnimation", "fail_closed: no authenticated CSB PC34 animation owner"),
    BLOCK(2249, "FMTOWNS.H; STARTUP2.C; TOWNSIO.C F2249_CLEAR_FRONT_SCREEN", "fail_closed: no authenticated CSB PC34 Towns screen owner"),
    NONE(2250),
    BLOCK(2251, "FLOPPY.C; CEDT029.C F2251_INSTALL_CRITICAL_HANDLER", "fail_closed: no authenticated CSB PC34 floppy owner"),
    NONE(2252),
    BLOCK(2253, "ANIMTOWN.C; IO2.C F2253_KEYREAD", "fail_closed: no authenticated CSB PC34 Towns input owner"),
    BLOCK(2254, "ANIMTOWN.C; CEDT029.C; IO2.C F2254_KEYAVAIL", "fail_closed: no authenticated CSB PC34 Towns input owner"),
    BLOCK(2255, "FMTOWNS.H; ANIMTOWN.C; TOWNSIO.C; SWSH.C F2255_SWOOSH_main", "existing CSB swoosh playback remains separate; no CSB PC34 F2255 owner"),
    BLOCK(2256, "FMTOWNS.H; MUSIC.C; ANIMTOWN.C; TOWNSIO.C F2256_CD_INIT_CPSX", "fail_closed: no authenticated CSB PC34 CD owner"),
    BLOCK(2257, "ANIM.C; MUSIC.C F2257_IsCDPlaying", "fail_closed: no authenticated CSB PC34 CD owner"),
    BLOCK(2258, "MUSIC.C; ANIMTOWN.C; TOWNSIO.C; ANIMSND.C F2258_SetVolume", "fail_closed: no authenticated CSB PC34 Towns audio owner"),
    BLOCK(2259, "FMTOWNS.H; SWITCH.C; ANIMTOWN.C; TOWNSIO.C; TOWNSSCR.C F2259_INIT_TOWNS_SCREEN", "fail_closed: no authenticated CSB PC34 Towns screen owner"),
    BLOCK(2260, "FMTOWNS.H; TOWNSIO.C; TOWNSSCR.C F2260_RESTORE_TOWNS_SCREEN", "fail_closed: no authenticated CSB PC34 Towns screen owner"),
    BLOCK(2261, "DRAWVIEW.C; SWITCH.C; ANIMTOWN.C F2261_Palette", "fail_closed: no authenticated CSB PC34 Towns palette owner"),
    BLOCK(2262, "FMTOWNS.H; SWITCH.C; ANIMTOWN.C; TOWNSIO.C F2262_TIMER_A_EVENT", "csb_v1_f2262_timer_a_event_pc34_compat preserved separately"),
    BLOCK(2263, "FMTOWNS.H; SWITCH.C; ANIMTOWN.C; TOWNSIO.C F2263_INIT_TIMER", "fail_closed: no authenticated CSB PC34 Towns timer owner"),
    BLOCK(2264, "FMTOWNS.H; SWITCH.C; ANIMTOWN.C; TOWNSIO.C; ANIMSND.C F2264_INIT_PCM_SOUND", "fail_closed: no authenticated CSB PC34 PCM owner"),
    NONE(2265), NONE(2266), NONE(2267), NONE(2268), NONE(2269), NONE(2270), NONE(2271), NONE(2272), NONE(2273), NONE(2274), NONE(2275), NONE(2276),
    BLOCK(2277, "CEDT029.C F2277_KEYREAD", "fail_closed: no authenticated CSB PC34 key owner"),
    NONE(2278), NONE(2279), NONE(2280),
    BLOCK(2281, "MEM1CLCP.C; COMPILE.H; MEM1MAIN.C F2281_MEM1_05_ClearBytes", "fail_closed: no authenticated CSB PC34 MEM1 owner"),
    NONE(2282), NONE(2283), NONE(2284), NONE(2285)
};

#undef BLOCK
#undef NONE

const CSB_V1_F2246F2285SourceAuditPc34 *
csb_v1_f2246_f2285_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F2246F2285SourceAuditPc34 *
csb_v1_f2246_f2285_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f2246_f2285_source_audit_evidence_pc34(void)
{
    return "ReDMCSB FMTOWNS.H, STARTUP2.C, TOWNSIO.C, ANIM.C, ANIMTOWN.C, "
           "FLOPPY.C, CEDT029.C, IO2.C, SWSH.C, MUSIC.C, ANIMSND.C, SWITCH.C, "
           "TOWNSSCR.C, DRAWVIEW.C, MEM1CLCP.C, COMPILE.H, and MEM1MAIN.C own the "
           "identified F2246-F2285 routes. Existing CSB F2262 Timer A ownership "
           "remains separate. No other CSB PC34 owner is present, so all routes fail "
           "closed without authenticated material. This audit does not render or synthesize "
           "graphics, UI, timing, input, audio, CD, floppy, or memory behavior.";
}
