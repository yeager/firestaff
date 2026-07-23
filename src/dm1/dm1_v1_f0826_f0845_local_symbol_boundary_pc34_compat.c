#include "dm1_v1_f0826_f0845_local_symbol_boundary_pc34_compat.h"

static const DM1_V1_F0826F0845LocalSymbolBoundaryPc34 kLocalSymbols[] = {
    {826, "L0826_ps_Champion", "REVIVE.C", "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"},
    {827, "L0827_ps_Sensor", "REVIVE.C", "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"},
    {828, "L0828_i_MapX", "REVIVE.C", "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"},
    {829, "L0829_i_MapY", "REVIVE.C", "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"},
    {830, "L0830_ai_Box", "REVIVE.C", "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"},
    {831, "L0831_ui_Multiple", "REVIVE.C", "F0283_CHAMPION_ViAltarRebirth"},
    {832, "L0832_ps_Champion", "REVIVE.C", "F0283_CHAMPION_ViAltarRebirth"},
    {833, "L0833_ui_ChampionIndex", "CHAMPION.C", "F0284_CHAMPION_SetPartyDirection"},
    {834, "L0834_i_Delta", "CHAMPION.C", "F0284_CHAMPION_SetPartyDirection"},
    {835, "L0835_ps_Champion", "CHAMPION.C", "F0284_CHAMPION_SetPartyDirection"},
    {836, "L0836_ui_ChampionIndex", "CHAMPION.C", "F0285_CHAMPION_GetIndexInCell"},
    {837, "L0837_ps_Champion", "CHAMPION.C", "F0285_CHAMPION_GetIndexInCell"},
    {838, "L0838_ui_Counter", "CHAMPION.C", "F0286_CHAMPION_GetTargetChampionIndex"},
    {839, "L0839_i_ChampionIndex", "CHAMPION.C", "F0286_CHAMPION_GetTargetChampionIndex"},
    {840, "L0840_auc_OrderedCellsToAttack", "CHAMPION.C", "F0286_CHAMPION_GetTargetChampionIndex"},
    {841, "L0841_i_Multiple", "CHAMDRAW.C", "F0287_CHAMPION_DrawBarGraphs"},
    {842, "L0842_i_Multiple", "CHAMDRAW.C", "F0287_CHAMPION_DrawBarGraphs"},
    {843, "L0843_l_Multiple", "CHAMDRAW.C", "F0287_CHAMPION_DrawBarGraphs"},
    {844, "L0844_i_BarGraphMaskInverted1", "CHAMDRAW.C", "F0287_CHAMPION_DrawBarGraphs"},
    {845, "L0845_ps_Multiple", "CHAMDRAW.C", "F0287_CHAMPION_DrawBarGraphs"},
};

const DM1_V1_F0826F0845LocalSymbolBoundaryPc34 *
dm1_v1_f0826_f0845_local_symbol_boundary_pc34(unsigned int number) {
    if (number < 826U || number > 845U) return 0;
    return &kLocalSymbols[number - 826U];
}

int dm1_v1_f0826_f0845_has_standalone_pc34_route(unsigned int number) {
    (void)number;
    return 0;
}

const char *dm1_v1_f0826_f0845_local_symbol_source_evidence_pc34(void) {
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_AUDIT.tsv L0826-L0845; "
           "REVIVE.C F0282/F0283; CHAMPION.C F0284-F0286; "
           "CHAMDRAW.C F0287. These are local storage labels, not F callables.";
}
