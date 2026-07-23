#include "csb_v1_f1766_f1805_media_source_boundary_pc34_compat.h"

#include <string.h>

static const char *anchor_for(unsigned int number)
{
    switch (number) {
    case 1766u: return "UTDEBUG.C:215 unreferenced debug helper";
    case 1769u: return "DEFS.H:9635 GetLastErrorCode";
    case 1779u: return "PALETTE.C:428 SwapPalette";
    case 1785u: return "SOUND.C:6 SOUND_RestoreKeyClick";
    case 1788u: return "SOUND.C:1383 PlaySound";
    case 1789u: return "SOUND.C:1388 unreferenced";
    case 1790u: return "SOUND.C:1392 unreferenced";
    case 1792u: return "ANIM.C:24 LoadAnimationScript";
    case 1795u: return "ANIM.C:118 Prepare";
    case 1796u: return "ANIM.C:159 Clean";
    case 1797u: return "ANIM.C:171 ReadInstructionAndParameters";
    case 1799u: return "ANIM.C:67 LoadAnimationData";
    case 1803u: return "ANIMLOAD.C:164 unreferenced";
    default: return "no numbered callable body in ReDMCSB corpus";
    }
}

static const char *boundary_for(unsigned int number)
{
    if (number == 1779u)
        return "existing CSB F0904 palette owner remains exclusive";
    if (number == 1785u || (number >= 1788u && number <= 1790u))
        return "DM1 sound owner; no CSB PC34 sound consumer";
    if (number == 1792u || (number >= 1795u && number <= 1797u) || number == 1799u)
        return "DM1 ANIM owner; no CSB PC34 animation consumer";
    return "fail_closed: absent, debug, or non-CSB source";
}

int csb_v1_f1766_f1805_media_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F1766F1805MediaSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F1766F1805MediaSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 1766u || number > 1805u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_anchor = anchor_for(number);
    receipt.existing_owner_or_boundary = boundary_for(number);
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f1766_f1805_media_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB UTDEBUG.C, PALETTE.C, SOUND.C, ANIM.C, and ANIMLOAD.C "
           "are the authority for F1766-F1805. Existing palette, DM1 sound, and "
           "DM1 animation owners remain exclusive. No authenticated CSB PC34 "
           "media consumer is proven, so every CSB route fails closed without "
           "synthetic UI, graphics, audio, or timing.";
}
