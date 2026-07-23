#include "dm1_v1_f1266_f1285_source_ownership_pc34_compat.h"

static const DM1_V1_F1266F1285OwnershipPc34 kOwnership[] = {
    {1266, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1266_i_SlotIndex", "MENU.C F0411", "Function-local flask-slot storage; no standalone F1266 route."},
    {1267, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1267_ui_Multiple", "MENU.C F0412", "Function-local spell-cast storage; no standalone F1267 route."},
    {1268, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1268_i_PowerSymbolOrdinal", "MENU.C F0412", "Function-local spell-cast storage; no standalone F1268 route."},
    {1269, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1269_ui_Multiple", "MENU.C F0412", "Function-local spell-cast storage; no standalone F1269 route."},
    {1270, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1270_ps_Champion", "MENU.C F0412", "Function-local spell-cast storage; no standalone F1270 route."},
    {1271, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1271_ps_Spell", "MENU.C F0412", "Function-local spell-cast storage; no standalone F1271 route."},
    {1272, DM1_V1_F1266_F1285_PLATFORM_BOUNDARY_PC34, "F1272_Initialize_CPSX", "SWSH.C:915; SWSHIIGS.C", "No authenticated PC34 swoosh media/initialization receipt; no generated startup route."},
    {1273, DM1_V1_F1266_F1285_PLATFORM_BOUNDARY_PC34, "F1273_Cleanup", "SWSH.C:2870", "No authenticated PC34 swoosh cleanup route; no host media teardown substitute."},
    {1274, DM1_V1_F1266_F1285_PLATFORM_BOUNDARY_PC34, "F1274_Unreferenced", "SWSH.C:2878", "Unreferenced swoosh route; no authenticated PC34 media owner."},
    {1275, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1275_ps_Potion", "MENU.C F0412", "Function-local spell-cast storage; no standalone F1275 route."},
    {1276, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1276_s_Event", "MENU.C F0412", "Function-local spell-cast storage; no standalone F1276 route."},
    {1277, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1277_ps_Junk", "MENU.C F0412", "Function-local spell-cast storage; no standalone F1277 route."},
    {1278, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1278_pc_TildeCharacter", "SAVEPATH.C F0414", "Function-local save-path storage; no standalone F1278 route."},
    {1279, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1279_ui_TildeCharacterPosition", "SAVEPATH.C F0414", "Function-local save-path storage; no standalone F1279 route."},
    {1280, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1280_l_ByteCount", "READWRIT.C F0415", "Function-local read-result storage; no standalone F1280 route."},
    {1281, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1281_ul_ByteCount", "READWRIT.C F0416", "Function-local write-result storage; no standalone F1281 route."},
    {1282, DM1_V1_F1266_F1285_PLATFORM_BOUNDARY_PC34, "F1282_WriteString", "AMIGINIT.C:308", "Amiga host writer; no PC34 console/UI substitute."},
    {1283, DM1_V1_F1266_F1285_PLATFORM_BOUNDARY_PC34, "F1283_WriteCharacter_Unreferenced", "AMIGINIT.C:312", "Amiga host writer; no PC34 console/UI substitute."},
    {1284, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1284_B_IsWriteObfuscatedBytesSuccessful", "READWRIT.C F0420", "Function-local save-write storage; no standalone F1284 route."},
    {1285, DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34, "L1285_i_WordCount", "READWRIT.C F0420", "Function-local save-write storage; no standalone F1285 route."},
};

const DM1_V1_F1266F1285OwnershipPc34 *
dm1_v1_f1266_f1285_source_ownership_pc34(unsigned int number)
{
    if (number < 1266U || number > 1285U) return 0;
    return &kOwnership[number - 1266U];
}

int dm1_v1_f1266_f1285_admits_authentic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_f1266_f1285_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1266_f1285_source_ownership_evidence_pc34(void)
{
    return "ReDMCSB MENU.C F0411-F0412; SAVEPATH.C F0414; READWRIT.C "
           "F0415-F0420; SWSH.C:915,2870,2878; SWSHIIGS.C; AMIGINIT.C:308,312. "
           "No generated UI, graphics, timing, swoosh, text, or save route.";
}
