#include "dm2_v1_quirk_decisions.h"

static const DM2_V1_QuirkDecision g_dm2_v1_quirk_decisions[] = {
    {
        DM2_V1_QUIRK_DUNGEON_DAT_EDITS_NEW_GAME_ONLY,
        DM2_V1_QUIRK_STATUS_EMULATE_ORIGINAL,
        "dm2_v1_dungeon_dat_edits_new_game_only",
        "Dungeon.dat Edits Only Affect New Games",
        "Existing saves keep using the dungeon snapshot embedded in the save; "
        "changed dungeon.dat files are only consumed by a fresh new-game path.",
        "docs/dm2_bugs.md: Dungeon.dat Edits Only Affect New Games\n"
        "docs/dm2_modding.md: dmgame/SKSave state derives from dungeon.dat at save time\n"
        "docs/dm2_save_format.md: SKSave embeds dungeon header, maps, stacks, text, DB pools, map data, and extra dungeon data\n"
        "SKULL.ASM: skload_table_60 + STORE_EXTRA_DUNGEON_DATA save/load surface\n"
        "ReDMCSB LOADSAVE.C F0435 lines 2721-2749 restores serialized GLOBAL_DATA before runtime rebuild\n"
        "ReDMCSB SAVEHEAD.C F0429/F0430 lines 13-109 read/write the save header before part payloads",
        1,
        0,
        1
    }
};

const DM2_V1_QuirkDecision *dm2_v1_quirk_decision_get(
    DM2_V1_QuirkDecisionId id)
{
    if ((int)id < 0 || id >= DM2_V1_QUIRK_DECISION_COUNT) {
        return 0;
    }
    return &g_dm2_v1_quirk_decisions[(int)id];
}

const char *dm2_v1_quirk_status_name(DM2_V1_QuirkDecisionStatus status)
{
    switch (status) {
        case DM2_V1_QUIRK_STATUS_UNDECIDED:
            return "UNDECIDED";
        case DM2_V1_QUIRK_STATUS_EMULATE_ORIGINAL:
            return "EMULATE_ORIGINAL";
        case DM2_V1_QUIRK_STATUS_GUARD_MODERN:
            return "GUARD_MODERN";
        case DM2_V1_QUIRK_STATUS_DOCUMENT_ONLY:
            return "DOCUMENT_ONLY";
        default:
            return "UNKNOWN";
    }
}
