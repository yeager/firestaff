#include "csb_v1_f1246_f1265_source_boundary_pc34_compat.h"

static const CSB_V1_F1246F1265SourceBoundaryReceiptPc34 k_receipts[] = {
    { 1246u, "MENU.C:1215 L1246_i_Multiple, F0407 local", "local", 1, 1, 1 },
    { 1247u, "MENU.C:1204 L1247_ps_Champion, F0407 local", "local", 1, 1, 1 },
    { 1248u, "MENU.C:1205 L1248_ps_Weapon, F0407 local", "local", 1, 1, 1 },
    { 1249u, "MENU.C:1228 L1249_ui_ActionDisabledTicks, F0407 local", "local", 1, 1, 1 },
    { 1250u, "MENU.C:1229 L1250_i_Multiple, F0407 local", "local", 1, 1, 1 },
    { 1251u, "MENU.C:1230 L1251_i_MapX, F0407 local", "local", 1, 1, 1 },
    { 1252u, "MENU.C:1241 L1252_i_MapY, F0407 local", "local", 1, 1, 1 },
    { 1253u, "ANIM.C:1379 F1253_FadeOutSound; MENU.C:1246 L1253", "DM1 audio/local", 1, 1, 1 },
    { 1254u, "MENU.C:1247 L1254_i_ActionSkillIndex, F0407 local", "local", 1, 1, 1 },
    { 1255u, "CNFG.C:144 F1255_CheckMasterDisk; MENU.C:1248 L1255", "DM1 config/local", 1, 1, 1 },
    { 1256u, "MENU.C:1213 L1256_ps_WeaponInfoActionHand, F0407 local", "local", 1, 1, 1 },
    { 1257u, "CNFG.C:201 F1257_CheckMemory; MENU.C:1214 L1257", "DM1 config/local", 1, 1, 1 },
    { 1258u, "CNFG.C:212 F1258_CheckPrerequisites; MENU.C:1253 L1258", "DM1 config/local", 1, 1, 1 },
    { 1259u, "MENU.C:1646 L1259_i_SpellCastResult, F0408 local", "local", 1, 1, 1 },
    { 1260u, "MENU.C:1644 L1260_ps_Champion, F0408 local", "local", 1, 1, 1 },
    { 1261u, "MENU.C:1673 L1261_l_Symbols, F0409 local", "local", 1, 1, 1 },
    { 1262u, "MENU.C:1676 L1262_i_Multiple, F0409 local", "local", 1, 1, 1 },
    { 1263u, "MENU.C:1675 L1263_ps_Spell, F0409 local", "local", 1, 1, 1 },
    { 1264u, "SPELFAIL.C:13 L1264_pc_Message, F0410 local", "local", 1, 1, 1 },
    { 1265u, "MENU.C:1728 L1265_T_Thing, F0411 local", "local", 1, 1, 1 }
};

const CSB_V1_F1246F1265SourceBoundaryReceiptPc34 *
csb_v1_f1246_f1265_source_boundary_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_receipts) / sizeof(k_receipts[0]);
    return k_receipts;
}

const CSB_V1_F1246F1265SourceBoundaryReceiptPc34 *
csb_v1_f1246_f1265_source_boundary_find_pc34(unsigned int number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_receipts) / sizeof(k_receipts[0]); ++index) {
        if (k_receipts[index].symbol_number == number) return &k_receipts[index];
    }
    return 0;
}

const char *csb_v1_f1246_f1265_source_boundary_evidence_pc34(void)
{
    return "ReDMCSB MENU.C, SPELFAIL.C, ANIM.C, and CNFG.C classify "
           "F1246-F1265 as DM1 locals, audio, and config routes. No "
           "authenticated CSB PC34 owner is proven, so every CSB route fails "
           "closed and this receipt does not render, synthesize UI, or create timing.";
}
