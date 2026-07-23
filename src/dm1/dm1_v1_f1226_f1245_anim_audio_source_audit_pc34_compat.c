#include "dm1_v1_f1226_f1245_anim_audio_source_audit_pc34_compat.h"

static const DM1_V1_F1226F1245SourceAuditPc34 k_audit[] = {
    { 1226u, "ANIM.C:1456 F1226_", "fail_closed: Amiga display bitmap-plane route", 1, 1, 1, 1 },
    { 1227u, "ANIM.C:1472 F1227_", "fail_closed: Amiga double-buffer/palette route", 1, 1, 1, 1 },
    { 1228u, "ANIM.C:1495 F1228_", "fail_closed: Amiga blitter copy route", 1, 1, 1, 1 },
    { 1229u, "no numbered F1229 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 1230u, "ANIMSND.C:100 F1230_CleanupSound", "fail_closed: Amiga audio.device cleanup route", 1, 1, 1, 1 },
    { 1231u, "ANIMSND.C:160 F1231_", "fail_closed: Amiga IOAudio request setup", 1, 1, 1, 1 },
    { 1232u, "ANIMSND.C:175 F1232_", "fail_closed: Amiga sound channel CheckIO route", 1, 1, 1, 1 },
    { 1233u, "ANIMSND.C:184 F1233_", "fail_closed: Amiga sound channel CheckIO route", 1, 1, 1, 1 },
    { 1234u, "ANIMSND.C:193 F1234_", "fail_closed: Amiga audio finish/stop route", 1, 1, 1, 1 },
    { 1235u, "ANIMSND.C:211 F1235_", "fail_closed: Amiga synchronized audio stop route", 1, 1, 1, 1 },
    { 1236u, "ANIMSND.C:228 F1236_", "fail_closed: Amiga sound-channel selection route", 1, 1, 1, 1 },
    { 1237u, "ANIMSND.C:243 F1237_", "fail_closed: Amiga sound bank one control", 1, 1, 1, 1 },
    { 1238u, "ANIMSND.C:249 F1238_", "fail_closed: Amiga sound bank one control", 1, 1, 1, 1 },
    { 1239u, "ANIMSND.C:255 F1239_", "fail_closed: Amiga sound bank one status", 1, 1, 1, 1 },
    { 1240u, "ANIMSND.C:261 F1240_", "fail_closed: Amiga sound bank one playback", 1, 1, 1, 1 },
    { 1241u, "ANIMSND.C:290 F1241_", "fail_closed: Amiga sound bank two control", 1, 1, 1, 1 },
    { 1242u, "ANIMSND.C:296 F1242_", "fail_closed: Amiga sound bank two control", 1, 1, 1, 1 },
    { 1243u, "ANIMSND.C:302 F1243_", "fail_closed: Amiga sound bank two status", 1, 1, 1, 1 },
    { 1244u, "ANIMSND.C:308 F1244_", "fail_closed: Amiga sound bank two playback", 1, 1, 1, 1 },
    { 1245u, "ANIMSND.C:337 F1245_", "fail_closed: Amiga dual-bank sound stop", 1, 1, 1, 1 }
};

const DM1_V1_F1226F1245SourceAuditPc34 *
dm1_v1_f1226_f1245_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1226F1245SourceAuditPc34 *
dm1_v1_f1226_f1245_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1226_f1245_source_audit_evidence_pc34(void)
{
    return "ReDMCSB ANIM.C and ANIMSND.C are the authority for F1226-F1245. "
           "F1229 has no numbered source body in the audited corpus. Bitmap "
           "planes, blitter, display, and audio.device routes remain fail closed "
           "without authentic raw PC34 material. The audit does not render or "
           "synthesize UI or timing paths.";
}
