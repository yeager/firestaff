#include "dm1_v1_f0726_f0745_source_ownership_pc34_compat.h"

static const DM1_V1_F0726F0745OwnershipPc34 kOwnership[] = {
    {726, DM1_V1_F0726_F0745_NOT_A_REDMCSB_SYMBOL_PC34, "F0726", "ReDMCSB callable inventory", "No F0726 callable symbol exists; do not assign an unrelated compatibility helper."},
    {727, DM1_V1_F0726_F0745_NOT_A_REDMCSB_SYMBOL_PC34, "F0727", "ReDMCSB callable inventory", "No F0727 callable symbol exists; do not assign an unrelated compatibility helper."},
    {728, DM1_V1_F0726_F0745_NOT_A_REDMCSB_SYMBOL_PC34, "F0728", "ReDMCSB callable inventory", "No F0728 callable symbol exists; do not assign an unrelated compatibility helper."},
    {729, DM1_V1_F0726_F0745_NOT_A_REDMCSB_SYMBOL_PC34, "F0729", "ReDMCSB callable inventory", "No F0729 callable symbol exists; do not assign an unrelated compatibility helper."},
    {730, DM1_V1_F0726_F0745_NOT_A_REDMCSB_SYMBOL_PC34, "F0730", "ReDMCSB callable inventory", "No F0730 callable symbol exists; do not assign an unrelated compatibility helper."},
    {731, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0731_InvertZone", "INVRTZON.C; dm1_v1_f0731_f0734_inventory_zone_material_pc34_compat", "Raw C009 spell-zone receipt owner."},
    {732, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0732_FillScreenArea", "BLITFILL.C; dm1_v1_f0732_f0735_fill_material_pc34_compat", "Raw C009 spell-area material receipt owner."},
    {733, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0733_FillZoneByIndex", "BLITFILL.C; dm1_v1_f0732_f0735_fill_material_pc34_compat", "Raw C009 resolved-zone material receipt owner."},
    {734, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0734_ClearZoneInInventory", "BLITFILL.C; dm1_v1_f0731_f0734_inventory_zone_material_pc34_compat", "Raw C017 inventory-zone material receipt owner."},
    {735, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0735_FillViewportBox", "BLITFILL.C; dm1_v1_f0732_f0735_fill_material_pc34_compat", "Raw C017 viewport material receipt owner."},
    {736, DM1_V1_F0726_F0745_NOT_A_REDMCSB_SYMBOL_PC34, "F0736", "ReDMCSB callable inventory", "No F0736 callable symbol exists; do not assign an unrelated combat helper."},
    {737, DM1_V1_F0726_F0745_NOT_A_REDMCSB_SYMBOL_PC34, "F0737", "ReDMCSB callable inventory", "No F0737 callable symbol exists; do not assign an unrelated combat helper."},
    {738, DM1_V1_F0726_F0745_PC34_NOOP_PC34, "F0738_MUSIC_Continue", "MUSIC.C:513-524", "I34E/I34M selects an empty function body; no host playback callback."},
    {739, DM1_V1_F0726_F0745_PC34_NOOP_PC34, "F0739_MUSIC_Stop", "MUSIC.C:540-557", "All statements are F20/F31 CD routes; I34E/I34M has no source audio operation."},
    {740, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0740_MUSIC_Pause", "MUSIC.C:559-565; dm1_v1_f0740_f0743_music_source_pc34_compat", "Authenticated SONG.DAT receipt owner."},
    {741, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0741_MUSIC_PlayGameMusic", "MUSIC.C:568-581; dm1_v1_f0740_f0743_music_source_pc34_compat", "Authenticated SONG.DAT receipt owner."},
    {742, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0742_MUSIC_SetTrack", "MUSIC.C:583-595; dm1_v1_f0740_f0743_music_source_pc34_compat", "Authenticated SONG.DAT receipt owner."},
    {743, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0743_MUSIC_Update", "MUSIC.C:597-672; dm1_v1_f0740_f0743_music_source_pc34_compat", "Authenticated SONG.DAT receipt owner."},
    {744, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0744_ReplaceTildeInFileName", "FILENAME.C:60-82; redmcsb_f0744_replace_tilde_in_file_name_pc34_compat", "I34M source filename mutation owner."},
    {745, DM1_V1_F0726_F0745_EXISTING_PC34_OWNER_PC34, "F0745_SetFileNamesAccordingToLanguage", "FILENAME.C:84-105; redmcsb_f0745_set_file_names_according_to_language_pc34_compat", "I34M source language filename owner."},
};

const DM1_V1_F0726F0745OwnershipPc34 *
dm1_v1_f0726_f0745_source_ownership_pc34(unsigned int functionId) {
    if (functionId < 726U || functionId > 745U) return 0;
    return &kOwnership[functionId - 726U];
}

void dm1_v1_f0738_music_continue_pc34_noop(void) {
    /* MUSIC.C:513-524: I34E/I34M has no enclosed operation. */
}

void dm1_v1_f0739_music_stop_pc34_noop(void) {
    /* MUSIC.C:540-557: only F20/F31 CD branches have statements. */
}

const char *dm1_v1_f0726_f0745_source_ownership_evidence_pc34(void) {
    return "ReDMCSB WIP20210206 MUSIC.C:513-524,540-557,559-672; "
           "FILENAME.C:60-105; BLITFILL.C:475-491; INVRTZON.C F0731. "
           "F0726-F0730/F0736-F0737 do not occur in the ReDMCSB callable inventory.";
}
