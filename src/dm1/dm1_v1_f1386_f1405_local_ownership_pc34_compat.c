#include "dm1_v1_f1386_f1405_local_ownership_pc34_compat.h"

static const DM1_V1_F1386F1405LocalOwnershipPc34 kOwnership[] = {
    {1386, "L1386_apuc_Bitmap_Screens", "F0437_STARTEND_DrawTitle", "TITLE.C"},
    {1387, "L1387_apuc_Bitmap_ShrinkedTitle", "F0437_STARTEND_DrawTitle", "TITLE.C"},
    {1388, "L1388_puc_Bitmap_Temporary", "F0437_STARTEND_DrawTitle", "TITLE.C"},
    {1389, "L1389_puc_Bitmap_Master_StrikesBack", "F0437_STARTEND_DrawTitle", "TITLE.C"},
    {1390, "L1390_ac_Unreferenced", "F0437_STARTEND_DrawTitle", "TITLE.C"},
    {1391, "L1391_aai_Coordinates", "F0437_STARTEND_DrawTitle", "TITLE.C"},
    {1392, "L1392_aui_Palette", "F0437_STARTEND_DrawTitle", "TITLE.C"},
    {1393, "L1393_ui_AnimationStep", "F0438_STARTEND_OpenEntranceDoors", "ENTRANCE.C"},
    {1394, "L1394_ppuc_Bitmap_EntranceDoorAnimationSteps", "F0438_STARTEND_OpenEntranceDoors", "ENTRANCE.C"},
    {1395, "L1395_pl_Bitmap_EntranceDoor", "F0438_STARTEND_OpenEntranceDoors", "ENTRANCE.C"},
    {1396, "L1396_pc_Bitmap_CompositeDungeonViewAndDoors", "F0438_STARTEND_OpenEntranceDoors", "ENTRANCE.C"},
    {1397, "L1397_ui_ColumnIndex", "F0439_STARTEND_DrawEntrance/F0797", "ENTRANCE.C"},
    {1398, "L1398_apuc_MicroDungeonCurrentMapData", "F0439_STARTEND_DrawEntrance/F0797", "ENTRANCE.C"},
    {1399, "L1399_auc_MicroDungeonSquares", "F0439_STARTEND_DrawEntrance/F0797", "ENTRANCE.C"},
    {1400, "L1400_s_MicroDungeonMap", "F0439_STARTEND_DrawEntrance/F0797", "ENTRANCE.C"},
    {1401, "L1401_l_GraphicDecompressedByteCount", "F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount", "MEMORY.C; ENTRANCE.C"},
    {1402, "L1402_ui_AnimationStep", "F0441_STARTEND_ProcessEntrance", "ENTRANCE.C"},
    {1403, "L1403_ul_ByteCount", "F0441_STARTEND_ProcessEntrance", "ENTRANCE.C"},
    {1404, "L1404_i_Unreferenced", "F0441_STARTEND_ProcessEntrance", "ENTRANCE.C"},
    {1405, "L1405_ai_Box", "F0441_STARTEND_ProcessEntrance", "ENTRANCE.C"},
};

const DM1_V1_F1386F1405LocalOwnershipPc34 *
dm1_v1_f1386_f1405_local_ownership_pc34(unsigned int number)
{
    if (number < 1386U || number > 1405U) return 0;
    return &kOwnership[number - 1386U];
}

int dm1_v1_f1386_f1405_admits_standalone_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_f1386_f1405_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1386_f1405_local_ownership_evidence_pc34(void)
{
    return "ReDMCSB TITLE.C F0437; ENTRANCE.C F0438-F0441/F0797; MEMORY.C. "
           "All F1386-F1405 identifiers are L-local storage; no generated "
           "title, entrance, graphics, palette, timing, or UI route exists.";
}
